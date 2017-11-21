#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "platform.h"
#include "HikConfig.h"

#define MAX_STEP_NUM	16
#define STEP_MSG_TYPE	100
#define MSGSIZE (sizeof(struct msgbuf) - sizeof(long))

typedef enum
{
	STEP_START = 0,	//步进开始
	STEP_EXCUTE,	//步进执行
	STEP_END,		//步进结束
} StepCmd;
struct msgbuf
{
	long type;
	StepCmd cmd;
	UInt8 stageNo;
};
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define LOFF		0
#define LGREEN		1
#define LYELLOW		4
#define LRED		2
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef struct stepParameterInfo
{
	UInt8 cycleTime;	//当前周期时间
	UInt8 runTime;		//已经运行的时间
	UInt8 maxStageNo;	//最大阶段号
	UInt8 curStageNo;	//当前运行的阶段号
	UInt8 nextStageNo;	//下一个即将运行的阶段号
	UInt8 stageNeedRunTimes[MAX_STEP_NUM];	//定周期时到达每个阶段需要运行时间
	UInt8 phaseSplit[NUM_PHASE];	//相位的绿信比
	UInt8 phaseStatus[NUM_PHASE];	//相位状态
} StepParams;

static StepParams gStepParams;

static int msgid = -1;
UInt8 gStepFlag = 0;	//步进标志，0：表示未步进，1：表示步进
UInt16 gLightArray[8] = {0};	//步进时使用的点灯数组
pthread_rwlock_t gLightArrayLock = PTHREAD_RWLOCK_INITIALIZER;	//步进点灯数组的锁

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;
extern pthread_rwlock_t gCountDownLock;
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;
extern PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //当前使用的相序

extern void udp_send_change_ctrl();

