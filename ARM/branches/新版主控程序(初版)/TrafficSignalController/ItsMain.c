/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : ItsMain.c
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月29日
  最近修改   :
  功能描述   : 主要初始化了通信模块、定时器模块、策略调度模块等等
  函数列表   :
              CommunicationModule
              createThread
              getMsgId
              InitGolobalVar
              init_communication_server
              itstaskmain
              ThreadHardYellowLight
              TimerModule
  修改历史   :
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "common.h"
#include "LogSystem.h"
#include "Util.h"
#include "HikConfig.h"
#include "HikMsg.h"
#include "Database.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define LOG_PATH    "./logDir/"
#define LOG_MAX_SIZE    200//M
#define LOG_MAX_SIZE_EACH   10//M

#define IPPORT	8888
#define SA(x) ((struct sockaddr *)&x)

#define FILE_LEN	(4096 * 16) //64k

#define FAULT_RED_GREEN_CONFLICT    0xb
#define FAULT_RED_GREEN_CLEAR       0xc
#define FAULT_RED_OFF               0xd
#define FAULT_RED_OFF_CLEAR         0xe
#define FAULT_GREEN_CONFLICT        0xf
#define FAULT_GREEN_CLEAR           0x10
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern struct FAILURE_INFO s_failure_info; 
extern struct SPECIAL_PARAMS s_special_params;
typedef unsigned short  uint16;
extern int PhaseLampOutput(int boardNum, uint16 outLamp); 
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void HardflashDogOutput(void);
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
int msgid;
//信号机配置参数主结构全局变量指针，用来存放配置信息
SignalControllerPara *gSignalControlpara = NULL;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : getMsgId
 功能描述  : 创建一个消息队列并返回msgid，如果已经创建则直接返回已经创建的msgid
 输入参数  : int proj_id  幻数，一般就是一个字符
 返 回 值  : 返回创建好的消息队列的msgid
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int getMsgId(int proj_id)
{
	key_t key = ftok("/dev/null", proj_id);
	int id = -1;
	if (key == -1) 
	{
		return -1;
	}
	id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
	if (id == -1 && errno == EEXIST) 
	{
		id = msgget(key, 0666);
	}
	return id;
}


