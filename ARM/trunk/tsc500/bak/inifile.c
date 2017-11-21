#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "common.h"
#include "hik.h"
#include "parse_ini.h"


#define MAX_FILE_SIZE 1024*512
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'

struct SPECIAL_PARAMS s_special_params;       //�����������
unsigned int g_failurenumber;                 //�����¼����
struct CURRENT_PARAMS g_struRecCurrent[32];   //��������
struct Count_Down_Params g_struCountDown;    //����ʱ����
PHASE_DESC_PARAMS phase_desc_params;        //��λ����  
CHANNEL_DESC_PARAMS channel_desc_params;   //ͨ������
PATTERN_NAME_PARAMS pattern_name_params;   //��������
PLAN_NAME_PARAMS plan_name_params;         //�ƻ�����
DATE_NAME_PARAMS date_name_params;		   //��������
COM_PARAMS com_params;                     //�������ò���
COM_PARAMS g_com_params[4];                //ȫ�ִ��ڲ���
CHANNEL_LOCK_PARAMS gChannelLockedParams;
UInt8 gChannelLockFlag = 0;   //0��ʾδ������1��ʾ����

#define FILE_HIK_CFG        "/home/config.ini"      //ȫ�ֲ��������ļ�������ͨ������λ����źŻ����б��������GPS���ص�������Ϣ���������кŵ���Ϣ
#define FILE_DESC           "/home/desc.ini"        //�������������ļ�������ͨ��������������������Ϣ
#define FILE_CUSTOM         "/home/custom.ini"      //������Ϣ�����ļ� ����������ʱ�����ڡ�����������


/*****************************************************************************
 �� �� ��  : get_special_params
 ��������  : �������ļ��л�ȡ�������
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ͳһ���ýӿڣ����ٶ�δ��ļ�

  3.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �������ļ���ȡ���������ӵ��ýӿ���

  4.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���������ӵ��ýӿ���
*****************************************************************************/
void get_special_params()
{
	parse_start(FILE_HIK_CFG);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //һЩ������
    s_special_params.iErrorDetectSwitch = get_one_value(section, "ErrorDetectSwitch");
	s_special_params.iCurrentAlarmSwitch = get_one_value(section, "CurrentAlarmSwitch");
	s_special_params.iVoltageAlarmSwitch = get_one_value(section, "VoltageAlarmSwitch");
	s_special_params.iCurrentAlarmAndProcessSwitch = get_one_value(section, "CurrentAlarmAndProcessSwitch");
	s_special_params.iVoltageAlarmAndProcessSwitch = get_one_value(section, "VoltageAlarmAndProcessSwitch");
	s_special_params.iWatchdogSwitch = get_one_value(section, "WatchdogSwitch");
	s_special_params.iGpsSwitch = get_one_value(section, "GpsSwitch");

    //���Ϻ�
    g_failurenumber = get_one_value("failureinfo", "FailureNumber");

    
	DBG("ErrorDetectSwitch=%d  CurrentAlarmSwitch=%d  VoltageAlarmSwitch=%d  CurrentAlarmAndProcessSwitch=%d  \n\t\t\t\t VoltageAlarmAndProcessSwitch=%d  WatchdogSwitch=%d  GpsSwitch=%d  SignalMachineType=%d\n",
		s_special_params.iErrorDetectSwitch,s_special_params.iCurrentAlarmSwitch,s_special_params.iVoltageAlarmSwitch,
		s_special_params.iCurrentAlarmAndProcessSwitch,s_special_params.iVoltageAlarmAndProcessSwitch,
		s_special_params.iWatchdogSwitch,s_special_params.iGpsSwitch,s_special_params.iSignalMachineType);
		
    //�������
    section = "currentparams";
	for(i=0;i<32;i++)
	{
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		g_struRecCurrent[i].RedCurrentBase  = get_one_value(section, tmpStr);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
		g_struRecCurrent[i].RedCurrentDiff  = get_one_value(section, tmpStr);

		if( (0 != g_struRecCurrent[i].RedCurrentDiff) && (0 != g_struRecCurrent[i].RedCurrentBase))
		{
    		DBG("ͨ��%02d����ƫ��ֵ:%03d ������׼ֵ:%03d \n",i+1, g_struRecCurrent[i].RedCurrentDiff, g_struRecCurrent[i].RedCurrentBase);
		}
	}


    parse_end();
}



