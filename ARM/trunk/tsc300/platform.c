#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "HikConfig.h"
#include "platform.h"
#include "lfq.h"
#include "common.h"

#define INDUCTIVE_CONTROL_ACTION_ID		118
#define INDUCTIVE_CONTROL_SCHEME_ID		254

pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;	//主要保护多线程访问全局配置
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //倒计时接口信息
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //用来供udpserver调用的倒计时缓存


void *gHandle = NULL;	//倒计时使用的队列句柄

UInt8 gChannelLockFlag;   //0表示未锁定，1表示锁定,2表示待锁定（待锁定状态表示收到了通道锁定命令但是当前时间为非锁定时间段的状态）
UInt8 gSpecialControlSchemeId;       //特殊控制方案号

PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //当前使用的相序

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;

extern PATTERN_NAME_PARAMS pattern_name_params;        //方案描述

extern CountDownVeh countdown_veh[17];                 //机动车相位倒计时参数
extern CountDownPed countdown_ped[17];				   //行人相位倒计时参数

extern UInt8 GetActionID();

void InitCountDownParams(void)
{
	gCountDownParams = calloc(1, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	if (gCountDownParams == NULL)
	{
		ERR("No memory enough to store count down params!");
		exit(1);
	}
	gCountDownParams->unExtraParamHead = 0x6e6e;
	gCountDownParams->unExtraParamID = 0x9e;
	
	memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
}

static UInt8 CalCountDownCalledCount()	//计算倒计时接口总共调用次数
{
	UInt8 MotorCount[NUM_PHASE] = {0};	//记录机动车相位调用次数
	UInt8 PedCount[NUM_PHASE] = {0};	//记录行人相位调用次数
	UInt8 phaseId = 0;
	ControllerType type;
	int i;
	UInt8 sum = 0;
	
	for (i = 0; i < NUM_CHANNEL; i++)
	{
		phaseId = gSignalControlpara->stChannel[i].nControllerID;
		if (phaseId == 0)
			continue;
		type = gSignalControlpara->stChannel[i].nControllerType;
		if (type == MOTOR && MotorCount[phaseId - 1] == 0)
			MotorCount[phaseId - 1] = 1;
		else if (type == PEDESTRIAN && PedCount[phaseId - 1] == 0)
			PedCount[phaseId - 1] = 1;
	}
	for (i = 0; i < NUM_PHASE; i++)
	{
		sum += MotorCount[i] + PedCount[i];
	}
	return sum;
}

static void SetCountDownParams(UInt8 nSchemeID, GreenSignalRationItem *splitTable, PhaseItem *phaseTable)
{
	int i = 0;
	
	gCountDownParams->ucPlanNo = nSchemeID;
	memset(gCountDownParams->stPhaseRunningInfo, 0, sizeof(gCountDownParams->stPhaseRunningInfo));
	if (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)	//说明是感应控制
	{
		for(i = 0; i < NUM_PHASE; i++)
		{
			if (phaseTable[i].nCircleID == 0)
				continue;
			gCountDownParams->stPhaseRunningInfo[i][0] = phaseTable[i].nMinGreen + phaseTable[i].nYellowTime + phaseTable[i].nAllRedTime;
		}
	}
	else
	{
		for(i = 0 ; i < NUM_PHASE; i++)
		{
			gCountDownParams->stPhaseRunningInfo[i][0] = splitTable[i].nGreenSignalRationTime;
		}
	}
	gCountDownParams->ucChannelLockStatus = gChannelLockFlag;
	if (nSchemeID > 0 && nSchemeID <= 16)
		memcpy(gCountDownParams->ucCurPlanDsc, pattern_name_params.stPatternNameDesc[nSchemeID - 1], sizeof(gCountDownParams->ucCurPlanDsc));
	for(i = 0; i < NUM_PHASE; i++)
	{
		gCountDownParams->stVehPhaseCountingDown[i][0] = countdown_veh[i + 1].veh_color;
		gCountDownParams->stVehPhaseCountingDown[i][1] = countdown_veh[i + 1].veh_phaseTime;
		gCountDownParams->stPedPhaseCountingDown[i][0] = countdown_ped[i + 1].ped_color;
		gCountDownParams->stPedPhaseCountingDown[i][1] = countdown_ped[i + 1].ped_phaseTime;
	}
}

static UInt8 GetSchemeId(GreenSignalRationItem *splitTable, PhaseItem *phaseTable, PhaseTurnItem *phaseTrunTable, FollowPhaseItem *followPhaseTable, UInt8 *greenBlinkTimes)
{
	UInt8 nActionID = 0;
	UInt8 nSchemeID = 0;
	UInt8 nGreenSignalRatioID = 0;
	UInt8 nPhaseTurnID = 0;
	int i;
	
	if (gSpecialControlSchemeId == INDUCTIVE_CONTROL_SCHEME_ID)
		nSchemeID = INDUCTIVE_CONTROL_SCHEME_ID;
	else
	{
		nActionID = GetActionID();
		if (nActionID == 0)
		{
			DBG("Current time has not config action!");
			return 0;
		}
		else if (nActionID == INDUCTIVE_CONTROL_ACTION_ID)
			nSchemeID = INDUCTIVE_CONTROL_SCHEME_ID;
		else
			nSchemeID = gSignalControlpara->stAction[nActionID - 1].nSchemeID;
	}
	
	if (nSchemeID == 0)
		DBG("action %d has not config scheme!", nActionID);
	else
	{	//感应控制默认使用绿信比表1和相序表1
		nGreenSignalRatioID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gSignalControlpara->stScheme[nSchemeID - 1].nGreenSignalRatioID;
		nPhaseTurnID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gSignalControlpara->stScheme[nSchemeID - 1].nPhaseTurnID;
		if(nGreenSignalRatioID == 0 || nPhaseTurnID == 0)
		{
			if (nGreenSignalRatioID == 0)
				DBG("scheme %d has not config split", nSchemeID);
			else
				DBG("scheme %d has not config phaseTurn", nSchemeID);
			nSchemeID = 0;
		}
		else
		{
			memcpy(splitTable, gSignalControlpara->stGreenSignalRation[nGreenSignalRatioID - 1], sizeof(GreenSignalRationItem) * NUM_PHASE);
			memcpy(phaseTrunTable, gSignalControlpara->stPhaseTurn[nPhaseTurnID - 1], sizeof(PhaseTurnItem) * NUM_RING_COUNT);
			memcpy(phaseTable, gSignalControlpara->stPhase, sizeof(gSignalControlpara->stPhase));
			memcpy(followPhaseTable, gSignalControlpara->stFollowPhase, sizeof(gSignalControlpara->stFollowPhase));
			for (i = 0; i < NUM_PHASE; i++)
				greenBlinkTimes[i] = gSignalControlpara->AscSignalTransTable[i].nGreenLightTime;
		}	
	}
	return nSchemeID;
}

/*****************************************************************************
 函 数 名  : IsNextPhaseInFollowPhaseList
 功能描述  : 判断下一相位是否在跟随相位表中
 输入参数  : 
 返 回 值  : 如果相位在相序中则返回TRUE，反正返回false
 修改历史  
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline Boolean IsNextPhaseInFollowPhaseList(FollowPhaseItem *followPhaseItem, UInt8 nextPhaseId)
{
    int i;
	UInt8 motherPhase;
	if (nextPhaseId == 0)
		return FALSE;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		motherPhase = followPhaseItem->nArrayMotherPhase[i];
		if (motherPhase == 0) 
			break;
		if (nextPhaseId == motherPhase)
			return TRUE;
	}
	return FALSE;
}

static UInt8 CalFollowPhaseCountDown(UInt8 *nextPhaseIds, FollowPhaseItem *followPhaseItem, PhaseItem *phaseTable, PhaseChannelStatus status)
{
	int i;
	PhaseChannelStatus motherPhaseStatus, bakMotherPhase;
	UInt8 motherPhase, nextPhase;
	UInt8 followPhaseCountDown = 0, motherPhaseCountDown = 0;
	UInt8 uIsPhaseRunning = 0;//判断相位是否在运行
	
	for (i = 0; i < NUM_PHASE; i++)
	{
		motherPhase = followPhaseItem->nArrayMotherPhase[i];
		if (motherPhase == 0) 
			break;
		motherPhaseStatus = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][0];
		motherPhaseCountDown = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][1];
		uIsPhaseRunning = gCountDownParams->stPhaseRunningInfo[motherPhase - 1][1];//判断相位是否运行的条件是该相位运行时间是否为0
		switch (status)
		{
			case GREEN:
				if (uIsPhaseRunning != 0)//modified by xiaowh
				{	
					bakMotherPhase = motherPhase;
					do
					{
						nextPhase = nextPhaseIds[motherPhase - 1];
						if (IsNextPhaseInFollowPhaseList(followPhaseItem, nextPhase) == FALSE)
						{
							if (motherPhaseStatus == GREEN)
								followPhaseCountDown = max(followPhaseCountDown, motherPhaseCountDown);
							break;
						}
						else
						{
							followPhaseCountDown = gCountDownParams->stVehPhaseCountingDown[nextPhase - 1][1]
												+ gCountDownParams->stPhaseRunningInfo[nextPhase - 1][0]
												- phaseTable[nextPhase - 1].nYellowTime
												- phaseTable[nextPhase - 1].nAllRedTime;
							motherPhase = nextPhase;
						}
					} while (bakMotherPhase != motherPhase);	 
				}
				break;
			case YELLOW:
				if (motherPhaseStatus == YELLOW)
					followPhaseCountDown = max(followPhaseCountDown, motherPhaseCountDown);
				break;
			case RED:
				if (motherPhaseStatus == RED)
				{
					if (followPhaseCountDown == 0)
						followPhaseCountDown = motherPhaseCountDown;
					else
						followPhaseCountDown = min(followPhaseCountDown, motherPhaseCountDown);
				}
				break;
			default: break;
		}
	}
	return followPhaseCountDown;
}
static void SetFollowPhaseStatus(UInt8 *nextPhaseIds, FollowPhaseItem *followPhaseTable, PhaseItem *phaseTable)
{
	int i, j;
	PhaseChannelStatus status, newStatus, motherPhaseStatus;
	UInt8 motherPhase;
	UInt8 uIsPhaseRunning = 0;//判断相位是否在运行
	
	memset(gCountDownParams->ucOverlap, 0, sizeof(gCountDownParams->ucOverlap));
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (followPhaseTable[i].nArrayMotherPhase[0] == 0)
		{
			gCountDownParams->ucOverlap[i][0] = (UInt8)TURN_OFF;
			continue;
		}
			
		status = RED;	
		for (j = 0; j < NUM_PHASE; j++)
		{
			motherPhase = followPhaseTable[i].nArrayMotherPhase[j];
			if (motherPhase == 0) 
				break;
			motherPhaseStatus = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][0];
			uIsPhaseRunning = gCountDownParams->stPhaseRunningInfo[motherPhase - 1][1];//判断相位是否运行的条件是该相位运行时间是否为0
			if (uIsPhaseRunning != 0)
			{	//当母相位正在运行时，如果即将运行下一相位也在跟随相位表中，那么此时跟随相位状态应是常绿GREEN，反之它的状态和母相位状态一致
				newStatus = IsNextPhaseInFollowPhaseList(&followPhaseTable[i], nextPhaseIds[motherPhase - 1]) ? GREEN : motherPhaseStatus;
				if (status == RED)
				{
					status = newStatus;
					continue;
				}
				//用以处理多环时母相位同时运行的情况
				if ((newStatus == GREEN) 
					|| (newStatus == GREEN_BLINK && (status == YELLOW || status == ALLRED))
					|| (newStatus == YELLOW && status == ALLRED))
					status = newStatus;
			}
		}
		gCountDownParams->ucOverlap[i][0] = (UInt8)status;
		gCountDownParams->ucOverlap[i][1] = CalFollowPhaseCountDown(nextPhaseIds, &followPhaseTable[i], phaseTable, status);
	}				
}

static inline void FindNextPhaseIdInPhaseTrun(PhaseTurnItem *phaseTrunTable, UInt8 *nextPhaseIds)
{
	int i = 0, ring = 0;
	UInt8 nPhaseId = 0, nextPhaseId = 0;
	
	memset(nextPhaseIds, 0, NUM_PHASE);
	for (ring = 0; ring < NUM_RING_COUNT; ring++) 
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = phaseTrunTable[ring].nTurnArray[i];
			nextPhaseId = phaseTrunTable[ring].nTurnArray[((i + 1) == NUM_PHASE) ? 0 : (i + 1)];
			if (nPhaseId == 0) 
			{
				break;
			}
			nextPhaseIds[nPhaseId - 1] = (nextPhaseId != 0) ? nextPhaseId : phaseTrunTable[ring].nTurnArray[0];
		}
	}
}

/*****************************************************************************
 函 数 名  : UpdateCountDownTime
 功能描述  : 保证感应发生时，所有相位的倒计时都能保持相同增量的变化
 输入参数  : int iDiffValue                                
             unsigned char *pPhaseArray                    
             int *pOldPhaseCountDownVaule                  
             PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void UpdateCountDownTime(int iDiffValue,unsigned char *pPhaseArray,int *pOldPhaseCountDownVaule,PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    int i = 0;
    unsigned char cPhaseId = 0;
    
    if((iDiffValue < 0) || (pPhaseArray == NULL) || (pCountDownParams == NULL) || (pOldPhaseCountDownVaule == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        cPhaseId = pPhaseArray[i];
        if(cPhaseId == 0)
        {
            break;
        }
        //如果该环内，有相位的倒计时跳变值不等于绿灯相位跳变的值，则手动更改其倒计时时间

        if(pOldPhaseCountDownVaule[cPhaseId - 1] == 0)//表明是第一次调用该函数，上一次的倒计时数据还没有保存下来，就直接保存
        {
            pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
            continue;
        }
        
        if(((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - pOldPhaseCountDownVaule[cPhaseId - 1]) != iDiffValue) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))
        {
            //fprintf(stderr,"Changed %d , old  %d Diff %d \n\n",cPhaseId,pOldPhaseCountDownVaule[cPhaseId - 1],iDiffValue);
            pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] = pOldPhaseCountDownVaule[cPhaseId - 1] + iDiffValue;

           // pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
        }
    }

}

/*****************************************************************************
 函 数 名  : StoreOldCountDownData
 功能描述  : 保存上一次的倒计时信息
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             int *nArray                                   
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月6日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void StoreOldCountDownData(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,int *nArray)
{
    int i = 0;

    if((nArray == NULL) || (pCountDownParams == NULL))
    {

    }

    for(i = 0 ; i < 16; i++)
    {

        nArray[i] = pCountDownParams->stVehPhaseCountingDown[i][1];
    }


}

/*****************************************************************************
 函 数 名  : CalcGreenSplit
 功能描述  : 计算相位的绿信比
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             unsigned char cCircleNo                       
             PhaseTurnItem *pPhaseTurn                     
             unsigned char uIndex                          
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcGreenSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,unsigned char cCircleNo,PhaseTurnItem *pPhaseTurn,unsigned char uIndex)
{
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;

    for(k = 0 ; k < uIndex - 2; k++)//向后遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位

        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            break;
        }

        pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
                                                                ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
        
    }

    for(k = uIndex; k < NUM_PHASE - 1; k++)//向前遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位
        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            if((uIndex >= 2) && (cPhaseId != 0))//
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] == 0) ?  pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] : pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1]) - 
                                                                        ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
            }
            
            break;
        }

        pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
                                                                ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]); 
    }
}

/*****************************************************************************
 函 数 名  : CalcCircleTime
 功能描述  : 计算运行周期长
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void CalcCircleTime(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn)
{
    int i = 0;
    int j = 0;
    
    int iTempVal = 0;
    
    for(i = 0 ; i < 4; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//如果某个环没有配置，则不继续计算
        {
            continue;
        } 
        iTempVal = 0;
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(0 == pPhaseTurn[i].nTurnArray[j])
            {
                break;
            }
            iTempVal += pCountDownParams->stPhaseRunningInfo[pPhaseTurn[i].nTurnArray[j] - 1][0];
        }

        pCountDownParams->ucCurCycleTime = iTempVal;
        break;
    }
}

/*****************************************************************************
 函 数 名  : AlterPhaseGreenLightTime
 功能描述  : 根据相位绿闪时间，将特定时间段的绿灯状态改为绿闪
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             unsigned char *pPhaseGreenBlink               
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void AlterPhaseGreenLightTime(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,unsigned char *pPhaseGreenBlink)
{
    int i = 0;

    for(i = 0 ; i < 16 ; i++)
    {
        if((pCountDownParams->stVehPhaseCountingDown[i][0] == GREEN) && (pCountDownParams->stVehPhaseCountingDown[i][1] <= pPhaseGreenBlink[i]))
        {
            pCountDownParams->stVehPhaseCountingDown[i][0] = GREEN_BLINK;
           // fprintf(stderr,"===>  Phase  Green Blink %d\n",i+1);
        }
    }
}


/*****************************************************************************
 函 数 名  : CalcPhaseRunTimeAndSplit
 功能描述  : 在倒计时中计算各相位的绿信比时间、运行时间及整个周期时间、运行时间
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  倒计时结构体指针
             PhaseTurnItem *pPhaseTurn                     当前运行的相序表         二维数组[4][16]
             unsigned char *pPhaseGreenLight                相位绿闪时间指针        长度是16
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月4日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年3月9日
    作    者   : 肖文虎
    修改内容   : 暂不考虑行人感应的情况
*****************************************************************************/
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn, UInt8 *pPhaseGreenBlink)
{
    int i = 0;
    int j = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iDiffVal = 0;//用来记录感应时当前相位的绿灯增加值
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//记录当前绿灯相位号，当该环内，绿灯相位发生变动时，将上个相位的相位运行时间清零
    static int nArrayCurrentPhaseCountDown[16] = {0};//本地记录一份各个相位的倒计时信息，用来感应时，保证各个相位的倒计时能同时增加
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

   // fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                      //  pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);

    //轮询相序表，计算单环中各个相位的绿信比
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 >= pPhaseTurn[i].nTurnArray[0])//如果某个环没有配置，则不继续计算
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//轮询该环内的相位
        {
            cPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 >= cPhaseId)//如果该环内已找到相位ID为0的相位，则表明已轮询结束，直接break掉，不再计算该环。
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//如果当前相位运行时间不为0，则表明继续运行
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0]))//找到灯色为绿色的相位
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//记录当前绿灯相位
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//如果运行相位发生变动，则清空上个相位的运行时间
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//更新当前绿灯相位
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//如果运行时间是0，则将运行时间改为1
                }

                index = j+1;//记录绿灯相位的顺序，以此为起点，依次做差值，可得除首末相位外的绿信比

                //在感应时，要保证一个环内所有相位的倒计时时间都要同步增加相同值
                if(nArrayCurrentPhaseCountDown[cPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];//刚开始时记录
                }
                iDiffVal = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - nArrayCurrentPhaseCountDown[cPhaseId - 1];


                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[cPhaseId - 1] != 1 ) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))//只有在相位未发生改变且倒计时跳变时才调用。
                {
                    //fprintf(stderr,"===>phase %d  countdown   %d\n",cPhaseId,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //计算当前相位的绿信比时间,等于下一个相位的倒计时时间加上当前相位的运行时间
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//如果该相位的下一个相位不存在，则下一个相位为相序表起始值
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;

                //用做差值的方式计算除最后一个相位以外的绿信比
                CalcGreenSplit(pCountDownParams,i,pPhaseTurn,index);
            }
        }
    }
    
    //运行时间大于或等于周期后，需要清零，重新开始下一个周期
    if((pCountDownParams->ucCurRunningTime >= pCountDownParams->ucCurCycleTime) && (pCountDownParams->ucCurCycleTime != 0))
    {
        pCountDownParams->ucCurRunningTime = 0;
    }
    //计算运行时间
    pCountDownParams->ucCurRunningTime++;

    //计算运行周期
    CalcCircleTime(pCountDownParams,pPhaseTurn);

    //store old data
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown);

    //修改绿闪状态
    AlterPhaseGreenLightTime(pCountDownParams,pPhaseGreenBlink);
}

