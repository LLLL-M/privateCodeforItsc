/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : calculate.c
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月27日
  最近修改   :
  功能描述   : 策略控制模块，主要控制点灯逻辑
  函数列表   :
              GetActionID
              GetPhaseTurnAndSetPhaseTimeInfo
              GetTimeIntervalID
			  GetPhaseStatusByTime
              InitChannelLightStatus
              IsPhaseInPhaseTurn
              RunCycle
              SetCurrentChannelStatus
              SetPhaseTime
              SignalControllerRun
  修改历史   :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "hikmsg.h"
#include "lfq.h"
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //把时分转换成分
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //一天中的最大时间差值，即24小时

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern int msgid;
extern void *gHandle;

extern pthread_rwlock_t gConfigLock;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void CalStageInfo(void);
extern void IgnorePhaseDeal(void);
extern void TransitionDeal(void);
extern Boolean CalculateByGbconfig(struct msgbuf *msg);
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

CalInfo gCalInfo;
CustomInfo gCustomInfo = {
	.checkTime = 3,		//默认感应检测时间为3s
	.ignoreOption = 1,	//默认忽略相位向前忽略
};

/*****************************************************************************
 函 数 名  : SetPhaseTime
 功能描述  : 根据绿信比号初始化每个相位的各个时间段，并设置协调相位号和忽略相位属性
 输入参数  : UInt8  nGreenSignalRatioID  绿信比号
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void SetPhaseTime(void)
{  
    int i = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;

    for(i = 0 ; i < NUM_PHASE; i++)
    {
		splitItem = &gRunConfigPara->stGreenSignalRation[gCalInfo.splitId - 1][i];
		phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i];
		if ((phaseItem->nCircleID == 0) || (splitItem->nGreenSignalRationTime == 0))
			continue;
		if (splitItem->nIsCoordinate)
		{	//说明配置了协调相位，此时如果没有其他特殊控制应该执行协调控制
			if (gCalInfo.coordinatePhaseId == 0)	//配置多个协调相位时，以最先找到的为准
				gCalInfo.coordinatePhaseId = splitItem->nPhaseID;
			SET_BIT(gCalInfo.isCoordinatePhase, i);
		}
		if (splitItem->nType == IGNORE_PHASE)
		{
			SET_BIT(gCalInfo.isIgnorePhase, i);
		}

		phaseTimes[i].splitTime = splitItem->nGreenSignalRationTime;
		phaseTimes[i].greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		phaseTimes[i].greenBlinkTime = entry->nGreenLightTime;
		phaseTimes[i].yellowTime = phaseItem->nYellowTime;
		phaseTimes[i].allRedTime = phaseItem->nAllRedTime;
    }
}

/*****************************************************************************
 函 数 名  : GetTimeIntervalID
 功能描述  : 遍历调度表，根据当前时间，确定当前运行时段表号
 输入参数  : struct tm *now     当前时间指针
 输出参数  : 
 返 回 值  : 当前运行的时段表号
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static UInt8 GetTimeIntervalID(struct tm *now)
{
    int i = 0, schduleNum = GET_PROTOCOL_PARAM(NUM_SCHEDULE, MAX_SCHEDULE_LIST_NUM);
	UInt16 month;
	UInt8 week;
	UInt32 day;

	for(i = 0 ; i < schduleNum ; i++) { //如果当前时间和某一项调度表相符，则返回该调度表的时段号
		month = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].month, gRunGbconfig->scheduleTable[i].month);
		day = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].day, gRunGbconfig->scheduleTable[i].day);
		week = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].week, gRunGbconfig->scheduleTable[i].week);
        if (BIT(month, now->tm_mon + 1) == 1)
		{	//tm_mon范围是[0,11]
			if ((BIT(day, now->tm_mday) == 1)
				|| (BIT(week, now->tm_wday + 1) == 1))//tm_wday == 0代表星期日
			    return GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].nTimeIntervalID, gRunGbconfig->scheduleTable[i].timeIntervalListNo);
		}
    }

    return 0;
}



/*****************************************************************************
 函 数 名  : GetSchemeIdAndTimeGap
 功能描述  : 根据当前时间，判断应属于哪个时段，进而确定当前时段需要执行的方案
 输入参数  : struct tm *now			当前时间指针
			 int *ret				用于返回当前时间与所在时段起始时间的差值
 输出参数  : 无
 返 回 值  : 当前运行的动作号

 修改历史      :
  1.日    期   : 2014年11月20日
    作    者   : Jicky
    修改内容   : 修改了输入参数，并重新修改了函数实现

*****************************************************************************/
UInt8 GetSchemeIdAndTimeGap(struct tm *now, int *ret)
{
	UInt8 hour, minute;
    int i = 0, index = -1;
	int maxTimeintervalNum = GET_PROTOCOL_PARAM(NUM_TIME_INTERVAL, MAX_TIMEINTERVAL_LIST_NUM);
	int maxTimeintervalIdNum = GET_PROTOCOL_PARAM(NUM_TIME_INTERVAL_ID, MAX_TIMEINTERVAL_NUM);
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nSchemeID = 0;
	
	nTimeIntervalID = GetTimeIntervalID(now);//根据当前时间，遍历调度表，得到时段表ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > maxTimeintervalNum)       
	{
		log_error("can't find timeinterval id");
        return 0;
	}
	gCalInfo.timeIntervalId = nTimeIntervalID;
    //此函数中的时间值单位为分钟
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//预先设置一个最大的差值
	do
	{	//循环找出当前时间与时段表中差值最小的时段所对应的actionID即为当前应该运行的actionID
		for(i= 0; i < maxTimeintervalIdNum; i++) 
		{
			if (gRunConfigPara != NULL && gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //这说明该时段可能没有被使用，直接continue掉
				continue;
			}
			
			hour = GET_PROTOCOL_PARAM(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour, gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][i].hour);
			minute = GET_PROTOCOL_PARAM(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute, gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][i].minute);
			nTempTime = HoursToMinutes(hour, minute);
			if (nCurrentTime == nTempTime) 
			{
				index = i;
				minTimeGap = 0;
			} 
			else if (nCurrentTime > nTempTime) 
			{
				nTimeGap = nCurrentTime - nTempTime;
				if (nTimeGap < minTimeGap) 
				{
					minTimeGap = nTimeGap;
					index = i;
				}
			}
		}
		
		if (nTempTime == -1)
		{
			log_error("the nTimeIntervalID %d isn't configed", nTimeIntervalID);
			return 0;	//说明此时段表没有配置
		}
		if (minTimeGap == MAX_TIME_GAP) 
		{ //说明当前时间处于时段表中最小的位置
			nCurrentTime += MAX_TIME_GAP;	//把当前时间增加24小时然后再次循环
		}
	} while (index == -1);
	if (ret != NULL)
	    *ret = minTimeGap;
	if (gRunConfigPara != NULL)
	{
		gCalInfo.actionId = gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;//根据时段表ID，得到动作表ID    
		if (gCalInfo.actionId == 0 || gCalInfo.actionId > NUM_ACTION)
		{
			log_error("can't find action id");
			return 0;
		}
		nSchemeID = gRunConfigPara->stAction[gCalInfo.actionId - 1].nSchemeID;
	}
	else if (gRunGbconfig != NULL)
	{
		switch (gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][index].controlMode)
		{
			case SYSTEM_MODE: 
				nSchemeID = gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][index].schemeId;
				break;
			case TURNOFF_LIGHTS_MODE: nSchemeID = TURNOFF_SCHEMEID; break;
			case YELLOWBLINK_MODE: nSchemeID = YELLOWBLINK_SCHEMEID; break;
			case ALLRED_MODE: nSchemeID = ALLRED_SCHEMEID; break;
			case INDUCTIVE_MODE: nSchemeID = INDUCTIVE_SCHEMEID; break;
			default: nSchemeID = 1;	//国标的其他控制方式未实现，默认使用方案1
		}
	}
	return nSchemeID;
}

