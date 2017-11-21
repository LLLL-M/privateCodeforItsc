//步进时读取线性队列数据，只为步进使用
static inline void ReadLineQueueDataForStep(LineQueueData *data)
{
	int left = lfq_element_count(gHandle);
	
	//if (left == 0)	//当队列中没有数据时，倒序一个周期读取当前周期的第1s从头开始
		//lfq_read_inverted(gHandle, data, data->cycleTime);
	//else
	if (left > 0)
		lfq_read(gHandle, data);	//读取一个元素
}

static UINT8 StepToStage(LineQueueData *data, UInt8 stageNum)
{
	int i, t = data->cycleTime;
	int left = 0;
	struct msgbuf msg;
	LineQueueData dt;
	LineQueueData* pdt = NULL;
	if (data->stageNum == 0)//stagenum invalid, don't step phase.
		return 1;
	memcpy(&dt, data, sizeof(LineQueueData));
	pdt = lfq_read_prefetch(gHandle, 0);
	if (pdt != NULL)
		memcpy(&dt, pdt, sizeof(LineQueueData));
	while (stageNum > 0 && dt.stageNum != stageNum)
	{
		
		left = lfq_element_count(gHandle);
		if (left == 0)
			break;
		lfq_read(gHandle, &dt);
		//if (left == 0)	
			//lfq_read_inverted(gHandle, data, data->cycleTime);
		if (left <= AHEAD_OF_TIME)
		{
		    memset(&msg, 0, sizeof(msg));
		    msg.mtype = MSG_START_CALCULATE_NEXT_CYCLE;
	    	msg.msgSchemeId = dt.schemeId;
			msg.msgCalTime = gCurTime + left;
	    	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
		}
			
		if (stageNum > dt.maxStageNum)
			break;
		pdt = lfq_read_prefetch(gHandle, 0);
		if (pdt != NULL)
			memcpy(&dt, pdt, sizeof(LineQueueData));
	}
	if (dt.stageNum == stageNum || stageNum > dt.maxStageNum )
	{
		memcpy(data, &dt, sizeof(LineQueueData));
		return 1;
	}
	else
		return 0;
}

//判断是否处于过渡期
static inline Boolean WithinTransitionalPeriod(PhaseInfo *phaseInfos)
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
			return TRUE;
	}
	return FALSE;
}
//判断步进是否无效
static inline Boolean IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	if ((data->stageNum == 0)    //表明当前正在执行黄闪、全红或是关灯控制
	//	|| (data->stageNum == stageNum)	//表明当前正在运行的正是需要步进的阶段
		|| (stageNum > data->maxStageNum))	//表明需要步进的阶段号大于系统最大的阶段号
	{
		log_error("excute step invalid, stageNum = %d, data->stageNum = %d, maxStageNum = %d", stageNum, data->stageNum, data->maxStageNum);
		return TRUE;	//以上这些情况进行步进都是无效的
	}
	if (stageNum == 0)
	{	//单步步进不能在过渡期执行
		if (WithinTransitionalPeriod(phaseInfos))
		{
			log_error("excute step(stageNum = %d) invalid, because of within transitional period!", stageNum);
			return TRUE;
		}
	}
	return FALSE;
}

//直接步进到某个阶段，中间不会有绿闪、黄灯和全红的过渡
static UINT8 DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
	int i, t = data->cycleTime;
	UINT8 stepOk = 0;
	
	if (data->stageNum == 0)
		return 1;
	if (stageNum > 0 && data->stageNum != stageNum)
	{	//步进到阶段号指定的阶段
		//ReadLineQueueDataForStep(data);
		stepOk = StepToStage(data, stageNum);
		if (stepOk == 0 || (data->stageNum == 0) || (--t) < 0)
		    return 0;
	}
	else if (stageNum == 0)//single step
	{
		if (lfq_read(gHandle, data) != 0)
			return 0;
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
	return 1;
}

//current phase end 1s
static UINT8 isCurrentPhaseEnd(LineQueueData *data)
{
	int i = 0;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 currentStageNum = data->stageNum;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseSplitLeftTime == 1)
		{
			//INFO("current phase =%d ", i + 1);
			return 1;
		}
	}
	return 0;
}

//判断步进过渡是否完成
static Boolean IsStepTransitionComplete(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i, t, times = data->leftTime;
	UInt8 currentStageNum = data->stageNum;
	
	if (currentStageNum == stageNum && WithinTransitionalPeriod(phaseInfos) == FALSE)
		return TRUE;//如果当前阶段恰好是要跳转的阶段并且不处在过渡期就直接跳转，反之处于过渡期就进行过度处理
	for (t = 0; t < times; t++)
	{	
		ReadLineQueueData(data, data->schemeId);
		//INFO("tansition : curstage=%d,currentnum =%d cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
		if (isCurrentPhaseEnd(data) || currentStageNum != data->stageNum  || data->stageNum == 0)//阶段变化或周期最后一秒
		{	//INFO("transition over ,curstage=%d currentnum =%d , cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
			return TRUE;	//已经步进到下一阶段，步进过渡完成
		}
		if (WithinTransitionalPeriod(phaseInfos))
		{
			//INFO("in transition period...");
			return FALSE;
		}
		
	}
	return FALSE;
}
