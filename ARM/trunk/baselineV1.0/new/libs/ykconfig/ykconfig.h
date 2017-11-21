#ifndef __YKCONFIG_H__
#define __YKCONFIG_H__


#ifndef NUM_PHASE
#define NUM_PHASE					16	//最大相位个数
#endif
#ifndef NUM_CHANNEL
#define NUM_CHANNEL					32	//最大通道个数
#endif
#ifndef NUM_SCHEDULE
#define NUM_SCHEDULE				40	//最大调度表个数
#endif
#ifndef NUM_TIME_INTERVAL
#define NUM_TIME_INTERVAL           16  //时段表总数
#endif
#ifndef NUM_TIME_INTERVAL_ID
#define NUM_TIME_INTERVAL_ID    	48  //每个时段表含有的时段号总数
#endif
#ifndef	NUM_SCHEME
#define NUM_SCHEME					16	//最大方案个数
#endif


#ifndef __HIK_H__
typedef unsigned char 		UInt8;
typedef unsigned short 		UInt16;
typedef unsigned int		UInt32;
typedef unsigned long long	UInt64;

typedef enum
{
	FALSE = 0,
	TRUE,
} Boolean;

#endif

#ifndef __HIKCONFIG_H__
typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//计划时间

#define BIT(v, n) (((v) >> (n)) & 0x1)		//取v的第 n bit位

#endif

//协议头部结构，头部后面就是下面定义的数据结构体,也就是值域部分
typedef struct
{
	UInt32 iHead;		//上载为0x7e7e，下载为0x8e8e
	UInt32 iType;		//具体定义如下所示
	UInt32 iValue[0];	//值域，具体数据为如下所示的参数结构体,此成员不占用存储空间，如果windows端不支持0数组定义，可以注释掉
} YK_ProtocolHead;

/***********************************************************************************
在上下载调度表、时段表、方案表以及整体配置这几个配置表项时需要先发送上下载开始和结束标志，具体如下所示：
开始标志:	iType = 0x7e00
结束标志:	iType = 0x7eff
后面无需传输任何数据类型，只需要传输iHead和iType就可以，无需值域部分
***********************************************************************************/

/*所有的上载返回对应的数据结构，而下载则返回12字节，包含iHead、iType和一个4字节返回值(也就是iValue[0]),
返回值为0表示正常，反之则异常,另外在发送上载开始和结束标志信息时返回值与下载时返回值一致*/

/*
iType		表项
0x7f01		调度表
0x7f02		时段表
0x7f03		通道表
0x7f04		方案表
0x7f05		整体配置
0x7f06		车流量信息表
0x7f07		通道锁定
0x7f08		实时状态信息
0x7f09		手动控制
0x7f0a		故障日志
0x7f0b		对时
0x7f0c		网络配置
*/



//调度表结构，iType为0x7f01
typedef struct 
{
    UInt16  nScheduleID;    /*调度计划号，范围[1,40],上下载时都需要指定一个编号*/
	UInt16  month;	  /* bit1-bit12，每位表示一个月。置1表示允许对应计划在该月执行*/
	UInt8   week;	/* bit1-bit7分别对应星期日、星期1到6，每位表示一周中的一天。置1表示允许对应计划在该天执行*/
	UInt32  day;	/* bit1-bit31，每位表示一月中的一天。置1表示允许对应计划在该天执行*/
    UInt8   nTimeIntervalID;   /*时段表号，范围[1,16]*/
} YK_PlanScheduleItem;

//时段表结构，iType为0x7f02
typedef struct
{
    UInt8   nTimeIntervalID;                 /*时段表号,范围[1,16],上下载时都需要指定一个编号*/
    UInt8   nTimeID;                        /*时段号，范围[1,48],上下载时都需要指定一个编号*/
    UInt8   cStartTimeHour;                  /*开始执行时刻的整点数，用时间（24时制）。*/
    UInt8   cStartTimeMinute;                /*开始执行的整分数*/
    UInt8   nSchemeId;                       /*配时方案号*/
	UInt16	phaseOffset;					 /*相位差*/
	UInt8	IsCorrdinateCtl;				 /*是否是协调控制,0:不协调，1:协调*/
} YK_TimeIntervalItem;

