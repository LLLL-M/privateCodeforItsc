#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include "hikmsg.h"
#include "lfq.h"
#include "LogSystem.h"

typedef enum
{
	STEP_UNUSED_FLAG = 100,       //步进未使用标志
	STEP_TRANSITION_FLAG = 111,   //步进过渡标志
	STEP_EXCUTE_FLAG = 222,		//步进执行标志
}STEP_FLAG;

typedef struct controlInfos
{
	ControlMode mode;	//当前控制方式
	UInt8 flag;	//步进控制时使用
	UInt16 extendTime;	//感应控制时使用
	UInt16 cycleLeftTime;	//感应协调控制时剩余的周期时间
	UInt8 totalExtendTime[MAX_PHASE_NUM];	//存放感应控制时相位已经延长的时间和
	UInt8 stageNum;	//步进阶段号
	UInt8 schemeId;	//控制方案号
	LineQueueData cycleLeftData;	//感应协调控制时存放周期剩余时间的临时数据
} ControlInfos;

//extern SignalControllerPara *gRunConfigPara;

extern int gOftenPrintFlag; //打印标志
extern void *gHandle; //线性队列句柄
extern int msgid;
extern sem_t gSemForCtl;
extern sem_t gSemForVeh;
extern sem_t gSemForChan;	//用来给通道控制模块发送定时1s的信号

static volatile time_t gCurTime;		//当前时间
static CustomParams gCustomParams = {
	.isContinueRun = FALSE,	//默认黄闪等特殊控制之后不接着之前的周期继续运行
	.addTimeToFirstPhase = FALSE,	//默认感应协调控制时把剩余时间加到最后一个相位
};
static inline void ReadLineQueueData(LineQueueData *data, UInt8 schemeId);
SpecialCarControl gSpecialCarControl;

#include "step.h"
#include "inductive.h"

void ItsIsContinueRun(Boolean val)
{
	gCustomParams.isContinueRun = val;
}


//设置黄闪、全红、关灯时的数据信息
static inline void SetSpericalData(LineQueueData *data, LightStatus status)
{
	int i;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 channelFlashStatus[MAX_CHANNEL_NUM] = {0};	//通道闪光状态

	if (gCustomParams.isContinueRun == FALSE)
		lfq_reinit(gHandle);	//清除线性队列中的数据块，不再接着之前的方案继续运行
    //data->cycleTime = 0;
    //data->leftTime = 0;
    data->stageNum = 0;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
	    if (phaseInfos[i].followPhaseStatus != INVALID)
	    {
            phaseInfos[i].followPhaseStatus = status;
	        phaseInfos[i].followPhaseLeftTime = 0;
	    }
	    if (phaseInfos[i].phaseStatus == INVALID)
	        continue;
	    phaseInfos[i].phaseStatus = status;    
	    phaseInfos[i].phaseLeftTime = 0;
	    phaseInfos[i].phaseSplitLeftTime = 0;
	    phaseInfos[i].splitTime = 0;
	    if (phaseInfos[i].pedestrianPhaseStatus != INVALID)
	    {
	        phaseInfos[i].pedestrianPhaseStatus = status;
	        phaseInfos[i].pedestrianPhaseLeftTime = 0;
	    }
	    
	}
	
	if (status == YELLOW_BLINK)
	{
//		pthread_rwlock_rdlock(&gConfigLock);
		for (i = 0; i < MAX_CHANNEL_NUM; i++)
		{
#if 0
			if (gRunConfigPara->stChannel[i].nFlashLightType == 0x04)	//红闪
				channelFlashStatus[i] = RED_BLINK;
			else if (gRunConfigPara->stChannel[i].nFlashLightType == 0)	//无闪
				channelFlashStatus[i] = TURN_OFF;
			else
#endif
				channelFlashStatus[i] = YELLOW_BLINK;
		}
//		pthread_rwlock_unlock(&gConfigLock);
	}
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (data->allChannels[i] != INVALID)
		{
			data->allChannels[i] = status;
			if (status == YELLOW_BLINK)
			{
				data->allChannels[i] = channelFlashStatus[i];
			}
		}
	}
}

