#ifndef __TSC_H__
#define __TSC_H__

#include "hik.h"

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

//特殊控制动作号
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

typedef enum
{
    SYSTEM_MODE = 0,        //系统控制模式
    TURNOFF_LIGHTS_MODE = 1,    //关灯控制模式
    YELLOWBLINK_MODE = 2,       //黄闪控制模式
    ALLRED_MODE = 3,            //全红控制模式
    INDUCTIVE_COORDINATE_MODE = 4,  //感应协调模式
    INDUCTIVE_MODE = 6,         //感应控制模式
    //以下4个是国标新增的控制方式
    SINGLE_SPOT_OPTIMIZE = 8,       //单点优化
    MASTER_SLAVE_LINE_CONTROL = 11, //主从线控
    SYSTEM_OPTIMIZE = 12,           //系统优化
    INTERPOSE_LINE_CONTROL = 13,    //干预线控

    MANUAL_MODE,            //手动控制模式
    STEP_MODE,              //步进控制模式
} ControlMode;  //控制方式

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
	bool isStep;					//是否正在步进
	UInt8 schemeId;                 //当前运行的方案号
	UInt8 phaseTableId;				//当前使用的相位表号
	UInt8 channelTableId;			//当前使用的通道表号
	UInt8 actionId;					//动作号
	UInt8 checkTime;				//感应检测时间
	UInt8 collectCycle;				//流量采集周期，单位s
	PhaseInfo phaseInfos[MAX_PHASE_NUM];	//所有相位的信息
} LineQueueData;	//线性队列中存放的数据

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
	bool enableRedSignal;		//红灯信号使能
	char mcastIp[16];				//组播ip
	UInt16 mcastPort;				//组播端口
	UInt16 deviceId;				//设备id
} McastInfo;

struct PassTimeInfo	//放行时间信息
{
	UInt16 greenTime;			//绿灯时间
	UInt8 greenBlinkTime;		//绿闪时间
	UInt8 yellowTime;			//黄灯时间
	UInt8 redYellowTime;		//红黄时间
	UInt8 allRedTime;			//全红时间
};

struct PhaseTimeInfo
{
	UInt16 splitTime;			//绿信比时间
	PassTimeInfo passTimeInfo;	//放行时间信息
	UInt8 pedestrianPassTime;	//行人放行时间
	UInt8 pedestrianClearTime;	//行人清空时间
	UInt8 pedAutoRequestFlag;	//行人自动请求标志，0:没有设置请求,1:设置了请求
	UInt64 vehicleDetectorBits;	//相位对应的车检器号
	UInt8 unitExtendGreen;		//单位延长绿
	UInt8 maxExtendGreen;		//最大可以延长的绿灯时间
	UInt32 motorChannelBits;	//相位对应的机动车通道，bit0-bit31分别代表通道1-32
	UInt32 pedChannelBits;		//相位对应的行人通道，bit0-bit31分别代表通道1-32
};

struct FollowPhaseInfo
{
	UInt16 phaseBits;			//跟随相位跟随的母相位，bit0-bit15分别代表相位1-16
	UInt32 motorChannelBits;	//跟随相位对应的机动车通道，bit0-bit31分别代表通道1-32
	UInt32 pedChannelBits;		//跟随相位对应的行人通道，bit0-bit31分别代表通道1-32
};

struct StageInfo
{
	PassTimeInfo passTimeInfo;					//放行时间信息
	UInt16 runTime;								//阶段运行时间
	bool isBarrierStart;						//阶段是否是屏障起始
	bool isBarrierEnd;						//阶段是否是屏障结束
	UInt8 includeNum;							//阶段包含相位的个数
	UInt8 includePhases[MAX_PHASE_NUM];	//阶段包含相位
};

struct CalInfo
{
	UInt8 timeIntervalId;					//时段表号
	UInt8 actionId;							//动作号
	UInt8 schemeId;							//方案号
	UInt8 splitId;							//绿信比号
	UInt8 phaseTurnId;						//相序号
	UInt8 phaseTableId;						//相位表号
	UInt8 channelTableId;					//通道表号
	UInt16 cycleTime;						//周期时间
	UInt16 inductiveCoordinateCycleTime;	//感应协调控制周期时间
	
	UInt8 transitionCycle;					//过渡周期
	UInt8 coordinatePhaseId;				//协调相位
	UInt16 isCoordinatePhase;				//是否是协调相位,每bit代表一个相位,0:不是,1:是
	UInt16 phaseOffset;						//相位差
	int timeGapSec;							//当前时间与时段起始时间差，单位为s
	
	UInt8 collectCycle;						//采集周期
	UInt8 checkTime;						//检测时间

	UInt16 isIgnorePhase;					//是否是忽略相位,每bit代表一个相位,0:不是,1:是
	
	PhaseTimeInfo phaseTimes[MAX_PHASE_NUM];	//相位时间相关的信息
	FollowPhaseInfo followPhaseInfos[MAX_FOLLOWPHASE_NUM];	//跟随相位的相关信息
	
	UInt8 maxStageNum;						//最大阶段号
	StageInfo stageInfos[MAX_STAGE_NUM];	//阶段相关信息
	
	UInt8 includeNums[MAX_PHASE_NUM];						//每个相位所包含的阶段号个数
	UInt8 phaseIncludeStage[MAX_PHASE_NUM][MAX_STAGE_NUM];	//相位所包含的阶段号
};

#endif
