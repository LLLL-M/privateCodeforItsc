/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
#include "HikConfig.h"
#include "common.h"
#include "configureManagement.h"
#include "platform.h"
#include "countDown.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //把时分转换成分
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //一天中的最大时间差值，即24小时

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;
extern STRUCT_BINFILE_DESC gStructBinfileDesc;
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara; 
extern Boolean gAddTimeToFirstPhase;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void SendRunInfoTOBoard(LineQueueData *data, SignalControllerPara *para);
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static SignalControllerPara *gRunConfigPara = NULL;//信号机运行的配置参数
static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static LineQueueData gCurRunData;		//当前这1s的运行数据
static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;   //倒计时接口信息
static MsgRealtimePattern gCurrentCycleRealtimePattern;		//当前周期方案信息
static MsgRealtimePattern gNextCycleRealtimePattern;		//下一周期方案信息
static UInt32 gConflictChannel[2][NUM_CHANNEL] = {{0}, {0}};//[0]保存的为当前周期的冲突通道表，[1]保存的是计算下一周期时一起算出来的，在每个周期第1s会把[1]赋值到[0]

#include "ignorephase.h"	//忽略相位有关的函数，只能放在这个位置引用

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
	initFunc(gRunConfigPara->stUnitPara.nBootYellowLightTime, gRunConfigPara->stUnitPara.nBootAllRedTime);
}

static void CalConflictChannel(const PhaseItem *phaseTable, const ChannelItem *channelTable, const FollowPhaseItem *followPhaseTable, UInt32 *conflictChannels)
{
	int ph, ch, n;
	UInt8 phaseId = 0, motherPhaseId = 0, concurrencyPhaseId = 0;
	UInt32 phaseIncludeChannelBits[NUM_PHASE] = {0};	//相位包含的通道bit位
	UInt32 concurrencyChannels[NUM_CHANNEL] = {0};		//并发的通道bit位
	UInt32 unuseChannelBit = 0;	//未使用的通道bit位
	
	if (phaseTable == NULL || channelTable == NULL 
		|| followPhaseTable == NULL || conflictChannels == NULL)
		return;
	//首先找到各相位所包含的通道bit位
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		phaseId = channelTable[ch].nControllerID;
		if (phaseId == 0 || phaseId > NUM_PHASE)
		{	//默认未配置通道可与所有通道并发
			concurrencyChannels[ch] = 0xffffffff;
			SET_BIT(unuseChannelBit, ch);
			continue;
		}
		switch (channelTable[ch].nControllerType) 
		{
			case MOTOR:
			case PEDESTRIAN: 
				SET_BIT(phaseIncludeChannelBits[phaseId - 1], ch);
				break;
			case FOLLOW:	//机动车跟随
			case OTHER:		//行人跟随
				for (n = 0; n < NUM_PHASE; n++)
				{
					motherPhaseId = followPhaseTable[phaseId - 1].nArrayMotherPhase[n];
					if (motherPhaseId == 0 || motherPhaseId > NUM_PHASE)
						continue;
					SET_BIT(phaseIncludeChannelBits[motherPhaseId - 1], ch);
				}
				break;
			default: concurrencyChannels[ch] = 0xffffffff; SET_BIT(unuseChannelBit, ch); break;//默认未配置通道可与所有通道并发
		}
	}
	//其次通过并发相位找到各相位包含的通道所并发的通道bit
	for (ph = 0; ph < NUM_PHASE; ph++)
	{
		if (GET_BIT(phaseTable[ph].wPhaseOptions, 0) == FALSE)
		{	//相位未使能，默认此相位所包含的通道与所有通道可并发
			for (ch = 0; ch < NUM_CHANNEL; ch++)
			{
				if (GET_BIT(phaseIncludeChannelBits[ph], ch))
				{
					concurrencyChannels[ch] = 0xffffffff;
					SET_BIT(unuseChannelBit, ch);
				}
			}
		}
		else
		{
			for (ch = 0; ch < NUM_CHANNEL; ch++)
			{
				if (GET_BIT(phaseIncludeChannelBits[ph], ch) == FALSE)
					continue;
				concurrencyChannels[ch] |= phaseIncludeChannelBits[ph];
				for (n = 0; n < NUM_PHASE; n++)
				{
					concurrencyPhaseId = phaseTable[ph].byPhaseConcurrency[n];
					if (concurrencyPhaseId == 0 || concurrencyPhaseId > NUM_PHASE)
						continue;
					concurrencyChannels[ch] |= phaseIncludeChannelBits[concurrencyPhaseId - 1];
				}
			}
		}
	}
	//最后对并发通道取反即为冲突通道
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		concurrencyChannels[ch] |= unuseChannelBit;	//未使用通道可与其他任何通道并发
		conflictChannels[ch] = ~concurrencyChannels[ch];	//并发通道取反即为冲突通道
		CLR_BIT(conflictChannels[ch], ch);	//清除自身的冲突bit位
	}
}

