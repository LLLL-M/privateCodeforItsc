/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : HikConfig.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : HikConfig.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月3日
    作    者   : 肖文虎
    修改内容   : 按照章文超给的头文件进行修改
******************************************************************************/

#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "hik.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#pragma pack(push, 4)




/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HIKCONFIG_LIB_VERSION	"1.1.0"

#define CFG_NAME "/home/hikconfig.dat"
#define CFG_BAK_NAME "/home/hikconfig.bak"

#define BIT(v, n) (((v) >> (n)) & 0x1)		//取v的第 n bit位

//相位是否使能，传入参数为相位选项，若使能了，则返回值为1，否则返回值为0
#define IS_PHASE_INABLE(option)     ((((option)&&0x01) == 1 ) ? 1 : 0)

#define     NUM_SCHEDULE                    255               //调度表总数,modified by xiaowh
#define     NUM_TIME_INTERVAL               16               //时段表总数
#define     NUM_TIME_INTERVAL_ID            48               //每个时段表含有的时段号总数
#define     NUM_ACTION                     255               //动作表总数
#define     NUM_SCHEME                     108               //方案表总数
#define     NUM_CHANNEL                     32               //通道表总数	changed by Jicky
#define     NUM_GREEN_SIGNAL_RATION         36               //绿信比表总数
#define     NUM_PHASE                       16               //相位表总数
#define     NUM_FOLLOW_PHASE                16               //跟随相位表总数
#define     NUM_PHASE_TURN                  16               //相序表总数
#define     NUM_RING_COUNT                   4               //相序表中支持的环数最大值

#define     MAX_VEHICLEDETECTOR_COUNT			72//80	//车辆检测器表最大行数 trap
#define		MAX_VEHICLEDETECTOR_COUNT_S			48 		//车辆检测器表最大行数 snmp
#define		MAX_VEHICLEDETECTOR_STATUS_COUNT	8		//车辆检测器状态组表最大行数
#define		MAX_PEDESTRIANDETECTOR_COUNT		8		//行人检测器表最大行数

#define		MAX_VOLUMEOCCUPANCY_COUNT			MAX_VEHICLEDETECTOR_COUNT		//流量占有率表最大行数

#define		MAX_OVERLAP_COUNT					16		//重叠表最大行数
#define		MAX_PHASE_COUNT_IN_OVERLAP			16		//重叠中的最大include相位数
#define		MAX_PHASE_COUNT_MO_OVERLAP			16		//重叠中的最大Modifier相位数
#define		MAX_OVERLAP_STATUS_COUNT			2		//重叠状态表最大行数
#define		MAX_MODULE_COUNT					255		//模块表最大行数
#define		MAX_MODULE_STRING_LENGTH			128		//模块中字符串长度

#define		MAX_EVENTLOG_COUNT					255		//事件表最大行数
#define		MAX_EVENTCLASS_COUNT				1		//事件类型最大值，实际为3
#define		MAX_EVENTCONFIG_COUNT				50		//事件配置表最大行

#define		MAX_PREEMPT_COUNT					8		//优先一致组表数
#define		MAX_PEDESTRIANPHASE_COUNT			8  		//行人相位表行数

#define		MAX_COUNTDOWNBOARD_COUNT			24		//最大倒计时牌个数
#define		MAX_COUNTDOWNBOARD_INCLUDEDPHASES	16		//每个倒计时牌所能包含的最多相位数

#define		MAX_SPECIALFUNCOUTPUT_COUNT			8		//最大支持的特殊功能行数
#define		MAX_DYNPATTERNSEL_COUNT				8		//最大动态方案选择配置表行数
#define		MAX_RS232PORT_COUNT					3		//最大支持的端口配置表行数
#define 	MAX_SIGNALTRANS_LIMIT	            20		//信号转换序列配置时间限制
#define 	MAX_ALLSTOPPHASE_LIMIT              32      //跟随相位号限制 
#define		MAX_BYTE_VALUE			255
#define		MAX_2BYTE_VALUE			65535

#define		MAX_TRANSINTENSITYCHOICE_COUNT			90		//交通强度周期选择表最大行数
#define     MAX_PHASE_TYPE_COUNT                3         //相位类型   

#define     MAX_ITSCONSERI_COUNT                6       //串口表最大表行数    
#define     MAX_ITSCONIO_COUNT                  24      //IO 配置表最大行数   
#define     MAX_ITSCONCHANNEL_COUNT             32      //冲突通道表最大行数  

#define 	MAX_EXTEND_TABLE_COUNT				16						//最大相位表个数
#define 	MAX_PHASE_TABLE_COUNT			MAX_EXTEND_TABLE_COUNT		//最大相位表个数
#define		MAX_CHANNEL_TABLE_COUNT			MAX_EXTEND_TABLE_COUNT		//最大通道表个数
#define 	MAX_FOLLOW_PHASE_TABLE_COUNT	MAX_EXTEND_TABLE_COUNT		//最大跟随相位表个数

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/

