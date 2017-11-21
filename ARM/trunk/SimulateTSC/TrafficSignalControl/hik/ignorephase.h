#include <stdlib.h>
#include <algorithm>
#include "its.h"
//#include "calculate.h"
#include "tsc.h"
#include "hiktsc.h"

void Hiktsc::SetBarrier(CalInfo *calInfo, UInt8 stageNO)
{
	StageInfo *stageInfos = calInfo->stageInfos;
	UInt8 nPhaseId;
	PhaseItem *item;
	int i, timesum = 0;

	if (stageNO > calInfo->maxStageNum)
		return;
	stageInfos[stageNO - 1].isBarrierStart = TRUE;
	if (stageNO == calInfo->maxStageNum)
	{
		stageInfos[stageNO - 1].isBarrierEnd = TRUE;
		return;
	}

	nPhaseId = stageInfos[stageNO - 1].includePhases[0];
	item = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
	for (i = 0; i < NUM_PHASE; i++)
	{	//�ҳ�������λʱ���ܺ�
		nPhaseId = item->byPhaseConcurrency[i];
		if (nPhaseId == 0)
			break;
		timesum += calInfo->phaseTimes[nPhaseId - 1].splitTime;
	}
	for (i = stageNO - 1; i < calInfo->maxStageNum; i++)
	{
		timesum -= stageInfos[i].runTime;
		if (timesum <= 0)
			break;
	}
	stageInfos[i].isBarrierEnd = TRUE;
	SetBarrier(calInfo, i + 2);	//���õݹ�������һ������
}
/*
void Hiktsc::PrintStageInfo(CalInfo *calInfo)
{
	int i, ring;
	StageInfo *stageInfos = calInfo->stageInfos;
	INFO("maxStageNum = %d, cycleTime = %d", calInfo->maxStageNum, calInfo->cycleTime);
	for (i = 0; i < calInfo->maxStageNum; i++)
	{
		fprintf(stderr, "stage%d: ", i + 1);
		for (ring = 0; ring < stageInfos[i].includeNum; ring++)
			fprintf(stderr, "%d ", stageInfos[i].includePhases[ring]);
		fprintf(stderr, " runTime = %d, isBarrierStart:%d, isBarrierEnd:%d\n", stageInfos[i].runTime, stageInfos[i].isBarrierStart, stageInfos[i].isBarrierEnd);
	}
	fprintf(stderr, "ignore phase: ");
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (GET_BIT(calInfo->isIgnorePhase, i) == TRUE)
			fprintf(stderr, "%d ", i + 1);
	}
	fputc('\n', stderr);
}
*/
void Hiktsc::CalStageInfo(CalInfo *calInfo)
{
	int i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt16 stageStartRunTime = 0;	//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt16 stageEndRunTime = 0;	//�׶ν����Ѿ����е�����ʱ��
	UInt8 stageNO = 0;	//�׶κ�
	UInt16 phaseEndRunTime = 0;	//ÿ��������ִ����ÿ����λ���е�ʱ��
	StageInfo *stageInfos = calInfo->stageInfos;
	
	memset(stageInfos, 0, sizeof(calInfo->stageInfos));
	memset(calInfo->includeNums, 0, sizeof(calInfo->includeNums));
	memset(calInfo->phaseIncludeStage, 0, sizeof(calInfo->phaseIncludeStage));
	stageInfos[0].isBarrierStart = TRUE;	//�׶�1��ȻΪ������ʼ
	while (stageStartRunTime < calInfo->cycleTime) 
	{	
		stageNO++;
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	
			phaseEndRunTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
				if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
					break;
				phaseEndRunTime += calInfo->phaseTimes[nPhaseId - 1].splitTime;
				if (phaseEndRunTime > stageStartRunTime)
				{
					if (stageEndRunTime == stageStartRunTime)
						stageEndRunTime = phaseEndRunTime;
					else
                        stageEndRunTime = std::min(stageEndRunTime, phaseEndRunTime);
					//��¼�´���λ�������Ľ׶κ�
					calInfo->phaseIncludeStage[nPhaseId - 1][calInfo->includeNums[nPhaseId - 1]++] = stageNO;
					//��¼�´˽׶�����������λ��
					stageInfos[stageNO - 1].includePhases[stageInfos[stageNO - 1].includeNum++] = nPhaseId;
					break;
				}
			}
		}
		stageInfos[stageNO - 1].runTime = stageEndRunTime - stageStartRunTime;
		stageStartRunTime = stageEndRunTime;
	}
	stageInfos[stageNO - 1].isBarrierEnd = TRUE;	//���һ���׶ε�ȻΪ���Ͻ���
	calInfo->maxStageNum = stageNO;
	SetBarrier(calInfo, 1);	//�ӵ�һ�׶ο�ʼʹ�õݹ������������
	//PrintStageInfo(calInfo);
}

