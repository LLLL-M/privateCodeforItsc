#include <pthread.h>
#include <string.h>
#include "convert.h"
#include "platform.h"
#include "hikmsg.h"

#define OBJECT_INIT(type,member,choice,minval,maxval)	{\
	.baseOffset = offsetof(type, member),\
	.size = sizeof(((type *)0)->member),\
	.isCheck = choice,\
	.minValue = minval,\
	.maxValue = maxval,\
}
#define	OBJECT_INIT_WITH_RANGE(type,member,minval,maxval)	OBJECT_INIT(type,member,TRUE,minval,maxval)
#define	OBJECT_INIT_WITHOUT_RANGE(type,member)	OBJECT_INIT(type,member,FALSE,0,0)
#define SIMPLE_OBJECT_INIT_WITH_RANGE(member,minval,maxval,id,bit)  {\
    .object = OBJECT_INIT_WITH_RANGE(GbConfig,member,minval,maxval),\
    .objectId = id,\
    .ntcipUpdateBit = bit,\
}
#define SIMPLE_OBJECT_INIT_WITHOUT_RANGE(member,id,bit) {\
    .object = OBJECT_INIT_WITHOUT_RANGE(GbConfig,member),\
    .objectId = id,\
    .ntcipUpdateBit = bit,\
}
#define COMPLEX_OBJECT_INIT(member,id,membertype,bit,childnum,childobjectset...)    {\
    .object = {\
                .baseOffset = offsetof(GbConfig, member),\
                .size = sizeof(membertype),\
                .isCheck = FALSE,\
              },\
    .objectId = id,\
    .maxIndex = sizeof(((GbConfig *)0)->member) / sizeof(membertype),\
    .childObjectNum = childnum,\
    .childObject = {childobjectset},\
    .ntcipUpdateBit = (UInt8)bit,\
}

#define	LOCAL_STANDARD_ZONE_SEC	(8 * 3600)	//本地标准时区，默认比UTC时间多了8个小时

extern GbConfig *gGbconfig;					//全局国标配置指针
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;   //倒计时接口信息
extern pthread_rwlock_t gCountDownLock;
extern UInt8 gCurTimeIntervalId;	//当前所使用的时段表
extern UInt8 gCurSchemeId;	//当前所使用的方案

extern void SendMsgToStrategyControl(UInt8 schemeId, UInt8 stageNum);