typedef enum
{
	ERROR_NULL_POINTER = 1,						    //全局指针为空
	ERROR_SUBSCRIPT_PHASE = 2,				 		//相位数组下标值+1与实际相位值不相同。nPhaseArray[i] != (i+1)
	ERROR_SUBSCRIPT_PHASE_TURN = 3,				    //相序表的数组下标值+1与实际相序表号不相同。
	ERROR_SUSCRIPT_PHASE_TURN_CIRCLE = 4,			//相序表的环号组的下标值+1与实际环号不相同。
	ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION = 5,		//绿信比表的数组下标值+1与实际绿信比表号不相同。
	ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE = 6,	//绿信比表的相位数组下标值+1与实际相位号不相同。
	ERROR_SUBSCRIPT_CHANNEL = 7,					//通道表的数组下标值+1与实际通道号不相同。
	ERROR_SUBSCRIPT_SCHEME = 8,					//方案表的数组下标值+1与实际方案号不相同。
	ERROR_SUBSCRIPT_ACTION = 9,					//动作表的数组下标值+1与实际动作号不相同。
	ERROR_SUBSCRIPT_TIMEINTERVAL = 10,				//时段表的数组下标值+1与实际时段表号不相同。
	ERROR_SUBSCRIPT_TIMEINTERVAL_TIME = 11,			//动作表的时段数组下标值+1与实际时段号不相同。
	ERROR_SUBSCRIPT_SCHEDULE = 12,					//调度表的数组下标值+1与调度表号不相同。
	ERROR_SUBSCRIPT_FOLLOW_PHASE = 13,				//跟随相位表的数组下标值+1与实际跟随相位号不相同。
	
	ERROR_SAME_CIRCLE_CONCURRENT_PHASE = 14,		//并发相位表中相位号存在于并发相位数组中。
	ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE = 15,	//并发相位的相位号不存在于其并发相位的并发相位数组中。
	ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE = 16, //并发相位的子相位在相序表中不连续。
	ERROR_BARRIER_CONCURRENT_PHASE = 17, 			//并发相位的屏障一侧绿信比时间不相等。
	ERROR_NOT_EQUAL_PHASE_TURN_CIRCLE = 18,			//相序表中环号和相位表中该相位的环号不相同。
	ERROR_NOT_CORRECT_PHASE_TURN = 19,				//相序表中相位的序号和并发相位不匹配
	ERROR_SPLIT_LOW_MOTO_GREEN_SIGNAL_RATION = 20,	//绿信比表中周期长比机动车的绿闪、黄灯、全红之和小
	ERROR_SPLIT_LOW_PEDESTRIAN_GREEN_SIGNAL_RATION = 21,//绿信比表中周期长比行人清空、行人放行时间之和小。
	ERROR_ILLEGAL_SCHME = 22,						//方案表中相序表或绿信比表ID非法。
	ERROR_CIRCLE_TIME_SCHEME = 23,					//方案表中周期长不等于单环绿信比之和
	
	ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE = 24,	//并发相位表中的并发相位ID不存在。
	ERROR_NOT_EXIST_PHASE_TURN_PHASE = 25,			//相序表中相位号不存在。
	ERROR_NOT_EXIST_SOURCE_CHANNEL = 26,				//通道表中该通道的控制源ID不存在。
	ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL = 27,		//通道表中该通道的跟随控制源ID不存在。
	ERROR_NOT_EXIST_SOURCE_OTHER_CHANNEL = 28,		//通道表中该通道的其他控制源ID不存在。
	ERROR_NOT_EXIST_PHASE_TURN_SHCEME = 29,			//方案表中相序表不存在。
	ERROR_NOT_EXIST_GREEN_SIGNAL_RATION_SCHEME = 30,	//方案表中绿信比表不存在。
	ERROR_NOT_EXIST_SCHEME_ACTION = 31,				//动作表中方案表不存在。
	ERROR_NOT_EXIST_ACTION_TIMEINTERVAL = 32,		//时段表中动作表不存在。
	ERROR_NOT_EXIST_TIMEINTERVAL_SCHEDULE = 33,		//调度表中时段表不存在。
	ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE = 34,	//跟随表中母相位不存在。
	ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE_2 = 35,	//相位表中有相位不存在并发相位。
	ERROR_NOT_EXIST_PHASE_TURN_PHASE_2 = 36,			//相位表中有相位不存在于相序表中。
	ERROR_NOT_EXIST_PHASE_GREEN_SIGNAL_RATION = 37,	//相位表中有相位不存在与绿信比表中。
	
	ERROR_REPEATE_CONCURRENT_PHASE = 38,			//某相位的并发相位数组有重复值。
	ERROR_REPEATE_PHASE_TURN = 39,					//某相序表的相序数组有重复值。
	ERROR_REPEATE_FOLLOW_PHASE = 40,				//跟随相位表的跟随相位数组有重复值。
	ERROR_REPEATE_SCHEDULE = 41,						//调度表中有重复日期调度
	
	ERROR_NOT_CONFIG_INFORMATION = 42,				//配置信息为空
	
	//
	ERROR_ID_LEGAL_SCHEDULE = 43,					//调度表ID不合法，范围必须是[0,108]
	ERROR_ID_LEGAL_INTERVAL = 44,					//时段表ID不合法，范围必须是[0,16]
	ERROR_ID_LEGAL_INTERVAL_TIME = 45,				//时段表中的时段ID不合法，范围必须是[0,48]
	ERROR_ID_LEGAL_ACTION = 46,						//动作表ID不合法，范围必须是[0,255]
	ERROR_ID_LEGAL_SCHEME = 47,						//方案表ID不合法，范围必须是[0,108]
	ERROR_ID_LEGAL_PHASE = 48,						//相位表ID不合法，范围必须是[0,16]
	ERROR_ID_LEGAL_SPLIT = 49,						//绿信比ID不合法，范围必须是[0,36]
	ERROR_ID_LEGAL_PHASE_TURN = 50,					//相序表ID不合法，范围必须是[0,16]
	ERROR_ID_LEGAL_PHASE_TURN_ID = 51,				//相序表的环号不合法，范围必须是[0,4]
	ERROR_ID_LEGAL_FOLLOW_PHASE = 52,				//跟随相位ID不合法，范围必须是[0,16]
	ERROR_ID_LEGAL_CHANNEL = 53,					//通道表ID不合法，范围必须是[0,16]
	//
	ERROR_PHASE_DISABLE = 54						//相位未使能
}
TSC_Para_Error_Num;

typedef enum {
	InitSetting_OTHER = 1,
    InitSetting_RED,//红灯
    InitSetting_PEDESTRIAN_CLEAR,//行人清空
    InitSetting_GREEN,//绿灯    
    InitSetting_YELLOW,//黄灯    
    InitSetting_ALL_RED,//全红    
    
}InitSetting;//初始化设置

typedef enum {

    UNDEFINEED = 0,//未定义
    OTHER_TYPE,//其他类型
    NONE,//无
    MIN_MOTO,//最小车辆响应    
    MAX_MOTO,//最大车辆响应
    PEDESTRIAN_RES,//行人响应
    PEDESTRIAN_QEQ,//最大车辆/行人请求
    IGNORE_PHASE//忽略相位
}GreenSignalRationType;//绿信比模式

typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//计划时间

typedef enum {

    OTHER = 1,//其他类型
    MOTOR,//机动车类型
    PEDESTRIAN,//行人类型
    FOLLOW,//跟随类型
}ControllerType;//控制源的类型

typedef enum {

    ALTERNATE = 0,//交替
    REDLIGHT,//红闪
    YELLOWLIGHT//黄闪

}FlashLightType;//闪光模式