static void SetChannelBits(CalInfo *calInfo)
{
	UInt8 phaseId = 0, motherPhaseId = 0;
	int ch, n;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		phaseId = gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerID;
		if (phaseId == 0 || phaseId > NUM_PHASE)
			continue;
		switch (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType) 
		{
			case MOTOR:
				SET_BIT(calInfo->phaseTimes[phaseId - 1].motorChannelBits, ch);
                SET_BIT(calInfo->motorChanType,ch);
				break;
			case PEDESTRIAN: 
				SET_BIT(calInfo->phaseTimes[phaseId - 1].pedChannelBits, ch);
                SET_BIT(calInfo->pedChanType,ch);
				break;
			case FOLLOW:
			case OTHER:
				if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType == FOLLOW)
				{
					SET_BIT(calInfo->followPhaseInfos[phaseId - 1].motorChannelBits, ch);
                    SET_BIT(calInfo->motorChanType,ch);
				}
                else if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType == OTHER)	
				{
                    SET_BIT(calInfo->followPhaseInfos[phaseId - 1].pedChannelBits, ch);//把other作为行人跟随通道
                    SET_BIT(calInfo->pedChanType,ch);
				}
                else
					break;
				for (n = 0; n < NUM_PHASE; n++)
				{
					motherPhaseId = gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1][phaseId - 1].nArrayMotherPhase[n];
					if (motherPhaseId == 0 || motherPhaseId > NUM_PHASE)
						continue;
					SET_BIT(calInfo->followPhaseInfos[phaseId - 1].phaseBits, motherPhaseId - 1);
				}
#if 0
				INFO("followPhaseId: %d, motorChannelBits: %#x, pedChannelBits: %#x, phaseBits: %#x", 
					phaseId, 
					calInfo->followPhaseInfos[phaseId - 1].motorChannelBits,
					calInfo->followPhaseInfos[phaseId - 1].pedChannelBits,
					calInfo->followPhaseInfos[phaseId - 1].phaseBits);