/*****************************************************************************
 函 数 名  : Light
 功能描述  : 根据所有通道的状态来设置点灯的数组
 输入参数  : UInt8 *allChannel  		描述所有通道状态的数组指针
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void SetLightArray(UInt8 *allChannel)
{
	int i;
	UInt16 tmp, value;
	UInt16 *nOutLamp = gLightArray;
	
	pthread_rwlock_wrlock(&gLightArrayLock);
	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		//根据通道所处状态找出这个通道应该所赋的值
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK:
						tmp = get_lamp_value(nOutLamp, i % 4);
						value = (tmp == LOFF) ? LGREEN : LOFF;
						break;	
			case YELLOW:	value = LYELLOW; break;
			case YELLOW_BLINK:
							tmp = get_lamp_value(nOutLamp, i % 4);
							value = (tmp == LOFF) ? LYELLOW: LOFF;
							break;
			case RED:	value = LRED; break;
			case ALLRED: value = LRED; break;
			default: value = LOFF; break;
		}
		//给这个通道赋值
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			nOutLamp++;
		}
	}
	pthread_rwlock_unlock(&gLightArrayLock);
}
//根据相位状态设置通道状态,随后设置点灯数组
static void SetChannel(UInt8 *phaseStatus)
{    
    int i = 0, j = 0;
    UInt8 channelId = 0;
	PhaseChannelStatus status;
	UInt8 phaseId, motherPhase;
	UInt8 allChannel[NUM_CHANNEL];

	pthread_rwlock_rdlock(&gConfigLock);
    for(i = 0; i < NUM_CHANNEL; i++)
    {
    	channelId = gSignalControlpara->stChannel[i].nChannelID;
		phaseId = gSignalControlpara->stChannel[i].nControllerID;
		if (channelId == 0 || phaseId == 0)
		{
			allChannel[i] = TURN_OFF;
			continue;
		}	
		
		status = RED;
		switch (gSignalControlpara->stChannel[i].nControllerType) 
		{
			case MOTOR: status = phaseStatus[phaseId - 1]; break;
			case PEDESTRIAN: status = phaseStatus[phaseId - 1]; break;
			case FOLLOW: 
				for (j = 0; j < NUM_PHASE; j++)
				{
					motherPhase = gSignalControlpara->stFollowPhase[phaseId - 1].nArrayMotherPhase[j];
					if (motherPhase == 0) 
						break;
					if (phaseStatus[motherPhase - 1] == GREEN)
					{
						status = GREEN;
						break;
					}
				}
				break;
			default: status = TURN_OFF; break;
		}
		allChannel[i] = (UInt8)status;
    }
	pthread_rwlock_unlock(&gConfigLock);
	SetLightArray(allChannel);
}

//通过到达指定阶段需要运行的时间来设置指定阶段的相位状态
static void SetPhaseStatus(UInt8 stageNo, StepParams *p)
{
	int i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt8 phaseEndRunTime = 0;	//相位执行完已经运行的周期时间
	UInt8 stageNeedRunTime = p->stageNeedRunTimes[stageNo - 1];
	
	p->curStageNo = stageNo;
	p->nextStageNo = (stageNo + 1 > p->maxStageNo) ? 1 : (stageNo + 1);
	memset(p->phaseStatus, 0, NUM_PHASE);
	for (ring = 0; ring < NUM_RING_COUNT; ring++) 
	{	
		phaseEndRunTime = 0;
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gPhaseTrunTable[ring].nTurnArray[i];
			if (nPhaseId == 0) 
			{
				break;
			}
			phaseEndRunTime += p->phaseSplit[nPhaseId - 1];
			if (phaseEndRunTime > stageNeedRunTime)
			{
				p->phaseStatus[nPhaseId - 1] = (UInt8)GREEN;
				break;
			}
		}
	}
}

//设置定周期时到达每个阶段需要运行的时间，并根据当前运行时间和周期找出下一个运行的阶段号
static void SetStepParams(StepParams *p)
{
	int t = 0, i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt8 phaseEndRunTime = 0;	//相位执行完已经运行的周期时间
	UInt8 stageNo = 1;	//阶段号
	UInt8 nStageNeedRunTime = 0;	//到达指定阶段需要运行的时间
	UInt8 curStageNo = 1;
	
	memset(p->stageNeedRunTimes, 0, MAX_STEP_NUM);
	for (t = 0; t < p->cycleTime; t++) 
	{	
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	
			phaseEndRunTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gPhaseTrunTable[ring].nTurnArray[i];
				if (nPhaseId == 0) 
				{
					break;
				}
				phaseEndRunTime += p->phaseSplit[nPhaseId - 1];
				if (t == phaseEndRunTime && phaseEndRunTime > nStageNeedRunTime)
				{
					stageNo++;
					nStageNeedRunTime = phaseEndRunTime;
					p->stageNeedRunTimes[stageNo - 1] = nStageNeedRunTime;
				}
			}
		}
		if (t + 1 == p->runTime)
			curStageNo = stageNo;
	}
	p->curStageNo = curStageNo;
	p->nextStageNo = (curStageNo + 1 > stageNo) ? 1 : (curStageNo + 1);
	p->maxStageNo = stageNo;
}
//步进参数初始化
static void StepParamsInit(StepParams *p)
{
	int i;
	memset(p, 0, sizeof(StepParams));
	pthread_rwlock_rdlock(&gCountDownLock);
	p->cycleTime = gCountDownParams->ucCurCycleTime;
	p->runTime = gCountDownParams->ucCurRunningTime;
	for (i = 0; i < NUM_PHASE; i++)
	{
		p->phaseSplit[i] = gCountDownParams->stPhaseRunningInfo[i][0];
		p->phaseStatus[i] = gCountDownParams->stVehPhaseCountingDown[i][0];
	}
	pthread_rwlock_unlock(&gCountDownLock);
	SetStepParams(p);
}
//单步步进要经过绿闪、黄灯、全红的过渡期
static void OneStepTransition(StepParams *p)
{
	int i, n = 0;
	UInt8 curPhaseStatus[NUM_PHASE];	//用以保存当前阶段的相位状态
	UInt8 phaseIndexs[NUM_PHASE] = {0};	//运行到下一阶段时相位状态会改变的相位号索引
	PhaseItem *phaseItem = NULL;
	UInt8 greenBlinkTime, yellowTime, allRedTime;
	
	memcpy(curPhaseStatus, p->phaseStatus, sizeof(p->phaseStatus));
	SetPhaseStatus(p->nextStageNo, p);
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (curPhaseStatus[i] != p->phaseStatus[i])
			phaseIndexs[n++] = i;
	}
	pthread_rwlock_rdlock(&gConfigLock);
	phaseItem = &gSignalControlpara->stPhase[phaseIndexs[0]];
	greenBlinkTime = gSignalControlpara->AscSignalTransTable[phaseIndexs[0]].nGreenLightTime;
	yellowTime = phaseItem->nYellowTime;
	allRedTime = phaseItem->nAllRedTime;
	pthread_rwlock_unlock(&gConfigLock);
	//对于相位状态在下一阶段要改变的相位依次经过绿闪、黄灯、全红的过渡
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = GREEN_BLINK;	//把需要改变的相位状态设置为绿闪
	for (i = 0; i < greenBlinkTime * 4; i++)
	{	//因为要绿灯闪烁，所以每隔250ms设置一次通道
		SetChannel(curPhaseStatus);
		usleep(250000);
	}
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = YELLOW;	//把需要改变的相位状态设置为黄灯
	SetChannel(curPhaseStatus);
	sleep(yellowTime);	//用延时达到黄灯亮的时间
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = ALLRED;	//把需要改变的相位状态设置为全红
	SetChannel(curPhaseStatus);
	sleep(allRedTime);	//用延时达到全红亮的时间
	SetChannel(p->phaseStatus);	//过渡完成后设置下一阶段的通道
}
//跳转到指定阶段
static void udp_send_step(int iStep)
{
	//创建UDP服务器
	int socketFd = -1;
	//数组最后一个值为阶段号
	char JumpToiStep[46]={0x30,0x2C,0x02,0x01,0x00,0x04,0x06,0x70,
						0x75,0x62,0x6C,0x69,0x63,0xA3,0x1F,0x02,
						0x01,0x0D,0x02,0x01,0x00,0x02,0x01,0x00,
						0x30,0x14,0x30,0x12,0x06,0x0D,0x2B,0x06,
						0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
						0x50,0x02,0x00,0x02,0x01,0x00};
	struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	socklen_t localLen = sizeof(localAddr);
	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( -1 == socketFd )
	{
		printf("socket udp init error!!!\n");
		return;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//使用161端口
	localAddr.sin_port = htons (161);

	// 1.向本机161端口发送步进
	//阶段号赋值
	JumpToiStep[45] = iStep;

	printf("跳转步进iStep=%d\n",iStep);
	int len2 = sendto(socketFd,JumpToiStep,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len2 < 0)
	{
		printf("Send flashing signal failed!!!\n");
	}
	close(socketFd);
}
//取消步进
static void udp_send_cancel_jump()
{
	//创建UDP服务器
	int socketFd = -1;
	//取消步进
	unsigned char cancelJumpStep[46]={0x30,0x2C,0x02,0x01,0x00,0x04,0x06,0x70,
							0x75,0x62,0x6C,0x69,0x63,0xA3,0x1F,0x02,
							0x01,0x0C,0x02,0x01,0x00,0x02,0x01,0x00,
							0x30,0x14,0x30,0x12,0x06,0x0D,0x2B,0x06,
							0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
							0x50,0x05,0x00,0x02,0x01,0x00};
    struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = sizeof(localAddr);
    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	return;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//使用161端口
    localAddr.sin_port = htons (161);
	//向本机161端口发送取消步进
	printf("取消步进\n");
	int len = sendto(socketFd,cancelJumpStep,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		printf("Send flashing signal failed!!!\n");
	}
	close(socketFd);
	
}

//步进线程处理函数
static void *StepProcess(void *arg)
{
	struct msgbuf msg;
	StepParams *p = &gStepParams;

	while (1)
	{
		msgrcv(msgid, &msg, MSGSIZE, STEP_MSG_TYPE, 0);
		if (msg.cmd == STEP_START)
		{
			SetChannel(p->phaseStatus);
			gStepFlag = 1;
		}
		else if (msg.cmd == STEP_EXCUTE)
		{
			if (msg.stageNo == 0)
			{	//单步步进
				OneStepTransition(p);
			}
			else
			{	
				SetPhaseStatus(msg.stageNo, p);
				SetChannel(p->phaseStatus);
				gStepFlag = 1;
			}
		}
		else if (msg.cmd == STEP_END)
		{
			udp_send_change_ctrl();
			usleep(50000);
			udp_send_step(p->curStageNo);
			usleep(50000);
			udp_send_cancel_jump();
			gStepFlag = 0;
		}
	}
	pthread_exit(NULL);
}

void StepPthreadInit(void)
{
	key_t key = ftok("/dev/null", 'a');
	pthread_t id;
	
	if (key == -1) 
	{
		ERR("ftok fail when create message queue");
		exit(1);
	}
	while (msgid == -1)
	{
		msgid = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
		if (msgid == -1)
		{	
			if (errno == EEXIST)
			{	//若是已经存在此消息队列，则先删除此队列再创建以此来清空消息队列	
				msgid = msgget(key, 0666);
				msgctl(msgid, IPC_RMID, 0);
				msgid = -1;
			}
			else
			{
				ERR("create message queue fail!");
				exit(1);
			}
		}
	}
	INFO("create message queue successful! msgid = %d", msgid);
	
	if (pthread_create(&id, NULL, StepProcess, NULL) != 0)
	{
		ERR("step pthread create fail");
		exit(1);
	}
	pthread_detach(id);
	INFO("create step pthread successful!");
}

//检查步进是否有效，在有绿闪、黄灯、全红时是不能进行步进的
Boolean IsStepValid(UInt8 stageNo, UInt8 flag)
{
	UInt8 status;
	Boolean ret = TRUE;
	struct msgbuf msg;
	int i;
	
	pthread_rwlock_rdlock(&gCountDownLock);
	for (i = 0; i < NUM_PHASE; i++)
	{
		status = gCountDownParams->stVehPhaseCountingDown[i][0];
		if (status == GREEN_BLINK || status == YELLOW 
			|| (status == RED && gCountDownParams->stPhaseRunningInfo[i][1] != 0))
		{
			ret = FALSE;
			break;
		}	
	}
	pthread_rwlock_unlock(&gCountDownLock);
	if (ret == TRUE)
	{
		msg.type = STEP_MSG_TYPE;
		msg.stageNo = stageNo;
		if (flag == 0)
			StepParamsInit(&gStepParams);
		if (stageNo == 0)
		{	//单步步进
			msg.cmd = (flag == 0) ? STEP_START : STEP_EXCUTE;
		}
		else
		{	//直接步进到相应阶段		
			if (stageNo > gStepParams.maxStageNo)
			{
				ERR("No such stage number %d", stageNo);
				ret = FALSE;
			}
			else
				msg.cmd = STEP_EXCUTE;
		}
		if (ret == TRUE)	
			msgsnd(msgid, &msg, MSGSIZE, 0);
	}
	return ret;
}
//取消步进
void StepCancel(void)
{
	struct msgbuf msg = {STEP_MSG_TYPE, STEP_END, 0};
	
	msgsnd(msgid, &msg, MSGSIZE, 0);
	//memset(gStepParams, 0, sizeof(StepParams));
}
