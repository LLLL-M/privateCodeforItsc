#include <stdlib.h>
#include <unistd.h>
#include "lfq.h"
#include "its.h"
#include "LogSystem.h"
#include "calculate.h"
#include "hikmsg.h"

static CalInfo gCalInfo;

static inline Boolean IsPhaseIncludeStage(int phaseId, int stageNum)
{
	int i;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	for (i = 0; i < gCalInfo.includeNums[phaseId - 1]; i++)
	{
		if (stageNum == gCalInfo.phaseIncludeStage[phaseId - 1][i])
			return TRUE;
	}
	return FALSE;
}

static inline Boolean IsStageIncludePhase(int stageNum, int phaseId)
{
	int i;
	StageInfo *stageinfo = NULL;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	stageinfo = &gCalInfo.stageInfos[stageNum - 1];
	for (i = 0; i < stageinfo->includeNum; i++)
	{
		if (phaseId == stageinfo->includePhases[i])
			return TRUE;
	}
	return FALSE;
}

#define NEXT_STAGE(_CUR)	((_CUR) % gCalInfo.maxStageNum + 1)

#include "transition.h"		//����Э���̲���صĴ�����
#include "followphase.h"	//���ڸ�����λ��صĴ�����

extern void *gHandle;
extern int msgid;

extern pthread_rwlock_t gConfigLock;

void ItsUnitInit(void (*initFunc)(UInt8, UInt8))
{
	initFunc(0, 0);
}

Boolean FillCalInfo(CalInfo *calInfo, UInt8 schemeId, time_t calTime)
{
	return FALSE;
}

void IgnorePhaseDeal(CalInfo *calInfo)
{
}

static void SendSpecialControlData(UInt8 schemeId, UInt8 leftTime)
{
    LineQueueData data;
    int i;
    LightStatus status = INVALID;

	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: status = YELLOW_BLINK; break;	//��������
		case ALLRED_SCHEMEID: status = ALLRED; break;		//ȫ�����
		case TURNOFF_SCHEMEID: status = TURN_OFF; break;	//�صƿ���
		default: break;
	}
    memset(&data, 0, sizeof(data));
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
		data.allChannels[i] = status;
    }
    data.schemeId = schemeId;
	data.leftTime = leftTime;
    if (0 != lfq_write(gHandle, &data))
		ERR("fill line queue fail");
}