#endif
				break;
			default: break;
		}
	}
}
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
static void SetPhaseTime(CalInfo *calInfo)
{  
    int i = 0, j = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;

	//INFO("DEBUG: enter %s function!!!!!", __func__);
    for(i = 0 ; i < NUM_PHASE; i++)
    {
		splitItem = &gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1][i];
		phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][i];
		if ((phaseItem->nCircleID == 0) || (splitItem->nGreenSignalRationTime == 0))
			continue;
		if (splitItem->nIsCoordinate)
		{	//说明配置了协调相位，此时如果没有其他特殊控制应该执行协调控制
			if (calInfo->coordinatePhaseId == 0)	//配置多个协调相位时，以最先找到的为准
				calInfo->coordinatePhaseId = splitItem->nPhaseID;
			SET_BIT(calInfo->isCoordinatePhase, i);
		}
		if (splitItem->nType == IGNORE_PHASE)
		{
			SET_BIT(calInfo->isIgnorePhase, i);
		}
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = splitItem->nGreenSignalRationTime;
		passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
		passTimeInfo->yellowTime = phaseItem->nYellowTime;
		passTimeInfo->allRedTime = phaseItem->nAllRedTime;
		
		phaseTimes[i].pedestrianPassTime = phaseItem->nPedestrianPassTime;
		phaseTimes[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
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
    int i = 0;

	//INFO("DEBUG: enter %s function!!!!!", __func__);
	for(i = 0 ; i < NUM_SCHEDULE ; i++) { //如果当前时间和某一项调度表相符，则返回该调度表的时段号
        if (BIT(gRunConfigPara->stPlanSchedule[i].month, now->tm_mon + 1) == 1)
		{	//tm_mon范围是[0,11]
			if ((BIT(gRunConfigPara->stPlanSchedule[i].day, now->tm_mday) == 1)
				|| (BIT(gRunConfigPara->stPlanSchedule[i].week, now->tm_wday + 1) == 1))//tm_wday == 0代表星期日
			    return gRunConfigPara->stPlanSchedule[i].nTimeIntervalID;
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
static UInt8 GetSchemeIdAndTimeGap(CalInfo *calInfo, struct tm *now, int *ret)
{
    int i = 0, index = -1;
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nActionID = 0;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	nTimeIntervalID = GetTimeIntervalID(now);//根据当前时间，遍历调度表，得到时段表ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > NUM_TIME_INTERVAL)       
	{
		//log_error("can't find timeinterval id");
        return 0;
	}
    //此函数中的时间值单位为分钟
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//预先设置一个最大的差值
	do
	{	//循环找出当前时间与时段表中差值最小的时段所对应的actionID即为当前应该运行的actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if (gRunConfigPara != NULL && gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //这说明该时段可能没有被使用，直接continue掉
				continue;
			}
			
			nTempTime = HoursToMinutes(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour, gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute);
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
	    *ret = minTimeGap;
	nActionID = gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;//根据时段表ID，得到动作表ID    
	if (nActionID == 0 || nActionID > NUM_ACTION)
	{
		//log_error("can't find action id");
		return 0;
	}
	if (calInfo != NULL)
	{
		calInfo->timeIntervalId = nTimeIntervalID;
		calInfo->actionId = nActionID;
	}
	//INFO("DEBUG: leave %s function $$$$$$$$", __func__);
	return gRunConfigPara->stAction[nActionID - 1].nSchemeID;
}

//得到相位对应的车检器bits，以后默认情况下，配置文件中车检器对应的请求相位其实是通道。
static UInt64 GetDetectorBitsByPhase(UInt8 nPhaseId, CalInfo *calInfo)
{
	int i;
    UInt8 nChannelId = 0, nControllerType;
	UInt64 vehicleDetectorBits = 0;
	
    if(nPhaseId == 0 || nPhaseId > MAX_PHASE_NUM || calInfo == NULL || gRunConfigPara == NULL)
        return 0;
	for (i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{	//轮询车检器配置信息查找相位对应的车检器号
		nChannelId = gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase;
		if (nChannelId == 0 || nChannelId > NUM_CHANNEL)
			continue;
		nControllerType = gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerType;
	    if ((nControllerType == MOTOR || nControllerType == PEDESTRIAN)
			&& gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerID == nPhaseId)
			SET_BIT(vehicleDetectorBits, i);
	}
    return vehicleDetectorBits;
}


static void SetInductiveControlTime(CalInfo *calInfo)
{
    int i = 0, j = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;

    for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
				break;
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][nPhaseId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			passTimeInfo = &phaseTimes[nPhaseId - 1].passTimeInfo;
			phaseTimes[nPhaseId - 1].splitTime = phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime;
			passTimeInfo->greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
			passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
			passTimeInfo->yellowTime = phaseItem->nYellowTime;
			passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			
			if (ring == 0)	//在第一个环轮询时计算周期
			{
				calInfo->cycleTime += phaseTimes[nPhaseId - 1].splitTime;
			}
				
			phaseTimes[nPhaseId - 1].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			phaseTimes[nPhaseId - 1].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			phaseTimes[nPhaseId - 1].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
			
			phaseTimes[nPhaseId - 1].vehicleDetectorBits = GetDetectorBitsByPhase(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].unitExtendGreen = phaseItem->nUnitExtendGreen;
			if (phaseItem->nMaxGreen_1 > phaseItem->nMinGreen)
			{
				phaseTimes[nPhaseId - 1].maxExtendGreen = phaseItem->nMaxGreen_1 - phaseItem->nMinGreen;
                if(phaseItem->nMaxGreen_2 > phaseItem->nMaxGreen_1)
                {
                    phaseTimes[nPhaseId - 1].maxExtendGreen2 = phaseItem->nMaxGreen_2 - phaseItem->nMinGreen;
                    //INFO("@@@phaseTimes[%d] = %d\n",nPhaseId , phaseTimes[nPhaseId - 1].maxExtendGreen2);
                }
            }
		}
	}
}

static void SetInductiveCoordinateControlTime(CalInfo *calInfo)
{
    int i = 0, j = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	PGreenSignalRationItem splitItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	UInt16 cycleTime = 0;

	for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		cycleTime = 0;
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
			{
				if (calInfo->cycleTime < cycleTime)
					calInfo->cycleTime = cycleTime;
				break;
			}
			
			splitItem = &gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1][nPhaseId - 1];
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][nPhaseId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			passTimeInfo = &phaseTimes[nPhaseId - 1].passTimeInfo;
			
			if (i == 0)	//每环中的第一个相位作为协调相位并默认执行绿信比的配时时间
				passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime - phaseItem->nYellowTime - phaseItem->nAllRedTime;
			else	//其他后续的相位都默认执行最小绿
			{
				if (splitItem->nGreenSignalRationTime <= phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime)	//当绿信比的时间小于等于最小绿时，默认执行绿信比时间
					passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime - phaseItem->nYellowTime - phaseItem->nAllRedTime;
				else
					passTimeInfo->greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
			}
				
			passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
			passTimeInfo->yellowTime = phaseItem->nYellowTime;
			passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			phaseTimes[nPhaseId - 1].splitTime = passTimeInfo->greenTime 
												+ passTimeInfo->greenBlinkTime 
												+ passTimeInfo->yellowTime 
												+ passTimeInfo->allRedTime;
			cycleTime += phaseTimes[nPhaseId - 1].splitTime;
			
			phaseTimes[nPhaseId - 1].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			phaseTimes[nPhaseId - 1].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			phaseTimes[nPhaseId - 1].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
			
			phaseTimes[nPhaseId - 1].vehicleDetectorBits = GetDetectorBitsByPhase(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].unitExtendGreen = phaseItem->nUnitExtendGreen;
			phaseTimes[nPhaseId - 1].maxExtendGreen = splitItem->nGreenSignalRationTime - phaseTimes[nPhaseId - 1].splitTime;
		}
	}
}