GbObjectIdentify gObjectArray[] = {	//国标所有对象标识数组
	[0] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pubDevIdentifyParam,0x81,INVALID_BIT),
	[1] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pubModMaxLineNum,0x82,INVALID_BIT),
	[2] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pubSyncTime,0x83,INVALID_BIT),
	[3] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pubSyncFlag,0x84,INVALID_BIT),
	[4] = COMPLEX_OBJECT_INIT(modParamTable,0x85,GbModParamList,INVALID_BIT,10,
                              OBJECT_INIT_WITH_RANGE(GbModParamList,modNo,1,MAX_MOD_PARAM_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modDevNodeLen),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modDevNode),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modManufacturerLen),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modManufacturer),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modStyleLen),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modStyle),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modVersionLen),
                              OBJECT_INIT_WITHOUT_RANGE(GbModParamList,modVersion),
                              OBJECT_INIT_WITH_RANGE(GbModParamList,modType,1,3)),
	[5] = SIMPLE_OBJECT_INIT_WITH_RANGE(pubTime,0,0x7fffffff,0x86,INVALID_BIT),
	[6] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(standardTimeZone,0x87,INVALID_BIT),
	[7] = SIMPLE_OBJECT_INIT_WITH_RANGE(localTime,0,0x7fffffff,0x88,INVALID_BIT),
	[8] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxScheduleListNum,1,MAX_SCHEDULE_LIST_NUM,0x89,INVALID_BIT),
	[9] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxTimeIntervalListNum,1,MAX_TIMEINTERVAL_LIST_NUM,0x8A,INVALID_BIT),
	[10] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxTimeIntervalNum,1,MAX_TIMEINTERVAL_NUM,0x8B,INVALID_BIT),
	[11] = SIMPLE_OBJECT_INIT_WITH_RANGE(activeTimeIntervalListNo,0,MAX_TIMEINTERVAL_LIST_NUM,0x8C,INVALID_BIT),
	[12] = COMPLEX_OBJECT_INIT(scheduleTable,0x8D,GbScheduleList,SCHEDULE_BIT,5,
                              OBJECT_INIT_WITH_RANGE(GbScheduleList,scheduleNo,1,MAX_SCHEDULE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbScheduleList,month),
                              OBJECT_INIT_WITHOUT_RANGE(GbScheduleList,week),
                              OBJECT_INIT_WITHOUT_RANGE(GbScheduleList,day),
                              OBJECT_INIT_WITHOUT_RANGE(GbScheduleList,timeIntervalListNo)),
	[13] = COMPLEX_OBJECT_INIT(timeIntervalTable,0x8E,GbTimeIntervalList,TIMEINTERVAL_BIT,8,
                              OBJECT_INIT_WITH_RANGE(GbTimeIntervalList,timeIntervalListNo,1,MAX_TIMEINTERVAL_LIST_NUM),
							  OBJECT_INIT_WITH_RANGE(GbTimeIntervalList,timeIntervalNo,1,MAX_TIMEINTERVAL_NUM),
							  OBJECT_INIT_WITH_RANGE(GbTimeIntervalList,hour,0,23),
							  OBJECT_INIT_WITH_RANGE(GbTimeIntervalList,minute,0,59),
							  OBJECT_INIT_WITH_RANGE(GbTimeIntervalList,controlMode,0,13),
                              OBJECT_INIT_WITHOUT_RANGE(GbTimeIntervalList,schemeId),
                              OBJECT_INIT_WITHOUT_RANGE(GbTimeIntervalList,assistFunctionOutput),
                              OBJECT_INIT_WITHOUT_RANGE(GbTimeIntervalList,specialFunctionOutput)),
	[14] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxEventTypeListNum,1,MAX_EVENT_TYPE_LIST_NUM,0x8F,INVALID_BIT),
	[15] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxEventLogListNum,1,MAX_EVENT_LOG_LIST_NUM,0x90,INVALID_BIT),
	[16] = COMPLEX_OBJECT_INIT(eventTypeTable,0x91,GbEventTypeList,INVALID_BIT,5,
                              OBJECT_INIT_WITH_RANGE(GbEventTypeList,eventTypeNo,1,MAX_EVENT_TYPE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbEventTypeList,eventTypeClearTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbEventTypeList,eventTypeDescLen),
                              OBJECT_INIT_WITHOUT_RANGE(GbEventTypeList,eventTypeDesc),
							  OBJECT_INIT_WITHOUT_RANGE(GbEventTypeList,eventTypeLineNo)),
	[17] = COMPLEX_OBJECT_INIT(eventLogTable,0x92,GbEventLogList,INVALID_BIT,4,
                              OBJECT_INIT_WITH_RANGE(GbEventLogList,eventTypeNo,1,MAX_EVENT_TYPE_LIST_NUM),
							  OBJECT_INIT_WITH_RANGE(GbEventLogList,eventStreamNo,1,MAX_EVENT_LOG_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbEventLogList,eventCheckedTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbEventLogList,eventValue)),
	[18] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxPhaseListNum,1,MAX_PHASE_LIST_NUM,0x93,INVALID_BIT),
	[19] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxPhaseStatusGroupNum,1,MAX_PHASE_STATUS_GROUP_NUM,0x94,INVALID_BIT),
	[20] = COMPLEX_OBJECT_INIT(phaseTable,0x95,GbPhaseList,PHASE_BIT,12,
                              OBJECT_INIT_WITH_RANGE(GbPhaseList,phaseNo,1,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,pedestrianPassTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,pedestrianClearTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,minGreen),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,unitExtendGreen),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,maxGreen_1),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,maxGreen_2),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,fixGreen),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,greenBlinkTime),
							  OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,phaseType),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,phaseOption),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseList,extendField)),
	[21] = COMPLEX_OBJECT_INIT(phaseStatusTable,0x96,GbStatusGroup,INVALID_BIT,4,
                              OBJECT_INIT_WITH_RANGE(GbStatusGroup,statusGroupNo,1,MAX_PHASE_STATUS_GROUP_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,redStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,yellowStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,greenStatus)),
	[22] = COMPLEX_OBJECT_INIT(phaseConflictTable,0x97,GbPhaseConflictList,CONFLICTPHASE_BIT,2,
                              OBJECT_INIT_WITH_RANGE(GbPhaseConflictList,phaseConflictNo,1,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbPhaseConflictList,conflictPhase)),
	[23] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxVehDetectorNum,1,MAX_VEH_DETECTOR_NUM,0x98,INVALID_BIT),
	[24] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxDetectorStatusGroupNum,1,MAX_DETECTOR_STATUS_LIST_NUM,0x99,INVALID_BIT),
	[25] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(detectDateStreamNo,0x9A,INVALID_BIT),
	[26] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(dataCollectCycle,0x9B,UNIT_BIT),
	[27] = SIMPLE_OBJECT_INIT_WITH_RANGE(activeDetectorNum,0,MAX_VEH_DETECTOR_NUM,0x9C,INVALID_BIT),
	[28] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pulseDataStreamNo,0x9D,INVALID_BIT),
	[29] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pulseDataCollectCycle,0x9E,INVALID_BIT),
	[30] = COMPLEX_OBJECT_INIT(vehDetectorTable,0x9F,GbVehDetectorList,VEHDETECTOR_BIT,8,
                              OBJECT_INIT_WITH_RANGE(GbVehDetectorList,detectorNo,1,MAX_VEH_DETECTOR_NUM),
							  OBJECT_INIT_WITH_RANGE(GbVehDetectorList,requestPhase,0,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorList,detectorType),
							  OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorList,detectorDirection),
							  OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorList,detectorRequestValidTime),
							  OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorList,detectorOption),
							  OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorList,laneFullFlow),
                              OBJECT_INIT_WITH_RANGE(GbVehDetectorList,laneFullRate,0,200)),
	[31] = COMPLEX_OBJECT_INIT(detectorStatusTable,0xA0,GbDetectorStatusList,INVALID_BIT,3,
                              OBJECT_INIT_WITH_RANGE(GbDetectorStatusList,detectorStatusNo,1,MAX_DETECTOR_STATUS_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbDetectorStatusList,detectorStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbDetectorStatusList,detectorStatusAlarm)),
	[32] = COMPLEX_OBJECT_INIT(trafficDetectDataTable,0xA1,GbTrafficDetectDataList,INVALID_BIT,7,
                              OBJECT_INIT_WITH_RANGE(GbTrafficDetectDataList,detectorNo,1,MAX_VEH_DETECTOR_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,totalFlow),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,largeVehFlow),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,smallVehFlow),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,rate),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,speed),
                              OBJECT_INIT_WITHOUT_RANGE(GbTrafficDetectDataList,vehBodyLen)),
	[33] = COMPLEX_OBJECT_INIT(vehDetectorAlarmTable,0xA2,GbVehDetectorAlarmList,INVALID_BIT,3,
                              OBJECT_INIT_WITH_RANGE(GbVehDetectorAlarmList,detectorNo,1,MAX_VEH_DETECTOR_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorAlarmList,detectorAlarmStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbVehDetectorAlarmList,inductiveCoilAlarmStatus)),
	[34] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(bootBlinkTime,0xA3,UNIT_BIT),
	[35] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(bootAllRedTime,0xA4,UNIT_BIT),
	[36] = SIMPLE_OBJECT_INIT_WITH_RANGE(controlStatus,1,6,0xA5,INVALID_BIT),
	[37] = SIMPLE_OBJECT_INIT_WITH_RANGE(flashControlMode,1,7,0xA6,INVALID_BIT),
	[38] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(deviceAlarm2,0xA7,INVALID_BIT),
	[39] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(deviceAlarm1,0xA8,INVALID_BIT),
	[40] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(deviceAlarmSummary,0xA9,INVALID_BIT),
	[41] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(allowRemoteActivate,0xAA,UNIT_BIT),
	[42] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(flashFrequency,0xAB,UNIT_BIT),
	[43] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(brightnessControlStartTime,0xAC,INVALID_BIT),
	[44] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(brightnessControlEndTime,0xAD,INVALID_BIT),
	[45] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxSupportChannelNum,1,MAX_CHANNEL_LIST_NUM,0xAE,INVALID_BIT),
	[46] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxChannelStatusGroupNum,1,MAX_CHANNEL_STATUS_GROUP_NUM,0xAF,INVALID_BIT),
	[47] = COMPLEX_OBJECT_INIT(channelTable,0xB0,GbChannelList,CHANNEL_BIT,4,
                              OBJECT_INIT_WITH_RANGE(GbChannelList,channelNo,1,MAX_CHANNEL_LIST_NUM),
							  OBJECT_INIT_WITH_RANGE(GbChannelList,channelRelatedPhase,0,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbChannelList,channelFlashStatus),
							  OBJECT_INIT_WITHOUT_RANGE(GbChannelList,channelControlType)),
	[48] = COMPLEX_OBJECT_INIT(channelStatusTable,0xB1,GbStatusGroup,INVALID_BIT,4,
                              OBJECT_INIT_WITH_RANGE(GbStatusGroup,statusGroupNo,1,MAX_CHANNEL_STATUS_GROUP_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,redStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,yellowStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,greenStatus)),
	[49] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxSchemeNum,1,MAX_SCHEME_LIST_NUM,0xB2,INVALID_BIT),
	[50] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxStageTimingNum,1,MAX_STAGE_TIMING_LIST_NUM,0xB3,INVALID_BIT),
	[51] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxStageNum,1,MAX_STAGE_NUM,0xB4,INVALID_BIT),
	[52] = SIMPLE_OBJECT_INIT_WITH_RANGE(manualControlScheme,0, 254, 0xB5,INVALID_BIT),
	[53] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(systemControlScheme,0xB6,INVALID_BIT),
	[54] = SIMPLE_OBJECT_INIT_WITH_RANGE(controlMode,0,13,0xB7,INVALID_BIT),
	[55] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(pubCycleTime,0xB8,INVALID_BIT),
	[56] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(coordinatePhaseGap,0xB9,INVALID_BIT),
	[57] = SIMPLE_OBJECT_INIT_WITH_RANGE(stageStatus,0,MAX_STAGE_NUM,0xBA,INVALID_BIT),
	[58] = SIMPLE_OBJECT_INIT_WITH_RANGE(stepCommand,0,MAX_STAGE_NUM,0xBB,INVALID_BIT),
	[59] = SIMPLE_OBJECT_INIT_WITH_RANGE(demotionMode,0,13,0xBC,INVALID_BIT),
	[60] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(demotionStandardSchemeTable,0xBD,INVALID_BIT),
	[61] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(stageTime,0xBE,INVALID_BIT),
	[62] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(keyPhaseGreenTime,0xBF,INVALID_BIT),
	[63] = COMPLEX_OBJECT_INIT(schemeTable,0xC0,GbSchemeList,SCHEME_BIT,5,
                              OBJECT_INIT_WITH_RANGE(GbSchemeList,schemeNo,1,MAX_SCHEME_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbSchemeList,cycleTime),
							  OBJECT_INIT_WITHOUT_RANGE(GbSchemeList,phaseGap),
							  OBJECT_INIT_WITH_RANGE(GbSchemeList,coordinatePhase,0,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITH_RANGE(GbSchemeList,stageTimingNo,0,MAX_STAGE_TIMING_LIST_NUM)),
	[64] = COMPLEX_OBJECT_INIT(stageTimingTable,0xC1,GbStageTimingList,STAGE_TIMING_BIT,7,
                              OBJECT_INIT_WITH_RANGE(GbStageTimingList,stageTimingNo,1,MAX_STAGE_TIMING_LIST_NUM),
							  OBJECT_INIT_WITH_RANGE(GbStageTimingList,stageNo,1,MAX_STAGE_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbStageTimingList,phaseNo),
                              OBJECT_INIT_WITHOUT_RANGE(GbStageTimingList,greenTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbStageTimingList,yellowTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbStageTimingList,allRedTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbStageTimingList,stageOption)),
	[65] = SIMPLE_OBJECT_INIT_WITH_RANGE(downloadFlag,1,2,0xC2,INVALID_BIT),
	[66] = SIMPLE_OBJECT_INIT_WITHOUT_RANGE(controlHostOption,0xC3,INVALID_BIT),
	[67] = SIMPLE_OBJECT_INIT_WITH_RANGE(deviceBaseAddr,0,8192,0xC4,INVALID_BIT),
	[68] = SIMPLE_OBJECT_INIT_WITH_RANGE(crossingNum,1,8,0xC5,INVALID_BIT),
	[69] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxFollowPhaseListNum,1,MAX_FOLLOW_PHASE_LIST_NUM,0xC6,INVALID_BIT),
	[70] = SIMPLE_OBJECT_INIT_WITH_RANGE(maxFollowPhaseStatusNum,0,MAX_FOLLOW_PHASE_STATUS_GROUP_NUM,0xC7,INVALID_BIT),
	[71] = COMPLEX_OBJECT_INIT(followPhaseTable,0xC8,GbFollowPhaseList,FOLLOWPHASE_BIT,9,
                              OBJECT_INIT_WITH_RANGE(GbFollowPhaseList,followPhaseNo,1,MAX_FOLLOW_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,operateType),
                              OBJECT_INIT_WITH_RANGE(GbFollowPhaseList,motherPhaseNum,0,MAX_PHASE_LIST_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,motherPhase),
                              OBJECT_INIT_WITH_RANGE(GbFollowPhaseList,correctPhaseNum,0,MAX_PHASE_LIST_NUM),
							  OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,correctPhase),
                              OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,tailGreenTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,tailYellowTime),
                              OBJECT_INIT_WITHOUT_RANGE(GbFollowPhaseList,tailAllRedTime)),
	[72] = COMPLEX_OBJECT_INIT(followPhaseStatusTable,0xC9,GbStatusGroup,INVALID_BIT,4,
                              OBJECT_INIT_WITH_RANGE(GbStatusGroup,statusGroupNo,1,MAX_FOLLOW_PHASE_STATUS_GROUP_NUM),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,redStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,yellowStatus),
                              OBJECT_INIT_WITHOUT_RANGE(GbStatusGroup,greenStatus)),
};

