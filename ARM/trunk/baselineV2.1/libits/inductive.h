static UInt64 gVehicleData = 0;	//车检数据,0-47bit分别对应1-48号车检器,0:无过车,1:有过车
extern UInt32 gNextCycleExtendTime[MAX_PHASE_NUM];	//自适应控制下一周期各相位最大绿应延长的时间
extern CalInfo gCalInfo;

StepInductiveInfo stepInductive = {0,0};

//calculate next  stage of step phase inductive 
int ItsCalcuInductiveStepStage(CalInfo *pcalinfo, UINT8 current_phaseid)
{
	return 0;
}
int ItsGetOneSecVehPass(UINT8 inwindow, UINT64* vehdata_1s)
{
	return 0;
}
int ItsClearStepPhaseTimeGap(CalInfo *pcalinfo, UINT8 phaseid)
{
	return 0;
}

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
	UINT8 curStage = 0;
	
	if (data == NULL || extendTime == 0 || inductivePhaseIndex >= MAX_PHASE_NUM)
		return;
	phaseInfos = data->phaseInfos;
	phaseInfos[inductivePhaseIndex].splitTime += extendTime;
	phaseInfos[inductivePhaseIndex].phaseSplitLeftTime += extendTime;
	curStage = data->stageNum;
	for (j = 0; j < gCalInfo.stageInfos[curStage - 1].includeNum; j++)
	{
		if (gCalInfo.stageInfos[curStage - 1].includePhases[j] != (inductivePhaseIndex + 1))
		{
			phaseInfos[gCalInfo.stageInfos[curStage - 1].includePhases[j] - 1].splitTime += extendTime;
			phaseInfos[gCalInfo.stageInfos[curStage - 1].includePhases[j] - 1].phaseSplitLeftTime += extendTime;
		}
	}
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

static UINT8 StepToInductiveStage(LineQueueData *data, UInt8 stageNum)
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

