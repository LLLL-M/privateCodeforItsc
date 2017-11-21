#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "parse_ini.h"


#define MAX_FILE_SIZE 1024*16
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'

struct SPECIAL_PARAMS s_special_params;       //特殊参数定义
unsigned int g_failurenumber;                 //故障事件序号
struct CURRENT_PARAMS g_struRecCurrent[32];   //电流参数
FAULT_CFG gFaultCfg;

/*********************************************************************************
*
* 	从配置文件获取故障序号
*
***********************************************************************************/
void get_special_params(char *cfgName)
{
	//把配置文件转换为linux下格式
	//char cmd[128] = "0";
   // sprintf(cmd,"dos2unix %s",cfgName);
    ///system(cmd);
	
    const char * section = "config" ;
    s_special_params.iErrorDetectSwitch = read_profile_int(section, "ErrorDetectSwitch", 0, cfgName);
	s_special_params.iCurrentAlarmSwitch = read_profile_int(section, "CurrentAlarmSwitch", 0, cfgName);
	s_special_params.iVoltageAlarmSwitch = read_profile_int(section, "VoltageAlarmSwitch", 0, cfgName);
	s_special_params.iCurrentAlarmAndProcessSwitch = read_profile_int(section, "CurrentAlarmAndProcessSwitch", 0, cfgName);
	s_special_params.iVoltageAlarmAndProcessSwitch = read_profile_int(section, "VoltageAlarmAndProcessSwitch", 0, cfgName);
	s_special_params.iWatchdogSwitch = read_profile_int(section, "WatchdogSwitch", 0, cfgName);
	s_special_params.iGpsSwitch = read_profile_int(section, "GpsSwitch", 0, cfgName);
	printf("ErrorDetectSwitch=%d\nCurrentAlarmSwitch=%d\nVoltageAlarmSwitch=%d\nCurrentAlarmAndProcessSwitch=%d\nVoltageAlarmAndProcessSwitch=%d\nWatchdogSwitch=%d\nGpsSwitch=%d\n",
		s_special_params.iErrorDetectSwitch,s_special_params.iCurrentAlarmSwitch,s_special_params.iVoltageAlarmSwitch,
		s_special_params.iCurrentAlarmAndProcessSwitch,s_special_params.iVoltageAlarmAndProcessSwitch,
		s_special_params.iWatchdogSwitch,s_special_params.iGpsSwitch);
}


/*********************************************************************************
*
* 	从配置文件获取故障序号
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
* 	将故障序号写入配置文件
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
* 	将特殊参数写入配置文件
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
}


/*********************************************************************************
*
* 	从配置文件中获取电流参数
*
***********************************************************************************/
void get_current_params(char *cfgName)
{
    const char * section = "currentparams" ;
	char tmpstr[32] = "";
	int i = 0;
	for(i=0;i<32;i++)
	{
		sprintf(tmpstr,"RedCurrentBase%02d",i+1);
		g_struRecCurrent[i].RedCurrentBase  = read_profile_int(section, tmpstr, 0, cfgName);
		printf("通道%02d电流基准值:%03d\n",i+1, g_struRecCurrent[i].RedCurrentBase);
		sprintf(tmpstr,"RedCurrentDiff%02d",i+1);
		g_struRecCurrent[i].RedCurrentDiff  = read_profile_int(section, tmpstr, 0, cfgName);
		printf("通道%02d电流偏差值:%03d\n",i+1, g_struRecCurrent[i].RedCurrentDiff);
	}	
}

void get_fault_cfg(char *cfgName)
{
    const char * section = "FaultConfig" ;
    gFaultCfg.nIsControlRecord = read_profile_int(section, "ControlRecord", 0, cfgName);
    gFaultCfg.nIsLogRecord = read_profile_int(section, "LogRecord", 0, cfgName);


    printf("ControlRecord=%d,LogRecord=%d\n",gFaultCfg.nIsControlRecord,gFaultCfg.nIsLogRecord);
}


/*********************************************************************************
*
* 	将电流参数写入配置文件
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
