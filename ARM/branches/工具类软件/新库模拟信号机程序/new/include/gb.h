#ifndef __GB_H__
#define __GB_H__

#include "hik.h"

#define GB_EVENT_FILE	"/home/gbevent.dat"

#define	MAX_PHASE_LIST_NUM		16		//最大相位列表个数

#define GB_INDUCTIVE_SCHEME		252		//这个协议中没有说明，是我猜测的
#define	GB_ALLRED_SCHEME		253
#define GB_YELLOWFLASH_SCHEME	254
#define GB_TURNOFF_SCHEME		255

#pragma pack(push, 1)

typedef enum
{
	GB_QUERY_REQ = 0,		//查询请求
	GB_SET_REQ = 1,			//设置请求
	GB_SET_NO_REPONSE = 2,	//设置请求，但不需要确认应答
	GB_AUTO_UPLOAD = 3,		//主动上报
	GB_QUERY_REPONSE = 4,	//查询应答
	GB_SET_REPONSE = 5,		//设置应答
	GB_ERR_REPONSE = 6,		//出错应答
} GbOperateType;	//操作类型

typedef struct gbMsgHead
{
	UInt8 operateType:4;	//操作类型,赋值对应于GbOperateType枚举
	UInt8 objectNum:3;	//对象个数
	UInt8 default1:1;		//固定为1
} GbMsgTypeField;	//消息类型域

typedef struct gbObjectField
{
	UInt8 objectId;			//对象标识
	UInt8 childObject:6;	//子对象
	UInt8 indexNum:2;		//索引个数
	UInt8 indexs[0];		//索引值
} GbObjectField;

//错误类型
typedef enum
{
	MSG_LENGTH_TOO_LONG = 1,				//消息长度过长
	MSG_TYPE_ERR,							//消息类型错误
	SETTING_OBJECT_ERR,						//设置的对象值超出规定的范围
	MSG_LENGTH_TOO_SHORT,					//消息长度太短
	MSG_OTHER_ERR,							//不属于上述类型的其他错误
	MSG_IGNORE,								//忽略该对象
	MSG_OK,									//消息无误
}MSG_ERR_TYPE;

typedef struct gbModParamList
{
	UInt8 modNo;										//模块表行号(索引)，范围[1,16]
	UInt8 modDevNodeLen;								//模块设备节点长度
#define MAX_MOD_DEV_NODE_LEN		255					//最大模块设备节点长度
	UInt8 modDevNode[MAX_MOD_DEV_NODE_LEN];				//模块设备类型的节点识别号
	UInt8 modManufacturerLen;							//模块制造商长度
#define	MAX_MOD_MANUFACTURER_LEN	255					//最大模块制造商长度
	UInt8 modManufacturer[MAX_MOD_MANUFACTURER_LEN];	//模块制造商
	UInt8 modStyleLen;									//模块型号长度
#define	MAX_MOD_STYLE_LEN			255					//最大模块型号长度
	UInt8 modStyle[MAX_MOD_STYLE_LEN];					//模块型号
	UInt8 modVersionLen;								//模块版本长度
#define	MAX_MOD_VERSION_LEN			255					//最大模块版本长度
	UInt8 modVersion[MAX_MOD_VERSION_LEN];				//模块版本
	UInt8 modType;				//模块类型，1:其他、2:硬件、3:软件
} GbModParamList;	//模块表参数列表

typedef struct gbScheduleList
{
	UInt8  scheduleNo;		//调度计划编号，范围[1,40]
	UInt16 month;			//bit1-bit12，每位表示一个月。置1表示允许对应计划在该月执行
	UInt8  week;			//bit1-bit7，每位表示一周中的一天。置1表示允许对应计划在该天执行
	UInt32 day;		        //bit1-bit31，每位表示一月中的一天。置1表示允许对应计划在该天执行
    UInt8  timeIntervalListNo;	//时段表编号，0表示本行无效
} GbScheduleList;	//调度计划列表

typedef struct gbTimeIntervalList
{
	UInt8 timeIntervalListNo;	//时段表编号，范围[1,16]
	UInt8 timeIntervalNo;		//时段号，范围[1,48]
	UInt8 hour;					//开始执行的整点数，24小时制
	UInt8 minute;				//开始执行的整分数
	UInt8 controlMode;			//控制方式
	UInt8 schemeId;				//方案号，0表示没有可执行的方案，即信号机自主决定控制方式
	UInt8 assistFunctionOutput;	//辅助功能输出
	UInt8 specialFunctionOutput;//特殊功能输出
} GbTimeIntervalList;	//时段列表

