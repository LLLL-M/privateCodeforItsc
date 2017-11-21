#ifndef __EXTERN_H__
#define __EXTERN_H__

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"

#ifdef __GNUC__
#define WEAKATTR    __attribute__((weak))
#define UNUSEDATTR  __attribute__((unused))
#else
#define WEAKATTR
#define UNUSEDATTR
#endif

#ifndef MAX_STAGE_NUM
#define MAX_STAGE_NUM	16
#endif
#ifndef MAX_PHASE_NUM
#define MAX_PHASE_NUM	16
#endif
#ifndef MAX_FOLLOWPHASE_NUM
#define MAX_FOLLOWPHASE_NUM MAX_PHASE_NUM
#endif
#ifndef MAX_CHANNEL_NUM
#define MAX_CHANNEL_NUM	32
#endif
#define MAX_BOARD_NUM	8		//点灯时总共的点灯数组个数

//特殊控制动作号
#define SINGLE_ADAPT_ACTIONID   112
#define	INDUCTIVE_ACTIONID		118
#define YELLOWBLINK_ACTIONID	119
#define	ALLRED_ACTIONID			116
#define TURNOFF_ACTIONID		115
#define INDUCTIVE_COORDINATE_ACTIONID	114

//特殊控制方案号
#define	INDUCTIVE_SCHEMEID				254			//实时感应方案号
#define YELLOWBLINK_SCHEMEID			255			//黄闪方案号
#define	ALLRED_SCHEMEID					252			//全红方案号
#define TURNOFF_SCHEMEID				251			//关灯方案号
#define INDUCTIVE_COORDINATE_SCHEMEID	250			//感应协调控制
#define STEP_SCHEMEID           		249			//步进方案号
#define SINGLE_ADAPT_SCHEMEID			248			//自适应感应方案号	/*!!!!!!!!!!!!宏定义名字有修改!!!!!!!!!!!*/
#define CHANNEL_LOCK_SCHEMEID	247			//通道锁定方案号,当执行通道锁定时只在上载实时状态的方案号时用到,目前用以优控平台
#define SYSTEM_RECOVER_SCHEMEID	0			//恢复系统控制方案

#include "calculate.h"

//故障日志类型定义
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
	TIMEINTERVAL_COORDINATE_INDUCTIVE = 0x4a,		//本地协调感应
	SENDMSG_FAIL = 0x4b,							//发送消息失败
	//added by kevin
	MANUAL_PANEL_AUTO=0x4c,							//手动控制面板自动按键
	MANUAL_PANEL_MANUAL=0x4d,						//手动控制面板手动按键
	MANUAL_PANEL_EAST=0x4e,							//手动控制面板东放行按键
	MANUAL_PANEL_SOUTH=0x4f,						//手动控制面板南放行按键
	MANUAL_PANEL_WEST=0x50,							//手动控制面板西放行按键
	MANUAL_PANEL_NORTH=0x51,						//手动控制面板北放行按键
	MANUAL_PANEL_EASTWEST_DIRECT=0x52,				//手动控制面板东西直行按键
	MANUAL_PANEL_SOUTHNORTH_DIRECT=0x53,			//手动控制面板南北直行按键
	MANUAL_PANEL_EASTWEST_LEFT=0x54,				//手动控制面板东西左转按键
	MANUAL_PANEL_SOUTHNORTH_LEFT=0x55,				//手动控制面板南北左转按键
	WIRELESS_KEY_AUTO=0x56,							//无线遥控器自动按键
	WIRELESS_KEY_MANUAL=0x57,						//无线遥控器手动按键
	WIRELESS_KEY_YELLOWBLINK=0x58,					//无线遥控器黄闪按键
	WIRELESS_KEY_ALLRED=0x59,						//无线遥控器全红按键
	WIRELESS_KEY_STEP=0x5a							//无线遥控器步进按键	
} FaultLogType;

typedef void (*UploadFaultLogFunc)(void *netArg, void *data, int datasize);	//上载故障日志的函数

//故障信息结构
typedef struct _fault_log_info
{
	int nNumber;    		//序列号
	int nID;                //消息类型ID,对应于枚举FaultLogType
	long nTime;        		//发生时间,北京时间，单位为秒
	int nValue;         	//时间值(通道号)
} FaultLogInfo;

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
	
	OFF_GREEN = 100,	//绿灯脉冲倒计时，第一个250ms关灯
	OFF_YELLOW = 101,	//绿灯脉冲倒计时，第一个250ms关灯
	OFF_RED = 102,		//红灯脉冲倒计时，第一个250ms关灯
} LightStatus;

