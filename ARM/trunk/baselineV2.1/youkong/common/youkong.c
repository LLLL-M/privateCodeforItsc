/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
#include "ykconfig.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //把时分转换成分
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //一天中的最大时间差值，即24小时

#define MOTOR		2	//机动车通道类型
#define PEDESTRIAN	3	//行人通道类型

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
 
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static YK_Config *gRunConfigPara = NULL;//信号机运行的配置参数
static UInt8 gSingleAdaptRunCycleNum = 0;	//单点自适应运行周期个数
static YK_SchemeItem gSingleAdaptScheme;	//单点自适应方案

static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static YK_RealTimeInfo gRealTimeInfo;		//实时信息
static LineQueueData gCurRunData;		//当前这1s的运行数据

void ItsSetSingleAdaptScheme(YK_SchemeItem *pScheme)
{
	if (pScheme == NULL)
		return;
	memcpy(&gSingleAdaptScheme, pScheme, sizeof(YK_SchemeItem));
	gSingleAdaptRunCycleNum = gRunConfigPara->wholeConfig.adaptCtlEndRunCycleNum;	//默认运行3个周期后结束
}

void *ItsAllocConfigMem(void *config, int configSize)
{
	gRunConfigPara = calloc(1, configSize);
	if(gRunConfigPara != NULL)
	{
		memcpy(gRunConfigPara, config, configSize);
	}
	return gRunConfigPara;
}

void ItsUnitInit(void (*initFunc)(UInt8, UInt8))
{
	initFunc(gRunConfigPara->wholeConfig.bootYellowBlinkTime, gRunConfigPara->wholeConfig.bootAllRedTime);
}

static void SetFollowPhase(CalInfo *calInfo, UInt16 *channelIncludePhaseBits)
{
	YK_ChannelItem *pChannelTable = (YK_ChannelItem *)&gRunConfigPara->channelTable;
	FollowPhaseInfo *followPhaseInfos = calInfo->followPhaseInfos;
	int ch, ph, fo;
	int channelIncludePhaseNum;	//通道包含的相位个数
	
	if (channelIncludePhaseBits == NULL)
		return;
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		if (channelIncludePhaseBits[ch] == 0)
			continue;
		channelIncludePhaseNum = 0;
		for (ph = 0; ph < MAX_PHASE_NUM; ph++)
		{
			if (GET_BIT(channelIncludePhaseBits[ch], ph) == 1)
				channelIncludePhaseNum++;
		}
		if (channelIncludePhaseNum <= 1)
			continue;
		//说明一个通道在多个相位中放行则设置为跟随相位
		for (fo = 0; fo < MAX_FOLLOWPHASE_NUM; fo++)
		{
			if (followPhaseInfos[fo].phaseBits == 0 
				|| followPhaseInfos[fo].phaseBits == channelIncludePhaseBits[ch])
			{
				followPhaseInfos[fo].phaseBits = channelIncludePhaseBits[ch];
				if (pChannelTable[ch].nControllerType == MOTOR)
					SET_BIT(followPhaseInfos[fo].motorChannelBits, ch);
				else if (pChannelTable[ch].nControllerType == PEDESTRIAN)
					SET_BIT(followPhaseInfos[fo].pedChannelBits, ch);
			}
		}
	}
}	

