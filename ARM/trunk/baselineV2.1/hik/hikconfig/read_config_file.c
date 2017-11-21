#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "HikConfig.h"
//#include "parse_ini.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

#define DEG(fmt,...) fprintf(stderr,"HikConfig library debug : "fmt "\n",##__VA_ARGS__)

#define MAX_SECTION_LEN	64

sqlite3* g_pdatabase;

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
    sqlite3_select_unit(g_pdatabase, item);
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
void ReadPhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE])
{
  sqlite3_select_phase(g_pdatabase, item);
  sqlite3_select_signal_trans(g_pdatabase, entry);
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

void ReadChannelItem(ChannelItem channel[][NUM_CHANNEL])
{
 	sqlite3_select_channel(g_pdatabase, channel);
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
void ReadGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE])
{
	sqlite3_select_green_split(g_pdatabase, green_split);
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
void ReadPhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT])
{
	sqlite3_select_phase_turn(g_pdatabase, phase_turn);	

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
void ReadSchemeItem(SchemeItem *scheme)
{
 	sqlite3_select_scheme(g_pdatabase, scheme);
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
void ReadActionItem(ActionItem *action)
{
	sqlite3_select_action(g_pdatabase, action); 
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
void ReadTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID])
{
 	sqlite3_select_timeinterval(g_pdatabase, time_interval);
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
void ReadPlanSchedule(PlanScheduleItem *schedule)
{
	sqlite3_select_schedule(g_pdatabase, schedule);
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
void ReadFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE])
{
	sqlite3_select_follow_phase(g_pdatabase, follow_phase); 
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
void ReadVehicleDetector(struct STRU_N_VehicleDetector *vehicle)
{
	sqlite3_select_vehicle(g_pdatabase, vehicle);
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

}

/*****************************************************************************
 �� �� ��  : LoadDataFromCfg
 ��������  : �������ļ��м����������ݵ��ڴ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : unsigned
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��31��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara)
{
    SignalControllerPara zero;
	
	if ((NULL == pSignalControlpara))
    {
        return FALSE;
    }
	sqlite3_open_wrapper(DATABASE_HIKCONFIG, &g_pdatabase);
	memset(pSignalControlpara, 0, sizeof(SignalControllerPara));

    //���ص�Ԫ���� 
    ReadUnitPara(&(pSignalControlpara->stUnitPara));

    //������λ���� 
    ReadPhaseItem(pSignalControlpara->stPhase,pSignalControlpara->AscSignalTransTable);
    

    //����ͨ������ 
    ReadChannelItem(pSignalControlpara->stChannel);
    

    //�������űȲ��� 
    ReadGreenSignalRationItem(pSignalControlpara->stGreenSignalRation);

    
    //To Do  �����
    ReadPhaseTurnItem(pSignalControlpara->stPhaseTurn);
    

    //���ط�������� 
    ReadSchemeItem(pSignalControlpara->stScheme);

    //���ض��������
    ReadActionItem(pSignalControlpara->stAction);

    //����ʱ�α����
    ReadTimeIntervalItem(pSignalControlpara->stTimeInterval);
    
    
    //���ص��Ȳ���
    ReadPlanSchedule(pSignalControlpara->stPlanSchedule);

    //���ظ������
    ReadFollowPhaseItem(pSignalControlpara->stFollowPhase);

    //
    ReadVehicleDetector(pSignalControlpara->AscVehicleDetectorTable);

    //
    //ReadPedestrianDetector(pSignalControlpara->AscPedestrianDetectorTable,MAX_PEDESTRIANDETECTOR_COUNT);

	sqlite3_close_wrapper(g_pdatabase);
	g_pdatabase = NULL;
	
	memset(&zero, 0, sizeof(SignalControllerPara));
	if (memcmp(pSignalControlpara, &zero, sizeof(SignalControllerPara)) == 0) {	//˵��û��������Ϣ
		//log_error("config file is not exist or there is no  config information in file . \n");
		return FALSE;
	}	
	
	return TRUE;
}