void GbPubTimeDeal(GbOperateType type)
{
	time_t tim;
	if (type == GB_QUERY_REQ)
	{
		gGbconfig->pubTime = LITTLE32_TO_BIG32((UInt32)time(NULL));
	}
	else if (type == GB_SET_REQ || type == GB_SET_NO_REPONSE)
	{
		tim = (time_t)BIG32_TO_LITTLE32(gGbconfig->pubTime);
		stime(&tim);
	}
}

void GbLocalTimeDeal(GbOperateType type)
{
	time_t tim;
	if (type == GB_QUERY_REQ)
	{
		gGbconfig->localTime = LITTLE32_TO_BIG32((UInt32)time(NULL) + BIG32_TO_LITTLE32(gGbconfig->standardTimeZone));
	}
	else if (type == GB_SET_REQ || type == GB_SET_NO_REPONSE)
	{
		tim = (time_t)(BIG32_TO_LITTLE32(gGbconfig->localTime) - BIG32_TO_LITTLE32(gGbconfig->standardTimeZone));
		stime(&tim);
	}
}

void GbActiveTimeDeal(GbOperateType type)
{
	if (type == GB_QUERY_REQ)
		gGbconfig->activeTimeIntervalListNo = gCurTimeIntervalId;
}

extern void PrintVehCountDown();
void GbPhaseStatusDeal(GbOperateType type)
{
	int i, j;
	
	if (type != GB_QUERY_REQ)
		return;
	memset(gGbconfig->phaseStatusTable, 0, sizeof(gGbconfig->phaseStatusTable));
	pthread_rwlock_rdlock(&gCountDownLock);
	//PrintVehCountDown();
	for (i = 0; i < MAX_PHASE_STATUS_GROUP_NUM; i++)
	{
		for (j = 0; j < 8; j++)
		{
			switch (gCountDownParams.stVehPhaseCountingDown[i * 8 + j][0])
			{
				case GREEN: gGbconfig->phaseStatusTable[i].greenStatus |= (1 << j); break;
				case GREEN_BLINK: gGbconfig->phaseStatusTable[i].greenStatus |= (1 << j); break;
				case YELLOW: gGbconfig->phaseStatusTable[i].yellowStatus |= (1 << j); break;
				case YELLOW_BLINK: gGbconfig->phaseStatusTable[i].yellowStatus |= (1 << j); break;
				case RED: gGbconfig->phaseStatusTable[i].redStatus |= (1 << j); break;
				case RED_BLINK: gGbconfig->phaseStatusTable[i].redStatus |= (1 << j); break;
			}
		}
	}
	pthread_rwlock_unlock(&gCountDownLock);
}