/*
void ItsCustom(LineQueueData *data, CustomParams *customParams)
{	//此函数专门用于优控平台，其他情况都需要在库外另外实现此函数
	time_t timeNow, timeStart, timeEnd;    
    struct tm start, end;
	
	if (gChannelLockParams.ucChannelLockStatus == 1 && gChannelLockParams.ucWorkingTimeFlag == 1)
	{	//通道按时间锁定
		timeNow = time(NULL);
		localtime_r(&timeNow, &start);
		memcpy(&end, &start, sizeof(end));
		start.tm_hour = gChannelLockParams.ucBeginTimeHour;
		start.tm_min = gChannelLockParams.ucBeginTimeMin;
		start.tm_sec = gChannelLockParams.ucBeginTimeSec;
		timeStart = mktime(&start);
		end.tm_hour = gChannelLockParams.ucEndTimeHour;
		end.tm_min = gChannelLockParams.ucEndTimeMin;
		end.tm_sec = gChannelLockParams.ucEndTimeSec;
		timeEnd = mktime(&end);
		if (((timeStart > timeEnd) && (timeNow < timeStart && timeNow > timeEnd))
			|| ((timeStart < timeEnd) && (timeNow < timeStart || timeNow > timeEnd)))
		{	//当前时间不在锁定时间范围内应该进行解锁
			//ItsChannelUnlock();
			gChannelLockParams.ucWorkingTimeFlag = 0;
		}
	}
}*/

void ItsSetCurRunData(LineQueueData *data)
{

}

static void SendCalculateNextCycleMsg(LineQueueData* data, UINT8 schemeId)
{
	struct msgbuf msg;
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_START_CALCULATE_NEXT_CYCLE;
    msg.msgSchemeId = schemeId;
	msg.msgCalTime = gCurTime + data->leftTime;
    msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);	
}

//读取线性队列中的1s数据信息
static inline void ReadLineQueueData(LineQueueData *data, UInt8 schemeId)
{
    struct msgbuf msg;
	int left = lfq_element_count(gHandle);
	LineQueueData *block;
    int i;
	
	if (left <= AHEAD_OF_TIME)
	{
	    memset(&msg, 0, sizeof(msg));
	    msg.mtype = MSG_START_CALCULATE_NEXT_CYCLE;
    	msg.msgSchemeId = schemeId;
		msg.msgCalTime = gCurTime + left;
    	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
	}
	lfq_read(gHandle, data);	//读取一个元素

	
    //如果开始从队列中取下一个周期的数据块，并且下一个周期是自适应控制。
    if(data->cycleTime == data->leftTime && data->schemeId == SINGLE_ADAPT_SCHEMEID)
    {
		ExtendMaxGreenTime(data);	//先延长周期第1s数据块的最大绿
		for (i = 0; i < data->cycleTime - 1; i++)	//减去1是因为周期第1s已经读取了
		{	//接着预读取此周期剩余的数据块，并延长相应相位的最大绿
			block = (LineQueueData *)lfq_read_prefetch(gHandle, i);
			ExtendMaxGreenTime(block);
        }
    }
}

//读车检板数据
UInt64 ItsReadVehicleDetectorData()
{
	return 0;
}

static void RecvControlModeMsg(LineQueueData *data, ControlInfos *infos)
{
	struct msgbuf msg;

	if (msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_MODE, IPC_NOWAIT) == -1)
		return;
	//接收到新的控制方式
	if (msg.msgMode == STEP_MODE)
	{
		if (!IsStepInvalid(data, msg.msgStageNum))
		{	//步进有效
			infos->stageNum = msg.msgStageNum;
			if (infos->mode != STEP_MODE && infos->stageNum == 0)//first single step
				infos->flag = STEP_UNUSED_FLAG;
			else 
			{
				if (infos->flag == STEP_UNUSED_FLAG)//transition
				{
					infos->flag = STEP_TRANSITION_FLAG;
				}
			}
			log_debug("recv step control, stageNO = %d", infos->stageNum);
			infos->mode = STEP_MODE;
			infos->extendTime = infos->cycleLeftTime = 0;
			
		}
	}
	else
	{
		if (infos->mode != INDUCTIVE_MODE && infos->mode != INDUCTIVE_COORDINATE_MODE && infos->mode != SINGLE_ADAPT_MODE)
		{	//如果之前既不是感应也不是协调感应,则先清空存放绿灯延长时间和的数组以及车流量
			memset(infos->totalExtendTime, 0, sizeof(infos->totalExtendTime));
			gVehicleData = 0;
		}
		infos->mode = msg.msgMode;
		infos->flag = STEP_UNUSED_FLAG;
		infos->schemeId = msg.msgmSchemeId;	
		log_debug("PhasecontrolModule recv new control mode = %d, schemeId = %d", infos->mode, infos->schemeId);
	}
}