#define GET_COLOR(val)    (((val) == 1) ? "绿" : ((val == 2) ? "红" : ((val == 3) ? "黄" : "")))

void PrintVehCountDown()
{
    fprintf(stderr,"PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n",
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[0][0]),gCountDownParams->stVehPhaseCountingDown[0][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[1][0]),gCountDownParams->stVehPhaseCountingDown[1][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[2][0]),gCountDownParams->stVehPhaseCountingDown[2][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[3][0]),gCountDownParams->stVehPhaseCountingDown[3][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[4][0]),gCountDownParams->stVehPhaseCountingDown[4][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[5][0]),gCountDownParams->stVehPhaseCountingDown[5][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[6][0]),gCountDownParams->stVehPhaseCountingDown[6][1],
                                                        GET_COLOR(gCountDownParams->stVehPhaseCountingDown[7][0]),gCountDownParams->stVehPhaseCountingDown[7][1]);
}

void PrintOverLapCountDown()
{
    fprintf(stderr,"PrintOverLapCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n",
                                                        GET_COLOR(gCountDownParams->ucOverlap[0][0]),gCountDownParams->ucOverlap[0][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[1][0]),gCountDownParams->ucOverlap[1][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[2][0]),gCountDownParams->ucOverlap[2][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[3][0]),gCountDownParams->ucOverlap[3][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[4][0]),gCountDownParams->ucOverlap[4][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[5][0]),gCountDownParams->ucOverlap[5][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[6][0]),gCountDownParams->ucOverlap[6][1],
                                                        GET_COLOR(gCountDownParams->ucOverlap[7][0]),gCountDownParams->ucOverlap[7][1]);
}



