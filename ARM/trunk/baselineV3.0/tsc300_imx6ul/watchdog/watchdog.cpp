#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "linux.h"
#include <string>

#define LOCKFILE "/var/watchdog.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

//成功返回0，若有错误返回-1，错误原因存于errno。

int lockfile(int ff)
{
    struct flock fl;
    
    //l_type是设置锁定的状态。它有三种状态：F_RDLCK是建立一个供读取用的锁定；F_WRLCK是建立一个供写入用的锁定；F_UNLCK是删除之前建立的锁定。

     fl.l_type = F_WRLCK;

    //l_start是设置锁定区域的开头位置

     fl.l_start = 0;

    //l_whenec是决定l_start的位置。它有三种状态：SEEK_SET是以文件开头为锁定的起始位置；SEEK_CUR是以目前文件读写位置为锁定的起始位置；SEEK_END是以文件结尾为锁定的起始位置。

     fl.l_whence = SEEK_SET;

    //l_len是设置锁定区域的大小

     fl.l_len = 0;
    //还有一个l_pid是设置锁定动作的进程

    
    //fcntl()是唯一的符合POSIX标准的文件锁实现，所以也是唯一可移植的，功能比较强大。

    //F_SETLK的作用：在l_whence、l_start、l_len等都设置好的情况下，如果l_type被设置为F_RDLCK或F_WRLCK就分配一个锁，如果l_type被设置为F_UNLCK就释放一个锁。如果失败就返回-1，并把errno的错误设为EACCES或EAGAIN。

    return fcntl(ff, F_SETLK, &fl);
}

void already_running(void)
{
    int fd;
    char buf[16]    ;

    //此处的open是系统调用函数，第一个参数是打开的文件路径；第二个参数是对文件操作的权限，此处是可读写，可创建（如果文件不存在就会自动创建它）；第三个参数是文件被创建后给文件的权限，比如S_IRUSR相当于400，权此用户有读权限。

     fd = open(LOCKFILE,O_RDWR|O_CREAT,LOCKMODE);

    if (fd < 0)
    {
        //strerror()，返回一个合适的string型的错误

         syslog(LOG_ERR,"can't open %s: %s",LOCKFILE,strerror(errno));
        exit(1);
    }

    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            exit(1);
        }
         syslog(LOG_ERR,"can't lock %s: %s",LOCKFILE,strerror(errno));
        exit(1);
    }
    //把fd指定的文件大小改为0。参数fd为已打开的文件描述词，而且必须是以写入模式打开的文件。之所以要把文件长度截短为0，是因为上一个进程的ID字符串可以长于当前进程的ID字符串。

     ftruncate(fd,0);

    //把整型转换成字符串

    sprintf(buf,"%d",getpid());

    //把转换成字符串的进程ID号写入文件/var/run/*.pid中

    write(fd,buf,strlen(buf));
}


static hik::watchdog dog;

static void SigHandle(int sigNum)
{
    
    switch (sigNum)
    {
        case SIGSEGV:   printf("This program has sigmentfault"); dog.disable();break;
        case SIGINT:    printf("This program has been interrupted by ctrl+C"); dog.disable();break;
        case SIGTERM:   printf("This program has been interrupted by command 'kill' or 'killall'"); dog.disable();break;
    //    case SIGUSR1:   gOftenPrintFlag = !gOftenPrintFlag; return;
    }
    exit(1);
}

int main(int argc,char *argv[])
{
    int childpid,fd,fdtablesize;

    int fp;
    
    //屏蔽一些有关控制终端操作的信号。防止在守护进程没有正常运转起来时，控制终端受到干扰退出或挂起，此处忽略了终端I/O信号、STOP信号

    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGHUP,SIG_IGN);

    //由于子进程会继承父进程的某些特性，如控制终端、登录会话、进程组等，而守护进程最终要脱离控制终端到后台去运行，所以必须把父进程杀掉，以确保子进程不是进程组长，这也是成功调用setsid()所要求的。

    if (fork() != 0)
    {
        exit(1);
    }
        
    //脱离了控制终端还要脱离登录会话和进程组，这里可以调用setsid()函数，调用成功后程成为新的会话组长和新的进程组长，并与原来的登录会话和进程组脱离，由于会话过程对控制终端的独占性，进程同时与控制终端脱离。

    if (setsid() < 0)
    {
     exit(1);
    }

    //要达到setsid()函数的功能也可以用如下处理方法。"/dev/tty"是一个流设备，也是终端映射，调用close()函数将终端关闭。