//系统初始化，先等待线性队列中有数据后依次执行黄闪和全红，随后初始化控制模式
static inline void SystemInit(LineQueueData *data, ControlInfos *infos, UInt32 *countvalue)
{
	struct msgbuf msg;
	UInt8 initSchemeId = 0;	//默认初始化为系统控制,即系统运行的第一个周期
	
	memset(data, 0, sizeof(LineQueueData));
	msgrcv(msgid, &msg, MSGSIZE, MSG_BEGIN_READ_DATA, 0);	//等待计算模块向线性队列中填充数据
	//启动定时器
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_START_TIMER;
	msgsnd(msgid, &msg, MSGSIZE, 0);
	do
	{
		RecvControlModeMsg(data, infos);
		switch (infos->mode)
		{
			case MANUAL_MODE: initSchemeId = infos->schemeId; break;
			case INDUCTIVE_MODE: initSchemeId = INDUCTIVE_SCHEMEID; break;
			case INDUCTIVE_COORDINATE_MODE: initSchemeId = INDUCTIVE_COORDINATE_SCHEMEID; break;
			default: initSchemeId = 0; break;
		}
		gCurTime = time(NULL);
		ReadLineQueueData(data, initSchemeId);
		sem_wait(&gSemForCtl);
		(*countvalue)++;
		ItsSetCurRunData(data);
		sem_post(&gSemForChan);
	} while (data->leftTime != 1); //循环结束表示启动时的黄闪和全红已经运行完毕，下面要按照控制方式开始运行了
	INFO("yellow and all red init over");
}

static void DataDeal(LineQueueData *data, ControlInfos *infos)
{
	static int tmp = 0;
	//INFO("mode=%d VehicleGreenLightKeep=%d", infos->mode, VehicleGreenLightKeep);
	switch (infos->mode)
	{
		case SYSTEM_MODE: ReadLineQueueData(data, 0); break;
		case MANUAL_MODE: ReadLineQueueData(data, infos->schemeId); break;
		case YELLOWBLINK_MODE: 
			if (infos->schemeId == 0)	//说明此时本地时段黄闪
				ReadLineQueueData(data, 0);	//读取队列信息用于本地时段切换
			data->schemeId = YELLOWBLINK_SCHEMEID;
			SetSpericalData(data, YELLOW_BLINK);
			break;
		case TURNOFF_LIGHTS_MODE: 
			if (infos->schemeId == 0)	//说明此时本地时段关灯
				ReadLineQueueData(data, 0);	//读取队列信息用于本地时段切换
			data->schemeId = TURNOFF_SCHEMEID;
			SetSpericalData(data, TURN_OFF);
			break;
		case ALLRED_MODE: 
			if (infos->schemeId == 0)	//说明此时本地时段全红
				ReadLineQueueData(data, 0);	//读取队列信息用于本地时段切换
			data->schemeId = ALLRED_SCHEMEID;
			SetSpericalData(data, ALLRED);
			break;
		case STEP_MODE:
			if ((infos->flag == STEP_UNUSED_FLAG && infos->stageNum == 0) 
				|| data->stageNum == 0) //单步步进开始,停留在当前阶段
				break;
		/*	if (infos->flag == STEP_TRANSITION_FLAG)
			{
				INFO("Transition ing...");
				if (IsStepTransitionComplete(data, infos->stageNum))
				{INFO("step falg STEP_TRANSITION_FLAG >>>> STEP_EXCUTE_FLAG");
				infos->flag = STEP_EXCUTE_FLAG;}
				INFO("after transition infos->flag = %X", infos->flag);
			}
			//步进过渡完成
			else 
			{	if (infos->flag == STEP_EXCUTE_FLAG && DirectStepToStage(data, infos->stageNum));
				{
				INFO("step falg STEP_EXCUTE_FLAG >>>> STEP_UNUSED_FLAG, step flag=%X", infos->flag);
				infos->flag = STEP_UNUSED_FLAG;
				}
			}*/
			if (infos->flag != STEP_UNUSED_FLAG)
			{
				if (IsStepTransitionComplete(data, infos->stageNum))
					if (DirectStepToStage(data, infos->stageNum))
						infos->flag = STEP_UNUSED_FLAG;
			}
			//INFO("stageNum = %d, current stageNum = %d, maxStageNum = %d", infos->stageNum, data->stageNum, data->maxStageNum);
			break;
		case INDUCTIVE_MODE:
			if (infos->extendTime > 0)	//说明当前处于感应延长期需要调整一下时间
			{
				infos->extendTime--;
				ReduceInductiveExtendTime(data);
			}
			else if (stepInductive.stepNow != 2)
			{	//SendCalculateNextCycleMsg(data, infos->schemeId);
				ReadLineQueueData(data, infos->schemeId);
			}
			else if (stepInductive.stepNow == 2)
			{//半感应驻留，每隔一周期判断时段表方案切换
				if (tmp++ > data->cycleTime)
				{
					SendCalculateNextCycleMsg(data, infos->schemeId);
					tmp = 0;
				}
			}
			break;
		 case SINGLE_ADAPT_MODE:
			if (infos->extendTime > 0)	//说明当前处于感应延长期需要调整一下时间
			{
				infos->extendTime--;
				ReduceInductiveExtendTime(data);
			}
			else
			{
				ReadLineQueueData(data, infos->schemeId);
			}
			break;
		case INDUCTIVE_COORDINATE_MODE:
			if (infos->extendTime > 0)	//说明当前处于感应延长期需要调整一下时间
			{
				infos->extendTime--;
				ReduceInductiveExtendTime(data);
			}
			else
			{
				if (infos->cycleLeftTime == 0)
				{
					ReadLineQueueData(data, infos->schemeId);
					infos->cycleLeftTime = InductiveCoordinateDeal(data, &infos->cycleLeftData);
				}
				else
				{
					infos->cycleLeftTime--;
					memcpy(data, &infos->cycleLeftData, sizeof(LineQueueData));
					ReduceInductiveExtendTime(&infos->cycleLeftData);
				}
			}
			break;
		case PEDESTRIAN_REQ_MODE:
			if (VehicleGreenLightKeep == 0)
			{
				ReadLineQueueData(data, infos->schemeId);
			}
			else if (VehicleGreenLightKeep > 0)
			{//行人过街停留在机动车常绿相位，每隔一周期判断一次时段表的方案切换
				if (tmp++ > data->cycleTime)
				{
					SendCalculateNextCycleMsg(data, infos->schemeId);
					tmp = 0;
				}
			}
			break;
		case BUS_PRIORITY_MODE:
			if (infos->extendTime > 0)
			{
				infos->extendTime--;
				ReduceInductiveExtendTime(data);
			}
			else
			{
				ReadLineQueueData(data, infos->schemeId);
			}
			break;
		case SINGLE_SPOT_OPTIMIZE: 
			ReadLineQueueData(data, infos->schemeId); 
			break;
		default: break;
	}
}

