#include <stdlib.h>
#include "hikmsg.h"
#include "its.h"
#include "calculate.h"

extern CustomInfo gCustomInfo;

static void SetBarrier(UInt8 stageNO)
{
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 nPhaseId;
	PhaseItem *item;
	int i, timesum = 0;

	if (stageNO > gCalInfo.maxStageNum)
		return;
	stageInfos[stageNO - 1].isBarrierStart = TRUE;
	if (stageNO == gCalInfo.maxStageNum)
	{
		stageInfos[stageNO - 1].isBarrierEnd = TRUE;
		return;
	}

	nPhaseId = stageInfos[stageNO - 1].includePhases[0];
	item = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][nPhaseId - 1];
	for (i = 0; i < NUM_PHASE; i++)
	{	//�ҳ�������λʱ���ܺ�
		nPhaseId = item->byPhaseConcurrency[i];
		if (nPhaseId == 0)
			break;
		timesum += gCalInfo.phaseTimes[nPhaseId - 1].splitTime;
	}
	for (i = stageNO - 1; i < gCalInfo.maxStageNum; i++)
	{
		timesum -= stageInfos[i].runTime;
		if (timesum <= 0)
			break;
	}
	stageInfos[i].isBarrierEnd = TRUE;
	SetBarrier(i + 2);	//���õݹ�������һ������
}

UNUSEDATTR static void PrintStageInfo(void)
{
	int i, ring;
	StageInfo *stageInfos = gCalInfo.stageInfos;
	INFO("maxStageNum = %d, cycleTime = %d", gCalInfo.maxStageNum, gCalInfo.cycleTime);
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		fprintf(stderr, "stage%d: ", i + 1);
		for (ring = 0; ring < stageInfos[i].includeNum; ring++)
			fprintf(stderr, "%d ", stageInfos[i].includePhases[ring]);
		fprintf(stderr, " runTime = %d, isBarrierStart:%d, isBarrierEnd:%d\n", stageInfos[i].runTime, stageInfos[i].isBarrierStart, stageInfos[i].isBarrierEnd);
	}
	fprintf(stderr, "ignore phase: ");
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (GET_BIT(gCalInfo.isIgnorePhase, i) == TRUE)
			fprintf(stderr, "%d ", i + 1);
	}
	fputc('\n', stderr);
}