static void SetNextCycleRealtimePattern(CalInfo *calInfo)
{
	MsgRealtimePattern *p = &gNextCycleRealtimePattern;
	int i;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	memset(p, 0, sizeof(MsgRealtimePattern));
	p->nPatternId = calInfo->schemeId;
	p->nSplitId = calInfo->splitId;
	p->nPhaseTurnId = calInfo->phaseTurnId;
	p->nPhaseTableId = calInfo->phaseTableId;
	p->nChannelTableId = calInfo->channelTableId;
	if (calInfo->schemeId > 0 && calInfo->schemeId <= NUM_SCHEME)
		p->nOffset = gRunConfigPara->stScheme[calInfo->schemeId - 1].nOffset;
	if (calInfo->phaseTurnId > 0 && calInfo->phaseTurnId <= NUM_PHASE_TURN)
		memcpy(p->sPhaseTurn, gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1], sizeof(p->sPhaseTurn));
	p->nCoordinatePhase = calInfo->isCoordinatePhase;
	if (calInfo->phaseTableId > 0 && calInfo->phaseTableId <= MAX_PHASE_TABLE_COUNT)
	{
		for (i = 0; i < NUM_PHASE; i++)
		{
			p->phaseTime[i].greenBlink = gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][i].nGreenLightTime;
			p->phaseTime[i].yellow = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i].nYellowTime;
			p->phaseTime[i].allred = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i].nAllRedTime;
		}
		memcpy(p->stFollowPhase, gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1], sizeof(p->stFollowPhase));
	}
	memcpy(p->phaseDesc, gStructBinfileDesc.phaseDescText[calInfo->phaseTableId - 1], sizeof(p->phaseDesc));
}

static int IsConcurrencyPhase(PhaseItem* phaseItems, UINT8 phaseId,UINT8 concurrencyPhase)
{
	int i = 0;
	if (0 >= phaseId || phaseId > NUM_PHASE)
		return 0;
	for (i = 0; i < NUM_PHASE; i++)
		if (phaseItems[phaseId - 1].byPhaseConcurrency[i] == concurrencyPhase)
			return 1;
	return 0;
}

