#include <string.h>
#include "convert.h"
#include "hikmsg.h"

extern GbConfig *gGbconfig;					//全局国标配置指针
extern SignalControllerPara *gSignalControlpara;

UInt32	gbToNtcipFlag;					//国标转NTPIP标志

static void GbUnitConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	UnitPara *item = &ntcip->stUnitPara;
	
	memset(item, 0, sizeof(UnitPara));
	item->byFluxCollectCycle = gb->dataCollectCycle;
	item->nBootYellowLightTime = gb->bootBlinkTime;
	item->nBootAllRedTime = gb->bootAllRedTime;
	item->byUnitControl = gb->allowRemoteActivate;
	item->byFlashFrequency = gb->flashFrequency;
	item->byTransCycle = 2;	//默认过渡周期为2
}

static void GbPhaseConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PhaseItem *item = ntcip->stPhase;//[NUM_PHASE]
	GbPhaseList *list = gb->phaseTable;
	struct STRU_SignalTransEntry *entry = ntcip->AscSignalTransTable;
	int i;
	int clearSize = offsetof(PhaseItem, nCircleID);	
	UInt8 yellowTime, allRedTime;
	
	for (i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		yellowTime = item[i].nYellowTime;
		allRedTime = item[i].nAllRedTime;
		memset(&item[i], 0, clearSize);	//清除每个相位中与环和并发相位无关的信息
		if (BIT(list[i].phaseOption, 0) == 0)
			continue;	//相位未使能
		item[i].nPhaseID = i + 1;//list[i].phaseNo;
		item[i].nPedestrianPassTime = list[i].pedestrianPassTime;
		item[i].nPedestrianClearTime = list[i].pedestrianClearTime;
		item[i].nMinGreen = list[i].minGreen;
		item[i].nUnitExtendGreen = list[i].unitExtendGreen / 10;	//国标要求10倍关系，所以除以10
		item[i].nMaxGreen_1 = list[i].maxGreen_1;
		item[i].nMaxGreen_2 = list[i].maxGreen_2;
		item[i].nYellowTime = yellowTime;
		item[i].nAllRedTime = allRedTime;
		item[i].wPhaseOptions = BIT(list[i].phaseOption, 0) | (BIT(list[i].phaseOption, 3) << 13);//设置相位使能和行人自动放行标志
		entry[i].nGreenLightTime = list[i].greenBlinkTime / 10;		//国标要求10倍关系，所以除以10
	}
}

static void GbConflictPhaseConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PhaseItem *item = ntcip->stPhase;//[NUM_PHASE]
	GbPhaseConflictList *list = gb->phaseConflictTable;
	int i, j, n;
	UInt16 conflictPhase;
	
	//并发相位
	for (i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		n = 0;
		memset(item[i].byPhaseConcurrency, 0, sizeof(item[i].byPhaseConcurrency));
		conflictPhase = BIG16_TO_LITTLE16(list[i].conflictPhase);
		for (j = 0; j < MAX_PHASE_LIST_NUM; j++)
		{
			if (j == i || list[i].phaseConflictNo == 0 || BIT(conflictPhase, j))
				continue;	//如果未设置冲突相位，或是设置了对于有冲突的相位直接返回，剩下的就是并发相位
			if (BIT(gb->phaseTable[j].phaseOption, 0))
				item[i].byPhaseConcurrency[n++] = j + 1;
		}
	}
}

