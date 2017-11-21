
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>

#include "its.h"
#include "lfq.h"
#include "hikmsg.h"
#include "LogSystem.h"
#include "gb.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define PTHREAD_STACK_SIZE	(2 << 20)	//2M
#define LINE_QUEUE_SIZE		(128 * 1024)	//128k

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
 
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void *FaultLogModule(void *arg);
extern void *RedSignalCheckModule(void *arg);
extern void *PhaseDriverModule(void *arg);
extern void *CalculateModule(void *arg);
extern void *PhaseControlModule(void *arg);
extern void *StrategyControlModule(void *arg);
extern void *TimerModule(void *arg);

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
//消息队列id
int msgid = -1;
//信号机配置参数主结构全局变量指针，用来存放配置信息
SignalControllerPara *gRunConfigPara = NULL;//信号机运行的配置参数
GbConfig *gRunGbconfig = NULL;					//全局国标配置指针
//读写全局配置信息的锁
pthread_rwlock_t gConfigLock = PTHREAD_RWLOCK_INITIALIZER;
//线性队列句柄
void *gHandle = NULL;
//定时模块发送给相位控制和相位驱动模块的无名信号量
sem_t gSemForCtl;	//用来给相位控制模块发送定时1s的信号
sem_t gSemForDrv;	//用来给相位驱动模块发送定时250ms的信号

extern int gComPort;//各个信号机的通讯端口号，消息队列ID与此值相同。

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
static int GetMsgId(int proj_id)
{
	key_t key = ftok("/dev/null", proj_id);
	int id = -1;
	
	if (key == -1) 
	{
		return -1;
	}
	while (id == -1)
	{
		id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
		if (id == -1)
		{	
			if (errno == EEXIST)
			{	//若是已经存在此消息队列，则先删除此队列再创建以此来清空消息队列	
				id = msgget(key, 0666);
				INFO("excute msgctl to delete msg queue before, ret = %d", msgctl(id, IPC_RMID, 0));
				id = -1;
			}
			else
				return -1;
		}
	}
	return id;
}