void YellowBlinkAllRedInit(UInt8 bootYellowFlashTime, UInt8 bootAllRedTime)
{
	UInt8 firstCycle = bootYellowFlashTime + bootAllRedTime;
	struct msgbuf msg;
	int i;
	
	//�ѻ�����ȫ����Ϊ��һ�����ڣ������ڵ�ʣ��ʱ��д����Ϣ�б�����λ����ģ��SystemInit����ʹ��
	if (firstCycle == 0)
	{	//���û�����û�����ȫ�������ʱ�������ȷ���1s�Ĺص����ݱ�����λ����ģ��SystemInit����ʹ��
		SendSpecialControlData(TURNOFF_SCHEMEID, 1);
	}
	else
	{
		//��ʼ��Ϊ��������
		for (i = 0; i < bootYellowFlashTime; i++) 	
			SendSpecialControlData(YELLOWBLINK_SCHEMEID, firstCycle--);
		//��ʼ��Ϊȫ�����
		for (i = 0; i < bootAllRedTime; i++) 
			SendSpecialControlData(ALLRED_SCHEMEID, firstCycle--);
	}
	//��֪��λ����ģ�鿪ʼ��ȡ����
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_BEGIN_READ_DATA;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

static inline UInt8 GetPhaseEndStageNum(UInt8 phaseId, UInt8 curStageNum)
{
	UInt8 ret = curStageNum;
	UInt8 s;
	
	if (curStageNum == 0 || curStageNum > gCalInfo.maxStageNum
		|| phaseId == 0 || phaseId > MAX_PHASE_NUM)
		return 0;
	for (s = NEXT_STAGE(curStageNum); s != curStageNum; s = NEXT_STAGE(s))
	{
		if (IsPhaseIncludeStage(phaseId, s))
			ret = s;
		else
			break;
	}
	if (s == curStageNum)
		return 0;
	else
		return ret;
}

static inline void SetPedestrianTime(PhaseTimeInfo *times)
{
	int timeGap = 0;
	UInt8 pedestrianPassTime = times->pedestrianPassTime;
	UInt8 pedestrianClearTime = times->pedestrianClearTime;

	if (times->splitTime == 0)
		return;
/*�����λ������"�����Զ�����"��������˷���ʱ����������ʱ��֮�ʹ��ڻ�������λ���̵ƺ�����ʱ��֮�ͣ���ô���˷���ʱ����������ʱ����Ҫ������������λ���̵ƺ�����ʱ�������� */
	//if (pedestrianClearTime == 0)
		//pedestrianClearTime = 10;	//���û������������գ���Ĭ��10s
	if (times->pedAutoRequestFlag == 0)//���˲��Զ�����
	{
		timeGap = times->passTimeInfo.greenTime + times->passTimeInfo.greenBlinkTime + 
			times->passTimeInfo.yellowTime - pedestrianClearTime;
	}
	else if ((times->pedAutoRequestFlag == 1) 
		|| (times->passTimeInfo.greenTime + times->passTimeInfo.greenBlinkTime < pedestrianPassTime + pedestrianClearTime)) 
	{					
		timeGap = times->passTimeInfo.greenTime + times->passTimeInfo.greenBlinkTime - pedestrianClearTime;
	}
	if (timeGap > 0) 
	{
		times->pedestrianPassTime = (UInt8)timeGap;
		times->pedestrianClearTime = pedestrianClearTime;
	} 
	else 
	{
		times->pedestrianPassTime = 0;
		times->pedestrianClearTime = pedestrianClearTime + timeGap;
	}
	
}

static void SetStagePhaseTime(UInt8 stageNum)
{
	int i, s;
	UInt8 endStageNum = 0;
	UInt16 tmpGreenTime = 0;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;
	PassTimeInfo *passTimeInfo;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		if (IsPhaseIncludeStage(i + 1, stageNum) == FALSE
			|| (passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + passTimeInfo->yellowTime 
				+ passTimeInfo->redYellowTime + passTimeInfo->allRedTime > 0))
			continue;
		/*�ж���λ�ӵ�ǰ�׶ο�ʼ��ֹ���еĽ׶κţ�������صĽ׶κ�Ϊ0����˵������λһֱ������*/
		endStageNum = GetPhaseEndStageNum(i + 1, stageNum);
		if (endStageNum == 0)
		{
			phaseTimes[i].splitTime = gCalInfo.cycleTime;
			passTimeInfo->greenTime = gCalInfo.cycleTime;
			phaseTimes[i].pedestrianPassTime = gCalInfo.cycleTime;
			continue;
		}
		else if (endStageNum == stageNum)
		{
			memcpy(passTimeInfo, &gCalInfo.stageInfos[stageNum - 1].passTimeInfo, sizeof(PassTimeInfo));
			phaseTimes[i].splitTime = gCalInfo.stageInfos[stageNum - 1].runTime;
		}
		else
		{	//˵������λ��׶�����
			tmpGreenTime = 0;
			for (s = stageNum; s != endStageNum; s = NEXT_STAGE(s))
			{
				tmpGreenTime += gCalInfo.stageInfos[s - 1].runTime;
			}
			memcpy(passTimeInfo, &gCalInfo.stageInfos[endStageNum - 1].passTimeInfo, sizeof(PassTimeInfo));
			phaseTimes[i].splitTime = tmpGreenTime + gCalInfo.stageInfos[endStageNum - 1].runTime;
			passTimeInfo->greenTime += tmpGreenTime;
		}
		SetPedestrianTime(&phaseTimes[i]);//����������ص�ʱ��
	}
}

static inline LightStatus GetPhaseStatusByTime(PassTimeInfo *passTimeInfo)
{
    if (passTimeInfo->greenTime)
    {
		passTimeInfo->greenTime--;
		return GREEN;
	}
	if (passTimeInfo->greenBlinkTime)
    {
		passTimeInfo->greenBlinkTime--;
		return GREEN_BLINK;
	}
	if (passTimeInfo->yellowTime)
    {
		passTimeInfo->yellowTime--;
		return YELLOW;
	}
	if (passTimeInfo->redYellowTime)
    {
		passTimeInfo->redYellowTime--;
		return RED_YELLOW;
	}
	if (passTimeInfo->allRedTime)
    {
		passTimeInfo->allRedTime--;
		return ALLRED;
	}
	return RED;
}