typedef enum
{	//控制优先级依次从低到高
	AUTO_CONTROL = 0,	//自动控制
	WEB_CONTROL,		//web网页控制
	TOOL_CONTROL,		//配置工具控制
	PLATFORM_CONTROL,	//平台控制
	KEY_CONTROL,		//按键控制
	FAULT_CONTROL,		//故障控制
} ControlType;	//控制类型

typedef struct
{
	UInt16 splitTime;			//相位绿信比时间
	UInt16 phaseSplitLeftTime;	//相位执行时在绿信比内所剩余的时间
	UInt8 phaseStatus;			//相位当前的状态,取值范围为枚举LightStatus所定义
	UInt16 phaseLeftTime;		//相位剩余的绿灯时间或是红灯时间
	
	UInt8 pedestrianPhaseStatus;//行人相位状态
	UInt16 pedestrianPhaseLeftTime;//行人相位剩余的绿灯时间或是红灯时间
	
	UInt8 followPhaseStatus;	//跟随相位的状态
	UInt16 followPhaseLeftTime;	//跟随相位剩余的绿灯时间或是红灯时间
	
	UInt64 vehicleDetectorBits;	//相位对应的车检器号
	UInt8 unitExtendGreen;		//单位延长绿
	UInt8 maxExtendGreen;		//最大可以延长的绿灯时间
	UInt8 maxExtendGreen2;      //自适应控制的最大可以延长的绿灯时间
	UInt32 motorChannelBits;	//相位对应的机动车通道，bit0-bit31分别代表通道1-32
	UInt32 pedChannelBits;		//相位对应的行人通道，bit0-bit31分别代表通道1-32
} PhaseInfo;

typedef struct
{
	UInt8 allChannels[MAX_CHANNEL_NUM];	//所有通道的状态
	UInt16 channelCountdown[MAX_CHANNEL_NUM];	//通道的倒计时
	UInt16 cycleTime;				//当前周期总时间
	UInt16 inductiveCoordinateCycleTime;	//感应协调控制周期时间
	//UInt8 runTime;				//当前周期已经运行了多少时间
	UInt16 leftTime;					//当前周期还剩下多少时间
	UInt8 stageNum;					//当前运行的阶段号
	UInt8 maxStageNum;				//最大阶段号
	Boolean isStep;					//是否正在步进
	UInt8 schemeId;                 //当前运行的方案号
	UInt8 phaseTableId;				//当前使用的相位表号
	UInt8 channelTableId;			//当前使用的通道表号
	UInt8 actionId;					//动作号
	UInt8 checkTime;				//感应检测时间
	UInt8 collectCycle;				//流量采集周期，单位s

    UInt32 motorChanType;           //配置的机动车，跟随类型通道
    UInt32 pedChanType;             //配置的行人，行人跟随类型通道
    
	PhaseInfo phaseInfos[MAX_PHASE_NUM];	//所有相位的信息
} LineQueueData;	//线性队列中存放的数据

typedef struct 
{
    UInt16 L0:3;
    UInt16 L1:3;
    UInt16 L2:3;
    UInt16 L3:3;
    UInt16 unused:4;
} lamp_t;

typedef void *(*threadProcessFunc)(void *arg);
struct threadInfo
{
	pthread_t id;				//线程号，用来在ItsExit时释放线程资源使用
	threadProcessFunc func;		//线程的处理函数
	char *moduleName;			//线程的名称
	UInt32 countvalue;			//计数值，主要用来判断相位控制模块和驱动模块是否一直处于睡眠状态
};

//通道锁定结构体
typedef struct channel_lock_params
{
#define CHANNEL_UNLOCK			0	//通道未锁定
#define CHANNEL_LOCK			1	//通道锁定
#define CHANNEL_WAIT_FOR_LOCK	2	//通道待锁定
#define CHANGEABLE_CHANNEL_LOCK	3	//可变通道锁定
	unsigned char    ucChannelLockStatus;		//通道锁定状态，如上述宏定义所述
	unsigned char    ucChannelStatus[MAX_CHANNEL_NUM];		//32个通道锁定的状态，具体值如枚举LightStatus所述
	unsigned char    ucWorkingTimeFlag;		//时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
	unsigned char    ucBeginTimeHour;			//控制起效时间：小时
	unsigned char    ucBeginTimeMin;			//控制起效时间：分钟
	unsigned char    ucBeginTimeSec;			//控制起效时间：秒
	unsigned char    ucEndTimeHour;			//控制结束时间：小时
	unsigned char    ucEndTimeMin;			//控制结束时间：分钟
	unsigned char    ucEndTimeSec;			//控制结束时间：秒
} ChannelLockParams;

