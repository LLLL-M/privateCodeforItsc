#include <unistd.h>
#include "ykconfig.h"
#include "parse_ini.h"

#define MAX_ARRAY_LEN	64
#define MAX_SECTION_LEN	64

#define DEG(fmt,...) fprintf(stderr,"ykConfig library debug : "fmt "\n",##__VA_ARGS__)

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
		if (num == 7)
		{	//��value��ʾ������ڵ���ֵʱ��bit1-7�ֱ��ʾ�����յ�������������������һ��������
			array[v++] = (i == 1) ? 7 : (i - 1);
		}
		else
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

static void WritePlanSchedule(YK_PlanScheduleItem *item)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	
	for(i = 0; i < NUM_SCHEDULE; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		memset(array,0,sizeof(array));
		sprintf(section, "%s%d", "PlanScheduleItem", item[i].nScheduleID);
		if (item[i].nTimeIntervalID == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nScheduleID", item[i].nScheduleID);
		add_one_key(section, "timeIntervalListNo", item[i].nTimeIntervalID);
		n = StoreBitValueToArray(item[i].month, 12, array);
		add_more_key(section, "month", array, n);
		n = StoreBitValueToArray(item[i].day, 31, array);
		add_more_key(section, "day", array, n);
		n = StoreBitValueToArray(item[i].week, 7, array);
		add_more_key(section, "week", array, n);

#ifdef PRINT_ARGS
        if(item[i].timeIntervalListNo != 0)
            DEG("%s  stPlanSchedule  nScheduleID  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,item[i].scheduleNo,
                                                                                     item[i].nScheduleID,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif

	}
}

static void WriteTimeIntervalItem(YK_TimeIntervalItem (*table)[NUM_TIME_INTERVAL_ID])
{
	int i,j;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < NUM_TIME_INTERVAL; i++)
	{
        for(j = 0; j < NUM_TIME_INTERVAL_ID; j++)
        {
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "TimeIntervalItem", table[i][j].nTimeIntervalID, table[i][j].nTimeID);
    		if (table[i][j].nSchemeId == 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "nTimeIntervalID", table[i][j].nTimeIntervalID);
    		add_one_key(section, "nTimeID", table[i][j].nTimeID);
    		add_one_key(section, "cStartTimeHour", table[i][j].cStartTimeHour);
    		add_one_key(section, "cStartTimeMinute", table[i][j].cStartTimeMinute);
    		add_one_key(section, "nSchemeId", table[i][j].nSchemeId);
    		add_one_key(section, "phaseOffset", table[i][j].phaseOffset);
			add_one_key(section, "IsCorrdinateCtl", table[i][j].IsCorrdinateCtl);
#ifdef PRINT_ARGS
            if(table[i][j].schemeId != 0)
                DEG("%s  stTimeInterval  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , nActionID  %d   \n",__func__,
                                    table[i][j].nTimeIntervalID,
                                    table[i][j].nTimeID,
                                    table[i][j].cStartTimeHour,
                                    table[i][j].cStartTimeMinute,
                                    table[i][j].nSchemeId);
#endif      
        }
	}
}

static void WriteSchemeItem(YK_SchemeItem *item)
{
	int i;
	int j = 0;
	int n = 0;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN] = {0};
	 
	for(i = 0; i < NUM_SCHEME; i++)
	{
		//DEG("---->    %d  \n",i+1);
		for(j = 0; j < NUM_PHASE; j++)
		{
			memset(section, 0, MAX_SECTION_LEN); 
			sprintf(section, "%s%d_%d", "SchemeItem", i+1,j+1);
			if (item[i].cycleTime == 0 || item[i].phaseInfo[j].channelBits == 0)
			{
				del_section(section);
				continue;
			}
			add_one_key(section, "nSchemeId", item[i].nSchemeId);
			add_one_key(section, "cycleTime", item[i].cycleTime);
			add_one_key(section, "totalPhaseNum", item[i].totalPhaseNum);	
			
			add_one_key(section, "greenTime", item[i].phaseInfo[j].greenTime);
			add_one_key(section, "greenBlinkTime", item[i].phaseInfo[j].greenBlinkTime);	
			add_one_key(section, "yellowTime", item[i].phaseInfo[j].yellowTime);
			add_one_key(section, "redYellowTime", item[i].phaseInfo[j].redYellowTime);	
			add_one_key(section, "allRedTime", item[i].phaseInfo[j].allRedTime);
			add_one_key(section, "minGreenTime", item[i].phaseInfo[j].minGreenTime);				
			add_one_key(section, "maxGreenTime", item[i].phaseInfo[j].maxGreenTime);
			add_one_key(section, "unitExtendTime", item[i].phaseInfo[j].unitExtendTime);		
			n = StoreBitValueToArray(item[i].phaseInfo[j].channelBits, 32, array);
			add_more_key(section, "channelBits", array, n);
		}
	}
}

static void WriteChannelItem(YK_ChannelItem *item)
{
	int i;
	int n = 0;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN] = {0};
	for(i = 0; i < NUM_CHANNEL; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "ChannelItem", item[i].nChannelID);
		if(item[i].nControllerType == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "nChannelID", item[i].nChannelID);
		add_one_key(section, "nControllerType", item[i].nControllerType);
		add_one_key(section, "nVehDetectorNum", item[i].nVehDetectorNum);
		n = StoreBitValueToArray(item[i].conflictChannel, 32, array);
		add_more_key(section, "conflictChannel", array, n);
	}
}

static void WriteUnitPara(YK_WholeConfig *item)
{
	char section[MAX_SECTION_LEN] = "UnitPara";
	
	add_one_key(section, "bootYellowBlinkTime", item->bootYellowBlinkTime);
	add_one_key(section, "bootAllRedTime", item->bootAllRedTime);
	add_one_key(section, "vehFlowCollectCycleTime", item->vehFlowCollectCycleTime);//�����ɼ�����

	add_one_key(section, "transitionCycle", item->transitionCycle);
	add_one_key(section, "adaptCtlEndRunCycleNum", item->adaptCtlEndRunCycleNum);
	add_one_key(section, "watchdogEnable", item->watchdogEnable);
	add_one_key(section, "signalMachineType", item->signalMachineType);
	add_one_key(section, "weekPreemptSchedule", item->weekPreemptSchedule);
}



Boolean WriteykConfigFile(YK_Config *param, const char *path)
{
	const char *filename = (path == NULL) ? "/home/ykconfig.ini" : path;
	
	unlink(filename);	//ɾ��֮ǰ�������ļ�����ΪҪ����д��
	if (!parse_start(filename))
		return FALSE;

	WriteUnitPara(&param->wholeConfig);	//д��Ԫ����

	WriteChannelItem(param->channelTable);	//дͨ����

	WriteSchemeItem(param->schemeTable);	//д������

	WriteTimeIntervalItem(param->timeIntervalTable);	//дʱ���
	
	WritePlanSchedule(param->scheduleTable);	//д���ȱ�
	
	parse_end();

    return TRUE;
}

