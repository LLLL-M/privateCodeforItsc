#pragma once

#include <functional>
#include <string>

using namespace std;

struct TscRetval
{
	bool succ;			//返回是否成功
	std::string msg;	//上载成功返回上载的内容，下载成功为空，失败返回失败信息

	TscRetval(bool _succ = true) : succ(_succ) {}
	//设置出错信息
	template<typename ... Args>
	void Err(const char *fmt, Args&&... args)
	{
		succ = false;
		if (fmt == nullptr)
			return;
		char buf[1024] = {0};
		sprintf(buf, fmt, forward<Args>(args)...);
		if (msg.empty())
	    	msg = buf;
	    else
	    {
	    	msg += ";";
	    	msg += buf;
	    }
	}
	void operator +=(const TscRetval &ret) 
	{
		if (!ret.succ)
		{
			succ = false;
			if(!msg.empty())
				msg += ";";
			msg += ret.msg;
		}
	}
	operator bool() const
	{
		return succ;
	}
};

typedef enum
{
	PLATFORM = 1,				//平台控制
	CONFIG_TOOL = 2,			//配置工具控制
	TOUCH_SCREEN = 3,			//触摸屏控制
	MOBILE_APP = 4,				//手机app控制
	SOURCE_MAX
} TscSource;

//配置类型枚举
typedef enum
{
	TSC_TYPE_PUBKEY = 0,		//通信双方交互公钥，上位机不可用
	TSC_TYPE_LOGIN = 1,			//登录验证，上位机不可用
	TSC_TYPE_UNIT = 2,			//单元配置，上位机可用
	TSC_TYPE_CHANNEL = 3,		//通道配置，上位机可用
	TSC_TYPE_PHASE = 4,			//相位配置，上位机可用
	TSC_TYPE_SCHEME = 5,		//方案配置，上位机可用
	TSC_TYPE_TIMEINTERVAL = 6,	//时段配置，上位机可用
	TSC_TYPE_SCHEDULE = 7,		//调度配置，上位机可用
	TSC_TYPE_VEHDETECTOR = 8,	//车检器配置，上位机可用
	TSC_TYPE_PEDDETECTOR = 9,	//行人检测器配置，上位机可用
	TSC_TYPE_PRIOR = 10,		//优先配置，上位机可用
	TSC_TYPE_COUNTDOWN = 11,	//倒计时牌配置，上位机可用
	TSC_TYPE_WHOLE = 12,		//整体配置，上位机不可用
	TSC_TYPE_MANUAL_CTRL = 13,	//上位机控制，上位机可用
	TSC_TYPE_SYSTIME = 14,		//系统时间配置，上位机可用
	TSC_TYPE_REMOTE = 15,		//远程控制，上位机可用
	TSC_TYPE_NETWORK = 16,		//网络配置，上位机可用
	TSC_TYPE_WIFI = 17,			//wifi模块配置，上位机可用
	TSC_TYPE_SERIAL = 18,		//串口配置，上位机可用
	TSC_TYPE_CHANNEL_CHECK = 19,//通道检测，上位机可用
	TSC_TYPE_REDCHECK = 20,		//红灯信号检测器配置，上位机可用
	TSC_TYPE_WIRELESS = 21,		//无线遥控器配置，上位机可用
	TSC_TYPE_PANEL = 22,		//手动面板配置，上位机可用
	TSC_TYPE_LOG = 23,			//信号机log，get为获取log，set为清除log，上位机不可用
	TSC_TYPE_UPDATE = 24,		//信号机升级，上位机不可用
	TSC_TYPE_MULTILOCK = 25,	//多时段锁定，上位机可用
	TSC_TYPE_PASSWORD = 26,		//信号机密码，上位机可用
	TSC_TYPE_MAX,

} TscConfigType;

//信号机实时信息类型
typedef enum 
{
	TSC_RT_STATUS = 0,	//状态信息
	TSC_RT_ALARM = 1,	//报警信息
	TSC_RT_TFAFFIC = 2,	//车检器交通流信息
	TSC_RT_CYCLE = 3,	//周期统计信息
	TSC_RT_CONNECT = 4,	//其他上位机联机信息
} TscRealtimeType;

//上位机实时信息处理回调函数类型
typedef function<void(TscRealtimeType, const string &)> CallbackFunc;

