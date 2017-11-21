#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "hikmsg.h"
#include "lfq.h"
#include "platform.h"
#include "gettime.h"
#include "configureManagement.h"


#define STEP_UNUSED_FLAG 	0	//步进未使用标志
#define STEP_EXCUTE_FLAG   	1	//步进执行标志

#define GREEN_BLINK_TRANSITION_TIME		3
#define YELLOW_TRANSITION_TIME			3

typedef struct controlInfos
{
	ControlMode mode;	//当前控制方式
	UInt8 flag;	//步进控制时使用
	UInt8 extendTime;	//感应控制时使用
	UInt8 totalExtendTime[NUM_PHASE];	//存放感应控制时相位已经延长的时间和
	UInt8 stageNum;	//步进阶段号
	UInt8 mSchemeId;	//手动方案号
} ControlInfos;

//extern SignalControllerPara *gRunConfigPara;
extern pthread_rwlock_t gConfigLock;
extern int gOftenPrintFlag; //打印标志
extern void *gHandle; //线性队列句柄
extern int msgid;
extern sem_t gSemForCtl;
extern sem_t gSemForDrv;
extern CustomInfo gCustomInfo;
extern SignalControllerPara *gRunConfigPara;
extern STRUCT_BINFILE_DESC gStructBinfileDesc; 
static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;   //倒计时接口信息
static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static CHANNEL_LOCK_PARAMS gChannelLockParams;
static UInt8 gChannelLockFlag = 0;
static UInt8 gGreenBlinkTransitionTime = 0;	//绿闪过渡时间
static UInt8 gYellowTransitionTime = 0;		//黄灯过渡时间
static time_t gCurTime;		//当前时间