/*****************************************************************************
 函 数 名  : InitGolobalVar
 功能描述  : 初始化全局参数：日志参数、配置参数
 输入参数  : 无
 返 回 值  : 

 修改历史      :
  1.日    期   : 2015年11月26日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void InitGolobalVar(ProtocolType type, void *config)
{
	InitLogSystem("/home/Log/",1,1);
	msgid = GetMsgId(gComPort);
	if (msgid == -1) 
	{
		log_error("get msgid fail, error info: %s\n", strerror(errno));
		exit(1);
	}
	if (sem_init(&gSemForCtl, 0, 0) == -1
		|| sem_init(&gSemForDrv, 0, 0) == -1)
	{
		log_error("int sem fail, error info: %s\n", strerror(errno));
		exit(1);
	}
	
    //初始化内存用于全局的信号机配置参数
	switch (type)
	{
		case NTCIP:
			gRunConfigPara = (PSignalControllerPara)calloc(1, sizeof(SignalControllerPara));
			if(!gRunConfigPara)
			{
				log_error("error to alloc memory for gRunConfigPara  : %s\n",strerror(errno));
				exit(1);
			}
			memcpy(gRunConfigPara, config, sizeof(SignalControllerPara));
			break;
		case GB2007:
			gRunGbconfig = (GbConfig *)calloc(1, sizeof(GbConfig));
			if(!gRunGbconfig)
			{
				log_error("error to alloc memory for gRunGbconfig  : %s\n",strerror(errno));
				exit(1);
			}
			memcpy(gRunGbconfig, config, sizeof(GbConfig));
		default:
			log_error("the protocoltype[%d] is wrong", type);
			exit(1);
	}
    
	//为线性队列申请内存，并初始化线性队列
	gHandle = calloc(1, LINE_QUEUE_SIZE);
	if (NULL == gHandle)
	{
		log_error("init line queue fail because of no enough memory");
		exit(1);
	}
	lfq_init(gHandle, LINE_QUEUE_SIZE, sizeof(LineQueueData));
	INFO("init libits global variable successful! msgid = %d\n", msgid);
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
	UInt32 *countvalue = (UInt32 *)arg;
	struct msgbuf msg;
	struct timespec start = {0, 0}, end = {0, 0};
	long gap, times = 0;

	msgrcv(msgid, &msg, MSGSIZE, MSG_START_TIMER, 0);	//阻塞等待相位控制模块发送的启动定时器消息
	INFO("Timer begin to run!");
	//先发送一次定时信号给相位控制模块
	//sem_post(&gSemForCtl);
	clock_gettime(CLOCK_MONOTONIC, &start);	//记录下起始时间
	while (1)	//每DELAY_TIME ms循环一次
	{
		usleep(DELAY_TIME_USECS_BEFORE);	//先延时DELAY_TIME_USECS_BEFORE us,在最后的1ms内精确延时
		while (1)
		{
			usleep(10000);	//每次延时10ms
			clock_gettime(CLOCK_MONOTONIC, &end);
			gap = (end.tv_sec - start.tv_sec) * ONE_SECOND_NSECS + end.tv_nsec - start.tv_nsec;
			if (gap >= DELAY_TIME_NSECS)
			{	      
				times += 1;
				//if ( % LOOP_TIMES_PER_SECOND == 0)
				//{	//给相位控制模块发送定时信号,每1s一次
					//sem_post(&gSemForCtl);
					//times = 0;
				//}
				
				//if(times >= 4){
					sem_post(&gSemForCtl);
				//}else{
				//	sem_post(&gSemForDrv);
				//}
				
                //给相位驱动模块发送定时信号,每250ms一次
				//sem_post(&gSemForDrv);
				start.tv_nsec += DELAY_TIME_NSECS;
				if (start.tv_nsec >= ONE_SECOND_NSECS)
				{
					start.tv_nsec -= ONE_SECOND_NSECS;
					start.tv_sec += 1;
				}
				(*countvalue)++;
				break;
			}
		}
	}
	pthread_exit(NULL);
}

/*****************************************************************************
 函 数 名  : CreateThread
 功能描述  : 线程创建需要调用的函数，主要用来创建一个线程
 输入参数  : struct threadInfo *info	存放线程有关信息的指针
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline void CreateThread(struct threadInfo *info)
{
	if (info->func == NULL)
		return;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE);
	if (pthread_create(&info->id, &attr, info->func, (void *)&info->countvalue) == -1) 
	{
		log_error("create %s module fail, error info:%s\n", info->moduleName, strerror(errno));
		exit(1);
	}
	pthread_detach(info->id);
	pthread_attr_destroy(&attr);
	INFO("create %s pthread successful!\n", info->moduleName);
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
#define MAX_THREAD_NUM	16
static struct threadInfo gThreads[MAX_THREAD_NUM] = {
	{0, FaultLogModule, "fault log", 0},
	{0, TimerModule, "timer", 0},
	{0, RedSignalCheckModule, "red light signal check", 0},
	{0, PhaseDriverModule, "phase driver", 0},
	{0, PhaseControlModule, "phase control", 0},
	{0, StrategyControlModule, "strategy control", 0},
	{0, CalculateModule, "calculate", 0},
};
static int gThreadNum = 7;	//线程个数需要与上面的个数一一对应
Boolean ItsInit(struct threadInfo *threads, int num, ProtocolType type, void *config)
{
	int i;

	if (num + gThreadNum > MAX_THREAD_NUM)
	{
		ERR("regsiter thread num[%d] exceed the limit[%d]", num, MAX_THREAD_NUM - gThreadNum);
		return FALSE;
	}
	if (threads == NULL || config == NULL)
	{
		ERR("the argument threads or config is NULL");
		return FALSE;
	}
	fclose(stdout);
	InitGolobalVar(type, config);

	if (threads != NULL && num > 0)
	{
		memcpy(&gThreads[gThreadNum], threads, sizeof(struct threadInfo) * num);
		gThreadNum += num;
	}
	for (i = 0; i < gThreadNum; i++)
		CreateThread(gThreads + i);
	log_debug("all threads init successful!");
	return TRUE;
}

void ItsExit(void)
{
	int i;
	for (i = 0; i < gThreadNum; i++)
	{
		if (gThreads[i].id != 0)
			pthread_cancel(gThreads[i].id);
	}
	if (msgid != -1)
	{
		msgctl(msgid, IPC_RMID, NULL);
		msgid = -1;
	}
	sem_destroy(&gSemForCtl);
	sem_destroy(&gSemForDrv);
	if (gRunConfigPara != NULL)
	{
		free(gRunConfigPara);
		gRunConfigPara = NULL;
	}
	if (gHandle != NULL)
	{
		free(gHandle);
		gHandle = NULL;
	}
	INFO("The Its resource is free!");
}

#define IS_CORE_MODULE(F) ((F) == TimerModule \
				|| (F) == PhaseDriverModule \
				|| (F) == PhaseControlModule \
				|| (F) == StrategyControlModule \
				|| (F) == CalculateModule)
void ItsThreadCheck(void)
{
	static UInt32 countvalues[MAX_THREAD_NUM] = {0};
	struct msqid_ds msgstat;
	int i, semvalForDrv = -1, semvalForCtl = -1;

	memset(&msgstat, 0, sizeof(msgstat));
	for (i = 0; i < gThreadNum; i++)
	{
		if (pthread_kill(gThreads[i].id, 0) == ESRCH) 
		{
			ItsWriteFaultLog(THREAD_EXCEPTION_EXIT, i);
			if (IS_CORE_MODULE(gThreads[i].func))
			{	//如果是几个核心模块挂掉则自动重启系统
				log_error("%s module has already stop, reboot system automatically!", gThreads[i].moduleName);
				sync();
				system("reboot");
			}
			else
			{
				log_error("%s module has already stop, recreate it!", gThreads[i].moduleName);
				CreateThread(gThreads + i);
			}
		}
		if (IS_CORE_MODULE(gThreads[i].func))
		{
			if (countvalues[i] != gThreads[i].countvalue)
				countvalues[i] = gThreads[i].countvalue;
			else
			{
				if (gThreads[i].countvalue == 0 && countvalues[i] == 0)
					continue;	//说明模块没有使用计数值,那么便不做任何处理
				/*如果模块的计数值有使用到且模块的计数值与此函数的计数值相等则说明模块被阻塞了,
				 * 目前只有定时模块、相位控制模块、相位驱动模块用到了计数值countvalue,
				 * 并且这三个模块阻塞时间不会超过1s,我在主函数main里面每隔3s检测一次二者计数,
				 * 若相等则说明模块已经出现故障了,默认情况重启系统避免一直黄闪*/
				msgctl(msgid, IPC_STAT, &msgstat);
				sem_getvalue(&gSemForDrv, &semvalForDrv);
				sem_getvalue(&gSemForCtl, &semvalForCtl);
				log_error("checked %s module running unnormally, reboot system automatically!\n"\
					"msgnum:%lu, semvalForCtl:%d, semvalForDrv:%d", 
					gThreads[i].moduleName, msgstat.msg_qnum, semvalForCtl, semvalForDrv);
				sync();
				system("reboot");
			}
		}
	}
}

