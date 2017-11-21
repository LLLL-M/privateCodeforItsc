#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "common.h"
#include "hik.h"
#include "parse_ini.h"
#include "platform.h"

#define MAX_FILE_SIZE 1024*512
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'

struct SPECIAL_PARAMS s_special_params;       //特殊参数定义
unsigned int g_failurenumber;                 //故障事件序号
struct CURRENT_PARAMS g_struRecCurrent[32];   //电流参数
struct Count_Down_Params g_struCountDown;    //倒计时参数
PHASE_DESC_PARAMS phase_desc_params;        //相位描述  
CHANNEL_DESC_PARAMS channel_desc_params;   //通道描述
PATTERN_NAME_PARAMS pattern_name_params;   //方案描述
PLAN_NAME_PARAMS plan_name_params;         //计划描述
DATE_NAME_PARAMS date_name_params;		   //日期描述
COM_PARAMS com_params;                     //串口设置参数
COM_PARAMS g_com_params[4];                //全局串口参数
CHANNEL_LOCK_PARAMS gChannelLockedParams;
unsigned int g_car_detect_switch;
unsigned int g_printf_switch;
extern UInt8 gChannelLockFlag;
extern UInt8 gSpecialControlSchemeId;       //特殊控制方案号
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息

#define FILE_HIK_CFG        "/home/config.ini"      //全局参数配置文件，包括通道表、相位表等信号机运行必须参数及GPS开关等配置信息及故障序列号等信息
#define FILE_DESC           "/home/desc.ini"        //参数描述配置文件，包括通道描述、方案描述等信息
#define FILE_CUSTOM         "/home/custom.ini"      //定制信息配置文件 ，包括倒计时、串口、电流参数等


/*****************************************************************************
 函 数 名  : CopyChannelLockInfoToCountDownParams
 功能描述  : 将通道锁定的配置信息拷贝到倒计时接口中去。
 输入参数  : PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams  
             CHANNEL_LOCK_PARAMS *pChannelLockedParams              
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月31日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CopyChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_PARAMS *pChannelLockedParams)
{
    if((pCountDownParams == NULL) || (pChannelLockedParams == NULL))
    {
        return;
    }

    pCountDownParams->ucWorkingTimeFlag = pChannelLockedParams->ucWorkingTimeFlag;
    pCountDownParams->ucBeginTimeHour = pChannelLockedParams->ucBeginTimeHour;
    pCountDownParams->ucBeginTimeMin = pChannelLockedParams->ucBeginTimeMin;
    pCountDownParams->ucBeginTimeSec = pChannelLockedParams->ucBeginTimeSec;
    pCountDownParams->ucEndTimeHour = pChannelLockedParams->ucEndTimeHour;
    pCountDownParams->ucEndTimeMin = pChannelLockedParams->ucEndTimeMin;
    pCountDownParams->ucEndTimeSec = pChannelLockedParams->ucEndTimeSec;
    
    memcpy(pChannelLockedParams->ucChannelStatus,pChannelLockedParams->ucChannelStatus,sizeof(pChannelLockedParams->ucChannelStatus));
}

/*****************************************************************************
 函 数 名  : get_special_params
 功能描述  : 从配置文件中获取特殊参数
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 统一调用接口，不再多次打开文件

  3.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 从配置文件获取故障序号添加到该接口中

  4.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 电流检测添加到该接口中
*****************************************************************************/
void get_special_params()
{
	parse_start(FILE_HIK_CFG);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //一些开关量
    s_special_params.iErrorDetectSwitch = get_one_value(section, "ErrorDetectSwitch");
	s_special_params.iCurrentAlarmSwitch = get_one_value(section, "CurrentAlarmSwitch");
	s_special_params.iVoltageAlarmSwitch = get_one_value(section, "VoltageAlarmSwitch");
	s_special_params.iCurrentAlarmAndProcessSwitch = get_one_value(section, "CurrentAlarmAndProcessSwitch");
	s_special_params.iVoltageAlarmAndProcessSwitch = get_one_value(section, "VoltageAlarmAndProcessSwitch");
	s_special_params.iWatchdogSwitch = get_one_value(section, "WatchdogSwitch");
	s_special_params.iGpsSwitch = get_one_value(section, "GpsSwitch");
	g_car_detect_switch = get_one_value(section, "CarDetectSwitch");//车检板是否存在
	g_printf_switch = get_one_value(section, "PrintfLogSwitch");//printf打印信息是否开启
	

    //故障号
    g_failurenumber = get_one_value("failureinfo", "FailureNumber");

    
	DBG("ErrorDetectSwitch=%d  CurrentAlarmSwitch=%d  VoltageAlarmSwitch=%d  CurrentAlarmAndProcessSwitch=%d  \n\t\t\t\t VoltageAlarmAndProcessSwitch=%d  WatchdogSwitch=%d  GpsSwitch=%d  CarDetectSwitch=%d  PrintfLogSwitch=%d \n",
		s_special_params.iErrorDetectSwitch,s_special_params.iCurrentAlarmSwitch,s_special_params.iVoltageAlarmSwitch,
		s_special_params.iCurrentAlarmAndProcessSwitch,s_special_params.iVoltageAlarmAndProcessSwitch,
		s_special_params.iWatchdogSwitch,s_special_params.iGpsSwitch,g_car_detect_switch,g_printf_switch);
		
    //电流检测
    section = "currentparams";
	for(i=0;i<32;i++)
	{
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		g_struRecCurrent[i].RedCurrentBase  = get_one_value(section, tmpStr);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
		g_struRecCurrent[i].RedCurrentDiff  = get_one_value(section, tmpStr);

		if( (0 != g_struRecCurrent[i].RedCurrentDiff) && (0 != g_struRecCurrent[i].RedCurrentBase))
		{
    		DBG("通道%02d电流偏差值:%03d 电流基准值:%03d \n",i+1, g_struRecCurrent[i].RedCurrentDiff, g_struRecCurrent[i].RedCurrentBase);
		}
	}


    parse_end();
}