void ItsChannelLock(CHANNEL_LOCK_PARAMS *lockparams)
{
	if (lockparams == NULL)
		return;
	gChannelLockFlag++;
	gGreenBlinkTransitionTime = GREEN_BLINK_TRANSITION_TIME;	//锁定时进行3s绿灯过渡
	gYellowTransitionTime = YELLOW_TRANSITION_TIME;  		//锁定时进行3s黄灯过渡
	pthread_rwlock_wrlock(&gCountDownLock);
	gCountDownParams.ucChannelLockStatus = 1;
	memcpy(gCountDownParams.ucChannelStatus, lockparams->ucChannelStatus, sizeof(lockparams->ucChannelStatus));
	gCountDownParams.ucWorkingTimeFlag = lockparams->ucWorkingTimeFlag;
	gCountDownParams.ucBeginTimeHour = lockparams->ucBeginTimeHour;
	gCountDownParams.ucBeginTimeMin = lockparams->ucBeginTimeMin;
	gCountDownParams.ucBeginTimeSec = lockparams->ucBeginTimeSec;
	gCountDownParams.ucEndTimeHour = lockparams->ucEndTimeHour;
	gCountDownParams.ucEndTimeMin = lockparams->ucEndTimeMin;
	gCountDownParams.ucEndTimeSec = lockparams->ucEndTimeSec;
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsChannelUnlock(void)
{
	if (gChannelLockFlag > 0)
	{
		gChannelLockFlag = 0;
		//gGreenBlinkTransitionTime = GREEN_BLINK_TRANSITION_TIME;	//解锁时进行3s绿灯过渡
		gYellowTransitionTime = YELLOW_TRANSITION_TIME;  		//解锁时进行3s黄灯过渡
	}
}

void ItsCountDownGet(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *cd)
{
	if (cd == NULL)
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(cd, &gCountDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsIsContinueRun(Boolean val)
{
	gCustomInfo.isContinueRun = val;
}

//设置黄闪、全红、关灯时的数据信息
static inline void SetSpericalData(LineQueueData *data, LightStatus status)
{
	int i;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 channelFlashStatus[NUM_CHANNEL] = {0};	//通道闪光状态

	if (gCustomInfo.isContinueRun == FALSE)
		lfq_reinit(gHandle);	//清除线性队列中的数据块，不再接着之前的方案继续运行
    //data->cycleTime = 0;
    //data->leftTime = 0;
    data->stageNum = 0;
	for (i = 0; i < NUM_PHASE; i++)
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
	    phaseInfos[i].phaseSplitLeftTime= 0;
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
		for (i = 0; i < NUM_CHANNEL; i++)
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
	for (i = 0; i < NUM_CHANNEL; i++)
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
//通道锁定过渡
static void ChannelLockTransition(UInt8 *curChannels)
{
	UInt8 transitionLockChannelStatus[NUM_CHANNEL] = {INVALID};	//要锁定的通道过渡时的状态
	int i;
	UInt8 transitionStatus = INVALID;
	Boolean transitionFlag = FALSE;
	
	if (gGreenBlinkTransitionTime == GREEN_BLINK_TRANSITION_TIME && gYellowTransitionTime == YELLOW_TRANSITION_TIME)
	{	//在进行过渡之前先把要锁定通道的当前状态保存一份
		if (gChannelLockFlag == 1)
		{	//单次通道锁定
			for (i = 0; i < NUM_CHANNEL; i++)
			{	//保存要锁定通道的当前状态
				gChannelLockParams.ucChannelStatus[i] = (gCountDownParams.ucChannelStatus[i] != INVALID) 
												 ? curChannels[i] : INVALID;
			}
		}
		else if (gChannelLockFlag >= 2)
		{	//连续通道锁定
			for (i = 0; i < NUM_CHANNEL; i++)
			{	//保存除了上次锁定的通道之外的其他通道的当前状态
				if (gChannelLockParams.ucChannelStatus[i] == INVALID 
					&& gCountDownParams.ucChannelStatus[i] != INVALID)
					gChannelLockParams.ucChannelStatus[i] = curChannels[i];
			}
		}
	}
	//首先在过渡时间内进行判断是否需要进行过渡
	if (gGreenBlinkTransitionTime + gYellowTransitionTime > 0)
	{
		if (gGreenBlinkTransitionTime > 0)
		{
			transitionStatus = GREEN_BLINK;
			gGreenBlinkTransitionTime--;
		}
		else
		{
			transitionStatus = YELLOW;
			gYellowTransitionTime--;
		}
		memcpy(transitionLockChannelStatus, gChannelLockParams.ucChannelStatus, sizeof(transitionLockChannelStatus));
		for (i = 0; i < NUM_CHANNEL; i++)
		{
			if (gCountDownParams.ucChannelStatus[i] == RED && (transitionLockChannelStatus[i] == GREEN || transitionLockChannelStatus[i] == GREEN_BLINK))
			{	//根据当前通道状态判断是否有通道是从绿或是绿闪直接锁定为红灯来决定是否需要进行过渡
				transitionFlag = TRUE;
				transitionLockChannelStatus[i] = transitionStatus;
			}
		}
	}
	else	//说明锁定过渡完成，保存当前锁定的状态用以连续过渡使用
		memcpy(gChannelLockParams.ucChannelStatus, gCountDownParams.ucChannelStatus, sizeof(gCountDownParams.ucChannelStatus));
	//根据过渡标志来决定当前需要锁定的通道状态
	for (i = 0; i < NUM_CHANNEL; i++)
	{
		if (transitionFlag)
		{	//需要过渡时按照过渡状态放行
			if (transitionLockChannelStatus[i] != INVALID)
				curChannels[i] = transitionLockChannelStatus[i];
		}
		else
		{	//无需过渡时直接按照锁定状态放行
			if (gCountDownParams.ucChannelStatus[i] != INVALID)
				curChannels[i] = gCountDownParams.ucChannelStatus[i];
		}
	}
}
//是否可进行通道解锁
static inline Boolean IsChannelUnlockAvailable(UInt8 *curChannels)
{
	UInt8 transitionUnlockChannelStatus[NUM_CHANNEL] = {INVALID};	//要解锁的通道过渡时的状态
	Boolean ret = TRUE;
	int i;
	
	if (gYellowTransitionTime > 0)
	{
		gYellowTransitionTime--;
		memcpy(transitionUnlockChannelStatus, gCountDownParams.ucChannelStatus, sizeof(transitionUnlockChannelStatus));
		for (i = 0; i < NUM_CHANNEL; i++)
		{
			if (curChannels[i] == RED && (gCountDownParams.ucChannelStatus[i] == GREEN || gCountDownParams.ucChannelStatus[i] == GREEN_BLINK))
			{	//根据当前通道状态判断是否有通道是从绿或是绿闪解锁为红灯来决定是否可进行通道解锁
				ret = FALSE;
				transitionUnlockChannelStatus[i] = YELLOW;
			}
		}
	}
	
	if (ret == FALSE)
	{	//表明不能直接解锁，需要过渡
		for (i = 0; i < NUM_CHANNEL; i++)
		{
			if (transitionUnlockChannelStatus[i] != INVALID)
			{
				curChannels[i] = transitionUnlockChannelStatus[i];
			}
		}
	}
	return ret;
}
//通道锁定
static inline void ChannelLock(LineQueueData *data)
{
	int i;
	time_t timeNow, timeStart, timeEnd;    
    struct tm start, end;
	
	if (gChannelLockFlag == 0 && gCountDownParams.ucChannelLockStatus == 0)
		return;	//表明通道已经解锁或是未锁定
	else if (gChannelLockFlag == 0 && gCountDownParams.ucChannelLockStatus > 0)
	{
		if (IsChannelUnlockAvailable(data->allChannels))
			gCountDownParams.ucChannelLockStatus = 0;
		return;	//通道即将解锁
	}
	if (gCountDownParams.ucWorkingTimeFlag == 1)
	{	//通道按时间锁定
		timeNow = time(NULL);
		localtime_r(&timeNow, &start);
		memcpy(&end, &start, sizeof(end));
		start.tm_hour = gCountDownParams.ucBeginTimeHour;
		start.tm_min = gCountDownParams.ucBeginTimeMin;
		start.tm_sec = gCountDownParams.ucBeginTimeSec;
		timeStart = mktime(&start);
		end.tm_hour = gCountDownParams.ucEndTimeHour;
		end.tm_min = gCountDownParams.ucEndTimeMin;
		end.tm_sec = gCountDownParams.ucEndTimeSec;
		timeEnd = mktime(&end);
		if (((timeStart > timeEnd) && (timeNow < timeStart && timeNow > timeEnd))
			|| ((timeStart < timeEnd) && (timeNow < timeStart || timeNow > timeEnd)))
		{
			gCountDownParams.ucChannelLockStatus = 2;	//表示通道待锁定
			return;	//当前时间不在锁定时间范围内不进行通道锁定
		}
	}
	gCountDownParams.ucChannelLockStatus = 1;
	ChannelLockTransition(data->allChannels);
}

void ItsCustom(LineQueueData *data)
{
}

void ItsCountDownOutput(LineQueueData *data)
{
}
//倒计时的输出，1s一次
void CountDownOutPut(LineQueueData *data, ControlMode mode)
{
	int i;
	UInt8 nChannelId = 0;
	UInt8 nPhaseId = 0;	
	UInt8 channelTableId = data->phaseTableId;
	PhaseInfo *phaseInfos = data->phaseInfos;
	struct msgbuf msg;
	
	pthread_rwlock_wrlock(&gCountDownLock);
	gCountDownParams.unExtraParamHead = 0x6e6e;
	gCountDownParams.unExtraParamID = 0x9e;
	gCountDownParams.ucPlanNo = data->schemeId;
	gCountDownParams.ucCurCycleTime = data->cycleTime;
	gCountDownParams.ucCurRunningTime = (data->cycleTime) ? (data->cycleTime - data->leftTime + 1) : 0;	//周期运行时间是从1开始

    
	switch (data->schemeId)//1~16方案描述采用配置文件里面的，特殊方案采用中文(英文长度越界，慎用strcpy)，其余方案直接采用中文加方案号
	{
		case YELLOWBLINK_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "黄闪"); break;
		case ALLRED_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "全红"); break;
		case TURNOFF_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "关灯"); break;
		case INDUCTIVE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "感应"); break;
	}
	for (i = 0; i < NUM_PHASE; i++)
	{
		gCountDownParams.stVehPhaseCountingDown[i][0] = phaseInfos[i].phaseStatus;
		gCountDownParams.stVehPhaseCountingDown[i][1] = phaseInfos[i].phaseLeftTime;
		gCountDownParams.stPedPhaseCountingDown[i][0] = phaseInfos[i].pedestrianPhaseStatus;
		gCountDownParams.stPedPhaseCountingDown[i][1] = phaseInfos[i].pedestrianPhaseLeftTime;

		gCountDownParams.ucOverlap[i][0] = phaseInfos[i].followPhaseStatus;
		gCountDownParams.ucOverlap[i][1] = phaseInfos[i].followPhaseLeftTime;
		gCountDownParams.stPhaseRunningInfo[i][0] = phaseInfos[i].splitTime;
		gCountDownParams.stPhaseRunningInfo[i][1] = (phaseInfos[i].phaseStatus != RED && phaseInfos[i].splitTime > 0) ? (phaseInfos[i].splitTime - phaseInfos[i].phaseSplitLeftTime + 1) : 0;
	}
	gCountDownParams.ucReserved[0] = (mode == STEP_MODE) ? 1 : 0;
	gCountDownParams.ucReserved[1] = data->maxStageNum;
	memcpy(gCountDownParams.ucChannelRealStatus, data->allChannels, sizeof(data->allChannels));
	//计算各个通道的倒计时
	for(i = 0; i < 32; i++)
	{
        nChannelId = gRunConfigPara->stChannel[channelTableId - 1][i].nChannelID;
        nPhaseId = gRunConfigPara->stChannel[channelTableId - 1][i].nControllerID;
        if((nChannelId < 1 || nChannelId > 32) || (nPhaseId < 1 || nPhaseId > 16))
            continue;
        
        switch(gRunConfigPara->stChannel[channelTableId - 1][i].nControllerType)
        {
            case 2://机动车
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.stVehPhaseCountingDown[nPhaseId - 1][1];
                break;
            case 3://行人
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.stPedPhaseCountingDown[nPhaseId - 1][1];
                break;
            case 4://跟随
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.ucOverlap[nPhaseId - 1][1];
                break;
            default:
                gCountDownParams.ucChannelCountdown[nChannelId - 1]= 0;
                break;
        }
	}
	pthread_rwlock_unlock(&gCountDownLock);
	
	ItsCountDownOutput(data);	//输出倒计时牌
	//发送通道状态给红灯信号检测器
	msg.mtype = MSG_RED_SIGNAL_CHECK;
	memcpy(msg.msgAllChannels, data->allChannels, sizeof(data->allChannels));
	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
}
//读取线性队列中的1s数据信息
static inline void ReadLineQueueData(LineQueueData *data, UInt8 schemeId)
{
    struct msgbuf msg;
	int left = lfq_element_count(gHandle);
	
	if (left <= AHEAD_OF_TIME)
	{
	    memset(&msg, 0, sizeof(msg));
	    msg.mtype = MSG_START_CALCULATE_NEXT_CYCLE;
    	msg.msgSchemeId = schemeId;
		msg.msgCalTime = gCurTime + left;
    	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
	}
	lfq_read(gHandle, data);	//读取一个元素
}
//判断步进是否无效
static inline Boolean IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	if ((data->stageNum == 0)    //表明当前正在执行黄闪、全红或是关灯控制
		//|| (data->stageNum == stageNum)	//表明当前正在运行的正是需要步进的阶段
		|| (stageNum > data->maxStageNum))	//表明需要步进的阶段号大于系统最大的阶段号
	    return TRUE;	//以上这些情况进行步进都是无效的
	for (i = 0; i < NUM_PHASE; i++)
	{	//当正在运行的相位有绿闪、黄闪、黄灯、全红、关灯时，此时步进无效
        if (phaseInfos[i].phaseStatus == GREEN_BLINK
			|| phaseInfos[i].phaseStatus == YELLOW_BLINK
			|| phaseInfos[i].phaseStatus == YELLOW
			|| phaseInfos[i].phaseStatus == ALLRED
			|| phaseInfos[i].phaseStatus == RED_YELLOW
			|| phaseInfos[i].phaseStatus == TURN_OFF)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//直接步进到某个阶段，中间不会有绿闪、黄灯和全红的过渡
static void DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
	int i;
	
	if (data->stageNum == 0)
		return;
	while (stageNum > 0 && data->stageNum != stageNum)
	{	//步进到阶段号指定的阶段
		ReadLineQueueData(data, data->schemeId);
		if (data->stageNum == 0)
		    return;
	}
	for (i = 0; i < NUM_PHASE; i++)
	{	//此时放行的相位状态都应为GREEN
        if (phaseInfos[i].followPhaseStatus != INVALID && phaseInfos[i].followPhaseStatus != RED)
	    {
            phaseInfos[i].followPhaseStatus = GREEN;
	        phaseInfos[i].followPhaseLeftTime = 0;
	    }
		if (phaseInfos[i].phaseStatus != INVALID && phaseInfos[i].phaseStatus != RED)
		{
			phaseInfos[i].phaseStatus = GREEN;
			phaseInfos[i].phaseLeftTime = 0;
	        phaseInfos[i].phaseSplitLeftTime= 0;
	        //phaseInfos[i].splitTime = 0;
			if (phaseInfos[i].pedestrianPhaseStatus != INVALID)
    	    {
    	        phaseInfos[i].pedestrianPhaseStatus = GREEN;
    	        phaseInfos[i].pedestrianPhaseLeftTime = 0;
    	    }
		}
		
	}
	for (i = 0; i < NUM_CHANNEL; i++)
	{	//如果此时正在运行的通道有绿闪、黄灯、全红，但由于是步进控制所以这些状态都应为GREEN
		if (allChannels[i] == GREEN_BLINK
			|| allChannels[i] == YELLOW
			|| allChannels[i] == ALLRED)
		{
			allChannels[i] = GREEN;
		}
	}
}
//判断步进过渡是否完成
static Boolean IsStepTransitionComplete(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	UInt8 currentStageNum = data->stageNum;
	
	while (1)
	{	
		ReadLineQueueData(data, data->schemeId);
		if (currentStageNum != data->stageNum || data->stageNum == 0)
			return TRUE;	//已经步进到下一阶段，步进完成
		for (i = 0; i < NUM_PHASE; i++)
		{
            if (phaseInfos[i].phaseStatus == GREEN_BLINK
				|| phaseInfos[i].phaseStatus == YELLOW
				|| phaseInfos[i].phaseStatus == ALLRED)
			{	//步进要经历绿闪、黄灯、全红这些状态，不能直接跳到下一相位
				return FALSE;
			}
		}
	}
}