void GbChannelStatusDeal(GbOperateType type)
{
	int i, j;
	UInt8 phase, status;
	
	if (type != GB_QUERY_REQ)
		return;
	memset(gGbconfig->channelStatusTable, 0, sizeof(gGbconfig->channelStatusTable));
	pthread_rwlock_rdlock(&gCountDownLock);
	for (i = 0; i < MAX_CHANNEL_STATUS_GROUP_NUM; i++)
	{
		for (j = 0; j < 8; j++)
		{
			phase = gGbconfig->channelTable[i * 8 + j].channelRelatedPhase;
			switch (gGbconfig->channelTable[i * 8 + j].channelControlType)
			{
				case MOTOR: status = gCountDownParams.stVehPhaseCountingDown[phase - 1][0]; break;
				case PEDESTRIAN: status = gCountDownParams.stPedPhaseCountingDown[phase - 1][0]; break;
				case FOLLOW: status = gCountDownParams.ucOverlap[phase - 1][0]; break;
				default: status = TURN_OFF; break;
			}
			switch (status)
			{
				case GREEN: gGbconfig->channelStatusTable[i].greenStatus |= (1 << j); break;
				case GREEN_BLINK: gGbconfig->channelStatusTable[i].greenStatus |= (1 << j); break;
				case YELLOW: gGbconfig->channelStatusTable[i].yellowStatus |= (1 << j); break;
				case YELLOW_BLINK: gGbconfig->channelStatusTable[i].yellowStatus |= (1 << j); break;
				case RED: gGbconfig->channelStatusTable[i].redStatus |= (1 << j); break;
				case RED_BLINK: gGbconfig->channelStatusTable[i].redStatus |= (1 << j); break;
			}
		}
	}
	pthread_rwlock_unlock(&gCountDownLock);
}