static Boolean SetChannelBits(CalInfo *calInfo, YK_SchemeItem *pScheme)
{
	YK_ChannelItem *pChannelTable = (YK_ChannelItem *)&gRunConfigPara->channelTable;
	YK_PhaseInfo *phaseInfo;
	UInt16 channelIncludePhaseBits[NUM_CHANNEL] = {0};	//通道包含的相位bit
	int ch, ph;
	UInt32 conflictChannelBits = 0;
	
	if (pScheme == NULL)
		return FALSE;
	phaseInfo = pScheme->phaseInfo;
	//设置通道与相位的对应关系,并检测是否有通道绿冲突
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		if (pChannelTable[ch].nChannelID == 0 || pChannelTable[ch].nChannelID > NUM_CHANNEL 
			|| (pChannelTable[ch].nControllerType != MOTOR && pChannelTable[ch].nControllerType != PEDESTRIAN))
			continue;
		for (ph = 0; ph < pScheme->totalPhaseNum; ph++)
		{
			if (GET_BIT(phaseInfo[ph].channelBits, ch) == 1)
			{
				conflictChannelBits = phaseInfo[ph].channelBits & pChannelTable[ch].conflictChannel;
				if (conflictChannelBits == 0)
				{	//无通道冲突
					if (pChannelTable[ch].nControllerType == MOTOR)
						SET_BIT(calInfo->phaseTimes[ph].motorChannelBits, ch);
					else if (pChannelTable[ch].nControllerType == PEDESTRIAN)
						SET_BIT(calInfo->phaseTimes[ph].pedChannelBits, ch);
					SET_BIT(channelIncludePhaseBits[ch], ph);
					if (pChannelTable[ch].nVehDetectorNum > 0 && pChannelTable[ch].nVehDetectorNum <= 48)
						SET_BIT(calInfo->phaseTimes[ph].vehicleDetectorBits, pChannelTable[ch].nVehDetectorNum - 1);
					//INFO("!!!!!!set channel %d type %d", ch + 1, pChannelTable[ch].nControllerType);
				}
				else
				{	//当一个相位中包含互相冲突的通道时便会产生绿冲突
					SET_BIT(conflictChannelBits, ch);
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
					ItsWriteFaultLog(GREEN_CONFLICT, conflictChannelBits);
					log_error("scheme %d occur green conflict, conflictChannelBits: %#x", pScheme->nSchemeId, conflictChannelBits);
					return FALSE;
				}
			}
				
		}
	}
	SetFollowPhase(calInfo, channelIncludePhaseBits);
	return TRUE;
}