static void SetInductiveControlTime(void)
{
    int i = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;

    for(i = 0; i < NUM_PHASE; i++)
    {
		phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i];
		if (phaseItem->nCircleID == 0)
			continue;
		phaseTimes[i].splitTime = phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime;
		phaseTimes[i].greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
		phaseTimes[i].greenBlinkTime = entry->nGreenLightTime;
		phaseTimes[i].yellowTime = phaseItem->nYellowTime;
		phaseTimes[i].allRedTime = phaseItem->nAllRedTime;
    }
	//设置周期
	gCalInfo.cycleTime = 0;
	for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{	//感应控制默认使用相序表1
			nPhaseId = gRunConfigPara->stPhaseTurn[0][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
				break;
			gCalInfo.cycleTime += phaseTimes[nPhaseId - 1].splitTime;
		}
		if (gCalInfo.cycleTime > 0)
			break;
	}
	//INFO("DEBUG inductive control: gCalInfo.cycleTime = %d", gCalInfo.cycleTime);
}

static void SendSpecialControlData(UInt8 schemeId, UInt8 leftTime)
{
    LineQueueData data;
    int i, channelNum = GET_PROTOCOL_PARAM(NUM_CHANNEL, MAX_CHANNEL_LIST_NUM);
    UInt8 phaseId = 0;
    LightStatus status = INVALID;
	UInt8 controllerType = 0;

	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: status = YELLOW_BLINK; break;	//黄闪控制
		case ALLRED_SCHEMEID: status = ALLRED; break;		//全红控制
		case TURNOFF_SCHEMEID: status = TURN_OFF; break;	//关灯控制
		default: break;
	}
    memset(&data, 0, sizeof(data));
    for (i = 0; i < channelNum; i++)
    {
		phaseId = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[0][i].nControllerID, gRunGbconfig->channelTable[i].channelRelatedPhase);
        if (phaseId == 0 || phaseId > NUM_PHASE)
            continue;
        
    	data.allChannels[i] = status;
		controllerType = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[0][i].nControllerType, gRunGbconfig->channelTable[i].channelControlType);
    	switch (controllerType)
    	{
    	    case MOTOR: data.phaseInfos[phaseId - 1].phaseStatus = status; break;
    	    case PEDESTRIAN: data.phaseInfos[phaseId - 1].pedestrianPhaseStatus = status; break;
    	    case FOLLOW: data.phaseInfos[phaseId - 1].followPhaseStatus = status; break;
    	}
    }
    data.schemeId = schemeId;
	data.leftTime = leftTime;
    while (0 != lfq_write(gHandle, &data));
}