void GbManualControlDeal(GbOperateType type)
{
	UInt8 schemeId;
	
	if (type == GB_QUERY_REQ)
	{
		pthread_rwlock_rdlock(&gCountDownLock);
		switch (gCountDownParams.ucPlanNo)
		{
			case YELLOWBLINK_SCHEMEID: gGbconfig->manualControlScheme = GB_YELLOWFLASH_SCHEME; break;
			case ALLRED_SCHEMEID: gGbconfig->manualControlScheme = GB_ALLRED_SCHEME; break;
			case INDUCTIVE_SCHEMEID: gGbconfig->manualControlScheme = 1; break;
			case TURNOFF_SCHEMEID: gGbconfig->manualControlScheme = GB_TURNOFF_SCHEME; break;
			default: gGbconfig->manualControlScheme = gCountDownParams.ucPlanNo; break;
		}
		pthread_rwlock_unlock(&gCountDownLock);
	}
}

void GbSystemControlDeal(GbOperateType type)
{
	UInt8 schemeId;
	
	if ((type == GB_SET_REQ || type == GB_SET_NO_REPONSE) && gGbconfig->controlMode == 0)
	{
		switch (gGbconfig->systemControlScheme)
		{
			case 1 ... MAX_SCHEME_LIST_NUM: schemeId = gGbconfig->systemControlScheme; break;
			//case GB_INDUCTIVE_SCHEME: schemeId = INDUCTIVE_SCHEMEID; break;
			case GB_ALLRED_SCHEME: schemeId = ALLRED_SCHEMEID; break;
			case GB_YELLOWFLASH_SCHEME: schemeId = YELLOWBLINK_SCHEMEID; break;
			case GB_TURNOFF_SCHEME: schemeId = TURNOFF_SCHEMEID; break;
			case 0: schemeId = 0; break; //系统控制
			default: ERR("manual scheme %d invalid", gGbconfig->systemControlScheme); return;
		}
		SendMsgToStrategyControl(schemeId, 0);
	}
}

