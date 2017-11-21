/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : inifile.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年3月13日
  最近修改   :
  功能描述   : 将配置结构体信息写入到文本文件，及将文本文件中的配置信息读取到结构体中。该文件主要供转换工具BinaryTextConvert使用
                ，用来实现二进制与文本文件间的转换。
                每次新增配置文件或者新增配置项，均需要修改本文件，否则转换工具可能无法正常使用。
  函数列表   :
              get_all_params_from_config_file
              get_custom_params
              get_desc_params
              get_special_params
              set_custom_params
              set_desc_params
              set_failure_number
              set_special_params
  修改历史   :
  1.日    期   : 2015年3月13日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "inifile.h"
#include "parse_ini.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


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
void get_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config)
{
	parse_start(fileName);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //一些开关量
    config->sSpecialParams.iErrorDetectSwitch = get_one_value(section, "ErrorDetectSwitch");
	config->sSpecialParams.iCurrentAlarmSwitch = get_one_value(section, "CurrentAlarmSwitch");
	config->sSpecialParams.iVoltageAlarmSwitch = get_one_value(section, "VoltageAlarmSwitch");
	config->sSpecialParams.iCurrentAlarmAndProcessSwitch = get_one_value(section, "CurrentAlarmAndProcessSwitch");
	config->sSpecialParams.iVoltageAlarmAndProcessSwitch = get_one_value(section, "VoltageAlarmAndProcessSwitch");
	config->sSpecialParams.iWatchdogSwitch = get_one_value(section, "WatchdogSwitch");
	config->sSpecialParams.iGpsSwitch = get_one_value(section, "GpsSwitch");
	config->cCarDetectSwitch = get_one_value(section, "CarDetectSwitch");//车检板是否存在
	config->cPrintfLogSwitch = get_one_value(section, "PrintfLogSwitch");//printf打印信息是否开启
	config->sSpecialParams.iSignalMachineType = get_one_value(section, "SignalMachineType");
	config->sSpecialParams.iRedSignalCheckSwitch = get_one_value(section, "RedSignalCheckSwitch");


    //故障号
    config->cFailureNumber = get_one_value("failureinfo", "FailureNumber");

    
	DBG("ErrorDetectSwitch=%d  CurrentAlarmSwitch=%d  VoltageAlarmSwitch=%d  CurrentAlarmAndProcessSwitch=%d  \n\t\t\t\t VoltageAlarmAndProcessSwitch=%d  WatchdogSwitch=%d  GpsSwitch=%d  CarDetectSwitch=%d  PrintfLogSwitch=%d \n",
		config->sSpecialParams.iErrorDetectSwitch,config->sSpecialParams.iCurrentAlarmSwitch,config->sSpecialParams.iVoltageAlarmSwitch,
		config->sSpecialParams.iCurrentAlarmAndProcessSwitch,config->sSpecialParams.iVoltageAlarmAndProcessSwitch,
		config->sSpecialParams.iWatchdogSwitch,config->sSpecialParams.iGpsSwitch,config->cCarDetectSwitch,config->cPrintfLogSwitch);
		
    //电流检测
    section = "currentparams";
	for(i=0;i<32;i++)
	{
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		config->sCurrentParams[i].RedCurrentBase  = get_one_value(section, tmpStr);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
		config->sCurrentParams[i].RedCurrentDiff  = get_one_value(section, tmpStr);

		if( (0 != config->sCurrentParams[i].RedCurrentDiff) && (0 != config->sCurrentParams[i].RedCurrentBase))
		{
    		DBG("通道%02d电流偏差值:%03d 电流基准值:%03d \n",i+1, config->sCurrentParams[i].RedCurrentDiff, config->sCurrentParams[i].RedCurrentBase);
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
void set_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config)
{
	parse_start(fileName);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //一些开关量
	add_one_key(section, "ErrorDetectSwitch" ,config->sSpecialParams.iErrorDetectSwitch);
	add_one_key(section, "CurrentAlarmSwitch" ,config->sSpecialParams.iCurrentAlarmSwitch);
	add_one_key(section, "VoltageAlarmSwitch" ,config->sSpecialParams.iVoltageAlarmSwitch);
	add_one_key(section, "CurrentAlarmAndProcessSwitch" ,config->sSpecialParams.iCurrentAlarmAndProcessSwitch);
	add_one_key(section, "VoltageAlarmAndProcessSwitch" ,config->sSpecialParams.iVoltageAlarmAndProcessSwitch);
	add_one_key(section, "WatchdogSwitch" ,config->sSpecialParams.iWatchdogSwitch);
	add_one_key(section, "GpsSwitch" ,config->sSpecialParams.iGpsSwitch);
	add_one_key(section, "CarDetectSwitch" ,config->cCarDetectSwitch);
	add_one_key(section, "PrintfLogSwitch" ,config->cPrintfLogSwitch);
	add_one_key(section, "SignalMachineType" ,config->sSpecialParams.iSignalMachineType);
	add_one_key(section, "RedSignalCheckSwitch" ,config->sSpecialParams.iRedSignalCheckSwitch);

    //故障号
    add_one_key("failureinfo", "FailureNumber" ,config->cFailureNumber);

    //电流检测
    section = "currentparams";
	for(i=0;i<32;i++)
	{
	    if(0 == config->sCurrentParams[i].RedCurrentBase)
	    {
            continue;
	    }
	    
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		add_one_key(section, tmpStr ,config->sCurrentParams[i].RedCurrentBase);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
        add_one_key(section, tmpStr ,config->sCurrentParams[i].RedCurrentDiff);
	}

	parse_end();

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
void get_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom)
{
	parse_start(fileName);

	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //获取倒计时参数
    pCustom->sCountdownParams.iCountDownMode = get_one_value(section, "CountDownMode");
    pCustom->sCountdownParams.iFreeGreenTime = get_one_value(section, "FreeGreenTime");
    pCustom->sCountdownParams.iPulseGreenTime = get_one_value(section, "PulseGreenTime");
    pCustom->sCountdownParams.iPulseRedTime = get_one_value(section, "PulseRedTime");
    
    DBG("倒计时参数  CountDownMode:%d  PulseGreenTime:%d  PulseRedTime:%d\n", 
                                        pCustom->sCountdownParams.iCountDownMode,
                                        pCustom->sCountdownParams.iPulseGreenTime,
                                        pCustom->sCountdownParams.iPulseRedTime);

    //倒计时对应的通道信息                                     
    for(i=1; i<33; i++)
    {
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase = get_one_value(section, tmpstr);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType = get_one_value(section, tmpstr);

	    if(0 != pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase)
	    {
            DBG("PhaseChannel_%02d  iphase %d  iType  %d\n",i,
                                        pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase,
                                        pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType);
	    }
    }

    //串口参数
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    pCustom->sComParams[i].unExtraParamValue = i+1;
	    
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		pCustom->sComParams[i].unBaudRate = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		pCustom->sComParams[i].unParity = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		pCustom->sComParams[i].unDataBits = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		pCustom->sComParams[i].unStopBits = get_one_value(section, tmpstr);

		if(0 != pCustom->sComParams[i].unBaudRate)
		{
            DBG("comNo=%d unBaudRate=%d unParity=%d unDataBits=%d unStopBits=%d\n",i+1,
                                                    pCustom->sComParams[i].unBaudRate,
                                                    pCustom->sComParams[i].unParity,
                                                    pCustom->sComParams[i].unDataBits,
                                                    pCustom->sComParams[i].unStopBits);
		}
	}	

    //通道锁定参数
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    pCustom->sChannelLockedParams.ucWorkingTimeFlag = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeHour");
    pCustom->sChannelLockedParams.ucBeginTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeMin");
    pCustom->sChannelLockedParams.ucBeginTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeSec");
    pCustom->sChannelLockedParams.ucBeginTimeSec = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeHour");
    pCustom->sChannelLockedParams.ucEndTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeMin");
    pCustom->sChannelLockedParams.ucEndTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeSec");
    pCustom->sChannelLockedParams.ucEndTimeSec = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucReserved");
    pCustom->sChannelLockedParams.ucReserved = get_one_value(section,tmpstr);

    get_more_value(section,"ucChannelStatus",pCustom->sChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    pCustom->cChannelLockFlag = get_one_value(section,tmpstr);

//    CopyChannelLockInfoToCountDownParams(gCountDownParams,&pCustom->sChannelLockedParams);
	section = "MulPeroidsChanLock";
	pCustom->sMulPeriodsChanLockParams.cLockFlag = get_one_value(section, "chanLockFlag");
	for(i=0; i<16; i++)
	{
		sprintf(tmpstr, "MulPeroidsChanLock%02d", i+1);
		get_more_value(tmpstr,"channelStatus", pCustom->sMulPeriodsChanLockParams.chans[i].ucChannelStatus,32);		
		pCustom->sMulPeriodsChanLockParams.chans[i].ucWorkingTimeFlag = get_one_value(tmpstr, "workingTimeFlag");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeHour = get_one_value(tmpstr, "beginTimeHour");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeMin = get_one_value(tmpstr, "beginTimeMin");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeSec = get_one_value(tmpstr, "beginTimeSec");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeHour = get_one_value(tmpstr, "endTimeHour");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeMin = get_one_value(tmpstr, "endTimeMin");
		pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeSec = get_one_value(tmpstr, "endTimeSec");
	}

    //特殊控制号
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    pCustom->cSpecialControlSchemeId = get_one_value(section,tmpstr);

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
void set_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom)
{
	parse_start(fileName);
	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //倒计时参数
    add_one_key(section, "CountDownMode", pCustom->sCountdownParams.iCountDownMode);
    add_one_key(section, "FreeGreenTime", pCustom->sCountdownParams.iFreeGreenTime);
    add_one_key(section, "PulseGreenTime", pCustom->sCountdownParams.iPulseGreenTime);
    add_one_key(section, "PulseRedTime", pCustom->sCountdownParams.iPulseRedTime);
    

    //倒计时对应的通道信息                                     
    for(i=1; i<33; i++)
    {
        if(0 == pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase)
        {
            continue;
        }
        
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		add_one_key(section, tmpstr, pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		add_one_key(section, tmpstr, pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType);
    }

    //串口参数
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    if(0 == pCustom->sComParams[i].unBaudRate)
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"ComBaudRate_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unBaudRate);
		
		sprintf(tmpstr,"ComParity_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unParity);
		
		sprintf(tmpstr,"ComDataBits_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unDataBits);
		
		sprintf(tmpstr,"ComStopBits_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unStopBits);

	}	

    //通道锁定参数
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucWorkingTimeFlag);

    strcpy(tmpstr,"ucBeginTimeHour");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeHour);

    strcpy(tmpstr,"ucBeginTimeMin");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeMin);

    strcpy(tmpstr,"ucBeginTimeSec");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeSec);

    strcpy(tmpstr,"ucEndTimeHour");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeHour);

    strcpy(tmpstr,"ucEndTimeMin");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeMin);

    strcpy(tmpstr,"ucEndTimeSec");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeSec);
	
    strcpy(tmpstr,"ucReserved");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucReserved);

    add_more_key(section, "ucChannelStatus",pCustom->sChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    add_one_key(section, tmpstr, ((pCustom->cChannelLockFlag >= 1) ? 1 : pCustom->cChannelLockFlag));

	//多时段通道锁定参
	section = "MulPeroidsChanLock";
	add_one_key(section, "chanLockFlag", pCustom->sMulPeriodsChanLockParams.cLockFlag);
	for(i=0; i<16; i++)
	{
		sprintf(tmpstr, "MulPeroidsChanLock%02d", i+1);
		add_more_key(tmpstr, "channelStatus",pCustom->sMulPeriodsChanLockParams.chans[i].ucChannelStatus,32);		
		add_one_key(tmpstr, "workingTimeFlag", pCustom->sMulPeriodsChanLockParams.chans[i].ucWorkingTimeFlag);
		add_one_key(tmpstr, "beginTimeHour", pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeHour);
		add_one_key(tmpstr, "beginTimeMin", pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeMin);
		add_one_key(tmpstr, "beginTimeSec", pCustom->sMulPeriodsChanLockParams.chans[i].ucBeginTimeSec);
		add_one_key(tmpstr, "endTimeHour", pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeHour);
		add_one_key(tmpstr, "endTimeMin", pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeMin);
		add_one_key(tmpstr, "endTimeSec", pCustom->sMulPeriodsChanLockParams.chans[i].ucEndTimeSec);	
	}

    //特殊控制号
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    add_one_key(section, tmpstr, pCustom->cSpecialControlSchemeId);

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
void get_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc)
{
	parse_start(fileName);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //获取相位描述信息
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPhaseDescParams.stPhaseDesc[i]);

		if(0 != strlen((char *)pDesc->sPhaseDescParams.stPhaseDesc[i]))
		{
    		DBG("%s=%s\n",tmpstr,pDesc->sPhaseDescParams.stPhaseDesc[i]);
		}
	}	

    //获取通道信息描述
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sChannelDescParams.stChannelDesc[i]);

		if(0 != strlen((char *)pDesc->sChannelDescParams.stChannelDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sChannelDescParams.stChannelDesc[i]);
		}
	}	

    //获取方案信息描述
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPatternNameParams.stPatternNameDesc[i]);

		if(0 != strlen((char *)pDesc->sPatternNameParams.stPatternNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sPatternNameParams.stPatternNameDesc[i]);
		}
	}	
	
    //获取计划信息描述
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPlanNameParams.stPlanNameDesc[i]);

		if(0 != strlen((char *)pDesc->sPlanNameParams.stPlanNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sPlanNameParams.stPlanNameDesc[i]);
		}
	}	

	//获取日期信息描述
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sDateNameParams.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		pDesc->sDateNameParams.stNameDesc[i].dateType = get_one_value(section, tmpstr);

        if(0 != strlen((char *)pDesc->sDateNameParams.stNameDesc[i].dateName))
        {
            DBG("DateDesc_%02d=%s  dateType= %d\n",i+1,pDesc->sDateNameParams.stNameDesc[i].dateName,pDesc->sDateNameParams.stNameDesc[i].dateType);
        }
	}	

    parse_end();
}

