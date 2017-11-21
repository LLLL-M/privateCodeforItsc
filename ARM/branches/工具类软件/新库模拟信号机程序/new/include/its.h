#ifndef __EXTERN_H__
#define __EXTERN_H__

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "HikConfig.h"
#include "platform.h"

#ifdef __GNUC__
#define WEAKATTR    __attribute__((weak))
#define UNUSEDATTR  __attribute__((unused))
#else
#define WEAKATTR
#define UNUSEDATTR
#endif

#define MAX_BOARD_NUM	8		//点灯时总共的点灯数组个数
//特殊控制方案号
#define	INDUCTIVE_SCHEMEID		254
#define YELLOWBLINK_SCHEMEID	255
#define	ALLRED_SCHEMEID			252
#define TURNOFF_SCHEMEID		251
#define STEP_SCHEMEID           249

typedef enum
{
	NTCIP = 0,
	GB2007 = 1,
} ProtocolType;

typedef enum
{	//控制优先级依次从低到高
	AUTO_CONTROL = 0,	//自动控制
	WEB_CONTROL,		//web网页控制
	TOOL_CONTROL,		//配置工具控制
	//PLATFORM_CONTROL,	//平台控制
	KEY_CONTROL,		//按键控制
	FAULT_CONTROL,		//故障控制
} ControlType;	//控制类型

//已实现的故障日志类型定义
typedef enum
{
	INVALID_TYPE = 0,								//无效的类型	
	//DISCONNECT_WITH_CENTER = 1,					//与中心断开
	//CONNECT_WITH_CENTER = 2,						//与中心断开后又联机
	COMMUNICATION_DISCONNECT = 3,					//通信断开
	COMMUNICATION_CONNECT = 4,						//通信断开后又联机
	//DETECTOR_NO_RESPONSE = 5,						//检测器无响应
	//DETECTOR_NO_RESPONSE_CLEAR = 6,				//检测器无响应清除
	//DOOR_OPEN = 7,								//开门
	//DOOR_CLOSE = 8,								//关门
	//POWER_OFF = 9,								//断电
	//POWER_ON = 0x0A,								//上电
	RED_GREEN_CONFLICT = 0x0B,						//红绿冲突
	RED_GREEN_CONFLICT_CLEAR = 0x0C,				//红绿冲突清除
	RED_LIGHT_OFF = 0x0D,							//红灯熄灭
	RED_LIGHT_OFF_CLEAR = 0x0E,						//红灯熄灭清除
	GREEN_CONFLICT = 0x0F,							//绿冲突
	GREEN_CONFLICT_CLEAR = 0x10,					//绿冲突清除
	//VOLTAGE_HIGH = 0x11,							//电压过高
	//VOLTAGE_HIGH_CLEAR = 0x12,					//电压过高清除
	//VOLTAGE_LOW = 0x13,							//电压过低
	//VOLTAGE_LOW_CLEAR = 0x14,						//电压过低清除
	GREEN_LIGHT_OFF = 0x15,							//绿灯熄灭
	GREEN_LIGHT_OFF_CLEAR = 0x16,					//绿灯熄灭清除
	AUTO_TO_MANUAL = 0x17,							//自动转手动控制
	MANUAL_TO_AUTO = 0x18,							//手动转自动控制
	//POW_ON_REBOOT = 0x19,							//上电重启
	UNNORMAL_OR_SOFTWARE_REBOOT = 0x1A,				//异常重启或是软件重启
	//COUNTDOWN_COMMUNICATION_FAULT = 0x25,			//倒计时牌通讯故障
	//COUNTDOWN_COMMUNICATION_FAULT_RECOVER = 0x26,	//倒计时牌通讯恢复
	FAULT_FLASH = 0x29,								//故障闪光
	TIMEINTERVAL_FLASH = 0x2B,						//时段闪光
	MANUAL_PANEL_FLASH = 0x2C,						//手动面板闪光
	TIMEINTERVAL_TURN_OFF = 0x2D,					//时段关灯
	TIMEINTERVAL_ALL_RED = 0x2E,					//时段全红
	MANUAL_PANEL_ALL_RED = 0x2F,					//手动面板全红
	MANUAL_PANEL_STEP = 0x30,						//手动面板步进

	//add by Jicky
	INIT_LOCAL_CONFIG_SUCC = 0x41,					//初始化本地配置成功
	INIT_LOCAL_CONFIG_FAIL = 0x42,					//初始化本地配置失败
	THREAD_EXCEPTION_EXIT = 0x43,					//线程异常退出
	LOCAL_CONFIG_UPDATE = 0x44,						//本地配置更新
	DOWNLOAD_CONFIG_CHECK_FAIL = 0x45,				//下载配置校验失败
	TIMEINTERVAL_INDUCTIVE = 0x46,					//本地感应
	CALCULATE_FAIL_CAUSE_FLASH = 0x47,				//计算模块计算失败引起的闪光
	WEB_CONTROL_LOG = 0x48,							//web网页控制日志
	TOOL_CONTROL_LOG = 0x49,						//工具控制日志
} FaultLogType;

typedef struct
{
	UInt8 splitTime;			//相位绿信比时间
	UInt8 phaseSplitLeftTime;	//相位执行时在绿信比内所剩余的时间
	UInt8 phaseStatus;			//相位当前的状态,取值范围为枚举LightStatus所定义
	UInt8 phaseLeftTime;		//相位剩余的绿灯时间或是红灯时间
	
	UInt8 pedestrianPhaseStatus;//行人相位状态
	UInt8 pedestrianPhaseLeftTime;//行人相位剩余的绿灯时间或是红灯时间
	
	UInt8 followPhaseStatus;	//跟随相位的状态
	UInt8 followPhaseLeftTime;	//跟随相位剩余的绿灯时间或是红灯时间
	
	UInt8 vehicleDetectorId;	//相位对应的车检器号
	UInt8 unitExtendGreen;		//单位延长绿
	UInt8 maxExtendGreen;		//最大可以延长的绿灯时间
} PhaseInfo;

