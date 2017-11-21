#include <unistd.h>
#include "HikConfig.h"
#include "parse_ini.h"

#define MAX_ARRAY_LEN	64
#define MAX_SECTION_LEN	64

#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)


//#define PRINT_ARGS      //��ӡ��Ϣ����

/*****************************************************************************
 �� �� ��  : StoreBitValueToArray
 ��������  : ����ֵvalue�дӵ�1bit��ʼ��bitֵ����1��bitλ�ô��뵽������
 �������  : const unsigned int value  ��ֵ
             const int num             Ҫ�洢�����bitλ��
             unsigned char *array      ����
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int StoreBitValueToArray(const unsigned int value, 
									   const int num,
									   unsigned char *array)
{
	int i, v = 0;
	
	memset(array, 0, MAX_ARRAY_LEN);
	for (i = 1; i <= num; i++) 
	{
		if (BIT(value, i) == 0)
			continue;
		array[v++] = (unsigned char)i;
	}
	return v;
}

/*****************************************************************************
 �� �� ��  : StoreVaildValueToArray
 ��������  : ������values�в�����0����ֵ���뵽��һ������array��
 �������  : const unsigned char *values  ԭʼ����
             const int num                ԭʼ����Ԫ�صĸ���
             unsigned char *array         Ҫ�������ݵ�����
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline int StoreVaildValueToArray(const unsigned char *values,
										 const int num,
										 unsigned char *array)
{
	int i, v = 0;
	
	memset(array, 0, MAX_ARRAY_LEN);
	for (i = 0; i < num; i++) 
	{
		if (values[i] == 0)
			continue;
		array[v++] = values[i];
	}
	return v;
}

/*****************************************************************************
 �� �� ��  : WritePlanSchedule
 ��������  : д���ȱ������ļ���ÿ��д��num���ʹ��֮ǰҪ����ini�������-
             parse_start��ʹ�����Ҫ����parse_end
 �������  : PPlanScheduleItem item  
             int num                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WritePlanSchedule(PPlanScheduleItem item)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	
	for(i = 0; i < NUM_SCHEDULE; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "PlanScheduleItem", item[i].nScheduleID);
		if (item[i].nTimeIntervalID == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nScheduleID", item[i].nScheduleID);
		add_one_key(section, "nTimeIntervalID", item[i].nTimeIntervalID);
		n = StoreBitValueToArray(item[i].month, 12, array);
		add_more_key(section, "month", array, n);
		n = StoreBitValueToArray(item[i].day, 31, array);
		add_more_key(section, "day", array, n);
		n = StoreBitValueToArray(item[i].week, 7, array);
		add_more_key(section, "week", array, n);

#ifdef PRINT_ARGS
        if(item[i].nTimeIntervalID != 0)
            DEG("%s  stPlanSchedule  nScheduleID  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,item[i].nScheduleID,
                                                                                     item[i].nTimeIntervalID,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif

	}
}

/*****************************************************************************
 �� �� ��  : WriteTimeIntervalItem
 ��������  : дʱ�α������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : TimeIntervalItem *item  
             int num                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteTimeIntervalItem(TimeIntervalItem (*table)[NUM_TIME_INTERVAL_ID])
{
	int i,j;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < NUM_TIME_INTERVAL; i++)
	{
        for(j = 0; j < NUM_TIME_INTERVAL_ID; j++)
        {
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "TimeIntervalItem", table[i][j].nTimeIntervalID, table[i][j].nTimeID);
    		if (table[i][j].nActionID == 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "nTimeIntervalID", table[i][j].nTimeIntervalID);
    		add_one_key(section, "nTimeID", table[i][j].nTimeID);
    		add_one_key(section, "cStartTimeHour", table[i][j].cStartTimeHour);
    		add_one_key(section, "cStartTimeMinute", table[i][j].cStartTimeMinute);
    		add_one_key(section, "nActionID", table[i][j].nActionID);
#ifdef PRINT_ARGS
            if(table[i][j].nActionID != 0)
                DEG("%s  stTimeInterval  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , nActionID  %d   \n",__func__,
                                    table[i][j].nTimeIntervalID,
                                    table[i][j].nTimeID,
                                    table[i][j].cStartTimeHour,
                                    table[i][j].cStartTimeMinute,
                                    table[i][j].nActionID);
#endif      
        }
	}
}

/*****************************************************************************
 �� �� ��  : WriteActionItem
 ��������  : д�����������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : PActionItem item  
             int num           
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteActionItem(PActionItem item)
{
	int i;
	char section[MAX_SECTION_LEN];
	int __attribute__((unused)) n;
	unsigned char __attribute__((unused)) array[MAX_ARRAY_LEN];
	
	for(i = 0; i < NUM_ACTION; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "ActionItem", item[i].nActionID);
		if (item[i].nSchemeID == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nActionID", item[i].nActionID);
		add_one_key(section, "nSchemeID", item[i].nSchemeID);
#ifdef USE_OTHER_OPTION
		add_more_key(section, "cAuxiliary", item[i].cAuxiliary);
		add_more_key(section, "cSpecialFun", item[i].cSpecialFun);
#endif

#ifdef PRINT_ARGS
        if(item[i].nSchemeID != 0)
         DEG("%s  stAction  nActionID  %d  , nSchemeID  %d   \n",__func__,
                        item[i].nActionID,
                        item[i].nSchemeID);
#endif                        
	}
}

/*****************************************************************************
 �� �� ��  : WriteSchemeItem
 ��������  : д�����������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : PSchemeItem item  
             int num           
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteSchemeItem(PSchemeItem item)
{
	int i;
	char section[MAX_SECTION_LEN];
	 
	for(i = 0; i < NUM_SCHEME; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "SchemeItem", item[i].nSchemeID);
		if ((item[i].nGreenSignalRatioID == 0) || (item[i].nPhaseTurnID == 0))
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nSchemeID", item[i].nSchemeID);
		add_one_key(section, "nCycleTime", item[i].nCycleTime);
		add_one_key(section, "nOffset", item[i].nOffset);
		add_one_key(section, "nGreenSignalRatioID", item[i].nGreenSignalRatioID);
		add_one_key(section, "nPhaseTurnID", item[i].nPhaseTurnID);

#ifdef PRINT_ARGS
        if(item[i].nGreenSignalRatioID != 0)
            DEG("%s  stScheme  nSchemeID  %d  , nCycleTime  %d  , nGreenSignalRatioID  %d  , nPhaseTurnID  %d   \n",__func__,
                        item[i].nSchemeID,
                        item[i].nCycleTime,
                        item[i].nGreenSignalRatioID,
                        item[i].nPhaseTurnID);
#endif
	}
}

/*****************************************************************************
 �� �� ��  : WriteChannelItem
 ��������  : дͨ���������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : PChannelItem item  
             int num            
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteChannelItem(PChannelItem item)
{
	int i;
	char section[MAX_SECTION_LEN];

	for(i = 0; i < NUM_CHANNEL; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "ChannelItem", item[i].nChannelID);
		if(item[i].nControllerID == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nChannelID", item[i].nChannelID);
		add_one_key(section, "nControllerID", item[i].nControllerID);
		add_one_key(section, "nControllerType", item[i].nControllerType);
#ifdef USE_OTHER_OPTION
		add_one_key(section, "nFlashLightType", item[i].nFlashLightType);
		add_one_key(section, "byChannelDim", item[i].byChannelDim);
#endif
#ifdef PRINT_ARGS
        if(item[i].nControllerID != 0)
            DEG("%s  stChannel  nChannelID  %d  , nControllerID  %d  , nControllerType  %d    \n",__func__,
                        item[i].nChannelID,
                        item[i].nControllerID,
                        item[i].nControllerType);
#endif
	}
}

/*****************************************************************************
 �� �� ��  : WriteGreenSignalRationItem
 ��������  : д���űȱ������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini����
             ���parse_start������֮��Ӧ����parse_end
 �������  : GreenSignalRationItem *item  
             int num                      
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���ռ����ķ�ʽ�޸��β�
*****************************************************************************/
void WriteGreenSignalRationItem(GreenSignalRationItem (*table)[NUM_PHASE])
{
	int i = 0;
    int j = 0;
	char section[MAX_SECTION_LEN];

    for(i = 0; i < NUM_GREEN_SIGNAL_RATION; i++)
    {
        for(j = 0; j < NUM_PHASE; j++)
        {
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "GreenSignalRationItem", table[i][j].nGreenSignalRationID, table[i][j].nPhaseID);
    		if (table[i][j].nGreenSignalRationTime == 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "nGreenSignalRationID", table[i][j].nGreenSignalRationID);
    		add_one_key(section, "nPhaseID", table[i][j].nPhaseID);
    		add_one_key(section, "nGreenSignalRationTime", table[i][j].nGreenSignalRationTime);
    		add_one_key(section, "nIsCoordinate", table[i][j].nIsCoordinate);
#ifdef USE_OTHER_OPTION
    		add_one_key(section, "nType", table[i][j].nType);
#endif

#ifdef PRINT_ARGS
            if(table[i][j].nPhaseID != 0)
             DEG("%s  stGreenSignalRation  nGreenSignalRationID  %d  , nPhaseID  %d , nGreenSignalRationTime  %d \n",__func__,
                                    table[i][j].nGreenSignalRationID,
                                    table[i][j].nPhaseID,
                                    table[i][j].nGreenSignalRationTime);
#endif

        }
    }
}

