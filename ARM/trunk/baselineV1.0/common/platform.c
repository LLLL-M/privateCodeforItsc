#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "HikConfig.h"
#include "platform.h"
#include "common.h"
#include "specialControl.h"
#include "countDown.h"
#include "configureManagement.h"

#define INDUCTIVE_CONTROL_ACTION_ID		118
#define INDUCTIVE_CONTROL_SCHEME_ID		254

UInt8 outPutCount = 0; 	//倒计时接口调用计数
pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;	//主要保护多线程访问全局配置
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //倒计时接口信息，用来在倒计时接口中计算
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //用来供udpserver调用的倒计时缓存

MsgRealtimePattern gStructMsgRealtimePatternInfo;           //在倒计时接口中，每个周期开始时进行更新
MsgPhaseSchemeId gStructMsg1049;                            //供平台1049协议使用
pthread_rwlock_t gLockRealtimeVol = PTHREAD_RWLOCK_INITIALIZER;//保护实时流量的读写锁
MsgRealtimeVolume gStructMsgRealtimeVolume;                 //实时流量，只有流量是实时的

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述

PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //当前使用的相序

CountdownConfigFlag gCountdownConfigFlag = COUNTDOWN_CONFIG_HAVE_NEW;
static SignalControllerPara *gConfigPara2 = NULL;
extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;


extern CountDownVeh countdown_veh[17];                 //机动车相位倒计时参数
extern CountDownPed countdown_ped[17];				   //行人相位倒计时参数
extern unsigned char g_cCurrentActionId;//当前运行动作号,系统控制时，可能会进行感应、黄闪、全红.

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
		phaseId = gConfigPara2->stChannel[i].nControllerID;
		if (phaseId == 0)
			continue;
		type = gConfigPara2->stChannel[i].nControllerType;
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
	//INFO("set count down params, special scheme %d\n", gStructBinfileCustom.cSpecialControlSchemeId);
	gCountDownParams->ucPlanNo = nSchemeID;
	//memset(gCountDownParams->stPhaseRunningInfo, 0, sizeof(gCountDownParams->stPhaseRunningInfo));
	if (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)	//说明是感应控制
	{
		for(i = 0; i < NUM_PHASE; i++)
		{
			if (IS_PHASE_INABLE(phaseTable[i].wPhaseOptions) == 0)
				continue;
			gCountDownParams->stPhaseRunningInfo[i][0] = phaseTable[i].nMinGreen + phaseTable[i].nYellowTime + phaseTable[i].nAllRedTime;
		}
	}
	else
	{
		for(i = 0 ; i < NUM_PHASE; i++)
		{
			if (IS_PHASE_INABLE(phaseTable[i].wPhaseOptions) == 0)
				continue;
			gCountDownParams->stPhaseRunningInfo[i][0] = splitTable[i].nGreenSignalRationTime;
		}
	}
	gCountDownParams->ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;
	if (nSchemeID > 0 && nSchemeID <= 48)//modified by xiaowh 
		memcpy(gCountDownParams->ucCurPlanDsc, gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[(nSchemeID - 1) / 3], sizeof(gCountDownParams->ucCurPlanDsc));
	for(i = 0; i < NUM_PHASE; i++)
	{
		gCountDownParams->stVehPhaseCountingDown[i][0] = countdown_veh[i + 1].veh_color;
		gCountDownParams->stVehPhaseCountingDown[i][1] = countdown_veh[i + 1].veh_phaseTime;
		gCountDownParams->stPedPhaseCountingDown[i][0] = countdown_ped[i + 1].ped_color;
		gCountDownParams->stPedPhaseCountingDown[i][1] = countdown_ped[i + 1].ped_phaseTime;
	}
}