void CalStageInfo(void)
{
	int i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt8 stageStartRunTime = 0;	//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt8 stageEndRunTime = 0;	//�׶ν����Ѿ����е�����ʱ��
	UInt8 stageNO = 0;	//�׶κ�
	UInt8 phaseEndRunTime = 0;	//ÿ��������ִ����ÿ����λ���е�ʱ��
	StageInfo *stageInfos = gCalInfo.stageInfos;
	
	memset(stageInfos, 0, sizeof(gCalInfo.stageInfos));
	memset(gCalInfo.includeNums, 0, sizeof(gCalInfo.includeNums));
	memset(gCalInfo.phaseIncludeStage, 0, sizeof(gCalInfo.phaseIncludeStage));
	stageInfos[0].isBarrierStart = TRUE;	//�׶�1��ȻΪ������ʼ
	while (stageStartRunTime < gCalInfo.cycleTime) 
	{	
		stageNO++;
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	
			phaseEndRunTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring].nTurnArray[i];
				if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
					break;
				phaseEndRunTime += gCalInfo.phaseTimes[nPhaseId - 1].splitTime;
				if (phaseEndRunTime > stageStartRunTime)
				{
					if (stageEndRunTime == stageStartRunTime)
						stageEndRunTime = phaseEndRunTime;
					else
						stageEndRunTime = min(stageEndRunTime, phaseEndRunTime);
					//��¼�´���λ�������Ľ׶κ�
					gCalInfo.phaseIncludeStage[nPhaseId - 1][gCalInfo.includeNums[nPhaseId - 1]++] = stageNO;
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
	gCalInfo.maxStageNum = stageNO;
	SetBarrier(1);	//�ӵ�һ�׶ο�ʼʹ�õݹ������������
	//PrintStageInfo();
}

static inline Boolean IsStageIgnore(StageInfo *s)
{
	int i, n = 0;
	for (i = 0; i < s->includeNum; i++)
	{
		if (GET_BIT(gCalInfo.isIgnorePhase, s->includePhases[i] - 1) == TRUE)
			n++;
	}
	return (n == s->includeNum) ? TRUE : FALSE;	//����˽׶�����������λȫ���Ǻ�����λ����˽׶κ���
}

static void AdjustPhaseIncludeStage(UInt8 ignoreStageNO)
{
	int i, j;
	StageInfo *s = &gCalInfo.stageInfos[ignoreStageNO - 1];
	PhaseTimeInfo *times = NULL;
	UInt8 nPhaseId = 0;
	int runTime = s->runTime;
	
	for (i = 0; i < NUM_PHASE; i++)
	{
		j = 0;
		while (j < gCalInfo.includeNums[i])
		{
			if (gCalInfo.phaseIncludeStage[i][j] == ignoreStageNO)
			{	//�������Ҫ���ԵĽ׶κţ���Ѱ��������һ���׶κ����㣬���Ѱ����׶θ�����һ
				gCalInfo.phaseIncludeStage[i][gCalInfo.includeNums[i] - 1] = 0;
				gCalInfo.includeNums[i]--;
				break;
			}
			else if (gCalInfo.phaseIncludeStage[i][j] > ignoreStageNO)
				gCalInfo.phaseIncludeStage[i][j]--;
			j++;
		}
	}
	//�������Խ׶���������λ��ʱ��
	for (i = 0; i < s->includeNum; i++)
	{
		nPhaseId = s->includePhases[i];
		times = &gCalInfo.phaseTimes[nPhaseId - 1];
		times->splitTime -= s->runTime;
		//���ΰ�ȫ�졢�������̵ơ��Ƶ�ʱ��������Ϊ���Խ׶�ʡ�Ե�ʱ��
		runTime -= times->allRedTime;
		if (runTime > 0)
		{
			times->allRedTime = 0;
			runTime -= times->greenBlinkTime;
			if (runTime > 0)
			{
				times->greenBlinkTime = 0;
				runTime -= times->greenTime;
				if (runTime > 0)
				{
					times->greenTime = 0;
					times->yellowTime -= runTime;
				}
				else
					times->greenTime = abs(runTime);
			}
			else
				times->greenBlinkTime = abs(runTime);
		}
		else
			times->allRedTime = abs(runTime);
	}
	gCalInfo.cycleTime -= s->runTime;
	INFO("ignoreStageNO = %d, cycleTime = %d", ignoreStageNO, gCalInfo.cycleTime);
}

#define PHASE_START_STAGE(P)	gCalInfo.phaseIncludeStage[P - 1][0]
static UInt8 FindPrevPhase(UInt8 ignorePhaseId)
{
	int i, index, ring = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][ignorePhaseId - 1].nCircleID;
	UInt8 prevPhase = 0;
	UInt8 stageNO = PHASE_START_STAGE(ignorePhaseId);	//��λ��ʼ���ڵĽ׶�
	
	if (stageNO == 0 || gCalInfo.stageInfos[stageNO - 1].isBarrierStart)
		return 0;	//���������λλ��������ʼ�����õ���
	
	for (i = NUM_PHASE - 1; i >= 0; i--) 
	{	//���ҵ�������λ�������е�����
		prevPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (prevPhase == 0)
			continue;
		if (prevPhase == ignorePhaseId)
			break;
	}
	for (i = i - 1; i >= 0; i--) 
	{	//����Ѱ�Һ�����λ��ǰһ����λ
		prevPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (GET_BIT(gCalInfo.isIgnorePhase, prevPhase - 1) == TRUE)
		{	//���ǰһ����λ���Ǻ�����λ,����λ������������ʼ���˳�,��֮����Ѱ��
			stageNO = PHASE_START_STAGE(prevPhase);
			if (gCalInfo.stageInfos[stageNO - 1].isBarrierStart)
				return 0;
		}
		else
			return prevPhase;
	}
	return 0;
}

#define PHASE_END_STAGE(P)	gCalInfo.phaseIncludeStage[P - 1][gCalInfo.includeNums[P - 1] - 1]
static UInt8 FindNextPhase(UInt8 ignorePhaseId)
{
	int i, index, ring = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][ignorePhaseId - 1].nCircleID;
	UInt8 nextPhase = 0;
	UInt8 stageNO = PHASE_END_STAGE(ignorePhaseId);	//��λ����ʱ���ڵĽ׶�
	
	if (stageNO == 0 || gCalInfo.stageInfos[stageNO - 1].isBarrierEnd)
		return 0;	//���������λλ�����Ͻ��������õ���
	for (i = 0; i < NUM_PHASE; i++)
	{	//���ҵ�������λ�������е�����
		nextPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		else if (nextPhase == ignorePhaseId)
			break;
	}
	for (i = i + 1; i < NUM_PHASE; i++) 
	{	//����Ѱ�Һ�����λ�ĺ�һ����λ
		nextPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		if (GET_BIT(gCalInfo.isIgnorePhase, nextPhase - 1) == TRUE)
		{	//�����һ����λ���Ǻ�����λ,����λ���������Ͻ������˳�,��֮����Ѱ��
			stageNO = PHASE_END_STAGE(nextPhase);
			if (gCalInfo.stageInfos[stageNO - 1].isBarrierEnd)
				return 0;
		}
		else
			return nextPhase;
	}
	return 0;
}