void set_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc)
{
	parse_start(fileName);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //相位描述信息
	for(i = 0; i < 16; i ++)
	{
	    if(0 == strlen((char *)pDesc->sPhaseDescParams.stPhaseDesc[i]))
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPhaseDescParams.stPhaseDesc[i]);

	}	

    //通道信息描述
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		if(0 == strlen((char *)pDesc->sChannelDescParams.stChannelDesc[i]))
		{
            continue;
		}	
		
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sChannelDescParams.stChannelDesc[i]);


	}	

    //方案信息描述
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pDesc->sPatternNameParams.stPatternNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPatternNameParams.stPatternNameDesc[i]);


	}	
	
    //计划信息描述
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pDesc->sPlanNameParams.stPlanNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPlanNameParams.stPlanNameDesc[i]);
	}	

	//日期信息描述
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
        if(0 == strlen((char *)pDesc->sDateNameParams.stNameDesc[i].dateName))
        {
            continue;
        }
	
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sDateNameParams.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		add_one_key(section,tmpstr, pDesc->sDateNameParams.stNameDesc[i].dateType);
	}	

    parse_end();

}

/*****************************************************************************
 函 数 名  : ReadCountdowncfgFromIni
 功能描述  : 读出倒计时牌的配置信息
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月23日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadCountdowncfgFromIni(char *fileName,CountDownCfg *cfg)
{
	parse_start(fileName);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"Device_%d",i);

        cfg->cDeviceId[i] = i;
        get_more_value(tmpstr,"cControllerID",cfg->cControllerID[i],MAX_NUM_PHASE);
        cfg->cControllerType[i] = get_one_value(tmpstr,"cControllerType");

        //INFO("===>  id %d, cControllerId  %d, type %d \n",g_CountDownCfg.cDeviceId[i],g_CountDownCfg.cControllerID[i][0],g_CountDownCfg.cControllerType[i]);
    }

    parse_end();
}

/*****************************************************************************
 函 数 名  : WriteCountdownCfgToIni
 功能描述  : 写倒计时牌的配置信息
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月23日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void WriteCountdownCfgToIni(char *fileName,CountDownCfg *cfg)
{
	parse_start(fileName);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(cfg->cControllerType[i] == 0)//如果没有配置倒计时器，就不再显示
        {
            continue;
        }
        
        sprintf(tmpstr,"Device_%d",i);

        add_one_key(tmpstr,"cControllerType",cfg->cControllerType[i]);
        add_more_key(tmpstr,"cControllerID",cfg->cControllerID[i],MAX_NUM_PHASE);
    }

    parse_end();
}

void ReadMiscCfgFromIni(char *fileName,STRUCT_BINFILE_MISC *cfg)
{
	parse_start(fileName);

    char tmpstr[64] = "MISC";
    cfg->cIsCanRestartHiktscAllowed = get_one_value(tmpstr,"cControllerType");
    parse_end();
}

void WriteMiscCfgToIni(char *fileName,STRUCT_BINFILE_MISC *cfg)
{
	parse_start(fileName);

    char tmpstr[64] = "MISC";
    add_one_key(tmpstr,"cControllerType",cfg->cIsCanRestartHiktscAllowed);
    parse_end();
}





