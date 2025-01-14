//步进时读取线性队列数据，只为步进使用
static inline void ReadLineQueueDataForStep(LineQueueData *data)
{
	int left = lfq_element_count(gHandle);
	
	//if (left == 0)	//褰撻槦鍒椾腑娌℃湁鏁版嵁鏃讹紝鍊掑簭涓�涓懆鏈熻鍙栧綋鍓嶅懆鏈熺殑绗�1s浠庡ご寮�濮�
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
    LineQueueData *tmp_block = NULL;
    UInt8 tmp_status = 0;
	
	if (currentStageNum == stageNum && WithinTransitionalPeriod(phaseInfos) == FALSE)
		return TRUE;//如果当前阶段恰好是要跳转的阶段并且不处在过渡期就直接跳转，反之处于过渡期就进行过度处理
	for (t = 0; t < times; t++)
	{	
		ReadLineQueueData(data, data->schemeId);
		//INFO("tansition : curstage=%d,currentnum =%d cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
		if (isCurrentPhaseEnd(data) || currentStageNum != data->stageNum  || data->stageNum == 0)//闃舵鍙樺寲鎴栧懆鏈熸渶鍚庝竴绉�
		{	//INFO("transition over ,curstage=%d currentnum =%d , cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
			return TRUE;	//宸茬粡姝ヨ繘鍒颁笅涓�闃舵锛屾杩涜繃娓″畬鎴�
		}
        
		if (WithinTransitionalPeriod(phaseInfos))
		{
            //不是单步步进并且当前阶段大于等于要步进到的阶段时，
            //往回查找需要步进到的阶段开始时的状态；
            if(stageNum != 0 && currentStageNum >= stageNum)
            {
                i = data->cycleTime - data->leftTime;
//                   INFO("%s: 1 i = %d",__func__,i);
                while(tmp_block == NULL && i > 0) 
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 2 i = %d , tmp_block = %p",__func__,i,tmp_block);
                //找到要步进到的阶段开始时的数据块
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0 && i > 0)
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 3 i = %d",__func__,i);
            }
            //不是单步步进，当前阶段小于要步进道德阶段时，
            //预读查找要步进到的阶段开始时的状态；
            else if(stageNum != 0)
            {
                i = 0;
                tmp_block = lfq_read_prefetch(gHandle,i++);
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0)
                    tmp_block = lfq_read_prefetch(gHandle,i++);
//                INFO("%s: 4 i = %d",__func__,i);
            }

            //查找到了要步进到的阶段开始的数据块；
            if(NULL != tmp_block && tmp_block->stageNum == stageNum && tmp_block->stageNum != 0 && i > 0)
            {
//                INFO("%s: motorChanType = %x, pedChanType = %x",__func__,data->motorChanType,data->pedChanType);
                //遍历所有通道的状态，找到过渡的状态，黄灯优先于全红；
                for (i = 0; i < MAX_CHANNEL_NUM; i++)
                {
                    if (data->allChannels[i] == YELLOW)
                    {
                        tmp_status = YELLOW;
                        break;
                    }
                    else if(data->allChannels[i] == ALLRED)
                    {
                        tmp_status = ALLRED;
                    }
                }
                for (i = 0; i < MAX_CHANNEL_NUM; i++)
                {
                    //如果通道当前的状态时绿灯，步进后状态是红灯，而且该通道是机动车通道，
                    //则该通道需要过渡，使用找到的过渡状态进行过渡；
                    if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->motorChanType,i) == 1)
                        data->allChannels[i] = tmp_status;
                    //如果通道当前是绿灯，步进后是红灯，而且该通道是非机动车通道，
                    //则将该通道改为全红状态进行过渡；
                    else if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->pedChanType,i) == 1)
                        data->allChannels[i] = ALLRED;
                }
            }
			return FALSE;
        }
	}
	return FALSE;
}