#define UNKNOWN_CONTROL		0
#define SPERICAL_CONTROL 	1
#define GENERAL_CONTROL		2
static UInt8 GetPhaseTurnAndSetPhaseTimeInfo(UInt8 schemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//当前时刻与当前时段的起始时间的差值，单位为分钟

	localtime_r(&calTime, &now);
	memset(&gCalInfo, 0, sizeof(CalInfo));
    gCalInfo.schemeId = (schemeId > 0) ? schemeId : GetSchemeIdAndTimeGap(&now, &timeGapMinutes);
    if (gCalInfo.schemeId == YELLOWBLINK_SCHEMEID || gCalInfo.schemeId == ALLRED_SCHEMEID || gCalInfo.schemeId == TURNOFF_SCHEMEID)
	{
		//SendSpecialControlData(YELLOWBLINK_SCHEMEID, 0);
		log_debug("special control, scheme: %d", gCalInfo.schemeId);
        return SPERICAL_CONTROL;	//特殊控制返回最大相序号+1
	}
	else if (gCalInfo.schemeId == INDUCTIVE_SCHEMEID)	
    {	
    	gCalInfo.phaseTurnId = 1;	//感应控制默认使用相序表1
		gCalInfo.actionId = INDUCTIVE_ACTIONID;
		gCalInfo.channelTableId=1;
		gCalInfo.phaseTableId =1;
		
    	//如果是感应控制应当使用最小绿+黄灯时间+全红时间作为绿信比时间
        SetInductiveControlTime();

		return GENERAL_CONTROL;
    }

    if(gCalInfo.schemeId == 0 || gCalInfo.schemeId > NUM_SCHEME)
	{
		log_error("can't find scheme id, action: %d", gCalInfo.actionId);
        return UNKNOWN_CONTROL;
	}
    gCalInfo.splitId = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nGreenSignalRatioID;//根据方案表ID，得到绿信比表ID
    if(gCalInfo.splitId == 0 || gCalInfo.splitId > NUM_GREEN_SIGNAL_RATION)
	{
		log_error("can't find split id, scheme: %d", gCalInfo.schemeId);
        return UNKNOWN_CONTROL;
	}

	gCalInfo.cycleTime = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nCycleTime;
    gCalInfo.phaseTurnId = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nPhaseTurnID;//根据方案表ID，得到相序表ID
    if(gCalInfo.phaseTurnId == 0 || gCalInfo.phaseTurnId > NUM_PHASE_TURN)
	{
		log_error("can't find phase turn id, scheme: %d", gCalInfo.schemeId);
        return UNKNOWN_CONTROL;
	}
	if (gCalInfo.actionId == 0)	//方案1,4,7对应动作1，2，3
		gCalInfo.actionId = (gCalInfo.schemeId + 2) / 3;
	gCalInfo.phaseTableId = gRunConfigPara->stAction[gCalInfo.actionId - 1].nPhaseTableID;
	gCalInfo.channelTableId = gRunConfigPara->stAction[gCalInfo.actionId - 1].nChannelTableID;
	if (gCalInfo.phaseTableId == 0 || gCalInfo.phaseTableId > MAX_PHASE_TABLE_COUNT)
	{	//如果相位表号不合理则从之前旧的
		memcpy(gRunConfigPara->stPhase[0], gRunConfigPara->stOldPhase, sizeof(gRunConfigPara->stOldPhase));
		memcpy(gRunConfigPara->AscSignalTransTable[0], gRunConfigPara->OldAscSignalTransTable, sizeof(gRunConfigPara->OldAscSignalTransTable));
		memcpy(gRunConfigPara->stFollowPhase[0], gRunConfigPara->stOldFollowPhase, sizeof(gRunConfigPara->stOldFollowPhase));
		gCalInfo.phaseTableId = 1;
	}
	if (gCalInfo.channelTableId == 0 || gCalInfo.channelTableId > MAX_CHANNEL_TABLE_COUNT)
	{
		memcpy(gRunConfigPara->stChannel[0], gRunConfigPara->stOldChannel, sizeof(gRunConfigPara->stOldChannel));
		gCalInfo.channelTableId = 1;
	}
	//根据当前的绿信比表ID，给相位表中各相位的绿信比时间段赋值，并同时找出协调相位
    SetPhaseTime();
	gCalInfo.timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	return GENERAL_CONTROL;
}

