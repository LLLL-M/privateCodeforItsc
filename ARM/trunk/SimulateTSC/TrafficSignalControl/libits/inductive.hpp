inline void Phasecontrol::InductiveControl::ReduceInductiveExtendTime(LineQueueData *data)
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

inline void Phasecontrol::InductiveControl::AdjustInductiveLeftTime(LineQueueData *data, UInt16 extendTime, UInt8 inductivePhaseIndex)
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

UInt16 Phasecontrol::InductiveControl::CheckVehicleData(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    int i = 0, n = 0;
	UInt64 vehicleData = 0;	//过车数据
	UInt16 extendTime = 0;	//延长时间
	int timeGap = 0;
	LineQueueData *block;
	
	//查看当前绿灯相位是否在窗口期内有过车信息
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseStatus == INVALID || phaseInfos[i].vehicleDetectorBits == 0 || phaseInfos[i].unitExtendGreen == 0)
		    continue;
		if (phaseInfos[i].phaseStatus != GREEN && phaseInfos[i].phaseStatus != GREEN_BLINK)
		{	//当相位不是绿灯状态时清除之前延长的绿灯时间和
			totalExtendTime[i] = 0;
			continue;
		}
		//下面的相位状态不是GREEN就是GREEN_BLINK
		if (phaseInfos[i].maxExtendGreen > totalExtendTime[i])
		{	//绿灯时间还可以延长
			vehicleData = gVehicleData;	//采集过车信息
			if ((phaseInfos[i].phaseLeftTime > data->checkTime)
				&& (phaseInfos[i].phaseLeftTime <= data->checkTime + phaseInfos[i].unitExtendGreen))	//窗口期内发送信号给车检器采集模块采集车检数据
				pc.ipc.SemPostForVeh();
			else if (phaseInfos[i].phaseLeftTime == data->checkTime)
				gVehicleData = 0;	//窗口期过后清除全局车检过车数据
			else	//其他非窗口期不处理过车数据
				continue;
			if (vehicleData & phaseInfos[i].vehicleDetectorBits)
			{	//表示在窗口期内有过车而且还可以继续延长绿灯时间
				timeGap = phaseInfos[i].maxExtendGreen - totalExtendTime[i];
				extendTime = std::min((UInt8)timeGap, phaseInfos[i].unitExtendGreen);
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
		block = static_cast<LineQueueData *>(pc.ipc.gLfq.lfq_read_prefetch(n));
		block->cycleTime += extendTime;
		block->phaseInfos[i].splitTime += extendTime;
	}
	data->leftTime += extendTime;
	data->cycleTime += extendTime;
	return extendTime;
}


UInt16 Phasecontrol::InductiveControl::InductiveCoordinateDeal(LineQueueData *data, LineQueueData *cycleLeftData)
{
	UInt16 cycleLeftTime = 0;
	LineQueueData *nextData, *block;
	int i, j, n;
	
	if (data->inductiveCoordinateCycleTime <= data->cycleTime)
		return 0;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if ((pc.ptl.addTimeToFirstPhase == false
				&& data->stageNum + 1 == data->maxStageNum 
				&& data->phaseInfos[i].phaseSplitLeftTime == 1)	
				//倒数第二个相位运行的最后1s
			|| (pc.ptl.addTimeToFirstPhase == true
				&& data->stageNum == data->maxStageNum 
				&& data->phaseInfos[i].phaseSplitLeftTime == 1))
				//倒数第一个相位运行的最后1s
		{
			//调整下一个相位放行的第1s的相关信息
			nextData = static_cast<LineQueueData *>(pc.ipc.gLfq.lfq_read_prefetch(0));
			std::memcpy(cycleLeftData, nextData, sizeof(LineQueueData));
			for (j = 0; j < MAX_PHASE_NUM; j++)
			{
				if ((cycleLeftData->phaseInfos[j].phaseStatus == GREEN || cycleLeftData->phaseInfos[j].phaseStatus == GREEN_BLINK)
					&& cycleLeftData->phaseInfos[j].splitTime > 0
					&& cycleLeftData->phaseInfos[j].phaseSplitLeftTime == cycleLeftData->phaseInfos[j].splitTime)
				{
					cycleLeftTime = data->inductiveCoordinateCycleTime - data->cycleTime;
					AdjustInductiveLeftTime(cycleLeftData, cycleLeftTime, j);
					if (pc.ptl.addTimeToFirstPhase == false)	//只调整当前周期数据
					{	//如果是把剩余时间全部添加到最后一个相位，则还需调整此周期后续的data
						//调整线性队列中当前周期剩余的所有1s钟信息里的周期时间和对应的绿信比
						for (n = 0; n < cycleLeftData->leftTime; n++)
						{
							block = static_cast<LineQueueData *>(pc.ipc.gLfq.lfq_read_prefetch(n));
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

inline void Phasecontrol::InductiveControl::ClearData()
{
	m_extendTime = 0;
	m_cycleLeftTime = 0;
	std::memset(totalExtendTime, 0, sizeof(totalExtendTime));
	std::memset(&m_cycleLeftData, 0, sizeof(m_cycleLeftData));
	gVehicleData = 0;
}

void Phasecontrol::InductiveControl::Deal(LineQueueData *data)
{
	if (data->actionId == INDUCTIVE_ACTIONID || data->actionId == INDUCTIVE_COORDINATE_ACTIONID)
	{	//感应或协调感应模式下实时检查过程数据,判断是否在窗口期内有过车，计算需要延长的绿灯时间
		m_extendTime += CheckVehicleData(data);
	}
	else
		ClearData();
	if (m_extendTime > 0)	//说明当前处于感应延长期需要调整一下时间
	{
		m_extendTime--;
		ReduceInductiveExtendTime(data);
		return;
	}
	switch(pc.curMode)
	{
		case INDUCTIVE_MODE:		
			pc.ReadLineQueueData(data, pc.mSchemeId);
			break;
		case INDUCTIVE_COORDINATE_MODE:
			if (m_cycleLeftTime == 0)
			{
				pc.ReadLineQueueData(data, pc.mSchemeId);
				m_cycleLeftTime = InductiveCoordinateDeal(data, &m_cycleLeftData);
			}
			else
			{
				m_cycleLeftTime--;
				std::memcpy(data, &m_cycleLeftData, sizeof(LineQueueData));
				ReduceInductiveExtendTime(&m_cycleLeftData);
			}
			break;
        default: break;
	}
}

void Phasecontrol::InductiveControl::run(void *arg)
{
	int i;
	
	while (1)
	{
		pc.ipc.SemWaitForVeh();
		gVehicleData = 0;
		for (i = 0; i < 40; i++)
		{
			gVehicleData = pc.ptl.ItsReadVehicleDetectorData();
			msleep(25);
		}
	}
}