//�ҵ���λ����һ�׶ο�ʼ���´ο�ʼ����Ҫ������ʱ��
static UInt16 FindPhaseNextRunPassTime(UInt8 phaseId, UInt8 curStageNum)
{
	int s = NEXT_STAGE(curStageNum);
	UInt16 ret = 0;

	if (curStageNum == 0 || curStageNum > gCalInfo.maxStageNum)
		return 0;
	if (IsStageIncludePhase(curStageNum, phaseId))
	{	//�����ǰ�׶ΰ�����phaseId,�������������еĽ׶�
		for (; s != curStageNum; s = NEXT_STAGE(s))
		{
			if (IsStageIncludePhase(s, phaseId) == FALSE)
				break;
		}
	}
	for (; s != curStageNum; s = NEXT_STAGE(s))
	{
		if (IsStageIncludePhase(s, phaseId))
			break;
		else
			ret += gCalInfo.stageInfos[s - 1].runTime;
	}
	return ret;
}

void SetChannelInfo(LineQueueData *data, UInt32 channelBits, UInt8 status, UInt16 leftTime)
{
	int i;
	
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (GET_BIT(channelBits, i) == 1)
		{
			if (((data->allChannels[i] == status)
					&& (((status == GREEN || status == GREEN_BLINK 
								|| status == YELLOW || status == RED_YELLOW)
							&& data->channelCountdown[i] < leftTime)
						|| ((status == ALLRED || status == RED)
							&& data->channelCountdown[i] > leftTime)))
				|| ((data->allChannels[i] != status)
					&& ((data->allChannels[i] == INVALID || data->allChannels[i] == RED)
						|| (data->allChannels[i] == ALLRED && status != RED)
						|| (data->allChannels[i] == RED_YELLOW && (status == YELLOW
																 || status == GREEN_BLINK
																 || status == GREEN))
						|| (data->allChannels[i] == YELLOW && (status == GREEN_BLINK
																 || status == GREEN))
						|| (data->allChannels[i] == GREEN_BLINK && status == GREEN))))
			{
				data->allChannels[i] = status;
				data->channelCountdown[i] = leftTime;
			}
		}
	}
}