//每周期填充实时数据到结构体中
static void SetRealtimeRunInfo()
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    //计算当前绿信比中的协调相位，此处不必校验绿信比或相序是否合法，上层调用已确保其合法性
    for(i = 0; i < 16; i++)
    {
        if(gConfigPara2->stGreenSignalRation[gStructMsgRealtimePatternInfo.nSplitId - 1][i].nIsCoordinate == 1)
        {
            gStructMsgRealtimePatternInfo.nCoordinatePhase |= (1<<i);//如果相位作为协调相位，则其bit位设置为1
        }
    }

    //设置各个相位在转换序列中的时间，比如全红、黄闪及绿闪时间。
    for(i = 0; i < 16; i++)
    {
        gStructMsgRealtimePatternInfo.phaseTime[i].yellow = gConfigPara2->stPhase[i].nYellowTime;
        gStructMsgRealtimePatternInfo.phaseTime[i].allred = gConfigPara2->stPhase[i].nAllRedTime;
        gStructMsgRealtimePatternInfo.phaseTime[i].greenBlink = gConfigPara2->AscSignalTransTable[i].nGreenLightTime;
    }

    //设置跟随相位信息
    memcpy(gStructMsgRealtimePatternInfo.stFollowPhase,gConfigPara2->stFollowPhase,sizeof(gConfigPara2->stFollowPhase));


    //信号机配置的所有相位的相位号
    for(i = 0,k = 0; i < 4; i++)
    {
        for(j = 0; j < 16; j++)
        {
            if(gStructMsgRealtimePatternInfo.sPhaseTurn[i].nTurnArray[j] != 0)
            {
                gStructMsg1049.nPhaseArray[k++] = gStructMsgRealtimePatternInfo.sPhaseTurn[i].nTurnArray[j] ;
                
            }
            else
            {
                break;
            }
        }
    }

    //信号机配置的所有方案号，不做转换
    for(i = 0,j = 0; i < 108; i++)
    {
        if(gConfigPara2->stScheme[i].nCycleTime != 0)
        {
            gStructMsg1049.nPatternArray[j] = gConfigPara2->stScheme[i].nSchemeID;
            j++;
        }
    }
}

static UInt8 GetSchemeId(GreenSignalRationItem *splitTable, PhaseTurnItem *phaseTrunTable, UInt8 *greenBlinkTimes)
{
	UInt8 nActionID = 0;
	UInt8 nSchemeID = 0;
	UInt8 nGreenSignalRatioID = 0;
	UInt8 nPhaseTurnID = 0;
	int i;
    
    memset(&gStructMsgRealtimePatternInfo,0,sizeof(gStructMsgRealtimePatternInfo));//每个周期都将参数清空重新计算
    
	if (gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_SYSTEM || gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_STEP)	//系统控制
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
			nSchemeID = gConfigPara2->stAction[nActionID - 1].nSchemeID;
	}
	else if (gStructBinfileCustom.cSpecialControlSchemeId == INDUCTIVE_CONTROL_SCHEME_ID)
		nSchemeID = INDUCTIVE_CONTROL_SCHEME_ID;
	else
		nSchemeID = gStructBinfileCustom.cSpecialControlSchemeId;

		
	if (nSchemeID == 0)
		DBG("action %d has not config scheme!", nActionID);
	else
	{	//感应控制默认使用绿信比表1和相序表1
		nGreenSignalRatioID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gConfigPara2->stScheme[nSchemeID - 1].nGreenSignalRatioID;
		nPhaseTurnID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gConfigPara2->stScheme[nSchemeID - 1].nPhaseTurnID;

		gStructMsgRealtimePatternInfo.nSplitId = nGreenSignalRatioID;
		gStructMsgRealtimePatternInfo.nPhaseTurnId = nPhaseTurnID;
		gStructMsgRealtimePatternInfo.nPatternId = nSchemeID;
		if(nSchemeID <= 108)
    		gStructMsgRealtimePatternInfo.nOffset = gConfigPara2->stScheme[nSchemeID - 1].nOffset;//填充必备运行信息
		
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
			memcpy(splitTable, gConfigPara2->stGreenSignalRation[nGreenSignalRatioID - 1], sizeof(GreenSignalRationItem) * NUM_PHASE);
			memcpy(phaseTrunTable, gConfigPara2->stPhaseTurn[nPhaseTurnID - 1], sizeof(PhaseTurnItem) * NUM_RING_COUNT);

            memcpy(gStructMsgRealtimePatternInfo.sPhaseTurn,phaseTrunTable,sizeof(PhaseTurnItem) * NUM_RING_COUNT);//拷贝相序表
			//确保绿信比和相序表合法有效的情况下，填充协调相位等基本数据
			SetRealtimeRunInfo();
			for (i = 0; i < NUM_PHASE; i++)
				greenBlinkTimes[i] = gConfigPara2->AscSignalTransTable[i].nGreenLightTime;
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
Boolean IsNextPhaseInFollowPhaseList(FollowPhaseItem *followPhaseItem, UInt8 nextPhaseId)
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
							if (motherPhaseStatus == GREEN || motherPhaseStatus == GREEN_BLINK)
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
			case GREEN_BLINK:
				//INFO("-->Mother phase:%d, status: %d, countdown:%d", motherPhase, motherPhaseStatus, motherPhaseCountDown);
				if (motherPhaseStatus == GREEN_BLINK)
					followPhaseCountDown = max(followPhaseCountDown, motherPhaseCountDown);

				//INFO("-->follow countdown:%d", followPhaseCountDown);
				//INFO("-->chan14: id-%d, ctrlId:%d, ctrlType:%d, flashType:%d", gSignalControlpara->stChannel[13].nChannelID, gSignalControlpara->stChannel[13].nControllerID, gSignalControlpara->stChannel[13].nControllerType,gSignalControlpara->stChannel[13].nFlashLightType);
				break;
			default: break;
		}
	}
	return followPhaseCountDown;
}
UINT8 gFollowPhaseGreenBlinkFlag[NUM_PHASE]={0};
UINT8 gFollowPhaseMotherPhase[NUM_PHASE]={0};//range: 1-16
static void SetFollowPhaseStatus(UInt8 *nextPhaseIds, FollowPhaseItem *followPhaseTable, PhaseItem *phaseTable)
{
	int i, j;
	PhaseChannelStatus status, newStatus, motherPhaseStatus;
	UInt8 motherPhase;
	UInt8 uIsPhaseRunning = 0;//判断相位是否在运行
	
	memset(gCountDownParams->ucOverlap, 0, sizeof(gCountDownParams->ucOverlap));
	memset(gFollowPhaseGreenBlinkFlag, 0, sizeof(gFollowPhaseGreenBlinkFlag));
	memset(gFollowPhaseMotherPhase, 0, sizeof(gFollowPhaseMotherPhase));
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
				if(IsNextPhaseInFollowPhaseList(&followPhaseTable[i], nextPhaseIds[motherPhase - 1]))
				{
					newStatus = GREEN;
				}
				else
				{
					newStatus = motherPhaseStatus;
					gFollowPhaseMotherPhase[i] = motherPhase;
				}
				
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
//		if (status == GREEN_BLINK)
//			status = GREEN; //跟随相位没有绿闪状态
		gCountDownParams->ucOverlap[i][0] = status;	
		gCountDownParams->ucOverlap[i][1] = CalFollowPhaseCountDown(nextPhaseIds, &followPhaseTable[i], phaseTable, status);

		if(status == GREEN_BLINK)
		{
			gFollowPhaseGreenBlinkFlag[i]=1;
		}
	}				
}

