#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include "hikmsg.h"
#include "lfq.h"
#include "LogSystem.h"

typedef enum
{
	STEP_UNUSED_FLAG = 100,       //����δʹ�ñ�־
	STEP_TRANSITION_FLAG = 111,   //�������ɱ�־
	STEP_EXCUTE_FLAG = 222,		//����ִ�б�־
}STEP_FLAG;

typedef struct controlInfos
{
	ControlMode mode;	//��ǰ���Ʒ�ʽ
	UInt8 flag;	//��������ʱʹ��
	UInt16 extendTime;	//��Ӧ����ʱʹ��
	UInt16 cycleLeftTime;	//��ӦЭ������ʱʣ�������ʱ��
	UInt8 totalExtendTime[MAX_PHASE_NUM];	//��Ÿ�Ӧ����ʱ��λ�Ѿ��ӳ���ʱ���
	UInt8 stageNum;	//�����׶κ�
	UInt8 schemeId;	//���Ʒ�����
	LineQueueData cycleLeftData;	//��ӦЭ������ʱ�������ʣ��ʱ�����ʱ����
} ControlInfos;

//extern SignalControllerPara *gRunConfigPara;

extern int gOftenPrintFlag; //��ӡ��־
extern void *gHandle; //���Զ��о��
extern int msgid;
extern sem_t gSemForCtl;
extern sem_t gSemForVeh;
extern sem_t gSemForChan;	//������ͨ������ģ�鷢�Ͷ�ʱ1s���ź�

static volatile time_t gCurTime;		//��ǰʱ��
static CustomParams gCustomParams = {
	.isContinueRun = FALSE,	//Ĭ�ϻ������������֮�󲻽���֮ǰ�����ڼ�������
	.addTimeToFirstPhase = FALSE,	//Ĭ�ϸ�ӦЭ������ʱ��ʣ��ʱ��ӵ����һ����λ
};
static inline void ReadLineQueueData(LineQueueData *data, UInt8 schemeId);
SpecialCarControl gSpecialCarControl;

#include "step.h"
#include "inductive.h"

void ItsIsContinueRun(Boolean val)
{
	gCustomParams.isContinueRun = val;
}