static void SetStagePhaseInfo(UInt8 stageNum, UInt16 stageLeftTime, LineQueueData *data)
{
	int i, s;
	LightStatus status;
	PhaseTimeInfo *times = NULL;
	PassTimeInfo *passTimeInfo = NULL;
	PhaseInfo *phaseinfos = data->phaseInfos;
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 phaseId = 0;
	
	if (stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return;
	for (i = 0; i < gCalInfo.stageInfos[stageNum - 1].includeNum; i++)
	{
		phaseId = gCalInfo.stageInfos[stageNum - 1].includePhases[i];
		if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
			break;
		times = &gCalInfo.phaseTimes[phaseId - 1];
		passTimeInfo = &times->passTimeInfo;
		status = GetPhaseStatusByTime(passTimeInfo);
		phaseinfos[phaseId - 1].phaseStatus = status;
		if (status == GREEN || status == GREEN_BLINK)	//�̵Ƶ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + 1;
		else if (status == YELLOW)	//�ƵƵ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->yellowTime + 1;
		else if (status == RED_YELLOW)	//��Ƶ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->redYellowTime + 1;
		else	//��Ƶ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->allRedTime + 1 + FindPhaseNextRunPassTime(phaseId, stageNum);
		phaseinfos[phaseId - 1].splitTime = times->splitTime;
		phaseinfos[phaseId - 1].phaseSplitLeftTime = passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + passTimeInfo->yellowTime + passTimeInfo->redYellowTime + passTimeInfo->allRedTime + 1;
		if (times->pedestrianPassTime)
		{
			times->pedestrianPassTime--;
			status = GREEN;
		}
		else
		{
			if (times->pedestrianClearTime) 
			{
				times->pedestrianClearTime--;
				status = GREEN_BLINK;
			}
			else
				status = RED;
		}
		phaseinfos[phaseId - 1].pedestrianPhaseStatus = status;
		if (status == GREEN || status == GREEN_BLINK)	//�����̵Ƶ���ʱ
			phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = times->pedestrianPassTime + times->pedestrianClearTime + 1;
		else	//���˺�Ƶ���ʱ
			phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = phaseinfos[phaseId - 1].phaseSplitLeftTime + FindPhaseNextRunPassTime(phaseId, stageNum);
		SetChannelInfo(data, times->motorChannelBits, phaseinfos[phaseId - 1].phaseStatus, phaseinfos[phaseId - 1].phaseLeftTime);
		SetChannelInfo(data, times->pedChannelBits, phaseinfos[phaseId - 1].pedestrianPhaseStatus, phaseinfos[phaseId - 1].pedestrianPhaseLeftTime);
		phaseinfos[phaseId - 1].vehicleDetectorBits = times->vehicleDetectorBits;
		phaseinfos[phaseId - 1].unitExtendGreen = times->unitExtendGreen;
		phaseinfos[phaseId - 1].maxExtendGreen = times->maxExtendGreen;
        phaseinfos[phaseId - 1].maxExtendGreen2 = times->maxExtendGreen2;
        phaseinfos[phaseId - 1].pedChannelBits = times->pedChannelBits;
        phaseinfos[phaseId - 1].motorChannelBits = times->motorChannelBits;
        
//        INFO("%s: phaseid = %d , pedChannelBits = %d",__func__,phaseId,phaseinfos[phaseId - 1].pedChannelBits);
	}
	//����δ������λ��״̬�͵���ʱ
	for (s = NEXT_STAGE(stageNum); s != stageNum; s = NEXT_STAGE(s))
	{
		for (i = 0; i < gCalInfo.stageInfos[s - 1].includeNum; i++)
		{
			phaseId = gCalInfo.stageInfos[s - 1].includePhases[i];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			if (IsPhaseIncludeStage(phaseId, stageNum) == FALSE)
			{	//�����λ���ڵ�ǰ�׶ζ��������׶Σ������ô���λ����Ϣ
				phaseinfos[phaseId - 1].splitTime = gCalInfo.phaseTimes[phaseId - 1].splitTime;
				phaseinfos[phaseId - 1].phaseSplitLeftTime = phaseinfos[phaseId - 1].splitTime;
				phaseinfos[phaseId - 1].phaseStatus = RED;
				phaseinfos[phaseId - 1].phaseLeftTime = FindPhaseNextRunPassTime(phaseId, stageNum) + stageLeftTime;
				phaseinfos[phaseId - 1].pedestrianPhaseStatus = RED;
				phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = phaseinfos[phaseId - 1].phaseLeftTime;
				SetChannelInfo(data, gCalInfo.phaseTimes[phaseId - 1].motorChannelBits, RED, phaseinfos[phaseId - 1].phaseLeftTime);
				SetChannelInfo(data, gCalInfo.phaseTimes[phaseId - 1].pedChannelBits, RED, phaseinfos[phaseId - 1].phaseLeftTime);
			}
		}
	}
}

static void RunCycle()
{
	LineQueueData data;
	UInt8 stageNum = 0;		//�׶κ�
	UInt16 stageStartRunTime = 0;    			//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt16 phaseGreen[MAX_PHASE_NUM] = {0};		//ÿ����λ���̵�������ʱ��֮��
	int i, t;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		SetPedestrianTime(&gCalInfo.phaseTimes[i]);//����������ص�ʱ��
		phaseGreen[i] = gCalInfo.phaseTimes[i].passTimeInfo.greenTime + gCalInfo.phaseTimes[i].passTimeInfo.greenBlinkTime;
	}
	for (t = 0; t < gCalInfo.cycleTime; t++) 
	{	//һ��ѭ����ȡ1���Ӹ�ͨ����״̬�Լ�������Ϣ���͸���λ����ģ��
		memset(&data, 0, sizeof(data));
		if (t == stageStartRunTime)
		{
			if (stageNum == gCalInfo.maxStageNum)
				break;
			stageNum++;
			stageStartRunTime += gCalInfo.stageInfos[stageNum - 1].runTime;
			SetStagePhaseTime(stageNum);
		}
		SetStagePhaseInfo(stageNum, stageStartRunTime - t, &data);
		SetFollowPhaseInfo(&data, stageNum, phaseGreen);
		data.cycleTime = gCalInfo.cycleTime;
		data.inductiveCoordinateCycleTime = gCalInfo.inductiveCoordinateCycleTime;
		//data.runTime = t + 1;
		data.leftTime = data.cycleTime - t;
		data.schemeId = gCalInfo.schemeId;
		data.stageNum = stageNum;
		data.maxStageNum = gCalInfo.maxStageNum;
		data.collectCycle = gCalInfo.collectCycle;
		data.checkTime = gCalInfo.checkTime;
		data.phaseTableId = gCalInfo.phaseTableId;
		data.channelTableId = gCalInfo.channelTableId;
		data.actionId = gCalInfo.actionId;

        data.motorChanType = gCalInfo.motorChanType;
        data.pedChanType = gCalInfo.pedChanType;
        
		while (0 != lfq_write(gHandle, &data))	//��֤һ���ܰ�����д�뵽������
		{
			usleep(500000);
			log_error("fill line queue fail");
		}
	}
	//INFO("max stage num is %d", stageNum);
}