void FindNextPhaseIdInPhaseTrun(PhaseTurnItem *phaseTrunTable, UInt8 *nextPhaseIds)
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
static void StoreOldCountDownData(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,int *nArray,int *nArrayColor)
{
    int i = 0;

    if((nArray == NULL) || (pCountDownParams == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        //如果机动车倒计时为0的话，就保存行人的倒计时，
        nArray[i] = ((pCountDownParams->stVehPhaseCountingDown[i][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[i][1] : pCountDownParams->stVehPhaseCountingDown[i][1]);
        nArrayColor[i] = pCountDownParams->stVehPhaseCountingDown[i][0];
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
    int tmp = 0;
    
    for(k = 0 ; k < uIndex - 2; k++)//向后遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位

        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            break;
        }

        tmp = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
                                                                ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
        if(tmp >= 0)
        {
            pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = tmp;
        }
        //INFO("back  %d\n",cPhaseId);
    }

    for(k = uIndex; k < NUM_PHASE - 1; k++)//向前遍历
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//得到当前的相位号
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//得到该相序中下一个相位
        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            if((uIndex >= 2) && (cPhaseId != 0))//
            {
               // INFO("front  %d\n",cPhaseId);
                tmp = ((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] == 0) ?  pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] : pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1]) - 
                                                                        ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
                if(tmp >= 0)
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = tmp;
                }
            }
            
            break;
        }

        tmp = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
                                                                ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]); 
        if(tmp >= 0)
        {
            pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = tmp;
        }
       // INFO("front  %d\n",cPhaseId);
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
          // INFO("---->  phase id  %d\n",i+1);
        }
    }
}
#if 0
/*****************************************************************************
 函 数 名  : IsPhaseJumpChange
 功能描述  : 判断相位倒计时是否发生突变，一旦突变，则清空倒计时信息，重新计
             算
 输入参数  : UInt8 nSchemeID                                        
             int *time                                              
             int *color                                             
             PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static UInt8 IsPhaseJumpChange(UInt8 nSchemeID,int *time,int *color,PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    if(nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)//如果当前是感应控制，则不做处理
    {
        return 0;
    }

    //INFO("%d-%d\n",time[0],color[0]);
    int i = 0;
    for(i = 0 ; i < 16; i++)
    {
        if((time[i] == 0 )|| (pCountDownParams->stVehPhaseCountingDown[i][1] == 0))//如果倒计时为0，表明该相位没有倒计时信息，不做处理
        {
            continue;
        }
        //如果前后灯色相同，但倒计时值不是比上个小1，就认为发生了跳变
        if(color[i] == pCountDownParams->stVehPhaseCountingDown[i][0])
        {
            //INFO("%d  %d\n",time[i],pCountDownParams->stVehPhaseCountingDown[i][1]);
            if((time[i] -  pCountDownParams->stVehPhaseCountingDown[i][1]) != 1)
            {
                INFO("IsPhaseJumpChange = 1\n");
                return 1;
            }
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : ClearPhaseCountDownInfo
 功能描述  : 清空倒计时信息
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月10日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void ClearPhaseCountDownInfo(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    int i = 0;

    for(i = 0 ; i < 16; i++)
    {
        pCountDownParams->stPhaseRunningInfo[i][1] = 0;//相位运行时间清空;
    }

    pCountDownParams->ucCurCycleTime = 0;
    pCountDownParams->ucCurRunningTime = 0;//当前周期长及当前运行时间清零

    INFO("ClearPhaseCountDownInfo\n");
}
#endif

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
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn, UInt8 *pPhaseGreenBlink,UInt8 nSchemeID)
{
    int i = 0;
    int j = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iDiffVal = 0;//用来记录感应时当前相位的绿灯增加值
    int tmp = 0;
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//记录当前绿灯相位号，当该环内，绿灯相位发生变动时，将上个相位的相位运行时间清零
    static int nArrayCurrentPhaseCountDown[16] = {0};//本地记录一份各个相位的倒计时信息，用来感应时，保证各个相位的倒计时能同时增加
    static int nArrayCurrentPhaseColor[16] = {0};//本地记录一份各个相位的灯色
    
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

    //如果发生相位突变，则清空相位倒计时信息
    //if(IsPhaseJumpChange(nSchemeID,nArrayCurrentPhaseCountDown,nArrayCurrentPhaseColor, pCountDownParams) == 1)
   // {
    //    ClearPhaseCountDownInfo(pCountDownParams);
    //}
    

    //fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                       // pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);
   // INFO("===>   %d\n",pCountDownParams->ucCurCycleTime);

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
            //INFO("  cPhase Id   %d ,  veh  %d , ped  %d, running val  %d\n",cPhaseId,
            //                            pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0],
            //                            pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0],
             //                           pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]);
            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//如果当前相位运行时间不为0，则表明继续运行
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;//就把当前相位的运行时间加1
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0])/* || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0])*/)//找到灯色为绿色的相位
            {
                if(nArrayCurrentPhaseId[i] == -1)//首次运行时，保存上次的绿灯相位
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//记录当前绿灯相位
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//如果运行相位发生变动，则清空上个相位的运行时间
                {
                   // OFTEN("phase changed , from %d to %d \n",nArrayCurrentPhaseId[i],cPhaseId);
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//更新当前绿灯相位

                    if(j == 0)//如果相位重新运行到了第一个相位，则将周期运行时间清零。
                    {
                        pCountDownParams->ucCurRunningTime = 0;
                    }
                }
                else//如果相位没有发生变动，但本次的倒计时值不是比上次倒计时值小1，也要重新计算，将本次运行时间清零，同时将周期运行时间也清零。
                {
                    tmp = ((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) ? pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1]);
                    tmp = tmp - nArrayCurrentPhaseCountDown[cPhaseId - 1];
                   // OFTEN("CalcPhaseRunTimeAndSplit tmp %d, nArrayCurrentPhaseCountDown[i]  %d\n",tmp,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    if((tmp != -1) && (pCountDownParams->ucPlanNo != 254))//清空运行时间需要排除感应的情况.
                    {
                        pCountDownParams->ucCurRunningTime = 0;
                        pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 0;

                    }
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//如果运行时间是0，则将运行时间改为1
                   // INFO("phase %d , change running time to  1 \n",cPhaseId);
                }

                index = j+1;//记录绿灯相位的顺序，以此为起点，依次做差值，可得除首末相位外的绿信比

                //在感应时，要保证一个环内所有相位的倒计时时间都要同步增加相同值
                if(pCountDownParams->ucPlanNo == 254)
                {
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
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //计算当前相位的绿信比时间,等于下一个相位的倒计时时间加上当前相位的运行时间
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//如果该相位的下一个相位不存在，则下一个相位为相序表起始值
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;
               // INFO("---->    green phase  %d  runnin time  %d\n",cPhaseId,pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]);
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
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown,nArrayCurrentPhaseColor);

    //修改绿闪状态
    AlterPhaseGreenLightTime(pCountDownParams,pPhaseGreenBlink);
}