/*
    if ((fp=open("/dev/tty",O_RDWR)) >= 0)
    {
         ioctl(fp,TIOCNOTTY,NULL);
        close(fp);
    }
*/
    //进程已经成为无终端的会话组长，但它可以重新申请打开一个新的控制终端。可以通过不再让进程成为会话组长的方式来禁止进程重新打开控制终端，需要再次调用fork函数。

    if (fork() != 0)
    {
        exit(1);
    }
    
    //从父进程继承过来的当前工作目录可能在一个装配的文件系统中。因为守护进程通常在系统重启之前是一直存在的，所以如果守护进程的当前工作目录在一个装配文件系统中，那么该文件系统就不能被卸载。比如说从父进程继承的当前目录是/mnt下面的一个被挂载的目录。

    if (chdir("/tmp") == -1)
    {
        exit(1);
    }

    //关闭打开的文件描述符，或重定向标准输入、标准输出和标准错误输出的文件描述符。进程从创建它的父进程那里继承了打开的文件描述符。如果不关闭，将会浪费系统资源，引起无法预料的错误。getdtablesize()返回某个进程所能打开的最大的文件数。

    for (fd = 0,fdtablesize = getdtablesize(); fd < fdtablesize ; fd++)
    {
        close(fd);
    }

    //有的程序有些特殊的需求，还需要将这三者重新定向。

/*
	error=open("/tmp/error",O_WRONLY|O_CREAT,0600);
	dup2(error,2);
	close(error);
	in=open("/tmp/in",O_RDONLY|O_CREAT,0600);
	if(dup2(in,0)==-1) perror("in");
	close(in);
	out=open("/tmp/out",O_WRONLY|O_CREAT,0600);
	if(dup2(out,1)==-1) perror("out");
	close(out);
*/

    //由继承得来的文件方式创建的屏蔽字可能会拒绝设置某些权限，所以要重新赋于所有权限。

	umask(0);

    //如果父进程不等待子进程结束，子进程将成为僵尸进程（zombie）从而占用系统资源，如果父进程等待子进程结束，将增加父进程的负担，影响服务器进程的并发性能。因此需要对SIGCHLD信号做出处理，回收僵尸进程的资源，避免造成不必要的资源浪费。

    signal(SIGCHLD,SIG_IGN);

    //守护进程不属于任何终端，所以当需要输出某些信息时，它无法像一般程序那样将信息直接输出到终端，可以使用linux中自带的syslogd守护进程，它向用户提供了syslog()系统调用函数。信息都保存在/var/log/syslog文件中。

	syslog(LOG_USER|LOG_INFO,"守护进程测试!/n");

    //判断守护进程是否正在运行
    already_running();

    using namespace hik;
    
    int gap;
    std::string period,timegap;

    if (argc != 5)
        return -1;
    
    std::string key = argv[1];
    if (key == "-T")
    {
        period = argv[2];
        dog.enable(std::atoi(period.data()));
    }
    else if (key == "-t")
    {
        timegap = argv[2];
        gap = std::atoi(timegap.data());
    }

    key = argv[3];
    if (key == "-T")
    {
        period = argv[4];
        dog.enable(std::atoi(period.data()));
    }
    else if (key == "-t")
    {
        timegap = argv[4];
        gap = std::atoi(timegap.data());
    }

    //捕获终止信号，并关闭watchdog暂用的句柄
    signal(SIGSEGV, SigHandle);
    signal(SIGINT, SigHandle);      //for ctrl + c
    signal(SIGTERM, SigHandle);    //for command 'kill' or 'killall'

    while (1)
    {
        dog.feed();
        sleep(gap); 

    }
    return 0;
}