//相位表定义
typedef struct {

    UInt8 nPhaseID;                          /*2.2.2.1 相位号，不能超过maxPhases所定义的值。(1..32)*/
    UInt8  nPedestrianPassTime;              /*2.2.2.2行人放行时间，控制行人过街绿灯的秒数。(0..255),second*/
    UInt8  nPedestrianClearTime;             /* 2.2.2.3 行人清空时间（行人绿闪时间）(0..255)*/
    UInt8  nMinGreen;                        /* 2.2.2.4最小绿灯时间，简称最小绿。一般根据入口检测器与停车线间的车辆排队情况确定。(0..255)*/
    UInt8  nUnitExtendGreen;                 /*2.2.2.5单位绿灯延长时间(0-25.5 sec)，简称单位延长绿。(0..255)，单位为1/10秒*/
    UInt8  nMaxGreen_1;                      /* 2.2.2.6最大绿灯时间1，简称最大绿1。(0..255)*/
    UInt8  nMaxGreen_2;                      /* 2.2.2.7最大绿灯时间2，简称最大绿2。(0..255)*/
    UInt8  nYellowTime;                      /*2.2.2.8相位黄灯时间，单位为1/10秒*/        
    UInt8  nAllRedTime;			            /*2.2.2.9 NTCIP称作相位红灯清空时间,也就是我们通常所说的全红时间，单位为1/10秒*/
    UInt8  nRedProtectedTime;                /*2.2.2.10保证黄灯以后不能直接返回绿灯控制的红灯时间保护*/
	UInt8  byPhaseAddedInitial;		        /*2.2.2.11相位增加初始值*/
	UInt8  byPhaseMaximumInitial;	        /*2.2.2.12相位最大初始值*/
	UInt8  byPhaseTimeBeforeReduction;       /*2.2.2.13 gap递减之前的时间与2.2.2.14综合使用*/
	UInt8  byPhaseCarsBeforeReduction;       /*2.2.2.14 gap递减之前通过的车辆数*/
	UInt8  byPhaseTimeToReduce;		        /*2.2.2.15 gap递减到minigap的时间*/
	UInt8  byPhaseReduceBy;			        /*2.2.2.16单位递减率，与2.2.2.15和16任选一个*/
	UInt8  byPhaseMinimumGap;		        /*2.2.2.17可以递减到的最小gap，应该和phaseTimeToReduce综合使用*/
	UInt8  byPhaseDynamicMaxLimit;	        /*2.2.2.18运行MAX的限定值。当小于此值时，运行MAX1，反之运行MAX2*/
	UInt8  byPhaseDynamicMaxStep;	        /*2.2.2.19动态调整步长*/    
	UInt8  byPhaseStartup;			        /*2.2.2.20相位初始化设置
					other(1)相位不使能（非定义）标志位（或者phaseOption的bit0=0或者phaseRing=0）
					phaseNotON(2)相位初始为红（非活动）
					greenWalk(3)相位初始为最小绿和行人时间
					greenNoWalk(4)相位初始为最小绿的开始
					yellowChange(5)相位初始为黄灯开始
					redClear(6)相位初始化为红灯开始*/

	UInt16  wPhaseOptions;			        /*2.2.2.21相位选项
					Bit 0 - Enabled Phase相位使能
					Bit 1 – 当进入自动闪光操作时，设置为1时此相位进行闪光操作，闪光前先进行红灯操作
					Bit 2 – 当闪光结束时，设置为1时首先运行此相位。
					Bit 3 – 非感应1
					Bit 4 -  非感应2
					Bit 5 – 设置为0时，检测器请求从相位黄灯开始记录。设置为1时，检测器请求操作依赖于detectorOptions对象。
					Bit 6 - Min Vehicle Recall人为设置为最小绿请求
					Bit 7 - Max Vehicle Recall人为设置为最大绿请求
					Bit 8 - Actuated Rest In Walk：1代表感应相位在冲突方没有请求时保持行人放行
					Bit 9 - Soft Vehicle Recall：当本相位没有实际请求而所有的冲突相位的MAX RECALL时间用完也没有请求驻留在绿灯时，给本相位一个soft recall，使相位转换到本相位。
					Bit 10 - Dual Entry Phase：设置为1时使一个没有检测器请求的相位跟随另一个环中同时放行的相位一起放行。
					Bit 11 - Simultaneous Gap Disable：设置为1使不进行Gap操作
					Bit 12 - Guaranteed Passage：设置为1时保证感应相位最后安全绿（passage）
					Bit 13 - Ped Recall人为设置为行人请求
					Bit 14 - Conditional Service Enable：相位时间没用完，把剩余的时间给在同一个barrier的相位。
					Bit 15 - Added Initial Calculation：设置为1时，选择检测器的最大值，设置为0时，选择检测器的加和。*/
    UInt8 nCircleID;                     /*2.2.2.22用到该相位的ring表号*/
    UInt8  byPhaseConcurrency[NUM_PHASE];		/*2.2.2.23可以并发的相位，每个字节代表一个可并发的相位号*/

}PhaseItem,*PPhaseItem;


//车辆检测器定义
struct STRU_N_VehicleDetector
{
	UInt8  byVehicleDetectorNumber;          /*2.3.2.1车辆检测器序列号。(1..48)*/
	UInt8  byVehicleDetectorOptions;	        /*2.3.2.2车辆检测器选项
			Bit 0 – 流量检测器
			Bit 1 – 占有率检测器
			Bit 2 -  相连相位在黄灯时间纪录车辆数
			Bit 3 -  相连相位在红灯时间纪录车辆数
			Bit 4 –感应相位增加单位延长绿（passage）
			Bit 5 –AddedInitial检测器
			Bit 6 – 排队检测器
			Bit 7 – 请求检测器*/
	UInt8  byVehicleDetectorCallPhase;       /*2.3.2.3该检测器对应的请求相位*/
	UInt8  byVehicleDetectorSwitchPhase;     /*2.3.2.4是个相位号,该相位可接收该车辆检测器的请求，当assigned phase 为红灯或黄灯时并且the Switch Phase时绿灯时，该相位被转换*/
	UInt16  byVehicleDetectorDelay;	        /*2.3.2.5当assigned phase不是绿灯时，检测器的输入将被延迟一段时间（00-999秒）。一旦发生了延迟，那麽之后发生的延迟将被累计*/
	UInt8  byVehicleDetectorExtend;          /*2.3.2.6当assigned phase是绿灯时，检测器的每次输入，相位都将在绿灯的终止点被延长一段时间（00-999秒）*/
	UInt8  byVehicleDetectorQueueLimit;      /*2.3.2.7检测器排队长度限制，当超过这一限制时，到达的车辆不再有效*/
	UInt8  byVehicleDetectorNoActivity;      /*2.3.2.8 0-255分钟，在这段指定的时间中检测器没有发出感应信息则被判断为一个错误，如果值为0，则不会被判断*/
	UInt8  byVehicleDetectorMaxPresence;     /*2.3.2.9 0-255分钟，在这段时间内，检测器持续发出感应信息，则被判断为一个错误，如果值为0，则不会被判断*/
	UInt8  byVehicleDetectorErraticCounts;	/*2.3.2.10每分钟0-255次，如果检测器发出的感应信息的频率超过这个值，则被判断为一个错误，如果值为0，则不会被判断*/
	UInt8  byVehicleDetectorFailTime;        /*2.3.2.11 检测器失败时间，单位：秒*/
	UInt8  byVehicleDetectorAlarms;	        /*2.3.2.12 检测器告警
					Bit 7: Other Fault – 其他故障
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Configuration Fault – 配置的检测器没有使用或跟一个不存在的相位联系
					Bit 3: Communications Fault –检测器通信错误
					Bit 2: Erratic Output Fault – 检测器计数错误(过快或过慢)
					Bit 1: Max Presence Fault – 检测器一直有车
					Bit 0：No Activity Fault - 检测器一直没车*/
	UInt8  byVehicleDetectorReportedAlarms; /*2.3.2.12检测器报告
					Bit 7: Reserved.
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Excessive Change Fault - 检测器计数过多。
					Bit 3: Shorted Loop Fault - 检测器闭环。
					Bit 2: Open Loop Fault – 检测器开环
					Bit 1: Watchdog Fault -  watchdog 错误
					Bit 0: Other – 其他错误*/
	UInt8  byVehicleDetectorReset;	/*2.3.2.13 当该对象被设置为非零时，将引起cu命令检测器重启。在cu发出重启命令之后该对象被自动设置为0*/
};		


//行人检测器定义
struct STRU_N_PedestrianDetector
{
	UInt8  byPedestrianDetectorNumber;		/*2.3.7.1行人检测器行号*/
	UInt8  byPedestrianDetectorCallPhase;	/*2.3.7.2行人检测器对应的请求相位*/
	UInt8  byPedestrianDetectorNoActivity;	/*2.3.7.3 0-255分钟，在这段指定的时间中行人检测器没有发出感应信息则被判断为一个错误，如果值为0，则不会被判断*/
	UInt8  byPedestrianDetectorMaxPresence;	/*2.3.7.4  0-255分钟，在这段时间内，行人检测器持续发出感应信息，则被判断为一个错误，如果值为0，则不会被判断*/
	UInt8  byPedestrianDetectorErraticCounts;/*2.3.7.5 每分钟0-255次，如果行人检测器发出的感应信息的频率超过这个值，则被判断为一个错误，如果值为0，则不会被判断*/
	//UInt8  byPedestrianDetectorAlarms;		/*2.3.7.6 行人检测器警告信息，
	//		Bit 7: Other Fault – 其他错误
	//		Bit 6: Reserved.
	//		Bit 5: Reserved.
	//		Bit 4: Configuration Fault – 配置错误
	//		Bit 3: Communications Fault – 通信错误
	//		Bit 2: Erratic Output Fault – 计数过多
	//		Bit 1: Max Presence Fault – 长期又车
	//		Bit 0: No Activity Fault – 长期没车*/
};

