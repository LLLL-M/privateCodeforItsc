/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : strategy.c
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月27日
  最近修改   :
  功能描述   : 策略控制模块，主要控制点灯逻辑
  函数列表   :
              GetActionID
              GetPhaseTurnAndSetLightTime
              GetTimeIntervalID
			  GetChannelStatusByTime
              InitChannelLightStatus
              IsPhaseInPhaseTurn
              RunCycle
              SetCurrentChannelStatus
              SetPhaseChannelArrayTotal
              SetPhaseGreenSignalRationItem
              SignalControllerRun
  修改历史   :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "HikConfig.h"
#include "light.h"
#include "Util.h"
#include "HikMsg.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //把时分转换成分
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //一天中的最大时间差值，即24小时
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern int msgid;
extern SignalControllerPara *gSignalControlpara;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
/*备份配置参数的全局指针，当有新的配置信息下达后，信号控制模块便会复制一份配置参数，在后面的控制中就只会使用这一份数据，
	如果只有一份数据的话，那么可能会出现一边重新读取配置文件信息一边又同时使用配置信息的情况，所以为避免冲突另外备份一份数据 */
SignalControllerPara *gConfigPara = NULL;//信号机配置参数备份
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static unsigned short nCycleTime = 0;//周期长
static lightTime_t gPhaseLightTime[NUM_PHASE];//当前周期内每个相位的点灯时间



/*****************************************************************************
 函 数 名  : SetPhaseGreenSignalRationItem
 功能描述  : 根据绿信比号初始化相位表中相关的内容，主要初始化相位的绿信比时
             间和绿灯时间
 输入参数  : unsigned short  nGreenSignalRatioID  绿信比号
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void SetPhaseGreenSignalRationItem(unsigned short  nGreenSignalRatioID)
{  
    int i = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	int timeGap = 0;

	memset(gPhaseLightTime,0,sizeof(gPhaseLightTime));
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        splitItem = gConfigPara->stGreenSignalRation[nGreenSignalRatioID - 1] + i;
//      splitItem = &gConfigPara->stGreenSignalRation[nGreenSignalRatioID - 1][i];
		phaseItem = gConfigPara->stPhase + i;
		entry = gConfigPara->AscSignalTransTable + i;
		if (phaseItem->nPhaseID != i + 1)
			continue;

		gPhaseLightTime[i].greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		gPhaseLightTime[i].greenBlinkTime = entry->nGreenLightTime;
		gPhaseLightTime[i].yellowTime = phaseItem->nYellowTime;
		gPhaseLightTime[i].allRedTime = phaseItem->nAllRedTime;
		/*如果相位设置了"行人自动请求"项，或者行人放行时间和行人清空时间之和大于机动车相位的绿灯和绿闪时间之和，那么行人放行时间和行人清空时间需要依赖机动车相位的绿灯和绿闪时间来调整 */
		if ((BIT(phaseItem->wPhaseOptions, 8) == 1) 
			|| (gPhaseLightTime[i].greenTime + entry->nGreenLightTime < phaseItem->nPedestrianPassTime + phaseItem->nPedestrianClearTime)) 
		{					
			timeGap = gPhaseLightTime[i].greenTime + entry->nGreenLightTime - phaseItem->nPedestrianClearTime;
			if (timeGap > 0) 
			{
				gPhaseLightTime[i].pedestrianPassTime = (unsigned short)timeGap;
				gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			} 
			else 
			{
				gPhaseLightTime[i].pedestrianPassTime = 0;
				gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime + timeGap;
			}
		} 
		else 
		{	
			gPhaseLightTime[i].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
		}
        //log_debug("nPhaseId   %d   nTime   %d\n",nPhaseID,splitItem->nGreenSignalRationTime);
		//printf("DEBUG----- passTime: %d, clearTime: %d\n", gPhaseLightTime[i].pedestrianPassTime, gPhaseLightTime[i].pedestrianClearTime);
    }
}