typedef enum
{
	PROGRAM_START_EVENT = 1,		//程序启动事件
	PROGRAM_END_EVENT = 2,			//程序结束事件
	NOT_CONFIG_EVENT = 3,			//没有配置
	AUTO_KEY_PRESS_EVENT = 4,		//自动键按下事件
	MANUAL_KEY_PRESS_EVENT = 5,		//手动键按下事件
	YELLOWFLASH_KEY_PRESS_EVENT = 6,//黄闪键按下事件
	ALLRED_KEY_PRESS_EVENT = 7,		//全红键按下事件
	STEP_KEY_PRESS_EVENT = 8,		//步进键按下事件
	RED_GREEN_CONFLICT_EVENT = 9,	//红绿冲突事件
	GREEN_CONFLICT_EVENT = 10,		//绿冲突事件
	RED_LIGHT_TURNOFF_EVENT = 11,	//红灯熄灭事件
	//以下属于车检器故障事件
	DETECTOR_UNKNOWN_FAULT_EVENT = 12,		//未知故障事件
	DETECTOR_CONFIG_FAULT_EVENT = 13,		//配置故障事件
	DETECTOR_COMMUNICATION_FAULT_EVENT = 14,//通信故障事件
	DETECTOR_NOT_STABLE_EVENT = 15,			//不稳定事件
	DETECTOR_EXIST_TIMEOUT_EVENT = 16,		//存在时间过长
	DETECTOR_NOT_ACTIVITY_EVENT = 17,		//不活动
	//以下属于感应线圈故障事件
	INDUCTOR_COIL_BEYOND_EVENT = 18,		//感应变化量超限
	INDUCTOR_COIL_INSUFFICIENT_EVENT = 19,	//电感不足
	INDUCTOR_COIL_OPEN_EVENT = 20,			//线圈开路
	INDUCTOR_COIL_WATCHDOG_FAULT_EVENT = 21,//watchdog故障
	INDUCTOR_COIL_UNKNOWN_FAULT_EVENT = 22,	//未知故障
} GbEventType;

typedef struct gbEventTypeList
{
	UInt8	eventTypeNo;							//事件类型编号(索引)，范围[1,255]
	UInt32	eventTypeClearTime;						//事件类型清除时间
	UInt8	eventTypeDescLen;						//事件类型描述长度
#define	MAX_EVENT_TYPE_DESC_LEN		255				//最大事件类型描述长度
	UInt8	eventTypeDesc[MAX_EVENT_TYPE_DESC_LEN];	//事件类型描述
	UInt8	eventTypeLineNo;						//这个事件类型在事件日志表中的行号
} GbEventTypeList;	//事件类型列表

typedef struct gbEventLogList
{
	UInt8	eventTypeNo;		//事件类型编号(索引)，范围[1,255]
	UInt8	eventStreamNo;		//事件流水编号，从1开始，循环记录(索引)
	UInt32	eventCheckedTime;	//事件被检测到的时间
	UInt32	eventValue;			//事件值
} GbEventLogList;	//事件日志列表

typedef struct gbPhaseList
{
	UInt8	phaseNo;				//相位编号，范围[1,16]
    UInt8  	pedestrianPassTime;		//行人放行时间，控制行人过街绿灯的秒数
    UInt8  	pedestrianClearTime;	//行人清空时间（行人绿闪时间）
    UInt8  	minGreen;				//最小绿灯时间，简称最小绿
    UInt8  	unitExtendGreen;		//单位绿灯延长时间,简称单位延长绿
    UInt8  	maxGreen_1;				//最大绿灯时间1，简称最大绿1
    UInt8  	maxGreen_2;				//最大绿灯时间2，简称最大绿2
	UInt8	fixGreen;				//弹性相位固定绿灯时间，简称固定绿
	UInt8	greenBlinkTime;			//绿闪时间
	UInt8	phaseType;				/* 相位类型属性，0:False/Disabled，1:True/Enabled
									   bit7:固定相位
									   bit6:待定相位
									   bit5:弹性相位
									   bit4:关键相位
									   bit3-0: Reserved	*/
	UInt8	phaseOption;			/* 相位选项功能，0:False/Disabled，1:True/Enabled
									   bit7-5: Reserved
									   bit	4:设置为1时使一个没有检测器请求的相位跟随另一个同阶段中同时放行的相位一起放行
									   bit	3:行人是否跟随机动车一起放行
									   bit	2:如果本相位是待定相位，同阶段相位出现时，本相位是否出现
									   bit	1:是否路段中的行人过街相位
									   bit	0:相位启动标志	*/
	UInt8	extendField;			//扩展字段，作为扩展使用
} GbPhaseList;	//相位列表

