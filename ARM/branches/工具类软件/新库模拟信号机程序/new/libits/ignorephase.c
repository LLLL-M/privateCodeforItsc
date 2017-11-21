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
	{	//找出并发相位时间总和
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
	SetBarrier(i + 2);	//利用递归设置下一个屏障
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
	UInt8 stageStartRunTime = 0;	//阶段开始时已经运行的周期时间
	UInt8 stageEndRunTime = 0;	//阶段结束已经运行的周期时间
	UInt8 stageNO = 0;	//阶段号
	UInt8 phaseEndRunTime = 0;	//每个相序中执行完每个相位运行的时间
	StageInfo *stageInfos = gCalInfo.stageInfos;
	
	memset(stageInfos, 0, sizeof(gCalInfo.stageInfos));
	memset(gCalInfo.includeNums, 0, sizeof(gCalInfo.includeNums));
	memset(gCalInfo.phaseIncludeStage, 0, sizeof(gCalInfo.phaseIncludeStage));
	stageInfos[0].isBarrierStart = TRUE;	//阶段1当然为屏障起始
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
					//记录下此相位所包含的阶段号
					gCalInfo.phaseIncludeStage[nPhaseId - 1][gCalInfo.includeNums[nPhaseId - 1]++] = stageNO;
					//记录下此阶段所包含的相位号
					stageInfos[stageNO - 1].includePhases[stageInfos[stageNO - 1].includeNum++] = nPhaseId;
					break;
				}
			}
		}
		stageInfos[stageNO - 1].runTime = stageEndRunTime - stageStartRunTime;
		stageStartRunTime = stageEndRunTime;
	}
	stageInfos[stageNO - 1].isBarrierEnd = TRUE;	//最后一个阶段当然为屏障结束
	gCalInfo.maxStageNum = stageNO;
	SetBarrier(1);	//从第一阶段开始使用递归操作设置屏障
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
	return (n == s->includeNum) ? TRUE : FALSE;	//如果此阶段所包含的相位全部是忽略相位，则此阶段忽略
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
			{	//如果等于要忽略的阶段号，则把包含的最后一个阶段号清零，并把包含阶段个数减一
				gCalInfo.phaseIncludeStage[i][gCalInfo.includeNums[i] - 1] = 0;
				gCalInfo.includeNums[i]--;
				break;
			}
			else if (gCalInfo.phaseIncludeStage[i][j] > ignoreStageNO)
				gCalInfo.phaseIncludeStage[i][j]--;
			j++;
		}
	}
	//调整忽略阶段所包含相位的时间
	for (i = 0; i < s->includeNum; i++)
	{
		nPhaseId = s->includePhases[i];
		times = &gCalInfo.phaseTimes[nPhaseId - 1];
		times->splitTime -= s->runTime;
		//依次把全红、绿闪、绿灯、黄灯时间缩减作为忽略阶段省略的时间
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
	UInt8 stageNO = PHASE_START_STAGE(ignorePhaseId);	//相位起始所在的阶段
	
	if (stageNO == 0 || gCalInfo.stageInfos[stageNO - 1].isBarrierStart)
		return 0;	//如果忽略相位位于屏障起始，则不用调整
	
	for (i = NUM_PHASE - 1; i >= 0; i--) 
	{	//先找到忽略相位在相序中的索引
		prevPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (prevPhase == 0)
			continue;
		if (prevPhase == ignorePhaseId)
			break;
	}
	for (i = i - 1; i >= 0; i--) 
	{	//倒序寻找忽略相位的前一个相位
		prevPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (GET_BIT(gCalInfo.isIgnorePhase, prevPhase - 1) == TRUE)
		{	//如果前一个相位仍是忽略相位,此相位若处于屏障起始则退出,反之继续寻找
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
	UInt8 stageNO = PHASE_END_STAGE(ignorePhaseId);	//相位结束时所在的阶段
	
	if (stageNO == 0 || gCalInfo.stageInfos[stageNO - 1].isBarrierEnd)
		return 0;	//如果忽略相位位于屏障结束，则不用调整
	for (i = 0; i < NUM_PHASE; i++)
	{	//先找到忽略相位在相序中的索引
		nextPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		else if (nextPhase == ignorePhaseId)
			break;
	}
	for (i = i + 1; i < NUM_PHASE; i++) 
	{	//正序寻找忽略相位的后一个相位
		nextPhase = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nextPhase == 0)
			return 0;
		if (GET_BIT(gCalInfo.isIgnorePhase, nextPhase - 1) == TRUE)
		{	//如果后一个相位仍是忽略相位,此相位若处于屏障结束则退出,反之继续寻找
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
	//先处理忽略阶段
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
			stageInfos[i + 1].isBarrierStart = TRUE;	//下一阶段即为屏障起始
		if (stageInfos[i].isBarrierEnd && i - 1 >= 0)
			stageInfos[i - 1].isBarrierEnd = TRUE;		//上一阶段即为屏障结束
		//把后面的阶段信息覆盖此忽略阶段
		memmove(&stageInfos[i], &stageInfos[i + 1], (gCalInfo.maxStageNum - stageNO) * sizeof(StageInfo));
		gCalInfo.maxStageNum--;
	}
	//接着处理忽略相位的时间
	if (gCustomInfo.ignoreOption == NONE_IGNORE)
		return;	//不予处理
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (GET_BIT(gCalInfo.isIgnorePhase, i) == FALSE || gCalInfo.phaseTimes[i].splitTime == 0)
			continue;
		ignorePhase = i + 1;
		prevPhase = nextPhase = 0;
		if (gCustomInfo.ignoreOption == FORWARD_IGNORE)	
			prevPhase = FindPrevPhase(ignorePhase);//找寻前一个可增加忽略相位时间的相位
		else if (gCustomInfo.ignoreOption == BACKWARD_IGNORE)
			nextPhase = FindNextPhase(ignorePhase);//找寻后一个可增加忽略相位时间的相位
		else if (gCustomInfo.ignoreOption == ALL_IGNORE)
		{
			prevPhase = FindPrevPhase(ignorePhase);//找寻前一个可增加忽略相位时间的相位
			nextPhase = FindNextPhase(ignorePhase);//找寻后一个可增加忽略相位时间的相位
		}
		AdjustIgnorePhaseTime(prevPhase, nextPhase, ignorePhase);
	}
	//PrintStageInfo();
	CalStageInfo();	//对忽略相位处理之后再次计算一次各阶段信息
}

void ItsSetIgnoreAttr(IgnoreAttr attr)
{
	gCustomInfo.ignoreOption = attr;
}