/*****************************************************************************
 函 数 名  : set_special_params
 功能描述  : 将特殊参数写入配置文件
 输入参数  : struct SPECIAL_PARAMS special_params  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void set_special_params()
{
	parse_start(FILE_HIK_CFG);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //一些开关量
	add_one_key(section, "ErrorDetectSwitch" ,s_special_params.iErrorDetectSwitch);
	add_one_key(section, "CurrentAlarmSwitch" ,s_special_params.iCurrentAlarmSwitch);
	add_one_key(section, "VoltageAlarmSwitch" ,s_special_params.iVoltageAlarmSwitch);
	add_one_key(section, "CurrentAlarmAndProcessSwitch" ,s_special_params.iCurrentAlarmAndProcessSwitch);
	add_one_key(section, "VoltageAlarmAndProcessSwitch" ,s_special_params.iVoltageAlarmAndProcessSwitch);
	add_one_key(section, "WatchdogSwitch" ,s_special_params.iWatchdogSwitch);
	add_one_key(section, "GpsSwitch" ,s_special_params.iGpsSwitch);
	add_one_key(section, "CarDetectSwitch" ,g_car_detect_switch);
	add_one_key(section, "PrintfLogSwitch" ,g_printf_switch);
	
    //故障号
    add_one_key("failureinfo", "FailureNumber" ,g_failurenumber);

    //电流检测
    section = "currentparams";
	for(i=0;i<32;i++)
	{
	    if(0 == g_struRecCurrent[i].RedCurrentBase)
	    {
            continue;
	    }
	    
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		add_one_key(section, tmpStr ,g_struRecCurrent[i].RedCurrentBase);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
        add_one_key(section, tmpStr ,g_struRecCurrent[i].RedCurrentDiff);
	}

	ERR("set_special_params  succeed\n");
	parse_end();
}

/*********************************************************************************
*
* 	将故障序号写入配置文件
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
 函 数 名  : get_custom_params
 功能描述  : 获取自定义配置文件中的倒计时、串口配置信息
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年3月31日
    作    者   : 肖文虎
    修改内容   : 新增：将通道锁定信息拷贝到倒计时接口中。
*****************************************************************************/
void get_custom_params()
{
	parse_start(FILE_CUSTOM);

	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //获取倒计时参数
    g_struCountDown.iCountDownMode = get_one_value(section, "CountDownMode");
    g_struCountDown.iFreeGreenTime = get_one_value(section, "FreeGreenTime");
    g_struCountDown.iPulseGreenTime = get_one_value(section, "PulseGreenTime");
    g_struCountDown.iPulseRedTime = get_one_value(section, "PulseRedTime");
    
    DBG("倒计时参数  CountDownMode:%d  PulseGreenTime:%d  PulseRedTime:%d\n", 
                                        g_struCountDown.iCountDownMode,
                                        g_struCountDown.iPulseGreenTime,
                                        g_struCountDown.iPulseRedTime);

    //倒计时对应的通道信息                                     
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

    //串口参数
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

    //通道锁定参数
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

    CopyChannelLockInfoToCountDownParams(gCountDownParams,&gChannelLockedParams);

    //特殊控制号
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    gSpecialControlSchemeId = get_one_value(section,tmpstr);

    parse_end();
}

