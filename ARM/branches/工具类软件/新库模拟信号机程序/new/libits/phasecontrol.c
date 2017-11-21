#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "hikmsg.h"
#include "lfq.h"
#include "platform.h"
#include "gettime.h"
#include "configureManagement.h"


#define STEP_UNUSED_FLAG 	0	//����δʹ�ñ�־
#define STEP_EXCUTE_FLAG   	1	//����ִ�б�־

#define GREEN_BLINK_TRANSITION_TIME		3
#define YELLOW_TRANSITION_TIME			3

typedef struct controlInfos
{
	ControlMode mode;	//��ǰ���Ʒ�ʽ
	UInt8 flag;	//��������ʱʹ��
	UInt8 extendTime;	//��Ӧ����ʱʹ��
	UInt8 totalExtendTime[NUM_PHASE];	//��Ÿ�Ӧ����ʱ��λ�Ѿ��ӳ���ʱ���
	UInt8 stageNum;	//�����׶κ�
	UInt8 mSchemeId;	//�ֶ�������
} ControlInfos;

//extern SignalControllerPara *gRunConfigPara;
extern pthread_rwlock_t gConfigLock;
extern int gOftenPrintFlag; //��ӡ��־
extern void *gHandle; //���Զ��о��
extern int msgid;
extern sem_t gSemForCtl;
extern sem_t gSemForDrv;
extern CustomInfo gCustomInfo;
extern SignalControllerPara *gRunConfigPara;
extern STRUCT_BINFILE_DESC gStructBinfileDesc; 
static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;   //����ʱ�ӿ���Ϣ
static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static CHANNEL_LOCK_PARAMS gChannelLockParams;
static UInt8 gChannelLockFlag = 0;
static UInt8 gGreenBlinkTransitionTime = 0;	//��������ʱ��
static UInt8 gYellowTransitionTime = 0;		//�Ƶƹ���ʱ��
static time_t gCurTime;		//��ǰʱ��

