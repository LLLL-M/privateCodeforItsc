#include <stdlib.h>
#include <pthread.h>
#include "hikmsg.h"
#include "lfq.h"
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"

extern void *gHandle;
extern CustomInfo gCustomInfo;

extern UInt8 GetSchemeIdAndTimeGap(struct tm *now, int *ret);
extern void SetCurrentChannelStatus(PhaseInfo *phaseInfos, UInt8 *allChannel);

static Boolean SetStageInfo(void)
{
	UInt8 stageTimingNo = gRunGbconfig->schemeTable[gCalInfo.schemeId - 1].stageTimingNo;
	GbStageTimingList *list = NULL;
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 cycleTime = 0;
	int i, j;
	
	if (stageTimingNo == 0 || stageTimingNo > MAX_STAGE_TIMING_LIST_NUM)
	{
		log_error("the stageTimingNo %d is invalid!", stageTimingNo);
		return FALSE;
	}
	list = gRunGbconfig->stageTimingTable[stageTimingNo - 1];
	for (i = 0; i < MAX_STAGE_NUM; i++)
	{
		if (list[i].phaseNo == 0 || list[i].greenTime == 0)
			break;
		for (j = 0; j < MAX_PHASE_LIST_NUM; j++)
		{
			if (GET_BIT(list[i].phaseNo, j) == 0)
				continue;
			stageInfos[i].greenTime = list[i].greenTime;
			stageInfos[i].includePhases[stageInfos[i].includeNum++] = j + 1;	//���ý׶ΰ�����λ
			gCalInfo.phaseIncludeStage[j][gCalInfo.includeNums[j]++] = i + 1;	//������λ�����׶�
		}
		stageInfos[i].runTime = list[i].greenTime + list[i].yellowTime + list[i].allRedTime;
	}
	gCalInfo.maxStageNum = i;
	//���׶�ʱ���ܺ��Ƿ��뷽������ʱ�����
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		cycleTime += stageInfos[i].runTime;
	}
	if (cycleTime != gCalInfo.cycleTime)
	{
		log_error("The timesum[%d] of stagetimelist %d isn't equle to scheme %d cycleTime %d",  
		cycleTime, stageTimingNo, gCalInfo.schemeId, gCalInfo.cycleTime);
		return FALSE;
	}
	else
		return TRUE;
}
//�ҵ���λ����һ�׶ο�ʼ���´ο�ʼ����Ҫ������ʱ��
static UInt8 FindPhaseNextRunPassTime(UInt8 phaseId, UInt8 curStageNum)
{
	int s = NEXT_STAGE(curStageNum);
	UInt8 ret = 0;

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

static inline void SetInductiveControlInfo(PhaseInfo *info, UInt8 phaseId)
{
	int i, maxVehDetectorNum = GET_PROTOCOL_PARAM(MAX_VEHICLEDETECTOR_COUNT, MAX_VEH_DETECTOR_NUM);
	UInt8 requestPhase = 0;

	if (gCalInfo.schemeId != INDUCTIVE_SCHEMEID)
		return;	//������Ǹ�Ӧ���Ʋ���Ҫ����Ĳ���
	for (i = 0; i < maxVehDetectorNum; i++)
	{	//��ѯ������������Ϣ������λ��Ӧ�ĳ�������
		requestPhase = GET_PROTOCOL_PARAM(gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase, gRunGbconfig->vehDetectorTable[i].requestPhase);
		if (phaseId == requestPhase)
		{
			info->vehicleDetectorId = i + 1;
			break;
		}
	}
	info->unitExtendGreen = GET_PROTOCOL_PARAM(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1].nUnitExtendGreen, gRunGbconfig->phaseTable[phaseId - 1].unitExtendGreen);
	info->maxExtendGreen = gCalInfo.phaseTimes[phaseId - 1].maxExtendGreen;
	/*
	info->maxExtendGreen = GET_PROTOCOL_PARAM(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1].nMaxGreen_1 
												- (info->splitTime 
													- gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1].nYellowTime 
													- gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1].nAllRedTime),
											  gCalInfo.stageInfos[stageNum - 1].runTime - ;
	*/
}