#define MAX_CHANNEL_NUM			48				//最大通道个数
#define MAX_PHASE_NUM			64				//最大相位个数
#define MAX_SCHEME_NUM			255				//最大方案个数
#define MAX_TIMELIST_NUM		64				//最大时段表个数
#define MAX_TIMEINTERVAL_NUM 	72				//最大时段数
#define MAX_SCHEDULE_NUM		128				//最大调度日期个数
#define MAX_VEHDETECTOR_NUM		64				//最大车辆检测器个数
#define MAX_PEDDETECTOR_NUM		64				//最大行人检测器个数
#define MAX_PRIOR_NUM			128				//最大优先表个数
#define MAX_COUNTDOWN_NUM		MAX_CHANNEL_NUM	//最大倒计时牌个数
#define MAX_NETWORK_NUM			3 				//最大网口个数
#define MAX_SERIAL_NUM			4 				//最大串口个数
#define MAX_WIRELESS_NUM		4 				//最大手动按键个数
#define MAX_PANEL_NUM			8 				//最大手动面板按键个数
#define MAX_RING_NUM			8 				//最大支持的环数



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
	RED_GREEN = 10,
	YELLOW_GREEN = 11,
	RED_YELLOW_GREEN = 12,
	/*以下几个枚举平台以及SDK都不能使用，只能使用上面的枚举*/
	OFF_GREEN = 13,		//绿灯脉冲倒计时，第一个250ms关灯
	OFF_YELLOW = 14,	//绿灯脉冲倒计时，第一个250ms关灯
	OFF_RED = 15,		//红灯脉冲倒计时，第一个250ms关灯
} TscStatus;

//控制模式
enum
{
	SYSTEM_MODE = 0,				//系统控制模式
	TURNOFF_MODE = 1,				//关灯控制模式
	YELLOWBLINK_MODE = 2,			//黄闪控制模式
	ALLRED_MODE = 3,				//全红控制模式
	FIXEDCYCLE_MODE = 4,			//定周期控制模式
	COORDINATE_MODE = 5,			//协调控制模式
	INDUCTIVE_MODE = 6,				//感应控制模式
	SINGLE_SPOT_OPTIMIZE = 7,		//单点优化模式
	STEP_MODE = 8,					//步进控制模式
	CANCEL_STEP = 9,				//取消步进控制模式
	PEDESTRIAN_INDUCTIVE_MODE = 10,	//行人感应控制模式
	BUS_ADVANCE_MODE = 11,			//公交优先控制模式
	INDUCTIVE_COORDINATE_MODE = 12,	//协调感应模式
	SINGLE_ADAPT_MODE = 13,         //自适应控制模式
	CHANNEL_LOCK = 14,				//通道锁定控制模式
	/*增加的控制模式*/
	CONTROL_MAX,					//此值只用于做控制模式的范围判断
};

//Unit Table
#define	TSC_AREA					"area"
#define TSC_JUNCTION				"junction"
#define TSC_BOOTFLASH				"bootFlash"
#define TSC_BOOTALLRED				"bootAllRed"
#define TSC_TRAFFIC_COLLECT			"collect"
#define	TSC_GREENWAVE_TRANS			"transition"
#define TSC_GPS						"GPS"
#define TSC_WATCHDOG				"watchdog"
#define TSC_VOLT_CHECK				"voltCheck"
#define TSC_CUR_CHECK				"curCheck"
#define TSC_FAULT_FLASH				"faultFlash"
#define TSC_TAKEOVER				"takeover"

typedef enum
{
	ACTION_ADD = 0,
	ACTION_DELETE = 1
} TscActionStatus;

//Channel Table
#define TSC_ID						"id"
#define TSC_ACTION					"action"
#define TSC_TYPE					"type"
#define	TSC_STATUS					"status"
#define TSC_CONFILCT_BITS			"conflictBits"
#define TSC_COUNTDOWN				"countdown"
#define TSC_VEH_BITS				"vehBits"
#define TSC_PED_BITS				"pedBits"
#define TSC_DESC  					"desc"

typedef enum
{
	CHANNEL_TYPE_UNUSED = 0,
	CHANNEL_TYPE_CAR = 1,
	CHANNEL_TYPE_PED = 2,
	CHANNEL_TYPE_ALTER = 3,
	CHANNEL_TYPE_TIDE = 4,
	CHANNEL_TYPE_MAX,
} TscChannelType;

//Phase Table
#define	TSC_GREENFLASH				"greenFlash"
#define	TSC_YELLOW_TIME				"yellow"
#define TSC_ALLRED_TIME				"allred"
#define TSC_REDYELLOW_TIME			"redyellow"
#define TSC_PED_CLEAR_TIME			"pedClear"
#define TSC_MIN_GREEN_TIME			"minGreen"
#define TSC_MAX_GREEN_TIME			"maxGreen"
#define	TSC_MAX_GREEN2_TIME			"maxGreen2"
#define TSC_UNI_EXTEND_TIME			"unitExtend"
#define TSC_CHECK_TIME				"checkTime"
#define	TSC_CHANNEL_BITS			"channelBits"
#define TSC_AUTO_REQ				"autoReq"

