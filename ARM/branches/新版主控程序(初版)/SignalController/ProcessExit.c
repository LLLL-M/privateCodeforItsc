/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : ProcessExit.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : 主要用来测试主进程获取子进程的状态，需要注意的是如果子进程通
               过监控ctrl+c退出子进程，父进程必须要忽略ctrl+c产生的信号的影
               响，否则父进程可能已经先退出，故无法检测到子进程的退出
  函数列表   :
              main
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>



int main(void)
{
    pid_t nchildPid;
    int status = 0;

    nchildPid = fork();

    if(nchildPid < 0)
    {
        printf("fork error \n");
    }
    else if (nchildPid == 0)
    {
        printf("child process start ...\n");
    	signal(SIGHUP, SIG_IGN);
    	signal(SIGPIPE, SIG_IGN);
    	signal(SIGQUIT, SIG_IGN);
	    signal(SIGTERM, SIG_IGN);
	    
        sleep(3);
        printf("child process exit ....\n");
        exit(4);
    }
    else
    {
        printf("parent .....\n");
        waitpid(nchildPid ,&status,0);
        printf("catch child process exit , status 0x%x\n",status);
        

    }



    return 0;
}