static int RingConcurrencyOk(CalInfo *calInfo, UINT8 ring)
{
	int i = 0, j = 0;
	UINT16 ringtime1 = 0, ringtime2 = 0;
	UINT16 cycletime = 0;
	PhaseItem* phaseItems = gRunConfigPara->stPhase[calInfo->phaseTableId - 1];
	PhaseTurnItem* phaseTurns = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1];
	PhaseTimeInfo* phaseTimes = calInfo->phaseTimes;

	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseTurns[ring].nTurnArray[i] <= 0 || phaseTurns[ring].nTurnArray[i] > NUM_PHASE)
			break;
		cycletime += phaseTimes[phaseTurns[ring].nTurnArray[i] - 1].splitTime;
	}
	//log_debug("ring[%d] cycletime=%d, calinfo cycletime=%d", ring, cycletime, calInfo->cycleTime);
	if (cycletime != calInfo->cycleTime)//ring's cycle time don't be equal
		return 0;
	i = 0;
	j = 0;
	ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
	ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
	//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
	while(1)
	{
		if (i >= NUM_PHASE || j >= NUM_PHASE)
			break;
		if (!IsConcurrencyPhase(phaseItems, phaseTurns[0].nTurnArray[i], phaseTurns[ring].nTurnArray[j]))
			return 0;
		if (ringtime1 == ringtime2)
		{
			if (ringtime1 == cycletime && ringtime2 == cycletime)
				break;
			i++;
			j++;
			ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
			ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
		else if (ringtime1 < ringtime2)
		{
			i++;
			ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
		else if (ringtime1 > ringtime2)
		{
			j++;
			ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
	}
	return 1;
}

static int JudgeConcurrencyLegal(CalInfo *calInfo)
{
	int ring = 0;
	int i = 0;
	PhaseTurnItem* pphaseturn = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1];

	for (ring = 1; ring < NUM_RING_COUNT; ring++)
	{
		if (pphaseturn[ring].nTurnArray[0] == 0)
			continue;
		if (0 == RingConcurrencyOk(calInfo, ring))
			return 0;
	}
	return 1;
}

Boolean FillCalInfo(CalInfo *calInfo, UInt8 mSchemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//当前时刻与当前时段的起始时间的差值，单位为分钟
    UInt8 nTempSchemeId = 0;
    static UINT8 is_first_cyle = TRUE;      //是否是程序启动后的第一个周期

	//INFO("DEBUG: enter %s function!!!!!", __func__);
	localtime_r(&calTime, &now);
	memset(calInfo, 0, sizeof(CalInfo));
	calInfo->transitionCycle = gRunConfigPara->stUnitPara.byTransCycle;
	calInfo->collectCycle = gRunConfigPara->stUnitPara.byFluxCollectCycle;
	calInfo->checkTime = gStructBinfileCustom.sCountdownParams.iFreeGreenTime;
    calInfo->schemeId = (mSchemeId > 0) ? mSchemeId : GetSchemeIdAndTimeGap(calInfo, &now, &timeGapMinutes);
	//INFO("calInfo->schemeId=%d", calInfo->schemeId);
    //calInfo->schemeId = SINGLE_ADAPT_SCHEMEID;            //自己测试
    
    if (calInfo->schemeId == YELLOWBLINK_SCHEMEID || calInfo->schemeId == ALLRED_SCHEMEID || calInfo->schemeId == TURNOFF_SCHEMEID)
	{	//特殊控制
		//log_debug("special control, scheme: %d", calInfo->schemeId);
        return TRUE;
	}
	else if ((calInfo->schemeId == INDUCTIVE_SCHEMEID) 
		|| ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (2 == calInfo->schemeId % 3)))
    {	//感应控制,默认按照方案1来跑感应，否则按照实际的方案号对应的映射方案来执行
        nTempSchemeId = (calInfo->schemeId == INDUCTIVE_SCHEMEID) ? 1 : (calInfo->schemeId - 1);
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->channelTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nPhaseTableID;
		calInfo->actionId = INDUCTIVE_ACTIONID;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //如果是程序启动后的第一个周期，则给gCurRunData.channelTableId赋一个有效的通道表号，以解决第一个周期为感应，协调感应，
        //自适应控制时不能感应的bug。
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
        if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
        {
            log_error("scheme %d config error when excute inductive control", nTempSchemeId);
            return FALSE;
        }
    	//如果是感应控制应当使用最小绿+黄灯时间+全红时间作为绿信比时间
        SetInductiveControlTime(calInfo);
    }
	else if ((calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID) 
		|| ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (0 == calInfo->schemeId % 3)))
    {	//协调感应控制,默认按照方案1来跑协调感应，否则按照实际的方案号对应的映射方案来执行	
        nTempSchemeId = (calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID) ? 1 : (calInfo->schemeId - 2);
		calInfo->inductiveCoordinateCycleTime = gRunConfigPara->stScheme[nTempSchemeId - 1].nCycleTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[nTempSchemeId - 1].nOffset;
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->splitId = gRunConfigPara->stScheme[nTempSchemeId - 1].nGreenSignalRatioID;
		calInfo->actionId = INDUCTIVE_COORDINATE_ACTIONID;
		calInfo->channelTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nPhaseTableID;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //如果是程序启动后的第一个周期，则给gCurRunData.channelTableId赋一个有效的通道表号，以解决第一个周期为感应，协调感应，
        //自适应控制时不能感应的bug。
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
		if (calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN
			|| calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION
			|| calInfo->inductiveCoordinateCycleTime == 0)
		{
			log_error("scheme %d config error when excute inductive coordinate control", nTempSchemeId);
			return FALSE;
		}
		calInfo->coordinatePhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][0].nTurnArray[0];
		//手动执行感应协调控制时默认以0点为起始时段进行过渡
		if (mSchemeId > 0)
			timeGapMinutes = now.tm_hour * 60 + now.tm_min;
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
        SetInductiveCoordinateControlTime(calInfo);
    }
	else if ((calInfo->schemeId == SINGLE_ADAPT_SCHEMEID)) 
    {	//自适应控制,默认按照方案1作为基础方案。
        nTempSchemeId = 1;
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->channelTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nPhaseTableID;
		calInfo->actionId = SINGLE_ADAPT_ACTIONID;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //如果是程序启动后的第一个周期，则给gCurRunData.channelTableId赋一个有效的通道表号，以解决第一个周期为感应，协调感应，
        //自适应控制时不能感应的bug。
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
        if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
        {
            log_error("scheme %d config error when excute single adapt control", nTempSchemeId);
            return FALSE;
        }
    	//自适应控制和感应控制一样，应当使用最小绿+黄灯时间+全红时间作为绿信比时间
        SetInductiveControlTime(calInfo);
    }
	else if ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (1 == calInfo->schemeId % 3))
	{	//普通方案
		calInfo->splitId = gRunConfigPara->stScheme[calInfo->schemeId - 1].nGreenSignalRatioID;//根据方案表ID，得到绿信比表ID
		if(calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION)
		{
			log_error("can't find split id, scheme: %d , splitId: %d", calInfo->schemeId,calInfo->splitId);
			return FALSE;
		}

		calInfo->cycleTime = gRunConfigPara->stScheme[calInfo->schemeId - 1].nCycleTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[calInfo->schemeId - 1].nOffset;
		calInfo->phaseTurnId = gRunConfigPara->stScheme[calInfo->schemeId - 1].nPhaseTurnID;//根据方案表ID，得到相序表ID
		if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
		{
			log_error("can't find phase turn id, scheme: %d", calInfo->schemeId);
			return FALSE;
		}
		if (calInfo->actionId == 0)	//方案1,4,7对应动作1，2，3
			calInfo->actionId = (calInfo->schemeId + 2) / 3;
		calInfo->phaseTableId = gRunConfigPara->stAction[calInfo->actionId - 1].nPhaseTableID;
		calInfo->channelTableId = gRunConfigPara->stAction[calInfo->actionId - 1].nChannelTableID;
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
		{	
			calInfo->phaseTableId = 1;
			gRunConfigPara->stAction[calInfo->actionId - 1].nPhaseTableID = 1;
		}
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		{
			calInfo->channelTableId = 1;
			gRunConfigPara->stAction[calInfo->actionId - 1].nChannelTableID = 1;
		}
        //如果是程序启动后的第一个周期，则给gCurRunData.channelTableId赋一个有效的通道表号，以解决第一个周期为感应，协调感应，
        //自适应控制时不能感应的bug。
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		//根据当前的绿信比表ID，给相位表中各相位的绿信比时间段赋值，并同时找出协调相位
		SetPhaseTime(calInfo);
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	}
	else
	{
		log_error("can't find scheme id, action: %d", calInfo->actionId);
        return FALSE;
	}
	if (calInfo->schemeId == INDUCTIVE_SCHEMEID || 
		calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID ||
		calInfo->schemeId == SINGLE_ADAPT_SCHEMEID ||
		((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (2 == calInfo->schemeId % 3)) ||
		((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (0 == calInfo->schemeId % 3)))
	{
		if (0 == JudgeConcurrencyLegal(calInfo)) //判断感应时并发是否合理，不合理降级黄闪
		{
			log_error("When run inductive scheme, ring concurrency illegal. YellowBlink control.");
			ItsCtl(TOOL_CONTROL, YELLOWBLINK_SCHEMEID, 0);
			return TRUE;
		}
	}
	CalStageInfo(calInfo);		//计算阶段相关的信息
    SetNextCycleRealtimePattern(calInfo);
	CalConflictChannel(gRunConfigPara->stPhase[calInfo->phaseTableId - 1], gRunConfigPara->stChannel[calInfo->channelTableId - 1], gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1], gConflictChannel[1]);
	SetChannelBits(calInfo);	//设置相位对应的通道bit位
	return TRUE;
}