//通道表结构体，iType为0x7f03
typedef struct
{
    UInt8 nChannelID;                        /*通道号，范围[1,32],上下载时都需要指定一个编号*/
    UInt8 nControllerType;                   /*通道控制类型，机动车通道值为2，行人通道值为3，其他值无效*/
    UInt8 nFlashLightType;                   /*自动闪光状态，默认为2
                                        	Bit 7-4: Reserved
                                			Bit 3: 交替闪光
                                				Bit=0: Off/Disabled & Bit=1: On/Enabled
                                			Bit 2: 红闪
                                				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
                                			Bit 1: 黄闪
                                				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
                                			Bit 0: Reserved
                                        	Bit 1 和 Bit 2 同时为1的效果是Bit1 = 0，Bit2 = 1*/
	UInt8 nVehDetectorNum;					/*通道对应的车检器编号*/
	UInt32 conflictChannel;					/*冲突通道，bit0-31分别代表通道1-32*/
} YK_ChannelItem;

typedef struct
{	//以下所有时间单位统一都为秒
	UInt8 greenTime;			//绿灯时间，第1步
	UInt8 greenBlinkTime;		//绿闪时间，第2步
	UInt8 yellowTime;			//黄灯时间，第3步
	UInt8 redYellowTime;		//红黄时间，第4步	!!!!!!!!!!!!!!!!!!!!!!!!!
	UInt8 allRedTime;			//全红时间，第5步	!!!!!!!!!!!!!!!!!!!!!!!!!
	//UInt8 pedestrianClearTime;	//行人清空时间，即行人绿闪时间	++++++++++++++++
	UInt8 minGreenTime;			//最小绿
	UInt8 maxGreenTime;			//最大绿
	UInt8 unitExtendTime;		//单位延长绿
	UInt32 channelBits;			//相位包含的通道，bit0-bit31分别代表通道1-32*/
} YK_PhaseInfo;

//方案表结构，iType为0x7f04
typedef struct
{
	UInt8 nSchemeId;					//方案号,范围[1,16],上下载时都需要指定一个编号
	UInt16 cycleTime;					//周期时间，单位为秒
	UInt8 totalPhaseNum;				//方案包含的相位个数,最大16个
	YK_PhaseInfo phaseInfo[NUM_PHASE];	//方案包含的相位信息，具体内容如上所示
} YK_SchemeItem;

//信号机整体配置结构，iType为0x7f05
typedef struct
{
	UInt8 bootYellowBlinkTime;		//启动黄闪时间
	UInt8 bootAllRedTime;			//启动全红时间
	UInt8 vehFlowCollectCycleTime;	//车流量采集周期,默认5s
	UInt8 transitionCycle;			//绿波控制时的过渡周期个数
	UInt8 adaptCtlEndRunCycleNum;	//自适应感应控制结束运行的周期个数
	UInt8 watchdogEnable:1;			//watchdog使能开关
	UInt8 signalMachineType:2;		//信号机类型选项,有三种信号机TSC500、TSC300-44、TSC300-22,对应的值分别为0、1、2
	UInt8 weekPreemptSchedule:1;	//星期优先调度，默认月日优先调度 ++++++++++++++
} YK_WholeConfig;