static inline void AdjustIgnorePhaseTime(UInt8 prevPhase, UInt8 nextPhase, UInt8 ignorePhase)
{
	PhaseTimeInfo *times = NULL;
	UInt8 ignoreTime = gCalInfo.phaseTimes[ignorePhase - 1].splitTime;

	INFO("prevPhase = %d, nextPhase = %d", prevPhase, nextPhase);
	if (prevPhase == 0 && nextPhase == 0)
		return;
	else if (prevPhase > 0 && nextPhase == 0)
	{
		times = &gCalInfo.phaseTimes[prevPhase - 1];
		times->splitTime += ignoreTime;
		times->greenTime += ignoreTime;
	}
	else if (prevPhase == 0 && nextPhase > 0)
	{
		times = &gCalInfo.phaseTimes[nextPhase - 1];
		times->splitTime += ignoreTime;
		times->greenTime += ignoreTime;
	}
	else if (prevPhase > 0 && nextPhase > 0)
	{
		times = &gCalInfo.phaseTimes[prevPhase - 1];
		times->splitTime += ignoreTime / 2;
		times->greenTime += ignoreTime / 2;
		ignoreTime -= ignoreTime / 2;
		times = &gCalInfo.phaseTimes[nextPhase - 1];
		times->splitTime += ignoreTime;
		times->greenTime += ignoreTime;
	}
	memset(&gCalInfo.phaseTimes[ignorePhase - 1], 0, sizeof(PhaseTimeInfo));
}

void IgnorePhaseDeal(void)
{
	int i = 0;
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 stageNO = 0;
	UInt8 prevPhase = 0, nextPhase = 0, ignorePhase = 0;
	//�ȴ�����Խ׶�
	while (i < gCalInfo.maxStageNum)
	{
		if (IsStageIgnore(&stageInfos[i]) == FALSE)
		{
			i++;
			continue;
		}
		stageNO = i + 1;
		AdjustPhaseIncludeStage(stageNO);
		if (stageInfos[i].isBarrierStart && i + 1 < gCalInfo.maxStageNum)
			stageInfos[i + 1].isBarrierStart = TRUE;	//��һ�׶μ�Ϊ������ʼ
		if (stageInfos[i].isBarrierEnd && i - 1 >= 0)
			stageInfos[i - 1].isBarrierEnd = TRUE;		//��һ�׶μ�Ϊ���Ͻ���
		//�Ѻ���Ľ׶���Ϣ���Ǵ˺��Խ׶�
		memmove(&stageInfos[i], &stageInfos[i + 1], (gCalInfo.maxStageNum - stageNO) * sizeof(StageInfo));
		gCalInfo.maxStageNum--;
	}
	//���Ŵ��������λ��ʱ��
	if (gCustomInfo.ignoreOption == NONE_IGNORE)
		return;	//���账��
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (GET_BIT(gCalInfo.isIgnorePhase, i) == FALSE || gCalInfo.phaseTimes[i].splitTime == 0)
			continue;
		ignorePhase = i + 1;
		prevPhase = nextPhase = 0;
		if (gCustomInfo.ignoreOption == FORWARD_IGNORE)	
			prevPhase = FindPrevPhase(ignorePhase);//��Ѱǰһ�������Ӻ�����λʱ�����λ
		else if (gCustomInfo.ignoreOption == BACKWARD_IGNORE)
			nextPhase = FindNextPhase(ignorePhase);//��Ѱ��һ�������Ӻ�����λʱ�����λ
		else if (gCustomInfo.ignoreOption == ALL_IGNORE)
		{
			prevPhase = FindPrevPhase(ignorePhase);//��Ѱǰһ�������Ӻ�����λʱ�����λ
			nextPhase = FindNextPhase(ignorePhase);//��Ѱ��һ�������Ӻ�����λʱ�����λ
		}
		AdjustIgnorePhaseTime(prevPhase, nextPhase, ignorePhase);
	}
	//PrintStageInfo();
	CalStageInfo();	//�Ժ�����λ����֮���ٴμ���һ�θ��׶���Ϣ
}

void ItsSetIgnoreAttr(IgnoreAttr attr)
{
	gCustomInfo.ignoreOption = attr;
}