static void GbStageTimingConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PhaseTurnItem *phaseTurnItem = ntcip->stPhaseTurn[0];//[NUM_PHASE_TURN][NUM_RING_COUNT]
	GreenSignalRationItem *splitItem = ntcip->stGreenSignalRation[0];//[NUM_GREEN_SIGNAL_RATION][NUM_PHASE]
	PhaseItem *phaseItem = ntcip->stPhase;//[NUM_PHASE]
	GbStageTimingList *list = NULL;//gb->stageTimingTable[MAX_STAGE_TIMING_LIST_NUM][MAX_STAGE_NUM]
	int i, j, k, ring = 0;
	int clearSize = offsetof(GreenSignalRationItem, nIsCoordinate);
	UInt16 phaseNo;	//阶段放行的相位
	
	for (i = 0; i < NUM_GREEN_SIGNAL_RATION; i++)
	{	//清空ntcip协议绿信比表中除去协调相位的其他值，因为等会要重新配置
		for (j = 0; j < NUM_PHASE; j++)
			memset(&ntcip->stGreenSignalRation[i][j], 0, clearSize);
	}
	for (i = 0; i < MAX_STAGE_TIMING_LIST_NUM; i++)
	{
		list = gb->stageTimingTable[i];
		phaseTurnItem = ntcip->stPhaseTurn[i];
		splitItem = ntcip->stGreenSignalRation[i];
		
		for (j = 0; j < MAX_STAGE_NUM; j++)
		{
			if (list[j].phaseNo == 0)
				break;	//此阶段没有相位，说明这个配时方案到这就截止了
			ring = 0;
			phaseNo = BIG16_TO_LITTLE16(list[j].phaseNo);
			for (k = 0; k < MAX_PHASE_LIST_NUM; k++)
			{
				if (BIT(phaseNo, k) == 0)
					continue;
				phaseItem[k].nYellowTime = list[j].yellowTime;
				phaseItem[k].nAllRedTime = list[j].allRedTime;
				phaseItem[k].nCircleID = ring + 1;
				
				splitItem[k].nGreenSignalRationID = i + 1;
				splitItem[k].nPhaseID = k + 1;
				splitItem[k].nGreenSignalRationTime = list[j].greenTime + list[j].yellowTime + list[j].allRedTime;
			
				phaseTurnItem[ring].nPhaseTurnID = i + 1;
				phaseTurnItem[ring].nCircleID = ring + 1;
				phaseTurnItem[ring].nTurnArray[j] = k + 1;
				if (++ring == NUM_RING_COUNT)
					break;
			}
		}
	}
}

static void GbChannelConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	ChannelItem	*item = ntcip->stChannel;//[NUM_CHANNEL]
	GbChannelList *list = gb->channelTable;
	int i;
	
	memset(ntcip->stChannel, 0, sizeof(ntcip->stChannel));
	for (i = 0; i < MAX_CHANNEL_LIST_NUM; i++)
	{
		if (list[i].channelRelatedPhase == 0)
			continue;
		item[i].nChannelID = i + 1;//list[i].channelNo;
		item[i].nControllerID = list[i].channelRelatedPhase;
		item[i].nControllerType = list[i].channelControlType;
		if ((list[i].channelFlashStatus & 0x06) == 0x06)
			list[i].channelFlashStatus = 0x04;	//如果同时设置了黄闪和红闪，则使用红闪
		item[i].nFlashLightType = list[i].channelFlashStatus;
	}
}

static void GbSchemeConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	SchemeItem *item = ntcip->stScheme;//[NUM_SCHEME]
	GbSchemeList *list = gb->schemeTable;//[MAX_SCHEME_LIST_NUM]
	GreenSignalRationItem *splitItem = ntcip->stGreenSignalRation[0];//[NUM_GREEN_SIGNAL_RATION][NUM_PHASE]
	int i, j;
	
	memset(ntcip->stScheme, 0, sizeof(ntcip->stScheme));
	for (i = 0; i < NUM_GREEN_SIGNAL_RATION; i++)
	{	//清空ntcip协议绿信比表中的协调相位，因为等会要重新配置
		for (j = 0; j < NUM_PHASE; j++)
			ntcip->stGreenSignalRation[i][j].nIsCoordinate = 0;
	}
	for (i = 0; i < MAX_SCHEME_LIST_NUM; i++)
	{
		if (list[i].stageTimingNo == 0)
			continue;
		item[i].nSchemeID = i + 1;//list[i].schemeNo;
		item[i].nCycleTime = list[i].cycleTime;
		item[i].nOffset = list[i].phaseGap;
		item[i].nGreenSignalRatioID = list[i].stageTimingNo;
		item[i].nPhaseTurnID = list[i].stageTimingNo;
		if (list[i].coordinatePhase != 0)
			ntcip->stGreenSignalRation[list[i].stageTimingNo - 1][list[i].coordinatePhase - 1].nIsCoordinate = 1;
	}
}