//流量占有率结构
typedef struct 
{
    /*提供两位小数点精度支持，实际值乘以100后进行存储，给用户显示时，需要除以100才能是实际值*/
	UInt8    byDetectorVolume;			/*2.3.5.4.1一个检测器在一个周期中采集的流量数,单位是 辆/周期*/
	UInt16    byDetectorOccupancy;		/*2.3.5.4.2一个检测器在一个周期中的占有率 这里我们提供的是时间占有率，除以100，得到百分比值*/
	UInt16	byVehicleSpeed;		        /*车速,按照*100存储的，所以实际值应该除以100后，单位才是km/h*/
	UInt16	wQueueLengh;		        /*排队长度,按照*100存储，除以100后，才能得到实际的车辆数，单位是辆*/
	UInt16    wGreenLost;                 /*绿损 损失绿灯时长/绿灯总时长 ，该值除以100即是*/
	UInt16    wVehicleDensity;            /*车流密度，按照*100存储，除以100后单位是 辆/km*/
	UInt16    wVehicleHeadDistance;       /*车头间距，同一车道、同一方向连续行驶前后相邻两辆车之间在某一瞬时的车头间距。符号：hd 单位：m/辆。*/
    UInt16    wVehicleHeadTimeDistance;   /*车头时距：同一车道、同一方向连续行驶前后相邻的两辆车的车头通过某一点的时间间隔。符号：ht 单位：s/辆 */
} YK_VolumeOccupancy;	

//历史流量与时间对应结构，iType为0x7f06，只提供上载
typedef struct 
{
    UInt32 dwTime;                       //历史时间点
    YK_VolumeOccupancy  struVolume[48]; //对应历史流量
} YK_TimeAndHistoryVolume;

//通道锁定结构，iType为0x7f07,只提供下载
typedef struct
{
	unsigned char    ucChannelStatus[NUM_CHANNEL];		//32个通道状态，数据如枚举LightStatus所示
	unsigned char    ucBeginTimeHour;			//控制起效时间：小时
	unsigned char    ucBeginTimeMin;			//控制起效时间：分钟
	unsigned char    ucBeginTimeSec;			//控制起效时间：秒
	unsigned char    ucEndTimeHour;			//控制结束时间：小时
	unsigned char    ucEndTimeMin;			//控制结束时间：分钟
	unsigned char    ucEndTimeSec;			//控制结束时间：秒
} YK_ChannnelLockParams;

//实时状态结构，iType为0x7f08，只提供上载
typedef struct
{
	UInt8 allChannels[NUM_CHANNEL];	//所有通道的状态，数据如枚举LightStatus所示
	UInt16 cycleTime;				//当前周期总时间
	UInt8 runPhase;					//当前运行相位
	UInt8 schemeId;                 //当前运行的方案号
	UInt8 stepNum;					//当前相位运行的步编号
	UInt8 stepLeftTime;				//当前运行步的剩余时间
} YK_RealTimeInfo;

//手动控制结构，iType为0x7f09，只提供下载
typedef UInt8 YK_ManualControl;		//具体要执行的方案号如上面宏定义所示

/*故障日志信息结构，每个结构代表一行日志，一次性上载16个，
如果上载所得结构的nTime==0则说明没有日志可供上载此时，应该终止发送上载日志信息，
iType为0x7f0a,另外提供清除故障日志功能，清除属于下载，所以此时iHead应为0x8e8e*/


//typedef FaultLogInfo YK_FaultLog[16];


//对时结构，iType为0x7f0b
struct YK_SAdjustTime
{
	unsigned long			ulGlobalTime;		//UTC时间,以秒为单位
	unsigned int			unTimeZone;			//时差,以秒为单位
};

//网络配置结构，统一为网络字节序，iType为0x7f0c
typedef struct
{
	UInt32 ip;
	UInt32 netmask;
	UInt32 gateway;
} YK_NetworkParam;

typedef struct
{
	YK_PlanScheduleItem scheduleTable[NUM_SCHEDULE];
	YK_TimeIntervalItem timeIntervalTable[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];
	YK_ChannelItem channelTable[NUM_CHANNEL];
	YK_SchemeItem schemeTable[NUM_SCHEME];
	YK_WholeConfig wholeConfig;
} YK_Config;

#endif
