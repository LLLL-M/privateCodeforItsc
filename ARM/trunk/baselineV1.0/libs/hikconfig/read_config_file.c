#include <stdio.h>
#include <string.h>
#include "HikConfig.h"
#include "parse_ini.h"


#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)

#define MAX_SECTION_LEN	64

//#define PRINT_ARGS      //��ӡ��Ϣ����

/*****************************************************************************
 �� �� ��  : ArrayToInt
 ��������  : ��char�����飬ת����int��.
 �������  : unsigned char *array  
             int len               
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
unsigned int ArrayToInt(unsigned char *array,int len)
{
	if(!array)
	{
		return 0;
	}
	
	int i = 0;
	unsigned int ret = 0;
	
	for(; i < len; i++)
	{
		if(0 != array[i])
		{
			ret |= (1 << array[i]);
		}
			
	}
	
	return ret;
}

/*****************************************************************************
 �� �� ��  : ReadUnitPara
 ��������  : �������ļ��ж�ȡ��Ԫ����
 �������  : PUnitPara item  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadUnitPara(PUnitPara item)
{
    if(NULL == item)
    {
        return ;
    }
    
	char cSection[MAX_SECTION_LEN] = "UnitPara";

    item->nBootAllRedTime = get_one_value(cSection,"nBootAllRedTime");
    item->nBootYellowLightTime = get_one_value(cSection,"nBootYellowLightTime");
    item->byFluxCollectCycle = get_one_value(cSection,"byFluxCollectCycle");
    item->byTransCycle = get_one_value(cSection,"byTransCycle");
    item->byCollectCycleUnit = get_one_value(cSection,"byCollectCycleUnit");

#ifdef PRINT_ARGS
    DEG("%s  stUnitPara nBootAllRedTime  %d  , nBootYellowLightTime  %d byFluxCollectCycle  %d , byTransCycle %d\n",__func__,
                        item->nBootAllRedTime,
                        item->nBootYellowLightTime,
                        item->byFluxCollectCycle,
                        item->byTransCycle);
#endif
}

/*****************************************************************************
 �� �� ��  : ReadPhaseItem
 ��������  : ��ȡ��λ�����
 �������  : PPhaseItem item                      
             struct STRU_SignalTransEntry *entry  
             int num                              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadPhaseItem(PPhaseItem item, struct STRU_SignalTransEntry *entry, int num)
{
    if((NULL == item) || (NULL == entry))
    {
        return ;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"PhaseItem%d",i+1);

        item[i].nPhaseID = get_one_value(cSection,"nPhaseID");
        //pSignalControlpara->stPhase[i].nPhaseID = i+1;
        item[i].nCircleID = get_one_value(cSection,"nCircleID");
		get_more_value(cSection,"byPhaseConcurrency",item[i].byPhaseConcurrency,sizeof(item[i].byPhaseConcurrency));
        item[i].nYellowTime = get_one_value(cSection,"nYellowTime");
        item[i].nAllRedTime = get_one_value(cSection,"nAllRedTime");
        entry[i].nGreenLightTime = get_one_value(cSection,"nGreenLightTime");//����ʱ�����ź�ת����
        item[i].wPhaseOptions = get_one_value(cSection,"wPhaseOptions");//��λʹ������λѡ����
        item[i].nPedestrianPassTime = get_one_value(cSection,"nPedestrianPassTime");
        item[i].nPedestrianClearTime = get_one_value(cSection,"nPedestrianClearTime");
		//pSignalControlpara->stPhase[i].cAutoPedestrianPass = get_one_value(cSection,"cAutoPedestrianPass");

        item[i].nMinGreen = get_one_value(cSection,"nMinGreen");
        item[i].nMaxGreen_1 = get_one_value(cSection,"nMaxGreen_1");
        item[i].nMaxGreen_2 = get_one_value(cSection,"nMaxGreen_2");
        item[i].nUnitExtendGreen = get_one_value(cSection,"nUnitExtendGreen");
        item[i].nRedProtectedTime = get_one_value(cSection,"nRedProtectedTime");
        item[i].byPhaseStartup = get_one_value(cSection,"byPhaseStartup");

#ifdef PRINT_ARGS
        if(item[i].nCircleID != 0)
            DEG("%s  stPhase  nPhaseID  %d  , nCircleID  %d  , nYellowTime  %d  , nAllRedTime  %d  , nGreenLightTime  %d, cIsEnablePhase  %d , nPedestrianPassTime  %d ,nPedestrianClearTime  %d\n",__func__,
                        item[i].nPhaseID,
                        item[i].nCircleID,
                        item[i].nYellowTime,
                        item[i].nAllRedTime,
                        entry[i].nGreenLightTime,
                        item[i].wPhaseOptions&0x01,
                        item[i].nPedestrianPassTime,
                        item[i].nPedestrianClearTime);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}


/*****************************************************************************
 �� �� ��  : ReadChannelItem
 ��������  : ������ȡͨ������
 �������  : PChannelItem item  
             int num            
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/

void ReadChannelItem(PChannelItem item, int num)
{
    if(NULL == item)
    {
        return;
    }

    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"ChannelItem%d",i+1);

        item[i].nChannelID = get_one_value(cSection,"nChannelID");
        item[i].nControllerID = get_one_value(cSection,"nControllerID");
        item[i].nControllerType = get_one_value(cSection,"nControllerType");

#ifdef PRINT_ARGS
        if(item[i].nControllerID != 0)
            DEG("%s  stChannel  nChannelID  %d  , nControllerID  %d  , nControllerType  %d    \n",__func__,
                        item[i].nChannelID,
                        item[i].nControllerID,
                        item[i].nControllerType);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadGreenSignalRationItem
 ��������  : ������ȡ���űȲ���
 �������  : PGreenSignalRationItem item  
             int nGreenSignalRationId     
             int nPhaseNum                
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadGreenSignalRationItem(PGreenSignalRationItem item, int nGreenSignalRationId,int nPhaseNum)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < nPhaseNum; i++)
    {
        sprintf(cSection,"GreenSignalRationItem%d_%d",nGreenSignalRationId+1,i+1);

        item[i].nGreenSignalRationID = get_one_value(cSection,"nGreenSignalRationID");
        item[i].nPhaseID = get_one_value(cSection,"nPhaseID");
        item[i].nGreenSignalRationTime = get_one_value(cSection,"nGreenSignalRationTime");
        item[i].nIsCoordinate = get_one_value(cSection,"nIsCoordinate");
#ifdef PRINT_ARGS
        if(item[i].nPhaseID != 0)
         DEG("%s  stGreenSignalRation  nGreenSignalRationID  %d  , nPhaseID  %d , nGreenSignalRationTime  %d \n",__func__,
                                item[i].nGreenSignalRationID,
                                item[i].nPhaseID,
                                item[i].nGreenSignalRationTime);
#endif

        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadPhaseTurnItem
 ��������  : ������ȡ�������
 �������  : PhaseTurnItem *item  
             int nCircleId        
             int num              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadPhaseTurnItem(PhaseTurnItem *item,int nCircleId, int num)
{
    int j = 0;
    char cSection[MAX_SECTION_LEN] = {0};
	
	if(item == NULL)
	{
		return;
	}
	
    for(j = 0 ; j < num; j++)
    {
        sprintf(cSection,"PhaseTurnItem%d_%d",nCircleId+1,j+1);

        item[j].nPhaseTurnID = get_one_value(cSection,"nPhaseTurnID");
        //pSignalControlpara->stPhaseTurn[i].nPhaseTurnID = i+1;
        item[j].nCircleID = get_one_value(cSection,"nCircleID");
        get_more_value(cSection,"nTurnArray",item[j].nTurnArray,sizeof(item[j].nTurnArray));
#ifdef PRINT_ARGS
        if(item[j].nCircleID != 0)
         DEG("%s  stPhaseTurn  nPhaseTurnID  %d  , nCircleID  %d ,  nTurnArray[0]  %d  , nTurnArray[1]  %d  , nTurnArray[2]  %d  , nTurnArray[3]  %d  \n",__func__,
                        item[j].nPhaseTurnID,
                        item[j].nCircleID ,
                        item[j].nTurnArray[0],
                        item[j].nTurnArray[1],
                        item[j].nTurnArray[2],
                        item[j].nTurnArray[3]);
#endif                        
        memset(cSection,0,sizeof(cSection));
    }


}

/*****************************************************************************
 �� �� ��  : ReadSchemeItem
 ��������  : ������ȡ���������
 �������  : PSchemeItem item  
             int num           
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadSchemeItem(PSchemeItem item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"SchemeItem%d",i+1);

        item[i].nSchemeID = get_one_value(cSection,"nSchemeID");
        //pSignalControlpara->stScheme[i].nSchemeID = i+1;
        item[i].nCycleTime = get_one_value(cSection,"nCycleTime");
        item[i].nGreenSignalRatioID = get_one_value(cSection,"nGreenSignalRatioID");
        item[i].nPhaseTurnID = get_one_value(cSection,"nPhaseTurnID");
#ifdef PRINT_ARGS
        if(item[i].nGreenSignalRatioID != 0)
            DEG("%s  stScheme  nSchemeID  %d  , nCycleTime  %d  , nGreenSignalRatioID  %d  , nPhaseTurnID  %d   \n",__func__,
                        item[i].nSchemeID,
                        item[i].nCycleTime,
                        item[i].nGreenSignalRatioID,
                        item[i].nPhaseTurnID);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadActionItem
 ��������  : ������ȡ���������
 �������  : PActionItem item  
             int num           
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadActionItem(PActionItem item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};
    
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"ActionItem%d",i+1);

        item[i].nActionID = get_one_value(cSection,"nActionID");
        item[i].nSchemeID = get_one_value(cSection,"nSchemeID");
#ifdef PRINT_ARGS
        if(item[i].nSchemeID != 0)
         DEG("%s  stAction  nActionID  %d  , nSchemeID  %d   \n",__func__,
                        item[i].nActionID,
                        item[i].nSchemeID);
#endif                        
        memset(cSection,0,sizeof(cSection));
    }

}


/*****************************************************************************
 �� �� ��  : ReadTimeIntervalItem
 ��������  : ������ȡʱ�α����
 �������  : TimeIntervalItem *item  
             int nTimeIntervalId     
             int nTime               
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadTimeIntervalItem(TimeIntervalItem *item, int nTimeIntervalId, int nTime)
{
    int j = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    for(j = 0 ; j < nTime ; j++)
    {
        sprintf(cSection,"TimeIntervalItem%d_%d",nTimeIntervalId+1,j+1);

        item[j].nTimeIntervalID = get_one_value(cSection,"nTimeIntervalID");
        //item[i][j].nTimeIntervalID = i+1;
        item[j].nTimeID = get_one_value(cSection,"nTimeID");
        item[j].cStartTimeHour = get_one_value(cSection,"cStartTimeHour");
        item[j].cStartTimeMinute = get_one_value(cSection,"cStartTimeMinute");
        item[j].nActionID = get_one_value(cSection,"nActionID");
#ifdef PRINT_ARGS

        if(item[j].nActionID != 0)
            DEG("%s  stTimeInterval  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , nActionID  %d   \n",__func__,
                                item[j].nTimeIntervalID,
                                item[j].nTimeID,
                                item[j].cStartTimeHour,
                                item[j].cStartTimeMinute,
                                item[j].nActionID);
#endif                                
        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadPlanSchedule
 ��������  : ������ȡ���������
 �������  : PPlanScheduleItem item  
             int num                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadPlanSchedule(PPlanScheduleItem item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    PlanTime planTime;
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"PlanScheduleItem%d",i+1);

        item[i].nScheduleID = get_one_value(cSection,"nScheduleID");
        item[i].nTimeIntervalID = get_one_value(cSection,"nTimeIntervalID");
        get_more_value(cSection,"month",planTime.month,sizeof(planTime.month));
        item[i].month = ArrayToInt(planTime.month,sizeof(planTime.month));
        
        get_more_value(cSection,"day",planTime.day,sizeof(planTime.day));
        item[i].day = ArrayToInt(planTime.day,sizeof(planTime.day));
        
        get_more_value(cSection,"week",planTime.week,sizeof(planTime.week));
        item[i].week = ArrayToInt(planTime.week,sizeof(planTime.week));
#ifdef PRINT_ARGS

        if(item[i].nTimeIntervalID != 0)
            DEG("%s  stPlanSchedule  nScheduleID  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,item[i].nScheduleID,
                                                                                     item[i].nTimeIntervalID,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif
        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadFollowPhaseItem
 ��������  : ������ȡ������λ����
 �������  : PFollowPhaseItem item  
             int num                
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadFollowPhaseItem(PFollowPhaseItem item, int num)
{
    int i = 0;
    char cSection[MAX_SECTION_LEN] = {0};

    memset(cSection,0,sizeof(cSection));
    for(i = 0 ; i < num; i++)
    {
        sprintf(cSection,"FollowPhaseItem%d",i+1);

        item[i].nFollowPhaseID = get_one_value(cSection,"nFollowPhaseID");
        //pSignalControlpara->stFollowPhase[i].nFollowPhaseID = i+1;
        get_more_value(cSection,"nArrayMotherPhase",item[i].nArrayMotherPhase,sizeof(item[i].nArrayMotherPhase));

#ifdef PRINT_ARGS

        if(item[i].nFollowPhaseID  != 0)
            DEG("%s  stFollowPhase  nFollowPhaseID  %d  , nArrayMotherPhase[0]  %d , nArrayMotherPhase[1]  %d  , nArrayMotherPhase[2]  %d  \n",__func__,
                        item[i].nFollowPhaseID,
                        item[i].nArrayMotherPhase[0],
                        item[i].nArrayMotherPhase[1],
                        item[i].nArrayMotherPhase[2]);

#endif
        memset(cSection,0,sizeof(cSection));
    }
}

/*****************************************************************************
 �� �� ��  : ReadVehicleDetector
 ��������  : ������ȡ�������������
 �������  : struct STRU_N_VehicleDetector *item  
             int num                              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadVehicleDetector(struct STRU_N_VehicleDetector *item, int num)
{
	int i;
	char section[MAX_SECTION_LEN]  = {0};
	
	for(i = 0; i < num; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "VehicleDetector", item[i].byVehicleDetectorNumber);

        item[i].byVehicleDetectorNumber = get_one_value(section,"byVehicleDetectorNumber");
        item[i].byVehicleDetectorCallPhase = get_one_value(section,"byVehicleDetectorCallPhase");
        item[i].byVehicleDetectorOptions = get_one_value(section,"byVehicleDetectorOptions");
        item[i].byVehicleDetectorSwitchPhase = get_one_value(section,"byVehicleDetectorSwitchPhase");
        item[i].byVehicleDetectorDelay = get_one_value(section,"byVehicleDetectorDelay");
        item[i].byVehicleDetectorExtend = get_one_value(section,"byVehicleDetectorExtend");
        item[i].byVehicleDetectorNoActivity = get_one_value(section,"byVehicleDetectorNoActivity");
        item[i].byVehicleDetectorMaxPresence = get_one_value(section,"byVehicleDetectorMaxPresence");
        item[i].byVehicleDetectorFailTime = get_one_value(section,"byVehicleDetectorFailTime");
        item[i].byVehicleDetectorAlarms = get_one_value(section,"byVehicleDetectorAlarms");
        item[i].byVehicleDetectorQueueLimit = get_one_value(section,"byVehicleDetectorQueueLimit");
        item[i].byVehicleDetectorErraticCounts = get_one_value(section,"byVehicleDetectorErraticCounts");

	}
}

/*****************************************************************************
 �� �� ��  : ReadPedestrianDetector
 ��������  : ������ȡ���˼��������
 �������  : struct STRU_N_PedestrianDetector *item  
             int num                                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadPedestrianDetector(struct STRU_N_PedestrianDetector *item, int num)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < num; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "PedestrianDetector", item[i].byPedestrianDetectorNumber);

        item[i].byPedestrianDetectorNumber = get_one_value(section,"byPedestrianDetectorNumber");
        item[i].byPedestrianDetectorCallPhase = get_one_value(section,"byPedestrianDetectorCallPhase");
        item[i].byPedestrianDetectorNoActivity = get_one_value(section,"byPedestrianDetectorNoActivity");
        item[i].byPedestrianDetectorMaxPresence = get_one_value(section,"byPedestrianDetectorMaxPresence");
        item[i].byPedestrianDetectorErraticCounts = get_one_value(section,"byPedestrianDetectorErraticCounts");
        
	}
}


