static UInt64 gVehicleData = 0;	//车检数据,0-47bit分别对应1-48号车检器,0:无过车,1:有过车
static UInt8 gNextCycleExtendTime[MAX_PHASE_NUM] = {0};	//自适应控制下一周期各相位最大绿应延长的时间
extern CalInfo gCalInfo;

static inline void ExtendMaxGreenTime(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	
	for(i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK
			|| phaseInfos[i].phaseStatus == YELLOW || phaseInfos[i].phaseStatus == ALLRED)
		{
			phaseInfos[i].maxExtendGreen += gNextCycleExtendTime[i];
			if (phaseInfos[i].maxExtendGreen > phaseInfos[i].maxExtendGreen2)
				phaseInfos[i].maxExtendGreen = phaseInfos[i].maxExtendGreen2;
		}
	}
}

static void ReduceInductiveExtendTime(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseLeftTime > 0)
			phaseInfos[i].phaseLeftTime--;
		if (phaseInfos[i].pedestrianPhaseLeftTime > 0)
			phaseInfos[i].pedestrianPhaseLeftTime--;
		if (phaseInfos[i].phaseSplitLeftTime > 0)
			phaseInfos[i].phaseSplitLeftTime--;
		if (phaseInfos[i].followPhaseLeftTime > 0)
			phaseInfos[i].followPhaseLeftTime--;
	}
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (data->channelCountdown[i] > 0)
			data->channelCountdown[i]--;
	}
	if (data->leftTime > 0)
		data->leftTime--;
}

static inline void AdjustInductiveLeftTime(LineQueueData *data, UInt16 extendTime, UInt8 inductivePhaseIndex)
{
	int j;
	PhaseInfo *phaseInfos = NULL;
	
	if (data == NULL || extendTime == 0 || inductivePhaseIndex >= MAX_PHASE_NUM)
		return;
	phaseInfos = data->phaseInfos;
	phaseInfos[inductivePhaseIndex].splitTime += extendTime;
	phaseInfos[inductivePhaseIndex].phaseSplitLeftTime += extendTime;
	for (j = 0; j < MAX_PHASE_NUM; j++)
	{
		if (phaseInfos[j].phaseStatus != INVALID)
		{
			if (phaseInfos[j].pedestrianPhaseLeftTime > 0)
				phaseInfos[j].pedestrianPhaseLeftTime += extendTime;
			phaseInfos[j].phaseLeftTime += extendTime;
		}
		if (phaseInfos[j].followPhaseStatus != INVALID)
			phaseInfos[j].followPhaseLeftTime += extendTime;
	}
	for (j = 0; j < MAX_CHANNEL_NUM; j++)
	{
		if (data->allChannels[j] != INVALID)
			data->channelCountdown[j] += extendTime;
	}
}

static UInt8 CheckVehicleData(UInt8 *totalExtendTime, LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i = 0, n = 0, j = 0;
	UInt64 vehicleData = 0;	//过车数据
	UInt8 extendTime = 0;	//延长时间
	int timeGap = 0;
	UInt8 vehicleDetectorId;
	LineQueueData *block;
	
	//查看当前绿灯相位是否在窗口期内有过车信息
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseSplitLeftTime == 1)
		{	//在相位运行的最后1s,如果此相位达到了最大绿则下一周期增加一个单位延长绿，反之减少
			if (phaseInfos[i].maxExtendGreen <= totalExtendTime[i])
			{
				if (phaseInfos[i].maxExtendGreen < phaseInfos[i].maxExtendGreen2)
					gNextCycleExtendTime[i] += phaseInfos[i].unitExtendGreen;
			}
			else
				gNextCycleExtendTime[i] -= min(phaseInfos[i].unitExtendGreen, gNextCycleExtendTime[i]);
			totalExtendTime[i] = 0;//清除之前延长的绿灯时间和
			continue;
		}
		if ((phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK)
			&& phaseInfos[i].maxExtendGreen > totalExtendTime[i])
		{	//绿灯时间还可以延长
			vehicleData = gVehicleData;	//采集过车信息
			if ((phaseInfos[i].phaseLeftTime > data->checkTime)
				&& (phaseInfos[i].phaseLeftTime <= data->checkTime + phaseInfos[i].unitExtendGreen))	//窗口期内发送信号给车检器采集模块采集车检数据
				sem_post(&gSemForVeh);
			else if (phaseInfos[i].phaseLeftTime == data->checkTime)
				gVehicleData = 0;	//窗口期过后清除全局车检过车数据
			else	//其他非窗口期不处理过车数据
				continue;
			if (vehicleData & phaseInfos[i].vehicleDetectorBits)
			{	//表示在窗口期内有过车而且还可以继续延长绿灯时间
				timeGap = phaseInfos[i].maxExtendGreen - totalExtendTime[i];
				if (timeGap <= 0)
					extendTime = 0;
				else
					extendTime = min((UInt8)timeGap, phaseInfos[i].unitExtendGreen);
				totalExtendTime[i] += extendTime;
				INFO("DEBUG extendTime = %d, totalExtendTime[%d] = %d", extendTime, i, totalExtendTime[i]);
				break;
			}
		}
	}
	if (extendTime == 0)
		return 0;
    if (data->inductiveCoordinateCycleTime > 0)
	{
		timeGap = data->inductiveCoordinateCycleTime - data->cycleTime;
		if (timeGap <= 0)
			return 0;
		else
		{
			if (timeGap < extendTime)
				extendTime = timeGap;
		}
	}
	AdjustInductiveLeftTime(data, extendTime, i);
	//调整线性队列中当前周期剩余的所有1s钟信息里的周期时间和对应的绿信比
	for (n = 0; n < data->leftTime - 1; n++)
	{							//减去1是因为当前这1s也包含在内
		block = (LineQueueData *)lfq_read_prefetch(gHandle, n);
		block->cycleTime += extendTime;
		block->phaseInfos[i].splitTime += extendTime;
		for (j = 0; j < gCalInfo.stageInfos[data->stageNum - 1].includeNum; j++)
		{
			if (gCalInfo.stageInfos[data->stageNum - 1].includePhases[j] != (i + 1))
			{
				block->phaseInfos[gCalInfo.stageInfos[data->stageNum - 1].includePhases[j] - 1].splitTime += extendTime;
			}
		}
	}
	data->leftTime += extendTime;
	data->cycleTime += extendTime;
	return extendTime;
}

