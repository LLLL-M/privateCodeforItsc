/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : ConfigureManagement.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年7月31日
  最近修改   :
  功能描述   : ConfigureManagement.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年7月31日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __CONFIGUREMANAGEMENT_H__
#define __CONFIGUREMANAGEMENT_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include <unistd.h>
#include "platform.h"
#include "common.h"
#include "hik.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define IS_FILE_EXIST(fileName)     ((access((fileName),F_OK) == 0) ? 1 : 0)   

#define FILE_HIK_CFG_INI            "/home/config.ini"      //全局参数配置文件，包括通道表、相位表等信号机运行必须参数及GPS开关等配置信息及故障序列号等信息
#define FILE_DESC_INI               "/home/desc.ini"        //参数描述配置文件，包括通道描述、方案描述等信息
#define FILE_CUSTOM_INI             "/home/custom.ini"      //定制信息配置文件 ，包括倒计时、串口、电流参数等
#define FILE_COUNTDOWN_INI          "/home/countdown.ini"   //整合所有倒计时接口，公用同一个配置文件。
#define FILE_MISC_INI               "/home/misc.ini"        //整合所有混杂的没有一定归类的配置参数，大家以后如果需要临时添加一些配置的话，可以放到该文件中
#define FILE_TSC_CFG_INI            "/home/hikconfig.ini"

#define FILE_HIK_CFG_DAT        "/home/config.dat"      //全局参数配置文件，包括通道表、相位表等信号机运行必须参数及GPS开关等配置信息及故障序列号等信息
#define FILE_DESC_DAT           "/home/desc.dat"        //参数描述配置文件，包括通道描述、方案描述等信息
#define FILE_CUSTOM_DAT         "/home/custom.dat"      //定制信息配置文件 ，包括倒计时、串口、电流参数等
#define FILE_COUNTDOWN_DAT      "/home/countdown.dat"   //整合所有倒计时接口，公用同一个配置文件。
#define FILE_MISC_DAT           "/home/misc.dat"        //整合所有混杂的没有一定归类的配置参数，大家以后如果需要临时添加一些配置的话，可以放到该文件中
#define FILE_VEHICLE_DAT        "/home/vehicle.dat"     //记录了单位周期车检器收到的数据及其统计结果
#define FILE_TSC_CFG_DAT        "/home/hikconfig.dat"   //这是新库运行必须的包含方案配置的文件，缺少该文件，老库的倒计时接口将会受到影响。

#define READ_BIN_CFG_PARAMS(FILENAME,CFG,SIZE)                  ReadBinCfgInfo(FILENAME,CFG,SIZE,0,1)    
#define READ_BIN_CFG_PARAMS_OFFSET(FILENAME,CFG,SIZE,OFFSET)    ReadBinCfgInfo(FILENAME,CFG,SIZE,OFFSET,1)    
#define WRITE_BIN_CFG_PARAMS(FILENAME,CFG,SIZE)                 WriteBinCfgInfo(FILENAME,CFG,SIZE)   
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
#pragma pack(push,1)

//config.dat
typedef struct
{
	STRU_SPECIAL_PARAMS sSpecialParams;											//特殊参数定义结构体
	UINT8				cReservedValue1[512];									//针对特殊参数定义结构体的保留字节
	UINT8				cCarDetectSwitch;										//车检板开关
	UINT8 				cPrintfLogSwitch;										//日志打印开关
	UINT32				cFailureNumber;											//错误序列号
	STRU_CURRENT_PARAMS	sCurrentParams[32];										//电流参数表
	STRU_WIRELESS_CONTROLLER_INFO	stWirelessController;						//无线控制器模块参数
	UINT8				cReservedValue2[512-(9+(MAX_WIRELESS_KEY-1)*64)];								
}STRUCT_BINFILE_CONFIG;

//custom.dat
typedef struct
{
	STRU_Count_Down_Params	sCountdownParams;									//针对倒计时牌的配置
	UINT8                   cIsCountdownValueLimit;                             //倒计时值是否受感应检测时间的限制，默认是不受限的.
	UINT8 					cReservedValue1[512 - 8 - 1];	//保留字节,减去8是因为增加黄灯、红灯闪烁、倒计时是否受限的功能
	COM_PARAMS 				sComParams[4];										//针对串口参数的配置
	UINT8 					cReservedValue2[512];
	CHANNEL_LOCK_PARAMS 	sChannelLockedParams;								//针对通道锁定参数的配置
	STRU_Channel_Lock_V2_INFO	sNewChannelLockedParams;							//针对新通道锁定参数的配置
	UINT8 					cReservedValue3[512];
	UINT8					cChannelLockFlag;									//通道锁定标识//0表示未锁定，1表示锁定,2表示待锁定（待锁定状态表示收到了通道锁定命令但是当前时间为非锁定时间段的状态）
	UINT8					cSpecialControlSchemeId;							//特殊控制方案号
}STRUCT_BINFILE_CUSTOM;

//desc.dat
typedef struct
{
	PHASE_DESC_PARAMS 		sPhaseDescParams;									//相位描述 
	UINT8 					cReservedValue1[512];
	CHANNEL_DESC_PARAMS		sChannelDescParams;   								//通道描述
	UINT8 					cReservedValue2[512];
	PATTERN_NAME_PARAMS 	sPatternNameParams;   								//方案描述
	UINT8 					cReservedValue3[512];
	PLAN_NAME_PARAMS 		sPlanNameParams; 							        //计划描述
	UINT8 					cReservedValue4[512];
	DATE_NAME_PARAMS 		sDateNameParams;							        //日期描述
	UINT8				    cReservedValue6[512];
}STRUCT_BINFILE_DESC;

//misc.dat
typedef struct
{
    UINT8                   cIsCanRestartHiktscAllowed;                         //是否运行CAN监控线程重启hikTSC程序
                                                                                //
}STRUCT_BINFILE_MISC;



#pragma pack(pop)
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void CopyChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_PARAMS *pChannelLockedParams);
extern void CopyNewChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_V2_PARAMS *pChannelLockedParams);
extern void ReadBinAllCfgParams(STRUCT_BINFILE_CONFIG *config,STRUCT_BINFILE_CUSTOM *custom,
                                STRUCT_BINFILE_DESC *desc,CountDownCfg *countdown,
                                STRUCT_BINFILE_MISC *misc);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __CONFIGUREMANAGEMENT_H__ */
