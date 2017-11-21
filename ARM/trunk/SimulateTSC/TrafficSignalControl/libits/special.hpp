//设置黄闪、全红、关灯时的数据信息
void Phasecontrol::SpecialControl::SetSpecialData(LineQueueData *data, LightStatus status)
{
	int i;
	PhaseInfo *phaseInfos = data->phaseInfos;
	UInt8 channelFlashStatus[MAX_CHANNEL_NUM] = {0};	//通道闪光状态

	if (!pc.ptl.isContinueRun)//特殊控制之后是否需要继续接着之前的周期运行
		pc.ipc.gLfq.lfq_reinit();	//清除线性队列中的数据块，不再接着之前的方案继续运行
    data->stageNum = 0;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
	    if (phaseInfos[i].followPhaseStatus != INVALID)
	    {
            phaseInfos[i].followPhaseStatus = status;
	        phaseInfos[i].followPhaseLeftTime = 0;
	    }
	    if (phaseInfos[i].phaseStatus == INVALID)
	        continue;
	    phaseInfos[i].phaseStatus = status;    
	    phaseInfos[i].phaseLeftTime = 0;
	    phaseInfos[i].phaseSplitLeftTime = 0;
	    phaseInfos[i].splitTime = 0;
	    if (phaseInfos[i].pedestrianPhaseStatus != INVALID)
	    {
	        phaseInfos[i].pedestrianPhaseStatus = status;
	        phaseInfos[i].pedestrianPhaseLeftTime = 0;
	    }
	    
	}
	
	if (status == YELLOW_BLINK)
	{
//		pthread_rwlock_rdlock(&gConfigLock);
		for (i = 0; i < MAX_CHANNEL_NUM; i++)
		{
#if 0
			if (gRunConfigPara->stChannel[i].nFlashLightType == 0x04)	//红闪
				channelFlashStatus[i] = RED_BLINK;
			else if (gRunConfigPara->stChannel[i].nFlashLightType == 0)	//无闪
				channelFlashStatus[i] = TURN_OFF;
			else
#endif
				channelFlashStatus[i] = YELLOW_BLINK;
		}
//		pthread_rwlock_unlock(&gConfigLock);
	}
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (data->allChannels[i] != INVALID)
		{
			data->allChannels[i] = status;
			if (status == YELLOW_BLINK)
			{
				data->allChannels[i] = channelFlashStatus[i];
			}
		}
	}
}

void Phasecontrol::SpecialControl::Deal(LineQueueData *data)
{
	if (pc.mSchemeId == 0)	//说明此时本地时段黄闪
		pc.ReadLineQueueData(data, 0);	//读取队列信息用于本地时段切换
	switch (pc.curMode)
	{
		case YELLOWBLINK_MODE: 
			data->schemeId = YELLOWBLINK_SCHEMEID;
			SetSpecialData(data, YELLOW_BLINK);
			break;
		case TURNOFF_LIGHTS_MODE: 
			data->schemeId = TURNOFF_SCHEMEID;
			SetSpecialData(data, TURN_OFF);
			break;
		case ALLRED_MODE: 
			data->schemeId = ALLRED_SCHEMEID;
			SetSpecialData(data, ALLRED);
			break;
        default: break;
	}
}
