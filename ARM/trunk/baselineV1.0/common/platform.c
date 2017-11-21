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

UInt8 outPutCount = 0; 	//����ʱ�ӿڵ��ü���
pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;	//��Ҫ�������̷߳���ȫ������
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams = NULL;       //����ʱ�ӿ���Ϣ�������ڵ���ʱ�ӿ��м���
PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //������udpserver���õĵ���ʱ����

MsgRealtimePattern gStructMsgRealtimePatternInfo;           //�ڵ���ʱ�ӿ��У�ÿ�����ڿ�ʼʱ���и���
MsgPhaseSchemeId gStructMsg1049;                            //��ƽ̨1049Э��ʹ��
pthread_rwlock_t gLockRealtimeVol = PTHREAD_RWLOCK_INITIALIZER;//����ʵʱ�����Ķ�д��
MsgRealtimeVolume gStructMsgRealtimeVolume;                 //ʵʱ������ֻ��������ʵʱ��

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //�����/home/config.dat�����е����нṹ�壬���������������ṹ��/����忪��/��־��ӡ����/�������к�/����������
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������

PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //��ǰʹ�õ�����

CountdownConfigFlag gCountdownConfigFlag = COUNTDOWN_CONFIG_HAVE_NEW;
static SignalControllerPara *gConfigPara2 = NULL;
extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;


extern CountDownVeh countdown_veh[17];                 //��������λ����ʱ����
extern CountDownPed countdown_ped[17];				   //������λ����ʱ����
extern unsigned char g_cCurrentActionId;//��ǰ���ж�����,ϵͳ����ʱ�����ܻ���и�Ӧ��������ȫ��.

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
	if (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)	//˵���Ǹ�Ӧ����
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

//ÿ�������ʵʱ���ݵ��ṹ����
static void SetRealtimeRunInfo()
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    //���㵱ǰ���ű��е�Э����λ���˴�����У�����űȻ������Ƿ�Ϸ����ϲ������ȷ����Ϸ���
    for(i = 0; i < 16; i++)
    {
        if(gConfigPara2->stGreenSignalRation[gStructMsgRealtimePatternInfo.nSplitId - 1][i].nIsCoordinate == 1)
        {
            gStructMsgRealtimePatternInfo.nCoordinatePhase |= (1<<i);//�����λ��ΪЭ����λ������bitλ����Ϊ1
        }
    }

    //���ø�����λ��ת�������е�ʱ�䣬����ȫ�졢����������ʱ�䡣
    for(i = 0; i < 16; i++)
    {
        gStructMsgRealtimePatternInfo.phaseTime[i].yellow = gConfigPara2->stPhase[i].nYellowTime;
        gStructMsgRealtimePatternInfo.phaseTime[i].allred = gConfigPara2->stPhase[i].nAllRedTime;
        gStructMsgRealtimePatternInfo.phaseTime[i].greenBlink = gConfigPara2->AscSignalTransTable[i].nGreenLightTime;
    }

    //���ø�����λ��Ϣ
    memcpy(gStructMsgRealtimePatternInfo.stFollowPhase,gConfigPara2->stFollowPhase,sizeof(gConfigPara2->stFollowPhase));


    //�źŻ����õ�������λ����λ��
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

    //�źŻ����õ����з����ţ�����ת��
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
    
    memset(&gStructMsgRealtimePatternInfo,0,sizeof(gStructMsgRealtimePatternInfo));//ÿ�����ڶ�������������¼���
    
	if (gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_SYSTEM || gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_STEP)	//ϵͳ����
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
	{	//��Ӧ����Ĭ��ʹ�����űȱ�1�������1
		nGreenSignalRatioID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gConfigPara2->stScheme[nSchemeID - 1].nGreenSignalRatioID;
		nPhaseTurnID = (nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID) ? 1 : gConfigPara2->stScheme[nSchemeID - 1].nPhaseTurnID;

		gStructMsgRealtimePatternInfo.nSplitId = nGreenSignalRatioID;
		gStructMsgRealtimePatternInfo.nPhaseTurnId = nPhaseTurnID;
		gStructMsgRealtimePatternInfo.nPatternId = nSchemeID;
		if(nSchemeID <= 108)
    		gStructMsgRealtimePatternInfo.nOffset = gConfigPara2->stScheme[nSchemeID - 1].nOffset;//���ر�������Ϣ
		
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

            memcpy(gStructMsgRealtimePatternInfo.sPhaseTurn,phaseTrunTable,sizeof(PhaseTurnItem) * NUM_RING_COUNT);//���������
			//ȷ�����űȺ������Ϸ���Ч������£����Э����λ�Ȼ�������
			SetRealtimeRunInfo();
			for (i = 0; i < NUM_PHASE; i++)
				greenBlinkTimes[i] = gConfigPara2->AscSignalTransTable[i].nGreenLightTime;
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
	UInt8 uIsPhaseRunning = 0;//�ж���λ�Ƿ�������
	
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
			uIsPhaseRunning = gCountDownParams->stPhaseRunningInfo[motherPhase - 1][1];//�ж���λ�Ƿ����е������Ǹ���λ����ʱ���Ƿ�Ϊ0
			if (uIsPhaseRunning != 0)
			{	//��ĸ��λ��������ʱ���������������һ��λҲ�ڸ�����λ���У���ô��ʱ������λ״̬Ӧ�ǳ���GREEN����֮����״̬��ĸ��λ״̬һ��
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
				//���Դ���໷ʱĸ��λͬʱ���е����
				if ((newStatus == GREEN) 
					|| (newStatus == GREEN_BLINK && (status == YELLOW || status == ALLRED))
					|| (newStatus == YELLOW && status == ALLRED))
					status = newStatus;
			}
		}