//调度表
typedef struct {
    UInt16  nScheduleID;                     /*2.4.3.2.1 调度计划号，由timeBaseScheduleMonth、timeBaseScheduleDate、timeBaseScheduleDate、timeBaseScheduleDayPlan四个参数共同决定计划是否可以执行。(1..40)*/
	UInt16  month;	                        /*2.4.3.2.2 bit1-bit12，每位表示一个月。置1表示允许对应计划在该月执行。(0..65535)*/
	UInt8   week;		                    /*2.4.3.2.3 bit1-bit7，每位表示一周中的一天。置1表示允许对应计划在该天执行。(0..255)*/
	UInt32  day;		                        /* 2.4.3.2.4 bit1-bit31，每位表示一月中的一天。置1表示允许对应计划在该天执行。(0..4294967295)*/
    UInt8   nTimeIntervalID;                 /*2.4.3.2.5时段表号，指向timeBaseScheduleDayPlan。0表示本行无效。(0..255)*/
}PlanScheduleItem,*PPlanScheduleItem;

//时段定义
typedef struct {

    UInt8   nTimeIntervalID;                 /*时段表号(1..16)，索引*/
    UInt8   nTimeID;                         /*时段（事件）号。(1..48)，索引,几个不同的事件可以在一天中的不同时段执行。如果两个事件出现的时段相同，则时段号小的先执行*/
    UInt8   cStartTimeHour;                  /*开始执行时刻的整点数，用时间（24时制）。*/
    UInt8   cStartTimeMinute;                /*开始执行的整分数*/
    UInt8   nActionID;                       /*配时方案号*/
}TimeIntervalItem,*PTimeIntervalItem;
//单元参数定义
typedef struct {

	UInt8 	nBootYellowLightTime;					// 2.4.1 启动时的闪光控制时间(秒)。启动时的闪光控制在掉电恢复后出现。掉电恢复具体包括哪些情况由设备定义。在这期间，硬件黄闪和信号灯监视是不活动的（如果有的话）。
	UInt8 	cIsPedestrianAutoClear;	                // 2.4.2行人自动清空参数（1 = False/Disable 2=True/Enable）。	当设置为1并且手动操作有效时，信号机便放行行人自动清空时间，以防止行人清空时间被预先设置的时间终止。
	UInt16	wUnitBackupTime;						// 2.4.3信号机离线后到降级前的时间。
	UInt8	byUnitRedRevert;						// 2.4.4最小红灯时间。此参数为所有的相位提供最小红灯时间（如：如果此值大于一个相位的红灯时间，则这个相位的红灯时间用这个参数来代替）。这个对象为黄灯之后和下一个绿灯之前提供这段时间提供一个最小指示。
	UInt8	byUnitControl;							// .10 允许远程控制实体激活信号机的某些功能( 0 = False / Disabled, 1 = True / Enabled)：
	//Bit 7: 灰度使能。设置为1时，表示进行通道灰度操作。为了实现这个功能，timebaseAscAuxillaryFunction参数必须设置为true。
	//Bit 6: 有缆协调 - 设置为1时，表示作为有缆协调的发起机。
	//Bit 5：Walk Rest Modifier - 当设置为1时，如果冲突相位没有服务请求，则感应相位停留在放行状态
	//Bit 4：Call to Non-Actuated 2 - 当设置为1时，使在phaseOptions字段中设置Non-Actuated 1的相位运行在非感应状态。
	//Bit3: Call to Non-Actuated 1 - 当设置为1时，使在phaseOptions字段中设置Non-Actuated 2的相位运行在非感应状态。
	//Bit2:External Minimum Recall -当设置为1时，使所有相位运行在最小请求状态
	//Bit 1～0: Reserved。
	UInt8  	byCoordOperationalMode;                 // 2.5.1协调的操作方式Automatic（0）：自动方式，可以为协调，感应，闪光等可以
	//ManualPattern（1~253）：运行手动设定的方案
	//ManualFree（254）：无协调自动感应
	//ManualFlash(255)：无协调自动闪光
	UInt8	byCoordCorrectionMode;                  // 2.5.2协调方式
	//other(1)协调建立在一个没有在本对象中定义的新的相位差
	//dwell(2)协调通过驻留协调相位立即变化达到相位差
	//shortway(3)协调通过某种限制周期变化的来减少和增加时间达到相位差，即平滑过渡
	//addOnly(4)协调通过某种限制周期变化的习惯来增加时间来达到相位差
	UInt8	byCoordMaximumMode;	                    // 2.5.3 协调的最大的方式
	//other(1)：不在此所定义的未知方式
	//maximum1(2)：Max1有效的协调
	//maximum2(3)：Max2有效的协调
	//maxinhibit(4)：当运行协调模式时，禁止运行最大绿
	UInt8	byCoordForceMode;		                // 2.5.4 Pattern强制模式
	//other(1)：信号机使用在此没有定义的模式
	//floating(2)：每个相位激活后强制到绿灯时间，允许不用的相位时间转化到协调相位
	//fixed(3)：每个相位被强制到周期固定位置，不用的相位时间加到接下来的相位中

	UInt8	byFlashFrequency;			            //闪光频率(UInt8)			
	UInt8	byThroughStreetTimeGap;		            //二次过街时差(UInt8)
	UInt8	byFluxCollectCycle;			            //流量采集周期(UInt8)
	UInt8    bySecondTimeDiff;			            //二次过街逆向协调参数
	UInt8	nBootAllRedTime;			            //启动全红时间
	UInt8	byCollectCycleUnit;			            //流量采集单位，秒 / 分钟 ( 0/1)
	UInt8	byUseStartOrder;			            //启用启动灯序
	UInt8	byCommOutTime;				            //通信超时时间
	UInt16	wSpeedCoef;					            //速度计算因子
	
	//自定义部分                         
	UInt8  acIpAddr[4];                     //IP地址
	UInt32          SubNetMask;                      //子网掩码
	UInt8  acGatwayIp[4];                   //网关
	UInt8           byPort;                          //端口号

	UInt8	byTransCycle;				            //平滑过渡周期
	UInt8	byOption;		                        //选项参数，按位取值
	//BIT 7---------高压不黄闪
	//BIT 6---------保留
	//BIT 5---------保留
	//BIT 4---------保留
	//BIT 3---------保留
	//BIT 2---------第一周期启用
	//BIT 1---------大灯启动序列
	//BIT 0---------启用板密码
	UInt8    byUnitTransIntensityCalCo;              //交通强度计算系数
}UnitPara,*PUnitPara;