inline Boolean Hiktsc::IsStageIgnore(CalInfo *calInfo, StageInfo *s)
{
	int i, n = 0;
	for (i = 0; i < s->includeNum; i++)
	{
		if (GET_BIT(calInfo->isIgnorePhase, s->includePhases[i] - 1) == TRUE)
			n++;
	}
	return (n == s->includeNum) ? TRUE : FALSE;	//����˽׶�����������λȫ���Ǻ�����λ����˽׶κ���
}

void Hiktsc::AdjustPhaseIncludeStage(CalInfo *calInfo, UInt8 ignoreStageNO)
{
	int i, j;
	StageInfo *s = &calInfo->stageInfos[ignoreStageNO - 1];
	PhaseTimeInfo *times = NULL;
	UInt8 nPhaseId = 0;
	int runTime = s->runTime;
	
	for (i = 0; i < NUM_PHASE; i++)
	{
		j = 0;
		while (j < calInfo->includeNums[i])
		{
			if (calInfo->phaseIncludeStage[i][j] == ignoreStageNO)
			{	//�������Ҫ���ԵĽ׶κţ���Ѱ��������һ���׶κ����㣬���Ѱ����׶θ�����һ
				calInfo->phaseIncludeStage[i][calInfo->includeNums[i] - 1] = 0;
				calInfo->includeNums[i]--;
				break;
			}
			else if (calInfo->phaseIncludeStage[i][j] > ignoreStageNO)
				calInfo->phaseIncludeStage[i][j]--;
			j++;
		}
	}
	//�������Խ׶���������λ��ʱ��
	for (i = 0; i < s->includeNum; i++)
	{
		nPhaseId = s->includePhases[i];
		times = &calInfo->phaseTimes[nPhaseId - 1];
		times->splitTime -= s->runTime;
		//���ΰ�ȫ�졢�������̵ơ��Ƶ�ʱ��������Ϊ���Խ׶�ʡ�Ե�ʱ��
		runTime -= times->passTimeInfo.allRedTime;
		if (runTime > 0)
		{
			times->passTimeInfo.allRedTime = 0;
			runTime -= times->passTimeInfo.greenBlinkTime;
			if (runTime > 0)
			{
				times->passTimeInfo.greenBlinkTime = 0;
				runTime -= times->passTimeInfo.greenTime;
				if (runTime > 0)
				{
					times->passTimeInfo.greenTime = 0;
					times->passTimeInfo.yellowTime -= runTime;
				}
				else
					times->passTimeInfo.greenTime = abs(runTime);
			}
			else
				times->passTimeInfo.greenBlinkTime = abs(runTime);
		}
		else
			times->passTimeInfo.allRedTime = abs(runTime);
	}
	calInfo->cycleTime -= s->runTime;
	INFO("ignoreStageNO = %d, cycleTime = %d", ignoreStageNO, calInfo->cycleTime);
}

#define PHASE_START_STAGE(P)	calInfo->phaseIncludeStage[P - 1][0]
UInt8 Hiktsc::FindPrevPhase(CalInfo *calInfo, UInt8 ignorePhaseId)
{
    int i, ring = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][ignorePhaseId - 1].nCircleID;
	UInt8 prevPhase = 0;
	UInt8 stageNO = PHASE_START_STAGE(ignorePhaseId);	//��λ��ʼ���ڵĽ׶�
	
	if (stageNO == 0 || calInfo->stageInfos[stageNO - 1].isBarrierStart)
		return 0;	//���������λλ��������ʼ�����õ���
	
	for (i = NUM_PHASE - 1; i >= 0; i--) 
	{	//���ҵ�������λ�������е�����
		prevPhase = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (prevPhase == 0)
			continue;
		if (prevPhase == ignorePhaseId)
			break;
	}
	for (i = i - 1; i >= 0; i--) 
	{	//����Ѱ�Һ�����λ��ǰһ����λ
		prevPhase = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (GET_BIT(calInfo->isIgnorePhase, prevPhase - 1) == TRUE)
		{	//���ǰһ����λ���Ǻ�����λ,����λ������������ʼ���˳�,��֮����Ѱ��
			stageNO = PHASE_START_STAGE(prevPhase);
			if (calInfo->stageInfos[stageNO - 1].isBarrierStart)
				return 0;
		}
		else
			return prevPhase;
	}
	return 0;
}

#define PHASE_END_STAGE(P)	calInfo->phaseIncludeStage[P - 1][calInfo->includeNums[P - 1] - 1]
UInt8 Hiktsc::FindNextPhase(CalInfo *calInfo, UInt8 ignorePhaseId)
{
    int i, ring = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][ignorePhaseId - 1].nCircleID;
	UInt8 nextPhase = 0;
	UInt8 stageNO = PHASE_END_STAGE(ignorePhaseId);	//��λ����ʱ���ڵĽ׶�
	
	if (stageNO == 0 || calInfo->stageInfos[stageNO - 1].isBarrierEnd)
		return 0;	//���������λλ�����Ͻ��������õ���
	for (i = 0; i < NUM_PHASE; i++)
	{	//���ҵ�������λ�������е�����
		nextPhase = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		else if (nextPhase == ignorePhaseId)
			break;
	}
	for (i = i + 1; i < NUM_PHASE; i++) 
	{	//����Ѱ�Һ�����λ�ĺ�һ����λ
		nextPhase = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		if (GET_BIT(calInfo->isIgnorePhase, nextPhase - 1) == TRUE)
		{	//�����һ����λ���Ǻ�����λ,����λ���������Ͻ������˳�,��֮����Ѱ��
			stageNO = PHASE_END_STAGE(nextPhase);
			if (calInfo->stageInfos[stageNO - 1].isBarrierEnd)
				return 0;
		}
		else
			return nextPhase;
	}
	return 0;
}

