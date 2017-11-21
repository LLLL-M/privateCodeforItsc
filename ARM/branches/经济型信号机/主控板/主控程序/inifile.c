#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "common.h"
#include "debug.h"
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


/*********************************************************************************
*
* 	�������ļ���ȡ�������
*
***********************************************************************************/
void get_special_params()
{
	//�������ļ�ת��Ϊlinux�¸�ʽ
	system("dos2unix /home/config.ini");
	
	const char * file = "/home/config.ini" ;
    const char * section = "config" ;
    s_special_params.iErrorDetectSwitch = read_profile_int(section, "ErrorDetectSwitch", 0, file);
	s_special_params.iCurrentAlarmSwitch = read_profile_int(section, "CurrentAlarmSwitch", 0, file);
	s_special_params.iVoltageAlarmSwitch = read_profile_int(section, "VoltageAlarmSwitch", 0, file);
	s_special_params.iCurrentAlarmAndProcessSwitch = read_profile_int(section, "CurrentAlarmAndProcessSwitch", 0, file);
	s_special_params.iVoltageAlarmAndProcessSwitch = read_profile_int(section, "VoltageAlarmAndProcessSwitch", 0, file);
	s_special_params.iWatchdogSwitch = read_profile_int(section, "WatchdogSwitch", 0, file);
	s_special_params.iGpsSwitch = read_profile_int(section, "GpsSwitch", 0, file);
	s_special_params.iSignalMachineType = read_profile_int(section, "SignalMachineType", 0, file);
	INFO("ErrorDetectSwitch=%d\nCurrentAlarmSwitch=%d\nVoltageAlarmSwitch=%d\nCurrentAlarmAndProcessSwitch=%d\nVoltageAlarmAndProcessSwitch=%d\nWatchdogSwitch=%d\nGpsSwitch=%d\nSignalMachineType=%d\n",
		s_special_params.iErrorDetectSwitch,s_special_params.iCurrentAlarmSwitch,s_special_params.iVoltageAlarmSwitch,
		s_special_params.iCurrentAlarmAndProcessSwitch,s_special_params.iVoltageAlarmAndProcessSwitch,
		s_special_params.iWatchdogSwitch,s_special_params.iGpsSwitch,s_special_params.iSignalMachineType);
}


/*********************************************************************************
*
* 	�������ļ���ȡ�������
*
***********************************************************************************/
void get_failure_number()
{
	const char * file = "/home/config.ini" ;
    const char * section = "failureinfo" ;
    g_failurenumber = read_profile_int(section, "FailureNumber", 0, file);
}


/*********************************************************************************
*
* 	���������д�������ļ�
*
***********************************************************************************/
void set_failure_number(unsigned int failurenumber)
{
	const char * file = "/home/config.ini" ;
    const char * section = "failureinfo";
	char tmpstr[16] = "";
	sprintf(tmpstr,"%d",failurenumber);
	write_profile_string(section, "FailureNumber" , tmpstr ,file);
}