static void AdjustInductiveControlTime(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	UInt8 nextPhaseIds[NUM_PHASE] = {0};	//存放每个相位运行之后即将运行的下一相位号
	
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseInfos[i].phaseLeftTime > 0)
			phaseInfos[i].phaseLeftTime--;
		if (phaseInfos[i].pedestrianPhaseLeftTime > 0)
			phaseInfos[i].pedestrianPhaseLeftTime--;
		if (phaseInfos[i].phaseSplitLeftTime > 0)
			phaseInfos[i].phaseSplitLeftTime--;
		if (phaseInfos[i].followPhaseLeftTime > 0)
			phaseInfos[i].followPhaseLeftTime--;
	}
	if (data->leftTime > 0)
		data->leftTime--;
}

//读车检板数据
UInt16 ItsReadVehicleDetectorData(int boardNum)
{
	return 0;
}

static UInt8 CheckVehicleData(UInt8 *totalExtendTime, LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i = 0, j = 0, n = 0;
	UInt16 vehicleData = 0;	//过车数据
	UInt8 extendTime = 0;	//延长时间
	UInt8 timeGap = 0;
	LineQueueData *block;
	
	//查看当前绿灯相位是否在窗口期内有过车信息
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseInfos[i].phaseStatus == INVALID || phaseInfos[i].vehicleDetectorId == 0 || phaseInfos[i].unitExtendGreen == 0)
		    continue;
		if (phaseInfos[i].phaseStatus != GREEN && phaseInfos[i].phaseStatus != GREEN_BLINK)
		{	//当相位不是绿灯状态时清除之前延长的绿灯时间和
			totalExtendTime[i] = 0;
			continue;
		}
		//下面的相位状态不是GREEN就是GREEN_BLINK
		if (phaseInfos[i].phaseLeftTime > data->checkTime && phaseInfos[i].phaseLeftTime <= data->checkTime + phaseInfos[i].unitExtendGreen)
		{	//在窗口期内采集过车信息
			vehicleData = ItsReadVehicleDetectorData((phaseInfos[i].vehicleDetectorId - 1) / 16 + 1);
			//INFO("DEBUG vehicleDetectorId = %d, vehicleData = %#x", phaseInfos[i].vehicleDetectorId, vehicleData);
			if ((BIT(vehicleData, (phaseInfos[i].vehicleDetectorId - 1) % 16) == 1)
				&& (phaseInfos[i].maxExtendGreen > totalExtendTime[i]))
			{	//表示在窗口期内有过车而且还可以继续延长绿灯时间
				timeGap = phaseInfos[i].maxExtendGreen - totalExtendTime[i];
				extendTime = min(timeGap, phaseInfos[i].unitExtendGreen);
				totalExtendTime[i] += extendTime;
				if (phaseInfos[i].pedestrianPhaseLeftTime > 0)
					phaseInfos[i].pedestrianPhaseLeftTime += extendTime;
				phaseInfos[i].phaseLeftTime += extendTime;
				phaseInfos[i].splitTime += extendTime;
				phaseInfos[i].phaseSplitLeftTime += extendTime;
				INFO("DEBUG extendTime = %d, totalExtendTime[%d] = %d", extendTime, i, totalExtendTime[i]);
				break;
			}
		}
	}
	if (extendTime > 0)
	{
		for (j = 0; j < NUM_PHASE; j++)
		{
			if (phaseInfos[i].phaseStatus != INVALID && j != i)
			{
				if (phaseInfos[j].pedestrianPhaseLeftTime > 0)
					phaseInfos[j].pedestrianPhaseLeftTime += extendTime;
				phaseInfos[j].phaseLeftTime += extendTime;
			}
			if (phaseInfos[i].followPhaseStatus != INVALID)
				phaseInfos[j].followPhaseLeftTime += extendTime;
		}
		//调整线性队列中当前周期剩余的所有1s钟信息里的周期时间和对应的绿信比
		for (n = 0; n < data->leftTime - 1; n++)
		{							//减去1是因为当前这1s也包含在内
			block = (LineQueueData *)lfq_read_prefetch(gHandle, n);
			block->cycleTime += extendTime;
			block->phaseInfos[i].splitTime += extendTime;
		}
		data->leftTime += extendTime;
		data->cycleTime += extendTime;
	}
	
	return extendTime;
}