//Scheme Table
#define TSC_CYCLE_TIME				"cycle"
#define TSC_OFFSET					"offset"
#define TSC_COORDINATE_PHASE		"coordPhase"
#define TSC_PHASE_ID				"phase"
#define TSC_PHASE_TIME				"time"
#define TSC_PHASE_STATUS			"status"
#define TSC_BARRIER_ID				"barrier"
#define	TSC_TIME_MODIFY				"timing"
#define TSC_TURN_MODIFY				"turn"

//Timeinterval Table
#define	TSC_TIME_INTERVAL			"timeinterval"
#define	TSC_HOUR					"hour"
#define	TSC_MINUTE					"minute"
#define TSC_CONTROL_MODE			"mode"
#define	TSC_SCHEME_ID				"scheme"
#define TSC_INTERVAL_SEQ			"seq"
#define TSC_INTERVAL_SEQ_DEL		"delseq"

typedef enum
{
	CTRL_MODE_CLOSE = 1,
	CTRL_MODE_YELLOW_FLASH = 2,
	CTRL_MODE_ALL_RED = 3,
	CTRL_MODE_INTERVAL = 4,
	CTRL_MODE_COORD = 5,
	CTRL_MODE_INDUCED = 6,
	CTRL_MODE_SPOT_OPT = 7,
	CTRL_MODE_PED_INDUCED = 10,
	CTRL_MODE_BUS_PRIOR = 11,
	CTRL_MODE_COOR_INDUCED = 12,
	CTRL_MODE_SELF_ADAPT = 13
} TscControlMode;

//Schedule Table
#define TSC_DATE 					"date"
#define TSC_WEEK					"week"
#define	TSC_MONTH					"month"
#define TSC_DAYS					"days"
#define TSC_YEAR					"year"
#define TSC_TIME_INTERVAL_ID		"timeinterval"

//Vehicle Dectector Table
#define TSC_COIL_LEN				"coilLen"
#define TSC_VD_GAP					"gap"
#define TSC_NO_RESPON_TIME			"noResponse"
#define	TSC_MAX_HOLD_TIME			"maxContinuous"
#define TSC_MAX_VEH_NUM				"maxVehcileNum"
#define	TSC_MIN_TIME_GAP			"minTimeGap"
#define	TSC_MAX_TIME_GAP			"maxTimeGap"
#define TSC_MIN_QUE_LEN				"minQueueLen"
#define TSC_MAX_QUE_LEN				"maxQueueLen"
#define TSC_OCCUPY_JAM				"occupyJam"

typedef enum
{
	VEH_DETECT_FLOW = 0,
	VEH_DETECT_JAM = 1,
	VEH_DETECT_CHANGEABLE = 2,
	VEH_DETECT_MAX
} TscVehDetectorType;

//Pedestrian Detector Table
#define	TSC_DELAY_PASS				"delayPass"
#define	TSC_MAX_WAIT				"maxWait"

//Prior Table
#define TSC_LABEL					"label"
#define TSC_READER					"reader"
#define TSC_CHANNEL 				"channel"
#define TSC_PRI_DUR					"duration"

//Countdown Table
#define TSC_COUNTDOWN_MODE			"mode"
#define	TSC_PULSE_GREEN				"pulseGreen"
#define	TSC_PULSE_RED				"pulseRed"

typedef enum
{
	CD_MODE_SELF_LEARN = 0,
	CD_MODE_FULL_PULSE = 1,
	CD_MODE_HALF_PULSE = 2,
	CD_MODE_GB_2014 = 3,
	CD_MODE_LES = 4,
	CD_MODE_HISENSE = 5,
	CD_MODE_GB_2004 = 6,
	CD_MODE_MAX
} TscCountdownMode;

//Control Table
#define TSC_CTRL_MODE				"ctrlMode"
#define TSC_CTRL_ID 				"ctrlId"
#define TSC_CTRL_DUR				"duration"
#define TSC_CTRL_LOCK				"status"

//Time Table
#define TSC_UTC_TIME				"UTC"
#define	TSC_TIME_ZONE				"timezone"

//Remote Control Table
#define TSC_CLEAR 					"clear"
#define TSC_RESTORE					"restore"
#define	TSC_REBOOT					"reboot"

//Network Table
#define	TSC_MAC						"mac"
#define TSC_IP 						"ip"
#define	TSC_NETMASK					"netmask"
#define	TSC_GATEWAY					"gateway"

//Wifi Table
#define	TSC_SSID					"ssid"
#define	TSC_PASSWD					"password"