void SetCurrentChannelStatus(PhaseInfo *phaseInfos, UInt8 *allChannel)
{    
    int i = 0, channelNum = GET_PROTOCOL_PARAM(NUM_CHANNEL, MAX_CHANNEL_LIST_NUM);
	LightStatus status;
	UInt8 phaseId, leftTime;
	UInt8 pedestrianPhase[NUM_PHASE][2];	//行人相位的状态和倒计时，[0]:状态，[1]:倒计时
	UInt8 redFlashSec = gCustomInfo.redFlashSec;	//红灯闪烁秒数
	UInt8 isYellowFlash = gCustomInfo.isYellowFlash;	//黄灯时是否闪烁

	memset(pedestrianPhase, 0, sizeof(pedestrianPhase));
    for(i = 0; i < channelNum; i++)
    {
		phaseId = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[gCalInfo.channelTableId - 1][i].nControllerID, gRunGbconfig->channelTable[i].channelRelatedPhase);
		if (phaseId == 0)
		{
			allChannel[i] = INVALID;
			continue;
		}	
		
		switch (GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[gCalInfo.channelTableId - 1][i].nControllerType, gRunGbconfig->channelTable[i].channelControlType)) 
		{
			case MOTOR: 
				status = phaseInfos[phaseId - 1].phaseStatus; 
				leftTime = phaseInfos[phaseId - 1].phaseLeftTime; 
				break;
			case PEDESTRIAN: 
				status = phaseInfos[phaseId - 1].pedestrianPhaseStatus; 
				pedestrianPhase[phaseId - 1][0] = phaseInfos[phaseId - 1].pedestrianPhaseStatus;
				pedestrianPhase[phaseId - 1][1] = phaseInfos[phaseId - 1].pedestrianPhaseLeftTime;
				leftTime = phaseInfos[phaseId - 1].pedestrianPhaseLeftTime;
				break;
			case FOLLOW: 
				status = phaseInfos[phaseId - 1].followPhaseStatus; 
				leftTime = phaseInfos[phaseId - 1].followPhaseLeftTime;  
				break;
			default: status = INVALID; break;
		}
		if (status == RED && redFlashSec >= leftTime)
			status = RED_BLINK;
		else if (status == YELLOW && isYellowFlash == 1)
			status = YELLOW_BLINK;
		allChannel[i] = (UInt8)status;
    }
	//更新行人相位状态和倒计时，因为之前在计算机动车相位时把对应的行人相位也计算了，有可能根本没有用到
	for (i = 0; i < NUM_PHASE; i++)
	{
		phaseInfos[i].pedestrianPhaseStatus = pedestrianPhase[i][0];
		phaseInfos[i].pedestrianPhaseLeftTime = pedestrianPhase[i][1];
	}
}

