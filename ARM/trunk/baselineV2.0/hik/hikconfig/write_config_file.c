#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include "HikConfig.h"
//#include "parse_ini.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

#define MAX_ARRAY_LEN	64
#define MAX_SECTION_LEN	64

#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)


extern sqlite3* g_pdatabase;

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
void WritePlanSchedule(PlanScheduleItem *schedule)
{
	sqlite3_insert_schedule(g_pdatabase, schedule);
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
void WriteTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID])
{
	sqlite3_insert_timeinterval(g_pdatabase, time_interval);
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
void WriteActionItem(ActionItem *action)
{
	sqlite3_insert_action(g_pdatabase, action); 
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
void WriteSchemeItem(SchemeItem *scheme)
{
	sqlite3_insert_scheme(g_pdatabase, scheme);
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
void WriteChannelItem(ChannelItem channel[][NUM_CHANNEL])
{
 	sqlite3_insert_channel(g_pdatabase, channel);

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
void WriteGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE])
{
	sqlite3_insert_green_split(g_pdatabase, green_split);
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
void WritePhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE])
{
	sqlite3_insert_phase(g_pdatabase, item);
  	sqlite3_insert_signal_trans(g_pdatabase, entry);
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
	sqlite3_insert_unit(g_pdatabase, item);
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
void WriteFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE])
{
	sqlite3_insert_follow_phase(g_pdatabase, follow_phase); 
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
void WritePhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT])
{
	sqlite3_insert_phase_turn(g_pdatabase, phase_turn);
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
void WriteVehicleDetector(struct STRU_N_VehicleDetector *vehicle)
{
	sqlite3_insert_vehicle(g_pdatabase, vehicle);
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
Boolean WriteConfigFile(SignalControllerPara *param)
{
	sqlite3_open_wrapper(DATABASE_HIKCONFIG, &g_pdatabase);
	//unlink(filename);	//ɾ��֮ǰ�������ļ�����ΪҪ����д��
	
	WriteUnitPara(&(param->stUnitPara));	//д��Ԫ����

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

	sqlite3_close_wrapper(g_pdatabase);
	g_pdatabase = NULL;

    return TRUE;
}