typedef struct gbStatusGroup
{
	UInt8	statusGroupNo;			//状态组编号，范围[1,2]
	UInt8	redStatus;				//红灯输出状态，bit0-7分别1-8或者9-16相位，1：有信号，0：无信号
	UInt8	yellowStatus;			//黄灯输出状态
	UInt8	greenStatus;			//绿灯输出状态
} GbStatusGroup;	//状态组

typedef struct gbPhaseConflictList
{
	UInt8	phaseConflictNo;			//相位冲突编号，范围[1,16]
	UInt16	conflictPhase;				//冲突相位，bit0-15分别表示相位1-16，1:有冲突，0:无冲突
} GbPhaseConflictList;	//相位冲突列表

typedef struct gbVehDetectorList
{
	UInt8	detectorNo;					//车检器编号，范围[1,48]
	UInt8	requestPhase;				//检测器请求相位，0表示无对应相位，范围[0,16]
	UInt8	detectorType;				/*车辆检测器类型 bit7:请求检测器，bit6:感应检测器，bit5:战术检测器
									bit4:战略检测器，bit3:行人按钮，bit2:公交车检测器，bit1:自行车检测器，bit0:机动车检测器 */
	UInt8	detectorDirection;			/*检测器方向，bit7:西北，bit6:西，bit5:西南，bit4:南
													  bit3:东南，bit2:东，bit1:东北，bit0:北 */
	UInt8	detectorRequestValidTime;	//检测器请求有效时间
	UInt8	detectorOption;				/*检测器选项参数，bit0:是否区分车型，bit1:是否埋在关键车道，bit2:流量，bit3:占有率
														  bit4:速度，bit5:排队长度，bit6-7: Reserved */
	UInt16	laneFullFlow;				//对应关键车道的饱和流量
	UInt8	laneFullRate;				//对应关键车道的饱和占有率，单位0.5
} GbVehDetectorList;	//车检器列表

typedef struct gbDetectorStatusList
{
	UInt8	detectorStatusNo;			//检测器状态列表编号
	UInt8	detectorStatus;				//检测器状态，1:检测到，0:未检测到，bit0-7分别对应于1-48共6组检测器，每组8个，依次类推
	UInt8	detectorStatusAlarm;		//检测器状态报警，任何一个检测器报警出现则把对应bit置位		
} GbDetectorStatusList;		//检测器状态列表

typedef struct gbTrafficDetectDataList
{
	UInt8	detectorNo;		//检测器编号，范围[1,48]
	UInt8	totalFlow;		//总流量，255表示溢出
	UInt8	largeVehFlow;	//大型车流量
	UInt8	smallVehFlow;	//小型车流量
	UInt8	rate;			//占有率，单位0.5，范围[0,200]
	UInt8	speed;			//速度，单位:千米km
	UInt8	vehBodyLen;		//车身长度，单位:分米dm
} GbTrafficDetectDataList;	//交通检测数据列表

typedef struct gbVehDetectorAlarmList
{
	UInt8	detectorNo;					//检测器编号，范围[1,48]
	UInt8	detectorAlarmStatus;		//检测器告警状态，发生故障时置位，故障消失时清零
	UInt8	inductiveCoilAlarmStatus;	//感应线圈告警状态
} GbVehDetectorAlarmList;	//车辆检测器告警列表

typedef	struct gbChannelList
{
	UInt8	channelNo;				//通道编号，范围[1,16]
	UInt8	channelRelatedPhase;	//通道关联相位
	UInt8	channelFlashStatus;		//闪光控制时通道的状态
	UInt8	channelControlType;		//通道控制类型，赋值对应于枚举ControllerType
} GbChannelList;	//通道列表