/*****************************************************************************
 函 数 名  : InitGolobalVar
 功能描述  : 初始化全局参数：日志参数、配置参数、CAN通信
 输入参数  : 无
 返 回 值  : 

 修改历史      :
  1.日    期   : 2014年7月30日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static void InitGolobalVar()
{
    //初始化本地日志系统        在PC机是打开,在嵌入式板卡上默认是关闭的。
#ifndef ARM_PLATFORM
    //初始化本地日志系统
    InitLogSys(LOG_PATH, LOG_MAX_SIZE, LOG_MAX_SIZE_EACH);
#endif

    log_debug("===============System  Start===============\n");

    //初始化全局的信号机配置参数
    gSignalControlpara = (PSignalControllerPara)malloc(sizeof(SignalControllerPara));
    if(!gSignalControlpara)
    {
        log_error("error to alloc memory for gSignalControlpara  : %s\n",strerror(errno));
		exit(1);
    }
    memset(gSignalControlpara,0,sizeof(SignalControllerPara));
    
}

/*****************************************************************************
 函 数 名  : init_communication_server
 功能描述  : 通信模块的udp服务器初始化
 输入参数  : void  
 返 回 值  : 正确返回sockfd，错误返回-1
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int init_communication_server(void)
{
	int sockfd;
	struct sockaddr_in addr = 
	{
		.sin_family = PF_INET,
		.sin_port = htons(IPPORT),
		.sin_zero = {0},
	};
	socklen_t len = sizeof(struct sockaddr);
	struct ifreq ifr;
	int opt = 1;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		log_error("create socket fail, error info: %s\n", strerror(errno));
		return -1;
	}
	//获取eth0的IP地址
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) 
	{
		log_error("get eth0 ip fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	} 
	else 
	{
		addr.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
		log_debug("eth0 ip: %s\n", inet_ntoa(addr.sin_addr));
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(int));
	if (bind(sockfd, SA(addr), len) == -1) 
	{
		log_error("bind socket fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	return sockfd;
}

/*****************************************************************************
 函 数 名  : CommunicationModule
 功能描述  : 通信模块的线程处理函数，主要是使用udp服务器接收外部的配置信息，
             然后通过消息队列传给信息给策略调度模块
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *CommunicationModule(void *arg)
{
	struct msgbuf msg = {MSG_CONFIG_UPDATE, {0}};	//发送给策略控制模块的消息
	int sockfd = init_communication_server();
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr);
	char buf[FILE_LEN];
	ssize_t recvsize, size = sizeof(SignalControllerPara);

	if (sockfd == -1) 
	{
		pthread_exit(NULL);
	}

	LoadDataFromCfg(gSignalControlpara, NULL);	//装载配置
	if (IsSignalControlparaLegal(gSignalControlpara) == 0) 
	{	//检查配置是否合理
		msgsnd(msgid, &msg, MSGSIZE, 0);
	}
	//与Gohead服务器通信
	while (1) 
	{
		memset(buf, 0, FILE_LEN);
		recvsize = recvfrom(sockfd, buf, FILE_LEN, 0, SA(addr), &len);
		if (recvsize >= size) 
		{
			memcpy(gSignalControlpara, buf, size);
			if (IsSignalControlparaLegal(gSignalControlpara) == 0) 
			{	//检查配置是否合理
				msgsnd(msgid, &msg, MSGSIZE, 0);
				WriteConfigFile(gSignalControlpara, NULL);
				log_debug("config file is update!\n");
				break;
			}
		}
	}

	pthread_exit(NULL);
}

/*****************************************************************************
 函 数 名  : TimerModule
 功能描述  : 定时器模块的线程处理函数，主要用来接收策略调度模块的定时消息，
             然后定时1s，最后再给策略模块发送定时完成的消息
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *TimerModule(void *arg)
{
	struct msgbuf msg = {MSG_TIMER_START, {0}};
	struct timespec start = {0, 0}, end = {0, 0};
	long gap;
	
	//阻塞接收定时器启动的消息
	while (msgrcv(msgid, &msg, MSGSIZE, MSG_TIMER_START, 0) == -1) 
	{
		log_error("timer receive MSG_TIMER_START error:%s\n", strerror(errno));
		usleep(10);
	}
	clock_gettime(CLOCK_MONOTONIC, &start);	//记录下起始时间
	msg.mtype = MSG_TIMER_COMPLETE;
	while (1)	//每DELAY_TIME ms循环一次
	{
		usleep(DELAY_TIME_USECS_BEFORE);	//先延时DELAY_TIME_USECS_BEFORE us,在最后的1ms内精确延时
		while (1)
		{
			usleep(50);	//每次延时50us
			clock_gettime(CLOCK_MONOTONIC, &end);
			gap = (end.tv_sec - start.tv_sec) * ONE_SECOND_NSECS + end.tv_nsec - start.tv_nsec;
			if (gap >= DELAY_TIME_NSECS)
			{	//发送定时完成消息
				msgsnd(msgid, &msg, MSGSIZE, 0);
				start.tv_nsec += DELAY_TIME_NSECS;
				if (start.tv_nsec >= ONE_SECOND_NSECS)
				{
					start.tv_nsec -= ONE_SECOND_NSECS;
					start.tv_sec += 1;
				}
			}
		}
	}
	pthread_exit(NULL);
}

/*****************************************************************************
 函 数 名  : Light
 功能描述  : 点灯模块的线程处理函数，主要接收点灯消息，然后消息内容进行点灯
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *Light(void *arg)
{
	struct msgbuf msg;
	unsigned short *group = (unsigned short *)(msg.mtext);
	int i;
	while (1)
	{
		memset(group, 0, MSGSIZE);
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_LIGHT, 0) == -1)
		{
			usleep(10);
			continue;
		}
		for (i = 0; i < NUM_BOARD; i++)
		{
			PhaseLampOutput(i + 1, group[i]);//call the old interface
		}
	}
}
 
/*****************************************************************************
 函 数 名  : ThreadHardYellowLight
 功能描述  : 黄闪控制模块的线程处理函数
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *ThreadHardYellowLight(void *arg)
{

    while(1)
    {
        usleep(300000);
        if(((FAULT_RED_GREEN_CONFLICT == s_failure_info.nID) || (FAULT_GREEN_CONFLICT == s_failure_info.nID)) && (s_special_params.iVoltageAlarmAndProcessSwitch == 1))
        {
            log_debug("%s   Id  %d\n",__func__,s_failure_info.nID);
            continue;
        }

        if((FAULT_RED_OFF == s_failure_info.nID) && (s_special_params.iCurrentAlarmAndProcessSwitch == 1))
        {
            log_debug("%s   Id  %d\n",__func__,s_failure_info.nID);
            continue;
        }

        HardflashDogOutput();

    }

    return NULL;
}

/*****************************************************************************
 函 数 名  : createThread
 功能描述  : 线程创建需要调用的函数，主要用来创建一个线程
 输入参数  : pthread_t *id            线程号的指针，创建成功后线程号会通过指针返回
             void *(*fun)(void *arg)  线程的处理函数指针
             char *moduleName         线程的模块名称
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline void createThread(pthread_t *id,
                                void *(*fun)(void *arg),
                                char *moduleName)
{
	if (pthread_create(id, NULL, fun, NULL) == -1) 
	{
		log_error("create %s module fail, error info:%s\n", moduleName, strerror(errno));
		return;
	}
	pthread_detach(*id);
	log_debug("create %s pthread successful!\n", moduleName);
}

/*****************************************************************************
 函 数 名  : itstaskmain
 功能描述  : 各模块的初始化函数入口，主要用来初始化各个模块的线程
 输入参数  : int argc      
             char *argv[]  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int itstaskmain(int argc, char *argv[])
{
    InitGolobalVar();

	msgid = getMsgId('a');
	if (msgid == -1) 
	{
		perror("get msgid fail, error info:");
		return -1;
	}
	log_debug("msgid = %d\n", msgid);

	pthread_t communication, timer, signalRun;
	pthread_t light;
	pthread_t t_database;
    pthread_t t_hard_yellow;

	//启动通信模块
	createThread(&communication, CommunicationModule, "communication");
	//启动定时器模块
	createThread(&timer, TimerModule, "timer");
	//启动信号机控制模块
	createThread(&signalRun, SignalControllerRun, "signalRun");
	//启动点灯模块
	createThread(&light, Light, "light");

    createThread(&t_database,ThreadCheckCfgChanged,"ThreadCheckCfgChanged");

	createThread(&t_hard_yellow, ThreadHardYellowLight,"ThreadHardYellowLight");

	while (1) 
	{
		if (pthread_kill(communication, 0) == ESRCH) 
		{
			createThread(&communication, CommunicationModule, "communication");
		}
		if (pthread_kill(timer, 0) == ESRCH) 
		{
			createThread(&timer, TimerModule, "timer");
		}
		if (pthread_kill(signalRun, 0) == ESRCH) 
		{
			createThread(&signalRun, SignalControllerRun, "signalRun");
		}
		if (pthread_kill(light, 0) == ESRCH) 
		{
			createThread(&light, Light, "light");
		}
		if (pthread_kill(t_database, 0) == ESRCH) 
		{
			createThread(&t_database, ThreadCheckCfgChanged, NULL);
		}		
		if (pthread_kill(t_hard_yellow, 0) == ESRCH) 
		{
			createThread(&t_hard_yellow, ThreadHardYellowLight, NULL);
		}	
		//清除系统缓存
        system("sync");
        system("echo 1 > /proc/sys/vm/drop_caches");
        system("echo \"0\" >> /proc/sys/vm/overcommit_memory");
		sleep(1);
	}

    return 0;
}