/*********************************************************************************
*
* 	���������д�������ļ�
*
***********************************************************************************/
void set_special_params(struct SPECIAL_PARAMS special_params)
{
	const char * file = "/home/config.ini" ;
    const char * section = "config";
	char tmpstr[16] = "";
	sprintf(tmpstr,"%d",special_params.iErrorDetectSwitch);
	write_profile_string(section, "ErrorDetectSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iCurrentAlarmSwitch);
	write_profile_string(section, "CurrentAlarmSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iVoltageAlarmSwitch);
	write_profile_string(section, "VoltageAlarmSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iCurrentAlarmAndProcessSwitch);
	write_profile_string(section, "CurrentAlarmAndProcessSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iVoltageAlarmAndProcessSwitch);
	write_profile_string(section, "VoltageAlarmAndProcessSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iWatchdogSwitch);
	write_profile_string(section, "WatchdogSwitch" , tmpstr ,file);
	sprintf(tmpstr,"%d",special_params.iGpsSwitch);
	write_profile_string(section, "GpsSwitch" , tmpstr ,file);	
	sprintf(tmpstr,"%d",special_params.iSignalMachineType);
	write_profile_string(section, "SignalMachineType" , tmpstr ,file);
}


/*********************************************************************************
*
* 	�������ļ��л�ȡ��������
*
***********************************************************************************/
void get_current_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "currentparams" ;
	char tmpstr[32] = "";
	int i = 0;
	for(i=0;i<32;i++)
	{
		sprintf(tmpstr,"RedCurrentBase%02d",i+1);
		g_struRecCurrent[i].RedCurrentBase  = read_profile_int(section, tmpstr, 0, file);
		DBG("ͨ��%02d������׼ֵ:%03d\n",i+1, g_struRecCurrent[i].RedCurrentBase);
		sprintf(tmpstr,"RedCurrentDiff%02d",i+1);
		g_struRecCurrent[i].RedCurrentDiff  = read_profile_int(section, tmpstr, 0, file);
		DBG("ͨ��%02d����ƫ��ֵ:%03d\n",i+1, g_struRecCurrent[i].RedCurrentDiff);
	}	
}

/*********************************************************************************
*
* 	����������д�������ļ�
*
***********************************************************************************/
void set_current_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "currentparams";
	char tmpvalue[16] = "";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpvalue,"%d",g_struRecCurrent[i].RedCurrentBase);
		sprintf(tmpstr,"RedCurrentBase%02d",i+1);
		write_profile_string(section, tmpstr, tmpvalue ,file);
		sprintf(tmpvalue,"%d",g_struRecCurrent[i].RedCurrentDiff);
		sprintf(tmpstr,"RedCurrentDiff%02d",i+1);
		write_profile_string(section, tmpstr, tmpvalue ,file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ����ʱ����
*
***********************************************************************************/
void get_count_down_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "countdown" ;
    int channel = 0;
    char tempPhaseOfChannel[24] = "";	

    g_struCountDown.iCountDownMode = read_profile_int(section, "CountDownMode", 0, file);
    g_struCountDown.iFreeGreenTime = read_profile_int(section, "FreeGreenTime", 0, file);
    g_struCountDown.iPulseGreenTime = read_profile_int(section, "PulseGreenTime", 0, file);
    g_struCountDown.iPulseRedTime = read_profile_int(section, "PulseRedTime", 0, file);
    
    DBG("����ʱ����\nCountDownMode:%d\nPulseGreenTime:%d\nPulseRedTime:%d\n", 
    g_struCountDown.iCountDownMode,
    g_struCountDown.iPulseGreenTime,g_struCountDown.iPulseRedTime);
    for(channel=1; channel<33; channel++)
    {
		sprintf(tempPhaseOfChannel,"PhaseChannel_%02d",channel);
		g_struCountDown.iPhaseOfChannel[channel-1].iphase = read_profile_int(section, tempPhaseOfChannel, 0, file);
		DBG("%s:%d\n",tempPhaseOfChannel,g_struCountDown.iPhaseOfChannel[channel-1].iphase);
		sprintf(tempPhaseOfChannel,"ChannelProperty_%02d",channel);
		g_struCountDown.iPhaseOfChannel[channel-1].iType = read_profile_int(section, tempPhaseOfChannel, 0, file);
		DBG("%s:%d\n",tempPhaseOfChannel,g_struCountDown.iPhaseOfChannel[channel-1].iType);
    }
}

/*********************************************************************************
*
* 	������ʱ�Ʋ���д�������ļ�
*
***********************************************************************************/
void set_count_down_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "countdown" ;
	char tmpstr[32] = "";
	char tmpstr1[32] = "";
	int i = 0;
	sprintf(tmpstr,"%d",g_struCountDown.iCountDownMode);
	write_profile_string(section, "CountDownMode", tmpstr,file);
	sprintf(tmpstr,"%d",g_struCountDown.iFreeGreenTime);
	write_profile_string(section, "FreeGreenTime", tmpstr,file);
	sprintf(tmpstr,"%d",g_struCountDown.iPulseGreenTime);
	write_profile_string(section, "PulseGreenTime", tmpstr,file);
	sprintf(tmpstr,"%d",g_struCountDown.iPulseRedTime);
	write_profile_string(section, "PulseRedTime", tmpstr,file);
	
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"PhaseChannel_%02d",i+1);
		sprintf(tmpstr1,"%d",g_struCountDown.iPhaseOfChannel[i].iphase);
		write_profile_string(section, tmpstr, (const char *)tmpstr1,file);	
		sprintf(tmpstr,"ChannelProperty_%02d",i+1);
		sprintf(tmpstr1,"%d",g_struCountDown.iPhaseOfChannel[i].iType);
		write_profile_string(section, tmpstr, (const char *)tmpstr1,file);
	}	
}


