inline bool Phasecontrol::StepControl::WithinTransitionalPeriod(PhaseInfo *phaseInfos)
{
    int i;
    for (i = 0; i < MAX_PHASE_NUM; i++)
    {	//当正在运行的相位有绿闪、黄闪、黄灯、全红、关灯时，此时步进无效
        if (phaseInfos[i].phaseStatus == GREEN_BLINK
                || phaseInfos[i].phaseStatus == YELLOW_BLINK
                || phaseInfos[i].phaseStatus == YELLOW
                || phaseInfos[i].phaseStatus == ALLRED
                || phaseInfos[i].phaseStatus == RED_YELLOW
                || phaseInfos[i].phaseStatus == TURN_OFF)
            return true;
    }
    return false;
}

bool Phasecontrol::StepControl::IsStepTransitionComplete(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    int t, time = data->leftTime;
	UInt8 currentStageNum = data->stageNum;
	
    for (t = 0; t < time; t++)
	{	
        pc.ReadLineQueueDataForStep(data);
		if (currentStageNum != data->stageNum || data->stageNum == 0)
			return true;	//已经步进到下一阶段，步进完成
        if (WithinTransitionalPeriod(phaseInfos))
            return false;
	}
    return false;
}

void Phasecontrol::StepControl::DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
    int i, t = data->cycleTime;
	
    if (data->stageNum == 0)
		return;
    while (stageNum > 0 && data->stageNum != stageNum)
	{	//步进到阶段号指定的阶段 
        pc.ReadLineQueueDataForStep(data);

        if (data->stageNum == 0 || (--t) < 0)
		    return;

	}
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{	//此时放行的相位状态都应为GREEN
        if (phaseInfos[i].followPhaseStatus != INVALID && phaseInfos[i].followPhaseStatus != RED)
	    {
            phaseInfos[i].followPhaseStatus = GREEN;
	        //phaseInfos[i].followPhaseLeftTime = 0;
	    }
		if (phaseInfos[i].phaseStatus != INVALID && phaseInfos[i].phaseStatus != RED)
		{
			phaseInfos[i].phaseStatus = GREEN;
			//phaseInfos[i].phaseLeftTime = 0;
	        //phaseInfos[i].phaseSplitLeftTime= 0;
	        //phaseInfos[i].splitTime = 0;
			if (phaseInfos[i].pedestrianPhaseStatus != INVALID)
    	    {
    	        phaseInfos[i].pedestrianPhaseStatus = GREEN;
    	        //phaseInfos[i].pedestrianPhaseLeftTime = 0;
    	    }
		}
		
	}
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{	//如果此时正在运行的通道有绿闪、黄灯、全红，但由于是步进控制所以这些状态都应为GREEN
		if (allChannels[i] == GREEN_BLINK
			|| allChannels[i] == YELLOW
			|| allChannels[i] == ALLRED)
		{
			allChannels[i] = GREEN;
		}
	}
}

inline void Phasecontrol::StepControl::Deal(LineQueueData *data)
{
	if ((pc.stepflag == STEP_UNUSED_FLAG && pc.stageNum == 0) 
		|| data->stageNum == 0
		|| pc.stageNum == data->stageNum)
	{   //单步步进开始、过渡完成或是跳转步进阶段号与当前阶段号一样时，停留在当前阶段
		return;
	}
	if (IsStepTransitionComplete(data) == true)
	{	//步进过渡完成
		DirectStepToStage(data, pc.stageNum);
		pc.stepflag = STEP_UNUSED_FLAG;
	}				
	//INFO("stageNum = %d, current stageNum = %d, maxStageNum = %d", pc.stageNum, data->stageNum, data->maxStageNum);
}

inline bool Phasecontrol::StepControl::IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;

	if ((data->stageNum == 0)    //表明当前正在执行黄闪、全红或是关灯控制
	//	|| (data->stageNum == stageNum)	//表明当前正在运行的正是需要步进的阶段
		|| (stageNum > data->maxStageNum))	//表明需要步进的阶段号大于系统最大的阶段号
	    return true;	//以上这些情况进行步进都是无效的
    if (stageNum == 0)
    {	//单步步进不能在过渡期执行
        if (WithinTransitionalPeriod(phaseInfos))
        {
            return true;
        }
    }
	return false;
}