//		if (status == GREEN_BLINK)
//			status = GREEN; //������λû������״̬
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
static void StoreOldCountDownData(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,int *nArray,int *nArrayColor)
{
    int i = 0;

    if((nArray == NULL) || (pCountDownParams == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        //�������������ʱΪ0�Ļ����ͱ������˵ĵ���ʱ��
        nArray[i] = ((pCountDownParams->stVehPhaseCountingDown[i][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[i][1] : pCountDownParams->stVehPhaseCountingDown[i][1]);
        nArrayColor[i] = pCountDownParams->stVehPhaseCountingDown[i][0];
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
    int tmp = 0;
    
    for(k = 0 ; k < uIndex - 2; k++)//������
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ

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

    for(k = uIndex; k < NUM_PHASE - 1; k++)//��ǰ����
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ
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
          // INFO("---->  phase id  %d\n",i+1);
        }
    }
}
#if 0
/*****************************************************************************
 �� �� ��  : IsPhaseJumpChange
 ��������  : �ж���λ����ʱ�Ƿ���ͻ�䣬һ��ͻ�䣬����յ���ʱ��Ϣ�����¼�
             ��
 �������  : UInt8 nSchemeID                                        
             int *time                                              
             int *color                                             
             PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static UInt8 IsPhaseJumpChange(UInt8 nSchemeID,int *time,int *color,PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    if(nSchemeID == INDUCTIVE_CONTROL_SCHEME_ID)//�����ǰ�Ǹ�Ӧ���ƣ���������
    {
        return 0;
    }

    //INFO("%d-%d\n",time[0],color[0]);
    int i = 0;
    for(i = 0 ; i < 16; i++)
    {
        if((time[i] == 0 )|| (pCountDownParams->stVehPhaseCountingDown[i][1] == 0))//�������ʱΪ0����������λû�е���ʱ��Ϣ����������
        {
            continue;
        }
        //���ǰ���ɫ��ͬ��������ʱֵ���Ǳ��ϸ�С1������Ϊ����������
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
 �� �� ��  : ClearPhaseCountDownInfo
 ��������  : ��յ���ʱ��Ϣ
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void ClearPhaseCountDownInfo(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams)
{
    int i = 0;

    for(i = 0 ; i < 16; i++)
    {
        pCountDownParams->stPhaseRunningInfo[i][1] = 0;//��λ����ʱ�����;
    }

    pCountDownParams->ucCurCycleTime = 0;
    pCountDownParams->ucCurRunningTime = 0;//��ǰ���ڳ�����ǰ����ʱ������

    INFO("ClearPhaseCountDownInfo\n");
}
#endif

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
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn, UInt8 *pPhaseGreenBlink,UInt8 nSchemeID)
{
    int i = 0;
    int j = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iDiffVal = 0;//������¼��Ӧʱ��ǰ��λ���̵�����ֵ
    int tmp = 0;
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//��¼��ǰ�̵���λ�ţ����û��ڣ��̵���λ�����䶯ʱ�����ϸ���λ����λ����ʱ������
    static int nArrayCurrentPhaseCountDown[16] = {0};//���ؼ�¼һ�ݸ�����λ�ĵ���ʱ��Ϣ��������Ӧʱ����֤������λ�ĵ���ʱ��ͬʱ����
    static int nArrayCurrentPhaseColor[16] = {0};//���ؼ�¼һ�ݸ�����λ�ĵ�ɫ
    
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

    //���������λͻ�䣬�������λ����ʱ��Ϣ
    //if(IsPhaseJumpChange(nSchemeID,nArrayCurrentPhaseCountDown,nArrayCurrentPhaseColor, pCountDownParams) == 1)
   // {
    //    ClearPhaseCountDownInfo(pCountDownParams);
    //}
    

    //fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                       // pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);
   // INFO("===>   %d\n",pCountDownParams->ucCurCycleTime);

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
            //INFO("  cPhase Id   %d ,  veh  %d , ped  %d, running val  %d\n",cPhaseId,
            //                            pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0],
            //                            pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0],
             //                           pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]);
            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//�����ǰ��λ����ʱ�䲻Ϊ0���������������
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;//�Ͱѵ�ǰ��λ������ʱ���1
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0])/* || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0])*/)//�ҵ���ɫΪ��ɫ����λ
            {
                if(nArrayCurrentPhaseId[i] == -1)//�״�����ʱ�������ϴε��̵���λ
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//��¼��ǰ�̵���λ
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//���������λ�����䶯��������ϸ���λ������ʱ��
                {
                   // OFTEN("phase changed , from %d to %d \n",nArrayCurrentPhaseId[i],cPhaseId);
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//���µ�ǰ�̵���λ

                    if(j == 0)//�����λ�������е��˵�һ����λ������������ʱ�����㡣
                    {
                        pCountDownParams->ucCurRunningTime = 0;
                    }
                }
                else//�����λû�з����䶯�������εĵ���ʱֵ���Ǳ��ϴε���ʱֵС1��ҲҪ���¼��㣬����������ʱ�����㣬ͬʱ����������ʱ��Ҳ���㡣
                {
                    tmp = ((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) ? pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1]);
                    tmp = tmp - nArrayCurrentPhaseCountDown[cPhaseId - 1];
                   // OFTEN("CalcPhaseRunTimeAndSplit tmp %d, nArrayCurrentPhaseCountDown[i]  %d\n",tmp,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    if((tmp != -1) && (pCountDownParams->ucPlanNo != 254))//�������ʱ����Ҫ�ų���Ӧ�����.
                    {
                        pCountDownParams->ucCurRunningTime = 0;
                        pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 0;

                    }
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//�������ʱ����0��������ʱ���Ϊ1
                   // INFO("phase %d , change running time to  1 \n",cPhaseId);
                }

                index = j+1;//��¼�̵���λ��˳���Դ�Ϊ��㣬��������ֵ���ɵó���ĩ��λ������ű�

                //�ڸ�Ӧʱ��Ҫ��֤һ������������λ�ĵ���ʱʱ�䶼Ҫͬ��������ֵͬ
                if(pCountDownParams->ucPlanNo == 254)
                {
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
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //���㵱ǰ��λ�����ű�ʱ��,������һ����λ�ĵ���ʱʱ����ϵ�ǰ��λ������ʱ��
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//�������λ����һ����λ�����ڣ�����һ����λΪ�������ʼֵ
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;
               // INFO("---->    green phase  %d  runnin time  %d\n",cPhaseId,pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]);
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
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown,nArrayCurrentPhaseColor);

    //�޸�����״̬
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



PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gTest ;       //����������������

int UnitTest_PhaseRunTime()
{
    int i = 0;
    
    //��λ������ʱ���������������Ҫһֱ���ӵ�
    for(i = 0; i < 16; i++)
    {
        if(gCountDownParams->stPhaseRunningInfo[i][1] != 0)//����ʱ�䲻Ϊ0����������λ������
        {
            if((gCountDownParams->stPhaseRunningInfo[i][1] - gTest.stPhaseRunningInfo[i][1]) != 1)
            {
               // log_error("UnitTest_PhaseRunTime  ERROR   1\n");
                return 1;//����ʱ�䷢������!!!!!
            }
        }

        if(gCountDownParams->ucCurRunningTime != 1)//����ʱ�䲻Ϊ0����������λ������
        {
            if((gCountDownParams->ucCurRunningTime - gTest.ucCurRunningTime) != 1)
            {
                //log_error("UnitTest_PhaseRunTime  ERROR   2\n");
                return 2;//����ʱ�䷢������!!!!!
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
	static UInt8 outPutTotal = 0;	//����ʱ�ӿ��ܹ����ô���
	static UInt8 nextPhaseIds[NUM_PHASE] = {0};	//���ÿ����λ����֮�󼴽����е���һ��λ��
	static UInt8 greenBlinkTimes[NUM_PHASE];	//��λ����ʱ��
	PhaseItem *phaseTable;	//��λ��
	FollowPhaseItem *followPhaseTable;	//������λ��
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
		//���ҳ�ÿ����λ��������е���һ��λ��,Ϊ�������������λ״̬��׼��
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
 �� �� ��  : UpdateSpecialControlDesc
 ��������  : �ڵ�ǰ���ж������������ʱ�������䷽�������������š�
 �������  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��13��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void UpdateSpecialControlDesc(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend)
{
    switch(g_cCurrentActionId)
    {
        case 115:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_TURN_OFF; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"ϵͳ����-�ص�"); break;//SPECIAL_CONTROL_TURN_OFF
        case 116:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_ALL_RED; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"ϵͳ����-ȫ��"); break;//SPECIAL_CONTROL_ALL_RED
        case 117:    break;
        case 118:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_INDUCTION; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"ϵͳ����-��Ӧ"); break;//SPECIAL_CONTROL_INDUCTION
        case 119:   pCountDownParamsSend->ucPlanNo = SPECIAL_CONTROL_YELLOW_BLINK; strcpy((char *)pCountDownParamsSend->ucCurPlanDsc,"ϵͳ����-����"); break;//SPECIAL_CONTROL_YELLOW_BLINK
        default:     break;
    }
}


void UpdateCountdownParams()
{
    switch(gStructBinfileCustom.cSpecialControlSchemeId)
    {
        case SPECIAL_CONTROL_SYSTEM:         UpdateSpecialControlDesc(gCountDownParams); break;// ֻ���ڵ�ǰ��ϵͳ����ʱ���Ÿ������ⷽ���š�
        case SPECIAL_CONTROL_YELLOW_BLINK:  
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;        
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"����");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,YELLOW_BLINK);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,YELLOW_BLINK);
                SetPhaseStatus(gCountDownParams->ucOverlap,YELLOW_BLINK);
                break;
            }
        case SPECIAL_CONTROL_INDUCTION:     gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;  strcpy((char *)gCountDownParams->ucCurPlanDsc,"��Ӧ");       break;
        case SPECIAL_CONTROL_TURN_OFF:       
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"�ص�");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,TURN_OFF);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,TURN_OFF);
                SetPhaseStatus(gCountDownParams->ucOverlap,TURN_OFF);    
                break;
            }
        case SPECIAL_CONTROL_ALL_RED:        
            {
                gCountDownParams->ucPlanNo = gStructBinfileCustom.cSpecialControlSchemeId;
                strcpy((char *)gCountDownParams->ucCurPlanDsc,"ȫ��");
                SetPhaseStatus(gCountDownParams->stVehPhaseCountingDown,RED);
                SetPhaseStatus(gCountDownParams->stPedPhaseCountingDown,RED);
                SetPhaseStatus(gCountDownParams->ucOverlap,RED);                
                break;
            }
        default: break;
    }
    memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
}
