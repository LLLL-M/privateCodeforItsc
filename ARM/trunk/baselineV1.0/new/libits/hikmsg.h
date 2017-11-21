/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : hikmsg.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月29日
  最近修改   :
  功能描述   : hikmsg.h 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HIKMSG_H__
#define __HIKMSG_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <netinet/in.h>

#include "hik.h"
#include "its.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

 //特殊控制动作号
#define	INDUCTIVE_ACTIONID		118
#define YELLOWBLINK_ACTIONID	119
#define	ALLRED_ACTIONID			116
#define TURNOFF_ACTIONID		115

//#define MSGZERO 0	//表明发送的消息没有数据
#define MSGSIZE	128	//表明发送消息数据的长度

#define AHEAD_OF_TIME	6	//下一个周期提前6s开始计算

#define ONE_SECOND_NSECS		1000000000					//1s换算的ns数值
#define DELAY_TIME				250							//延时时间，单位是ms，即每DELAY_TIMME ms延时一次
//每次延时之前先用usleep延时的时间，预留50ms做精确延时
#define DELAY_TIME_USECS_BEFORE	((DELAY_TIME - 50) * 1000)	
#define DELAY_TIME_NSECS		(DELAY_TIME * 1000000)			//每次延时时间换算的纳秒数
#define LOOP_TIMES_PER_SECOND	(ONE_SECOND_NSECS / DELAY_TIME_NSECS)	//每秒钟轮询发送点灯数值次数
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef enum 
{
	MSG_NEW_CONFIG = 1,			//有新的配置下发，通信模块发送给数据管理模块
	
	MSG_CONTROL_TYPE,		//通信模块和数据采集模块发送给策略控制模块的控制类型消息
	
	MSG_FAULT_LOG,	//故障日志信息
	
	MSG_CONTROL_MODE,		//策略控制摸发给相位驱动模块的控制方式
	MSG_BEGIN_READ_DATA,	//计算模块发送给相位驱动模块开始读取数据
	
	MSG_START_CALCULATE_NEXT_CYCLE,	//开始计算下一周期
	
	MSG_START_TIMER,		//相位控制模块发送给定时器模块
	MSG_CHANNEL_STATUS,		//相位控制模块发送给相位驱动模块的通道状态

	MSG_CHANNEL_CHECK,		//通信模块发送给相位控制模块的消息
	
	MSG_RED_SIGNAL_CHECK,	//相位控制模块发送给红灯信号检测器模块，每秒一次
} MsgType;		//消息类型编号

typedef enum
{
	SYSTEM_MODE = 0,		//系统控制模式
	TURNOFF_LIGHTS_MODE = 1,	//关灯控制模式
	YELLOWBLINK_MODE = 2,		//黄闪控制模式
	ALLRED_MODE = 3,			//全红控制模式
	INDUCTIVE_MODE = 6,			//感应控制模式
	//以下4个是国标新增的控制方式
	SINGLE_SPOT_OPTIMIZE = 8,		//单点优化
	MASTER_SLAVE_LINE_CONTROL = 11,	//主从线控
	SYSTEM_OPTIMIZE = 12,			//系统优化
	INTERPOSE_LINE_CONTROL = 13,	//干预线控
	
	MANUAL_MODE,			//手动控制模式
	STEP_MODE,				//步进控制模式
} ControlMode;	//控制方式


struct msgbuf
{
	long mtype;		//消息类型，具体的类型如 MsgType 所描述
	
	union
	{
		char mtext[MSGSIZE];	//其他模块发送给故障日志模块的故障信息
		char wFlag;		//配置信息是否写flash的标志，0:不写入，1:写入

		struct 
		{
			ControlType	controlType;	//策略控制模块接收的控制类型
			ControlMode mode;		//策略控制模块发给相位控制模块的控制方式
			UInt8 stageNum;		//步进阶段号，单步步进时为0，
			UInt8 mSchemeId;	//表示手动方案号
		} controlMsg;

		//相位控制模块发送给计算模块
		struct
		{
			UInt8 schemeId;		//开始计算的下一周期所应当使用的方案，0:表示按照本地系统方案
			time_t calTime;		//开始计算下一周期的时间
		} calMsg;

		UInt8 allChannels[NUM_CHANNEL];		//相位控制模块发送给相位驱动模块的通道状态
		//通道检测的相关参数
		struct
		{
			UInt8 channelId;
			LightStatus status;
		} channelCheckParam;

		struct
		{
#define READ_FAULT_LOG	1
#define WRITE_FAULT_LOG	2
#define CLEAR_FAULT_LOG	3
			UInt8 rwflag;
			FaultLogType type;
			time_t time;
			int value;
			int startLine;
			int lineNum;
			int socketfd;
			struct sockaddr dstaddr;
		} faultLogInfo;
	} mdata;
//以下是struct msgbuf内部数据成员的一些宏定义
#define msgText					mdata.mtext
#define msgwFlag				mdata.wFlag
#define	msgControlType			mdata.controlMsg.controlType
#define msgMode					mdata.controlMsg.mode
#define msgStageNum				mdata.controlMsg.stageNum
#define msgmSchemeId			mdata.controlMsg.mSchemeId
#define msgSchemeId	    		mdata.calMsg.schemeId
#define msgCalTime				mdata.calMsg.calTime
#define msgAllChannels			mdata.allChannels
#define msgChannelId			mdata.channelCheckParam.channelId
#define msgChannelStatus		mdata.channelCheckParam.status
#define msgFLrwflag				mdata.faultLogInfo.rwflag
#define msgFLtype				mdata.faultLogInfo.type
#define msgFLtime				mdata.faultLogInfo.time
#define	msgFLvalue				mdata.faultLogInfo.value
#define msgFLstartLine			mdata.faultLogInfo.startLine
#define msgFLlineNum			mdata.faultLogInfo.lineNum
#define msgFLsocketfd			mdata.faultLogInfo.socketfd
#define msgFLdstaddr			mdata.faultLogInfo.dstaddr
};

typedef struct 
{
	UInt8 checkTime;		//感应检测时间
	UInt8 redFlashSec;		//红灯倒计时闪烁秒数
	UInt8 isYellowFlash:1;	//黄灯是否闪烁
	UInt8 ignoreOption:2;	//0:忽略相位不予处理,1:向前忽略,2:向后忽略,3:前后忽略
	UInt8 isContinueRun:1;	//特殊控制切换到系统控制时是否继续运行
	//UInt8 phaseTakeOver:1;	//相位接管开关
} CustomInfo;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKMSG_H__ */