void GbControlModeDeal(GbOperateType type)
{
	UInt8 schemeId;
	
	if (type == GB_QUERY_REQ)
	{
		pthread_rwlock_rdlock(&gCountDownLock);
		switch (gCountDownParams.ucPlanNo)
		{
			case YELLOWBLINK_SCHEMEID: gGbconfig->controlMode = YELLOWBLINK_MODE; break;
			case ALLRED_SCHEMEID: gGbconfig->controlMode = ALLRED_MODE; break;
			case INDUCTIVE_SCHEMEID: gGbconfig->controlMode = INDUCTIVE_MODE; break;
			case TURNOFF_SCHEMEID: gGbconfig->controlMode = TURNOFF_LIGHTS_MODE; break;
			default: gGbconfig->controlMode = 0; break;
		}
		pthread_rwlock_unlock(&gCountDownLock);
	}
	else if (type == GB_SET_REQ || type == GB_SET_NO_REPONSE)
	{
		switch (gGbconfig->controlMode)
		{
			case INDUCTIVE_MODE: schemeId = INDUCTIVE_SCHEMEID; break;
			case ALLRED_MODE: schemeId = ALLRED_SCHEMEID; break;
			case YELLOWBLINK_MODE: schemeId = YELLOWBLINK_SCHEMEID; break;
			case TURNOFF_LIGHTS_MODE: schemeId = TURNOFF_SCHEMEID; break;
			case 0: schemeId = 0; break;	//系统控制
			default: ERR("The control mode %d isn't supported!", gGbconfig->controlMode); return;
		}
		SendMsgToStrategyControl(schemeId, 0);
	}
}

