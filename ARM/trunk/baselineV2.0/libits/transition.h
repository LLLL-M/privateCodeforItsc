static int CalculateTransitionTime(UInt16 cycleTime, int timeGap, int phaseOffset)
{ 
	static int transCycle = 0;		//过渡周期
	int totalTransitionTime = 0;	//总共需要过渡的时间
	int transitionTime = 0;			//每个周期需要过渡时间
	int excessTime = 0;	//从当前时段起始时间运行到现在的多余时间

	if (gCalInfo.transitionCycle == 0 || cycleTime == 0)	//如果没有配置过渡周期则不进行过渡
		return 0;
	excessTime = (timeGap - phaseOffset) % cycleTime;
	if (excessTime == 0 || excessTime == 1)
		transCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//如果多余时间在5s之内，可以通过递减协调相位时间来达到过渡
		transCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		if (transCycle == 0)
			transCycle = gCalInfo.transitionCycle;
		//周期时间减去多余时间，再加上相位差时间即是总共需要过渡时间
		totalTransitionTime = (excessTime > 0) ? (cycleTime - excessTime) : abs(excessTime);
		if (transCycle == 1 && totalTransitionTime > ((cycleTime * 3) / 4))
			totalTransitionTime = totalTransitionTime - (INT16)cycleTime;
		else if (transCycle > 1 && totalTransitionTime > (cycleTime / 2))
		{
			totalTransitionTime = totalTransitionTime - (INT16)cycleTime;
		}
		//当总共的过渡时间小于等于过渡周期或是小于等于5s时，便在一个周期内完成
		transitionTime = (abs(totalTransitionTime) <= 5) //totalTransitionTime <= transCycle || 
						 ? totalTransitionTime 
						 : totalTransitionTime / transCycle;
		transCycle--;
	}
#if 0
#define MAX_CYCLE_TIME	0xff	//最大周期时间
	if (cycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//如果周期时间加上过渡时间大于最大周期时间0xff,则缩短过渡时间并增加过渡周期
		transitionTime = MAX_CYCLE_TIME - cycleTime;
		transCycle++;
	}
#endif	
	INFO("DEBUG cycleTime=%d, timeGap=%d, phaseOffset=%d, totalTransitionTime=%d, transitionTime=%d", cycleTime, timeGap, phaseOffset, totalTransitionTime, transitionTime);
	return transitionTime;
}

static void AllocateTransitionTime(int transitionTime)
{
	StageInfo *stageInfos = gCalInfo.stageInfos;
	int i = 0, j = 0;
	UINT8 phaseId = 0;
	INT32 stageSumTime = 0;
	INT32 stageallocTime[MAX_STAGE_NUM] = {0};
		
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		stageSumTime += gCalInfo.stageInfos[i].runTime;
	}
	stageallocTime[gCalInfo.maxStageNum - 1] = transitionTime;
	for (i = 0; i < gCalInfo.maxStageNum - 1; i++)
	{
		stageallocTime[i] = (INT32)(((double)gCalInfo.stageInfos[i].runTime / (double)stageSumTime) * transitionTime);
		stageallocTime[gCalInfo.maxStageNum - 1] -= stageallocTime[i];
	}
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		for (j = 0; j < stageInfos[i].includeNum; j++)
		{
			phaseId = stageInfos[i].includePhases[j];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			gCalInfo.phaseTimes[phaseId - 1].splitTime += stageallocTime[i];
			gCalInfo.phaseTimes[phaseId - 1].passTimeInfo.greenTime += stageallocTime[i];
		}
		stageInfos[i].runTime += stageallocTime[i];
		stageInfos[i].passTimeInfo.greenTime += stageallocTime[i];
	}
}

static void TransitionDeal(void)
{
	int phaseOffset = gCalInfo.phaseOffset;	//协调相位差
    int transitionTime = 0;	//每个周期需要过渡的时间
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 phaseBeginStageNum = 0;	//相位起始阶段号
	int s, phaseId, i;
	
	if (gCalInfo.coordinatePhaseId == 0 || gCalInfo.coordinatePhaseId > MAX_PHASE_NUM)
		return;
	phaseBeginStageNum = gCalInfo.phaseIncludeStage[gCalInfo.coordinatePhaseId - 1][0];
	if (phaseBeginStageNum == 0 || phaseBeginStageNum > gCalInfo.maxStageNum)
	{
		ERR("transition: phaseBeginStageNum[%d] is invalid!", phaseBeginStageNum);
		return;
	}
	for (s = 1; s < phaseBeginStageNum; s++)
	{
		phaseOffset -= stageInfos[s - 1].runTime;
	}
	transitionTime = CalculateTransitionTime((gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID) ? gCalInfo.inductiveCoordinateCycleTime : gCalInfo.cycleTime, 
					gCalInfo.timeGapSec, phaseOffset);
    if (transitionTime == 0)
		return;
	if (gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
		gCalInfo.inductiveCoordinateCycleTime += transitionTime;
	gCalInfo.cycleTime += transitionTime;
	AllocateTransitionTime(transitionTime);
	/*
	if (gCalInfo.phaseTimes[gCalInfo.coordinatePhaseId - 1].splitTime == 0)
	{	//说明是按阶段进行配时，则直接延长阶段绿灯时间
		stageInfos[phaseBeginStageNum - 1].runTime += transitionTime;
		stageInfos[phaseBeginStageNum - 1].passTimeInfo.greenTime += transitionTime;
	}
	else
	{	//说明是按相位进行配时，则延长协调相位所在起始阶段中所包含相位的绿灯时间以及绿信比
		for (i = 0; i < stageInfos[phaseBeginStageNum - 1].includeNum; i++)
		{
			phaseId = stageInfos[phaseBeginStageNum - 1].includePhases[i];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			gCalInfo.phaseTimes[phaseId - 1].splitTime += transitionTime;	//增加绿信比时间
			gCalInfo.phaseTimes[phaseId - 1].passTimeInfo.greenTime += transitionTime;	//增加绿灯时间
		}
		stageInfos[phaseBeginStageNum - 1].runTime += transitionTime;
	}	
	*/
}