typedef	struct gbSchemeList
{
	UInt8	schemeNo;		//方案编号，范围[1,32]
	UInt8	cycleTime;		//周期时长
	UInt8	phaseGap;		//相位差
	UInt8	coordinatePhase;//协调相位
	UInt8	stageTimingNo;	//阶段配时表编号
} GbSchemeList;		//方案列表

typedef struct gbStageTimingList
{
	UInt8	stageTimingNo;	//阶段配时表编号，范围[1,16]
	UInt8	stageNo;		//阶段编号，范围[1,16]
	UInt16	phaseNo;		//放行的相位
	UInt8	greenTime;		//阶段绿灯时间，包含绿闪时间
	UInt8	yellowTime;		//阶段黄灯时间，行人相位放红灯
	UInt8	allRedTime;		//阶段全红时间
	UInt8	stageOption;	//阶段选项参数
} GbStageTimingList;	//阶段配时列表

typedef struct gbFollowPhaseList
{
	UInt8	followPhaseNo;		//跟随相位编号，范围[1,8]
	UInt8	operateType;		//操作类型
	UInt8	motherPhaseNum;		//母相位个数
	UInt8	motherPhase[MAX_PHASE_LIST_NUM];	//母相位
	UInt8	correctPhaseNum;	//修正相位个数
	UInt8	correctPhase[MAX_PHASE_LIST_NUM];	//修正相位
	UInt8	tailGreenTime;		//尾部绿灯时间
	UInt8	tailYellowTime;		//尾部黄灯时间
	UInt8	tailAllRedTime;		//尾部全红时间
} GbFollowPhaseList;	//跟随相位列表

