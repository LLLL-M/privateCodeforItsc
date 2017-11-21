#include <unistd.h>
#include "gbConfig.h"
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

static void WritePlanSchedule(GbScheduleList *item)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	
	for(i = 0; i < MAX_SCHEDULE_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "PlanScheduleItem", item[i].scheduleNo);
		if (item[i].timeIntervalListNo == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "scheduleNo", item[i].scheduleNo);
		add_one_key(section, "timeIntervalListNo", item[i].timeIntervalListNo);
		n = StoreBitValueToArray(item[i].month, 12, array);
		add_more_key(section, "month", array, n);
		n = StoreBitValueToArray(item[i].day, 31, array);
		add_more_key(section, "day", array, n);
		n = StoreBitValueToArray(item[i].week, 7, array);
		add_more_key(section, "week", array, n);

#ifdef PRINT_ARGS
        if(item[i].timeIntervalListNo != 0)
            DEG("%s  stPlanSchedule  nScheduleID  %d  , nTimeIntervalID  %d   Month  0x%x, Day 0x%x, Week  0x%x\n",__func__,item[i].scheduleNo,
                                                                                     item[i].timeIntervalListNo,
                                                                                     item[i].month,
                                                                                     item[i].day,item[i].week);
#endif

	}
}

static void WriteTimeIntervalItem(GbTimeIntervalList (*table)[MAX_TIMEINTERVAL_NUM])
{
	int i,j;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_TIMEINTERVAL_LIST_NUM; i++)
	{
        for(j = 0; j < MAX_TIMEINTERVAL_NUM; j++)
        {
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "TimeIntervalItem", table[i][j].timeIntervalListNo, table[i][j].timeIntervalNo);
    		if (table[i][j].schemeId == 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "timeIntervalListNo", table[i][j].timeIntervalListNo);
    		add_one_key(section, "timeIntervalNo", table[i][j].timeIntervalNo);
    		add_one_key(section, "hour", table[i][j].hour);
    		add_one_key(section, "minute", table[i][j].minute);
    		add_one_key(section, "schemeId", table[i][j].schemeId);
    		add_one_key(section, "controlMode", table[i][j].controlMode);
#ifdef PRINT_ARGS
            if(table[i][j].schemeId != 0)
                DEG("%s  stTimeInterval  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , nActionID  %d   \n",__func__,
                                    table[i][j].timeIntervalListNo,
                                    table[i][j].timeIntervalNo,
                                    table[i][j].hour,
                                    table[i][j].minute,
                                    table[i][j].schemeId);
#endif      
        }
	}
}

static void WriteSchemeItem(GbSchemeList *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	 
	for(i = 0; i < MAX_SCHEME_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "SchemeItem", item[i].schemeNo);
		if (item[i].cycleTime == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "schemeNo", item[i].schemeNo);
		add_one_key(section, "cycleTime", item[i].cycleTime);
		add_one_key(section, "phaseGap", item[i].phaseGap);
		add_one_key(section, "stageTimingNo", item[i].stageTimingNo);
		add_one_key(section, "coordinatePhase", item[i].coordinatePhase);
	}
}

static void WriteChannelItem(GbChannelList *item)
{
	int i;
	char section[MAX_SECTION_LEN];

	for(i = 0; i < MAX_CHANNEL_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "ChannelItem", item[i].channelNo);
		if(item[i].channelRelatedPhase == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "channelNo", item[i].channelNo);
		add_one_key(section, "channelRelatedPhase", item[i].channelRelatedPhase);
		add_one_key(section, "channelControlType", item[i].channelControlType);
		add_one_key(section, "channelFlashStatus", item[i].channelFlashStatus);
	}
}
static void WritePhaseItem(GbPhaseList *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "PhaseItem", item[i].phaseNo);
		if (IS_PHASE_INABLE(item[i].phaseOption) == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "phaseNo", item[i].phaseNo);
		add_one_key(section, "greenBlinkTime", item[i].greenBlinkTime);
		add_one_key(section, "pedestrianPassTime", item[i].pedestrianPassTime);
		add_one_key(section, "pedestrianClearTime", item[i].pedestrianClearTime);
		add_one_key(section, "phaseOption", item[i].phaseOption);
		
		add_one_key(section, "minGreen", item[i].minGreen);
		add_one_key(section, "maxGreen_1", item[i].maxGreen_1);
		add_one_key(section, "maxGreen_2", item[i].maxGreen_2);
		add_one_key(section, "unitExtendGreen", item[i].unitExtendGreen);
		add_one_key(section, "phaseType", item[i].phaseType);
	}
}

