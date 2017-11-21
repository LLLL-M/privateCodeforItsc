#include <string.h>
#include "convert.h"
#include "hikmsg.h"

#define CUSTOM_CONVERT

extern GbConfig *gGbconfig;					//全局国标配置指针
extern SignalControllerPara *gSignalControlpara;
extern UInt32 ntcipToGbFlag;

static void NtcipUnitConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	UnitPara *item = &ntcip->stUnitPara;
	
	gb->dataCollectCycle = item->byFluxCollectCycle;
	gb->bootBlinkTime = item->nBootYellowLightTime;
	gb->bootAllRedTime = item->nBootAllRedTime;
	gb->allowRemoteActivate = item->byUnitControl;
	gb->flashFrequency = item->byFlashFrequency;
}

static void NtcipPhaseConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PhaseItem *item = ntcip->stPhase;//[NUM_PHASE]
	GbPhaseList *list = gb->phaseTable;
	struct STRU_SignalTransEntry *entry = ntcip->AscSignalTransTable;
	int i, j;
	
	memset(gb->phaseTable, 0, sizeof(gb->phaseTable));
	for (i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		if (BIT(item[i].wPhaseOptions, 0) == 0)
			continue;	//相位未使能
		list[i].phaseNo = i + 1;//item[i].nPhaseID;
#ifdef CUSTOM_CONVERT
		list[i].pedestrianPassTime = (i % 2) ? 0 : 7;//item[i].nPedestrianPassTime;
		item[i].nPedestrianPassTime = list[i].pedestrianPassTime;
#else
		list[i].pedestrianPassTime = 7;//item[i].nPedestrianPassTime;
#endif
		list[i].pedestrianClearTime = item[i].nPedestrianClearTime;
		list[i].minGreen = item[i].nMinGreen;
		list[i].unitExtendGreen = item[i].nUnitExtendGreen * 10; //单位延长绿国标要求10倍关系
		list[i].maxGreen_1 = item[i].nMaxGreen_1;
		list[i].maxGreen_2 = 30;//item[i].nMaxGreen_2;
		list[i].phaseOption = BIT(item[i].wPhaseOptions, 0) | (BIT(item[i].wPhaseOptions, 13) << 3);//设置相位使能和行人自动放行标志
		list[i].greenBlinkTime = entry[i].nGreenLightTime * 10;	//绿闪时间国标要求10倍关系
		list[i].phaseType = (1 << 7);	//固定相位
		
		gb->phaseConflictTable[i].phaseConflictNo = item[i].nPhaseID;
		gb->phaseConflictTable[i].conflictPhase = (UInt16)~(1 << i);	//初始化默认与其他相位全部都冲突
		for (j = 0; j < MAX_PHASE_LIST_NUM; j++)
		{
			if (item[i].byPhaseConcurrency[j] == 0)
				break;
			BIT_CLEAR(gb->phaseConflictTable[i].conflictPhase, item[i].byPhaseConcurrency[j] - 1);
		}
		gb->phaseConflictTable[i].conflictPhase = LITTLE16_TO_BIG16(gb->phaseConflictTable[i].conflictPhase);
	}
}

static void NtcipPhaseTurnSplitConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PhaseTurnItem *phaseTurnItem = ntcip->stPhaseTurn[0];//[NUM_PHASE_TURN][NUM_RING_COUNT]
	GreenSignalRationItem *splitItem = ntcip->stGreenSignalRation[0];//[NUM_GREEN_SIGNAL_RATION][NUM_PHASE]
	PhaseItem *phaseItem = ntcip->stPhase;//[NUM_PHASE]
	GbStageTimingList *list = NULL;//gb->stageTimingTable[MAX_STAGE_TIMING_LIST_NUM][MAX_STAGE_NUM]
	int i, j, k;
	UInt8 phaseId;
	
	memset(gb->stageTimingTable, 0, sizeof(gb->stageTimingTable));
	for (i = 0; i < MAX_STAGE_TIMING_LIST_NUM; i++)
	{
		list = gb->stageTimingTable[i];
		phaseTurnItem = ntcip->stPhaseTurn[i];
		splitItem = ntcip->stGreenSignalRation[i];
		if (splitItem[0].nGreenSignalRationTime == 0)	
			continue;	//使用相位1的绿信比作为判断绿信比是否有效的依据
		for (j = 0; j < MAX_STAGE_NUM; j++)
		{
			list[j].stageTimingNo = i + 1;
			list[j].stageNo = j + 1;
			for (k = 0; k < NUM_RING_COUNT; k++)
			{
				phaseId = phaseTurnItem[k].nTurnArray[j];
				if (phaseId == 0)
					continue;
				BIT_SET(list[j].phaseNo, phaseId - 1);
				list[j].yellowTime = ntcip->stPhase[phaseId - 1].nYellowTime;
				list[j].allRedTime = ntcip->stPhase[phaseId - 1].nAllRedTime;
				list[j].greenTime = splitItem[phaseId - 1].nGreenSignalRationTime - list[j].yellowTime - list[j].allRedTime;
			}
			list[j].phaseNo = LITTLE16_TO_BIG16(list[j].phaseNo);
			//list[j].stageOption = ;
		}
	}
}

static void NtcipChannelConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	ChannelItem	*item = ntcip->stChannel;//[NUM_CHANNEL]
	GbChannelList *list = gb->channelTable;
	int i;
	
	memset(gb->channelTable, 0, sizeof(gb->channelTable));
	for (i = 0; i < MAX_CHANNEL_LIST_NUM; i++)
	{
		if (item[i].nControllerID == 0)
			continue;
		list[i].channelNo = i + 1;//item[i].nChannelID;
		list[i].channelRelatedPhase = item[i].nControllerID;
		list[i].channelControlType = item[i].nControllerType;
#ifdef CUSTOM_CONVERT
		if (i == 1)
			list[i].channelFlashStatus = (1 << 3);	//通道2交替闪
		else if (i == 2)
			list[i].channelFlashStatus = 0;	//通道3无闪模式
		else
			list[i].channelFlashStatus = (item[i].nControllerType == PEDESTRIAN) ? (1 << 2) : (1 << 1);//item[i].nFlashLightType;如果是行人相位则闪光模式为红闪，其他默认黄闪
		item[i].nFlashLightType = list[i].channelFlashStatus;
#else
		list[i].channelFlashStatus = (item[i].nControllerType == PEDESTRIAN) ? (1 << 2) : (1 << 1);
#endif
			
	}
}

static void NtcipSchemeConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	SchemeItem *item = ntcip->stScheme;//[NUM_SCHEME]
	GbSchemeList *list = gb->schemeTable;//[MAX_SCHEME_LIST_NUM]
	int i, j;
	
	memset(gb->schemeTable, 0, sizeof(gb->schemeTable));
	for (i = 0; i < MAX_SCHEME_LIST_NUM; i++)
	{
		if (item[i].nGreenSignalRatioID == 0 || item[i].nGreenSignalRatioID > MAX_STAGE_TIMING_LIST_NUM)
			continue;
		list[i].schemeNo = i + 1;//item[i].nSchemeID;
		list[i].cycleTime = item[i].nCycleTime;
		list[i].phaseGap = item[i].nOffset;
		list[i].stageTimingNo = item[i].nGreenSignalRatioID;
		for (j = 0; j < NUM_PHASE; j++)
		{
			if (ntcip->stGreenSignalRation[list[i].stageTimingNo - 1][j].nIsCoordinate == 1)
			{
				list[i].coordinatePhase = j + 1;
				break;
			}
		}
	}
}

static void NtcipActionTimeIntervalConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	TimeIntervalItem *item = ntcip->stTimeInterval[0];//[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID]
	ActionItem *actionItem = ntcip->stAction;//[NUM_ACTION]
	GbTimeIntervalList *list = gb->timeIntervalTable[0];//[MAX_TIMEINTERVAL_LIST_NUM][MAX_TIMEINTERVAL_NUM]
	int i, j;
	
	memset(gb->timeIntervalTable, 0, sizeof(gb->timeIntervalTable));
	for (i = 0; i < MAX_TIMEINTERVAL_LIST_NUM; i++)
	{
		item = ntcip->stTimeInterval[i];
		list = gb->timeIntervalTable[i];
		for (j = 0; j < MAX_TIMEINTERVAL_NUM; j++)
		{
			if (item[j].nActionID == 0)
				continue;
			list[j].timeIntervalListNo = i + 1;//item[j].nTimeIntervalID;
			list[j].timeIntervalNo = j + 1;//item[j].nTimeID;
			list[j].hour = item[j].cStartTimeHour;
			list[j].minute = item[j].cStartTimeMinute;
			switch (item[j].nActionID)
			{
				case 1 ... MAX_SCHEME_LIST_NUM: list[j].controlMode = 0; list[j].schemeId = item[j].nActionID; break;
				case ALLRED_ACTIONID: list[j].controlMode = ALLRED_MODE; list[j].schemeId = GB_ALLRED_SCHEME; break;
				case YELLOWBLINK_ACTIONID: list[j].controlMode = YELLOWBLINK_MODE; list[j].schemeId = GB_YELLOWFLASH_SCHEME; break;
				case TURNOFF_ACTIONID: list[j].controlMode = TURNOFF_LIGHTS_MODE; list[j].schemeId = GB_TURNOFF_SCHEME; break;
				case INDUCTIVE_ACTIONID: list[j].controlMode = INDUCTIVE_MODE; list[j].schemeId = 1; break;
				default: list[j].controlMode = 0; list[j].schemeId = 0; break;
			}
		}
	}
}

static void NtcipScheduleConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	PlanScheduleItem *item = ntcip->stPlanSchedule;//[NUM_SCHEDULE]
	GbScheduleList *list = gb->scheduleTable;//[MAX_SCHEDULE_LIST_NUM]
	int i;
	
	memset(gb->scheduleTable, 0, sizeof(gb->scheduleTable));
	for (i = 0; i < MAX_SCHEDULE_LIST_NUM; i++)
	{
		if (item[i].nTimeIntervalID == 0)
			continue;
		list[i].scheduleNo = i + 1;//item[i].nScheduleID;
		list[i].month = LITTLE16_TO_BIG16(item[i].month);
		list[i].week = item[i].week;
		list[i].day = LITTLE32_TO_BIG32(item[i].day);
		list[i].timeIntervalListNo = item[i].nTimeIntervalID;
	}
}

static void NtcipDetectorConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	struct STRU_N_VehicleDetector *item = ntcip->AscVehicleDetectorTable;//[MAX_VEHICLEDETECTOR_COUNT]
	GbVehDetectorList *list = gb->vehDetectorTable;//[MAX_VEH_DETECTOR_NUM]
	int i;
	
	memset(gb->vehDetectorTable, 0, sizeof(gb->vehDetectorTable));
	for (i = 0; i < MAX_VEH_DETECTOR_NUM; i++)
	{
		if (item[i].byVehicleDetectorCallPhase == 0)
			continue;
		list[i].detectorNo = i + 1;//item[i].byVehicleDetectorNumber;
		list[i].requestPhase = item[i].byVehicleDetectorCallPhase;
#ifdef CUSTOM_CONVERT
#define DETECTOR_VALUE_SET(type, direction) do {\
	list[i].detectorType = type; \
	list[i].detectorDirection = direction;\
} while (0)
		switch (i)
		{
			case 0: DETECTOR_VALUE_SET(0x41, 0x04); break;	//东
			case 1: DETECTOR_VALUE_SET(0x41, 0x80); break;	//西北
			case 2: DETECTOR_VALUE_SET(0x41, 0x10); break;	//南
			case 3: DETECTOR_VALUE_SET(0x41, 0x20); break;	//西南
			case 4: DETECTOR_VALUE_SET(0x41, 0x40); break;	//西
			case 5: DETECTOR_VALUE_SET(0x41, 0x08); break;	//东南
			case 6: DETECTOR_VALUE_SET(0x41, 0x01); break;	//北
			case 7: DETECTOR_VALUE_SET(0x41, 0x02); break;	//东北
			case 8: DETECTOR_VALUE_SET(0x48, 0x01); break;	//北
			case 9: DETECTOR_VALUE_SET(0x48, 0x04); break;	//东
			case 10: DETECTOR_VALUE_SET(0x48, 0x10); break;	//南
			case 11: DETECTOR_VALUE_SET(0x48, 0x40); break;	//西
			case 12: DETECTOR_VALUE_SET(0x82, 0x01); break;	//北
			case 13: DETECTOR_VALUE_SET(0x24, 0x02); break;	//东北
			case 14: DETECTOR_VALUE_SET(0x11, 0x04); break;	//东
		}
		list[i].detectorOption = 0x0C;	//流量和占有率
		list[i].laneFullFlow = LITTLE16_TO_BIG16(1500);
		list[i].laneFullRate = 50;