//Serial Table
#define	TSC_BAUD_RATE				"baudRate"
#define TSC_DATA_BITS				"dataBits"
#define TSC_STOP_BITS				"stopBits"
#define TSC_PARTIY					"parity"
#define	TSC_PARITY_ODD				"O"
#define TSC_PARITY_EVEN				"E"
#define TSC_PARTIY_NO				"N"

enum
{
	SERIAL_STOPBITS_1 = 1,		//串口停止位1
	SERIAL_STOPBITS_2 = 2,		//串口停止位2

	SERIAL_DATABITS_7 = 7,		//串口数据位7
	SERIAL_DATABITS_8 = 8,		//串口数据位8
	
	BAUDRATE_2400 = 2400,		//串口波特率2400
	BAUDRATE_4800 = 4800,		//串口波特率4800
	BAUDRATE_9600 = 9600,		//串口波特率9600
	BAUDRATE_19200 = 19200,		//串口波特率19200
	BAUDRATE_38400 = 38400,		//串口波特率38400
	BAUDRATE_57600 = 57600,		//串口波特率57600
	BAUDRATE_115200 = 115200 	//串口波特率115200
};

//ChannelCheck Table
typedef enum
{
	CHANNEL_CHECK_GREEN = 1,
	CHANNEL_CHECK_RED = 2,
	CHANNEL_CHECK_YELLOW = 3
} TscChannelCheckStatus;

//RedCheck Table
#define TSC_REDSIGNAL_ENABLE		"enable"
#define	TSC_REDSIGNAL_IP			"ip"
#define TSC_REDSIGNAL_PORT			"port"

//Key Table
#define TSC_KEY_ID					"id"
#define TSC_KEY_DESC				"desc"
#define TSC_KEY_ENABLE				"enable"
#define	TSC_CTRL					"ctrl"

//Log Table
#define TSC_LOG_START_TIME			"start"
#define TSC_LOG_END_TIME			"end"
#define TSC_LOG_OCCUR_TIME			"time"
#define	TSC_LOG_EVENT				"msg"

//Status Table
#define	TSC_CTRL_TYPE				"ctrlType"
#define TSC_RUN_CYCLE				"cycle"
#define	TSC_START_TIME 				"start"
#define TSC_TIME 					"time"
#define TSC_RING 					"ring"
#define TSC_RUN_CUR_TIME			"current"
#define TSC_PHASE_INFO				"phase"
#define TSC_STATUS_REMAIN			"countdown"
#define TSC_PHASE_TOTAL_TIME		"total"
#define TSC_PHASE_RUN_TIME			"now"
#define TSC_CHANNEL_INFO			"channel"

//Cycle Statues Table
#define TSC_CYCLE_START				"start"
#define TSC_CYCLE_DESC 				TSC_DESC
#define TSC_CYCLE_TOTAL				"cycle"
#define TSC_CYCLE_CAR_NUM			"total"
#define TSC_CYCLE_PHASE				"phase"
#define TSC_PHASE_MIN_GREEN			"min"
#define TSC_PHASE_MAX_GREEN			"max"
#define TSC_PHASE_EXT				"ext"
#define TSC_PHASE_ACT_TIME			"actual"
#define TSC_GREEN_LOST				"lost"
#define TSC_PHASE_FLOW_NUM			"flow"

//Traffice Table
#define	TSC_TRAFFIC_START			"start"
#define	TSC_TRAFFIC_END				"end"
#define TSC_TRAFFIC_INFO			"info"
#define TSC_TRAFFIC_FLOW			"flow"
#define	TSC_TRAFFIC_OCCU			"occupancy"
#define	TSC_AVE_SPEED				"speed"
#define TSC_AVE_TIME_GAP 			"headway"
#define TSC_QUEUE_LEN				"queueLen"
#define TSC_SATURATION				"saturation"

//Alarm Table
#define TSC_ALARM_TIME				"time"
#define	TSC_ALARM_MSG				"alarm"

//MultiChannel Lock Table
#define TSC_LOCK_HOUR				"hour"
#define	TSC_LOCK_MINUTE				"minute"
#define	TSC_LOCK_SEC				"sec"
#define TSC_LOCK_DURATION			"duration"
#define	TSC_LOCK_ARRARY				"status"

//Password Table
#define TSC_PASSWD_OLD				"old"
#define TSC_PASSWD_NEW				"new"

//Connect Table
#define TSC_CONNECT_IP				"ip"
#define	TSC_CONNECT_SRC				"src"
#define TSC_CONNECT_TIME			"time"

//Info
#define TSC_SOFTWARE_INFO			"software"
#define TSC_HARDWARE_INFO			"hardware"