void PrintVehCountDown()
{
    OFTEN("PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n",
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
    OFTEN("PrintOverLapCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n",
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
    OFTEN("PrintPedCountDown>  %d--%d--%d--%d   %d--%d--%d--%d\n\n",
                                                        gCountDownParams->stPedPhaseCountingDown[0][1],
                                                        gCountDownParams->stPedPhaseCountingDown[1][1],
                                                        gCountDownParams->stPedPhaseCountingDown[2][1],
                                                        gCountDownParams->stPedPhaseCountingDown[3][1],
                                                        gCountDownParams->stPedPhaseCountingDown[4][1],
                                                        gCountDownParams->stPedPhaseCountingDown[5][1],
                                                        gCountDownParams->stPedPhaseCountingDown[6][1],
                                                        gCountDownParams->stPedPhaseCountingDown[7][1]);
}



PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gTest ;       //用来测试跳秒现象

int UnitTest_PhaseRunTime()
{
    int i = 0;
    
    //相位的运行时间在正常情况下是要一直增加的
    for(i = 0; i < 16; i++)
    {
        if(gCountDownParams->stPhaseRunningInfo[i][1] != 0)//运行时间不为0，表明该相位在运行
        {
            if((gCountDownParams->stPhaseRunningInfo[i][1] - gTest.stPhaseRunningInfo[i][1]) != 1)
            {
               // log_error("UnitTest_PhaseRunTime  ERROR   1\n");
                return 1;//运行时间发生跳变!!!!!
            }
        }

        if(gCountDownParams->ucCurRunningTime != 1)//运行时间不为0，表明该相位在运行
        {
            if((gCountDownParams->ucCurRunningTime - gTest.ucCurRunningTime) != 1)
            {
                //log_error("UnitTest_PhaseRunTime  ERROR   2\n");
                return 2;//运行时间发生跳变!!!!!
            }
        }

    }

    return 0;
}


void PrintRunInfo()
{
    OFTEN(">  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d\n\n",
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
#if 0
    static int flag = 0;
    if(flag == 1)
    {
        if(UnitTest_PhaseRunTime() != 0)
        {
            ERR("################   Error !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }

        memcpy(&gTest,gCountDownParams,sizeof(gTest));
    }


    if(flag == 0)
    {
        flag = 1;
        //InitLogSystem("/root/log4HIK/",40,5);
        memcpy(&gTest,gCountDownParams,sizeof(gTest));
    }

#endif
}

static inline void UpdateCountDownConfig(void)
{
	if (gCountdownConfigFlag == COUNTDOWN_CONFIG_HAVE_NEW)
	{
		pthread_rwlock_rdlock(&gConfigLock);
		memmove(gConfigPara2, gSignalControlpara, sizeof(SignalControllerPara));
		pthread_rwlock_unlock(&gConfigLock);
		gCountdownConfigFlag = COUNTDOWN_CONFIG_NO_NEW;
		INFO("########### count down config update ###########");
	} 
	else if (gCountdownConfigFlag == COUNTDOWN_CONFIG_UPDATE)
	{
		gCountdownConfigFlag = COUNTDOWN_CONFIG_HAVE_NEW;
	}		
}

void CollectCountDownParams()
{
	static UInt8 outPutTotal = 0;	//倒计时接口总共调用次数
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//存放每个相位运行之后即将运行的下一相位号
	static UInt8 greenBlinkTimes[NUM_PHASE];	//相位绿闪时间
	PhaseItem *phaseTable;	//相位表
	FollowPhaseItem *followPhaseTable;	//跟随相位表
	GreenSignalRationItem splitTable[NUM_PHASE];
	static UInt8 nSchemeID = 0;
    
 //   log_debug("CollectCountDownParams  outPutCount: %d,ucCurCycleTime %d,ucCurRunningTime %d\n",outPutCount,
 //                   gCountDownParams->ucCurCycleTime,
 //                   gCountDownParams->ucCurRunningTime);
	if (gConfigPara2 == NULL)
	{
		gConfigPara2 = gSignalControlpara + 2;
	}
	phaseTable = gConfigPara2->stPhase;
	followPhaseTable = gConfigPara2->stFollowPhase;
	
	//((gStructBinfileCustom.cSpecialControlSchemeId >= 1)&&(gStructBinfileCustom.cSpecialControlSchemeId <= 248))
	if (gCountDownParams->ucCurCycleTime == gCountDownParams->ucCurRunningTime && outPutCount == 0)
	{
		UpdateCountDownConfig();
		outPutTotal = CalCountDownCalledCount();
		
		pthread_rwlock_wrlock(&gCountDownLock);
		nSchemeID = GetSchemeId(splitTable, gPhaseTrunTable, greenBlinkTimes);
	//	log_debug("CollectCountDownParams  , nSchemeId is  %d\n",nSchemeID);
		if (nSchemeID == 0)
		{
		    pthread_rwlock_unlock(&gCountDownLock);
    		return;
		}
		
		SetCountDownParams(nSchemeID, splitTable, phaseTable);
		pthread_rwlock_unlock(&gCountDownLock);
		//先找出每个相位在相序表中的下一相位号,为后续计算跟随相位状态做准备
		FindNextPhaseIdInPhaseTrun(gPhaseTrunTable, nextPhaseIds);
	}
	
	if (++outPutCount == outPutTotal)
	{
		outPutCount = 0;
		pthread_rwlock_wrlock(&gCountDownLock);
        //PrintRunInfo();
		CalcPhaseRunTimeAndSplit(gCountDownParams, gPhaseTrunTable, greenBlinkTimes, nSchemeID);
		SetFollowPhaseStatus(nextPhaseIds, followPhaseTable, phaseTable);

		CountDownInterface();
//		log_debug("schemeid %d, desc: %s", gCountDownParams->ucPlanNo, gCountDownParams->ucCurPlanDsc);
		PrintVehCountDown();
		PrintPedCountDown();
		//PrintOverLapCountDown();
		PrintRunInfo();
//        INFO("CollectCountDownParams   %d %d \n",gCountDownParams->ucChannelStatus[0],gCountDownParams->ucChannelStatus[1]);
		memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
		pthread_rwlock_unlock(&gCountDownLock);
	}
}



static void SetPhaseStatus(unsigned char (*cArray)[2],unsigned char cStatus)
{
    int i = 0;

    for(i = 0; i < 16; i++)
    {
        cArray[i][0] = cStatus;
        cArray[i][1] = 0;
    }
}


/*****************************************************************************
 函 数 名  : UpdateSpecialControlDesc
 功能描述  : 在当前运行动作是特殊控制时，更新其方案描述及方案号。
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月13日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void UpdateSpecialControlDesc(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend)
{
    switch(g_cCurrentActionId)
    {
        case 115:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_TURN_OFF; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"系统控制-关灯"); break;//SPECIAL_CONTROL_TURN_OFF
        case 116:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_ALL_RED; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"系统控制-全红"); break;//SPECIAL_CONTROL_ALL_RED
        case 117:    break;
        case 118:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_INDUCTION; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"系统控制-感应"); break;//SPECIAL_CONTROL_INDUCTION
        case 119:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_YELLOW_BLINK; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"系统控制-黄闪"); break;//SPECIAL_CONTROL_YELLOW_BLINK
        default:     break;
    }
}


void UpdateCountdownParams()
{
    switch(gStructBinfileCustom.cSpecialControlSchemeId)
    {
        case SPECIAL_CONTROL_SYSTEM:         UpdateSpecialControlDesc(gCountDownParams); break;// 只有在当前是系统控制时，才更新特殊方案号。
        case SPECIAL_CONTROL_YELLOW_BLINK:  
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;        
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"黄闪");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,YELLOW_BLINK);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,YELLOW_BLINK);
                SetPhaseStatus(gCountDownParams->ucOverlap,YELLOW_BLINK);
                break;
            }
        case SPECIAL_CONTROL_INDUCTION:     gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;  strcpy((char *)gCountDownParams->ucCurPlanDsc,"感应");       break;
        case SPECIAL_CONTROL_TURN_OFF:       
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"关灯");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,TURN_OFF);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,TURN_OFF);
                SetPhaseStatus(gCountDownParams->ucOverlap,TURN_OFF);    
                break;
            }
        case SPECIAL_CONTROL_ALL_RED:        
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"全红");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,RED);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,RED);
                SetPhaseStatus(gCountDownParams->ucOverlap,RED);                
                break;
            }
        default: break;
    }
    memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
}
