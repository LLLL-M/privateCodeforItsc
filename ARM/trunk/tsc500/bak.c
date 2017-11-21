#include <string.h>
#include <pthread.h>
#include "HikConfig.h"
#include "platform.h"
#include "lfq.h"

#define INDUCTIVE_CONTROL_ACTION_ID		118
#define INDUCTIVE_CONTROL_SCHEME_ID		254

pthread_rwlock_t countDownLock = PTHREAD_RWLOCK_INITIALIZER;	//主要保护多线程访问全局配置
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //倒计时接口信息
void *gHandle = NULL;	//倒计时使用的队列句柄

extern UInt8 gChannelLockFlag;   //0表示未锁定，1表示锁定
extern UInt8 gSpecialControlSchemeId;       //特殊控制方案号

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t configLock;

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
	gHandle = calloc(1, 128);
	if (gHandle == NULL)
	{
		ERR("No memory enough to create DestAddressInfo fifo");
		exit(1);
	}
	lfq_init(gHandle, 128, sizeof(DestAddressInfo));
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
		type = gSignalControlpara->stChannel[i].nControllerType
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
}

static UInt8 GetSchemeId(GreenSignalRationItem *splitTable, PhaseItem *phaseTable, PhaseTurnItem *phaseTrunTable, FollowPhaseItem *followPhaseTable)
{
	UInt8 nActionID = 0;
	UInt8 nSchemeID = 0;
	UInt8 nGreenSignalRatioID = 0;
	UInt8 nPhaseTurnID = 0; 
	
	if (gSpecialControlSchemeId == INDUCTIVE_CONTROL_SCHEME_ID)
		nSchemeID = INDUCTIVE_CONTROL_SCHEME_ID;
	else
	{
		nActionID = GetActionID();
		if (nActionID == 0)
		{
			ERR("Current time has not config action!");
		}
		else if (nActionID == INDUCTIVE_CONTROL_ACTION_ID)
			nSchemeID = INDUCTIVE_CONTROL_SCHEME_ID;
		else
			nSchemeID = gSignalControlpara->stAction[nActionID - 1].nSchemeID;
	}
	
	if (nSchemeID == 0)
		ERR("action %d has not config scheme!", nActionID);
	else
	{	//感应控制默认使用绿信比表1和相序表1
		nGreenSignalRatioID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gSignalControlpara->stScheme[nSchemeID - 1].nGreenSignalRatioID;
		nPhaseTurnID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gSignalControlpara->stScheme[nSchemeID - 1].nPhaseTurnID;
		if(nGreenSignalRatioID == 0 || nPhaseTurnID == 0)
		{
			if (nGreenSignalRatioID == 0)
				ERR("scheme %d has not config split", nSchemeID);
			else
				ERR("scheme %d has not config phaseTurn", nSchemeID);
			nSchemeID = 0;
		}
		else
		{
			memcpy(splitTable, gSignalControlpara->stGreenSignalRation[nGreenSignalRatioID - 1], sizeof(GreenSignalRationItem) * NUM_PHASE);
			memcpy(phaseTrunTable, gSignalControlpara->stPhaseTurn[nPhaseTurnID - 1], sizeof(PhaseTurnItem) * NUM_RING_COUNT);
			memcpy(phaseTable, gSignalControlpara->stPhase, sizeof(gSignalControlpara->stPhase));
			memcpy(followPhaseTable, gSignalControlpara->stFollowPhase, sizeof(gSignalControlpara->stFollowPhase));
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

static void SetFollowPhaseStatus(UInt8 *nextPhaseIds, FollowPhaseItem *followPhaseTable)
{
	int i, j;
	PhaseStatus status, newStatus, motherPhaseStatus;
	UInt8 motherPhase;
	
	memset(gCountDownParams->ucOverlap, 0, sizeof(gCountDownParams->ucOverlap));
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (followPhaseTable[i].nArrayMotherPhase[0] == 0)
			continue;
		status = RED;	
		for (j = 0; j < NUM_PHASE; j++)
		{
			motherPhase = followPhaseTable[i].nArrayMotherPhase[j];
			if (motherPhase == 0) 
				break;
			motherPhaseStatus = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][0];
			if (motherPhaseStatus != RED)
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
		gCountDownParams->ucOverlap[i] = (UInt8)status;
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

static inline void SendFeedback()
{
	int i;
	DestAddressInfo dst;
	int len = sizeof(struct sockaddr);
	
	while (0 != lfq_element_count(gHandle))
	{
		memset(&dst, 0, sizeof(dst));
		lfq_read(gHandle, dst);
		sendto(dst.sockfd, gCountDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, &dst.addr, len);
	}
}

void UpdateCountDownTime(int iDiffValue,unsigned char *pPhaseArray,int *pOldPhaseCountDownVaule,PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    int i = 0;
    int iPhaseId = 0;
    
    if((iDiffValue < 0) || (pPhaseArray == NULL) || (pCountDownParams == NULL) || (pOldPhaseCountDownVaule == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        iPhaseId = pPhaseArray[i];
        if(iPhaseId == 0)
        {
            break;
        }
        //如果该环内，有相位的倒计时跳变值不等于绿灯相位跳变的值，则手动更改其倒计时时间

        if(pOldPhaseCountDownVaule[iPhaseId - 1] == 0)//表明是第一次调用该函数，上一次的倒计时数据还没有保存下来，就直接保存
        {
            pOldPhaseCountDownVaule[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
            continue;
        }
        
        if((pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1] - pOldPhaseCountDownVaule[iPhaseId - 1]) != iDiffValue)
        {
            fprintf(stderr,"Changed %d , old  %d Diff %d \n\n",iPhaseId,pOldPhaseCountDownVaule[iPhaseId - 1],iDiffValue);
            pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1] = pOldPhaseCountDownVaule[iPhaseId - 1] + iDiffValue;

           // pOldPhaseCountDownVaule[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
        }
    }

}

void StoreOldCountDownData(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,int *nArray)
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
 函 数 名  : CalcPhaseRunTimeAndSplit
 功能描述  : 在倒计时中计算各相位的绿信比时间
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月4日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn)
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    int iPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iTempVal = 0 ;

    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//记录当前绿灯相位号，当该环内，绿灯相位发生变动时，将上个相位的相位运行时间清零
    static int nArrayCurrentPhaseCountDown[16] = {0};//本地记录一份各个相位的倒计时信息，用来感应时，保证各个相位的倒计时能同时增加

    int iDiffVal = 0;//用来记录感应时当前相位的绿灯增加值

    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

    
    //轮询相序表，计算单环中各个相位的绿信比
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//如果某个环没有配置，则不继续计算
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//轮询该环内的相位
        {
            iPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 == iPhaseId)//如果该环内已找到相位ID为0的相位，则表明已轮询结束，直接break掉，不再计算该环。
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] != 0)//如果当前相位运行时间不为0，则表明继续运行
            {
                pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1]++;
            }

            if(GREEN == pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][0])//找到灯色为绿色的相位
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = iPhaseId;//记录当前绿灯相位
                }
                if(iPhaseId != nArrayCurrentPhaseId[i])//如果运行相位发生变动，则清空上个相位的运行时间
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = iPhaseId;//记录当前绿灯相位
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] = 1;//如果运行时间是0，则将运行时间改为1
                }

                index = j+1;//记录绿灯相位的顺序，以此为起点，依次做差值，可得绿信比
    
                //计算当前相位的绿信比时间,等于下一个相位的倒计时时间加上当前相位的运行时间
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//如果该相位的下一个相位不存在，则下一个相位为相序表起始值
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] + pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] - 1;

                //在感应时，要保证一个环内所有相位的倒计时时间都要同步增加相同值
                if(nArrayCurrentPhaseCountDown[iPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];//刚开始时记录
                }

                iDiffVal = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1] - nArrayCurrentPhaseCountDown[iPhaseId - 1];
                

                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[iPhaseId - 1] != 1 ))//只有在相位未发生改变且倒计时跳变时才调用。
                {
                    fprintf(stderr,"===>  countdown   %d\n",nArrayCurrentPhaseCountDown[iPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];

                //用做差值的方式计算除最后一个相位以外的绿信比
                for(k = 0 ; k < index - 2; k++)//向后遍历
                {
                    iPhaseId = pPhaseTurn[i].nTurnArray[k];//得到当前的相位号
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[k+1];//得到该相序中下一个相位

                    if((0 == iPhaseId) || (0 == iNextPhaseId))
                    {
                        break;
                    }

                    pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] - pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
                    
                }

                for(k = index; k < NUM_PHASE - 1; k++)//向前遍历
                {
                    iPhaseId = pPhaseTurn[i].nTurnArray[k];//得到当前的相位号
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[k+1];//得到该相序中下一个相位
                    if((0 == iPhaseId) || (0 == iNextPhaseId))
                    {
                        if((index >= 2) && (iPhaseId != 0))//
                        {
                            pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[i].nTurnArray[0] - 1][1] - pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
                        }
                        
                        break;
                    }

                    pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] - pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
                }

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

    //store old data
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown);

}