//协调参数定义
struct STRU_N_CoordinationVariable
{
	UInt8	byCoordOperationalMode;		/*2.5.1协调的操作方式
					Automatic（0）：自动方式，可以为协调，感应，闪光等可以
					ManualPattern（1~253）：运行手动设定的方案
					ManualFree（254）：无协调自动感应
					ManualFlash(255)：无协调自动闪光*/
	UInt8	byCoordCorrectionMode;	/*2.5.2协调方式
					other(1)协调建立在一个没有在本对象中定义的新的相位差
					dwell(2)协调通过驻留协调相位立即变化达到相位差
					shortway(3)协调通过某种限制周期变化的来减少和增加时间达到相位差，即平滑过渡
					addOnly(4)协调通过某种限制周期变化的习惯来增加时间来达到相位差*/
	UInt8	byCoordMaximumMode;		/*2.5.3 协调的最大的方式
					other(1)：不在此所定义的未知方式
					maximum1(2)：Max1有效的协调
					maximum2(3)：Max2有效的协调
					maxinhibit(4)：当运行协调模式时，禁止运行最大绿*/
	UInt8	byCoordForceMode;			/*2.5.4 Pattern强制模式
					other(1)：信号机使用在此没有定义的模式
					floating(2)：每个相位激活后强制到绿灯时间，允许不用的相位时间转化到协调相位
					fixed(3)：每个相位被强制到周期固定位置，不用的相位时间加到接下来的相位中*/
	UInt8	byPatternTableType;		/*2.5.6定义方案表中需要的特殊组织结构
					other(1)：此处没有定义的
					patterns(2)：方案表的每一行代表唯一的一个方案，而且不依赖其他行
					offset3(3)：每个方案有3个相位差，占方案表的3行
					offset5(4)：每个方案有5个相位差，占5行*/
	UInt8	byCoordPatternStatus;		/*2.5.10 协调方案状态
				Not used（0）
				Pattern -（1-253）当前运行方案。
				Free - (254)感应
				Flash - (255)闪光*/
	UInt8	byLocalFreeStatus;		/*2.5.11 Free控制状态
				other: 其他状态
				notFree: 没有进行free控制
				commandFree: 
				transitionFree: 过渡free即将进入协调
				inputFree: 信号机输入导致free而不响应协调
				coordFree: the CU programming for the called pattern is to run Free.
				badPlan: 调用的方案不合法所以进行free
				badCycleTime: 周期不合法（不满足最小需求）所以进行free
				splitOverrun: 时间越界free
				invalidOffset: 保留或不用
				failed: 周期诊断导致free*/
	UInt16	wCoordCycleStatus;		/* 2.5.12 协调方案的周期状态（0-510sec），从周期长一直减少到0*/
	UInt16 	wCoordSyncStatus;			/*2.5.13协调同步状态（0－510）从协调基准点到目前周期的时间，从0记录到下个周期基准点。可以超过周期长*/
	UInt8 	bySystemPatternControl;	/*2.5.14系统方案控制
				Standby(0)系统放弃控制
				Pattern(1-253)系统控制方案号
				Free(254)call free 
				Flash(255)自动Flash */
	UInt8	bySystemSyncControl;		/*2.5.14 建立系统同步基准点*/
};


//老方案定义，周期长和相位差1个字节太小
typedef struct {

    UInt8 nSchemeID;                         /*2.5.7.1 方案表中该行属于哪个方案*/
    UInt8 nCycleTime;                        /*2.5.7.2 方案周期长,一个无感应的行人相位最小需求包括：miniGreen＋Walk＋PedClear＋yellow＋Red 一个感应的行人相位最小需求包括：miniGreen+Yellow+Red */
    UInt8 nOffset;                           /*2.5.7.3 相位差大小*/
    UInt8 nGreenSignalRatioID;               /*2.5.7.4方案对应的绿信比表号*/
    UInt8 nPhaseTurnID;                      /*2.5.7.5方案对应的相序表号*/
}SchemeItemOld,*PSchemeItemOld;

//新方案定义，周期长和相位差为2个字节
typedef struct {

    UInt8 nSchemeID;                         /*2.5.7.1 方案表中该行属于哪个方案*/
    UInt16 nCycleTime;                        /*2.5.7.2 方案周期长,一个无感应的行人相位最小需求包括：miniGreen＋Walk＋PedClear＋yellow＋Red 一个感应的行人相位最小需求包括：miniGreen+Yellow+Red */
    UInt16 nOffset;                           /*2.5.7.3 相位差大小*/
    UInt8 nGreenSignalRatioID;               /*2.5.7.4方案对应的绿信比表号*/
    UInt8 nPhaseTurnID;                      /*2.5.7.5方案对应的相序表号*/
}SchemeItem,*PSchemeItem;

//绿信比定义
typedef struct {

    UInt8 nGreenSignalRationID;              /* 2.5.9.1定义绿信比组号，一个组内的该字段相同*/
    UInt8 nPhaseID;                          /*2.5.9.2该行对应的相位号*/
    UInt8 nGreenSignalRationTime;            /*2.5.9.3对应绿信比相位绿时间*/
    UInt8 nType;                             /*2.5.9.4对于该相位应如何操作
            			other(1)：不再此处定义的模式
            			none(2)：没有绿信比模式控制
            			minimumVehicleRecall(3)：minimumVehRecall控制
            			maximumVehicleRecall(4)：maximumVehRecall控制
            			pedestrianRecall(5)：pedestrianRecall控制
            			maximumVehicleAndPedestrainRecall(6): maximumVeh&PedRecall控制
            			phaseOmitted(7)该相位被忽略*/
    
    UInt8 nIsCoordinate;                     /*2.5.9.5定义是否作为协调相位处理*/

}GreenSignalRationItem,*PGreenSignalRationItem;

//时基参数定义
struct STRU_N_TimeBaseVariable
{
	UInt16	wTimebaseAscPatternSync;	    /*2.6.1在经过午夜很短时间内的方案同步参考。设置为0XFFFF时，信号控制单元将把ACTION TIME作为方案的同步参考。*/
	UInt8	byTimebaseAscActionStatus;      /*2.6.4 这个对象表明当前用到的时基表号。*/
	UInt32	dwGlobalTime;				    /*2.4.1 UTC（或GMT）时间，从1970/1/1 00:00:00至今的秒数*/
	UInt8	byGlobalDaylightSaving;	        /*2.4.2夏令时
                        				other (1)：DST定义机制没有在本标准中。
                        				disableDST (2)：不使用DST
                        				enableUSDST (3)：DST使用美国习惯
                        				enableEuropeDST (4)：DST使用欧洲习惯
                        				enableAustraliaDST (5)：DST使用澳大利亚习惯
                        				enableTasmaniaDST (6) DST使用塔斯马尼亚习惯*/
	UInt8  	byDayPlanStatus;			    /*2．4．4．4表示活动时段表的编号。0表示没有活动的时段表*/
	UInt32 	nGlobalLocalTimeDifferential;	/*2．4．5 时差*/
	UInt32  	nGcontrollerStandardTimeZone;	/*2．4．6 本地标准时间与GMT的时差（秒）。正值表示本地时间在东半球，负值表示本地时间在西半球*/
	UInt32	dwControllerLocalTime;	        /*2．4．7 本地时间，等于1970/1/1 00:00:00以来的秒数*/
};

//时基动作定义
typedef struct {

    UInt8  nActionID;                        /* 2.6.3.1动作号*/                        
    UInt8  nSchemeID;                        /* 2.6.3.2感应方案号。这个参数不能超过maxPatterns, flash,或者free的值。设置为0表明没有方案被选择。*/
    UInt8  nPhaseTableID;                       /* 2.6.3.3方案表对应的相位表号*/
    UInt8  nChannelTableID;                     /* 2.6.3.4方案表对应的通道表*/
}ActionItem,*PActionItem;


//相序表定义
typedef struct {

    UInt8 nPhaseTurnID;                      /*2.8.3.1相序表号*/
    UInt8 nCircleID;                         /* 2.8.3.2环号*/
    UInt8 nTurnArray[32];             /*2.8.3.3 相序组*/

}PhaseTurnItem,*PPhaseTurnItem;