static void SetStagePhaseInfo(UInt8 stageNum, UInt8 stageLeftTime, PhaseInfo *phaseinfos)
{
	int i;
	LightStatus status;
	PhaseTimeInfo *times = NULL;
	UInt8 phaseId = 0;
	
	if (stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return;
	for (i = 0; i < gCalInfo.stageInfos[stageNum - 1].includeNum; i++)
	{
		phaseId = gCalInfo.stageInfos[stageNum - 1].includePhases[i];
		if (phaseId == 0 || phaseId > MAX_PHASE_LIST_NUM)
			break;
		times = &gCalInfo.phaseTimes[phaseId - 1];
		status = GetPhaseStatusByTime(times);
		phaseinfos[phaseId - 1].phaseStatus = status;
		if (status == GREEN || status == GREEN_BLINK)	//�̵Ƶ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = times->greenTime + times->greenBlinkTime + 1;
		else if (status == YELLOW)	//�ƵƵ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = times->yellowTime + 1;
		else	//��Ƶ���ʱ
			phaseinfos[phaseId - 1].phaseLeftTime = times->allRedTime + 1 + FindPhaseNextRunPassTime(phaseId, stageNum);
		phaseinfos[phaseId - 1].splitTime = times->splitTime;
		phaseinfos[phaseId - 1].phaseSplitLeftTime = times->greenTime + times->greenBlinkTime + times->yellowTime + times->allRedTime + 1;
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
	}
	//����δ������λ��״̬�͵���ʱ
	for (i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		if (GET_BIT(gRunGbconfig->phaseTable[i].phaseOption, 0) == 0 
			|| IsPhaseIncludeStage(i + 1, stageNum))
			continue;
		phaseinfos[i].phaseStatus = RED;
		phaseinfos[i].phaseLeftTime = FindPhaseNextRunPassTime(i + 1, stageNum) + stageLeftTime;
		phaseinfos[i].pedestrianPhaseStatus = RED;
		phaseinfos[i].pedestrianPhaseLeftTime = phaseinfos[i].phaseLeftTime;
	}
}

static inline UInt8 GetPhaseEndStageNum(UInt8 phaseId, UInt8 curStageNum)
{
	UInt8 ret = 0;
	UInt8 stageNum;
	
	for (stageNum = NEXT_STAGE(curStageNum); stageNum != curStageNum; stageNum = NEXT_STAGE(stageNum))
	{
		if (IsPhaseIncludeStage(phaseId, stageNum))
			ret = stageNum;
		else
			break;
	}
	if (stageNum == curStageNum)
		return curStageNum;
	else
		return ret;
}

static void __SetPhaseTimes(GbStageTimingList *list, UInt8 phaseIndex)
{
	GbPhaseList *phaseList = &gRunGbconfig->phaseTable[phaseIndex];
	PhaseTimeInfo *times = &gCalInfo.phaseTimes[phaseIndex];
	
	times->splitTime = list->greenTime + list->yellowTime + list->allRedTime;
	if (phaseList->greenBlinkTime > list->greenTime)
	{
		times->greenTime = 0;
		times->greenBlinkTime = list->greenTime;
	}
	else
	{
		times->greenTime = list->greenTime - phaseList->greenBlinkTime;
		times->greenBlinkTime = phaseList->greenBlinkTime;
	}
	times->yellowTime = list->yellowTime;
	times->allRedTime = list->allRedTime;
	times->pedestrianPassTime = phaseList->pedestrianPassTime;
	times->pedestrianClearTime = phaseList->pedestrianClearTime;
}

static void SetStagePhaseTime(GbStageTimingList *stageTable, UInt8 stageNum)
{
	int i, s;
	UInt8 endStageNum = 0;
	UInt8 tmpSplitTime = 0, tmpGreenTime = 0;
	GbStageTimingList *list = &stageTable[stageNum - 1];
	GbPhaseList *phaseTable = gRunGbconfig->phaseTable;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;
	
	for (i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		if (GET_BIT(list->phaseNo, i) == 0
			|| (phaseTimes[i].greenTime + phaseTimes[i].greenBlinkTime + phaseTimes[i].yellowTime + phaseTimes[i].allRedTime > 0))
			continue;
		/*�ж���λ�ں����׶�����ֹ���еĽ׶κţ�������ڴ˽׶���ֹ�򷵻�0��
		��֮���ش���λ���Ժ�Ľ׶����������е�����Ǹ��׶κ�,
		������صĽ׶κ���˽׶κ���ȣ���˵������λһֱ������*/
		endStageNum = GetPhaseEndStageNum(i + 1, stageNum);
		if (endStageNum == stageNum)
		{
			phaseTimes[i].splitTime = gCalInfo.cycleTime;
			phaseTimes[i].greenTime = gCalInfo.cycleTime;
			phaseTimes[i].pedestrianPassTime = gCalInfo.cycleTime;
		}
		else if (endStageNum == 0)
		{
			__SetPhaseTimes(list, i);
		}
		else
		{	//˵������λ��׶�����
			for (s = stageNum; s != endStageNum; s = NEXT_STAGE(s))
			{
				tmpSplitTime += gCalInfo.stageInfos[s - 1].runTime;
				tmpGreenTime += gCalInfo.stageInfos[s - 1].runTime;
			}
			__SetPhaseTimes(&stageTable[endStageNum - 1], i);
			phaseTimes[i].splitTime += tmpSplitTime;
			phaseTimes[i].greenTime += tmpGreenTime;
		}
		SetPedestrianTime(GET_BIT(phaseTable[i].phaseOption, 3), &phaseTimes[i], 
			phaseTable[i].pedestrianPassTime, phaseTable[i].pedestrianClearTime);//����������ص�ʱ��
	}
}

static void GBRunCycle(void)
{
	UInt8 stageTimingNo = gRunGbconfig->schemeTable[gCalInfo.schemeId - 1].stageTimingNo;
	LineQueueData data;
	UInt8 stageNum = 0;		//�׶κ�
	UInt8 stageStartRunTime = 0;    			//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt8 checkTime = gCustomInfo.checkTime;	//��Ӧ���ʱ��
	int t;
	
	if (stageTimingNo == 0 || stageTimingNo > MAX_STAGE_TIMING_LIST_NUM)
	{
		log_error("the stageTimingNo %d is invalid!", stageTimingNo);
		return;
	}
	for (t = 0; t < gCalInfo.cycleTime; t++) 
	{	//һ��ѭ����ȡ1���Ӹ�ͨ����״̬�Լ�������Ϣ���͸���λ����ģ��
		memset(&data, 0, sizeof(data));
		if (t == stageStartRunTime)
		{
			stageNum++;
			stageStartRunTime += gCalInfo.stageInfos[stageNum - 1].runTime;
			SetStagePhaseTime(gRunGbconfig->stageTimingTable[stageTimingNo - 1], stageNum);
		}
		SetStagePhaseInfo(stageNum, stageStartRunTime - t, data.phaseInfos);

		SetFollowPhaseInfo(data.phaseInfos, stageNum);
		SetCurrentChannelStatus(data.phaseInfos, data.allChannels);
		data.cycleTime = gCalInfo.cycleTime;
		//data.runTime = t + 1;
		data.leftTime = data.cycleTime - t;
		data.schemeId = gCalInfo.schemeId;
		data.stageNum = stageNum;
		data.maxStageNum = gCalInfo.maxStageNum;
		data.checkTime = checkTime;
		data.phaseTableId = gCalInfo.phaseTableId;
		data.channelTableId = gCalInfo.channelTableId;
		while (0 != lfq_write(gHandle, &data));	//��֤һ���ܰ�����д�뵽������
	}
	INFO("GB max stage num is %d", stageNum);
}

Boolean CalculateByGbconfig(struct msgbuf *msg)
{
	struct tm now;	
    int timeGapMinutes = 0;	//��ǰʱ���뵱ǰʱ�ε���ʼʱ��Ĳ�ֵ����λΪ����

	localtime_r(&msg->msgCalTime, &now);
	memset(&gCalInfo, 0, sizeof(CalInfo));
    gCalInfo.schemeId = (msg->msgSchemeId > 0) ? msg->msgSchemeId : GetSchemeIdAndTimeGap(&now, &timeGapMinutes);
    if (gCalInfo.schemeId == YELLOWBLINK_SCHEMEID || gCalInfo.schemeId == ALLRED_SCHEMEID || gCalInfo.schemeId == TURNOFF_SCHEMEID)
	{
		log_debug("special control, scheme: %d", gCalInfo.schemeId);
        return TRUE;
	}
	if (gCalInfo.schemeId == 0 || gCalInfo.schemeId > MAX_SCHEME_LIST_NUM)
	{
		log_error("can't find scheme id, timeintervalId: %d", gCalInfo.timeIntervalId);
		return FALSE;
	}
	gCalInfo.cycleTime = gRunGbconfig->schemeTable[gCalInfo.schemeId - 1].cycleTime;
	gCalInfo.coordinatePhaseId = gRunGbconfig->schemeTable[gCalInfo.schemeId - 1].coordinatePhase;
	gCalInfo.phaseOffset = gRunGbconfig->schemeTable[gCalInfo.schemeId - 1].phaseGap;
	SET_BIT(gCalInfo.isCoordinatePhase, gCalInfo.coordinatePhaseId - 1);
	gCalInfo.timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	if (SetStageInfo() == FALSE)
		return FALSE;
	GBRunCycle();
	return TRUE;
}