static UInt8 CheckVehicleData(UInt8 *totalExtendTime, LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i = 0, n = 0, j = 0;
	UInt64 vehicleData = 0;	//过车数据
	UInt8 extendTime = 0;	//延长时间
	int timeGap = 0;
	UInt8 vehicleDetectorId;
	LineQueueData *block;
	UINT8 ring = 0;
	UINT8 stage = 0;
	UINT8 tmp = 0;
	UINT8 phaseid = 0;
	
	//查看当前绿灯相位是否在窗口期内有过车信息
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseSplitLeftTime == 1)
		{	//在相位运行的最后1s,如果此相位达到了最大绿则下一周期增加一个单位延长绿，反之减少
			if (phaseInfos[i].maxExtendGreen <= totalExtendTime[i])
			{
				if (phaseInfos[i].maxExtendGreen < phaseInfos[i].maxExtendGreen2)
					gNextCycleExtendTime[i] += phaseInfos[i].unitExtendGreen;	
				//INFO("++gNextCycleExtendTime[%d] =%d", i, gNextCycleExtendTime[i]);
			}
			else
			{	
				gNextCycleExtendTime[i] -= min((UINT32)(phaseInfos[i].unitExtendGreen), gNextCycleExtendTime[i]);
				//INFO("--gNextCycleExtendTime[%d] =%d", i, gNextCycleExtendTime[i]);
			}
			totalExtendTime[i] = 0;//清除之前延长的绿灯时间和

			if (ring == 0 && data->actionId == INDUCTIVE_ACTIONID && 
				((i + 1) == gCalInfo.stageInfos[data->stageNum - 1].includePhases[0]))//current phaseinfo == current stage's include phaseinfo
			{//check vehicle and pedestrian request at the last one second,then update the step stage  that closer to current stage.
				stepInductive.stepNow = 1;  //step inductive phase .
				stage = ItsCalcuInductiveStepStage(&gCalInfo, data->stageNum);
				
				if ((stepInductive.stepStageNum > 0) && (stage > 0) && ((stage > data->stageNum)? 
									(stage - data->stageNum) : (data->maxStageNum - data->stageNum + stage)) <= 
								((stepInductive.stepStageNum > data->stageNum)? 
									(stepInductive.stepStageNum - data->stageNum) : (data->maxStageNum - data->stageNum + stepInductive.stepStageNum)))
					stepInductive.stepStageNum = stage;
				else if ((stepInductive.stepStageNum == 0) && (stage > 0))
					stepInductive.stepStageNum = stage;
				else if ((stepInductive.stepStageNum == 0) && (stage == 0))
					stepInductive.stepStageNum = data->stageNum;
				//INFO("1. step stage=%d(%d) currentphase=%d, curstage=%d", stage, stepInductive.stepStageNum, i+1, data->stageNum);
				ring++;
			}
			//INFO("phaseinfos[%d] current stage=%d", i, data->stageNum);
			continue;
		}
		if ((phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK))
		{	//绿灯时间还可以延长
			//vehicleData = gVehicleData;	//采集过车信息
			if (phaseInfos[i].maxExtendGreen > totalExtendTime[i])
			{
				if ((phaseInfos[i].phaseLeftTime > data->checkTime + phaseInfos[i].unitExtendGreen))
					ItsGetOneSecVehPass(0, &vehicleData);
				else if ((phaseInfos[i].phaseLeftTime > data->checkTime)
					&& (phaseInfos[i].phaseLeftTime <= data->checkTime + phaseInfos[i].unitExtendGreen))	//窗口期内发送信号给车检器采集模块采集车检数据
					//sem_post(&gSemForVeh);
					ItsGetOneSecVehPass(1, &vehicleData);
				
					//其他非窗口期不处理过车数据
					
				if ((vehicleData & phaseInfos[i].vehicleDetectorBits) && 
					((gCalInfo.vehDetectorState & phaseInfos[i].vehicleDetectorBits) == 0))//Veh detector state is normal
				{	//表示在窗口期内有过车而且还可以继续延长绿灯时间
					vehicleData &= (~phaseInfos[i].vehicleDetectorBits);
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
			if (phaseInfos[i].phaseLeftTime == data->checkTime)
			{	
				ItsGetOneSecVehPass(0, &vehicleData);
				//gVehicleData = 0;	//窗口期过后清除全局车检过车数据
				if (data->actionId == INDUCTIVE_COORDINATE_ACTIONID || data->actionId == SINGLE_ADAPT_ACTIONID)
					stepInductive.stepStageNum = 0;
				else if (ring == 0) 
				{
					ring++;
					//log_debug("--------phase=%d cal step stage, phase lefttime=%d, ", i+1, phaseInfos[i].phaseLeftTime);
					stepInductive.stepStageNum = ItsCalcuInductiveStepStage(&gCalInfo, data->stageNum);
					if (stepInductive.stepStageNum == 0)//keep lignt channel in current data.
						stepInductive.stepNow = 2;
					else if (stepInductive.stepStageNum > 0)
						stepInductive.stepNow = 0;
					//log_debug("--------phase=%d cal step stage=%d", i+1, stepInductive.stepStageNum);
					//INFO("current stagenum=%d, inductive step stage %d(phase=%d) ", data->stageNum, stepInductive.stepStageNum, gCalInfo.stageInfos[data->stageNum - 1].includePhases[0]);
				}
			}
		}
	}
	if (stepInductive.stepNow == 1 && stepInductive.stepStageNum > 0)
	{
		phaseid = gCalInfo.stageInfos[data->stageNum - 1].includePhases[0];
		ItsClearStepPhaseTimeGap(&gCalInfo, phaseid);
		ItsClearVehDataHaveCars(&gCalInfo, phaseid);
		ItsClearPedestrianDetectorData(data, phaseid, 0);
		//INFO("clear phase %d timegap havecars pedestriandata", phaseid);
		if (StepToInductiveStage(data, stepInductive.stepStageNum) == 1)
		{
			stepInductive.stepNow = 0;
			stepInductive.stepStageNum = 0;
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

//////////////////////////////Single point priority//////////////////////////////


int SingleSpotPriority(UInt8 *totalExtendTime, LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
	int i = 0, n = 0;
	static UInt64 vehicleData = 0;
	UInt8 extendTime = 0;
	INT32 timeGap = 0;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		//when phase's splitleft time less than 5s(phase status is yellow or yellowblink),begin to check vehicle
		if ((phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK) && 
				(phaseInfos[i].phaseLeftTime <= (gCalInfo.singlespotCheckTime > 0? gCalInfo.singlespotCheckTime + 1 : 4)))
		{
			ItsGetOneSecVehPass(1, &vehicleData);
			//INFO("vehicleData=%d, phaseLeftTime=%d", (UINT32)vehicleData, phaseInfos[i].phaseLeftTime);
		}
		if ((phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK) && 
			phaseInfos[i].phaseLeftTime == 1)
		{
			//if (vehicleData > 0)
				//INFO("have car pass during last time,vehicledata=%x, phase[%d]vehbits = %x", (UINT32)(vehicleData), i, (UINT32)(phaseInfos[i].vehicleDetectorBits));
			if (vehicleData & phaseInfos[i].vehicleDetectorBits)
			{
				vehicleData &= (~phaseInfos[i].vehicleDetectorBits);
				
				timeGap = phaseInfos[i].maxExtendGreen - gNextCycleExtendTime[i];
				gNextCycleExtendTime[i] += ((timeGap < phaseInfos[i].unitExtendGreen)? timeGap : phaseInfos[i].unitExtendGreen);
				//INFO("inc gNextcycleExtendtime[%d]=%d", i, gNextCycleExtendTime[i]);
			}
			else
			{	
				
				timeGap = gNextCycleExtendTime[i] + phaseInfos[i].minShortGreen;
				gNextCycleExtendTime[i] -= ((timeGap > phaseInfos[i].unitExtendGreen)? 
					phaseInfos[i].unitExtendGreen : timeGap);
				//INFO("dec gNextcycleExtendtime[%d]=%d", i, gNextCycleExtendTime[i]);
			}
			ItsGetOneSecVehPass(0, &vehicleData);
			
		}
	}
	return 0;
}




////////////////pedestrian once req cross street //////////////////////////////////
#define PEDKEY_DELAY_TIME 10
UINT8 VehicleGreenLightKeep = 0;//1: vehicle lane  keep green light flag,don't read linequeue data.

UINT8 ItsSetReadPedestrianReqTime(void)
{
	return 0;
}

UINT8 ItsReadPedestrianDetectorData(UINT8 readNowFlag)
{
	return 0;
}
UINT8 ItsClearPedestrianDetectorData(LineQueueData* data, UINT8 phaseId, UINT8 pedKey)
{
	return 0;
}
UINT16 ItsGetPedKeyDelayTime()
{
	return 0;
}
/*
处理行人过街一次请求
*/
UINT8 PedestrianReqOnceDeal(LineQueueData* data)
{
	int i = 0;
	static UINT8 pedes_req_phase = 0;
	UINT8 pedReqBits = 0;
	
	if (pedReqBits == 0 && VehicleGreenLightKeep == 1)
	{
		pedReqBits = ItsReadPedestrianDetectorData(0);
		if (pedReqBits > 0)
		{
			VehicleGreenLightKeep = 0;
			for (i = 0; i < MAX_PHASE_NUM; i++)
			if(gCalInfo.phaseTimes[i].pedestrianDetectorBits & pedReqBits)
			{	
				//INFO("next pedestrian phase %d", i + 1);
				pedes_req_phase = i + 1;
				break;
			}
		}
		return 0;
	}
	
	//pedestrian phase's split time == 1, clear pedestrian detector data.
	if (pedes_req_phase > 0 && pedes_req_phase <= MAX_PHASE_NUM && data->stageNum == 2)//stage 2 is pedestrian phase
	{
		for (i = 0; i < MAX_PHASE_NUM; i++)
			if (data->phaseInfos[i].pedestrianPhaseLeftTime == 1)
			{   //INFO("clear now.....");
				ItsClearPedestrianDetectorData(data, pedes_req_phase, 0);
				pedes_req_phase = 0;
				ItsSetReadPedestrianReqTime();
			}
	}
	if (pedes_req_phase == 0 && VehicleGreenLightKeep == 0)
	{//pedestrian phase end, need to stay in stage 1 when phaseLsftTime == timedelay
		UINT16 timedelay = 0;
		timedelay = ItsGetPedKeyDelayTime();
		if (data->stageNum == 1 && 
				(data->phaseInfos[0].phaseStatus == GREEN || data->phaseInfos[0].phaseStatus == GREEN_BLINK) && 
				data->phaseInfos[0].phaseLeftTime == timedelay)//set phaseleft time == the time of pedestrian button delay to response
				VehicleGreenLightKeep = 1;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////Pedestrian Two Step Cross Street////////////////////////////////
//C1,C2 are vehicle channel; C3,C4 are pedestrian channel; r1,r2,r3,r4 are pedestrian key
/**************************
            C1           C2
	   |         |*|         |
	   |         |*|         |
C3  r1|      r2|   |         |
	   |         |  |         |
	   |         |  |r4      |r3 C4
	   |         |*|         |
	   |         |*|         |
**************************/
typedef enum
{
	PEDESTRIAN_NO_REQ = 0,
	PEDESTRIAN_ROADCENTER_REQ = 1,
	PEDESTRIAN_ROADSIDE_TRANS = 2,
	PEDESTRIAN_ROADSIDE_REQ = 3,
}PedestrianReqState;
#define MAX_PEDESTRIANDETECTOR_COUNT 8
UINT8 pedestrianReqState = 0;

UINT8 ptcsReqPhase = 0;//pedestrian request cross street phase
UINT8 ptcsStepFollowstate[MAX_FOLLOWPHASE_NUM] = {INVALID}; //step phase's light state
/**
跳转相位，函数在获取queue中的数据后被调用(after deal())
*/
static UINT8 StepToStageAfterDeal(LineQueueData *data, UInt8 stageNum)
{
	int i, t = data->cycleTime;
	int left = 0;
	struct msgbuf msg;
	LineQueueData dt;
	if (data->stageNum == 0)//stagenum invalid, don't step phase.
		return 1;
	memcpy(&dt, data, sizeof(LineQueueData));
	while (stageNum > 0 && dt.stageNum != stageNum)
	{
		
		left = lfq_element_count(gHandle);
		if (left == 0)
			break;
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
		lfq_read(gHandle, &dt);
	}
	if (dt.stageNum == stageNum || stageNum > dt.maxStageNum )
	{
		memcpy(data, &dt, sizeof(LineQueueData));
		return 1;
	}
	else
		return 0;
}
/*
根据行人检测器按下的数据bits 获取对应的相位ID
*/
UINT8 GetPedesReqPhaseId(UINT8 pedDetectorBits, LineQueueData* data)
{
	UINT32 i = 0;
	PhaseTimeInfo *phaseTimeInfos = gCalInfo.phaseTimes;

	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseTimeInfos[i].pedestrianDetectorBits & pedDetectorBits)// has pedestrian press Detector
			break;
	}
	if (i == MAX_PHASE_NUM)
		return 0;
	return i + 1;
}
/*
获取行人请求的相位的对应的跟随相位状态
*/
UINT8 GetPedReqPhaseFollowState(UINT8 stepphase, LineQueueData* data, UINT8* follow_state)
{
	LineQueueData* block = NULL;
	UINT8 cur_phase = 0;
	UINT8 step_stage = 0;
	UINT32 i = 0, j = 0;
	UINT32 left = 0;

	if (follow_state == NULL || data == NULL || stepphase <= 0 || stepphase > MAX_PHASE_NUM)
		return 0;
	memset(follow_state, 0, MAX_PHASE_NUM);
	block = data;
	cur_phase = gCalInfo.stageInfos[data->stageNum - 1].includePhases[0];
	step_stage = gCalInfo.phaseIncludeStage[stepphase - 1][0];
	if (cur_phase > stepphase)//back
		left = data->cycleTime - data->leftTime;
	else
		left = data->leftTime;
	while (i < left)
	{
		
		if (cur_phase > stepphase)//back
			block = lfq_read_back_prefetch(gHandle, i + 1);
		else
			block = lfq_read_prefetch(gHandle, i);
		if (block->stageNum == step_stage && 
			block->phaseInfos[stepphase - 1].splitTime == block->phaseInfos[stepphase - 1].phaseSplitLeftTime)
		{
			for (j = 0; j < MAX_FOLLOWPHASE_NUM; j++)
				follow_state[j] = block->phaseInfos[j].followPhaseStatus;
			break;	
		}
		i++;	
	}
	return 0;
}

//check followphase'ch transition state neet to modify or don't
void PedCrossStreetTransitionStateModify(LineQueueData *data, UINT8* need_modify_state)
{
	UINT32 i = 0, j = 0;
	FollowPhaseInfo *fphaseInfos = gCalInfo.followPhaseInfos;
	PhaseInfo *phaseInfos = data->phaseInfos;
	
	for (i = 0; i < MAX_FOLLOWPHASE_NUM; i++)
	{
		if (fphaseInfos[i].phaseBits > 0)//have follow mother phase
		{
			for (j = 0; j < MAX_CHANNEL_NUM; j++)
				if(GET_BIT(fphaseInfos[i].motorChannelBits, j))
				{	
					if (((ptcsStepFollowstate[i] == GREEN || ptcsStepFollowstate[i] == GREEN_BLINK) &&
						  	(phaseInfos[i].followPhaseStatus == YELLOW ||
								phaseInfos[i].followPhaseStatus == YELLOW_BLINK ||
								phaseInfos[i].followPhaseStatus == RED))||
						((ptcsStepFollowstate[i] == YELLOW || ptcsStepFollowstate[i] == YELLOW_BLINK || ptcsStepFollowstate[i] == RED) && 
						  	(phaseInfos[i].followPhaseStatus == GREEN || 
							    phaseInfos[i].followPhaseStatus == GREEN_BLINK)))
						need_modify_state[j] = ptcsStepFollowstate[i];
				}
		}
	}
}

UINT8 ProcessPedDetectorData(LineQueueData *data, UINT8 pedDetectorBits)
{
	UINT8 pedReqPhase = 0;
	//LineQueueData* block = NULL;
	PhaseInfo *phaseInfos = data->phaseInfos;
	
	//pedDetectorBits = ItsReadPedestrianDetectorData();

	if (pedDetectorBits > 0)//ped button pressed
	{
		pedReqPhase = GetPedesReqPhaseId(pedDetectorBits, data);
		
		//if (phaseInfos[pedReqPhase - 1].followPhaseStatus )
		GetPedReqPhaseFollowState(pedReqPhase, data, ptcsStepFollowstate);
		//block = lfq_read_prefetch(gHandle, 0);
		//PedCrossStreetTransitionStateModify(block, ptcs_need_modify_state);
		//ptcsReqPhase = pedReqPhase; 
	}
	return pedReqPhase;
}
/*
根据需要跳转到的相位对应的跟随相位的状态来修改当前跳转时过渡的相位
对应通道的灯色，过渡阶段灯色处理:
1.当前是绿灯，跳转后为红灯或黄灯，将过渡期的灯色按照黄灯红灯进行过渡
2.当前是红灯或黄灯，跳转后是绿灯，将过渡期的灯色改为跳转后的灯色

*/
void ModifyFollowChannelLight(LineQueueData* data)
{
	UINT32 left = 0;
	PhaseInfo *phaseInfos = data->phaseInfos;
	FollowPhaseInfo* fphaseInfos = gCalInfo.followPhaseInfos;
	UINT8 cur_phase = 0;
	UINT8 i = 0, j = 0;
	UINT8 allred = 2;//default 2s

	cur_phase = gCalInfo.stageInfos[data->stageNum - 1].includePhases[0];
	left = phaseInfos[cur_phase - 1].phaseSplitLeftTime;
	allred = gCalInfo.phaseTimes[cur_phase - 1].passTimeInfo.allRedTime;
	for (i = 0; i < MAX_FOLLOWPHASE_NUM; i++)
	{
		if ((fphaseInfos[i].phaseBits > 0) && (phaseInfos[i].followPhaseStatus != ptcsStepFollowstate[i]))
		{
			if (((phaseInfos[i].followPhaseStatus == GREEN || phaseInfos[i].followPhaseStatus == GREEN_BLINK) && 
				   (ptcsStepFollowstate[i] == RED || ptcsStepFollowstate[i] == ALLRED || ptcsStepFollowstate[i] == YELLOW || ptcsStepFollowstate[i] == YELLOW_BLINK)) ||
				((phaseInfos[i].followPhaseStatus == YELLOW || phaseInfos[i].followPhaseStatus == YELLOW_BLINK || phaseInfos[i].followPhaseStatus == RED || phaseInfos[i].followPhaseStatus == ALLRED) && 
				    (ptcsStepFollowstate[i] == GREEN || ptcsStepFollowstate[i] == GREEN_BLINK)))
			{
				for (j = 0; j < MAX_CHANNEL_NUM; j++)
					if (GET_BIT(fphaseInfos[i].motorChannelBits, j))
					{
						data->allChannels[j] = (ptcsStepFollowstate[i] == RED && left > allred)? YELLOW : ptcsStepFollowstate[i];
					};
			}
		}
	}
}
/*
处理行人二次请求的操作步骤:
1.无请求时，获取行人请求数据，根据请求的数据的bit位设置请求状态
行人在路中请求或者行人在路边请求
2.根据行人请求的相位进行过渡处理，过渡完成后跳转到行人相位
3.根据相认请求的状态来判断当前行人相位放行后行人请求状态，请求状态
决定放行的行人相位次数
4.行人请求状态清空后，跳转到默认的第一阶段的相位驻留，默认放行机动车通道
*/
void PedestrianReqTwiceDeal(LineQueueData * data)
{
	UINT8 i = 0, j = 0;
	LineQueueData tmpdata;
	UINT8 pedestrian_req_data = 0;
	static UINT8 pedKey = 0;
	
	if (pedestrianReqState == PEDESTRIAN_NO_REQ && VehicleGreenLightKeep == 1)//pedestrian no req and vehicle lane keep light green
	{
		pedestrian_req_data = ItsReadPedestrianDetectorData(0);
		if (pedestrian_req_data > 0)
		{	
			pedestrianReqState = PEDESTRIAN_ROADCENTER_REQ;
			VehicleGreenLightKeep = 0;//has pedestrian req, begin to read line queue data
			ptcsReqPhase = ProcessPedDetectorData(data, pedestrian_req_data);
			for (i = 0; i < MAX_PEDESTRIANDETECTOR_COUNT; i++)
			{
				if (GET_BIT(pedestrian_req_data, i))//if pedestrian detector id is 1,3,5(odd),pedestrian button installed belong aside of road
				{	
					log_debug("ped req presskey %d ", i + 1);
					pedKey = i + 1;
					if (!(i % 2))
					{	
						pedestrianReqState = PEDESTRIAN_ROADSIDE_REQ;//SET_BIT(pedestrian_req_data, i + 1);// cross road need to light two pedestrian phase.
					}
					break;
				}
			}
		}
		return;
	}
	if (ptcsReqPhase > 0 )//transition to pedestrian req phase
	{
		UINT8 cur_phase = gCalInfo.stageInfos[data->stageNum - 1].includePhases[0];
		if (cur_phase != 1)//isn't phase 1,step to ptcsReqPhase
		{
			if (StepToStageAfterDeal(data, gCalInfo.stageInfos[ptcsReqPhase - 1].includePhases[0]) == 0)
				return;
			ptcsReqPhase = 0;
			memset(ptcsStepFollowstate, 0, MAX_FOLLOWPHASE_NUM);
		}
		//INFO("transtion curpahse=%d, curphase status=%d, split left time=%d", 
			//cur_phase, data->phaseInfos[cur_phase - 1].phaseStatus, data->phaseInfos[cur_phase - 1].phaseSplitLeftTime);
		if ((data->phaseInfos[cur_phase - 1].phaseStatus == YELLOW || 
			data->phaseInfos[cur_phase - 1].phaseStatus == YELLOW_BLINK ||
			data->phaseInfos[cur_phase - 1].phaseStatus == RED ||
			data->phaseInfos[cur_phase - 1].phaseStatus == ALLRED) && 
			data->phaseInfos[cur_phase - 1].phaseSplitLeftTime >= 1)
			ModifyFollowChannelLight(data);
		return;
	}
	
	//todo: change the pedestrianReqState  when pass over  pedestrian phase.
	if (pedestrianReqState > PEDESTRIAN_NO_REQ)
	{
		for (i = 0; i < MAX_PHASE_NUM; i++)
		{
			//phase attr's vechile isn't selected, means pedestrian phase
			if (data->phaseInfos[i].phaseSplitLeftTime == 1 && data->phaseInfos[i].vechileAutoReq == 0)
			{
				pedestrian_req_data = ItsReadPedestrianDetectorData(1);//read pedestrian detector data right now
				//INFO(" mid ped req = %X ", pedestrian_req_data);
				if (pedestrianReqState == PEDESTRIAN_ROADCENTER_REQ && (pedKey == 2 || pedKey == 4))
				{
					if ((pedKey == 2 && (GET_BIT(pedestrian_req_data, 0) == 0)) || //first pedreq2,  then no pedreq1
						(pedKey == 4 && (GET_BIT(pedestrian_req_data, 2) == 0)))//first pedreq4, then no pedreq3
						pedestrianReqState--;
				}
				else if (pedestrianReqState == PEDESTRIAN_ROADSIDE_REQ && (pedKey == 1 || pedKey == 3))// pedestrian req key 1 or 3
				{
					UINT8 opposite_side_req = 0;
					if (pedKey == 1)//current pedestrian key 1 pressed,check pedestrian key 3 req
						opposite_side_req = GET_BIT(pedestrian_req_data, 2);
					else if (pedKey == 3) //current pedestrian key 3 pressed,check pedestrian key 1 req
						opposite_side_req = GET_BIT(pedestrian_req_data, 0);
					if (opposite_side_req == 0)//the other side don't have pedestrian req
						pedestrianReqState--;

					pedestrianReqState--;
				}
				else //PEDESTRIAN_ROADCENTER_REQ && pedKey == 1 || pedKey == 3
					pedestrianReqState--;
				
				if (data->phaseInfos[i].pedestrianDetectorBits > 0)
					ItsClearPedestrianDetectorData(data, i + 1, 0);
				//INFO("pass one |||| phase=%d, reqstate=%d", i + 1, pedestrianReqState);
			}
		}
		if (pedestrianReqState == PEDESTRIAN_NO_REQ)
			ItsSetReadPedestrianReqTime();
		return;
	}
	if (pedestrianReqState == PEDESTRIAN_NO_REQ)
	{
		UINT16 timedelay = 0;
		pedKey = 0;
		if (data->stageNum != 1)//isn't 1st stage,need to step
			if (StepToStageAfterDeal(data, 1) == 0)
				return;
		timedelay = ItsGetPedKeyDelayTime();
		if ((data->phaseInfos[0].phaseStatus == GREEN || data->phaseInfos[0].phaseStatus == GREEN_BLINK) && 
			 data->phaseInfos[0].phaseLeftTime == timedelay)
			VehicleGreenLightKeep = 1;///when first phase's greenlight left time == 1, stop read line queue data
	}
}



//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////Bus Priority////////////////////////

UINT32 ItsGetBusPrioData(UINT32* busData)
{
	//todo: get busPrioData related to phase
	return 0;

}
/*
公交优先的处理函数，检测公交优先的数据，若有公交优先则
根据优先相位和对应的时间判断是否当前相位延长绿灯或者后续相位
执行最小绿，红灯缩短。
*/
static UINT32 CheckBusData(UInt8 *totalExtendTime, LineQueueData *data, UINT32* busPrioData)
{
	int i = 0, n = 0, k = 0;
	PhaseInfo *phaseInfos = data->phaseInfos;
	//UINT8 busPrioTime = 10; //default set 6sec, can config. more than the time that  bus get to stop line
	UINT8 extendTime = 0;
	UINT8 timeGap = 0;
	UINT32 delay = 0;
	UINT8 phase = 0;
	LineQueueData *block;
	StageInfo *stageinfos = gCalInfo.stageInfos;
	PhaseTimeInfo* phaseTimes = gCalInfo.phaseTimes;

	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (phaseInfos[i].phaseSplitLeftTime == 1)
		{
			totalExtendTime[i] = 0;
			continue;
		}
		
		if ((phaseInfos[i].phaseStatus == GREEN || phaseInfos[i].phaseStatus == GREEN_BLINK) && 
			((phaseInfos[i].busDetectorBits[0] & busPrioData[0]) || 
			(phaseInfos[i].busDetectorBits[1] & busPrioData[1]) ||
			(phaseInfos[i].busDetectorBits[2] & busPrioData[2]) || 
			(phaseInfos[i].busDetectorBits[3] & busPrioData[3])))
		{
			if (phaseInfos[i].phaseLeftTime > phaseInfos[i].busPrioPassTime)//has enough green light time ,don't need to add time
			{	
				//INFO("1.bus prio phase=%d has enough time,priopasstime=%d, don't need extend time, busBits=%x",
					//i + 1, phaseInfos[i].busPrioPassTime, data->phaseInfos[i].busDetectorBits[0]);
				ItsClearBusPrioData(data, i + 1);
				continue;
			}
			if (phaseInfos[i].maxExtendGreen > totalExtendTime[i]) 
			{
				timeGap = phaseInfos[i].maxExtendGreen - totalExtendTime[i];
				delay = phaseInfos[i].busPrioPassTime - phaseInfos[i].phaseLeftTime;
				delay = (((delay % 3) == 0)? delay : ((delay / 3)+1) * 3);
				extendTime = min((UINT32)timeGap, delay);
				totalExtendTime[i] += extendTime;
				//ItsClearBusPrioData(data, i + 1);
				//INFO("2.bus priopasstime=%d, extend time %ds,phase=%d, busBits=%x", 
				      //phaseInfos[i].busPrioPassTime, totalExtendTime[i], i + 1, data->phaseInfos[i].busDetectorBits[0]);
				for (k = 0; k < stageinfos[data->stageNum - 1].includeNum; k++)// dual ring phase
				{
					phase = stageinfos[data->stageNum - 1].includePhases[k];
					ItsClearBusPrioData(data, phase);//clear busPrioData of the same stage's concurrency phase 
				}
				break;
			}
			
		}
	}

	if (extendTime == 0)
		return 0;
	
	AdjustInductiveLeftTime(data, extendTime, i);
	for (n = 0; n < data->leftTime - 1; n++)
	{							
		block = (LineQueueData *)lfq_read_prefetch(gHandle, n);
		block->cycleTime += extendTime;
		block->phaseInfos[i].splitTime += extendTime;
		for (k = 0; k < stageinfos[data->stageNum - 1].includeNum; k++)// dual ring phase
		{
			if ((stageinfos[data->stageNum - 1].includePhases[k] - 1) != i)
				block->phaseInfos[stageinfos[data->stageNum - 1].includePhases[k] - 1].splitTime += extendTime;
		}
	}
	data->leftTime += extendTime;
	data->cycleTime += extendTime;
	return extendTime;
}



///////////////////////////SpecialCar Priority////////////////////////////
UINT8 ItsGetSpecialCarControlData(CalInfo *pcalinfo, SpecialCarControl* scarControl)
{
	return 0;
}
UINT8 ItsClearSpecialCarDataBits(CalInfo *pcalinfo, UINT8 phaseId)
{
	return 0;
}

void ItsAddSpecialCarChanLock(SpecialCarControl* scarControl)
{
	return;
}
void ItsDelSpecialCarChanLock()
{
	return;
}
/*
特勤优先级别L1，执行通道锁定处理
*/
static void SpecialCarLevel1(LineQueueData *data, SpecialCarControl* scarControl)
{
	ItsAddSpecialCarChanLock(scarControl);
	
	if (scarControl->SpecialCarControlTime-- <= 0)
	{
		ItsDelSpecialCarChanLock();
		ItsClearSpecialCarDataBits(&gCalInfo, scarControl->SpecialCarPhase);
		scarControl->SpecialCarControlTime = 0;
		scarControl->SpecialCarControlLevel = 0;
		scarControl->SpecialCarStepStage = 0;
		scarControl->SpecialCarPhase = 0;
	}
}
static UINT8 PreProcessL2(LineQueueData *data, SpecialCarControl* scarControl)
{
	UINT8 curstage = data->stageNum;
	UINT8 i = 0;
	UINT8 curphase = 0;

	if (curstage != scarControl->SpecialCarStepStage || (data->phaseInfos[scarControl->SpecialCarPhase - 1].phaseStatus != GREEN))
	{
		if(IsStepTransitionComplete(data, scarControl->SpecialCarStepStage) == TRUE)
			DirectStepToStage(data, scarControl->SpecialCarStepStage);
	}
	if (data->stageNum == scarControl->SpecialCarStepStage)//current stage == special car step stage
	{
		
		if (data->phaseInfos[scarControl->SpecialCarPhase - 1].phaseStatus == GREEN)
		{	
			if (scarControl->SpecialCarControlTime >= data->phaseInfos[scarControl->SpecialCarPhase - 1].phaseLeftTime)
				scarControl->SpecialCarControlTime -= data->phaseInfos[scarControl->SpecialCarPhase - 1].phaseLeftTime;
			else
				scarControl->SpecialCarControlTime = 0;
			return 1;
		}
	}
	return 0;
	
}
/*
特勤优先级别L2,执行相位跳转
*/
static void SpecialCarLevel2(LineQueueData *data, SpecialCarControl* scarControl)
{
	static UINT8 preprocess = 0;
	if (data == NULL || scarControl == NULL || 
		(scarControl->SpecialCarStepStage <= 0 && scarControl->SpecialCarStepStage > data->maxStageNum))
		return;
	if (preprocess == 0)
	{
		if (PreProcessL2(data, scarControl) == 1)
			preprocess = 1;
	}
	if (preprocess == 1 && data->stageNum == scarControl->SpecialCarStepStage)
	{	
		if (scarControl->SpecialCarControlTime <= 0)
		{
			ItsClearSpecialCarDataBits(&gCalInfo, scarControl->SpecialCarPhase);
			scarControl->SpecialCarControlTime = 0;
			scarControl->SpecialCarControlLevel = 0;
			scarControl->SpecialCarStepStage = 0;
			scarControl->SpecialCarPhase = 0;
			preprocess = 0;
		}
		else
			scarControl->SpecialCarControlTime--;
		//INFO("scarControl->SpecialCarControlTime=%d", scarControl->SpecialCarControlTime);	
	}
	else
		preprocess = 0;
		
}

static void SpecialCarDataDeal(LineQueueData *data, SpecialCarControl* scarControl)
{
	switch (scarControl->SpecialCarControlLevel)
	{
		case SPECIALCAR_LEVEL1:
			SpecialCarLevel1(data, scarControl);
			break;
		case SPECIALCAR_LEVEL2:
			SpecialCarLevel2(data, scarControl);
			break;
	}
}

/////////////////////////////////////////////////////////////