/*********************************************************************************
*
* 	����λ��������д�������ļ�
*
***********************************************************************************/
void set_phase_desc_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "phasedesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);
		write_profile_string(section, tmpstr, (const char *)phase_desc_params.stPhaseDesc[i],file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ��λ��������
*
***********************************************************************************/
void get_phase_desc_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "phasedesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		read_profile_string(section,tmpstr,(char *)phase_desc_params.stPhaseDesc[i],64,"",file);
		DBG("%s=%s\n",tmpstr,phase_desc_params.stPhaseDesc[i]);
	}	
}

/*********************************************************************************
*
* 	��ͨ����������д�������ļ�
*
***********************************************************************************/
void set_channel_desc_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "channeldesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);
		write_profile_string(section, tmpstr, (const char *)channel_desc_params.stChannelDesc[i],file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡͨ����������
*
***********************************************************************************/
void get_channel_desc_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "channeldesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		read_profile_string(section,tmpstr,(char *)channel_desc_params.stChannelDesc[i],64,"",file);
		DBG("%s=%s\n",tmpstr,channel_desc_params.stChannelDesc[i]);
	}	
}

/*********************************************************************************
*
* 	��������������д�������ļ�
*
***********************************************************************************/
void set_pattern_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "patterndesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PatternDesc_%02d",i+1);
		write_profile_string(section, tmpstr, (const char *)pattern_name_params.stPatternNameDesc[i],file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ������������
*
***********************************************************************************/
void get_pattern_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "patterndesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		read_profile_string(section,tmpstr,(char *)pattern_name_params.stPatternNameDesc[i],64,"",file);
		DBG("%s=%s\n",tmpstr,pattern_name_params.stPatternNameDesc[i]);
	}	
}

/*********************************************************************************
*
* 	���ƻ���������д�������ļ�
*
***********************************************************************************/
void set_plan_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "plandesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PlanDesc_%02d",i+1);
		write_profile_string(section, tmpstr, (const char *)plan_name_params.stPlanNameDesc[i],file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ������������
*
***********************************************************************************/
void get_plan_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "plandesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		read_profile_string(section,tmpstr,(char *)plan_name_params.stPlanNameDesc[i],64,"",file);
		DBG("%s=%s\n",tmpstr,plan_name_params.stPlanNameDesc[i]);
	}	
}

/*********************************************************************************
*
* 	��������������д�������ļ�
*
***********************************************************************************/
void set_date_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "datedesc";
	char tmpstr[32] = "";
	char tmpstr1[32] = "";
	int i = 0;
	for(i = 0; i < 40; i ++)
	{
		sprintf(tmpstr,"DateDesc_%02d",i+1);
		write_profile_string(section, tmpstr, (const char *)date_name_params.stNameDesc[i].dateName,file);	
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		sprintf(tmpstr1,"%d",date_name_params.stNameDesc[i].dateType);
		write_profile_string(section, tmpstr, (const char *)tmpstr1,file);	
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ������������
*
***********************************************************************************/
void get_date_name_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "datedesc";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 40; i ++)
	{
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		read_profile_string(section,tmpstr,(char *)date_name_params.stNameDesc[i].dateName,64,"",file);
		DBG("%s=%s\n",tmpstr,date_name_params.stNameDesc[i].dateName);
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		date_name_params.stNameDesc[i].dateType = read_profile_int(section, tmpstr, 0, file);
		DBG("%s=%d\n",tmpstr,date_name_params.stNameDesc[i].dateType);
	}	
}