/*****************************************************************************
 函 数 名  : set_custom_params
 功能描述  : 将定制参数保存到配置文件
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void set_custom_params()
{
	parse_start(FILE_CUSTOM);
	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //倒计时参数
    add_one_key(section, "CountDownMode", g_struCountDown.iCountDownMode);
    add_one_key(section, "FreeGreenTime", g_struCountDown.iFreeGreenTime);
    add_one_key(section, "PulseGreenTime", g_struCountDown.iPulseGreenTime);
    add_one_key(section, "PulseRedTime", g_struCountDown.iPulseRedTime);
    

    //倒计时对应的通道信息                                     
    for(i=1; i<33; i++)
    {
        if(0 == g_struCountDown.iPhaseOfChannel[i-1].iphase)
        {
            continue;
        }
        
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		add_one_key(section, tmpstr, g_struCountDown.iPhaseOfChannel[i-1].iphase);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		add_one_key(section, tmpstr, g_struCountDown.iPhaseOfChannel[i-1].iType);
    }

    //串口参数
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    if(0 == g_com_params[i].unBaudRate)
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		add_one_key(section, tmpstr, g_com_params[i].unBaudRate);
		
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		add_one_key(section, tmpstr, g_com_params[i].unParity);
		
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		add_one_key(section, tmpstr, g_com_params[i].unDataBits);
		
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		add_one_key(section, tmpstr, g_com_params[i].unStopBits);

	}	

    //通道锁定参数
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
    add_one_key(section, tmpstr, ((gChannelLockFlag >= 1) ? 1 : gChannelLockFlag));

    //特殊控制号
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    add_one_key(section, tmpstr, gSpecialControlSchemeId);

    DBG("set_custom_params  succeed\n");
    parse_end();
}

/*****************************************************************************
 函 数 名  : get_desc_params
 功能描述  : 获取desc.ini中关于相位描述、通道描述等信息
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void get_desc_params()
{
	parse_start(FILE_DESC);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //获取相位描述信息
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)phase_desc_params.stPhaseDesc[i]);

		if(0 != strlen((char *)phase_desc_params.stPhaseDesc[i]))
		{
    		DBG("%s=%s\n",tmpstr,phase_desc_params.stPhaseDesc[i]);
		}
	}	

    //获取通道信息描述
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

    //获取方案信息描述
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
	
    //获取计划信息描述
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

	//获取日期信息描述
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

void set_desc_params()
{
	parse_start(FILE_DESC);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //相位描述信息
	for(i = 0; i < 16; i ++)
	{
	    if(0 == strlen((char *)phase_desc_params.stPhaseDesc[i]))
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)phase_desc_params.stPhaseDesc[i]);

	}	

    //通道信息描述
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		if(0 == strlen((char *)channel_desc_params.stChannelDesc[i]))
		{
            continue;
		}	
		
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)channel_desc_params.stChannelDesc[i]);


	}	

    //方案信息描述
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pattern_name_params.stPatternNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pattern_name_params.stPatternNameDesc[i]);


	}	
	
    //计划信息描述
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)plan_name_params.stPlanNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)plan_name_params.stPlanNameDesc[i]);
	}	

	//日期信息描述
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
        if(0 == strlen((char *)date_name_params.stNameDesc[i].dateName))
        {
            continue;
        }
	
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)date_name_params.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		add_one_key(section,tmpstr, date_name_params.stNameDesc[i].dateType);
	}	

    ERR("set_desc_params  succeed\n");
    parse_end();
}


/*****************************************************************************
 函 数 名  : get_all_params_form_config_file
 功能描述  : 从配置文件获取所有配置参数
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 新生成函数

  2.日    期   : 2015年2月28日
    作    者   : 肖文虎
    修改内容   : 将所有参数统一调用接口
*****************************************************************************/
void get_all_params_from_config_file()
{
    //转换成UNIX格式
    system("dos2unix /home/config.ini");
    system("dos2unix /home/desc.ini");
    system("dos2unix /home/custom.ini");

    //获取config.ini中有关GPS开关、电流参数、故障号及其他参数信息
    get_special_params();

    //获取自定义配置文件中关于倒计时、串口等参数的配置信息
    get_custom_params();

    //获取desc.ini中关于相位描述、通道描述等信息
    get_desc_params();
}