/*****************************************************************************
 函 数 名  : GetChannelStatusByTime
 功能描述  : 获取当前时间段内被使用通道的状态
 输入参数  : lightTime_t *times  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline lightStatus GetChannelStatusByTime(lightTime_t *times)
{
    if (times->greenTime)
    {
		times->greenTime--;
		return GREEN;
	}
	if (times->greenBlinkTime)
    {
		times->greenBlinkTime--;
		return GREEN_BLINK;
	}
	if (times->yellowTime)
    {
		times->yellowTime--;
		return YELLOW;
	}
	if (times->allRedTime)
    {
		times->allRedTime--;
		return RED;
	}
	return TURN_OFF;
}

/*****************************************************************************
 函 数 名  : SetCurrentChannelStatus
 功能描述  : 很重要的接口函数，根据当前运行相位和下一个即将运行的相位决定当前时段内所有通道应处的状态
 输入参数  : lightStatus *allChannel	所有通道的状态  
             unsigned short nPhaseId     当前运行的相位
             unsigned short nextPhaseId       下一个即将运行的相位 
             PPhaseChannel nPhaseChannelArray	相位和通道对应关系的数组指针    
 返 回 值  : 如果当前相位的绿信比消耗完那么返回false，否则就根据各个状态的时间片决定当前相位对应通道的状态
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int SetCurrentChannelStatus(lightStatus *allChannel, unsigned short nPhaseId, unsigned short nextPhaseId, PPhaseChannel nPhaseChannelArray)
{    
    int i = 0, j = 0, ret = true;
    lightStatus status, tmp;
    unsigned short channelId = 0;
	lightTime_t *times = &gPhaseLightTime[nPhaseId - 1];
	PPhaseChannel phaseChannel = &nPhaseChannelArray[nPhaseId - 1];
	PPhaseChannel nextPhaseChannel = &nPhaseChannelArray[nextPhaseId - 1];
	
	//备份行人通道的放行时间和清空时间
	unsigned short pedestrianPassTime = times->pedestrianPassTime;
	unsigned short pedestrianClearTime = times->pedestrianClearTime;

	status = GetChannelStatusByTime(times);
    if (status == TURN_OFF) 
    {	//表面当前相位的绿信比时间用完了
		status = RED;	//把这个相位所对应的通道全部变为红色
		ret = false;
    }
    for(i = 0; i < phaseChannel->num; i++)//
    {
    	channelId = phaseChannel->channel[i];
		tmp = status;
        switch (gConfigPara->stChannel[channelId - 1].nControllerType) 
        {
			case MOTOR:	//如果是机动车通道就直接按照绿信比时间对应的状态
				break;
			case PEDESTRIAN:
			/*从备份中恢复当前剩余的行人放行时间和清空时间，防止一个相位对应多个行人通道时时间被多次递减的问题 */
				times->pedestrianPassTime = pedestrianPassTime;
				times->pedestrianClearTime = pedestrianClearTime;				
			
				if (times->pedestrianPassTime) 
				{
					times->pedestrianPassTime--;
					tmp = GREEN;
					break;
				}
				if (times->pedestrianClearTime) 
				{
					times->pedestrianClearTime--;
					tmp = GREEN_BLINK;
					break;
				}
				tmp = RED;
				break;
			case FOLLOW:	//如果是跟随通道还需要根据此通道是否在下一相位仍旧运行来决定它的绿灯时间
				for (j = 0; j < nextPhaseChannel->num; j++) 
				{
					if (channelId == nextPhaseChannel->channel[j]) 
					{
						tmp = GREEN;
						break;
					}
				}
				break;
			default:	break;
        }

		allChannel[channelId - 1] = tmp;
    }

    return ret;
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
static unsigned char GetTimeIntervalID(struct tm *now)
{
    int i = 0;

    for(i = 0 ; i < NUM_SCHEDULE ; i++) 
    { //如果当前时间和某一项调度表相符，则返回该调度表的时段号
        if (BIT(gConfigPara->stPlanSchedule[i].month, now->tm_mon + 1) == 1) 
        {													//因为tm_mon范围是[0,11]
            if (BIT(gConfigPara->stPlanSchedule[i].day, now->tm_mday) == 1) 
            {
			    return gConfigPara->stPlanSchedule[i].nTimeIntervalID;
			}
			if(BIT(gConfigPara->stPlanSchedule[i].week, (now->tm_wday == 0) ? 7 : now->tm_wday) == 1) 
			{												//tm_wday == 0代表星期日
				return gConfigPara->stPlanSchedule[i].nTimeIntervalID;
			}
		}
    }

    return 0;
}



