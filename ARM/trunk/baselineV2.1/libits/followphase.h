//找寻指定跟随相位从当前阶段开始，结束时所在的阶段号以及结束时跟随的母相位
static UInt8 FindFollowPhaseEndStage(FollowPhaseInfo *followPhaseInfo, UInt8 curStage, UInt8 *endPhase)
{
	int i, s;
	UInt8 stageNum, ret;
	UInt8 motherPhase, tmpPhase = 0;
	
	if (followPhaseInfo == NULL
		|| curStage == 0 || curStage > gCalInfo.maxStageNum
		|| endPhase == NULL)
		return 0;
	*endPhase = 0;
	ret = stageNum = curStage;
	for (s = 0; s < gCalInfo.maxStageNum; s++, stageNum = NEXT_STAGE(stageNum))
	{
		for (i = 0; i < MAX_PHASE_NUM; i++)
		{
			if (GET_BIT(followPhaseInfo->phaseBits, i) == 0)
				continue;
			motherPhase = i + 1;
			if (IsStageIncludePhase(stageNum, motherPhase))
			{
				tmpPhase = motherPhase;
				break;
			}
		}
		if (tmpPhase == 0)
			break;
		else if (tmpPhase > 0)
		{
			*endPhase = tmpPhase;
			tmpPhase = 0;
			ret = stageNum;
		}
	}
	if (s == gCalInfo.maxStageNum)
		ret = 0;
	return ret;
}
//获取指定跟随相位从下一阶段到下次开始放行时跟随的起始母相位
static inline UInt8 GetFollowPhaseStartMotherPhase(FollowPhaseInfo *followPhaseInfo, UInt8 curStage)
{
	int i, s;
	
	for (s = NEXT_STAGE(curStage); s != curStage; s = NEXT_STAGE(s))
	{
		for (i = 0; i < MAX_PHASE_NUM; i++)
		{
			if (GET_BIT(followPhaseInfo->phaseBits, i) == 1 && IsStageIncludePhase(s, i + 1))
				return i + 1;
		}
	}
	return 0;
}

static void SetFollowPhaseInfo(LineQueueData *data, UInt8 curStage, UInt16 *phaseGreen)
{
	int i, j, ch;
	PhaseInfo *phaseInfos = data->phaseInfos;
	LightStatus status, motherPhaseStatus;
	FollowPhaseInfo *followPhaseInfos = gCalInfo.followPhaseInfos;
	UInt8 motherPhase;
	UInt8 endStage = 0, startPhase = 0, endPhase = 0;
	
	for (i = 0; i < MAX_FOLLOWPHASE_NUM; i++)
	{
		if (followPhaseInfos[i].phaseBits == 0)
			continue;
		status = RED;
		endStage = FindFollowPhaseEndStage(&followPhaseInfos[i], curStage, &endPhase);
		if (endStage == 0)	//说明整个周期跟随相位一直常绿
		{
			phaseInfos[i].followPhaseStatus = GREEN;
			phaseInfos[i].followPhaseLeftTime = gCalInfo.cycleTime;
		}
		else
		{
			for (j = 0; j < MAX_PHASE_NUM; j++)
			{
				if (GET_BIT(followPhaseInfos[i].phaseBits, j) == 0)
					continue;
				motherPhase = j + 1;
				motherPhaseStatus = phaseInfos[motherPhase - 1].phaseStatus;
				if (motherPhaseStatus == GREEN || motherPhaseStatus == GREEN_BLINK || motherPhaseStatus == YELLOW || motherPhaseStatus == RED_YELLOW || motherPhaseStatus == ALLRED)
				{	/*当母相位正在放行时，如果跟随相位结束时的运行母相位与当前母相位相等，则跟随相位状态与此母相位一致*/
					if (endPhase == motherPhase)
						status = motherPhaseStatus;
					else
						status = GREEN;
					break;
				}
			}
			phaseInfos[i].followPhaseStatus = (UInt8)status;
			if (status == GREEN_BLINK || status == YELLOW || status == RED_YELLOW || status == ALLRED)
			{
				if (endPhase == 0 || endPhase > MAX_PHASE_NUM)
					continue;
				phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime;
			}
			else if (status == GREEN)
			{
				if (endPhase == 0 || endPhase > MAX_PHASE_NUM)
					continue;
				if (motherPhase == endPhase)
					phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime;
				else
				{
					phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime + phaseGreen[endPhase - 1];
				}
			}
			else if (status == RED)
			{
				startPhase = GetFollowPhaseStartMotherPhase(&followPhaseInfos[i], curStage);
				if (startPhase == 0 || startPhase > MAX_PHASE_NUM)
					continue;
				phaseInfos[i].followPhaseLeftTime = phaseInfos[startPhase - 1].phaseLeftTime;
			}
		}
		for (ch = 0; ch < MAX_CHANNEL_NUM; ch++)
		{
			if (GET_BIT(followPhaseInfos[i].motorChannelBits, ch) == 1)
			{
				data->allChannels[ch] = phaseInfos[i].followPhaseStatus;
				data->channelCountdown[ch] = phaseInfos[i].followPhaseLeftTime;
			}
			if (GET_BIT(followPhaseInfos[i].pedChannelBits, ch) == 1)
			{
				if (motherPhase != endPhase || endPhase == 0 || endPhase > MAX_PHASE_NUM)
				{
					data->allChannels[ch] = phaseInfos[i].followPhaseStatus;
					data->channelCountdown[ch] = phaseInfos[i].followPhaseLeftTime;
				}
				else
				{
					data->allChannels[ch] = phaseInfos[endPhase - 1].pedestrianPhaseStatus;
					data->channelCountdown[ch] = phaseInfos[endPhase - 1].pedestrianPhaseLeftTime;
				}
			}
		}
	}				
}