void *CalculateModule(void *arg)
{	
	struct msgbuf msg;
	UInt16 lastCycleSchemeId = 256;	//��һ���ڷ�����
	UInt8 switchSchemeId = 0;	//�л�������
	
	ItsUnitInit(YellowBlinkAllRedInit);
    while (1)
    {
		memset(&msg, 0, sizeof(msg));
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_START_CALCULATE_NEXT_CYCLE, 0) == -1)
		{
			usleep(10000);
			continue;
		}
		//Ϊ��ֹ��λ����ģ���η��ͼ�����һ������Ϣ�������һ�ж�
		if (lfq_element_count(gHandle) > AHEAD_OF_TIME)  
		    continue;
		pthread_rwlock_rdlock(&gConfigLock);
		//���ݵ�ǰʱ���ȡ����������ʼ������λ�����ű�ʱ���
		if (FillCalInfo(&gCalInfo, msg.msgSchemeId, msg.msgCalTime) == TRUE) 
		{	
			if (gCalInfo.isIgnorePhase > 0)	//��ʾ�����ú�����λ
				IgnorePhaseDeal(&gCalInfo);	//�Ժ�����λ���д���
			//ֻ�зǸ�Ӧ��������Ӧ�����ֶ����з���ʱ�ſɽ���Э���̲�
			if (gCalInfo.actionId != INDUCTIVE_ACTIONID	
				&& gCalInfo.actionId != SINGLE_ADAPT_ACTIONID
				&& gCalInfo.schemeId != SINGLE_ADAPT_SCHEMEID
				&& gCalInfo.schemeId != YELLOWBLINK_SCHEMEID
				&& gCalInfo.schemeId != ALLRED_SCHEMEID
				&& gCalInfo.schemeId != TURNOFF_SCHEMEID
				&& (msg.msgSchemeId == 0 || gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
				&& gCalInfo.coordinatePhaseId > 0)
					TransitionDeal();	//��Э����λ���й��ɴ���
			RunCycle();	//ÿ����ѯһ������������
			switchSchemeId = gCalInfo.schemeId;
		}
		else
		{
			switchSchemeId = YELLOWBLINK_SCHEMEID;
		}
		if (lastCycleSchemeId != switchSchemeId)
		{
			log_debug("lastCycleSchemeId=%d, msgschemeid=%d, gSchemeId=%d, cycleTime=%d", lastCycleSchemeId, msg.msgSchemeId, gCalInfo.schemeId, gCalInfo.cycleTime);	
		}
		if (gCalInfo.actionId == INDUCTIVE_ACTIONID)
			ItsCtlNonblock(AUTO_CONTROL, INDUCTIVE_SCHEMEID, switchSchemeId);	//��Ӧ�����л�
		else if (gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
			ItsCtlNonblock(AUTO_CONTROL, INDUCTIVE_COORDINATE_SCHEMEID, switchSchemeId);	//Э����Ӧ�����л�
		else if(gCalInfo.actionId == SINGLE_ADAPT_ACTIONID)
			ItsCtlNonblock(AUTO_CONTROL, SINGLE_ADAPT_SCHEMEID, switchSchemeId);	//����Ӧ���Ʒ����л�
		else
			ItsCtlNonblock(AUTO_CONTROL, switchSchemeId, 0);	//���ط����л�
		lastCycleSchemeId = switchSchemeId;
		pthread_rwlock_unlock(&gConfigLock);
    }
}