void PrintPedCountDown()
{
    fprintf(stderr,"PrintPedCountDown>  %d--%d--%d--%d   %d--%d--%d--%d\n\n",
                                                        gCountDownParams->stPedPhaseCountingDown[0][1],
                                                        gCountDownParams->stPedPhaseCountingDown[1][1],
                                                        gCountDownParams->stPedPhaseCountingDown[2][1],
                                                        gCountDownParams->stPedPhaseCountingDown[3][1],
                                                        gCountDownParams->stPedPhaseCountingDown[4][1],
                                                        gCountDownParams->stPedPhaseCountingDown[5][1],
                                                        gCountDownParams->stPedPhaseCountingDown[6][1],
                                                        gCountDownParams->stPedPhaseCountingDown[7][1]);
}


void PrintRunInfo()
{
    fprintf(stderr,"PrintRunInfo>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d\n",
                                                        gCountDownParams->stPhaseRunningInfo[0][1],gCountDownParams->stPhaseRunningInfo[0][0],
                                                        gCountDownParams->stPhaseRunningInfo[1][1],gCountDownParams->stPhaseRunningInfo[1][0],
                                                        gCountDownParams->stPhaseRunningInfo[2][1],gCountDownParams->stPhaseRunningInfo[2][0],
                                                        gCountDownParams->stPhaseRunningInfo[3][1],gCountDownParams->stPhaseRunningInfo[3][0],
                                                        gCountDownParams->stPhaseRunningInfo[4][1],gCountDownParams->stPhaseRunningInfo[4][0],
                                                        gCountDownParams->stPhaseRunningInfo[5][1],gCountDownParams->stPhaseRunningInfo[5][0],
                                                        gCountDownParams->stPhaseRunningInfo[6][1],gCountDownParams->stPhaseRunningInfo[6][0],
                                                        gCountDownParams->stPhaseRunningInfo[7][1],gCountDownParams->stPhaseRunningInfo[7][0],
                                                        
                                                        gCountDownParams->ucCurRunningTime,
                                                        gCountDownParams->ucCurCycleTime);

}