void CollectCountDownParams()
{
	static UInt8 outPutCount = 0, outPutTotal = 0;	//倒计时接口调用计数和总共调用次数
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//存放每个相位运行之后即将运行的下一相位号
	static PhaseTurnItem phaseTrunTable[NUM_RING_COUNT]; 
	static FollowPhaseItem followPhaseTable[NUM_FOLLOW_PHASE];
	PhaseItem phaseTable[NUM_PHASE];
	GreenSignalRationItem splitTable[NUM_PHASE];
	UInt8 nSchemeID = 0;
	
	if (gCountDownParams->ucCurRunningTime == gCountDownParams->ucCurCycleTime)
	{
		pthread_rwlock_rdlock(&configLock);
		outPutTotal = CalCountDownCalledCount();
		nSchemeID = GetSchemeId(splitTable, phaseTable, phaseTrunTable, followPhaseTable);
		pthread_rwlock_unlock(&configLock);
		if (nSchemeID == 0)
			return;
		
		pthread_rwlock_wrlock(&countDownLock);
		SetCountDownParams(nSchemeID, splitTable, phaseTable);
		pthread_rwlock_unlock(&countDownLock);
		//先找出每个相位在相序表中的下一相位号,为后续计算跟随相位状态做准备
		FindNextPhaseIdInPhaseTrun(phaseTrunTable, nextPhaseIds);
	}
	
	if (++outPutCount == outPutTotal)
	{
		outPutCount = 0;
		SetFollowPhaseStatus(nextPhaseIds, followPhaseTable);
		pthread_rwlock_wrlock(&countDownLock);
		CalcPhaseRunTimeAndSplit(gCountDownParams, phaseTrunTable);
		pthread_rwlock_unlock(&countDownLock);
		SendFeedback();
	}
}
