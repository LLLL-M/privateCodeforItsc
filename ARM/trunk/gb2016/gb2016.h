#ifndef __GB2016_H__
#define __GB2016_H__

#include "hik.h"

#pragma pack(push, 1)

typedef struct
{
	UInt8	version;			//版本号
	UInt8	sendFlag;			//发送方标识
	UInt8	recvFlag;			//接收方标识
	UInt8	dataLinkCode;		//数据链路码
	UInt8	areaNumber;			//区域号
	UInt16	roadNumber;			//路口号
	UInt8	operatorType;		//操作类型
	UInt8	objectId;			//对象标识
	UInt8	reserve[5];			//保留
} FrameHead;

typedef enum
{
	COMMUNICATION_RULE_LINK = 1,	//通信规程链路
	BASIC_INFORMATION_LINK = 2,		//基本信息链路
	FEATURES_PARAMETER_LINK = 3,	//特征参数一般交互链路
	INTERVENE_COMMAND_LINK = 4,		//干预指令链路
} DataLinkCode;		//数据链路码

typedef enum
{
	GB_QUERY_REQ = 0x80,		//查询请求
	GB_SET_REQ = 0x81,			//设置请求
	GB_ACTIVE_UPLOAD = 0x82,	//主动上报
	GB_QUERY_REPONSE = 0x83,	//查询应答
	GB_SET_REPONSE = 0x84,		//设置应答
	GB_ERR_REPONSE = 0x85,		//出错应答
} OperateType;	//操作类型

enum
{
	ONLINE = 1,					//联机
	TRAFFIC_FLOW_INFO = 2,		//交通流信息
	WORK_STATUS = 3,			//工作状态
	LAMP_STATUS = 4,			//灯色状态
	CURRENT_TINE = 5,			//当面时间
	SIGNAL_LAMP_GROUP = 6,		//信号灯组
	PHASE = 7,					//相位
	SIGNAL_TIMING_SCHEME = 8,	//信号配时方案
	SCHEME_SCHEDULE = 9,		//方案调度计划
	WORK_MODE = 10,				//工作方式
	SIGNAL_MACHINE_FAULT = 11,	//信号机故障
	SIGNAL_MACHINE_VER = 12,	//信号机版本
	FEATURES_PARAMETER = 13,	//特征参数
	SIGNAL_MACHINE_ID_CODE = 14,//信号机识别码
	REMOTE_CONTROL = 15,		//远程控制
	DETECTOR = 16,				//检测器
};

typedef struct
{
	UInt8	detectorId;			//检测器号
	UInt8	vehicleFlow;		//车流量
	UInt32	beginTime;			//起始时间
//	UInt8	occupancyRate;		//占有率
//	UInt8	vehicleSpeed;		//车速
//	UInt8	vehicleLength;		//车长
//	UInt8	queueLength;		//排队长度
} TrafficFlowInfo;	//交通流信息

typedef struct
{
	UInt8	ctrlType;					/*控制类型
										 0:本地时段控制
										 1:平台或客户端控制
										 2:按键控制
										 3:故障控制*/
	UInt8 	ctrlMode;					/*控制模式
										 1:关灯控制,可时段配置或手动执行
										 2:黄闪控制,可时段配置或手动执行
										 3:全红控制,可时段配置或手动执行
										 4:定周期控制,可时段配置或手动执行
										 5:协调控制,只能时段配置
										 6:感应控制,可时段配置或手动执行
										 7:单点优化控制,可时段配置或手动执行
										 8:步进控制,只能手动执行
										 10:行人感应控制,可时段配置或手动执行
										 11:公交优先控制,可时段配置或手动执行
										 */
    UInt8   ctrlId;                     /*使用的方案号[0,15],黄闪、全红、关灯时方案号为0
										  或者步进号(0:单步步进,其他:跳转步进)
										  或者通道锁定表号[1,15]
										 */
	UInt8	phase;						//当前运行的相位
	UInt16	cycleTime;					//当前的周期时间
} CurrentWorkStatus;	//信号机当前的工作状态

typedef enum
{
	LVOFF = 0,
	LVGREEN = 1,
	LVRED = 2,
	LVYELLOW = 3,
} LampValue;	//信号灯值

struct LampColorStatus
{
	UInt64	lamps;	//每2bit代表一个灯，最大支持32个灯，灯值如枚举LampValue
	UInt32	reserve;
};	//信号灯色状态

typedef struct
{
	UInt8	channelId;			//通道号,[1,32]
	UInt8	channelType;		//通道类型,0:未使用,1:机动车,2:行人,如上述枚举定义
	UInt32	conflictChannel;	//冲突通道，bit0-bit31分别代表通道1-32*/
	UInt8	direction;			//通道方向
	UInt8	flag;				//下载完成标志,0:未完成,1:完成
	UInt8	reserve[4];			//保留字节
} ChannelItem;	//通道