//通道定义
typedef struct {

    UInt8 nChannelID;                        /*2.9.2.1通道号，不能大于maxChannels。(1..32)*/
    UInt8 nControllerID;                     /*通道控制源[相位(phase)或者重叠(overlap)]，由channelControlType决定是相位或是重叠，不能大于最大相位数和最大重叠数。*/
    UInt8 nControllerType;                   /*2.9.2.2通道控制类型，有四个取值，other(1),phaseVehicle (2),phasePedestrian (3),overlap (4)，分别表示其他相位控制、机动车相位控制、行人相位控制和重叠控制。*/
    UInt8 nFlashLightType;                   /*2.9.2.3自动闪光状态。
                                        	Bit 7: Reserved
                                        	Bit 6: Reserved
                                			Bit 5: Reserved
                                			Bit 4: Reserved
                                			Bit 3: 交替闪光
                                				Bit=0: Off/Disabled & Bit=1: On/Enabled
                                			Bit 2: 红闪
                                				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
                                			Bit 1: 黄闪
                                				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
                                			Bit 0: Reserved
                                        	Bit 1 和 Bit 2 同时为1的效果是Bit 1 = 0，Bit 2 = 1，Reserved位必须为0，否则返回badValue(3)错误。*/
	UInt8  	byChannelDim;		            /*2.9.2.4通道灰度状态
                                			Bit 7: Reserved
                                			Bit 6: Reserved
                                			Bit 5: Reserved
                                			Bit 4: Reserved
                                			Bit 3: Dim Alternate Half Line Cycle
                                				Bit=0: Off/+ half cycle &
                                				Bit=1: On/- half cycle
                                			Bit 2: Dim Red
                                				Bit=0: Off/Red Not Dimmed &
                                				Bit=1: On/Dimmed Red
                                			Bit 1: Dim Yellow
                                				Bit=0: Off / Yellow Not Dimmed &
                                				Bit=1: On / Dimmed Yellow
                                			Bit 0: Dim Green
                                				Bit=0: Off / Green Not Dimmed &
                                				Bit=1: On / Dimmed Green*/

}ChannelItem,*PChannelItem;

typedef struct {

    UInt8 nFollowPhaseID;                    /*2.10.2.1 overlapNumber：overlap号，	不超过maxOverlaps。1 = Overlap A, 2 = Overlap B etc */
    UInt8 byOverlapType;                     /*2.10.2.2 overlap操作类型，枚举如下：
		other(1) ：未在此描述的操作类型。
		normal(2)：此种操作类型时，overlap的输出受overlapIncludedPhases参数控制。有下列情形时overlap输出绿灯：
		当overlap包含的相位是绿灯时。
		当overlap包含的相位是黄灯（或者全红red clearance）且overlap包含下一相位（included phase is next）时。
		如果overlap包含的相位是黄灯且overlap不包含下一相位（included phase is not next），overlap输出黄灯。如果overlap的绿灯和黄灯无效，将输出红灯。
		minusGreenYellow(3)：此种操作类型时，overlap的输出受overlapIncludedPhases和overlapModifierPhases参数控制。有下列情形时overlap输出绿灯：
		当overlap包含相位是绿灯且overlap的修正相位不是绿灯时（NOT green）
		当overlap包含的相位是黄灯（或者全红red clearance）且overlap包含下一相位（included phase is next）且overlap的修正相位不是绿灯时。
		如果overlap包含的相位是黄灯且overlap的修正相位不是黄灯且overlap不包含下一相位（included phase is not next），overlap输出黄灯。如果overlap的绿灯和黄灯无效，将输出红灯。*/
    UInt8 nArrayMotherPhase[NUM_PHASE];/*2.10.2.3 overlap包含的相位，每字节表示一个相位号。*/
    UInt8 byArrOverlapModifierPhases[MAX_PHASE_COUNT_MO_OVERLAP];	/*2.10.2.4 overlap的修正相位，每字节表示一个相位号。	如果为空值（null），overlapType为normal，如果为非空值（non-null），overlapType为minusGreenYellow。*/
    UInt8 nGreenTime;                        /*2.10.2.5  0-255秒，	如果此参数大于0且overlap的绿灯正常结束，overlap绿灯将延长此参数设定的秒数。*/
    UInt8 nYellowTime;                       /*2.10.2.6  3-25.5秒。如果overlap的绿灯被延长	（Trailing Green大于零），此参数将决定overlap Yellow Change的时间间隔长度。*/
    UInt8 nRedTime;                          /*2.10.2.7  0-25.5秒。如果overlap的绿灯被延长	（Trailing Green），此参数将决定overlap Red Clearance的时间间隔长度。*/
    
}FollowPhaseItem,*PFollowPhaseItem;


//优先配置定义
struct STRU_N_Preempt
{
	UInt8	byPreemptNumber;
	UInt8	byPreemptControl;
	UInt8	byPreemptLink;
	UInt16	wPreemptDelay;
	UInt16	wPreemptMinimumDuration;
	UInt8	byPreemptMinimumGreen;
	UInt8	byPreemptMinimumWalk;
	UInt8	byPreemptEnterPedClear;
	UInt8 	byPreemptTrackGreen;
	UInt8	byPreemptDwellGreen;
	UInt16	wPreemptMaximumPresence;
	UInt8	abyPreemptTrackPhase[NUM_PHASE];
	UInt8	byPreemptTrackPhaseLen;
	UInt8	abyPreemptDwellPhase[NUM_PHASE];
	UInt8	byPreemptDwellPhaseLen;
	UInt8	abyPreemptDwellPed[MAX_PEDESTRIANPHASE_COUNT];
	UInt8	byPreemptDwellPedLen;
	UInt8	abyPreemptExitPhase[NUM_PHASE];
	UInt8	byPreemptExitPhaseLen;
	UInt8	byPreemptState;
	UInt8	abyPreemptTrackOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptTrackOverlapLen;
	UInt8	abyPreemptDwellOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptDwellOverlapLen;
	UInt8 	abyPreemptCyclingPhase[NUM_PHASE];
	UInt8	byPreemptCyclingPhaseLen;
	UInt8	abyPreemptCyclingPed[MAX_PEDESTRIANPHASE_COUNT];
	UInt8	byPreemptCyclingPedLen;
	UInt8	abyPreemptCyclingOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptCyclingOverlapLen;
	UInt8	byPreemptEnterYellowChange;
	UInt8 	byPreemptEnterRedClear;
	UInt8	byPreemptTrackYellowChange;
	UInt8	byPreemptTrackRedClear;
};

/*==================================================================*/
/*消息名称:STRU_DynPatternSel										*/
/*说    明:动态方案选择配置表结构定义								*/
/*==================================================================*/
struct STRU_DynPatternSel
{
	UInt8  	byDynSn;			//索引号
	UInt8	byDynUpperLimit;	//阀值
	UInt8	byDynPatternNo;		//方案号
};



