#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "HikConfig.h"
#include "platform.h"
#include "lfq.h"
#include "common.h"

#define INDUCTIVE_CONTROL_ACTION_ID		118
#define INDUCTIVE_CONTROL_SCHEME_ID		254

pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;	//��Ҫ�������̷߳���ȫ������
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //����ʱ�ӿ���Ϣ
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //������udpserver���õĵ���ʱ����


void *gHandle = NULL;	//����ʱʹ�õĶ��о��

UInt8 gChannelLockFlag;   //0��ʾδ������1��ʾ����,2��ʾ��������������״̬��ʾ�յ���ͨ����������ǵ�ǰʱ��Ϊ������ʱ��ε�״̬��
UInt8 gSpecialControlSchemeId;       //������Ʒ�����

PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //��ǰʹ�õ�����

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;

extern PATTERN_NAME_PARAMS pattern_name_params;        //��������

extern CountDownVeh countdown_veh[17];                 //��������λ����ʱ����
extern CountDownPed countdown_ped[17];				   //������λ����ʱ����

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
	{	//��Ӧ����Ĭ��ʹ�����űȱ�1�������1
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

static UInt8 CalFollowPhaseCountDown(UInt8 *nextPhaseIds, FollowPhaseItem *followPhaseItem, PhaseItem *phaseTable, PhaseChannelStatus status)
{
	int i;
	PhaseChannelStatus motherPhaseStatus, bakMotherPhase;
	UInt8 motherPhase, nextPhase;
	UInt8 followPhaseCountDown = 0, motherPhaseCountDown = 0;
	UInt8 uIsPhaseRunning = 0;//�ж���λ�Ƿ�������
	
	for (i = 0; i < NUM_PHASE; i++)
	{
		motherPhase = followPhaseItem->nArrayMotherPhase[i];
		if (motherPhase == 0) 
			break;
		motherPhaseStatus = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][0];
		motherPhaseCountDown = gCountDownParams->stVehPhaseCountingDown[motherPhase - 1][1];
		uIsPhaseRunning = gCountDownParams->stPhaseRunningInfo[motherPhase - 1][1];//�ж���λ�Ƿ����е������Ǹ���λ����ʱ���Ƿ�Ϊ0
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
	UInt8 uIsPhaseRunning = 0;//�ж���λ�Ƿ�������
	
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
			uIsPhaseRunning = gCountDownParams->stPhaseRunningInfo[motherPhase - 1][1];//�ж���λ�Ƿ����е������Ǹ���λ����ʱ���Ƿ�Ϊ0
			if (uIsPhaseRunning != 0)
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
 �� �� ��  : UpdateCountDownTime
 ��������  : ��֤��Ӧ����ʱ��������λ�ĵ���ʱ���ܱ�����ͬ�����ı仯
 �������  : int iDiffValue                                
             unsigned char *pPhaseArray                    
             int *pOldPhaseCountDownVaule                  
             PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
        //����û��ڣ�����λ�ĵ���ʱ����ֵ�������̵���λ�����ֵ�����ֶ������䵹��ʱʱ��

        if(pOldPhaseCountDownVaule[cPhaseId - 1] == 0)//�����ǵ�һ�ε��øú�������һ�εĵ���ʱ���ݻ�û�б�����������ֱ�ӱ���
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
 �� �� ��  : StoreOldCountDownData
 ��������  : ������һ�εĵ���ʱ��Ϣ
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             int *nArray                                   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : CalcGreenSplit
 ��������  : ������λ�����ű�
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             unsigned char cCircleNo                       
             PhaseTurnItem *pPhaseTurn                     
             unsigned char uIndex                          
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcGreenSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,unsigned char cCircleNo,PhaseTurnItem *pPhaseTurn,unsigned char uIndex)
{
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;

    for(k = 0 ; k < uIndex - 2; k++)//������
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ

        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            break;
        }

        pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
                                                                ((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
        
    }

    for(k = uIndex; k < NUM_PHASE - 1; k++)//��ǰ����
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ
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
 �� �� ��  : CalcCircleTime
 ��������  : �����������ڳ�
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcCircleTime(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn)
{
    int i = 0;
    int j = 0;
    
    int iTempVal = 0;
    
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
}

/*****************************************************************************
 �� �� ��  : AlterPhaseGreenLightTime
 ��������  : ������λ����ʱ�䣬���ض�ʱ��ε��̵�״̬��Ϊ����
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             unsigned char *pPhaseGreenBlink               
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : CalcPhaseRunTimeAndSplit
 ��������  : �ڵ���ʱ�м������λ�����ű�ʱ�䡢����ʱ�估��������ʱ�䡢����ʱ��
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  ����ʱ�ṹ��ָ��
             PhaseTurnItem *pPhaseTurn                     ��ǰ���е������         ��ά����[4][16]
             unsigned char *pPhaseGreenLight                ��λ����ʱ��ָ��        ������16
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �ݲ��������˸�Ӧ�����
*****************************************************************************/
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn, UInt8 *pPhaseGreenBlink)
{
    int i = 0;
    int j = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iDiffVal = 0;//������¼��Ӧʱ��ǰ��λ���̵�����ֵ
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//��¼��ǰ�̵���λ�ţ����û��ڣ��̵���λ�����䶯ʱ�����ϸ���λ����λ����ʱ������
    static int nArrayCurrentPhaseCountDown[16] = {0};//���ؼ�¼һ�ݸ�����λ�ĵ���ʱ��Ϣ��������Ӧʱ����֤������λ�ĵ���ʱ��ͬʱ����
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

   // fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                      //  pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);

    //��ѯ��������㵥���и�����λ�����ű�
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 >= pPhaseTurn[i].nTurnArray[0])//���ĳ����û�����ã��򲻼�������
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//��ѯ�û��ڵ���λ
        {
            cPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 >= cPhaseId)//����û������ҵ���λIDΪ0����λ�����������ѯ������ֱ��break�������ټ���û���
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//�����ǰ��λ����ʱ�䲻Ϊ0���������������
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0]))//�ҵ���ɫΪ��ɫ����λ
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//��¼��ǰ�̵���λ
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//���������λ�����䶯��������ϸ���λ������ʱ��
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//���µ�ǰ�̵���λ
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//�������ʱ����0��������ʱ���Ϊ1
                }

                index = j+1;//��¼�̵���λ��˳���Դ�Ϊ��㣬��������ֵ���ɵó���ĩ��λ������ű�

                //�ڸ�Ӧʱ��Ҫ��֤һ������������λ�ĵ���ʱʱ�䶼Ҫͬ��������ֵͬ
                if(nArrayCurrentPhaseCountDown[cPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];//�տ�ʼʱ��¼
                }
                iDiffVal = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - nArrayCurrentPhaseCountDown[cPhaseId - 1];


                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[cPhaseId - 1] != 1 ) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))//ֻ������λδ�����ı��ҵ���ʱ����ʱ�ŵ��á�
                {
                    //fprintf(stderr,"===>phase %d  countdown   %d\n",cPhaseId,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //���㵱ǰ��λ�����ű�ʱ��,������һ����λ�ĵ���ʱʱ����ϵ�ǰ��λ������ʱ��
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//�������λ����һ����λ�����ڣ�����һ����λΪ�������ʼֵ
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;

                //������ֵ�ķ�ʽ��������һ����λ��������ű�
                CalcGreenSplit(pCountDownParams,i,pPhaseTurn,index);
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
    CalcCircleTime(pCountDownParams,pPhaseTurn);

    //store old data
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown);

    //�޸�����״̬
    AlterPhaseGreenLightTime(pCountDownParams,pPhaseGreenBlink);
}

#define GET_COLOR(val)    (((val) == 1) ? "��" : ((val == 2) ? "��" : ((val == 3) ? "��" : "")))

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
	static UInt8 outPutCount = 0, outPutTotal = 0;	//����ʱ�ӿڵ��ü������ܹ����ô���
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//���ÿ����λ����֮�󼴽����е���һ��λ��
	static FollowPhaseItem followPhaseTable[NUM_FOLLOW_PHASE];	//������λ������
	static UInt8 greenBlinkTimes[NUM_PHASE];	//��λ����ʱ��
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
		//���ҳ�ÿ����λ��������е���һ��λ��,Ϊ�������������λ״̬��׼��
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
