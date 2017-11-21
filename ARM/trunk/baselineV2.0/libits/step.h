//����ʱ��ȡ���Զ������ݣ�ֻΪ����ʹ��
static inline void ReadLineQueueDataForStep(LineQueueData *data)
{
	int left = lfq_element_count(gHandle);
	
	//if (left == 0)	//当队列中没有数据时，倒序一个周期读取当前周期的第1s从头开始
		//lfq_read_inverted(gHandle, data, data->cycleTime);
	//else
	if (left > 0)
		lfq_read(gHandle, data);	//��ȡһ��Ԫ��
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

//�ж��Ƿ��ڹ�����
static inline Boolean WithinTransitionalPeriod(PhaseInfo *phaseInfos)
{
	int i;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{	//���������е���λ���������������Ƶơ�ȫ�졢�ص�ʱ����ʱ������Ч
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
//�жϲ����Ƿ���Ч
static inline Boolean IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	if ((data->stageNum == 0)    //������ǰ����ִ�л�����ȫ����ǹصƿ���
	//	|| (data->stageNum == stageNum)	//������ǰ�������е�������Ҫ�����Ľ׶�
		|| (stageNum > data->maxStageNum))	//������Ҫ�����Ľ׶κŴ���ϵͳ���Ľ׶κ�
	{
		log_error("excute step invalid, stageNum = %d, data->stageNum = %d, maxStageNum = %d", stageNum, data->stageNum, data->maxStageNum);
		return TRUE;	//������Щ������в���������Ч��
	}
	if (stageNum == 0)
	{	//�������������ڹ�����ִ��
		if (WithinTransitionalPeriod(phaseInfos))
		{
			log_error("excute step(stageNum = %d) invalid, because of within transitional period!", stageNum);
			return TRUE;
		}
	}
	return FALSE;
}

//ֱ�Ӳ�����ĳ���׶Σ��м䲻�����������Ƶƺ�ȫ��Ĺ���
static UINT8 DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
	int i, t = data->cycleTime;
	UINT8 stepOk = 0;
	
	if (data->stageNum == 0)
		return 1;
	if (stageNum > 0 && data->stageNum != stageNum)
	{	//�������׶κ�ָ���Ľ׶�
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
	{	//��ʱ���е���λ״̬��ӦΪGREEN
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
	{	//�����ʱ�������е�ͨ�����������Ƶơ�ȫ�죬�������ǲ�������������Щ״̬��ӦΪGREEN
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

//�жϲ��������Ƿ����
static Boolean IsStepTransitionComplete(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i, t, times = data->leftTime;
	UInt8 currentStageNum = data->stageNum;
    LineQueueData *tmp_block = NULL;
    UInt8 tmp_status = 0;
	
	if (currentStageNum == stageNum && WithinTransitionalPeriod(phaseInfos) == FALSE)
		return TRUE;//�����ǰ�׶�ǡ����Ҫ��ת�Ľ׶β��Ҳ����ڹ����ھ�ֱ����ת����֮���ڹ����ھͽ��й��ȴ���
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
            //���ǵ����������ҵ�ǰ�׶δ��ڵ���Ҫ�������Ľ׶�ʱ��
            //���ز�����Ҫ�������Ľ׶ο�ʼʱ��״̬��
            if(stageNum != 0 && currentStageNum >= stageNum)
            {
                i = data->cycleTime - data->leftTime;
//                   INFO("%s: 1 i = %d",__func__,i);
                while(tmp_block == NULL && i > 0) 
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 2 i = %d , tmp_block = %p",__func__,i,tmp_block);
                //�ҵ�Ҫ�������Ľ׶ο�ʼʱ�����ݿ�
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0 && i > 0)
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 3 i = %d",__func__,i);
            }
            //���ǵ�����������ǰ�׶�С��Ҫ�������½׶�ʱ��
            //Ԥ������Ҫ�������Ľ׶ο�ʼʱ��״̬��
            else if(stageNum != 0)
            {
                i = 0;
                tmp_block = lfq_read_prefetch(gHandle,i++);
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0)
                    tmp_block = lfq_read_prefetch(gHandle,i++);
//                INFO("%s: 4 i = %d",__func__,i);
            }

            //���ҵ���Ҫ�������Ľ׶ο�ʼ�����ݿ飻
            if(NULL != tmp_block && tmp_block->stageNum == stageNum && tmp_block->stageNum != 0 && i > 0)
            {
//                INFO("%s: motorChanType = %x, pedChanType = %x",__func__,data->motorChanType,data->pedChanType);
                //��������ͨ����״̬���ҵ����ɵ�״̬���Ƶ�������ȫ�죻
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
                    //���ͨ����ǰ��״̬ʱ�̵ƣ�������״̬�Ǻ�ƣ����Ҹ�ͨ���ǻ�����ͨ����
                    //���ͨ����Ҫ���ɣ�ʹ���ҵ��Ĺ���״̬���й��ɣ�
                    if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->motorChanType,i) == 1)
                        data->allChannels[i] = tmp_status;
                    //���ͨ����ǰ���̵ƣ��������Ǻ�ƣ����Ҹ�ͨ���Ƿǻ�����ͨ����
                    //�򽫸�ͨ����Ϊȫ��״̬���й��ɣ�
                    else if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->pedChanType,i) == 1)
                        data->allChannels[i] = ALLRED;
                }
            }
			return FALSE;
        }
	}
	return FALSE;
}