void ItsGetRealtimePattern(void *udpdata)
{
	UInt32 *udpHead = (UInt32 *)udpdata;
	MsgRealtimePattern *p = (MsgRealtimePattern *)udpdata;
	if (udpdata == NULL)
		return;
	if (udpHead[2] == 1)
	{	//udp数据的第三个int为1表明获取的是当前周期方案信息
		memcpy(p, &gCurrentCycleRealtimePattern, sizeof(MsgRealtimePattern));
		p->ucIsCurrentCycle = 1;
	}
	else
	{
		memcpy(p, &gNextCycleRealtimePattern, sizeof(MsgRealtimePattern));
		p->ucIsCurrentCycle = 0;
	}
	p->unExtraParamHead = 0x6e6e;				//消息头，默认为0x6e6e
	p->unExtraParamID = 0xd0;					//消息类型，该信息方案数据的的值为0xd0
}

//设置实时信息，1s一次
void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data)
{
	int i;
	const PhaseInfo *phaseInfos = data->phaseInfos;
	
	pthread_rwlock_wrlock(&gCountDownLock);
	memset(&gCountDownParams, 0, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	gCountDownParams.unExtraParamHead = 0x6e6e;
	gCountDownParams.unExtraParamID = 0x9e;
	gCountDownParams.ucPlanNo = data->schemeId;
	gCountDownParams.ucCurCycleTime = (data->actionId == INDUCTIVE_COORDINATE_ACTIONID) ? data->inductiveCoordinateCycleTime : data->cycleTime;
	gCountDownParams.ucCurRunningTime = (data->cycleTime) ? (data->cycleTime - data->leftTime + 1) : 0;	//周期运行时间是从1开始
	if (gCountDownParams.ucCurRunningTime == 1)	//每个周期第1s更新当前周期方案信息
	{
		memcpy(&gCurrentCycleRealtimePattern, &gNextCycleRealtimePattern, sizeof(MsgRealtimePattern));
	}
		
	switch (data->schemeId)
	{
		case YELLOWBLINK_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, (ItsControlStatusGet() == 10) ? "故障黄闪" : "黄闪"); break;
		case ALLRED_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "全红"); break;
		case TURNOFF_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "关灯"); break;
		case INDUCTIVE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "感应"); break;
		case INDUCTIVE_COORDINATE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "协调感应"); break;
		case SINGLE_ADAPT_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "自适应"); break;
	}
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		gCountDownParams.stVehPhaseCountingDown[i][0] = phaseInfos[i].phaseStatus;
		gCountDownParams.stVehPhaseCountingDown[i][1] = phaseInfos[i].phaseLeftTime;
		gCountDownParams.stPedPhaseCountingDown[i][0] = phaseInfos[i].pedestrianPhaseStatus;
		gCountDownParams.stPedPhaseCountingDown[i][1] = phaseInfos[i].pedestrianPhaseLeftTime;

		gCountDownParams.ucOverlap[i][0] = phaseInfos[i].followPhaseStatus;
		gCountDownParams.ucOverlap[i][1] = phaseInfos[i].followPhaseLeftTime;
		gCountDownParams.stPhaseRunningInfo[i][0] = phaseInfos[i].splitTime;
		gCountDownParams.stPhaseRunningInfo[i][1] = (phaseInfos[i].phaseStatus != RED && phaseInfos[i].splitTime > 0) ? (phaseInfos[i].splitTime - phaseInfos[i].phaseSplitLeftTime + 1) : 0;
	}
	