/*****************************************************************************
 函 数 名  : GetActionID
 功能描述  : 根据时段表ID，遍历该时段表下的时段，根据当前时间，判断应属于哪
             个时段，进而确定当前时段需要执行的动作
 输入参数  : 当前运行的时段表号
 输出参数  : 无
 返 回 值  : 当前运行的动作号

 修改历史      :
  1.日    期   : 2014年11月20日
    作    者   : Jicky
    修改内容   : 修改了输入参数，并重新修改了函数实现

*****************************************************************************/
static unsigned char GetActionID(unsigned char nTimeIntervalID, struct tm *now)
{
    int i = 0;
	int nCurrentTime, nTempTime;
	int nTimeGap, oldTimeGap, index = -1;
    
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	oldTimeGap = MAX_TIME_GAP;	//预先设置一个最大的差值
	while (index == -1) 
	{	//循环找出当前时间与时段表中差值最小的时段所对应的actionID即为当前应该运行的actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if(gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //这说明该时段可能没有被使用，直接continue掉
				continue;
			}

			nTempTime = HoursToMinutes(gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour,gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute);
			if (nCurrentTime == nTempTime) 
			{
				return gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID;
			} 
			else if (nCurrentTime > nTempTime) 
			{
				nTimeGap = nCurrentTime - nTempTime;
				if (nTimeGap < oldTimeGap) 
				{
					oldTimeGap = nTimeGap;
					index = i;
				}
			}
		}
		
		if (oldTimeGap == MAX_TIME_GAP) 
		{ //说明当前时间处于时段表中最小的位置
			nCurrentTime += MAX_TIME_GAP;	//把当前时间增加24小时然后再次循环
		}
	}

	return gConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;
}


/*****************************************************************************
 函 数 名  : GetPhaseTurnAndSetLightTime
 功能描述  : 根据当前时间一次查询调度表、时段表、动作表、方案表最终获取当前应该使用的绿信比表和相序
			并根据绿信比表的时间对相位表的相关内容进行初始化，同时保存全局变量周期nCycleTime
 返 回 值  : 返回当前时间应使用的相序号

 修改历史      :
  1.日    期   : 2014年11月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数
*****************************************************************************/
static unsigned short GetPhaseTurnAndSetLightTime(void)
{
    unsigned short  nTimeIntervalID = 0;
    unsigned short  nActionID = 0;
    unsigned short  nSchemeID = 0;
    unsigned short  nGreenSignalRatioID = 0;
    unsigned short  nPhaseTurnID = 0;
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);
    //start ...... 计算一个周期内的相位
    nTimeIntervalID = GetTimeIntervalID(&now);//根据当前时间，遍历调度表，得到时段表ID
    if(nTimeIntervalID == 0)
    {
        return 0;
    }

    nActionID = GetActionID(nTimeIntervalID, &now);//根据时段表ID，得到动作表ID
    if(nActionID == 0)
    {
        return 0;
    }

    nSchemeID = gConfigPara->stAction[nActionID - 1].nSchemeID;//根据动作表ID，得到方案表ID
    if(nSchemeID == 0)
    {
        return 0;
    }
    
    nGreenSignalRatioID = gConfigPara->stScheme[nSchemeID - 1].nGreenSignalRatioID;//根据方案表ID，得到绿信比表ID
    if(nGreenSignalRatioID == 0)
    {
        return 0;
    }

	nCycleTime = gConfigPara->stScheme[nSchemeID - 1].nCycleTime;
    nPhaseTurnID = gConfigPara->stScheme[nSchemeID - 1].nPhaseTurnID;//根据方案表ID，得到相序表ID
    if(nPhaseTurnID == 0)
    {
        return 0;
    }
    
    log_debug("%s  nTimeIntervalID  : %d nActionID  : %d  nSchemeID  :  %d  nGreenSignalRatioID :  %d  nPhaseTurnID  :  %d\n",__func__,nTimeIntervalID,nActionID,nSchemeID,nGreenSignalRatioID,nPhaseTurnID);
    //end ...... 计算一个周期内的相位

    SetPhaseGreenSignalRationItem(nGreenSignalRatioID);//根据当前相位ID及当前的绿信比表ID，给相位表中的绿信比参数赋值。

#ifndef ARM_PLATFORM
    sleep(2);
#endif
    return nPhaseTurnID;
}