//倒计时牌配置关系结构，包括机动车和行人 
struct STRU_N_CountDownBoard
{
	UInt8 byCountdownNo; //倒计时牌号
	UInt8 abyIncludedPhaseNo[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//对应相位号
	UInt8 abyIncludedPhaseChannel[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//相位对应通道号 
	UInt8 abyIncludedPhaseType[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//通道对应相位类型 
	UInt8 byIncludedPhaseNum;//倒计时牌对应的相位数 
};

/*==================================================================*/
/*消息名称:STRU_SignalTransEntry									*/
/*说    明:信号转换序列结构定义										*/
/*==================================================================*/
struct STRU_SignalTransEntry
{
	UInt8  	byPhaseNumber;		//相位号
	UInt8	byRedYellow;		//信号转换序列中的红黄时间
	UInt8	nGreenLightTime;		//信号转换序列中的绿闪时间
	UInt8	bySafeRed;			//信号转换序列中的安全红灯时间
	UInt8	byGreenTime;		//第一周期绿灯
	UInt8	byAllStopPhase;		//跟随的同断的相位号

	UInt8 	byPhaseDirectionProperty;		//相位的方向属性，按位标志方向
															//方向：东、西、南、北、东北、东南、西南、
															//西北、南北、东西、东南西北、东北西南

	UInt8 	byPhaseLaneProperty;			//相位的方向车道属性，按位表示车道
															//车道：左、直、右、左直、左直右、直右、
															//掉头
	//UInt8 	byPhaseReservedPara;			//相位的保留属性，以后使用
};

//事件配置一致组
struct STRU_N_EventConfig	//这个表包括事件日志的配置信息，这个表定义的参数设备将进行监视并产生事件
{
	UInt16 wEventConfigID;				//行号
	UInt8 byEventConfigClass;			//事件号
	UInt8 byEventConfigMode;			//事件模式，主要根据这个参数进行事件的设置和产生
	/*int 	   nEventConfigCompareValue;	//比较值
	int 	   nEventConfigCompareValue2;	//这个值用来定义时间周期，为0时表示没有周期，事件立即生效
	int 	   anEventConfigCompareOID[128];//进行比较的OID的值，此值必须是一个INTEGER类型，否则BADVALUE error
	UInt8  byEventConfigCompareOIDLen;
	int 	   anEventConfigLogOID[128];		//将要存入的值
	UInt8  byEventConfigLogOIDLen;
	*/
	UInt8 byEventConfigAction;			//事件动作，表明纪录与否
};


//倒计时消息配置
struct SCountDownCfg_DEL		//倒计时参数，设置显示开关、波特率等参数,不再使用
{
	UInt8	byDriverFlag;			//信号机驱动倒计时牌标志
	UInt16	wRs232BaudRate;			//波特率
	UInt8  	byRs232DataBits;		//数据位											
	UInt8  	byRs232StopBits;		//停止位	
	UInt8  	byRs232ParityBits;		//奇偶校验位
};

struct STRU_ChannelRedLightCurrent
{
	UInt8  	byChannelNumber;			//通道号
	UInt8		byChannelRedCurrentBase;	//通道的红灯电流基准值
	UInt8		byChannelRedCurrentDelta;	//通道的红灯电流差值
};

struct STRU_BasicFaultDetect
{
    UInt16 wGreenSumValue;         // 绿灯电流总和
    UInt16 wGreenDiffValue;        // 绿灯差值
    UInt16 wOverflowVoltageCount;  // 过欠压检测滤波次数，默认50
    UInt8 byRedOffCount;          // 红灯熄灭检测滤波次数，默认4
    UInt8 byConflictCount;        // 绿灯冲突检测滤波次数，默认2
    UInt8 byManualKeyCount;       // 手动面板按键检测滤波次数，默认5
    UInt8 byTeleKeyCount;         // 遥控器按键检测滤波次数，默认50
	UInt8 byRedFaultSwitch;	     // 检测开关,bit0----故障检测总开关，默认为1
                                 //   bit1----电流检测开关，默认为0
                                 //   bit2----电压检测开关，默认为0
                                 //   bit3----电流故障报警处理开关，默认为1，
                                 //           即电流故障后报警并降级闪光，否则只报警
                                 //   bit4----电压故障报警处理开关，默认为1，
                                 //           即电流故障后报警并降级闪光，否则只报警
                                 //   bit5----watchdog启用开关，默认为1，即打开
};

struct STRU_FaultDetectSet
{
    struct STRU_ChannelRedLightCurrent AscChannelCurrentRedTable[NUM_CHANNEL];
    struct STRU_BasicFaultDetect       AscBasicFaultDetect;
};

struct STRU_Address
{
    UInt16 wSerialAddr;        //串口地址
    UInt8  ucIpAddr[4];       //IP地址
    UInt32 SubNetMask;        //子网掩码
    UInt8  ucGatwayIp[4];     //网关
    UInt8  byPort;            //端口号
    UInt8  ucMacAddr[6];      //MAC地址
    UInt8 reserved1;			//以下为保留字段
    UInt8 reserved2;
    UInt8 reserved3;
    UInt8 reserved4;
    UInt8 reserved5;
    UInt8 reserved6;
    UInt8 reserved7;
    UInt8 reserved8;
};
//对绿信比表附加上行人的数据
struct STRU_SplitPedEx
{
	UInt8 byWalk;		//行人放行时间
	UInt8 byClear;		//行人清空时间
	UInt8 byClearMode;	//行人清空模式，1表示红闪，其他表示绿闪
	UInt8 byCriticalGreen;	//临界绿灯
};
//对相位附加些灯色的设置
struct STRU_PhaseEx
{
	UInt8 byGreenTime;	//第一周期绿灯放行时间
	UInt8 byReserved1;	//以下参数保留
	UInt8 byReserved2;
	UInt8 byReserved3;
	UInt8 byReserved4;
	UInt8 byReserved5;
	UInt8 byReserved6;
	UInt8 byReserved7;
};


//交通强度选择定义 
struct STRU_N_TransIntensityChoice
{
	UInt8 byIndexNumber;   /*80.62.1.1 索引*/
	UInt8 byKeyPhase;      /*80.62.1.2 关键相位数*/
	UInt8 byTransIntensityLower; /*80.62.1.3 交通强度下限*/
	UInt8 byCycleTime;     /*80.62.1.4 周期*/
};
struct STRU_N_OverlapTableEx
{
	UInt8 byOverlapGreenFlash;/*80.61.1.1 */
};
//新增单元优化参数，显示在单元参数中，增加几个保留字段（4个字节和1个字）
struct STRU_UnitOptiPara  
{	

	UInt8	bySysMaxCycle;		//最大周期长（0～255s），优化使用（系统、单点）
	UInt8	bySysMinCycle;		//最小周期长（0～255s），优化使用（系统、单点）
	UInt8	bySysMaxIntensity;	//最大交通强度（0～100，实际值放大100倍，默认0.8）
	UInt8	bySysMinIntensity;		//最小交通强度（0～100，实际值放大100倍，默认0.3）
	UInt8	bySysOptiPara_a;		//周期计算公式参数a（0～max，默认为1）
	UInt8	bySysOptiPara_b;		//周期计算公式参数b（0～max，默认为1）
	UInt8	byDecInternalTime;	//滚动占有率采集间隔(1~60s)，默认为10s 
	UInt8	byDecDyFrequency;	//滚动占有率采集频率(1~255s)，默认为1s
//	UInt8	byReservedPara_1;		//保留参数1，此次定义节点，不做任何定义界面显示
//	UInt8	byReservedPara_2;		//保留参数2，此次定义节点，不做任何定义界面显示	
	UInt8	abyDegradePlanSeq[10];	//降级方案序列

};


//车辆检测器EX定义 
struct STRU_N_VehicleDetectorEx
{
	UInt8 byVehicleDetectorPasstime; /*80.60.1.1 车辆通过线圈至停车线间的时间 (0-255)*/
	UInt8 byVehicleDetectorStopNum;  /*80.60.1.2 线圈至停车线间停放车辆数 (0-255)*/
	UInt8 byVehicleDetectorPriority; /*80.60.1.3 检测器优先级 (0-255)*/
	UInt8 byCorPhaseSatTimeOccupancy; /*80.60.1.4 对应相位饱和时间占有率 (0-100)*/ 
	UInt16 byCorPhaseSatVolume;        /*80.60.1.5 对应相位饱和流量 (1500-1800)*/ 
	UInt8 byCongestionOccupancy;//拥堵触发占有率，默认100
	UInt8 	byVehicleDetectorOptionEx;//检测器选项，最低位标志检测器属性为拥堵检测器，其他位保留，默认0
};
struct STRU_UnitParamEx
{
	UInt8 byStartAllRedTime;	//启动全红时间
	UInt8 byCollectCycleMin;	//流量采集单位，秒 / 分钟
	UInt8 byUseStartOrder;	//启用启动灯序
	UInt8 byCommOutTime;		//通信超时时间
	UInt16 wSpeedCoef;		//速度计算因子
	UInt8 byTransCycle;		//平滑过渡周期
	UInt8 byOption;		    //选项参数，按位取值
							//BIT 7---------高压不黄闪
							//BIT 6---------保留
							//BIT 5---------保留
							//BIT 4---------保留
							//BIT 3---------保留
							//BIT 2---------第一周期启用
							//BIT 1---------大灯启动序列
							//BIT 0---------启用板密码
	UInt8 byTransIntensityCalCo;		//交通强度计算系数
	UInt8 byReserved2;
	UInt8 byReserved3;
	UInt8 byReserved4;
	UInt8 byReserved5;
	UInt8 byReserved6;
	UInt8 byReserved7;
	UInt8 byReserved8;
	UInt8 byReserved9;
	UInt8 byReserved10;
	UInt8 byReserved11;
	UInt8 byReserved12;
	UInt8 byReserved13;
	UInt8 byReserved14;
};



typedef struct {
    PhaseItem					stOldPhase[NUM_PHASE];                                     //之前未使用的相位表
    struct STRU_N_VehicleDetector		AscVehicleDetectorTable[MAX_VEHICLEDETECTOR_COUNT];     //车辆检测器表
    struct STRU_N_PedestrianDetector	AscPedestrianDetectorTable[MAX_PEDESTRIANDETECTOR_COUNT];//行人检测器表
    UnitPara 					stUnitPara;                                             //单元参数

    struct STRU_N_CoordinationVariable	AscCoordinationVariable;                                //协调参数
    SchemeItemOld					stOldScheme[NUM_SCHEME];                                   //方案表
    GreenSignalRationItem		stGreenSignalRation[NUM_GREEN_SIGNAL_RATION][NUM_PHASE];//绿信比表
    struct STRU_N_TimeBaseVariable		AscTimeBaseVariable;                                    //时基参数
    ActionItem					stAction[NUM_ACTION];                                   //动作表
    PlanScheduleItem			stPlanSchedule[NUM_SCHEDULE];                           //调度表
    TimeIntervalItem			stTimeInterval[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];//时段表
    PhaseTurnItem				stPhaseTurn[NUM_PHASE_TURN][NUM_RING_COUNT];            //相序表
    ChannelItem					stOldChannel[NUM_CHANNEL];                            //通道表
    FollowPhaseItem				stOldFollowPhase[NUM_FOLLOW_PHASE];                   //跟随相位
    //STRU_N_OverlapTable			AscOverlapTable[MAX_OVERLAP_COUNT];                  //重叠表
    struct STRU_N_Preempt				AscPreemptTable[MAX_PREEMPT_COUNT];                     //优先表
    struct STRU_DynPatternSel          AscDynPatternSel[MAX_DYNPATTERNSEL_COUNT];              //动态方案选择表
    struct STRU_N_CountDownBoard       AscCountDownBoardCfg[MAX_COUNTDOWNBOARD_COUNT];         //倒计时牌表

    struct STRU_SignalTransEntry	    OldAscSignalTransTable[NUM_PHASE];                   //信号转换序列表,其中定义了相位绿闪时间
    struct STRU_N_EventConfig          AscEventCfgTable[MAX_EVENTCONFIG_COUNT];                //故障配置表
    struct SCountDownCfg_DEL		    AscCountDownCfg;                                        //倒计时表
    struct STRU_FaultDetectSet         AscFaultDectectSet;                                     //故障检测设置
    struct STRU_Address                AscAddress;                                             //地址，包括串口地址和网口地址
    char						AscChannelNotes[NUM_CHANNEL][40];                 //通道接线注释
    struct STRU_UnitParamEx			AscUnitParamEx;                                         //增加的单元参数内容
    struct STRU_SplitPedEx				AscSplitPedEx[NUM_GREEN_SIGNAL_RATION][NUM_PHASE];        //增加的绿信比中行人的时间
    struct STRU_PhaseEx				AscPhaseEx[NUM_PHASE];                            //对相位表附加些功能，
    struct STRU_N_VehicleDetectorEx	AscVehicleDetectorTableEx[MAX_VEHICLEDETECTOR_COUNT];   //车辆检测器表
    struct STRU_N_TransIntensityChoice AscTransIntensityChoiceTable[MAX_TRANSINTENSITYCHOICE_COUNT];//交通强度选择表
    struct STRU_N_OverlapTableEx       AscOverlapTableEx[MAX_OVERLAP_COUNT];                   //Overlap绿闪设置表
    struct STRU_UnitOptiPara			AscUnitOptiPara;                                        //信号机单元优化参数
	PhaseItem					stPhase[MAX_PHASE_TABLE_COUNT][NUM_PHASE];                                     //相位表,其中也定义了可以并发的相位ID
    struct STRU_SignalTransEntry	    AscSignalTransTable[MAX_PHASE_TABLE_COUNT][NUM_PHASE];                   //信号转换序列表,其中定义了相位绿闪时间
	ChannelItem					stChannel[MAX_CHANNEL_TABLE_COUNT][NUM_CHANNEL];                                 //通道表
	FollowPhaseItem				stFollowPhase[MAX_FOLLOW_PHASE_TABLE_COUNT][NUM_FOLLOW_PHASE];                        //跟随相位
    SchemeItem					stScheme[NUM_SCHEME];                                   //新方案表
}SignalControllerPara,*PSignalControllerPara;//信号机配置参数主结构

#pragma pack(pop)

/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/

 
extern int IsSignalControlparaLegal(SignalControllerPara *pSignalControlpara);
extern Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara);


/*Read*/
extern unsigned int ArrayToInt(unsigned char *array,int len);
extern void ReadUnitPara(PUnitPara item);
extern void ReadPhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE]);
extern void ReadChannelItem(ChannelItem channel[][NUM_CHANNEL]);
extern void ReadGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE]);
extern void ReadPhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT]);
extern void ReadSchemeItem(SchemeItem *scheme);
extern void ReadActionItem(ActionItem *action);
extern void ReadTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID]);
extern void ReadPlanSchedule(PlanScheduleItem *schedule);
extern void ReadFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE]);
extern void ReadVehicleDetector(struct STRU_N_VehicleDetector *vehicle);
extern void ReadPedestrianDetector(struct STRU_N_PedestrianDetector *item, int num);

/*Write*/
extern Boolean WriteConfigFile(SignalControllerPara *param);

extern void WriteUnitPara(PUnitPara item);
extern void WritePhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE]);
extern void WriteChannelItem(ChannelItem channel[][NUM_CHANNEL]);
extern void WriteGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE]);
extern void WritePhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT]);
extern void WriteSchemeItem(SchemeItem *scheme);
extern void WriteActionItem(ActionItem *action);
extern void WriteTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID]);
extern void WritePlanSchedule(PlanScheduleItem *schedule);
extern void WriteFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE]);
extern void WriteVehicleDetector(struct STRU_N_VehicleDetector *vehicle);
extern void WritePedestrianDetector(struct STRU_N_PedestrianDetector *item);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKCONFIG_H__ */