//	gCountDownParams.ucChannelLockStatus = (lockparams->ucChannelLockStatus == CHANGEABLE_CHANNEL_LOCK) ? 0 : lockparams->ucChannelLockStatus;	//如果是可变通道锁定则倒计时中不显示为锁定状态
	gCountDownParams.ucChannelLockStatus = lockflag;	//如果是可变通道锁定则倒计时中不显示为锁定状态
	memcpy(gCountDownParams.ucChannelStatus, data->allChannels, sizeof(gCountDownParams.ucChannelStatus));	//把之前的锁定状态全部改为实时的通道状态
	memcpy(gCountDownParams.ucChannelCountdown, data->channelCountdown, sizeof(gCountDownParams.ucChannelCountdown));
	
	gCountDownParams.ucIsStep = data->isStep;
	gCountDownParams.ucMaxStageNum = data->maxStageNum;
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsCountDownGet(void *countdown, int size)
{
	if (countdown == NULL || size != sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS))
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(countdown, &gCountDownParams, size);
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
void ItsSetCurRunData(LineQueueData *data)
{
	if (data == NULL)
		return;
	//INFO("Set running data...0:%d, 1:%d", data->allChannels[0], data->allChannels[1]);
	pthread_rwlock_wrlock(&gCountDownLock);
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsGetConflictChannel(UInt32 *conflictChannels)
{
	if (conflictChannels == NULL)
		return;
	memcpy(conflictChannels, gConflictChannel[0], sizeof(gConflictChannel[0]));
}

//输出红绿信号到倒计时器
void ItsCountDownOutput(LineQueueData *data)
{
    CountDownInterface(data);
}

UNUSEDATTR static void PrintVehCountDown(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n\n",\
		GET_COLOR(phaseInfos[0].phaseStatus), phaseInfos[0].phaseLeftTime,
		GET_COLOR(phaseInfos[1].phaseStatus), phaseInfos[1].phaseLeftTime,
		GET_COLOR(phaseInfos[2].phaseStatus), phaseInfos[2].phaseLeftTime,
		GET_COLOR(phaseInfos[3].phaseStatus), phaseInfos[3].phaseLeftTime,
		GET_COLOR(phaseInfos[4].phaseStatus), phaseInfos[4].phaseLeftTime,
		GET_COLOR(phaseInfos[5].phaseStatus), phaseInfos[5].phaseLeftTime,
		GET_COLOR(phaseInfos[6].phaseStatus), phaseInfos[6].phaseLeftTime,
		GET_COLOR(phaseInfos[7].phaseStatus), phaseInfos[7].phaseLeftTime);
}