void GbStepControlDeal(GbOperateType type)
{
	if (type != GB_SET_REQ && type != GB_SET_NO_REPONSE)
		return;
	SendMsgToStrategyControl(STEP_SCHEMEID, gGbconfig->stepCommand);
}

void GbCurSchemeStageTimeDeal(GbOperateType type)
{
	int i;
	UInt8 stageTimingNo = 0;
	GbStageTimingList *list = NULL;
	
	if (type != GB_QUERY_REQ)
		return;
	memset(gGbconfig->stageTime, 0, sizeof(gGbconfig->stageTime));
	if (gCurSchemeId == 0 || gCurSchemeId > MAX_SCHEME_LIST_NUM)
		return;
	stageTimingNo = gGbconfig->schemeTable[gCurSchemeId - 1].stageTimingNo;
	if (stageTimingNo == 0 || stageTimingNo > MAX_STAGE_TIMING_LIST_NUM)
		return;
	list = gGbconfig->stageTimingTable[stageTimingNo - 1];
	for (i = 0; i < MAX_STAGE_NUM; i++)
	{
		gGbconfig->stageTime[i] = list[i].greenTime + list[i].yellowTime + list[i].allRedTime;
	}
}

void GbFollowPhaseStatusDeal(GbOperateType type)
{
	int i, j;
	
	if (type != GB_QUERY_REQ)
		return;
	memset(gGbconfig->followPhaseStatusTable, 0, sizeof(gGbconfig->followPhaseStatusTable));
	pthread_rwlock_rdlock(&gCountDownLock);
	for (i = 0; i < MAX_FOLLOW_PHASE_STATUS_GROUP_NUM; i++)
	{
		for (j = 0; j < 8; j++)
		{
			switch (gCountDownParams.ucOverlap[i * 8 + j][0])
			{
				case GREEN: gGbconfig->followPhaseStatusTable[i].greenStatus |= (1 << j); break;
				case GREEN_BLINK: gGbconfig->followPhaseStatusTable[i].greenStatus |= (1 << j); break;
				case YELLOW: gGbconfig->followPhaseStatusTable[i].yellowStatus |= (1 << j); break;
				case YELLOW_BLINK: gGbconfig->followPhaseStatusTable[i].yellowStatus |= (1 << j); break;
				case RED: gGbconfig->followPhaseStatusTable[i].redStatus |= (1 << j); break;
				case RED_BLINK: gGbconfig->followPhaseStatusTable[i].redStatus |= (1 << j); break;
			}
		}
	}
	pthread_rwlock_unlock(&gCountDownLock);
}

