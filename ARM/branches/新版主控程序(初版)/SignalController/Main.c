#include "Main.h"


PSignalControllerPara pSignalControllerPara = null;//�źŻ����ò������ṹȫ�ֱ���


extern unsigned short gCurrentChannelArray[MAX_CHANNEL_NUM] ;//��ǰ�����£��ܹ�ʹ�õ�ͨ�������飬�����������βΣ����ݸ���ƺ���
extern unsigned short gCurrentChannelNum ;//��ǰ�����£��ܹ�ʹ�õ�ͨ������

unsigned short IsRunning = 1;//����ò���Ϊ1�������ģ���������������Ϊ0����ģ�������˳� 

/*****************************************************************************
 �� �� ��  : InitSignalControllerPara
 ��������  : ϵͳ�������ȳ�ʼ��ȫ�ֵ��źſ��Ʋ��������а���������������ͷ
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��24��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitSignalControllerPara()
{
    pSignalControllerPara = (PSignalControllerPara)malloc(sizeof(SignalControllerPara));

    if(!pSignalControllerPara)
    {
        log_debug("%s  alloc pSignalControllerPara failed \n",__func__);
        return ;
    }

    memset(pSignalControllerPara,0,sizeof(SignalControllerPara));

    //��ʼ�����ȱ�
    InitTaskSchedule(&pSignalControllerPara->pPlanScheduleList);

    //��ʼ��ʱ�α�
    InitTimeIntervalList(&pSignalControllerPara->pTimeIntervalList);
    
    //��ʼ��������
    InitActionList(&pSignalControllerPara->pActionList);

    //��ʼ��������
    InitShemeList(&pSignalControllerPara->pSchemeList);

    //��ʼ��ͨ����
    InitChannelList(&pSignalControllerPara->pChannelList);

    //��ʼ�����űȱ�
    InitGreenSignalRationList(&pSignalControllerPara->pGreenSignalRationList);

    //��ʼ����λ��
    InitPhaseList(&pSignalControllerPara->pPhaseList);

    //���ȫ�ֵ�Ԫ����
    pSignalControllerPara->pUintPara = InitUnit();

    //��ʼ��������λ��
    InitFollowPhaseList(&pSignalControllerPara->pFollowPhaseList);

    //��ʼ�������
    InitPhaseTurnList(&pSignalControllerPara->pPhaseTurnList);

}


/*****************************************************************************
 �� �� ��  : InitGolobalVar
 ��������  : ��˳���ʼ��ָ����ȫ�ֱ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void InitGolobalVar()
{
    //��ʼ��������־ϵͳ
    InitLogSys(LOG_PATH, LOG_MAX_SIZE, LOG_MAX_SIZE_EACH);

    //��ʼ������
    InitDateTime();

    //��ʼ���̼߳�� ģ��
    InitThreadControllerList();

    //��ʼ��ȫ�ֲ��� 
    InitSignalControllerPara();

    //��ʼ��CANͨ�ż�CPLD
    InitLampController();

}

/*****************************************************************************
 �� �� ��  : SetGolobalVar
 ��������  : ��һ��˳���ȫ�ֱ�����ֵ������ֻ�Ǽ���Ĭ��ֵ����ʵӦ���Ǵ�����
             ����������ļ������ȡ�����ġ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void SetGolobalVar()
{
    LoadDefaultTaskSchedule(pSignalControllerPara->pPlanScheduleList);

    LoadDefaultTimeInterval(pSignalControllerPara->pTimeIntervalList);

    LoadDefaultSchemes(pSignalControllerPara->pSchemeList);

    LoadDefaultAction(pSignalControllerPara->pActionList);

    LoadDefaultPhase(pSignalControllerPara->pPhaseList);

    LoadDefaultChannel(pSignalControllerPara->pChannelList);

    LoadDefaultGreenSignalRation(pSignalControllerPara->pGreenSignalRationList);

    LoadDefaultUnitPara(pSignalControllerPara->pUintPara);

    LoadDefaultFollowPhase(pSignalControllerPara->pFollowPhaseList);

    LoadDefaultPhaseTurnList(pSignalControllerPara->pPhaseTurnList);
}



void DestroyGolobalVar()
{

    //TO DO  �������ĳ�ʼ������ͳһ����ں��棬��д�ɹ���ʾ��

    DestroyDateTime();


    DestroyThreadControllerList();

    DestroyLogSys();

    
}

/*****************************************************************************
 �� �� ��  : PowerOnAllRed
 ��������  : ϵͳ�������л�����������һ��ʱ���ȫ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void PowerOnAllRed()
{
    log_debug("%s time  %d\n",__func__,pSignalControllerPara->pUintPara->nBootAllRedTime);
    ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                    gCurrentChannelNum,gCurrentChannelArray,
                    Lamp_Red,pSignalControllerPara->pUintPara->nBootAllRedTime);

}
/*****************************************************************************
 �� �� ��  : PowerOnYellowLight
 ��������  : ϵͳ������������һ��ʱ��Ļ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
void PowerOnYellowLight()
{
    ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                    gCurrentChannelNum,gCurrentChannelArray,
                    Lamp_Yellow_Light,pSignalControllerPara->pUintPara->nBootYellowLightTime);
}


/*****************************************************************************
 �� �� ��  : SignalControllerRun
 ��������  : �źŻ��źſ�����ں��������а�������λ��ͨ�����߼�
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int SignalControllerRun(void)
{

    int nTimeIntervalID = 0;
    int nActionID = 0;
    int nSchemeID = 0;
    int nGreenSignalRatioID = 0;
    int nPhaseID = 0;
    int nControllerType = 0;
    int nPhaseTurnID = 0;
    int nPhaseNum = 0;
    unsigned short *nPhaseArray = null;
    
    
    PPhaseItem pTempPhaseItem = null;
    PChannelList pTempChannelList = pSignalControllerPara->pChannelList;

    unsigned short nArrayControllerChannel[MAX_CHANNEL_NUM];
    unsigned short nControllerChannelNum = 0;

    unsigned short nArrayControllerFollowChannel[MAX_CHANNEL_NUM];
    unsigned short nControllerFollowChannelNum = 0;

    //���� ����
    PowerOnYellowLight();
    
    //����ȫ��
    PowerOnAllRed();


    while(IsRunning)
    {

        //���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
        nTimeIntervalID = GetTimeIntervalIndex(&pSignalControllerPara->pPlanScheduleList);
        log_debug("nTimeIntervalID  :  %d\n",nTimeIntervalID);

        //����ʱ�α�ID���õ�������ID
        nActionID = GetActionID(&pSignalControllerPara->pTimeIntervalList, nTimeIntervalID);
        log_debug("nActionID  :  %d\n",nActionID);

        //���ݶ�����ID���õ�������ID
        nSchemeID = GetSchemeID(&pSignalControllerPara->pActionList,nActionID);
        log_debug("nSchemeID   : %d\n",nSchemeID);

        //���ݷ�����ID���õ����űȱ�ID
        nGreenSignalRatioID = GetGreenSignalRatioID(&pSignalControllerPara->pSchemeList,nSchemeID);
        log_debug("nGreenSignalRatioID  : %d\n",nGreenSignalRatioID);


        //���ݷ�����ID���õ������ID
        nPhaseTurnID = GetPhaseOrderID(&pSignalControllerPara->pSchemeList,nSchemeID);
        log_debug("nPhaseTurnID  : %d\n",nPhaseTurnID);

        //���������ID�õ���λ����
        if(nPhaseNum == 0)
        {
            nPhaseNum = GetPhaseNum(pSignalControllerPara->pPhaseTurnList,nPhaseTurnID);
            log_debug("nPhaseNum  : %d\n",nPhaseNum);

            nPhaseArray = GetPhaseArray(pSignalControllerPara->pPhaseTurnList,nPhaseTurnID);
        }
        
        //������λID���õ���λ��
        pTempPhaseItem = GetPhaseItem(pSignalControllerPara->pPhaseList, *nPhaseArray);
        if(!pTempPhaseItem)
        {
            log_debug("Phase %d doesn't exist \n",*nPhaseArray);
            nPhaseArray++;
            nPhaseNum--;
            continue;
        }

        
        nPhaseID = *nPhaseArray;
        nControllerType = MOTOR;

        log_debug("nPhaseID   %d\n",nPhaseID);

        //���ݵ�ǰ��λID����ǰ�����űȱ�ID������λ���е����űȲ�����ֵ��
        SetPhaseGreenSignalRationItem(&pSignalControllerPara->pGreenSignalRationList, nGreenSignalRatioID, nPhaseID);

        //log_debug("Current Color  : Green\n");
        //printf("Current Phase %d  Current Channel :",nPhaseID);

        nControllerChannelNum = 0;
        memset(nArrayControllerChannel,0,sizeof(nArrayControllerChannel));

        nControllerFollowChannelNum = 0;
        memset(nArrayControllerFollowChannel,0,sizeof(nArrayControllerFollowChannel));

        //�ҵ���ǰ��λ���ж���ͨ��
        while(pTempChannelList->next != null)
        {
            //��������λ��������λ
            if((pTempChannelList->pItem->nControllerID == nPhaseID) && (pTempChannelList->pItem->nControllerType == nControllerType))
            {
                //printf(" ########## %d\n",pTempChannelList->pItem->nChannelID);
                nArrayControllerChannel[nControllerChannelNum++] = pTempChannelList->pItem->nChannelID;
            }

            //�ж��Ƿ��Ǹ�����λ
            if((pTempChannelList->pItem->nControllerType == FOLLOW) && (IsPhaseInFollowPhase(pSignalControllerPara->pFollowPhaseList,nPhaseID,pTempChannelList->pItem->nControllerID)))
            {
                nArrayControllerFollowChannel[nControllerFollowChannelNum++] = pTempChannelList->pItem->nChannelID;
            }

            
            pTempChannelList = pTempChannelList->next;

        }
        log_debug("Green\n");
        //�̵�ʱ��
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Green, 
                        pTempPhaseItem->nGreenSignalRationTime - pTempPhaseItem->nAllRedTime - pTempPhaseItem->nGreenLightTime - pTempPhaseItem->nYellowTime);



        log_debug("Green Light\n");
        //����ʱ��
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Green_Light, 
                        pTempPhaseItem->nGreenLightTime);

        log_debug("Yellow Light\n");
        //�Ƶ�ʱ��
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Yellow,
                        pTempPhaseItem->nYellowTime);
        log_debug("All Red\n");                
        //ȫ��ʱ��
        ControlLampLight(gCurrentChannelNum,gCurrentChannelArray,
                        nControllerChannelNum,nArrayControllerChannel,
                        Lamp_Red, 
                        pTempPhaseItem->nYellowTime);

        //printf("\n");
        pTempChannelList = pSignalControllerPara->pChannelList;

        nPhaseArray++;
        nPhaseNum--;

    }

    return true;
}


/*****************************************************************************
 �� �� ��  : main
 ��������  : �źŻ�������
 �������  : int argc     
             char **argv  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��21��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int main(int argc, char **argv)
{
    InitGolobalVar();

    SetGolobalVar();

    ThreadCreate((void *)ThreadUpdateDateTime,NULL);

    ThreadCreate((void *)ThreadClearMem,NULL);
    
    ThreadCreate((void *)SignalControllerRun,NULL);




    MainThreadControllerFun();

    DestroyGolobalVar();

    
    return 0;
}