/*****************************************************************************
 函 数 名  : IsPhaseInPhaseTurn
 功能描述  : 判断相位是否在相序中
 输入参数  : unsigned short nPhaseTurn  相序号
             unsigned short nPhaseId    相位号
 返 回 值  : 如果相位在相序中则返回true，反正返回false
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static int IsPhaseInPhaseTurn(unsigned short nPhaseTurn, unsigned short nPhaseId)
{
    int i;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		if (gConfigPara->stPhaseTurn[nPhaseTurn - 1][0].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][1].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][2].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][3].nTurnArray[i] == nPhaseId)
			return true;
	}

	return false;
}

/*****************************************************************************
 函 数 名  : SetPhaseChannelArrayTotal
 功能描述  : 找出当前相序中相位和通道的对应关系
 输入参数  : unsigned short nPhaseTurn         当前相序表
             PPhaseChannel nPhaseChannelArray  相位与通道的对应关系结构体指针
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void SetPhaseChannelArrayTotal(unsigned short nPhaseTurn, PPhaseChannel nPhaseChannelArray)
{     
    int i = 0;
    int j = 0;
    unsigned short nTempId = 0, motherPhase = 0;
	PPhaseChannel phaseChannel = NULL;

	memset(nPhaseChannelArray, 0, sizeof(PhaseChannel) * NUM_PHASE);
    for(i = 0 ; i < NUM_CHANNEL; i++)
    {
        if(gConfigPara->stChannel[i].nChannelID != 0)
        {
            nTempId = gConfigPara->stChannel[i].nControllerID;
			phaseChannel = &nPhaseChannelArray[nTempId - 1];
            if((gConfigPara->stChannel[i].nControllerType == MOTOR) || (gConfigPara->stChannel[i].nControllerType == PEDESTRIAN))
            {
				if (BIT(gConfigPara->stPhase[nTempId - 1].wPhaseOptions, 0) == 0) 
				{//相位未使能
					continue;
				}
				if (IsPhaseInPhaseTurn(nPhaseTurn, nTempId) == true) 
				{
					phaseChannel->channel[phaseChannel->num++] = i + 1;
				}          
            } 
            else if(gConfigPara->stChannel[i].nControllerType == FOLLOW) 
            {
                for(j = 0 ; j < NUM_PHASE; j++)
                {
                	motherPhase = gConfigPara->stFollowPhase[nTempId - 1].nArrayMotherPhase[j];
                    if(motherPhase == 0) 
                    {
                        break;
                    }
					if (BIT(gConfigPara->stPhase[motherPhase - 1].wPhaseOptions, 0) == 0) 
					{//相位未使能
						continue;
					}
                    if(IsPhaseInPhaseTurn(nPhaseTurn, motherPhase) == true)
                    {
                    	phaseChannel = &nPhaseChannelArray[motherPhase - 1];
                        phaseChannel->channel[phaseChannel->num++] = i + 1;
                    }
                }
            }

        }

    }
}

/*****************************************************************************
 函 数 名  : InitChannelLightStatus
 功能描述  : 根据相位与通道的对应关系，初始化每个通道的状态
 输入参数  : lightStatus *allChannel           所有通道状态的结构体指针
             lightStatus status                设置的状态，一般都设为RED
             PPhaseChannel nPhaseChannelArray  相位与通道的对应关系结构体指针
 返 回 值  :  
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void InitChannelLightStatus(lightStatus *allChannel, lightStatus status, PPhaseChannel nPhaseChannelArray)
{
	int i, j;
	PPhaseChannel phaseChannel = NULL;

	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		allChannel[i] = (gConfigPara->stChannel[i].nChannelID != 0) ? status : TURN_OFF;
	}
	
	if (nPhaseChannelArray == NULL)
		return;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		phaseChannel = &nPhaseChannelArray[i];
		for (j = 0; j < phaseChannel->num; j++) 
		{
			allChannel[phaseChannel->channel[j] - 1] = status;
		}
	}
}

/*****************************************************************************
 函 数 名  : LightPerSecond
 功能描述  : 根据通道的状态循环设置点灯的数组，接收定时完成的消息，然后发送给
             点灯模块，每秒钟应该被调用一次
 输入参数  : lightStatus *allChannel  描述所有通道状态的数组
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void LightPerSecond(lightStatus *allChannel)
{
	int i;
	unsigned short nOutLampArray[NUM_BOARD] = {0};
	struct msgbuf msg = {0, {0}};
	for (i = 0; i < LOOP_TIMES_PER_SECOND; i++)
	{
		ControlLampLight(allChannel, nOutLampArray);
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_TIMER_COMPLETE, 0) == -1) 
		{	//检查定时是否完成
			log_error("error when yellowBlink, error info: %s\n", strerror(errno));
			i--;
			continue;
		}
		msg.mtype = MSG_LIGHT;
		memcpy(msg.mtext, nOutLampArray, sizeof(nOutLampArray));
		msgsnd(msgid, &msg, MSGSIZE, 0);
	}
}

/*****************************************************************************
 函 数 名  : RunCycle
 功能描述  : 通过传入的相序、所有通道的状态以及相位与通道的对应关系来运行一
             个周期
 输入参数  : unsigned short nPhaseTurn           当前要运行的相序
             lightStatus *nCurrentChannelStatus  存放当前1s中所有通道状态的结构体指针
             PPhaseChannel nPhaseChannelArray    相位与通道的对应关系结构体指针
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void RunCycle(unsigned short nPhaseTurn, 
					 lightStatus *nCurrentChannelStatus, 
					 PPhaseChannel nPhaseChannelArray)
{
	int i = 0, ring = 0;
    unsigned short  nPhaseId = 0, nextPhaseId = 0;
	
	while (nCycleTime--) 
	{	//一次递减1s
		for (ring = 0; ring < 4; ring++) 
		{	//每1s从相序的四个环中分别取出这1s内应该执行的相位所对应的通道的状态
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[i];
				nextPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[((i + 1) == NUM_PHASE) ? 0 : (i + 1)];
				if (nPhaseId == 0) 
				{
					break;
				}
				if (nextPhaseId == 0) 
				{
					nextPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[0];
				}
				//根据相位设置通道状态，若是设置失败说明此相位的绿信比时间耗完，应该继续找寻下一个相位并进行设置，直到设置成功为止。
				if (SetCurrentChannelStatus(nCurrentChannelStatus, nPhaseId, nextPhaseId, nPhaseChannelArray) == true) 
				{
					break;
				}
			}
		}
		//根据前面设置的通道状态对通道进行点灯
		LightPerSecond(nCurrentChannelStatus);	//每1s调用一次
	}
}


/*****************************************************************************
 函 数 名  : SignalControllerRun
 功能描述  : 信号机调度函数，主要根据当前时间获取应使用的相序已经被使用相位的绿信比时间，然后根据每个相位绿信比时间觉得对应通道应处的状态，使用全局变量周期nCycleTime每1s循环一次，即每1s获取一次各通道的状态然后进行点灯处理。
 返 回 值  : 若相序获取失败便返回false，否则便一直运行调度控制信号灯
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *SignalControllerRun(void *arg)
{
	unsigned short nPhaseTurn = 0;
	lightStatus nCurrentChannelStatus[NUM_CHANNEL] = {TURN_OFF};//当前时段内每个通道的状态
	PhaseChannel nPhaseChannelArray[NUM_PHASE];
    int i = 0;
	struct msgbuf msg = {MSG_CONFIG_UPDATE, {0}};

	//重新申请一块内存，用来备份配置信息
	if (gConfigPara == NULL) 
	{
		gConfigPara = (SignalControllerPara *)calloc(sizeof(SignalControllerPara), 1);
		if (gConfigPara == NULL) 
		{
			log_error("memory is not enough to store the backup config information\n");
			pthread_exit(NULL);
		}
	}
	while (msgrcv(msgid, &msg, MSGSIZE, MSG_CONFIG_UPDATE, 0) == -1) 
	{
		usleep(100000);	//延时100ms然后再次读取消息，直到读取到为止
	}
	//备份配置信息
	memcpy(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
	//发送定时器启动的消息
	msg.mtype = MSG_TIMER_START;
	msgsnd(msgid, &msg, MSGSIZE, 0);

	log_debug("start to excute yellow blink status");
	//启动黄闪
	InitChannelLightStatus(nCurrentChannelStatus, YELLOW_BLINK, NULL);
	for (i = 0; i < gConfigPara->stUnitPara.nBootYellowLightTime; i++) 
	{
		LightPerSecond(nCurrentChannelStatus);
	}
	log_debug("start to excute all red status");
    //启动全红
	InitChannelLightStatus(nCurrentChannelStatus, RED, NULL);
	for (i = 0; i < gConfigPara->stUnitPara.nBootAllRedTime; i++) 
	{
		LightPerSecond(nCurrentChannelStatus);
	}

	log_debug("start to run cycle");
    while(1)
    {
		//如果接收到新的配置信息，先备份再使用
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_CONFIG_UPDATE, IPC_NOWAIT) != -1) 
		{
			memcpy(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
		}
		//根据当前时间获取相序，并初始化各相位的绿信比
		nPhaseTurn = GetPhaseTurnAndSetLightTime();
		if (nPhaseTurn == 0) 
		{
			log_error("get phaseTurn error");
			continue;
		}
		//找出相序中每个相位所使用的通道，确定他们的对应关系
		SetPhaseChannelArrayTotal(nPhaseTurn, nPhaseChannelArray);
		//初始化被使用通道的状态为全红
		InitChannelLightStatus(nCurrentChannelStatus, RED, nPhaseChannelArray);
		//每次轮询一个完整的周期
        RunCycle(nPhaseTurn, nCurrentChannelStatus, nPhaseChannelArray);
    }

    pthread_exit(NULL);
}