void ItsSetConfig(ProtocolType type, void *config)
{
	char *start = NULL;
	size_t size = 0;

	if (config == NULL)
		return;
	switch (type)
	{
		case NTCIP:
			start = (char *)gRunConfigPara;
			size = sizeof(SignalControllerPara);
			break;
		case GB2007:
			start = (char *)gRunGbconfig;
			size = sizeof(GbConfig);
			break;
		default: return;
	}
	if (start == NULL)
		return;
	pthread_rwlock_wrlock(&gConfigLock);
	memcpy(start, config, size);
	pthread_rwlock_unlock(&gConfigLock);
}

void ItsGetConfig(ProtocolType type, void *config)
{
	char *start = NULL;
	size_t size = 0;

	if (config == NULL)
		return;
	switch (type)
	{
		case NTCIP:
			start = (char *)gRunConfigPara;
			size = sizeof(SignalControllerPara);
			break;
		case GB2007:
			start = (char *)gRunGbconfig;
			size = sizeof(GbConfig);
			break;
		default: return;
	}
	if (start == NULL)
		return;
	pthread_rwlock_rdlock(&gConfigLock);
	memcpy(config, start, size);
	pthread_rwlock_unlock(&gConfigLock);
}

void ItsCtl(ControlType type, UInt8 schemeId, UInt8 stageNum)
{
    struct msgbuf msg;
	int ret;

    memset(&msg, 0, sizeof(msg));
    msg.mtype = MSG_CONTROL_TYPE;
    msg.msgControlType = type;
	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: msg.msgMode = YELLOWBLINK_MODE; break;
		case ALLRED_SCHEMEID: msg.msgMode = ALLRED_MODE; break;
		case TURNOFF_SCHEMEID: msg.msgMode = TURNOFF_LIGHTS_MODE; break;
		case INDUCTIVE_SCHEMEID: msg.msgMode = INDUCTIVE_MODE; break;
		case STEP_SCHEMEID: msg.msgMode = STEP_MODE; msg.msgStageNum = stageNum; break;
		case 0: msg.msgMode = SYSTEM_MODE; break;
		default: msg.msgMode = MANUAL_MODE; msg.msgmSchemeId = schemeId; break;
	}
	if (msgid == -1)
	{
		msgid = msgget(ftok("/dev/null", 'a'), 0666);
		INFO("create msgid");
	}
	INFO("recv ITS control, schemeId: %d, stageNum: %d", schemeId, stageNum);
    ret = msgsnd(msgid, &msg, MSGSIZE, 0);
	if (ret == -1)
		ERR("ItsCtl msgsnd error: %s, msgid = %d", strerror(errno), msgid);
}

void ItsChannelCheck(UInt8 channelId, LightStatus status)
{
	struct msgbuf msg;
	int i = 0;
	
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_CHANNEL_CHECK;
	msg.msgChannelId = channelId;
	msg.msgChannelStatus = status;
	for (i = 0; i < 6; i++)	//通道检测时，持续只点亮这个通道6s
		msgsnd(msgid, &msg, MSGSIZE, 0);
}