typedef struct
{
	UInt8	phaseId;			//相位号,[1,16]
	UInt8	greenTime;			//绿灯时间
	UInt8	greenBlinkTime;		//绿闪时间
	UInt8	yellowTime;			//黄灯时间
	UInt8	allRedTime;			//全红时间
	UInt8	autoRequest;		//自动请求
	UInt8	pedClearTime;		//行人清空时间，即行人绿闪时间
	UInt8	pedResponseTime;	//行人过街响应时间
	UInt8	minGreenTime;		//最小绿
	UInt8	maxGreenTime;		//最大绿
	UInt8	maxGreenTime2;		//最大绿2
	UInt8	unitExtendTime;		//单位延长绿
	UInt8	checkTime;			//感应检测时间
	UInt32	channelBits;		//相位包含的通道，bit0-bit31分别代表通道1-32
	UInt32 	vehDetectorBits;	//相位关联的车检器，bit0-bit32分别代表车检器1-32
	UInt8 	pedDetectorBits;	//相位关联行人或公交检测器，bit0-bit7分别代表车检器1-8
	UInt8	advanceExtendTime;	//优先延长时间
	UInt8	flag;				//下载完成标志,0:未完成,1:完成
} PhaseItem;	//相位

typedef struct
{
	UInt8	schemeId;			//方案号,[1,16]
	UInt16	cycleTime;			//周期
	UInt8	phaseOffset;		//相位差
	UInt8	coordinatePhase;	//协调相位
	UInt8	phaseturn[16];		//相序
	UInt8	flag;				//下载完成标志,0:未完成,1:完成
	UInt8	reserve[2];			//保留字节
} SchemeItem;	//方案

typedef struct
{
	UInt8	scheduleId;			//调度号,[1,255]
	UInt8	week;				//星期,[0,7],0:不使用星期,7表示星期日,1~6表示星期一到星期六
	UInt8	month;				//月份,[0,12],0:不使用月份,1~12表示一月到十二月
	UInt8	day;				//日期,[1,31]
	UInt8	startHour;			//开始执行小时,[0,23]
	UInt8	startMin;			//开始执行分钟,[0,59]
	UInt8	endHour;			//结束执行小时,[0,24]
	UInt8	endMin;				//结束执行分钟,[0,59]
	UInt8	ctrlType;			/*控制类型，0:本地时段控制*/
	UInt8 	ctrlMode;			/*控制模式
								 1:关灯控制,可时段配置或手动执行
								 2:黄闪控制,可时段配置或手动执行
								 3:全红控制,可时段配置或手动执行
								 4:定周期控制,可时段配置或手动执行
								 5:协调控制,只能时段配置
								 6:感应控制,可时段配置或手动执行
								 7:单点优化控制,可时段配置或手动执行
								 10:行人感应控制,可时段配置或手动执行
								 11:公交优先控制,可时段配置或手动执行
								 */
    UInt8   ctrlId;             /*使用的方案号[0,15],黄闪、全红、关灯时方案号为0*/
	UInt8	flag;				//下载完成标志,0:未完成,1:完成
} ScheduleItem;		//调度计划

typedef struct
{
	UInt32	seq;    		//序列号
	UInt16	type;           //日志类型
	UInt16	value;         	//时间值(通道号)
	UInt32	time;        	//发生时间
} FaultLog;			//故障日志

typedef struct
{
	UInt8	gps:1;					//GPS开关
	UInt8	watchdog:1;				//watchdog开关
	UInt8	voltCheck:1;			//电压检测开关
	UInt8	curCheck:1;				//电流检测开关
	UInt8	faultYellowFlash:1;		//故障黄闪开关
	UInt8	takeover:1;				//灯控接管开关
	UInt8	reserve:2;				//保留位
} SwtichParam;		//开关参数

typedef enum
{
	RESTART = 1,			//重启
	RECOVER_DEFAULT = 2,	//恢复默认
	FAULT_CLEAR = 3,		//故障清除
} RemoteControlCmd;		//远程控制指令

typedef struct
{
	UInt8	detectorId;			//检测器号,[1,32]
	UInt8	noResponseTime;		//无响应时间,单位为分钟
	UInt8	maxContinuousTime;	//最大持续时间,单位为分钟
	UInt8	maxVehcileNum;		//最大车辆数,单位为辆/分钟
	UInt8	detectorType;		//检测器类型,1：机动车检测器，2：行人检测器，3：公交优先检测器
	UInt8	flag;				//下载完成标志,0:未完成,1:完成
	UInt8	reserve[10];		//保留字节
} DetectorItem;		//检测器

typedef struct
{
	UInt8	areaNumber;				//区域编号,默认1
	UInt16	roadNumber;				//本区域路口编号,默认1
	char	version[20];			//信号机版本,默认V2.0.1.1
	char	identifyCode[20];		//信号机识别码,默认HK344H20170001
	UInt8	bootYellowBlinkTime;	//启动黄闪时间,默认6s
	UInt8	bootAllRedTime;			//启动全红时间,默认6s
	UInt16	vehFlowUploadCycleTime;	//交通流上报周期,单位为s，默认5min
	UInt8	transitionCycle;		//绿波控制时的过渡周期个数,默认2个周期
	char	ip[20];					//上位机ip
	UInt16	port;					//上位机端口
} Basic;

#pragma pack(pop)

#endif