/*****************************************************************************
 �� �� ��  : set_special_params
 ��������  : ���������д�������ļ�
 �������  : struct SPECIAL_PARAMS special_params  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_special_params(STRU_SPECIAL_PARAMS *pSpecial_params,STRU_CURRENT_PARAMS *pstruRecCurrent, int failurenumber)
{
	parse_start(FILE_HIK_CFG);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //һЩ������
	add_one_key(section, "ErrorDetectSwitch" ,pSpecial_params->iErrorDetectSwitch);
	add_one_key(section, "CurrentAlarmSwitch" ,pSpecial_params->iCurrentAlarmSwitch);
	add_one_key(section, "VoltageAlarmSwitch" ,pSpecial_params->iVoltageAlarmSwitch);
	add_one_key(section, "CurrentAlarmAndProcessSwitch" ,pSpecial_params->iCurrentAlarmAndProcessSwitch);
	add_one_key(section, "VoltageAlarmAndProcessSwitch" ,pSpecial_params->iVoltageAlarmAndProcessSwitch);
	add_one_key(section, "WatchdogSwitch" ,pSpecial_params->iWatchdogSwitch);
	add_one_key(section, "GpsSwitch" ,pSpecial_params->iGpsSwitch);

    //���Ϻ�
    add_one_key("failureinfo", "FailureNumber" ,failurenumber);

    //�������
    section = "currentparams";
	for(i=0;i<32;i++)
	{
	    if(0 == pstruRecCurrent[i].RedCurrentBase)
	    {
            continue;
	    }
	    
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		add_one_key(section, tmpStr ,pstruRecCurrent[i].RedCurrentBase);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
        add_one_key(section, tmpStr ,pstruRecCurrent[i].RedCurrentDiff);
	}
	
	parse_end();
}

/*********************************************************************************
*
* 	���������д�������ļ�
*
***********************************************************************************/
void set_failure_number(unsigned int failurenumber)
{
    const char * section = "failureinfo";
	char tmpstr[16] = "";
	sprintf(tmpstr,"%d",failurenumber);
	write_profile_string(section, "FailureNumber" , tmpstr ,FILE_HIK_CFG);
}