typedef struct gbConfig
{
//其他额外的配置信息
	UInt16	pubDevIdentifyParam;		//公共设备识别参数，对象标识0x81
	UInt8	pubModMaxLineNum;			//公共模块表最大行数，对象标识0x82
	UInt16	pubSyncTime;				//公共同步时间，对象标识0x83
	UInt16	pubSyncFlag;				//公共同步标志，对象标识0x84
#define	MAX_MOD_PARAM_LIST_NUM	16		//最大模块参数列表个数
	GbModParamList modParamTable[MAX_MOD_PARAM_LIST_NUM];	//模块参数表，对象标识0x85
	UInt32	pubTime;					//公共时间，UTC或GMT时间，从1970/1/1 00:00:00到至今的秒数，对象标识0x86
	int		standardTimeZone;			//标准时区，本地标准时间与GMT的时差(秒)，北京时间时差固定为8*3600，对象标识0x87
	UInt32	localTime;					//本地时间，等于1970/1/1 00:00:00到至今的秒数，对象标识0x88
	UInt8	maxScheduleListNum;			//时基调度列表最大个数，默认40，对象标识0x89
	UInt8	maxTimeIntervalListNum;		//时段列表最大个数，默认16，对象标识0x8A
	UInt8	maxTimeIntervalNum;			//每个时段表所包含的时段数，默认48，对象标识0x8B
	UInt8	activeTimeIntervalListNo;	//活动时段表编号，0表示没有活动的时段表，对象标识0x8C
	UInt8	maxEventTypeListNum;		//事件类型列表最大个数，对象标识0x8F
	UInt8	maxEventLogListNum;			//事件日志列表最大个数，对象标识0x90
	UInt8	maxPhaseListNum;			//最大相位列表个数，默认16，对象标识0x93
	UInt8	maxPhaseStatusGroupNum;		//最大相位状态组个数，每组8个相位，默认2，对象标识0x94
	UInt8	phaseStatusGroupNum;		//相位状态组个数，对象标识0x96
	UInt8	maxVehDetectorNum;		//最大车辆检测器(包括行人按钮)总数，默认48，对象标识0x98
	UInt8	maxDetectorStatusGroupNum;	//检测器状态组的数量，8个检测器一组，默认6，对象标识0x99
	UInt8	detectDateStreamNo;			//检测数据流水号，每个采集周期顺序加1，循环计数，对象标识0x9A
	UInt8	dataCollectCycle;			//数据采集周期(单位秒)，对象标识0x9B
	UInt8	activeDetectorNum;			//活动检测器总数,范围[0,48]，对象标识0x9C
	UInt8	pulseDataStreamNo;			//脉冲数据流水号，每个采集周期顺序加1，循环计数，对象标识0x9D
	UInt8	pulseDataCollectCycle;		//脉冲数据采集周期(单位秒)，对象标识0x9E
	UInt8	controlStatus;				/*当前信号机的控制状态，1:未知模式,2:系统协调控制,3:主机协调控制
										4:手动面板控制,5:时段表控制,6:有缆协调,对象标识0xA5 */
	UInt8	flashControlMode;			//闪光控制模式，范围[1,7]，对象标识0xA6
	UInt8	deviceAlarm2;				//信号机设备报警2，对象标识0xA7
	UInt8	deviceAlarm1;				//信号机设备报警1，对象标识0xA8
	UInt8	deviceAlarmSummary;			//信号机设备报警摘要，对象标识0xA9
	UInt8	allowRemoteActivate;		//允许远程控制实体激活信号机的某些功能，对象标识0xAA
	UInt32	brightnessControlStartTime;	//辉度控制开启时间，对象标识0xAC
	UInt32	brightnessControlEndTime;	//辉度控制结束时间，对象标识0xAD
	UInt8	maxSupportChannelNum;		//信号机支持的最大通道数量，默认16，对象标识0xAE
	UInt8	maxChannelStatusGroupNum;	//通道输出状态组最大个数，默认2，对象标识0xAF
	UInt8	maxSchemeNum;				//可配置的最大方案个数，默认32，对象标识0xB2
	UInt8	maxStageTimingNum;			//可配置的最大阶段配时个数，默认16，对象标识0xB3
	UInt8	maxStageNum;				//可配置的最大阶段个数，默认16，对象标识0xB4
	UInt8	manualControlScheme;		//手动控制方案，对象标识0xB5
	UInt8	systemControlScheme;		//系统控制方案，对象标识0xB6
	UInt8	controlMode;				//控制方式，对象标识0xB7
	UInt8	pubCycleTime;				//公共周期时长，对象标识0xB8
	UInt8	coordinatePhaseGap;			//协调相位差，对象标识0xB9
	UInt8	stageStatus;				//阶段状态，对象标识0xBA
	UInt8	stepCommand;				//步进指令，对象标识0xBB
	UInt8	demotionMode;				//降级模式，对象标识0xBC
	UInt8	demotionStandardSchemeTable[14];	//降级基准方案表，对象标识0xBD
	UInt8	stageTime[16];				//当前方案各阶段时长，对象标识0xBE
	UInt8	keyPhaseGreenTime[MAX_PHASE_LIST_NUM];		//当前方案各关键相位绿灯时长，对象标识0xBF
	UInt8	downloadFlag;				//下载标志，1:下载开始，2:下载结束，对象标识0xC2
	UInt8	controlHostOption;			//控制主机选项参数，对象标识0xC3
	UInt16	deviceBaseAddr;				//信号机设备基地址，范围[0,8192]，对象标识0xC4
	UInt8	crossingNum;				//路口个数，范围[1,8]，对象标识0xC5
	UInt8	maxFollowPhaseListNum;		//跟随相位表的最大个数，默认8，对象标识0xC6
	UInt8	maxFollowPhaseStatusNum;	//跟随相位状态表数，默认1，对象标识0xC7

	
//与运行相关的配置信息
#define	MAX_SCHEDULE_LIST_NUM	40		//最大调度列表个数
	GbScheduleList scheduleTable[MAX_SCHEDULE_LIST_NUM];	//调度表，对象标识0x8D	
#define	MAX_TIMEINTERVAL_LIST_NUM	16	//最大时段列表个数
#define	MAX_TIMEINTERVAL_NUM		48	//最大时段数
	GbTimeIntervalList timeIntervalTable[MAX_TIMEINTERVAL_LIST_NUM][MAX_TIMEINTERVAL_NUM];	//时段表，对象标识0x8D
	GbPhaseList	phaseTable[MAX_PHASE_LIST_NUM];	//相位表，对象标识0x95
	GbPhaseConflictList	phaseConflictTable[MAX_PHASE_LIST_NUM];	//相位冲突表，对象标识0x97
#define	MAX_VEH_DETECTOR_NUM	48	//最大车辆检测器个数
	GbVehDetectorList vehDetectorTable[MAX_VEH_DETECTOR_NUM];	//车辆检测器表，对象标识0x9F
	UInt8	bootBlinkTime;				//启动闪光时间，对象标识0xA3
	UInt8	bootAllRedTime;			//启动全红时间，对象标识0xA4
	UInt8	flashFrequency;				//闪光频率，对象标识0xAB
#define	MAX_CHANNEL_LIST_NUM	16		//最大通道列表个数
	GbChannelList channelTable[MAX_CHANNEL_LIST_NUM];		//通道表，对象标识0xB0
#define	MAX_SCHEME_LIST_NUM		32		//最大方案列表个数
	GbSchemeList schemeTable[MAX_SCHEME_LIST_NUM];	//方案表，对象标识0xC0
#define	MAX_STAGE_TIMING_LIST_NUM	16	//最大阶段配时列表个数
#define	MAX_STAGE_NUM				16	//最大阶段数
	GbStageTimingList stageTimingTable[MAX_STAGE_TIMING_LIST_NUM][MAX_STAGE_NUM];	//阶段配时表，对象标识0xC1
#define	MAX_FOLLOW_PHASE_LIST_NUM	8	//最大跟随相位表个数
	GbFollowPhaseList followPhaseTable[MAX_FOLLOW_PHASE_LIST_NUM];	//跟随相位表，对象标识0xC8	


//运行过程中记录的故障或是日志信息
#define	MAX_EVENT_TYPE_LIST_NUM		255	//最大事件类型列表个数
	GbEventTypeList eventTypeTable[MAX_EVENT_TYPE_LIST_NUM];//事件类型表，对象标识0x91
#define	MAX_EVENT_LOG_LIST_NUM		255	//最大事件日志列表个数
	GbEventLogList eventLogTable[MAX_EVENT_TYPE_LIST_NUM][MAX_EVENT_LOG_LIST_NUM];//事件日志表，对象标识0x92
	

//一些只需要获取的状态信息
#define MAX_PHASE_STATUS_GROUP_NUM	2	//最大相位状态组个数
	GbStatusGroup phaseStatusTable[MAX_PHASE_STATUS_GROUP_NUM];	//相位状态表
#define	MAX_CHANNEL_STATUS_GROUP_NUM	2	//最大通道输出状态组个数
	GbStatusGroup channelStatusTable[MAX_CHANNEL_STATUS_GROUP_NUM];	//通道输出状态表，对象标识0xB1
#define MAX_FOLLOW_PHASE_STATUS_GROUP_NUM	1	//最大跟随相位状态组个数
	GbStatusGroup followPhaseStatusTable[MAX_FOLLOW_PHASE_STATUS_GROUP_NUM];	//跟随相位状态表，对象标识0xC9
#define MAX_DETECTOR_STATUS_LIST_NUM 6	//最大检测器状态列表个数
	GbDetectorStatusList detectorStatusTable[MAX_DETECTOR_STATUS_LIST_NUM];	//检测器状态表，对象标识0xA0	
	GbTrafficDetectDataList trafficDetectDataTable[MAX_VEH_DETECTOR_NUM];	//交通检测数据表，对象标识0xA1
	GbVehDetectorAlarmList vehDetectorAlarmTable[MAX_VEH_DETECTOR_NUM];	//车辆检测器告警表，对象标识0xA2
} GbConfig;

typedef struct gbObject
{
	UInt32	baseOffset;					//基础偏移量
	UInt16	size;						//所占大小
	Boolean	isCheck;					//是否检查
	UInt32	minValue;					//最小值
	UInt32	maxValue;					//最大值
} GbObject;

typedef void (*GbObjectDealFunc)(GbOperateType type);

typedef struct gbObjectIdentify
{
	GbObject 	object;					//父对象
	UInt8		objectId;				//对象标识
	UInt16		maxIndex;				//对象最大索引
	UInt8		childObjectNum;			//子对象个数
#define	MAX_CHILD_OBJECT_NUM	16		//最大子对象个数
	GbObject	childObject[MAX_CHILD_OBJECT_NUM];	//子对象集
	UInt8		ntcipUpdateBit;					//对应的NTPIP结构体的更新bit位
	GbObjectDealFunc func;
} GbObjectIdentify;

#pragma pack(pop)
#endif