void CollectCountDownParams()
{
	static UInt8 outPutCount = 0, outPutTotal = 0;	//倒计时接口调用计数和总共调用次数
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//存放每个相位运行之后即将运行的下一相位号
	static FollowPhaseItem followPhaseTable[NUM_FOLLOW_PHASE];	//跟随相位的配置
	static UInt8 greenBlinkTimes[NUM_PHASE];	//相位绿闪时间
	PhaseItem phaseTable[NUM_PHASE];
	GreenSignalRationItem splitTable[NUM_PHASE];
	UInt8 nSchemeID = 0;
	
	if (gCountDownParams->ucCurRunningTime == gCountDownParams->ucCurCycleTime)
	{
		pthread_rwlock_rdlock(&gConfigLock);
		outPutTotal = CalCountDownCalledCount();
		nSchemeID = GetSchemeId(splitTable, phaseTable, gPhaseTrunTable, followPhaseTable, greenBlinkTimes);
		pthread_rwlock_unlock(&gConfigLock);
		if (nSchemeID == 0)
			return;
		
		pthread_rwlock_wrlock(&gCountDownLock);
		SetCountDownParams(nSchemeID, splitTable, phaseTable);
		pthread_rwlock_unlock(&gCountDownLock);
		//先找出每个相位在相序表中的下一相位号,为后续计算跟随相位状态做准备
		FindNextPhaseIdInPhaseTrun(gPhaseTrunTable, nextPhaseIds);
	}
	
	if (++outPutCount == outPutTotal)
	{
		outPutCount = 0;
		pthread_rwlock_wrlock(&gCountDownLock);

		CalcPhaseRunTimeAndSplit(gCountDownParams, gPhaseTrunTable, greenBlinkTimes);
		SetFollowPhaseStatus(nextPhaseIds, followPhaseTable, phaseTable);
		
		PrintVehCountDown();
		PrintOverLapCountDown();
        PrintRunInfo();
		memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
		pthread_rwlock_unlock(&gCountDownLock);
	}
}
