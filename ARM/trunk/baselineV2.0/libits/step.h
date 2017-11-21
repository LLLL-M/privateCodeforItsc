//²½½øÊ±¶ÁÈ¡ÏßĞÔ¶ÓÁĞÊı¾İ£¬Ö»Îª²½½øÊ¹ÓÃ
static inline void ReadLineQueueDataForStep(LineQueueData *data)
{
	int left = lfq_element_count(gHandle);
	
	//if (left == 0)	//å½“é˜Ÿåˆ—ä¸­æ²¡æœ‰æ•°æ®æ—¶ï¼Œå€’åºä¸€ä¸ªå‘¨æœŸè¯»å–å½“å‰å‘¨æœŸçš„ç¬¬1sä»å¤´å¼€å§‹
		//lfq_read_inverted(gHandle, data, data->cycleTime);
	//else
	if (left > 0)
		lfq_read(gHandle, data);	//¶ÁÈ¡Ò»¸öÔªËØ
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

//ÅĞ¶ÏÊÇ·ñ´¦ÓÚ¹ı¶ÉÆÚ
static inline Boolean WithinTransitionalPeriod(PhaseInfo *phaseInfos)
{
	int i;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{	//µ±ÕıÔÚÔËĞĞµÄÏàÎ»ÓĞÂÌÉÁ¡¢»ÆÉÁ¡¢»ÆµÆ¡¢È«ºì¡¢¹ØµÆÊ±£¬´ËÊ±²½½øÎŞĞ§
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
//ÅĞ¶Ï²½½øÊÇ·ñÎŞĞ§
static inline Boolean IsStepInvalid(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	if ((data->stageNum == 0)    //±íÃ÷µ±Ç°ÕıÔÚÖ´ĞĞ»ÆÉÁ¡¢È«ºì»òÊÇ¹ØµÆ¿ØÖÆ
	//	|| (data->stageNum == stageNum)	//±íÃ÷µ±Ç°ÕıÔÚÔËĞĞµÄÕıÊÇĞèÒª²½½øµÄ½×¶Î
		|| (stageNum > data->maxStageNum))	//±íÃ÷ĞèÒª²½½øµÄ½×¶ÎºÅ´óÓÚÏµÍ³×î´óµÄ½×¶ÎºÅ
	{
		log_error("excute step invalid, stageNum = %d, data->stageNum = %d, maxStageNum = %d", stageNum, data->stageNum, data->maxStageNum);
		return TRUE;	//ÒÔÉÏÕâĞ©Çé¿ö½øĞĞ²½½ø¶¼ÊÇÎŞĞ§µÄ
	}
	if (stageNum == 0)
	{	//µ¥²½²½½ø²»ÄÜÔÚ¹ı¶ÉÆÚÖ´ĞĞ
		if (WithinTransitionalPeriod(phaseInfos))
		{
			log_error("excute step(stageNum = %d) invalid, because of within transitional period!", stageNum);
			return TRUE;
		}
	}
	return FALSE;
}

//Ö±½Ó²½½øµ½Ä³¸ö½×¶Î£¬ÖĞ¼ä²»»áÓĞÂÌÉÁ¡¢»ÆµÆºÍÈ«ºìµÄ¹ı¶É
static UINT8 DirectStepToStage(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 *allChannels = data->allChannels;
	int i, t = data->cycleTime;
	UINT8 stepOk = 0;
	
	if (data->stageNum == 0)
		return 1;
	if (stageNum > 0 && data->stageNum != stageNum)
	{	//²½½øµ½½×¶ÎºÅÖ¸¶¨µÄ½×¶Î
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
	{	//´ËÊ±·ÅĞĞµÄÏàÎ»×´Ì¬¶¼Ó¦ÎªGREEN
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
	{	//Èç¹û´ËÊ±ÕıÔÚÔËĞĞµÄÍ¨µÀÓĞÂÌÉÁ¡¢»ÆµÆ¡¢È«ºì£¬µ«ÓÉÓÚÊÇ²½½ø¿ØÖÆËùÒÔÕâĞ©×´Ì¬¶¼Ó¦ÎªGREEN
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

//ÅĞ¶Ï²½½ø¹ı¶ÉÊÇ·ñÍê³É
static Boolean IsStepTransitionComplete(LineQueueData *data, UInt8 stageNum)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i, t, times = data->leftTime;
	UInt8 currentStageNum = data->stageNum;
    LineQueueData *tmp_block = NULL;
    UInt8 tmp_status = 0;
	
	if (currentStageNum == stageNum && WithinTransitionalPeriod(phaseInfos) == FALSE)
		return TRUE;//Èç¹ûµ±Ç°½×¶ÎÇ¡ºÃÊÇÒªÌø×ªµÄ½×¶Î²¢ÇÒ²»´¦ÔÚ¹ı¶ÉÆÚ¾ÍÖ±½ÓÌø×ª£¬·´Ö®´¦ÓÚ¹ı¶ÉÆÚ¾Í½øĞĞ¹ı¶È´¦Àí
	for (t = 0; t < times; t++)
	{	
		ReadLineQueueData(data, data->schemeId);
		//INFO("tansition : curstage=%d,currentnum =%d cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
		if (isCurrentPhaseEnd(data) || currentStageNum != data->stageNum  || data->stageNum == 0)//é˜¶æ®µå˜åŒ–æˆ–å‘¨æœŸæœ€åä¸€ç§’
		{	//INFO("transition over ,curstage=%d currentnum =%d , cycle lefttime=%d", data->stageNum, currentStageNum, data->leftTime);
			return TRUE;	//å·²ç»æ­¥è¿›åˆ°ä¸‹ä¸€é˜¶æ®µï¼Œæ­¥è¿›è¿‡æ¸¡å®Œæˆ
		}
        
		if (WithinTransitionalPeriod(phaseInfos))
		{
            //²»ÊÇµ¥²½²½½ø²¢ÇÒµ±Ç°½×¶Î´óÓÚµÈÓÚÒª²½½øµ½µÄ½×¶ÎÊ±£¬
            //Íù»Ø²éÕÒĞèÒª²½½øµ½µÄ½×¶Î¿ªÊ¼Ê±µÄ×´Ì¬£»
            if(stageNum != 0 && currentStageNum >= stageNum)
            {
                i = data->cycleTime - data->leftTime;
//                   INFO("%s: 1 i = %d",__func__,i);
                while(tmp_block == NULL && i > 0) 
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 2 i = %d , tmp_block = %p",__func__,i,tmp_block);
                //ÕÒµ½Òª²½½øµ½µÄ½×¶Î¿ªÊ¼Ê±µÄÊı¾İ¿é
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0 && i > 0)
                    tmp_block = lfq_read_back(gHandle,i--);
//                INFO("%s: 3 i = %d",__func__,i);
            }
            //²»ÊÇµ¥²½²½½ø£¬µ±Ç°½×¶ÎĞ¡ÓÚÒª²½½øµÀµÂ½×¶ÎÊ±£¬
            //Ô¤¶Á²éÕÒÒª²½½øµ½µÄ½×¶Î¿ªÊ¼Ê±µÄ×´Ì¬£»
            else if(stageNum != 0)
            {
                i = 0;
                tmp_block = lfq_read_prefetch(gHandle,i++);
                while(NULL != tmp_block && tmp_block->stageNum != stageNum && tmp_block->stageNum != 0)
                    tmp_block = lfq_read_prefetch(gHandle,i++);
//                INFO("%s: 4 i = %d",__func__,i);
            }

            //²éÕÒµ½ÁËÒª²½½øµ½µÄ½×¶Î¿ªÊ¼µÄÊı¾İ¿é£»
            if(NULL != tmp_block && tmp_block->stageNum == stageNum && tmp_block->stageNum != 0 && i > 0)
            {
//                INFO("%s: motorChanType = %x, pedChanType = %x",__func__,data->motorChanType,data->pedChanType);
                //±éÀúËùÓĞÍ¨µÀµÄ×´Ì¬£¬ÕÒµ½¹ı¶ÉµÄ×´Ì¬£¬»ÆµÆÓÅÏÈÓÚÈ«ºì£»
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
                    //Èç¹ûÍ¨µÀµ±Ç°µÄ×´Ì¬Ê±ÂÌµÆ£¬²½½øºó×´Ì¬ÊÇºìµÆ£¬¶øÇÒ¸ÃÍ¨µÀÊÇ»ú¶¯³µÍ¨µÀ£¬
                    //Ôò¸ÃÍ¨µÀĞèÒª¹ı¶É£¬Ê¹ÓÃÕÒµ½µÄ¹ı¶É×´Ì¬½øĞĞ¹ı¶É£»
                    if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->motorChanType,i) == 1)
                        data->allChannels[i] = tmp_status;
                    //Èç¹ûÍ¨µÀµ±Ç°ÊÇÂÌµÆ£¬²½½øºóÊÇºìµÆ£¬¶øÇÒ¸ÃÍ¨µÀÊÇ·Ç»ú¶¯³µÍ¨µÀ£¬
                    //Ôò½«¸ÃÍ¨µÀ¸ÄÎªÈ«ºì×´Ì¬½øĞĞ¹ı¶É£»
                    else if(data->allChannels[i] == GREEN && tmp_block->allChannels[i] == RED && GET_BIT(data->pedChanType,i) == 1)
                        data->allChannels[i] = ALLRED;
                }
            }
			return FALSE;
        }
	}
	return FALSE;
}