/*****************************************************************************
 �� �� ��  : get_custom_params
 ��������  : ��ȡ�Զ��������ļ��еĵ���ʱ������������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void get_custom_params()
{
	parse_start(FILE_CUSTOM);

	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //��ȡ����ʱ����
    g_struCountDown.iCountDownMode = get_one_value(section, "CountDownMode");
    g_struCountDown.iFreeGreenTime = get_one_value(section, "FreeGreenTime");
    g_struCountDown.iPulseGreenTime = get_one_value(section, "PulseGreenTime");
    g_struCountDown.iPulseRedTime = get_one_value(section, "PulseRedTime");
    
    DBG("����ʱ����  CountDownMode:%d  PulseGreenTime:%d  PulseRedTime:%d\n", 
                                        g_struCountDown.iCountDownMode,
                                        g_struCountDown.iPulseGreenTime,
                                        g_struCountDown.iPulseRedTime);

    //����ʱ��Ӧ��ͨ����Ϣ                                     
    for(i=1; i<33; i++)
    {
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		g_struCountDown.iPhaseOfChannel[i-1].iphase = get_one_value(section, tmpstr);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		g_struCountDown.iPhaseOfChannel[i-1].iType = get_one_value(section, tmpstr);

	    if(0 != g_struCountDown.iPhaseOfChannel[i-1].iphase)
	    {
            DBG("PhaseChannel_%02d  iphase %d  iType  %d\n",i,
                                        g_struCountDown.iPhaseOfChannel[i-1].iphase,
                                        g_struCountDown.iPhaseOfChannel[i-1].iType);
	    }
    }

    //���ڲ���
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		g_com_params[i].unBaudRate = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		g_com_params[i].unParity = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		g_com_params[i].unDataBits = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		g_com_params[i].unStopBits = get_one_value(section, tmpstr);

		if(0 != g_com_params[i].unBaudRate)
		{
            DBG("comNo=%d unBaudRate=%d unParity=%d unDataBits=%d unStopBits=%d\n",i+1,
                                                    g_com_params[i].unBaudRate,
                                                    g_com_params[i].unParity,
                                                    g_com_params[i].unDataBits,
                                                    g_com_params[i].unStopBits);
		}
	}	

    //ͨ����������
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    gChannelLockedParams.ucWorkingTimeFlag = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeHour");
    gChannelLockedParams.ucBeginTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeMin");
    gChannelLockedParams.ucBeginTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeSec");
    gChannelLockedParams.ucBeginTimeSec = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeHour");
    gChannelLockedParams.ucEndTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeMin");
    gChannelLockedParams.ucEndTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeSec");
    gChannelLockedParams.ucEndTimeSec = get_one_value(section,tmpstr);

    get_more_value(section,"ucChannelStatus",gChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    gChannelLockFlag = get_one_value(section,tmpstr);


    parse_end();
}

/*****************************************************************************
 �� �� ��  : set_custom_params
 ��������  : �����Ʋ������浽�����ļ�
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_custom_params(STRU_Count_Down_Params *pStruCountDown,COM_PARAMS *pComParams )
{
	parse_start(FILE_CUSTOM);

	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //����ʱ����
    add_one_key(section, "CountDownMode", pStruCountDown->iCountDownMode);
    add_one_key(section, "FreeGreenTime", pStruCountDown->iFreeGreenTime);
    add_one_key(section, "PulseGreenTime", pStruCountDown->iPulseGreenTime);
    add_one_key(section, "PulseRedTime", pStruCountDown->iPulseRedTime);
    

    //����ʱ��Ӧ��ͨ����Ϣ                                     
    for(i=1; i<33; i++)
    {
        if(0 == pStruCountDown->iPhaseOfChannel[i-1].iphase)
        {
            continue;
        }
        
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		add_one_key(section, tmpstr, pStruCountDown->iPhaseOfChannel[i-1].iphase);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		add_one_key(section, tmpstr, pStruCountDown->iPhaseOfChannel[i-1].iType);
    }

    //���ڲ���
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    if(0 == pComParams[i].unBaudRate)
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		add_one_key(section, tmpstr, pComParams[i].unBaudRate);
		
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		add_one_key(section, tmpstr, pComParams[i].unParity);
		
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		add_one_key(section, tmpstr, pComParams[i].unDataBits);
		
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		add_one_key(section, tmpstr, pComParams[i].unStopBits);

	}	

    //ͨ����������
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    add_one_key(section, tmpstr, gChannelLockedParams.ucWorkingTimeFlag);

    strcpy(tmpstr,"ucBeginTimeHour");
    add_one_key(section, tmpstr, gChannelLockedParams.ucBeginTimeHour);

    strcpy(tmpstr,"ucBeginTimeMin");
    add_one_key(section, tmpstr, gChannelLockedParams.ucBeginTimeMin);

    strcpy(tmpstr,"ucBeginTimeSec");
    add_one_key(section, tmpstr, gChannelLockedParams.ucBeginTimeSec);

    strcpy(tmpstr,"ucEndTimeHour");
    add_one_key(section, tmpstr, gChannelLockedParams.ucEndTimeHour);

    strcpy(tmpstr,"ucEndTimeMin");
    add_one_key(section, tmpstr, gChannelLockedParams.ucEndTimeMin);

    strcpy(tmpstr,"ucEndTimeSec");
    add_one_key(section, tmpstr, gChannelLockedParams.ucEndTimeSec);

    add_more_key(section, "ucChannelStatus",gChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    add_one_key(section, tmpstr, gChannelLockFlag);



    parse_end();
}

/*****************************************************************************
 �� �� ��  : get_desc_params
 ��������  : ��ȡdesc.ini�й�����λ������ͨ����������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void get_desc_params()
{
	parse_start(FILE_DESC);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //��ȡ��λ������Ϣ
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)phase_desc_params.stPhaseDesc[i]);

		if(0 != strlen((char *)phase_desc_params.stPhaseDesc[i]))
		{
    		DBG("%s=%s\n",tmpstr,phase_desc_params.stPhaseDesc[i]);
		}
	}	

    //��ȡͨ����Ϣ����
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)channel_desc_params.stChannelDesc[i]);

		if(0 != strlen((char *)channel_desc_params.stChannelDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,channel_desc_params.stChannelDesc[i]);
		}
	}	

    //��ȡ������Ϣ����
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pattern_name_params.stPatternNameDesc[i]);

		if(0 != strlen((char *)pattern_name_params.stPatternNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pattern_name_params.stPatternNameDesc[i]);
		}
	}	
	
    //��ȡ�ƻ���Ϣ����
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)plan_name_params.stPlanNameDesc[i]);

		if(0 != strlen((char *)plan_name_params.stPlanNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,plan_name_params.stPlanNameDesc[i]);
		}
	}	

	//��ȡ������Ϣ����
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)date_name_params.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		date_name_params.stNameDesc[i].dateType = get_one_value(section, tmpstr);

        if(0 != strlen((char *)date_name_params.stNameDesc[i].dateName))
        {
            DBG("DateDesc_%02d=%s  dateType= %d\n",i+1,date_name_params.stNameDesc[i].dateName,date_name_params.stNameDesc[i].dateType);
        }
	}	

    parse_end();
}

void set_desc_params(PHASE_DESC_PARAMS *pPhaseDescParams, CHANNEL_DESC_PARAMS *pChannelDescParams,PATTERN_NAME_PARAMS *pPatternDescParams,
                    PLAN_NAME_PARAMS *pPlanDescParams,DATE_NAME_PARAMS *pDateDescParams)
{
	parse_start(FILE_DESC);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //��λ������Ϣ
	for(i = 0; i < 16; i ++)
	{
	    if(0 == strlen((char *)pPhaseDescParams->stPhaseDesc[i]))
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pPhaseDescParams->stPhaseDesc[i]);

	}	

    //ͨ����Ϣ����
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		if(0 == strlen((char *)pChannelDescParams->stChannelDesc[i]))
		{
            continue;
		}	
		
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pChannelDescParams->stChannelDesc[i]);


	}	

    //������Ϣ����
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pChannelDescParams->stChannelDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pPatternDescParams->stPatternNameDesc[i]);


	}	
	
    //�ƻ���Ϣ����
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pPlanDescParams->stPlanNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pPlanDescParams->stPlanNameDesc[i]);
	}	

	//������Ϣ����
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
        if(0 == strlen((char *)pDateDescParams->stNameDesc[i].dateName))
        {
            continue;
        }
	
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDateDescParams->stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		add_one_key(section,tmpstr, pDateDescParams->stNameDesc[i].dateType);
	}	

    parse_end();
}


/*****************************************************************************
 �� �� ��  : get_all_params_form_config_file
 ��������  : �������ļ���ȡ�������ò���
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����в���ͳһ���ýӿ�
*****************************************************************************/
void get_all_params_from_config_file()
{
    //ת����UNIX��ʽ
    system("dos2unix /home/config.ini");
    system("dos2unix /home/desc.ini");
    system("dos2unix /home/custom.ini");

    //��ȡconfig.ini���й�GPS���ء��������������Ϻż�����������Ϣ
    get_special_params();

    //��ȡ�Զ��������ļ��й��ڵ���ʱ�����ڵȲ�����������Ϣ
    get_custom_params();

    //��ȡdesc.ini�й�����λ������ͨ����������Ϣ
    get_desc_params();
}