/*****************************************************************************
 �� �� ��  : WritePhaseItem
 ��������  : д��λ�������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : PPhaseItem item                      
             struct STRU_SignalTransEntry *entry  ��������ʱ��Ľṹ��ָ��
             int num                              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����С�̵����������ı���

  3.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : �ж���λ�Ƿ�Ϊ�յķ�����Ϊ��λ�Ƿ�ʹ�ܣ�ֻ����λʹ�ܺ��д
                 ���ļ��С�
*****************************************************************************/
void WritePhaseItem(PPhaseItem item, struct STRU_SignalTransEntry *entry)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	
	for(i = 0; i < NUM_PHASE; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "PhaseItem", item[i].nPhaseID);
		if (IS_PHASE_INABLE(item[i].wPhaseOptions) == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nPhaseID", item[i].nPhaseID);
		add_one_key(section, "nCircleID", item[i].nCircleID);
		add_one_key(section, "nYellowTime", item[i].nYellowTime);
		add_one_key(section, "nAllRedTime", item[i].nAllRedTime);
		add_one_key(section, "nGreenLightTime", entry[i].nGreenLightTime);
		add_one_key(section, "nPedestrianPassTime", item[i].nPedestrianPassTime);
		add_one_key(section, "nPedestrianClearTime", item[i].nPedestrianClearTime);
		add_one_key(section, "wPhaseOptions", item[i].wPhaseOptions);
		//add_one_key(section, "cIsEnablePhase", item[i].cIsEnablePhase);
		//add_one_key(section, "cAutoPedestrianPass", item[i].cAutoPedestrianPass);
		n = StoreVaildValueToArray(item[i].byPhaseConcurrency, NUM_PHASE, array);
		add_more_key(section, "byPhaseConcurrency", array, n);
		
		add_one_key(section, "nMinGreen", item[i].nMinGreen);
		add_one_key(section, "nMaxGreen_1", item[i].nMaxGreen_1);
		add_one_key(section, "nMaxGreen_2", item[i].nMaxGreen_2);
		add_one_key(section, "nUnitExtendGreen", item[i].nUnitExtendGreen);
		add_one_key(section, "nRedProtectedTime", item[i].nRedProtectedTime);
		add_one_key(section, "byPhaseStartup", item[i].byPhaseStartup);

#ifdef USE_OTHER_OPTION
		add_one_key(section, "byPhaseAddedInitial", item[i].byPhaseAddedInitial);
		add_one_key(section, "byPhaseMaximumInitial", item[i].byPhaseMaximumInitial);
		add_one_key(section, "byPhaseTimeBeforeReduction", item[i].byPhaseTimeBeforeReduction);
		add_one_key(section, "byPhaseCarsBeforeReduction", item[i].byPhaseCarsBeforeReduction);
		add_one_key(section, "byPhaseTimeToReduce", item[i].byPhaseTimeToReduce);
		add_one_key(section, "byPhaseReduceBy", item[i].byPhaseReduceBy);
		add_one_key(section, "byPhaseMinimumGap", item[i].byPhaseMinimumGap);
		add_one_key(section, "byPhaseDynamicMaxLimit", item[i].byPhaseDynamicMaxLimit);
		add_one_key(section, "byPhaseDynamicMaxStep", item[i].byPhaseDynamicMaxStep);
#endif

#ifdef PRINT_ARGS
        if(IS_PHASE_INABLE(item[i].wPhaseOptions) == 1)
            DEG("%s  stPhase  nPhaseID  %d  , nCircleID  %d  , nYellowTime  %d  , nAllRedTime  %d  , nGreenLightTime  %d, cIsEnablePhase  %d , nPedestrianPassTime  %d ,nPedestrianClearTime  %d\n",__func__,
                        item[i].nPhaseID,
                        item[i].nCircleID,
                        item[i].nYellowTime,
                        item[i].nAllRedTime,
                        entry->nGreenLightTime,
                        item[i].wPhaseOptions&0x01,
                        item[i].nPedestrianPassTime,
                        item[i].nPedestrianClearTime);
#endif

	}
}