static Boolean SetPhaseTime(CalInfo *calInfo, YK_SchemeItem *pScheme)
{  
    int i = 0;
	YK_PhaseInfo *phaseInfo;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	UInt16 cycleTime = 0;
	
	if (pScheme == NULL || pScheme->totalPhaseNum > MAX_PHASE_NUM)
		return FALSE;
	phaseInfo = pScheme->phaseInfo;
	calInfo->cycleTime = pScheme->cycleTime;
    for (i = 0; i < pScheme->totalPhaseNum; i++)
    {
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = phaseInfo[i].greenTime + phaseInfo[i].greenBlinkTime + phaseInfo[i].yellowTime + phaseInfo[i].redYellowTime + phaseInfo[i].allRedTime;
		passTimeInfo->greenTime = phaseInfo[i].greenTime;
		passTimeInfo->greenBlinkTime = phaseInfo[i].greenBlinkTime;
		passTimeInfo->yellowTime = phaseInfo[i].yellowTime;
		passTimeInfo->redYellowTime = phaseInfo[i].redYellowTime;
		passTimeInfo->allRedTime = phaseInfo[i].allRedTime;
		phaseTimes[i].pedestrianClearTime = phaseInfo[i].pedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = 1;
		phaseTimes[i].unitExtendGreen = phaseInfo[i].unitExtendTime;
		phaseTimes[i].maxExtendGreen = phaseInfo[i].maxGreenTime - phaseInfo[i].minGreenTime;
		cycleTime += phaseTimes[i].splitTime;
		
		calInfo->stageInfos[i].runTime = phaseTimes[i].splitTime;
		calInfo->stageInfos[i].includeNum = 1;
		calInfo->stageInfos[i].includePhases[0] = i + 1;
		calInfo->includeNums[i] = 1;
		calInfo->phaseIncludeStage[i][0] = i + 1;
    }
	calInfo->maxStageNum = i;
	if (cycleTime != calInfo->cycleTime || calInfo->maxStageNum < 2)
	{
		log_error("all phase time sum[%d] of the scheme %d isn't eqult to cycleTime %d, maxStageNum = %d", cycleTime, calInfo->cycleTime, calInfo->maxStageNum);
		return FALSE;
	}
	else
		return SetChannelBits(calInfo, pScheme);
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
    int i = 0;

	if (gRunConfigPara->wholeConfig.weekPreemptSchedule == 1 && now->tm_wday >= 0 && now->tm_wday < 7)	//星期优先被调度
	{	//tm_wday == 0代表星期日
		if (gRunConfigPara->scheduleTable.week[now->tm_wday] > 0 && gRunConfigPara->scheduleTable.week[now->tm_wday] <=NUM_TIME_INTERVAL)
			return gRunConfigPara->scheduleTable.week[now->tm_wday];
	}
	for(i = 0; i < NUM_SPECIAL_DATE; i++) 
	{	//默认优先使用特殊日期的时段号
		if ((GET_BIT(gRunConfigPara->scheduleTable.specialDate[i].month, now->tm_mon + 1) == 1)//tm_mon范围是[0,11]
			&& (GET_BIT(gRunConfigPara->scheduleTable.specialDate[i].day, now->tm_mday) == 1))
			return gRunConfigPara->scheduleTable.specialDate[i].nTimeIntervalID;
	}
	return (now->tm_wday >= 0 && now->tm_wday < 7) ? gRunConfigPara->scheduleTable.week[now->tm_wday] : 0;
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
UInt8 GetSchemeIdAndTimeGap(CalInfo *calInfo, struct tm *now, int *ret)
{
	UInt8 hour, minute;
    int i = 0, index = -1;
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nSchemeID = 0;
	YK_TimeIntervalItem *item = NULL;
	
	nTimeIntervalID = GetTimeIntervalID(now);//根据当前时间，遍历调度表，得到时段表ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > NUM_TIME_INTERVAL)       
	{
		//log_error("can't find timeinterval id");
        return 0;
	}
	item = (YK_TimeIntervalItem *)&gRunConfigPara->timeIntervalTable[nTimeIntervalID - 1];
    //此函数中的时间值单位为分钟
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//预先设置一个最大的差值
	do
	{	//循环找出当前时间与时段表中差值最小的时段所对应的actionID即为当前应该运行的actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if (gRunConfigPara != NULL && item[i].nSchemeId  == 0) 
			{ //这说明该时段可能没有被使用，直接continue掉
				continue;
			}
			nTempTime = HoursToMinutes(item[i].cStartTimeHour, item[i].cStartTimeMinute);
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
			//log_error("the nTimeIntervalID %d isn't configed", nTimeIntervalID);
			return 0;	//说明此时段表没有配置
		}
		if (minTimeGap == MAX_TIME_GAP) 
		{ //说明当前时间处于时段表中最小的位置
			nCurrentTime += MAX_TIME_GAP;	//把当前时间增加24小时然后再次循环
		}
	} while (index == -1);
	if (ret != NULL)
	{
	    *ret = minTimeGap;
		if (calInfo != NULL)
		{
			calInfo->timeIntervalId = nTimeIntervalID;
			if (item[index].IsCorrdinateCtl == 1)
			{
				calInfo->coordinatePhaseId = 1;	//默认相位1作为协调相位
				calInfo->phaseOffset = item[index].phaseOffset;
				calInfo->transitionCycle = gRunConfigPara->wholeConfig.transitionCycle;
			}
		}
	}
	return item[index].nSchemeId;
}
//实时感应控制默认使用方案1的相位顺序
static Boolean SetInductiveControlTime(CalInfo *calInfo)
{
    int i = 0;
	YK_PhaseInfo *phaseInfo = gRunConfigPara->schemeTable[0].phaseInfo;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	
	calInfo->cycleTime = 0;
    for(i = 0; i < gRunConfigPara->schemeTable[0].totalPhaseNum; i++)
    {
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = phaseInfo[i].minGreenTime + phaseInfo[i].yellowTime + phaseInfo[i].redYellowTime + phaseInfo[i].allRedTime;
		passTimeInfo->greenTime = phaseInfo[i].minGreenTime - phaseInfo[i].greenBlinkTime;
		passTimeInfo->greenBlinkTime = phaseInfo[i].greenBlinkTime;
		passTimeInfo->yellowTime = phaseInfo[i].yellowTime;
		passTimeInfo->redYellowTime = phaseInfo[i].redYellowTime;
		passTimeInfo->allRedTime = phaseInfo[i].allRedTime;
		phaseTimes[i].pedestrianClearTime = phaseInfo[i].pedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = 1;
		phaseTimes[i].unitExtendGreen = phaseInfo[i].unitExtendTime;
		phaseTimes[i].maxExtendGreen = phaseInfo[i].maxGreenTime - phaseInfo[i].minGreenTime;
		calInfo->cycleTime += phaseTimes[i].splitTime;
		
		calInfo->stageInfos[i].runTime = phaseTimes[i].splitTime;
		calInfo->stageInfos[i].includeNum = 1;
		calInfo->stageInfos[i].includePhases[0] = i + 1;
		calInfo->includeNums[i] = 1;
		calInfo->phaseIncludeStage[i][0] = i + 1;
    }
	calInfo->maxStageNum = i;
	if (calInfo->maxStageNum < 2)
	{
		log_error("the maxStageNum %d is too small!", calInfo->maxStageNum);
		return FALSE;
	}
	return SetChannelBits(calInfo, &gRunConfigPara->schemeTable[0]);
}