static inline void SetpedestrianPhaseStatusAndTime(PhaseTimeInfo *times, UInt8 runTime, UInt8 startTime, PhaseInfo *info)
{
	LightStatus status = RED;
	if (times->pedestrianPassTime) 
	{
		times->pedestrianPassTime--;
		status = GREEN;
	}
	else
	{
		if (times->pedestrianClearTime) 
		{
			times->pedestrianClearTime--;
			status = GREEN_BLINK;
		}
	}
	info->pedestrianPhaseStatus = status;
	if (status == GREEN || status == GREEN_BLINK)	//行人绿灯倒计时
		info->pedestrianPhaseLeftTime = times->pedestrianPassTime + times->pedestrianClearTime + 1;
	else	//行人红灯倒计时
		info->pedestrianPhaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
}

static void SetPhaseInfo(UInt8 phaseId, UInt8 runTime, UInt8 startTime, PhaseInfo *phaseInfos)
{
	int i = 0;
	LightStatus status = RED;	//设置默认返回RED状态
	PhaseTimeInfo *times = &gCalInfo.phaseTimes[phaseId - 1];
	PhaseItem *phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1];
	PhaseInfo *info = &phaseInfos[phaseId - 1];
	int endTime = startTime + times->splitTime;
	
	if (startTime == endTime)
	{	//说明此相位已被忽略掉
		info->phaseStatus = TURN_OFF;
		info->pedestrianPhaseStatus = TURN_OFF;
		return;
	}
	info->pedestrianPhaseStatus = RED;	//行人相位默认为红色
	if (runTime >= startTime && runTime < endTime)
	{	//说明此时正在运行此相位的绿信比
		status = GetPhaseStatusByTime(times);
		SetpedestrianPhaseStatusAndTime(times, runTime, startTime, info);
		//最后加上1s是因为之前在GetPhaseStatusByTime函数中已经递减了1s
		if (status == GREEN || status == GREEN_BLINK)	//绿灯倒计时
		{
			info->phaseLeftTime = times->greenTime + times->greenBlinkTime + 1;
		}
		else if (status == YELLOW)	//黄灯倒计时
		{
			info->phaseLeftTime = times->yellowTime + 1;
		}	
		else	//红灯倒计时
		{
			info->phaseLeftTime = gCalInfo.cycleTime - times->splitTime + times->allRedTime + 1;
		}
		info->phaseSplitLeftTime = times->greenTime + times->greenBlinkTime + times->yellowTime + times->allRedTime + 1;
	}
	else if (runTime < startTime)
	{
		info->phaseLeftTime = startTime - runTime;
		info->pedestrianPhaseLeftTime = startTime - runTime;
	}
	else  //(runTime >= endTime)
	{
		info->phaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
		info->pedestrianPhaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
	}
	
	if (GET_BIT(gCalInfo.isIgnorePhase, phaseId - 1) == TRUE)
	{	//是忽略相位
		info->phaseStatus = TURN_OFF;
		info->pedestrianPhaseStatus = TURN_OFF;
	}
	else
		info->phaseStatus = status;
	info->splitTime = times->splitTime;
	if (gCalInfo.schemeId != INDUCTIVE_SCHEMEID)
		return;	//如果不是感应控制不需要下面的参数
	for (i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{	//轮询车检器配置信息查找相位对应的车检器号
		if (phaseId == gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase)
		{
			info->vehicleDetectorId = gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorNumber;
			break;
		}
	}
	info->unitExtendGreen = phaseItem->nUnitExtendGreen;
	info->maxExtendGreen = phaseItem->nMaxGreen_1 - (times->splitTime - phaseItem->nYellowTime - phaseItem->nAllRedTime);
}