static void GbTimeIntervalConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	TimeIntervalItem *item = ntcip->stTimeInterval[0];//[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID]
	ActionItem *actionItem = ntcip->stAction;//[NUM_ACTION]
	GbTimeIntervalList *list = gb->timeIntervalTable[0];//[MAX_TIMEINTERVAL_LIST_NUM][MAX_TIMEINTERVAL_NUM]
	int i, j;
	
	memset(ntcip->stTimeInterval, 0, sizeof(ntcip->stTimeInterval));
	memset(ntcip->stAction, 0, sizeof(ntcip->stAction));
	actionItem[INDUCTIVE_ACTIONID - 1].nActionID = INDUCTIVE_ACTIONID;
	actionItem[INDUCTIVE_ACTIONID - 1].nSchemeID = INDUCTIVE_SCHEMEID;
	actionItem[ALLRED_ACTIONID - 1].nActionID = ALLRED_ACTIONID;
	actionItem[ALLRED_ACTIONID - 1].nSchemeID = ALLRED_SCHEMEID;
	actionItem[YELLOWBLINK_ACTIONID - 1].nActionID = YELLOWBLINK_ACTIONID;
	actionItem[YELLOWBLINK_ACTIONID - 1].nSchemeID = YELLOWBLINK_SCHEMEID;
	actionItem[TURNOFF_ACTIONID - 1].nActionID = TURNOFF_ACTIONID;
	actionItem[TURNOFF_ACTIONID - 1].nSchemeID = TURNOFF_SCHEMEID;
	for (i = 0; i < MAX_TIMEINTERVAL_LIST_NUM; i++)
	{
		item = ntcip->stTimeInterval[i];
		list = gb->timeIntervalTable[i];
		for (j = 0; j < MAX_TIMEINTERVAL_NUM; j++)
		{
			item[j].nTimeIntervalID = i + 1;//list[j].timeIntervalListNo;
			item[j].nTimeID = j + 1;//list[j].timeIntervalNo;
			item[j].cStartTimeHour = list[j].hour;
			item[j].cStartTimeMinute = list[j].minute;
			switch (list[j].controlMode)
			{
				case SYSTEM_MODE: 
					switch (list[j].schemeId)
					{
						case 1 ... MAX_SCHEME_LIST_NUM:
							item[j].nActionID = list[j].schemeId;
							actionItem[list[j].schemeId - 1].nActionID = list[j].schemeId;
							actionItem[list[j].schemeId - 1].nSchemeID = list[j].schemeId;
							break;
#if 0
						case GB_INDUCTIVE_SCHEME: item[j].nActionID = INDUCTIVE_ACTIONID; break;
#endif
						case GB_ALLRED_SCHEME: item[j].nActionID = ALLRED_ACTIONID; break;
						case GB_YELLOWFLASH_SCHEME: item[j].nActionID = YELLOWBLINK_ACTIONID; break;
						case GB_TURNOFF_SCHEME: item[j].nActionID = TURNOFF_ACTIONID; break;
						default: item[j].nActionID = 0; break;	//默认信号机自主控制为没有响应
					}
					break;
				case TURNOFF_LIGHTS_MODE: item[j].nActionID = TURNOFF_ACTIONID; break;
				case YELLOWBLINK_MODE: item[j].nActionID = YELLOWBLINK_ACTIONID; break;
				case ALLRED_MODE: item[j].nActionID = ALLRED_ACTIONID; break;
				case INDUCTIVE_MODE: item[j].nActionID = INDUCTIVE_ACTIONID; break;
				default: item[j].nActionID = 0; break;	//其他未实现的控制方式暂时默认没有响应
			}
		}
	}
}

