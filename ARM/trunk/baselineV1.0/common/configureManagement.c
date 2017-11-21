/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : configureManagement.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年8月4日
  最近修改   :
  功能描述   : 配置管理模块，提供总接口，用来读取配置信息。以后我们对于配置
               文件的读取，仅在开机启动后读取一次，且读取都调用唯一接口。

               新增配置信息的步骤:

               1. 在

               
  函数列表   :
              CopyChannelLockInfoToCountDownParams
              ReadBinAllCfgParams
  修改历史   :
  1.日    期   : 2015年8月4日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include "configureManagement.h"
#include "binfile.h"
#include "inifile.h"

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
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息
extern unsigned char finalChannelStatus[32];         

STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述pthread_t thread;//接受电流电压消息的CAN句柄
CountDownCfg        g_CountDownCfg;              //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

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
    //memcpy(pCountDownParams->ucChannelStatus,pChannelLockedParams->ucChannelStatus,sizeof(pChannelLockedParams->ucChannelStatus));	
    memcpy(pCountDownParams->ucChannelStatus,finalChannelStatus,sizeof(pChannelLockedParams->ucChannelStatus));
}

/*****************************************************************************
 函 数 名  : ReadBinAllCfgParams
 功能描述  : 上电启动后，把所有二进制参数一次性都都出来
 输入参数  : STRUCT_BINFILE_CONFIG *config  
             STRUCT_BINFILE_CUSTOM *custom  
             STRUCT_BINFILE_DESC *desc      
             CountDownCfg *countdown      
             实现逻辑:
             1. 先判断dat文件是否存在，如果已存在，则表明是新版本，就直接读取二进制文件;这里如果dat文件不存在或大小为0，会自动读取备份文件。
             2. 如果dat文件不存在，则可能是老版本或者是升级包没有将文本文件转成二进制，则读取文本文件，读取结束后，将文本文件删除。
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年7月31日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReadBinAllCfgParams(STRUCT_BINFILE_CONFIG *config,STRUCT_BINFILE_CUSTOM *custom,
                                STRUCT_BINFILE_DESC *desc,CountDownCfg *countdown,
                                STRUCT_BINFILE_MISC *misc)
{
    if(READ_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,config,sizeof(STRUCT_BINFILE_CONFIG)) == 0)
    {
        if(IS_FILE_EXIST(FILE_HIK_CFG_INI) == 1)
        {
            //获取config.ini中有关GPS开关、电流参数、故障号及其他参数信息
            get_special_params(FILE_HIK_CFG_INI,config);      
            WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,config,sizeof(STRUCT_BINFILE_CONFIG));
            unlink(FILE_HIK_CFG_INI);
        }
    }
#if 0
    INFO("Just For TianJing NingHe , Set GPS = 1, Watchdog = 1\n");
    config->sSpecialParams.iGpsSwitch = 1;
    config->sSpecialParams.iWatchdogSwitch = 1;
#endif
    if(READ_BIN_CFG_PARAMS(FILE_DESC_DAT,desc,sizeof(STRUCT_BINFILE_DESC)) == 0)
    {
        //获取desc.ini中关于相位描述、通道描述等信息
        if(IS_FILE_EXIST(FILE_DESC_INI) == 1)
        {
            get_desc_params(FILE_DESC_INI,desc); 
            WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT,desc,sizeof(STRUCT_BINFILE_DESC));
            unlink(FILE_DESC_INI);
        }
    }

    if(READ_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,custom,sizeof(STRUCT_BINFILE_CUSTOM)) == 0)
    {
         //获取自定义配置文件中关于倒计时、串口等参数的配置信息
        if(IS_FILE_EXIST(FILE_CUSTOM_INI) == 1)
        {
            get_custom_params(FILE_CUSTOM_INI,custom); 
            WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,custom,sizeof(STRUCT_BINFILE_CUSTOM));
            unlink(FILE_CUSTOM_INI);
        }
    }

    custom->cSpecialControlSchemeId = 0;//特殊控制方案号虽然保存在配置中，但已经不再使用。
    CopyChannelLockInfoToCountDownParams(gCountDownParams,&custom->sChannelLockedParams);
    if(READ_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,countdown,sizeof(CountDownCfg)) == 0)
    {
        //倒计时器的设置参数
        if(IS_FILE_EXIST(FILE_COUNTDOWN_INI) == 1)
        {
            ReadCountdowncfgFromIni(FILE_COUNTDOWN_INI,countdown);
            WRITE_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,countdown,sizeof(CountDownCfg));
            unlink(FILE_COUNTDOWN_INI);
        }
    }
    //使用新源倒计时器的话，倒计时有个bug，必须配置ID为0的倒计时，因此，开机时如果发现没有配置ID为0的倒计时器，就默认添加一个.
    if(countdown->cControllerType[0] == 0)
    {
        countdown->cControllerType[0] = 2;
        countdown->cDeviceId[0] = 0;
        countdown->cControllerID[0][0] = 1;
    }

    //读取混杂的配置参数，不能以后自定义的一些开关量都可以放在这里
    if(READ_BIN_CFG_PARAMS(FILE_MISC_DAT,misc,sizeof(STRUCT_BINFILE_MISC)) == 0)
    {
        //倒计时器的设置参数
        if(IS_FILE_EXIST(FILE_MISC_INI) == 1)
        {
            ReadMiscCfgFromIni(FILE_MISC_INI,misc);
            WRITE_BIN_CFG_PARAMS(FILE_MISC_DAT,misc,sizeof(STRUCT_BINFILE_MISC));
            unlink(FILE_MISC_INI);
        }
    }
}