static UInt16 InductiveCoordinateDeal(LineQueueData *data, LineQueueData *cycleLeftData)
{
	UInt16 cycleLeftTime = 0;
	LineQueueData *nextData, *block;
	int i, j, n;
	
	if (data->inductiveCoordinateCycleTime <= data->cycleTime)
		return 0;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if ((gCustomParams.addTimeToFirstPhase == FALSE
				&& data->stageNum + 1 == data->maxStageNum 
				&& data->phaseInfos[i].phaseSplitLeftTime == 1)	
				//倒数第二个相位运行的最后1s
			|| (gCustomParams.addTimeToFirstPhase == TRUE
				&& data->stageNum == data->maxStageNum 
				&& data->phaseInfos[i].phaseSplitLeftTime == 1))
				//倒数第一个相位运行的最后1s
		{
			//调整下一个相位放行的第1s的相关信息
			nextData = (LineQueueData *)lfq_read_prefetch(gHandle, 0);
			memcpy(cycleLeftData, nextData, sizeof(LineQueueData));
			for (j = 0; j < MAX_PHASE_NUM; j++)
			{
				if ((cycleLeftData->phaseInfos[j].phaseStatus == GREEN || cycleLeftData->phaseInfos[j].phaseStatus == GREEN_BLINK)
					&& cycleLeftData->phaseInfos[j].splitTime > 0
					&& cycleLeftData->phaseInfos[j].phaseSplitLeftTime == cycleLeftData->phaseInfos[j].splitTime)
				{
					cycleLeftTime = data->inductiveCoordinateCycleTime - data->cycleTime;
					AdjustInductiveLeftTime(cycleLeftData, cycleLeftTime, j);
					if (gCustomParams.addTimeToFirstPhase == FALSE)	//只调整当前周期数据
					{	//如果是把剩余时间全部添加到最后一个相位，则还需调整此周期后续的data
						//调整线性队列中当前周期剩余的所有1s钟信息里的周期时间和对应的绿信比
						for (n = 0; n < cycleLeftData->leftTime; n++)
						{
							block = (LineQueueData *)lfq_read_prefetch(gHandle, n);
							block->cycleTime += cycleLeftTime;
							block->phaseInfos[j].splitTime += cycleLeftTime;
						}
						cycleLeftData->leftTime += cycleLeftTime;
						cycleLeftData->cycleTime += cycleLeftTime;
					}
					else
					{
						cycleLeftData->leftTime = cycleLeftTime;
						cycleLeftData->cycleTime = data->inductiveCoordinateCycleTime;
						cycleLeftData->inductiveCoordinateCycleTime = data->inductiveCoordinateCycleTime;
					}
					break;
				}
			}
			break;
		}
	}
	return cycleLeftTime;
}

void *VehicleCheckModule(void *arg)
{
	int i;
	
	while (1)
	{
		sem_wait(&gSemForVeh);
		gVehicleData = 0;
		for (i = 0; i < 40; i++)
		{
			gVehicleData |= ItsReadVehicleDetectorData();
			usleep(25000);
		}
	}
}