/*****************************************************************************
 �� �� ��  : WriteUnitPara
 ��������  : д��Ԫ�����������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini����
             ���parse_start������֮��Ӧ����parse_end
 �������  : PUnitPara item  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���������ɼ����ڼ��������ڵĴ洢
*****************************************************************************/
void WriteUnitPara(PUnitPara item)
{
	char section[MAX_SECTION_LEN] = "UnitPara";
	
	add_one_key(section, "nBootYellowLightTime", item->nBootYellowLightTime);
	add_one_key(section, "nBootAllRedTime", item->nBootAllRedTime);
	add_one_key(section, "byFluxCollectCycle", item->byFluxCollectCycle);//�����ɼ�����
	add_one_key(section, "byTransCycle", item->byTransCycle);//��������
    add_one_key(section, "byCollectCycleUnit", item->byCollectCycleUnit);
	
#ifdef USE_OTHER_OPTION
	add_one_key(section, "cIsPedestrianAutoClear", item->cIsPedestrianAutoClear);
	add_one_key(section, "nDemotionTime", item->nDemotionTime);
	add_one_key(section, "nMinRedTime", item->nMinRedTime);
	add_one_key(section, "nLightFreq", item->nLightFreq);
	add_one_key(section, "nGatherCycle", item->nGatherCycle);
	add_one_key(section, "nTransitCycle", item->nTransitCycle);
	add_one_key(section, "wUnitBackupTime", item->wUnitBackupTime);
	add_one_key(section, "byUnitRedRevert", item->byUnitRedRevert);
	add_one_key(section, "byUnitControl", item->byUnitControl);
	add_one_key(section, "byCoordOperationalMode", item->byCoordOperationalMode);
	add_one_key(section, "byCoordCorrectionMode", item->byCoordCorrectionMode);
	add_one_key(section, "byCoordMaximumMode", item->byCoordMaximumMode);
	add_one_key(section, "byCoordForceMode", item->byCoordForceMode);
	add_one_key(section, "byFlashFrequency", item->byFlashFrequency);
	add_one_key(section, "byThroughStreetTimeGap", item->byThroughStreetTimeGap);
	add_one_key(section, "bySecondTimeDiff", item->bySecondTimeDiff);
	add_one_key(section, "byCollectCycleUnit", item->byCollectCycleUnit);
	add_one_key(section, "byUseStartOrder", item->byUseStartOrder);
	add_one_key(section, "byCommOutTime", item->byCommOutTime);
	add_one_key(section, "wSpeedCoef", item->wSpeedCoef);
	add_more_key(section, "acIpAddr", item->acIpAddr, 4);
	add_more_key(section, "acGatwayIp", item->acGatwayIp, 4);
	add_one_key(section, "SubNetMask", item->SubNetMask);
	add_one_key(section, "byPort", item->byPort);
	add_one_key(section, "byOption", item->byOption);
	add_one_key(section, "byUnitTransIntensityCalCo", item->byUnitTransIntensityCalCo);
#endif
#ifdef PRINT_ARGS
    DEG("%s  stUnitPara nBootAllRedTime  %d  , nBootYellowLightTime  %d byFluxCollectCycle  %d , byTransCycle %d\n",__func__,
                        item->nBootAllRedTime,
                        item->nBootYellowLightTime,
                        item->byFluxCollectCycle,
                        item->byTransCycle);
#endif

}