UNUSEDATTR static void PrintRunInfo(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintRunInfo>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d",\
		phaseInfos[0].splitTime - phaseInfos[0].phaseSplitLeftTime + 1, phaseInfos[0].splitTime,
		phaseInfos[1].splitTime - phaseInfos[1].phaseSplitLeftTime + 1, phaseInfos[1].splitTime,
		phaseInfos[2].splitTime - phaseInfos[2].phaseSplitLeftTime + 1, phaseInfos[2].splitTime,
		phaseInfos[3].splitTime - phaseInfos[3].phaseSplitLeftTime + 1, phaseInfos[3].splitTime,
		phaseInfos[4].splitTime - phaseInfos[4].phaseSplitLeftTime + 1, phaseInfos[4].splitTime,
		phaseInfos[5].splitTime - phaseInfos[5].phaseSplitLeftTime + 1, phaseInfos[5].splitTime,
		phaseInfos[6].splitTime - phaseInfos[6].phaseSplitLeftTime + 1, phaseInfos[6].splitTime,
		phaseInfos[7].splitTime - phaseInfos[7].phaseSplitLeftTime + 1, phaseInfos[7].splitTime,
		data->cycleTime - data->leftTime + 1, data->cycleTime);
}
//此处只是举一个ItsCustom函数实现的例子,可以模仿去做一些其他的事情
void ItsCustom(LineQueueData *data, CustomParams *customParams)
{
	UInt16 redFlashSec = gStructBinfileCustom.sCountdownParams.redFlashSec;
	UInt8 isYellowFlash = GET_BIT(gStructBinfileCustom.sCountdownParams.option, 0);
	int i;

	customParams->addTimeToFirstPhase = gAddTimeToFirstPhase;
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (data->allChannels[i] == YELLOW && isYellowFlash)
			data->allChannels[i] = YELLOW_BLINK;
		if (data->allChannels[i] == RED && redFlashSec >= data->channelCountdown[i])
			data->allChannels[i] = RED_BLINK;
	}
	if (data->cycleTime == data->leftTime)	//每个周期刚开始时更新通道冲突表内容
		memcpy(gConflictChannel[0], gConflictChannel[1], sizeof(gConflictChannel[1]));
#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
	if (gStructBinfileConfigPara.sSpecialParams.iPhaseTakeOverSwtich)
		SendRunInfoTOBoard(data, gRunConfigPara);
#endif
	PrintRunInfo(data);
	//PrintVehCountDown(data);
}