/*********************************************************************************
*
* 	�����ڲ���д�������ļ�
*
***********************************************************************************/
void set_com_params(int comNo)
{
	const char * file = "/home/config.ini" ;
    const char * section = "comparams";
	char tmpstr[32] = "";
	char tmpstr1[32] = "";
	if((comNo < 1) || (comNo > 4))
	{
		return;
	}
	sprintf(tmpstr,"ComBaudRate_%02d",comNo);
	sprintf(tmpstr1,"%d",com_params.unBaudRate);
	write_profile_string(section, tmpstr,tmpstr1,file);	
	sprintf(tmpstr,"ComParity_%02d",comNo);
	sprintf(tmpstr1,"%d",com_params.unParity);
	write_profile_string(section, tmpstr, tmpstr1,file);
	sprintf(tmpstr,"ComDataBits_%02d",comNo);
	sprintf(tmpstr1,"%d",com_params.unDataBits);
	write_profile_string(section, tmpstr,tmpstr1,file);	
	sprintf(tmpstr,"ComStopBits_%02d",comNo);
	sprintf(tmpstr1,"%d",com_params.unStopBits);
	write_profile_string(section, tmpstr, tmpstr1,file);
}

/*********************************************************************************
*
* 	�������ļ���ȡ���ڲ���
*
***********************************************************************************/
void get_com_params(int comNo)
{
	const char * file = "/home/config.ini" ;
    const char * section = "comparams";
	char tmpstr[32] = "";
	if((comNo < 1) || (comNo > 4))
	{
		return;
	}	
	sprintf(tmpstr,"ComBaudRate_%02d",comNo);	
	com_params.unBaudRate = read_profile_int(section, tmpstr, 0, file);
	DBG("comNo=%d\n%s=%d\n",comNo,tmpstr,com_params.unBaudRate);
	sprintf(tmpstr,"ComParity_%02d",comNo);	
	com_params.unParity = read_profile_int(section, tmpstr, 0, file);
	DBG("%s=%d\n",tmpstr,com_params.unParity);
	sprintf(tmpstr,"ComDataBits_%02d",comNo);	
	com_params.unDataBits = read_profile_int(section, tmpstr, 0, file);
	DBG("%s=%d\n",tmpstr,com_params.unDataBits);
	sprintf(tmpstr,"ComStopBits_%02d",comNo);	
	com_params.unStopBits = read_profile_int(section, tmpstr, 0, file);
	DBG("%s=%d\n",tmpstr,com_params.unStopBits);
}

/*********************************************************************************
*
* 	�������ļ���ȡ���д��ڲ���
*
***********************************************************************************/
void get_all_com_params()
{
	const char * file = "/home/config.ini" ;
    const char * section = "comparams";
	char tmpstr[32] = "";
	int i = 0;
	for(i = 0; i < 4; i ++)
	{
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		g_com_params[i].unBaudRate = read_profile_int(section, tmpstr, 0, file);
		DBG("comNo=%d\n%s=%d\n",i+1,tmpstr,g_com_params[i].unBaudRate);
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		g_com_params[i].unParity = read_profile_int(section, tmpstr, 0, file);
		DBG("%s=%d\n",tmpstr,g_com_params[i].unParity);
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		g_com_params[i].unDataBits = read_profile_int(section, tmpstr, 0, file);
		DBG("%s=%d\n",tmpstr,g_com_params[i].unDataBits);
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		g_com_params[i].unStopBits = read_profile_int(section, tmpstr, 0, file);
		DBG("%s=%d\n",tmpstr,g_com_params[i].unStopBits);
	}	
}

/*********************************************************************************
*
* 	�������ļ���ȡ�������ò���
*
***********************************************************************************/
void get_all_params_form_config_file()
{
	//�������ļ��л�ȡ�������
	get_special_params();

	//�������ļ��л�ȡ������־���
	get_failure_number();

	//��ȡ��������
	get_current_params();

	//��ȡ����ʱ����
	get_count_down_params();

	//��ȡ��λ��Ϣ����
	get_phase_desc_params();

	//��ȡͨ����Ϣ����
	get_channel_desc_params();

	//��ȡ������Ϣ����
	get_pattern_name_params();

	//��ȡ�ƻ���Ϣ����
	get_plan_name_params();

	//��ȡ������Ϣ����
	get_date_name_params();

	//��ȡȫ���������ò���
	get_all_com_params();

	//��ȡ���˼�����
	//get_ped_detect_params();
}