/*****************************************************************************
 �� �� ��  : WriteFollowPhaseItem
 ��������  : д������λ�������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini��
             �����parse_start������֮��Ӧ����parse_end
 �������  : PFollowPhaseItem item  
             int num                
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteFollowPhaseItem(PFollowPhaseItem item)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	 
	for(i = 0; i < NUM_FOLLOW_PHASE; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "FollowPhaseItem", item[i].nFollowPhaseID);
		if (item[i].nArrayMotherPhase[0] == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nFollowPhaseID", item[i].nFollowPhaseID);
		
		n = StoreVaildValueToArray(item[i].nArrayMotherPhase, NUM_PHASE, array);
		add_more_key(section, "nArrayMotherPhase", array, n);
#ifdef USE_OTHER_OPTION
		add_one_key(section, "nFixPhaseID", item[i].nFixPhaseID);
		add_one_key(section, "byOverlapType", item[i].byOverlapType);
		n = StoreVaildValueToArray(item[i].byArrOverlapModifierPhases, MAX_PHASE_COUNT_MO_OVERLAP, array);
		add_more_key(section, "byArrOverlapModifierPhases", array, n);
		add_one_key(section, "nGreenTime", item[i].nGreenTime);
		add_one_key(section, "nYellowTime", item[i].nYellowTime);
		add_one_key(section, "nRedTime", item[i].nRedTime);
#endif
#ifdef PRINT_ARGS
        if(item[i].nFollowPhaseID  != 0)
            DEG("%s  stFollowPhase  nFollowPhaseID  %d  , nArrayMotherPhase[0]  %d , nArrayMotherPhase[1]  %d  , nArrayMotherPhase[2]  %d  \n",__func__,
                        item[i].nFollowPhaseID,
                        item[i].nArrayMotherPhase[0],
                        item[i].nArrayMotherPhase[1],
                        item[i].nArrayMotherPhase[2]);

#endif


	}
}

/*****************************************************************************
 �� �� ��  : WritePhaseTurnItem
 ��������  : д����������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini������
             ��parse_start������֮��Ӧ����parse_end
 �������  : PhaseTurnItem *item  
             int num              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WritePhaseTurnItem(PhaseTurnItem (*table)[NUM_RING_COUNT])
{
	int i, n,j;
	char section[MAX_SECTION_LEN] = {0};
	unsigned char array[MAX_ARRAY_LEN] = {0};

    for(i = 0; i < NUM_PHASE_TURN; i++)
    {
    	for(j = 0; j < NUM_RING_COUNT; j++)
    	{
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "PhaseTurnItem", table[i][j].nPhaseTurnID, table[i][j].nCircleID);
    		if (table[i][j].nTurnArray[0] == 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "nPhaseTurnID", table[i][j].nPhaseTurnID);
    		add_one_key(section, "nCircleID", table[i][j].nCircleID);
    		
    		n = StoreVaildValueToArray(table[i][j].nTurnArray, NUM_PHASE, array);
    		add_more_key(section, "nTurnArray", array, n);

#ifdef PRINT_ARGS
            if(table[i][j].nCircleID != 0)
             DEG("%s  stPhaseTurn  nPhaseTurnID  %d  , nCircleID  %d ,  nTurnArray[0]  %d  , nTurnArray[1]  %d  , nTurnArray[2]  %d  , nTurnArray[3]  %d  \n",__func__,
                            table[i][j].nPhaseTurnID,
                            table[i][j].nCircleID ,
                            table[i][j].nTurnArray[0],
                            table[i][j].nTurnArray[1],
                            table[i][j].nTurnArray[2],
                            table[i][j].nTurnArray[3]);
#endif          

    }

	
              

	}
}

/*****************************************************************************
 �� �� ��  : WriteVehicleDetector
 ��������  : д����������������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini
             �������parse_start������֮��Ӧ����parse_end
 �������  : struct STRU_N_VehicleDetector *item  
             int num                              
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteVehicleDetector(struct STRU_N_VehicleDetector *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "VehicleDetector", item[i].byVehicleDetectorNumber);
		if ((item[i].byVehicleDetectorCallPhase == 0) || (item[i].byVehicleDetectorSwitchPhase == 0))
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "byVehicleDetectorNumber", item[i].byVehicleDetectorNumber);
		add_one_key(section, "byVehicleDetectorCallPhase", item[i].byVehicleDetectorCallPhase);
		add_one_key(section, "byVehicleDetectorOptions", item[i].byVehicleDetectorOptions);
		add_one_key(section, "byVehicleDetectorSwitchPhase", item[i].byVehicleDetectorSwitchPhase);
		add_one_key(section, "byVehicleDetectorDelay", item[i].byVehicleDetectorDelay);
		add_one_key(section, "byVehicleDetectorExtend", item[i].byVehicleDetectorExtend);
		add_one_key(section, "byVehicleDetectorQueueLimit", item[i].byVehicleDetectorQueueLimit);
		add_one_key(section, "byVehicleDetectorNoActivity", item[i].byVehicleDetectorNoActivity);
		add_one_key(section, "byVehicleDetectorMaxPresence", item[i].byVehicleDetectorMaxPresence);
		add_one_key(section, "byVehicleDetectorErraticCounts", item[i].byVehicleDetectorErraticCounts);
		add_one_key(section, "byVehicleDetectorFailTime", item[i].byVehicleDetectorFailTime);
		add_one_key(section, "byVehicleDetectorAlarms", item[i].byVehicleDetectorAlarms);
	}
}

/*****************************************************************************
 �� �� ��  : WritePedestrianDetector
 ��������  : д���˼�����������ļ���ÿ��д��num�������֮ǰӦ�ȵ���ini
             �������parse_start������֮��Ӧ����parse_end
 �������  : struct STRU_N_PedestrianDetector *item  
             int num                                 
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void WritePedestrianDetector(struct STRU_N_PedestrianDetector *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_PEDESTRIANDETECTOR_COUNT; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "PedestrianDetector", item[i].byPedestrianDetectorNumber);
		if (item[i].byPedestrianDetectorCallPhase == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "byPedestrianDetectorNumber", item[i].byPedestrianDetectorNumber);
		add_one_key(section, "byPedestrianDetectorCallPhase", item[i].byPedestrianDetectorCallPhase);
		add_one_key(section, "byPedestrianDetectorNoActivity", item[i].byPedestrianDetectorNoActivity);
		add_one_key(section, "byPedestrianDetectorMaxPresence", item[i].byPedestrianDetectorMaxPresence);
		add_one_key(section, "byPedestrianDetectorErraticCounts", item[i].byPedestrianDetectorErraticCounts);
	}
}

/*****************************************************************************
 �� �� ��  : WriteConfigFile
 ��������  : д���е�����������ļ��������ٵ���parse_start��parse_end
 �������  : SignalControllerPara *param  �ܲ����ṹ��ָ��
             const char *path             Ҫд��������ļ�·���������ָ��Ĭ��Ϊ/config.ini
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��5��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
Boolean WriteConfigFile(SignalControllerPara *param, const char *path)
{
	const char *filename = (path == NULL) ? CFG_NAME : path;
	
	unlink(filename);	//ɾ��֮ǰ�������ļ�����ΪҪ����д��
	if (!parse_start(filename))
		return FALSE;

	WriteUnitPara(&param->stUnitPara);	//д��Ԫ����

	WritePhaseItem(param->stPhase, param->AscSignalTransTable);	//д��λ��

	WriteChannelItem(param->stChannel);	//дͨ����

	WriteGreenSignalRationItem(param->stGreenSignalRation);	//д���űȱ�
	
	WritePhaseTurnItem(param->stPhaseTurn);	//д�����
	
	WriteSchemeItem(param->stScheme);	//д������

	WriteActionItem(param->stAction);	//д������

	WriteTimeIntervalItem(param->stTimeInterval);	//дʱ���
	
	WritePlanSchedule(param->stPlanSchedule);	//д���ȱ�

	WriteFollowPhaseItem(param->stFollowPhase);	//д������λ��

    WriteVehicleDetector(param->AscVehicleDetectorTable);//д���������
    
    WritePedestrianDetector(param->AscPedestrianDetectorTable);//д���˼����
	
	parse_end();

    return TRUE;
}

