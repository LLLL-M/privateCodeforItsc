int Calculate::CalculateTransitionTime(UInt16 cycleTime, int timeGap, int phaseOffset)
{ 
	static int transCycle = 0;		//过渡周期
	int totalTransitionTime = 0;	//总共需要过渡的时间
	int transitionTime = 0;			//每个周期需要过渡时间
	int excessTime = 0;	//从当前时段起始时间运行到现在的多余时间

	if (calInfo.transitionCycle == 0 || cycleTime == 0)	//如果没有配置过渡周期则不进行过渡
		return 0;
	excessTime = timeGap % cycleTime - phaseOffset % cycleTime;
	if (excessTime == 0 || excessTime == 1)
		transCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//如果多余时间在5s之内，可以通过递减协调相位时间来达到过渡
		transCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		//周期时间减去多余时间，再加上相位差时间即是总共需要过渡时间
		totalTransitionTime = (excessTime > 0) ? (cycleTime - excessTime) : abs(excessTime);
		if (transCycle == 0)
			transCycle = calInfo.transitionCycle;
		//当总共的过渡时间小于等于过渡周期或是小于等于5s时，便在一个周期内完成
		transitionTime = (totalTransitionTime <= transCycle || totalTransitionTime <= 5) 
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
	//INFO("DEBUG cycleTime=%d, timeGap=%d, phaseOffset=%d, totalTransitionTime=%d, transitionTime=%d", cycleTime, timeGap, phaseOffset, totalTransitionTime, transitionTime);
	return transitionTime;
}

void Calculate::TransitionDeal()
{
	int phaseOffset = calInfo.phaseOffset;	//协调相位差
    int transitionTime = 0;	//每个周期需要过渡的时间
	StageInfo *stageInfos = calInfo.stageInfos;
	UInt8 phaseBeginStageNum = 0;	//相位起始阶段号
	int s, phaseId, i;
	
	if (calInfo.coordinatePhaseId == 0 || calInfo.coordinatePhaseId > MAX_PHASE_NUM)
		return;
	phaseBeginStageNum = calInfo.phaseIncludeStage[calInfo.coordinatePhaseId - 1][0];
	if (phaseBeginStageNum == 0 || phaseBeginStageNum > calInfo.maxStageNum)
	{
		ERR("transition: phaseBeginStageNum[%d] is invalid!", phaseBeginStageNum);
		return;
	}
	for (s = 1; s < phaseBeginStageNum; s++)
	{
		phaseOffset += stageInfos[s - 1].runTime;
	}
	transitionTime = CalculateTransitionTime((calInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID) ? calInfo.inductiveCoordinateCycleTime : calInfo.cycleTime, 
					calInfo.timeGapSec, phaseOffset);
    if (transitionTime == 0)
		return;
	if (calInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
		calInfo.inductiveCoordinateCycleTime += transitionTime;
	calInfo.cycleTime += transitionTime;
	if (calInfo.phaseTimes[calInfo.coordinatePhaseId - 1].splitTime == 0)
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
			calInfo.phaseTimes[phaseId - 1].splitTime += transitionTime;	//增加绿信比时间
			calInfo.phaseTimes[phaseId - 1].passTimeInfo.greenTime += transitionTime;	//增加绿灯时间
		}
		stageInfos[phaseBeginStageNum - 1].runTime += transitionTime;
	}	
}