static void GbScheduleConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PlanScheduleItem *item = ntcip->stPlanSchedule;//[NUM_SCHEDULE]
	GbScheduleList *list = gb->scheduleTable;//[MAX_SCHEDULE_LIST_NUM]
	int i;
	
	memset(ntcip->stPlanSchedule, 0, sizeof(ntcip->stPlanSchedule));
	for (i = 0; i < MAX_SCHEDULE_LIST_NUM; i++)
	{
		if (list[i].timeIntervalListNo == 0)
			continue;
		item[i].nScheduleID = i + 1;//list[i].scheduleNo;
		item[i].month = BIG16_TO_LITTLE16(list[i].month);
		item[i].week = list[i].week;
		item[i].day = BIG32_TO_LITTLE32(list[i].day);
		item[i].nTimeIntervalID = list[i].timeIntervalListNo;		
	}
}

static void GbDetectorConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	struct STRU_N_VehicleDetector *item = ntcip->AscVehicleDetectorTable;//[MAX_VEHICLEDETECTOR_COUNT]
	GbVehDetectorList *list = gb->vehDetectorTable;//[MAX_VEH_DETECTOR_NUM]
	int i;
	
	memset(ntcip->AscVehicleDetectorTable, 0, sizeof(ntcip->AscVehicleDetectorTable));
	for (i = 0; i < MAX_VEH_DETECTOR_NUM; i++)
	{
		item[i].byVehicleDetectorNumber = i + 1;//list[i].detectorNo;
		item[i].byVehicleDetectorCallPhase = list[i].requestPhase;
	}
}

static void GbFollowPhaseConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	FollowPhaseItem	*item = ntcip->stFollowPhase;//[NUM_FOLLOW_PHASE]
	GbFollowPhaseList *list = gb->followPhaseTable;//[MAX_FOLLOW_PHASE_LIST_NUM]
	int i;
	
	memset(ntcip->stFollowPhase, 0, sizeof(ntcip->stFollowPhase));
	for (i = 0; i < MAX_FOLLOW_PHASE_LIST_NUM; i++)
	{
		if (list[i].motherPhaseNum == 0)
			continue;
		item[i].nFollowPhaseID = i + 1;//list[i].followPhaseNo;
		item[i].byOverlapType = list[i].operateType;
		memcpy(item[i].nArrayMotherPhase, list[i].motherPhase, list[i].motherPhaseNum);
		memcpy(item[i].byArrOverlapModifierPhases, list[i].correctPhase, list[i].correctPhaseNum);
		item[i].nGreenTime = list[i].tailGreenTime;
		item[i].nYellowTime = list[i].tailYellowTime;
		item[i].nRedTime = list[i].tailAllRedTime;
	}
}

void GbConvertToNtcip()
{
	static ConvertStruct convert[] = 
	{
		{SCHEDULE_BIT, GbScheduleConvert},
		{TIMEINTERVAL_BIT, GbTimeIntervalConvert},
		{SCHEME_BIT, GbSchemeConvert},
		{CHANNEL_BIT, GbChannelConvert},
		{STAGE_TIMING_BIT, GbStageTimingConvert},
		{PHASE_BIT, GbPhaseConvert},
		{UNIT_BIT, GbUnitConvert},
		{VEHDETECTOR_BIT, GbDetectorConvert},
		{FOLLOWPHASE_BIT, GbFollowPhaseConvert},
		{CONFLICTPHASE_BIT, GbConflictPhaseConvert}
	};
	int i, num = sizeof(convert) / sizeof(ConvertStruct);
	struct msgbuf msg;
	GbConfig *bak = gGbconfig + 1;
	
	memmove(bak, gGbconfig, offsetof(GbConfig, scheduleTable));
	bak->downloadFlag = 0;
	if (gbToNtcipFlag > 1 && gGbconfig->downloadFlag == 2)
	{
		for (i = 0; i < num; i++)
		{
			if (BIT(gbToNtcipFlag, convert[i].bit))
				convert[i].func(gGbconfig, gSignalControlpara);
		}
		gbToNtcipFlag = 0;
		gGbconfig->downloadFlag = 0;
		msg.mtype = MSG_NEW_CONFIG;
		msg.msgwFlag = 1;
		msgsnd(msgid, &msg, MSGSIZE, 0);//发送给数据管理模块的消息
	}
}