static void WriteUnitPara(GbConfig *item)
{
	char section[MAX_SECTION_LEN] = "UnitPara";
	
	add_one_key(section, "bootBlinkTime", item->bootBlinkTime);
	add_one_key(section, "bootAllRedTime", item->bootAllRedTime);
	add_one_key(section, "flashFrequency", item->flashFrequency);//�����ɼ�����
}

static void WriteFollowPhaseItem(GbFollowPhaseList *item)
{
	int i, n;
	char section[MAX_SECTION_LEN];
	unsigned char array[MAX_ARRAY_LEN];
	 
	for(i = 0; i < MAX_FOLLOW_PHASE_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN); 
		sprintf(section, "%s%d", "FollowPhaseItem", item[i].followPhaseNo);
		if (item[i].motherPhase[0] == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "followPhaseNo", item[i].followPhaseNo);
		
		n = StoreVaildValueToArray(item[i].motherPhase, MAX_PHASE_LIST_NUM, array);
		add_more_key(section, "motherPhase", array, n);

		
	}
}

static void WriteVehicleDetector(GbVehDetectorList *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_VEH_DETECTOR_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "VehicleDetector", item[i].detectorNo);
		if (item[i].requestPhase == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "detectorNo", item[i].detectorNo);
		add_one_key(section, "requestPhase", item[i].requestPhase);
		add_one_key(section, "detectorOption", item[i].detectorOption);
	}
}


static void WriteStageTimingItem(GbStageTimingList (*table)[MAX_STAGE_NUM])
{
	int i,j;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_STAGE_TIMING_LIST_NUM; i++)
	{
        for(j = 0; j < MAX_STAGE_NUM; j++)
        {
    		memset(section, 0, MAX_SECTION_LEN);
    		sprintf(section, "%s%d_%d", "StageTimingItem", table[i][j].stageTimingNo, table[i][j].stageNo);
    		if (table[i][j].phaseNo== 0)
    		{
    			del_section(section);
    			continue;
    		}
    		add_one_key(section, "stageTimingNo", table[i][j].stageTimingNo);
    		add_one_key(section, "stageNo", table[i][j].stageNo);
    		add_one_key(section, "phaseNo", table[i][j].phaseNo);
    		add_one_key(section, "greenTime", table[i][j].greenTime);
    		add_one_key(section, "yellowTime", table[i][j].yellowTime);
    		add_one_key(section, "allRedTime", table[i][j].allRedTime);
        }
	}
}

static void WritePhaseConflict(GbPhaseConflictList *item)
{
	int i;
	char section[MAX_SECTION_LEN];
	
	for(i = 0; i < MAX_PHASE_LIST_NUM; i++)
	{
		memset(section, 0, MAX_SECTION_LEN);
		sprintf(section, "%s%d", "PhaseConflictItem", item[i].phaseConflictNo);
		if (item[i].conflictPhase == 0)
		{
			del_section(section);
			continue;
		}
		add_one_key(section, "phaseConflictNo", item[i].phaseConflictNo);
		add_one_key(section, "conflictPhase", item[i].conflictPhase);
	}
}


Boolean WriteGbConfigFile(GbConfig *param, const char *path)
{
	const char *filename = (path == NULL) ? CFG_NAME : path;
	
	unlink(filename);	//ɾ��֮ǰ�������ļ�����ΪҪ����д��
	if (!parse_start(filename))
		return FALSE;

	WriteUnitPara(param);	//д��Ԫ����

	WritePhaseItem(param->phaseTable);	//д��λ��

	WriteChannelItem(param->channelTable);	//дͨ����

	WriteSchemeItem(param->schemeTable);	//д������

	WriteTimeIntervalItem(param->timeIntervalTable);	//дʱ���
	
	WritePlanSchedule(param->scheduleTable);	//д���ȱ�

	WriteFollowPhaseItem(param->followPhaseTable);	//д������λ��

    WriteVehicleDetector(param->vehDetectorTable);//д���������
    
    WritePhaseConflict(param->phaseConflictTable);//д���˼����

    WriteStageTimingItem(param->stageTimingTable);
	
	parse_end();

    return TRUE;
}