/*****************************************************************************
 函 数 名  : RunCycle
 功能描述  : 通过传入的相序、所有通道的状态以及相位与通道的对应关系来运行一
             个周期
 输入参数  : unsigned short nPhaseTurn           当前要运行的相序
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void RunCycle(UInt8 nPhaseTurn)
{
	int t = 0, i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	LineQueueData data;
	UInt8 startTime = 0;	//相位执行时已经运行的周期时间
	UInt8 stageNum = 1;	//阶段号
	UInt8 stepRunTime = 0;	//阶段运行时间
	UInt8 checkTime = gCustomInfo.checkTime;	//感应检测时间

	for (t = 0; t < gCalInfo.cycleTime; t++) 
	{	//一次循环获取1秒钟各通道的状态以及其他信息发送给相位控制模块
		memset(&data, 0, sizeof(data));
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	//每循环一次从相序的四个环中设置这1s内所有通道的状态，以及相序中其他相位的信息
			startTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gRunConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[i];
				if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
					break;
				//设置相位的相关信息
				SetPhaseInfo(nPhaseId, (UInt8)t, startTime, data.phaseInfos);
				startTime += gCalInfo.phaseTimes[nPhaseId - 1].splitTime;
				if (t == startTime && startTime > stepRunTime)
				{
					stageNum++;
					stepRunTime = startTime;
				}
			}
		}
		SetFollowPhaseInfo(data.phaseInfos, stageNum);
		SetCurrentChannelStatus(data.phaseInfos, data.allChannels);
		data.cycleTime = gCalInfo.cycleTime;
		//data.runTime = t + 1;
		data.leftTime = data.cycleTime - t;
		data.schemeId = gCalInfo.schemeId;
		data.stageNum = stageNum;
		data.maxStageNum = gCalInfo.maxStageNum;
		data.checkTime = checkTime;
		data.phaseTableId = gCalInfo.phaseTableId;
		data.channelTableId = gCalInfo.channelTableId;
		while (0 != lfq_write(gHandle, &data));	//保证一定能把数据写入到队列中
	}
	INFO("max stage num is %d", stageNum);
}

static inline void YellowBlinkAllRedInit()
{
	UInt8 bootYellowFlashTime = GET_PROTOCOL_PARAM(gRunConfigPara->stUnitPara.nBootYellowLightTime, gRunGbconfig->bootBlinkTime);
	UInt8 bootAllRedTime = GET_PROTOCOL_PARAM(gRunConfigPara->stUnitPara.nBootAllRedTime, gRunGbconfig->bootAllRedTime);
	UInt8 firstCycle = bootYellowFlashTime + bootAllRedTime;
	struct msgbuf msg;
	int i;
	
	//把黄闪和全红作为第一个周期，把周期的剩余时间写入信息中便于相位控制模块SystemInit函数使用
	if (firstCycle == 0)
	{	//如果没有配置黄闪和全红的启动时间则首先发送1s的关灯数据便于相位控制模块SystemInit函数使用
		SendSpecialControlData(TURNOFF_SCHEMEID, 1);
	}
	else
	{
		//初始化为黄闪控制
		for (i = 0; i < bootYellowFlashTime; i++) 	
			SendSpecialControlData(YELLOWBLINK_SCHEMEID, firstCycle--);
		//初始化为全红控制
		for (i = 0; i < bootAllRedTime; i++) 
			SendSpecialControlData(ALLRED_SCHEMEID, firstCycle--);
	}
	//告知相位控制模块开始读取数据
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_BEGIN_READ_DATA;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

static Boolean CalculateByHikconfig(struct msgbuf *msg)
{
	UInt8 ret = 0;
	int i;
	
	//根据当前时间获取相序，并初始化各相位的绿信比时间段
	ret = GetPhaseTurnAndSetPhaseTimeInfo(msg->msgSchemeId, msg->msgCalTime);
	if (ret == UNKNOWN_CONTROL)
		return FALSE;
	if (ret == GENERAL_CONTROL) 
	{
		CalStageInfo();	//计算阶段相关的信息
		if (gCalInfo.maxStageNum < 2)
		{	//最少要2个阶段
			log_error("the maxStageNum[%d] is too small, schemeId = %d, splitId = %d, phaseTurnId = %d", gCalInfo.schemeId, gCalInfo.maxStageNum, gCalInfo.splitId, gCalInfo.phaseTurnId);
			return FALSE;
		}
		else
		{
			if (gCalInfo.schemeId != INDUCTIVE_SCHEMEID)
			{
				if (gCalInfo.isIgnorePhase > 0)	//表示有设置忽略相位
					IgnorePhaseDeal();	//对忽略相位进行处理
				if (msg->msgSchemeId == 0 && gCalInfo.coordinatePhaseId > 0)
					TransitionDeal();	//对协调相位进行过渡
			}
			for (i = 0; i < NUM_PHASE; i++)
			{
				SetPedestrianTime(GET_BIT(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].wPhaseOptions, 13), 
									&gCalInfo.phaseTimes[i], 
									gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nPedestrianPassTime, 
									gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nPedestrianClearTime);	//设置行人相关的时间
			}
			RunCycle(gCalInfo.phaseTurnId);	//每次轮询一个完整的周期	
		}
	}
	return TRUE; 
}

void *CalculateModule(void *arg)
{	
	struct msgbuf msg;
	
	YellowBlinkAllRedInit();
    while (1)
    {
		memset(&msg, 0, sizeof(msg));
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_START_CALCULATE_NEXT_CYCLE, 0) == -1)
		{
			usleep(10000);
			continue;
		}
		//为防止相位控制模块多次发送计算下一周期消息，因此作一判断
		if (lfq_element_count(gHandle) > AHEAD_OF_TIME)  
		    continue;
		pthread_rwlock_rdlock(&gConfigLock);
		if ((gRunConfigPara != NULL && CalculateByHikconfig(&msg))
			|| (gRunGbconfig != NULL && CalculateByGbconfig(&msg)))
			log_debug("msgschemeid = %d, gSchemeId = %d, cycleTime = %d", msg.msgSchemeId, gCalInfo.schemeId, gCalInfo.cycleTime);
		else
		{
			ItsCtl(TOOL_CONTROL, YELLOWBLINK_SCHEMEID, 0);
			ItsWriteFaultLog(CALCULATE_FAIL_CAUSE_FLASH, 0);
			if (gRunConfigPara == NULL && gRunGbconfig == NULL)
				log_error("There isn't a piece of config exist!");
		}
		pthread_rwlock_unlock(&gConfigLock);
    }
}

void ItsSetRedFlashSec(UInt8 sec)
{
	gCustomInfo.redFlashSec = sec;
}

void ItsYellowLampFlash(Boolean val)
{
	gCustomInfo.isYellowFlash = val;
}

void ItsGetRealtimePattern(MsgRealtimePattern *p)
{
	int i;
	pthread_rwlock_rdlock(&gConfigLock);
	p->nPatternId = gCalInfo.schemeId;
	p->nSplitId = gCalInfo.splitId;
	p->nPhaseTurnId = gCalInfo.phaseTurnId;
	if (gCalInfo.schemeId > 0 && gCalInfo.schemeId <= NUM_SCHEME)
		p->nOffset = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nOffset;
	if (gCalInfo.phaseTurnId > 0 && gCalInfo.phaseTurnId <= NUM_PHASE_TURN)
		memcpy(p->sPhaseTurn, gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1], sizeof(p->sPhaseTurn));
	p->nCoordinatePhase = gCalInfo.isCoordinatePhase;
	if (gCalInfo.phaseTableId > 0 && gCalInfo.phaseTableId <= MAX_PHASE_TABLE_COUNT)
	{
		for (i = 0; i < NUM_PHASE; i++)
		{
			p->phaseTime[i].greenBlink = gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i].nGreenLightTime;
			p->phaseTime[i].yellow = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nYellowTime;
			p->phaseTime[i].allred = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nAllRedTime;
		}
		memcpy(p->stFollowPhase, gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1], sizeof(p->stFollowPhase));
	}
	pthread_rwlock_unlock(&gConfigLock);
}