void *PhaseControlModule(void *arg)
{
	UInt32 *countvalue = (UInt32 *)arg;
	LineQueueData data;
	ControlInfos infos = {
		.mode = SYSTEM_MODE,
		.flag = STEP_UNUSED_FLAG,
		.extendTime = 0,
		.cycleLeftTime = 0,
		.totalExtendTime = {0},
		.stageNum = 0,
		.schemeId = 0,
	};
	struct msgbuf msg;
	UInt8 ret = 0;
	UINT32 busPrioData[4] = {0};

	memset(&gSpecialCarControl, 0, sizeof(SpecialCarControl));
	memset(&msg, 0, sizeof(msg));
	SystemInit(&data, &infos, countvalue);
	while (1)	//经历一次循环的时间为1s
	{	
		RecvControlModeMsg(&data, &infos);
		if (gSpecialCarControl.SpecialCarControlLevel == 0)
		{	
			gSpecialCarControl.SpecialCarControlLevel = ItsGetSpecialCarControlData(&gCalInfo, &gSpecialCarControl);
		}
		if (gSpecialCarControl.SpecialCarControlLevel > 0)
		{
			SpecialCarDataDeal(&data, &gSpecialCarControl);
		}
		else
		{
        if (((infos.mode == INDUCTIVE_MODE && data.actionId == INDUCTIVE_ACTIONID)
			|| (infos.mode == INDUCTIVE_COORDINATE_MODE && data.actionId == INDUCTIVE_COORDINATE_ACTIONID)
			|| (infos.mode == SINGLE_ADAPT_MODE && data.actionId == SINGLE_ADAPT_ACTIONID))
			&& (ret = CheckVehicleData(infos.totalExtendTime, &data)))	
		{	//感应模式下实时检查过程数据,判断是否在窗口期内有过车，计算需要延长的绿灯时间
			infos.extendTime += ret;	//ret>0表明在窗口期内，且需要延长绿灯时间
		}
		else if (infos.mode == BUS_PRIORITY_MODE)
		{
			ItsGetBusPrioData(busPrioData);
			if (busPrioData[0] > 0 || busPrioData[1] > 0 || busPrioData[2] > 0 || busPrioData[3] > 0)
				ret = CheckBusData(infos.totalExtendTime, &data, busPrioData);
			else
				ret = CheckVehicleData(infos.totalExtendTime, &data);
			infos.extendTime += ret;
			ret = 0;
		}
		else if (infos.mode == SINGLE_SPOT_OPTIMIZE)
		{
			SingleSpotPriority(infos.totalExtendTime, &data);
		}
		
		gCurTime = time(NULL);
		DataDeal(&data, &infos);	//数据处理

		if (infos.mode == PEDESTRIAN_REQ_MODE)//定周期的行人过街
		{
			if (data.maxStageNum == 4)//four phase, pedestrian twice req 
				PedestrianReqTwiceDeal(&data);
			else if (data.maxStageNum == 2)//two phase ,pedestrian once req
				PedestrianReqOnceDeal(&data);
		}
		}
		ItsCustom(&data, &gCustomParams);	//定制使用
		sem_wait(&gSemForCtl);
		(*countvalue)++;
		
		data.isStep = (infos.mode == STEP_MODE) ? TRUE : FALSE;
		ItsSetCurRunData(&data);
		sem_post(&gSemForChan);
	}
}