typedef struct
{
	UInt8 allChannels[NUM_CHANNEL];	//所有通道的状态
	UInt8 cycleTime;				//当前周期总时间
	//UInt8 runTime;				//当前周期已经运行了多少时间
	UInt8 leftTime;					//当前周期还剩下多少时间
	UInt8 stageNum;					//当前运行的阶段号
	UInt8 maxStageNum;				//最大阶段号
	UInt8 schemeId;                 //当前运行的方案号
	UInt8 phaseTableId;				//当前使用的相位表号
	UInt8 channelTableId;			//当前使用的通道表号
	UInt8 checkTime;				//感应检测时间
	PhaseInfo phaseInfos[NUM_PHASE];	//所有相位的信息
} LineQueueData;	//线性队列中存放的数据

//描述相位或是通道当前1s的状态
typedef enum 
{
    INVALID = 0,
	GREEN = 1,
	RED = 2,
	YELLOW = 3,
	GREEN_BLINK = 4,
	YELLOW_BLINK = 5,
	ALLRED = 6,
	TURN_OFF = 7,
	RED_BLINK = 8,
	RED_YELLOW = 9,
} LightStatus;

typedef struct 
{
    UInt16 L0:3;
    UInt16 L1:3;
    UInt16 L2:3;
    UInt16 L3:3;
    UInt16 unused:4;
} lamp_t;

/***************************************
以下5个函数需要用户自己去实现
***************************************/
//点灯输出
WEAKATTR extern void ItsLight(int boardNum, unsigned short *poutLamp);
//故障检测,参数为8组点灯数组的指针
WEAKATTR extern void ItsFaultCheck(UInt16 *lightValues);
//读车检板数据
WEAKATTR extern UInt16 ItsReadVehicleDetectorData(int boardNum);
//读行人按键
//extern UInt16 PedestrianCheckInput(void);
/*参数data为当前1s中信号机使用详细信息，用来做定制使用，
 * 此函数的执行过程不能超过700ms，如果用不上定制可以不予实现 */
WEAKATTR extern void ItsCustom(LineQueueData *data);
//预留的倒计时器的输出接口，参数为从1相位开始的num个相位的所有相关信息
WEAKATTR extern void ItsCountDownOutput(LineQueueData *data);

typedef void *(*threadProcessFunc)(void *arg);
struct threadInfo
{
	pthread_t id;				//线程号，用来在ItsExit时释放线程资源使用
	threadProcessFunc func;		//线程的处理函数
	char *moduleName;			//线程的名称
	UInt32 countvalue;			//计数值，主要用来判断相位控制模块和驱动模块是否一直处于睡眠状态
};

/*********************************************
以下是libits库提供的一些可供外部程序调用的接口
*********************************************/
//libits库的初始化、退出、线程状态检测
/*	threads:需要和libits一起初始化的线程相关信息的指针
	num:线程的个数	*/
extern Boolean ItsInit(struct threadInfo *threads, int num, ProtocolType type, void *config);
extern void ItsExit(void);
extern void ItsThreadCheck(void);
//libits库的设置和获取配置信息
extern void ItsSetConfig(ProtocolType type, void *config);
extern void ItsGetConfig(ProtocolType type, void *config);
//libits库的控制
/*	type:控制方式，具体请依照枚举ControlType
	mschemeid:手动控制时的方案号
	stageNum:步进的阶段号	*/
extern void ItsCtl(ControlType type, UInt8 schemeId, UInt8 stageNum);
//通道的加锁、解锁以及通道检测
extern void ItsChannelLock(CHANNEL_LOCK_PARAMS *lockparams);
extern void ItsChannelUnlock(void);
extern void ItsChannelCheck(UInt8 channelId, LightStatus status);
//获取平台倒计时信息
extern void ItsCountDownGet(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *countdown);
//获取当前的控制状态，具体内容请参详platform.h中的STRU_CoordinateStatus结构体变量byUnitControlStatus
extern UInt8 ItsControlStatusGet(void);
//故障日志的读取、写入和清除
extern void ItsReadFaultLog(int startLine, int lineNum, int socketfd, struct sockaddr *fromAddr);
extern void ItsWriteFaultLog(FaultLogType type, int value);
extern void ItsClearFaultLog(void);

//忽略相位的相关设置
typedef enum	//对于忽略相位时间的处理
{
	NONE_IGNORE = 0,		//不忽略
	FORWARD_IGNORE = 1,		//向前忽略，即把忽略相位时间给前面的相位
	BACKWARD_IGNORE = 2,	//向后忽略，即把忽略相位时间给后面的相位
	ALL_IGNORE = 3,			//前后都忽略，即把忽略相位的时间平分给前面和后面的相位
} IgnoreAttr;
extern void ItsSetIgnoreAttr(IgnoreAttr attr);

//设置红灯倒计时闪烁秒数
extern void ItsSetRedFlashSec(UInt8 sec);
//设置黄灯是否闪烁,FALSE:不闪烁，TRUE:闪烁
extern void ItsYellowLampFlash(Boolean val);
//是否在特殊控制切换到系统控制后接着之前的方案继续运行
extern void ItsIsContinueRun(Boolean val);
//获取实时的方案相关信息
extern void ItsGetRealtimePattern(MsgRealtimePattern *p);

#endif

