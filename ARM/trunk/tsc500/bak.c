#include <string.h>
#include <pthread.h>
#include "HikConfig.h"
#include "platform.h"
#include "lfq.h"

#define INDUCTIVE_CONTROL_ACTION_ID		118
#define INDUCTIVE_CONTROL_SCHEME_ID		254

pthread_rwlock_t countDownLock = PTHREAD_RWLOCK_INITIALIZER;	//��Ҫ�������̷߳���ȫ������
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //����ʱ�ӿ���Ϣ
void *gHandle = NULL;	//����ʱʹ�õĶ��о��

extern UInt8 gChannelLockFlag;   //0��ʾδ������1��ʾ����
extern UInt8 gSpecialControlSchemeId;       //������Ʒ�����

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

static UInt8 CalCountDownCalledCount()	//���㵹��ʱ�ӿ��ܹ����ô���
{
	UInt8 MotorCount[NUM_PHASE] = {0};	//��¼��������λ���ô���
	UInt8 PedCount[NUM_PHASE] = {0};	//��¼������λ���ô���
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
	if (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)	//˵���Ǹ�Ӧ����
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
	{	//��Ӧ����Ĭ��ʹ�����űȱ�1�������1
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
 �� �� ��  : IsNextPhaseInFollowPhaseList
 ��������  : �ж���һ��λ�Ƿ��ڸ�����λ����
 �������  : 
 �� �� ֵ  : �����λ���������򷵻�TRUE����������false
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

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
			{	//��ĸ��λ��������ʱ���������������һ��λҲ�ڸ�����λ���У���ô��ʱ������λ״̬Ӧ�ǳ���GREEN����֮����״̬��ĸ��λ״̬һ��
				newStatus = IsNextPhaseInFollowPhaseList(&followPhaseTable[i], nextPhaseIds[motherPhase - 1]) ? GREEN : motherPhaseStatus;
				if (status == RED)
				{
					status = newStatus;
					continue;
				}
				//���Դ���໷ʱĸ��λͬʱ���е����
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
        //����û��ڣ�����λ�ĵ���ʱ����ֵ�������̵���λ�����ֵ�����ֶ������䵹��ʱʱ��

        if(pOldPhaseCountDownVaule[iPhaseId - 1] == 0)//�����ǵ�һ�ε��øú�������һ�εĵ���ʱ���ݻ�û�б�����������ֱ�ӱ���
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
 �� �� ��  : CalcPhaseRunTimeAndSplit
 ��������  : �ڵ���ʱ�м������λ�����ű�ʱ��
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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

    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//��¼��ǰ�̵���λ�ţ����û��ڣ��̵���λ�����䶯ʱ�����ϸ���λ����λ����ʱ������
    static int nArrayCurrentPhaseCountDown[16] = {0};//���ؼ�¼һ�ݸ�����λ�ĵ���ʱ��Ϣ��������Ӧʱ����֤������λ�ĵ���ʱ��ͬʱ����

    int iDiffVal = 0;//������¼��Ӧʱ��ǰ��λ���̵�����ֵ

    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

    
    //��ѯ��������㵥���и�����λ�����ű�
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//���ĳ����û�����ã��򲻼�������
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//��ѯ�û��ڵ���λ
        {
            iPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 == iPhaseId)//����û������ҵ���λIDΪ0����λ�����������ѯ������ֱ��break�������ټ���û���
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] != 0)//�����ǰ��λ����ʱ�䲻Ϊ0���������������
            {
                pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1]++;
            }

            if(GREEN == pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][0])//�ҵ���ɫΪ��ɫ����λ
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = iPhaseId;//��¼��ǰ�̵���λ
                }
                if(iPhaseId != nArrayCurrentPhaseId[i])//���������λ�����䶯��������ϸ���λ������ʱ��
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = iPhaseId;//��¼��ǰ�̵���λ
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] = 1;//�������ʱ����0��������ʱ���Ϊ1
                }

                index = j+1;//��¼�̵���λ��˳���Դ�Ϊ��㣬��������ֵ���ɵ����ű�
    
                //���㵱ǰ��λ�����ű�ʱ��,������һ����λ�ĵ���ʱʱ����ϵ�ǰ��λ������ʱ��
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//�������λ����һ����λ�����ڣ�����һ����λΪ�������ʼֵ
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] + pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][1] - 1;

                //�ڸ�Ӧʱ��Ҫ��֤һ������������λ�ĵ���ʱʱ�䶼Ҫͬ��������ֵͬ
                if(nArrayCurrentPhaseCountDown[iPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];//�տ�ʼʱ��¼
                }

                iDiffVal = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1] - nArrayCurrentPhaseCountDown[iPhaseId - 1];
                

                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[iPhaseId - 1] != 1 ))//ֻ������λδ�����ı��ҵ���ʱ����ʱ�ŵ��á�
                {
                    fprintf(stderr,"===>  countdown   %d\n",nArrayCurrentPhaseCountDown[iPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[iPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];

                //������ֵ�ķ�ʽ��������һ����λ��������ű�
                for(k = 0 ; k < index - 2; k++)//������
                {
                    iPhaseId = pPhaseTurn[i].nTurnArray[k];//�õ���ǰ����λ��
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[k+1];//�õ�����������һ����λ

                    if((0 == iPhaseId) || (0 == iNextPhaseId))
                    {
                        break;
                    }

                    pCountDownParams->stPhaseRunningInfo[iPhaseId - 1][0] = pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] - pCountDownParams->stVehPhaseCountingDown[iPhaseId - 1][1];
                    
                }

                for(k = index; k < NUM_PHASE - 1; k++)//��ǰ����
                {
                    iPhaseId = pPhaseTurn[i].nTurnArray[k];//�õ���ǰ����λ��
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[k+1];//�õ�����������һ����λ
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
    
    //����ʱ����ڻ�������ں���Ҫ���㣬���¿�ʼ��һ������
    if((pCountDownParams->ucCurRunningTime >= pCountDownParams->ucCurCycleTime) && (pCountDownParams->ucCurCycleTime != 0))
    {
        pCountDownParams->ucCurRunningTime = 0;
    }
    //��������ʱ��
    pCountDownParams->ucCurRunningTime++;

    //������������
    for(i = 0 ; i < 4; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//���ĳ����û�����ã��򲻼�������
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
	static UInt8 outPutCount = 0, outPutTotal = 0;	//����ʱ�ӿڵ��ü������ܹ����ô���
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//���ÿ����λ����֮�󼴽����е���һ��λ��
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
		//���ҳ�ÿ����λ��������е���һ��λ��,Ϊ�������������λ״̬��׼��
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