void ItsChannelLock(CHANNEL_LOCK_PARAMS *lockparams)
{
	if (lockparams == NULL)
		return;
	gChannelLockFlag++;
	gGreenBlinkTransitionTime = GREEN_BLINK_TRANSITION_TIME;	//����ʱ����3s�̵ƹ���
	gYellowTransitionTime = YELLOW_TRANSITION_TIME;  		//����ʱ����3s�Ƶƹ���
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
		//gGreenBlinkTransitionTime = GREEN_BLINK_TRANSITION_TIME;	//����ʱ����3s�̵ƹ���
		gYellowTransitionTime = YELLOW_TRANSITION_TIME;  		//����ʱ����3s�Ƶƹ���
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

//���û�����ȫ�졢�ص�ʱ��������Ϣ
static inline void SetSpericalData(LineQueueData *data, LightStatus status)
{
	int i;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 channelFlashStatus[NUM_CHANNEL] = {0};	//ͨ������״̬

	if (gCustomInfo.isContinueRun == FALSE)
		lfq_reinit(gHandle);	//������Զ����е����ݿ飬���ٽ���֮ǰ�ķ�����������
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
			if (gRunConfigPara->stChannel[i].nFlashLightType == 0x04)	//����
				channelFlashStatus[i] = RED_BLINK;
			else if (gRunConfigPara->stChannel[i].nFlashLightType == 0)	//����
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
//ͨ����������
static void ChannelLockTransition(UInt8 *curChannels)
{
	UInt8 transitionLockChannelStatus[NUM_CHANNEL] = {INVALID};	//Ҫ������ͨ������ʱ��״̬
	int i;
	UInt8 transitionStatus = INVALID;
	Boolean transitionFlag = FALSE;
	
	if (gGreenBlinkTransitionTime == GREEN_BLINK_TRANSITION_TIME && gYellowTransitionTime == YELLOW_TRANSITION_TIME)
	{	//�ڽ��й���֮ǰ�Ȱ�Ҫ����ͨ���ĵ�ǰ״̬����һ��
		if (gChannelLockFlag == 1)
		{	//����ͨ������
			for (i = 0; i < NUM_CHANNEL; i++)
			{	//����Ҫ����ͨ���ĵ�ǰ״̬
				gChannelLockParams.ucChannelStatus[i] = (gCountDownParams.ucChannelStatus[i] != INVALID) 
												 ? curChannels[i] : INVALID;
			}
		}
		else if (gChannelLockFlag >= 2)
		{	//����ͨ������
			for (i = 0; i < NUM_CHANNEL; i++)
			{	//��������ϴ�������ͨ��֮�������ͨ���ĵ�ǰ״̬
				if (gChannelLockParams.ucChannelStatus[i] == INVALID 
					&& gCountDownParams.ucChannelStatus[i] != INVALID)
					gChannelLockParams.ucChannelStatus[i] = curChannels[i];
			}
		}
	}
	//�����ڹ���ʱ���ڽ����ж��Ƿ���Ҫ���й���
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
			{	//���ݵ�ǰͨ��״̬�ж��Ƿ���ͨ���Ǵ��̻�������ֱ������Ϊ����������Ƿ���Ҫ���й���
				transitionFlag = TRUE;
				transitionLockChannelStatus[i] = transitionStatus;
			}
		}
	}
	else	//˵������������ɣ����浱ǰ������״̬������������ʹ��
		memcpy(gChannelLockParams.ucChannelStatus, gCountDownParams.ucChannelStatus, sizeof(gCountDownParams.ucChannelStatus));
	//���ݹ��ɱ�־��������ǰ��Ҫ������ͨ��״̬
	for (i = 0; i < NUM_CHANNEL; i++)
	{
		if (transitionFlag)
		{	//��Ҫ����ʱ���չ���״̬����
			if (transitionLockChannelStatus[i] != INVALID)
				curChannels[i] = transitionLockChannelStatus[i];
		}
		else
		{	//�������ʱֱ�Ӱ�������״̬����
			if (gCountDownParams.ucChannelStatus[i] != INVALID)
				curChannels[i] = gCountDownParams.ucChannelStatus[i];
		}
	}
}
//�Ƿ�ɽ���ͨ������
static inline Boolean IsChannelUnlockAvailable(UInt8 *curChannels)
{
	UInt8 transitionUnlockChannelStatus[NUM_CHANNEL] = {INVALID};	//Ҫ������ͨ������ʱ��״̬
	Boolean ret = TRUE;
	int i;
	
	if (gYellowTransitionTime > 0)
	{
		gYellowTransitionTime--;
		memcpy(transitionUnlockChannelStatus, gCountDownParams.ucChannelStatus, sizeof(transitionUnlockChannelStatus));
		for (i = 0; i < NUM_CHANNEL; i++)
		{
			if (curChannels[i] == RED && (gCountDownParams.ucChannelStatus[i] == GREEN || gCountDownParams.ucChannelStatus[i] == GREEN_BLINK))
			{	//���ݵ�ǰͨ��״̬�ж��Ƿ���ͨ���Ǵ��̻�����������Ϊ����������Ƿ�ɽ���ͨ������
				ret = FALSE;
				transitionUnlockChannelStatus[i] = YELLOW;
			}
		}
	}
	
	if (ret == FALSE)
	{	//��������ֱ�ӽ�������Ҫ����
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
//ͨ������
static inline void ChannelLock(LineQueueData *data)
{
	int i;
	time_t timeNow, timeStart, timeEnd;    
    struct tm start, end;
	
	if (gChannelLockFlag == 0 && gCountDownParams.ucChannelLockStatus == 0)
		return;	//����ͨ���Ѿ���������δ����
	else if (gChannelLockFlag == 0 && gCountDownParams.ucChannelLockStatus > 0)
	{
		if (IsChannelUnlockAvailable(data->allChannels))
			gCountDownParams.ucChannelLockStatus = 0;
		return;	//ͨ����������
	}
	if (gCountDownParams.ucWorkingTimeFlag == 1)
	{	//ͨ����ʱ������
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
			gCountDownParams.ucChannelLockStatus = 2;	//��ʾͨ��������
			return;	//��ǰʱ�䲻������ʱ�䷶Χ�ڲ�����ͨ������
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
//����ʱ�������1sһ��
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
	gCountDownParams.ucCurRunningTime = (data->cycleTime) ? (data->cycleTime - data->leftTime + 1) : 0;	//��������ʱ���Ǵ�1��ʼ

    
	switch (data->schemeId)//1~16�����������������ļ�����ģ����ⷽ����������(Ӣ�ĳ���Խ�磬����strcpy)�����෽��ֱ�Ӳ������ļӷ�����
	{
		case YELLOWBLINK_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "����"); break;
		case ALLRED_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "ȫ��"); break;
		case TURNOFF_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "�ص�"); break;
		case INDUCTIVE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "��Ӧ"); break;
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
	//�������ͨ���ĵ���ʱ
	for(i = 0; i < 32; i++)
	{
        nChannelId = gRunConfigPara->stChannel[channelTableId - 1][i].nChannelID;
        nPhaseId = gRunConfigPara->stChannel[channelTableId - 1][i].nControllerID;
        if((nChannelId < 1 || nChannelId > 32) || (nPhaseId < 1 || nPhaseId > 16))
            continue;
        
        switch(gRunConfigPara->stChannel[channelTableId - 1][i].nControllerType)
        {
            case 2://������
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.stVehPhaseCountingDown[nPhaseId - 1][1];
                break;
            case 3://����
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.stPedPhaseCountingDown[nPhaseId - 1][1];
                break;
            case 4://����
                gCountDownParams.ucChannelCountdown[nChannelId - 1] = gCountDownParams.ucOverlap[nPhaseId - 1][1];
                break;
            default:
                gCountDownParams.ucChannelCountdown[nChannelId - 1]= 0;
                break;
        }
	}
	pthread_rwlock_unlock(&gCountDownLock);
	
	ItsCountDownOutput(data);	//�������ʱ��
	//����ͨ��״̬������źż����
	msg.mtype = MSG_RED_SIGNAL_CHECK;
	memcpy(msg.msgAllChannels, data->allChannels, sizeof(data->allChannels));
	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
}
//��ȡ���Զ����е�1s������Ϣ
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
	lfq_read(gHandle, data);	//��ȡһ��Ԫ��
}
//�жϲ����Ƿ���Ч
static inline Boolean IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	if ((data->stageNum == 0)    //������ǰ����ִ�л�����ȫ����ǹصƿ���
		//|| (data->stageNum == stageNum)	//������ǰ�������е�������Ҫ�����Ľ׶�
		|| (stageNum > data->maxStageNum))	//������Ҫ�����Ľ׶κŴ���ϵͳ���Ľ׶κ�
	    return TRUE;	//������Щ������в���������Ч��
	for (i = 0; i < NUM_PHASE; i++)
	{	//���������е���λ���������������Ƶơ�ȫ�졢�ص�ʱ����ʱ������Ч
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

//ֱ�Ӳ�����ĳ���׶Σ��м䲻�����������Ƶƺ�ȫ��Ĺ���
static void DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
	int i;
	
	if (data->stageNum == 0)
		return;
	while (stageNum > 0 && data->stageNum != stageNum)
	{	//�������׶κ�ָ���Ľ׶�
		ReadLineQueueData(data, data->schemeId);
		if (data->stageNum == 0)
		    return;
	}
	for (i = 0; i < NUM_PHASE; i++)
	{	//��ʱ���е���λ״̬��ӦΪGREEN
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
	{	//�����ʱ�������е�ͨ�����������Ƶơ�ȫ�죬�������ǲ�������������Щ״̬��ӦΪGREEN
		if (allChannels[i] == GREEN_BLINK
			|| allChannels[i] == YELLOW
			|| allChannels[i] == ALLRED)
		{
			allChannels[i] = GREEN;
		}
	}
}
//�жϲ��������Ƿ����
static Boolean IsStepTransitionComplete(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	UInt8 currentStageNum = data->stageNum;
	
	while (1)
	{	
		ReadLineQueueData(data, data->schemeId);
		if (currentStageNum != data->stageNum || data->stageNum == 0)
			return TRUE;	//�Ѿ���������һ�׶Σ��������
		for (i = 0; i < NUM_PHASE; i++)
		{
            if (phaseInfos[i].phaseStatus == GREEN_BLINK
				|| phaseInfos[i].phaseStatus == YELLOW
				|| phaseInfos[i].phaseStatus == ALLRED)
			{	//����Ҫ�����������Ƶơ�ȫ����Щ״̬������ֱ��������һ��λ
				return FALSE;
			}
		}
	}
}

static void AdjustInductiveControlTime(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	UInt8 nextPhaseIds[NUM_PHASE] = {0};	//���ÿ����λ����֮�󼴽����е���һ��λ��
	
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

//�����������
UInt16 ItsReadVehicleDetectorData(int boardNum)
{
	return 0;
}

static UInt8 CheckVehicleData(UInt8 *totalExtendTime, LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i = 0, j = 0, n = 0;
	UInt16 vehicleData = 0;	//��������
	UInt8 extendTime = 0;	//�ӳ�ʱ��
	UInt8 timeGap = 0;
	LineQueueData *block;
	
	//�鿴��ǰ�̵���λ�Ƿ��ڴ��������й�����Ϣ
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseInfos[i].phaseStatus == INVALID || phaseInfos[i].vehicleDetectorId == 0 || phaseInfos[i].unitExtendGreen == 0)
		    continue;
		if (phaseInfos[i].phaseStatus != GREEN && phaseInfos[i].phaseStatus != GREEN_BLINK)
		{	//����λ�����̵�״̬ʱ���֮ǰ�ӳ����̵�ʱ���
			totalExtendTime[i] = 0;
			continue;
		}
		//�������λ״̬����GREEN����GREEN_BLINK
		if (phaseInfos[i].phaseLeftTime > data->checkTime && phaseInfos[i].phaseLeftTime <= data->checkTime + phaseInfos[i].unitExtendGreen)
		{	//�ڴ������ڲɼ�������Ϣ
			vehicleData = ItsReadVehicleDetectorData((phaseInfos[i].vehicleDetectorId - 1) / 16 + 1);
			//INFO("DEBUG vehicleDetectorId = %d, vehicleData = %#x", phaseInfos[i].vehicleDetectorId, vehicleData);
			if ((BIT(vehicleData, (phaseInfos[i].vehicleDetectorId - 1) % 16) == 1)
				&& (phaseInfos[i].maxExtendGreen > totalExtendTime[i]))
			{	//��ʾ�ڴ��������й������һ����Լ����ӳ��̵�ʱ��
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
		//�������Զ����е�ǰ����ʣ�������1s����Ϣ�������ʱ��Ͷ�Ӧ�����ű�
		for (n = 0; n < data->leftTime - 1; n++)
		{							//��ȥ1����Ϊ��ǰ��1sҲ��������
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
	int ret = msgrcv(msgid, &msg, MSGSIZE, MSG_CHANNEL_CHECK, IPC_NOWAIT);	//����������ͨ�������Ϣ
	int i;

	if (ret > 0)
	{	//������ܵ�ͨ�������Ϣ����ʹ�ý��ܵ�ͨ��״̬���
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
	//���յ��µĿ��Ʒ�ʽ
	if (msg.msgMode == STEP_MODE)
	{
		if (IsStepInvalid(data, msg.msgStageNum))
		{	//������Чʱ��Ҫ�ѵ�ǰ�Ŀ��Ʒ�ʽ���������Կ���ģ��
			if (infos->mode != STEP_MODE)
			{
				msg.mtype = MSG_CONTROL_TYPE;
				msg.msgMode = infos->mode;
				msgsnd(msgid, &msg, MSGSIZE, 0);	//����֮ǰ�Ŀ��Ʒ�ʽ�����Կ���ģ��
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
			{	//��ת����Ӧģʽʱ����մ���̵��ӳ�ʱ��͵�����
				memset(infos->totalExtendTime, 0, sizeof(infos->totalExtendTime));
			}
			else if (infos->mode == MANUAL_MODE)
				infos->mSchemeId = msg.msgmSchemeId;	
			INFO("recv new control mode = %d", infos->mode);
		}
	}
}

//ϵͳ��ʼ�����ȵȴ����Զ����������ݺ�����ִ�л�����ȫ�죬����ʼ������ģʽ
static inline void SystemInit(LineQueueData *data, ControlInfos *infos, UInt32 *countvalue)
{
	struct msgbuf msg;
	UInt8 initSchemeId = 0;	//Ĭ�ϳ�ʼ��Ϊϵͳ����,��ϵͳ���еĵ�һ������
	
	memset(data, 0, sizeof(LineQueueData));
	msgrcv(msgid, &msg, MSGSIZE, MSG_BEGIN_READ_DATA, 0);	//�ȴ�����ģ�������Զ������������
	//������ʱ��
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
	} while (data->leftTime != 1); //ѭ��������ʾ����ʱ�Ļ�����ȫ���Ѿ�������ϣ�����Ҫ���տ��Ʒ�ʽ��ʼ������
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
			{   //����������ʼ��������ɻ�����ת�����׶κ��뵱ǰ�׶κ�һ��ʱ��ͣ���ڵ�ǰ�׶�
				break;
			}
			if (IsStepTransitionComplete(data) == TRUE)
			{	//�����������
				DirectStepToStage(data, infos->stageNum);
				infos->flag = STEP_UNUSED_FLAG;
			}				
			//INFO("stageNum = %d, current stageNum = %d, maxStageNum = %d", infos->stageNum, data->stageNum, data->maxStageNum);
			break;
		case INDUCTIVE_MODE:
			if (infos->extendTime > 0)	//˵����ǰ���ڸ�Ӧ�ӳ�����Ҫ����һ��ʱ��
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
	LineQueueData tmpData;//�������͸�����ʱ��������
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
	while (1)	//����һ��ѭ����ʱ��Ϊ1s
	{	
		if(nCount == 0)
		{
			RecvControlModeMsg(&data, &infos);
			if (infos.mode == INDUCTIVE_MODE && data.schemeId == INDUCTIVE_SCHEMEID 
					&& (ret = CheckVehicleData(infos.totalExtendTime, &data)))	
			{	//��Ӧģʽ��ʵʱ����������,�ж��Ƿ��ڴ��������й�����������Ҫ�ӳ����̵�ʱ��
				infos.extendTime += ret;	//ret>0�����ڴ������ڣ�����Ҫ�ӳ��̵�ʱ��
			}
			DataDeal(&data, &infos);	//���ݴ���
			ItsCustom(&data);	//����ʹ��
			ChannelLock(&data);			//ͨ������
			//sem_wait(&gSemForCtl);
			
			gCurTime = GetLocalTime();	
		}	
		nCount++;
		sem_wait(&gSemForCtl);
		(*countvalue)++;	
		memcpy(&tmpData,&data,sizeof(data));//ȷ��ÿ���ȡ�����ݲ��ᱻ�޸�
		CountDownOutPut(&tmpData, infos.mode);	//����ʱ���
		SendChannelStatus(&tmpData);
		nCount = ((nCount == 4) ? 0 : nCount);
		
	}
}