//���û�����ȫ�졢�ص�ʱ��������Ϣ
static inline void SetSpericalData(LineQueueData *data, LightStatus status)
{
	int i;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 channelFlashStatus[MAX_CHANNEL_NUM] = {0};	//ͨ������״̬

	if (gCustomParams.isContinueRun == FALSE)
		lfq_reinit(gHandle);	//������Զ����е����ݿ飬���ٽ���֮ǰ�ķ�����������
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
{	//�˺���ר�������ſ�ƽ̨�������������Ҫ�ڿ�������ʵ�ִ˺���
	time_t timeNow, timeStart, timeEnd;    
    struct tm start, end;
	
	if (gChannelLockParams.ucChannelLockStatus == 1 && gChannelLockParams.ucWorkingTimeFlag == 1)
	{	//ͨ����ʱ������
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
		{	//��ǰʱ�䲻������ʱ�䷶Χ��Ӧ�ý��н���
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

//��ȡ���Զ����е�1s������Ϣ
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
	lfq_read(gHandle, data);	//��ȡһ��Ԫ��

	
    //�����ʼ�Ӷ�����ȡ��һ�����ڵ����ݿ飬������һ������������Ӧ���ơ�
    if(data->cycleTime == data->leftTime && data->schemeId == SINGLE_ADAPT_SCHEMEID)
    {
		ExtendMaxGreenTime(data);	//���ӳ����ڵ�1s���ݿ�������
		for (i = 0; i < data->cycleTime - 1; i++)	//��ȥ1����Ϊ���ڵ�1s�Ѿ���ȡ��
		{	//����Ԥ��ȡ������ʣ������ݿ飬���ӳ���Ӧ��λ�������
			block = (LineQueueData *)lfq_read_prefetch(gHandle, i);
			ExtendMaxGreenTime(block);
        }
    }
}

//�����������
UInt64 ItsReadVehicleDetectorData()
{
	return 0;
}

static void RecvControlModeMsg(LineQueueData *data, ControlInfos *infos)
{
	struct msgbuf msg;

	if (msgrcv(msgid, &msg, MSGSIZE, MSG_CONTROL_MODE, IPC_NOWAIT) == -1)
		return;
	//���յ��µĿ��Ʒ�ʽ
	if (msg.msgMode == STEP_MODE)
	{
		if (!IsStepInvalid(data, msg.msgStageNum))
		{	//������Ч
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
		{	//���֮ǰ�Ȳ��Ǹ�ӦҲ����Э����Ӧ,������մ���̵��ӳ�ʱ��͵������Լ�������
			memset(infos->totalExtendTime, 0, sizeof(infos->totalExtendTime));
			gVehicleData = 0;
		}
		infos->mode = msg.msgMode;
		infos->flag = STEP_UNUSED_FLAG;
		infos->schemeId = msg.msgmSchemeId;	
		log_debug("PhasecontrolModule recv new control mode = %d, schemeId = %d", infos->mode, infos->schemeId);
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
	} while (data->leftTime != 1); //ѭ��������ʾ����ʱ�Ļ�����ȫ���Ѿ�������ϣ�����Ҫ���տ��Ʒ�ʽ��ʼ������
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
			if (infos->schemeId == 0)	//˵����ʱ����ʱ�λ���
				ReadLineQueueData(data, 0);	//��ȡ������Ϣ���ڱ���ʱ���л�
			data->schemeId = YELLOWBLINK_SCHEMEID;
			SetSpericalData(data, YELLOW_BLINK);
			break;
		case TURNOFF_LIGHTS_MODE: 
			if (infos->schemeId == 0)	//˵����ʱ����ʱ�ιص�
				ReadLineQueueData(data, 0);	//��ȡ������Ϣ���ڱ���ʱ���л�
			data->schemeId = TURNOFF_SCHEMEID;
			SetSpericalData(data, TURN_OFF);
			break;
		case ALLRED_MODE: 
			if (infos->schemeId == 0)	//˵����ʱ����ʱ��ȫ��
				ReadLineQueueData(data, 0);	//��ȡ������Ϣ���ڱ���ʱ���л�
			data->schemeId = ALLRED_SCHEMEID;
			SetSpericalData(data, ALLRED);
			break;
		case STEP_MODE:
			if ((infos->flag == STEP_UNUSED_FLAG && infos->stageNum == 0) 
				|| data->stageNum == 0) //����������ʼ,ͣ���ڵ�ǰ�׶�
				break;
		/*	if (infos->flag == STEP_TRANSITION_FLAG)
			{
				INFO("Transition ing...");
				if (IsStepTransitionComplete(data, infos->stageNum))
				{INFO("step falg STEP_TRANSITION_FLAG >>>> STEP_EXCUTE_FLAG");
				infos->flag = STEP_EXCUTE_FLAG;}
				INFO("after transition infos->flag = %X", infos->flag);
			}
			//�����������
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
			if (infos->extendTime > 0)	//˵����ǰ���ڸ�Ӧ�ӳ�����Ҫ����һ��ʱ��
			{
				infos->extendTime--;
				ReduceInductiveExtendTime(data);
			}
			else if (stepInductive.stepNow != 2)
			{	//SendCalculateNextCycleMsg(data, infos->schemeId);
				ReadLineQueueData(data, infos->schemeId);
			}
			else if (stepInductive.stepNow == 2)
			{//���Ӧפ����ÿ��һ�����ж�ʱ�α����л�
				if (tmp++ > data->cycleTime)
				{
					SendCalculateNextCycleMsg(data, infos->schemeId);
					tmp = 0;
				}
			}
			break;
		 case SINGLE_ADAPT_MODE:
			if (infos->extendTime > 0)	//˵����ǰ���ڸ�Ӧ�ӳ�����Ҫ����һ��ʱ��
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
			if (infos->extendTime > 0)	//˵����ǰ���ڸ�Ӧ�ӳ�����Ҫ����һ��ʱ��
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
			{//���˹���ͣ���ڻ�����������λ��ÿ��һ�����ж�һ��ʱ�α�ķ����л�
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
	while (1)	//����һ��ѭ����ʱ��Ϊ1s
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
		{	//��Ӧģʽ��ʵʱ����������,�ж��Ƿ��ڴ��������й�����������Ҫ�ӳ����̵�ʱ��
			infos.extendTime += ret;	//ret>0�����ڴ������ڣ�����Ҫ�ӳ��̵�ʱ��
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
		DataDeal(&data, &infos);	//���ݴ���

		if (infos.mode == PEDESTRIAN_REQ_MODE)//�����ڵ����˹���
		{
			if (data.maxStageNum == 4)//four phase, pedestrian twice req 
				PedestrianReqTwiceDeal(&data);
			else if (data.maxStageNum == 2)//two phase ,pedestrian once req
				PedestrianReqOnceDeal(&data);
		}
		}
		ItsCustom(&data, &gCustomParams);	//����ʹ��
		sem_wait(&gSemForCtl);
		(*countvalue)++;
		
		data.isStep = (infos.mode == STEP_MODE) ? TRUE : FALSE;
		ItsSetCurRunData(&data);
		sem_post(&gSemForChan);
	}
}