inline void Hiktsc::AdjustIgnorePhaseTime(CalInfo *calInfo, UInt8 prevPhase, UInt8 nextPhase, UInt8 ignorePhase)
{
	PhaseTimeInfo *times = NULL;
	UInt16 ignoreTime = calInfo->phaseTimes[ignorePhase - 1].splitTime;

	INFO("prevPhase = %d, nextPhase = %d", prevPhase, nextPhase);
	if (prevPhase == 0 && nextPhase == 0)
		return;
	else if (prevPhase > 0 && nextPhase == 0)
	{
		times = &calInfo->phaseTimes[prevPhase - 1];
		times->splitTime += ignoreTime;
		times->passTimeInfo.greenTime += ignoreTime;
	}
	else if (prevPhase == 0 && nextPhase > 0)
	{
		times = &calInfo->phaseTimes[nextPhase - 1];
		times->splitTime += ignoreTime;
		times->passTimeInfo.greenTime += ignoreTime;
	}
	else if (prevPhase > 0 && nextPhase > 0)
	{
		times = &calInfo->phaseTimes[prevPhase - 1];
		times->splitTime += ignoreTime / 2;
		times->passTimeInfo.greenTime += ignoreTime / 2;
		ignoreTime -= ignoreTime / 2;
		times = &calInfo->phaseTimes[nextPhase - 1];
		times->splitTime += ignoreTime;
		times->passTimeInfo.greenTime += ignoreTime;
	}
	memset(&calInfo->phaseTimes[ignorePhase - 1], 0, sizeof(PhaseTimeInfo));
}

void Hiktsc::IgnorePhaseDeal(CalInfo *calInfo)
{
	int i = 0;
	StageInfo *stageInfos = calInfo->stageInfos;
	UInt8 stageNO = 0;
	UInt8 prevPhase = 0, nextPhase = 0, ignorePhase = 0;
	//�ȴ�����Խ׶�
	while (i < calInfo->maxStageNum)
	{
		if (IsStageIgnore(calInfo, &stageInfos[i]) == FALSE)
		{
			i++;
			continue;
		}
		stageNO = i + 1;
		AdjustPhaseIncludeStage(calInfo, stageNO);
		if (stageInfos[i].isBarrierStart && i + 1 < calInfo->maxStageNum)
			stageInfos[i + 1].isBarrierStart = TRUE;	//��һ�׶μ�Ϊ������ʼ
		if (stageInfos[i].isBarrierEnd && i - 1 >= 0)
			stageInfos[i - 1].isBarrierEnd = TRUE;		//��һ�׶μ�Ϊ���Ͻ���
		//�Ѻ���Ľ׶���Ϣ���Ǵ˺��Խ׶�
		memmove(&stageInfos[i], &stageInfos[i + 1], (calInfo->maxStageNum - stageNO) * sizeof(StageInfo));
		calInfo->maxStageNum--;
	}
	//���Ŵ��������λ��ʱ��
	if (gIgnoreOption == NONE_IGNORE)
		return;	//���账��
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (GET_BIT(calInfo->isIgnorePhase, i) == FALSE || calInfo->phaseTimes[i].splitTime == 0)
			continue;
		ignorePhase = i + 1;
		prevPhase = nextPhase = 0;
		if (gIgnoreOption == FORWARD_IGNORE)	
			prevPhase = FindPrevPhase(calInfo, ignorePhase);//��Ѱǰһ�������Ӻ�����λʱ�����λ
		else if (gIgnoreOption == BACKWARD_IGNORE)
			nextPhase = FindNextPhase(calInfo, ignorePhase);//��Ѱ��һ�������Ӻ�����λʱ�����λ
		else if (gIgnoreOption == ALL_IGNORE)
		{
			prevPhase = FindPrevPhase(calInfo, ignorePhase);//��Ѱǰһ�������Ӻ�����λʱ�����λ
			nextPhase = FindNextPhase(calInfo, ignorePhase);//��Ѱ��һ�������Ӻ�����λʱ�����λ
		}
		AdjustIgnorePhaseTime(calInfo, prevPhase, nextPhase, ignorePhase);
	}
	//PrintStageInfo(calInfo);
	CalStageInfo(calInfo);	//�Ժ�����λ����֮���ٴμ���һ�θ��׶���Ϣ
}