//忽略相位的相关设置
typedef enum    //对于忽略相位时间的处理
{
    NONE_IGNORE = 0,        //不忽略
    FORWARD_IGNORE = 1,     //向前忽略，即把忽略相位时间给前面的相位
    BACKWARD_IGNORE = 2,    //向后忽略，即把忽略相位时间给后面的相位
    ALL_IGNORE = 3,         //前后都忽略，即把忽略相位的时间平分给前面和后面的相位
} IgnoreAttr;

typedef struct
{
	Boolean isContinueRun;			//黄闪等特殊控制之后是否接着之前的周期继续运行
	Boolean addTimeToFirstPhase;	//感应协调联合控制时是否把剩余时间累加到第一个相位
} CustomParams;

typedef struct
{
	Boolean enableRedSignal;		//红灯信号使能
	char mcastIp[16];				//组播ip
	UInt16 mcastPort;				//组播端口
	UInt16 deviceId;				//设备id
} McastInfo;

/***************************************
以下函数需要用户自己去实现
***************************************/
//申请存放配置的内存
WEAKATTR extern void *ItsAllocConfigMem(void *config, int configSize);
//初始化单元数据
WEAKATTR extern void ItsUnitInit(void (*initFunc)(UInt8, UInt8));
//填充计算信息
struct _CalInfo;
WEAKATTR extern Boolean FillCalInfo(struct _CalInfo *calInfo, UInt8 schemeId, time_t calTime);
//对于忽略相位的处理
WEAKATTR extern void IgnorePhaseDeal(struct _CalInfo *calInfo);
//设置实时信息
WEAKATTR extern void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data);
//点灯输出
WEAKATTR extern void ItsLight(int boardNum, unsigned short *poutLamp);
//故障检测,参数为8组点灯数组的指针
WEAKATTR extern void ItsFaultCheck(UInt16 *lightValues);
//读车检板数据
WEAKATTR extern UInt64 ItsReadVehicleDetectorData();
//读行人按键
//extern UInt16 PedestrianCheckInput(void);
/*参数data为当前1s中信号机使用详细信息，用来做定制使用，
 * 此函数的执行过程不能超过700ms，如果用不上定制可以不予实现 */
WEAKATTR extern void ItsCustom(LineQueueData *data, CustomParams *customParams);
//预留的倒计时器的输出接口，参数为从1相位开始的num个相位的所有相关信息
WEAKATTR extern void ItsCountDownOutput(LineQueueData *data);
WEAKATTR void ItsSetCurRunData(LineQueueData *data);

WEAKATTR extern unsigned char ChannelControl(unsigned char *chan);//通道锁定
WEAKATTR extern void channelLockTransition(unsigned char lockFlag, unsigned char *curStatus, unsigned char *lockstatus);//通道锁定过渡实现接口
WEAKATTR extern unsigned char GetBootTransitionTime(void);//获取系统启动时的黄闪全红时间

/*********************************************
以下是libits库提供的一些可供外部程序调用的接口
*********************************************/
//libits库的初始化、退出、线程状态检测
/*	threads:需要和libits一起初始化的线程相关信息的指针
	num:线程的个数	*/
extern Boolean ItsInit(struct threadInfo *threads, int num, void *config, int configSize);
extern void ItsExit(void);
extern void ItsThreadCheck(void);
//libits库的设置和获取配置信息
extern void ItsSetConfig(void *config, int configSize);
extern void ItsGetConfig(void *config, int configSize);
//libits库的控制
/*	type:控制方式，具体请依照枚举ControlType
	mschemeid:手动控制时的方案号
	stageNum:步进的阶段号	*/
extern void ItsCtl(ControlType type, UInt8 schemeId, int val);
//非阻塞发送控制消息
extern void ItsCtlNonblock(ControlType type, UInt8 schemeId, int val);
//通道的加锁、解锁以及通道检测

extern void ItsChannelCheck(UInt8 channelId, LightStatus status);
//获取平台倒计时信息
extern void ItsCountDownGet(void *countdown, int size);
extern void ItsGetCurRunData(LineQueueData *data);
//获取当前的控制状态，具体内容请参详platform.h中的STRU_CoordinateStatus结构体变量byUnitControlStatus
extern UInt8 ItsControlStatusGet(void);
//故障日志的读取、写入和清除
extern void ItsReadFaultLog(int startLine, int lineNum, void *netArg, int netArgSize, UploadFaultLogFunc func);
extern void ItsWriteFaultLog(FaultLogType type, int value);
extern void ItsClearFaultLog(void);
//黄闪、全红、关灯等特殊控制后是否继续接着之前的周期运行
extern void ItsIsContinueRun(Boolean val);
//设置组播相关的信息
extern void ItsSetMcastInfo(McastInfo *);

#endif