static inline void SendChannelStatus(LineQueueData *data)
{
	struct msgbuf msg;
	int ret = msgrcv(msgid, &msg, MSGSIZE, MSG_CHANNEL_CHECK, IPC_NOWAIT);	//非阻塞接受通道检测消息
	int i;

	if (ret > 0)
	{	//如果接受到通道检测消息，则使用接受的通道状态点灯
		for (i = 0; i < NUM_CHANNEL; i++)
		{
			if (data->allChannels[i] != INVALID)
				data->allChannels[i] = (i == msg.msgChannelId - 1) ? msg.msgChannelStatus : TURN_OFF;
		}
	}
	memset(&msg, 0, sizeof(msg));
	memcpy(msg.msgAllChannels, data->allChannels, sizeof(data->allChannels));
	msg.mtype = MSG_CHANNEL_STATUS;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

static void RecvControlModeMsg(LineQueueData *data, ControlInfos *infos)
{
	struct msgbuf msg;

	if (msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_MODE, IPC_NOWAIT) == -1)
		return;
	//接收到新的控制方式
	if (msg.msgMode == STEP_MODE)
	{
		if (IsStepInvalid(data, msg.msgStageNum))
		{	//步进无效时需要把当前的控制方式反馈给策略控制模块
			if (infos->mode != STEP_MODE)
			{
				msg.mtype = MSG_CONTROL_TYPE;
				msg.msgMode = infos->mode;
				msgsnd(msgid, &msg, MSGSIZE, 0);	//返回之前的控制方式给策略控制模块
			}
		}
		else
		{
			infos->stageNum = msg.msgStageNum;
			if (infos->mode == STEP_MODE)
				infos->flag = STEP_EXCUTE_FLAG;
			infos->mode = STEP_MODE;
			INFO("recv step control, stageNO = %d", infos->stageNum);
		}
	}
	else
	{
		if (infos->mode != msg.msgMode || infos->mode == MANUAL_MODE)
		{
			infos->mode = msg.msgMode;
			infos->flag = STEP_UNUSED_FLAG;
			if (infos->mode == INDUCTIVE_MODE)
			{	//跳转到感应模式时先清空存放绿灯延长时间和的数组
				memset(infos->totalExtendTime, 0, sizeof(infos->totalExtendTime));
			}
			else if (infos->mode == MANUAL_MODE)
				infos->mSchemeId = msg.msgmSchemeId;	
			INFO("recv new control mode = %d", infos->mode);
		}
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
			case MANUAL_MODE: initSchemeId = infos->mSchemeId; break;
			case INDUCTIVE_MODE: initSchemeId = INDUCTIVE_SCHEMEID; break;
			default: initSchemeId = 0; break;
		}
		ReadLineQueueData(data, initSchemeId);
		sem_wait(&gSemForCtl);
		(*countvalue)++;
		gCurTime = GetLocalTime();
		SendChannelStatus(data);
	} while (data->leftTime != 1); //循环结束表示启动时的黄闪和全红已经运行完毕，下面要按照控制方式开始运行了
	INFO("yellow and all red init over");
}