#endif
	}
}

static void NtcipFollowPhaseConvert(GbConfig *gb, SignalControllerPara *ntcip)
{
	FollowPhaseItem	*item = ntcip->stFollowPhase;//[NUM_FOLLOW_PHASE]
	GbFollowPhaseList *list = gb->followPhaseTable;//[MAX_FOLLOW_PHASE_LIST_NUM]
	int i, j;
	
	memset(gb->followPhaseTable, 0, sizeof(gb->followPhaseTable));
	for (i = 0; i < MAX_FOLLOW_PHASE_LIST_NUM; i++)
	{
		if (item[i].nArrayMotherPhase[0] == 0)
			continue;
		list[i].followPhaseNo = i + 1;//item[i].nFollowPhaseID;
		list[i].operateType = 2;//item[i].byOverlapType;
		for (j = 0; j < MAX_PHASE_LIST_NUM; j++)
		{
			if (item[i].nArrayMotherPhase[j] == 0)
				break;
		}
		list[i].motherPhaseNum = j;
		memcpy(list[i].motherPhase, item[i].nArrayMotherPhase, j);
		for (j = 0; j < MAX_PHASE_LIST_NUM; j++)
		{
			if (item[i].byArrOverlapModifierPhases[j] == 0)
				break;
		}
		list[i].correctPhaseNum = j;
		memcpy(list[i].correctPhase, item[i].byArrOverlapModifierPhases, j);
		list[i].tailGreenTime = item[i].nGreenTime;
		list[i].tailYellowTime = item[i].nYellowTime;
		list[i].tailAllRedTime = item[i].nRedTime;
	}
}

void NtcipConvertToGb()
{
	static ConvertStruct convert[] = 
	{
		{SCHEDULE_BIT, NtcipScheduleConvert},
		{TIMEINTERVAL_BIT, NtcipActionTimeIntervalConvert},
		{SCHEME_BIT, NtcipSchemeConvert},
		{CHANNEL_BIT, NtcipChannelConvert},
		{STAGE_TIMING_BIT, NtcipPhaseTurnSplitConvert},
		{PHASE_BIT, NtcipPhaseConvert},
		{UNIT_BIT, NtcipUnitConvert},
		{VEHDETECTOR_BIT, NtcipDetectorConvert},
		{FOLLOWPHASE_BIT, NtcipFollowPhaseConvert}
	};
	int i, num = sizeof(convert) / sizeof(ConvertStruct);
	
	if (ntcipToGbFlag == 0 || ntcipToGbFlag == 1)
		return;
	if (BIT(ntcipToGbFlag, PHASETURN_BIT) || BIT(ntcipToGbFlag, SPLIT_BIT))
		BIT_SET(ntcipToGbFlag, STAGE_TIMING_BIT);
	if (BIT(ntcipToGbFlag, ACTION_BIT))
		BIT_SET(ntcipToGbFlag, TIMEINTERVAL_BIT);
	
	for (i = 0; i < num; i++)
	{
		if (BIT(ntcipToGbFlag, convert[i].bit))
			convert[i].func(gGbconfig, gSignalControlpara);
	}
	ntcipToGbFlag = 0;
	memmove(gGbconfig + 1, gGbconfig, sizeof(GbConfig));
}