void GbInit()
{
	gGbconfig->pubModMaxLineNum = MAX_MOD_PARAM_LIST_NUM;
	gGbconfig->standardTimeZone = LITTLE32_TO_BIG32(LOCAL_STANDARD_ZONE_SEC);
	gGbconfig->maxScheduleListNum = MAX_SCHEDULE_LIST_NUM;
	gGbconfig->maxTimeIntervalListNum = MAX_TIMEINTERVAL_LIST_NUM;
	gGbconfig->maxTimeIntervalNum = MAX_TIMEINTERVAL_NUM;
	gGbconfig->maxEventTypeListNum = MAX_EVENT_TYPE_LIST_NUM;
	gGbconfig->maxEventLogListNum = MAX_EVENT_LOG_LIST_NUM;
	gGbconfig->maxPhaseListNum = MAX_PHASE_LIST_NUM;
	gGbconfig->maxPhaseStatusGroupNum = MAX_PHASE_STATUS_GROUP_NUM;
	gGbconfig->maxVehDetectorNum = MAX_VEH_DETECTOR_NUM;
	gGbconfig->maxDetectorStatusGroupNum = MAX_DETECTOR_STATUS_LIST_NUM;
	gGbconfig->maxSupportChannelNum = MAX_CHANNEL_LIST_NUM;
	gGbconfig->maxChannelStatusGroupNum = MAX_CHANNEL_STATUS_GROUP_NUM;
	gGbconfig->maxSchemeNum = MAX_SCHEME_LIST_NUM;
	gGbconfig->maxStageTimingNum = MAX_STAGE_TIMING_LIST_NUM;
	gGbconfig->maxStageNum = MAX_STAGE_NUM;
	gGbconfig->crossingNum = 1;
	gGbconfig->maxFollowPhaseListNum = MAX_FOLLOW_PHASE_LIST_NUM;
	gGbconfig->maxFollowPhaseStatusNum = MAX_FOLLOW_PHASE_STATUS_GROUP_NUM;
	gGbconfig->flashControlMode = 7;	//启动时闪光控制
	//gGbconfig->controlStatus = 2;	//系统协调控制
	gGbconfig->deviceAlarm1 = (1 << 6);	//本地单点控制
	gGbconfig->deviceAlarmSummary = 0;

	gGbconfig->modParamTable[0].modNo = 1;
	gGbconfig->modParamTable[0].modDevNodeLen = 6;
	memcpy(gGbconfig->modParamTable[0].modDevNode, "123456", 6);
	gGbconfig->modParamTable[0].modManufacturerLen = 9;
	memcpy(gGbconfig->modParamTable[0].modManufacturer, "hikvision", 9);
	gGbconfig->modParamTable[0].modStyleLen = 25;
	memcpy(gGbconfig->modParamTable[0].modStyle, "traffic signal controller", 25);
	gGbconfig->modParamTable[0].modVersionLen = 3;
	memcpy(gGbconfig->modParamTable[0].modVersion, "2.0", 3);
	gGbconfig->modParamTable[0].modType = 3;

	gObjectArray[0x86 - 0x81].func = GbPubTimeDeal;
	gObjectArray[0x88 - 0x81].func = GbLocalTimeDeal;
	gObjectArray[0x8C - 0x81].func = GbActiveTimeDeal;
	gObjectArray[0x96 - 0x81].func = GbPhaseStatusDeal;
	gObjectArray[0xB1 - 0x81].func = GbChannelStatusDeal;
	gObjectArray[0xB5 - 0x81].func = GbManualControlDeal;
	gObjectArray[0xB6 - 0x81].func = GbSystemControlDeal;
	gObjectArray[0xB7 - 0x81].func = GbControlModeDeal;
	gObjectArray[0xBB - 0x81].func = GbStepControlDeal;
	gObjectArray[0xBE - 0x81].func = GbCurSchemeStageTimeDeal;
	gObjectArray[0xC9 - 0x81].func = GbFollowPhaseStatusDeal;
}