static void DataDeal(LineQueueData *data, ControlInfos *infos)
{
	switch (infos->mode)
	{
		case SYSTEM_MODE: ReadLineQueueData(data, 0); break;
		case MANUAL_MODE: ReadLineQueueData(data, infos->mSchemeId); break;
		case YELLOWBLINK_MODE: 
			if (data->schemeId != YELLOWBLINK_SCHEMEID)
			{
				data->schemeId = YELLOWBLINK_SCHEMEID;
				SetSpericalData(data, YELLOW_BLINK);
			}
			break;
		case TURNOFF_LIGHTS_MODE: 
			if (data->schemeId != TURNOFF_SCHEMEID)
			{
				data->schemeId = TURNOFF_SCHEMEID;
				SetSpericalData(data, TURN_OFF);
			}
			break;
		case ALLRED_MODE: 
			if (data->schemeId != ALLRED_SCHEMEID)
			{
				data->schemeId = ALLRED_SCHEMEID;
				SetSpericalData(data, ALLRED);
			}
			break;
		case STEP_MODE:
			if ((infos->flag == STEP_UNUSED_FLAG && infos->stageNum == 0) 
				|| data->stageNum == 0
				|| infos->stageNum == data->stageNum)
			{   //单步步进开始、过渡完成或是跳转步进阶段号与当前阶段号一样时，停留在当前阶段
				break;
			}
			if (IsStepTransitionComplete(data) == TRUE)
			{	//步进过渡完成
				DirectStepToStage(data, infos->stageNum);
				infos->flag = STEP_UNUSED_FLAG;
			}				
			//INFO("stageNum = %d, current stageNum = %d, maxStageNum = %d", infos->stageNum, data->stageNum, data->maxStageNum);
			break;
		case INDUCTIVE_MODE:
			if (infos->extendTime > 0)	//说明当前处于感应延长期需要调整一下时间
			{
				infos->extendTime--;
				AdjustInductiveControlTime(data);
			}
			else
				ReadLineQueueData(data, INDUCTIVE_SCHEMEID);
			break;
		default: break;
	}
}