Boolean FillCalInfo(CalInfo *calInfo, UInt8 schemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//当前时刻与当前时段的起始时间的差值，单位为分钟

	localtime_r(&calTime, &now);
	memset(calInfo, 0, sizeof(CalInfo));
	calInfo->collectCycle = gRunConfigPara->wholeConfig.vehFlowCollectCycleTime;
	calInfo->checkTime = 3;
	if (gSingleAdaptRunCycleNum > 0)
	{
		gSingleAdaptRunCycleNum--;
		calInfo->schemeId = SINGLE_ADAPT_SCHEMEID;
	}
	else
		calInfo->schemeId = (schemeId > 0) ? schemeId : GetSchemeIdAndTimeGap(calInfo, &now, &timeGapMinutes);
    if (calInfo->schemeId == YELLOWBLINK_SCHEMEID || calInfo->schemeId == ALLRED_SCHEMEID || calInfo->schemeId == TURNOFF_SCHEMEID)
	{
		log_debug("special control, scheme: %d", calInfo->schemeId);
        return TRUE;
	}
	else if (calInfo->schemeId == INDUCTIVE_SCHEMEID)	
    {	//如果是感应控制应当使用最小绿+黄灯时间+红黄时间+全红时间作为绿信比时间
        return SetInductiveControlTime(calInfo);
    }
	else if (calInfo->schemeId == SINGLE_ADAPT_SCHEMEID)
		return SetPhaseTime(calInfo, &gSingleAdaptScheme);
	else if(calInfo->schemeId > 0 && calInfo->schemeId <= NUM_SCHEME)
	{
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
		//根据当前的方案号，给相位表中各相位的绿信比时间段赋值
		return SetPhaseTime(calInfo, &gRunConfigPara->schemeTable[calInfo->schemeId - 1]);
	}
	else
	{
		log_error("can't find scheme id, action: %d", calInfo->actionId);
        return FALSE;
	}
}

void SetRealTimeInfo(const ChannelLockParams *lockparams, const LineQueueData *data)
{
	int i;
	
	pthread_rwlock_wrlock(&gCountDownLock);
	memcpy(gRealTimeInfo.allChannels, data->allChannels, sizeof(data->allChannels));
	gRealTimeInfo.cycleTime = data->cycleTime;
	gRealTimeInfo.schemeId = (lockparams->ucChannelLockStatus == 1) ? CHANNEL_LOCK_SCHEMEID : data->schemeId;
	if (data->schemeId > 0 && data->schemeId <= NUM_SCHEME)
	{
		for (i = 0; i < gRunConfigPara->schemeTable[data->schemeId - 1].totalPhaseNum; i++)
		{
			if (data->phaseInfos[i].phaseStatus == GREEN)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 1;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == GREEN_BLINK)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 2;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == YELLOW)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 3;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == RED_YELLOW)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 4;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == ALLRED)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 5;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
		}
	}
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsCountDownGet(void *countdown, int size)
{
	if (countdown == NULL || size != sizeof(YK_RealTimeInfo))
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(countdown, &gRealTimeInfo, size);
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsGetCurRunData(LineQueueData *data)
{
	if (data == NULL)
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(data, &gCurRunData, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}