void *PhaseControlModule(void *arg)
{
	UInt32 *countvalue = (UInt32 *)arg;
	LineQueueData data;
	LineQueueData tmpData;//用来发送给倒计时器的数据
	ControlInfos infos = {
		.mode = SYSTEM_MODE,
		.flag = STEP_UNUSED_FLAG,
		.extendTime = 0,
		.totalExtendTime = {0},
		.stageNum = 0,
		.mSchemeId = 0,
	};
	struct msgbuf msg;
	UInt8 ret = 0;
	UInt8 nCount = 0;
	
	memset(&msg, 0, sizeof(msg));
	SystemInit(&data, &infos, countvalue);
	while (1)	//经历一次循环的时间为1s
	{	
		if(nCount == 0)
		{
			RecvControlModeMsg(&data, &infos);
			if (infos.mode == INDUCTIVE_MODE && data.schemeId == INDUCTIVE_SCHEMEID 
					&& (ret = CheckVehicleData(infos.totalExtendTime, &data)))	
			{	//感应模式下实时检查过程数据,判断是否在窗口期内有过车，计算需要延长的绿灯时间
				infos.extendTime += ret;	//ret>0表明在窗口期内，且需要延长绿灯时间
			}
			DataDeal(&data, &infos);	//数据处理
			ItsCustom(&data);	//定制使用
			ChannelLock(&data);			//通道锁定
			//sem_wait(&gSemForCtl);
			
			gCurTime = GetLocalTime();	
		}	
		nCount++;
		sem_wait(&gSemForCtl);
		(*countvalue)++;	
		memcpy(&tmpData,&data,sizeof(data));//确保每秒读取的数据不会被修改
		CountDownOutPut(&tmpData, infos.mode);	//倒计时输出
		SendChannelStatus(&tmpData);
		nCount = ((nCount == 4) ? 0 : nCount);
		
	}
}
