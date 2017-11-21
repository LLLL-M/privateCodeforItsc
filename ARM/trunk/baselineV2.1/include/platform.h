#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"
#include "HikConfig.h"
	   
#pragma pack(push, 4)
//倒计时接口获取命令：
typedef struct STRU_Extra_Param_Get_Phase_Counting_Down
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0x9E表示获取倒计时接口
}GET_PHASE_COUNTING_DOWN_PARAMS;    


//倒计时接口获取反馈：
typedef struct STRU_Extra_Param_Phase_Counting_Down_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0x9E表示上载
     unsigned short    stVehPhaseCountingDown[16][2];    //16个机动车相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     							//第1列表示当前机动车相位所属相位倒计时时间
     unsigned short    stPedPhaseCountingDown[16][2];    //16个行人相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
								//第1列表示当前行人相位所属相位倒计时时间
     unsigned char    ucPlanNo;				//当前运行方案号
     unsigned short   ucCurCycleTime;		//当前运行周期长	!!!!!!!!!!
     unsigned short   ucCurRunningTime;		//当前运行时间		!!!!!!!!!!
     unsigned char    ucChannelLockStatus;		//通道是否被锁定状态，1表示通道锁定，0表示通道未锁定
     unsigned char    ucCurPlanDsc[16];			//当前运行方案描述
     unsigned short    ucOverlap[16][2];                    //跟随相位状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     unsigned short    stPhaseRunningInfo[16][2];	//各相位运行时间和绿信比
							//第0列表示该相位绿信比；第1列表示该相位运行时间，绿灯亮起后第1列才有数值，否则为0
     unsigned char    ucChannelStatus[32]; /*32个通道当前状态
											0:不起效
											1:绿灯
											2:红灯
											3:黄灯
											4:绿闪
											5:黄闪
											6:全红
											7:灭灯
											8:红闪
											9:红黄
											*/
	 unsigned short	  ucChannelCountdown[32];	//实际的通道倒计时			++++++
	 unsigned char	  ucIsStep;			  //是否步进，0:不步进,1:正在步进	++++++
	 unsigned char	  ucMaxStageNum;	  //最大阶段号						++++++	
}PHASE_COUNTING_DOWN_FEEDBACK_PARAMS;


//通道锁定命令：
typedef struct STRU_Extra_Param_Channel_Lock
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xb9表示开启通道锁定
     unsigned char    ucChannelStatus[32];		//32个通道状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪, 6：全红， 7：当前通道不起效，						
     unsigned char    ucWorkingTimeFlag;		//时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
     unsigned char    ucBeginTimeHour;			//控制起效时间：小时
     unsigned char    ucBeginTimeMin;			//控制起效时间：分钟
     unsigned char    ucBeginTimeSec;			//控制起效时间：秒
     unsigned char    ucEndTimeHour;			//控制结束时间：小时
     unsigned char    ucEndTimeMin;			//控制结束时间：分钟
     unsigned char    ucEndTimeSec;			//控制结束时间：秒
     unsigned char    ucReserved;			//预留
}CHANNEL_LOCK_PARAMS;    

typedef struct STRU_Extra_Param_Channel_Lock DemotionParams;
//通道锁定反馈：
typedef struct STRU_Extra_Param_Channel_Lock_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xb9表示开启通道锁定
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}CHANNEL_LOCK_FEEDBACK_PARAMS;    


//通道解锁命令：
typedef struct STRU_Extra_Param_Channel_Unlock
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xba表示关闭通道锁定
}CHANNEL_UNLOCK_PARAMS;    

//通道解锁反馈：
typedef struct STRU_Extra_Param_Channel_Unlock_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xba表示关闭通道锁定
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}CHANNEL_UNLOCK_FEEDBACK_PARAMS;    


//信号机特殊控制：
typedef struct STRU_Extra_Param_Special_Ctrl
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbb表示手动特殊控制命令
     unsigned int     unSpecialCtrlNo;			//特殊控制方式，0：系统控制，255：黄闪，254：感应，252：全红，251：关灯，其它：手动方案
}SPECIAL_CTRL_PARAMS;    

//信号机特殊控制反馈：
typedef struct STRU_Extra_Param_Special_Ctrl_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbb表示手动特殊控制命令
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}SPECIAL_CTRL_FEEDBACK_PARAMS;    



//信号机步进控制（SDK收到平台或配置工具步进控制时，收到步进成功反馈后给第三方库发送关灯指令）：
typedef struct STRU_Extra_Param_Step_Ctrl
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbc表示步进控制命令
     unsigned int     unStepNo;				//步进阶段号，跳转步进有效范围值为1-16，表示锁定在1-16阶段；0为单步步进
}STEP_CTRL_PARAMS;    

//信号机步进控制反馈：
typedef struct STRU_Extra_Param_Step_Ctrl_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbc表示步进控制命令
     unsigned int     unStepNo;				//1表示步进成功，0表示步进失败
}STEP_CTRL_FEEDBACK_PARAMS;

//信号机取消步进（SDK收到平台或配置工具取消步进控制时，收到取消步进成功反馈后给第三方库系统控制指令）：：
typedef struct STRU_Extra_Param_Cancel_Step_Ctrl
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbd表示取消步进控制命令
}CANCEL_STEP_CTRL_PARAMS;    

//信号机取消步进反馈：
typedef struct STRU_Extra_Param_Cancel_Step_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbd表示取消步进控制命令
     unsigned int     unValue;				//1表示取消步进成功，0表示取消步进失败
}CANCEL_STEP_FEEDBACK_PARAMS;   


//经济型信号机型号设置（用于配置工具和平台配置对经济型信号机的型号进行设置，仅在经济型信号机上配置时起效）：
typedef struct STRU_Extra_Param_Set_Device_Type
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbe表示设置设备型号
     unsigned int     unResult;				//设置经济型信号机类型,1:DS-TSC300-44,2:DS-TSC300-22
}SET_DEVICE_TYPE_PARAMS;  


typedef struct STRU_Extra_Param_Set_Device_Type_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbe表示设置设备型号
     unsigned int     unResult;				//返回值,0:设置失败    1：设置成功
}SET_DEVICE_TYPE_FEEDBACK_PARAMS;  

//获取经济型信号机型号（用于配置工具和平台获取经济型信号机的型号，仅在经济型信号机上获取时才起效）：
typedef struct STRU_Extra_Param_Get_Device_Type
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbf表示获取设备型号
}GET_DEVICE_TYPE_PARAMS;  


typedef struct STRU_Extra_Param_Get_Device_Type_Feedback
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xbf表示获取设备型号
     unsigned int     unResult;				//返回经济型信号机类型,0：获取失败，1:DS-TSC300-44,2:DS-TSC300-22
}GET_DEVICE_TYPE_FEEDBACK_PARAMS; 

typedef struct STRU_Extra_Param_Response
{
	UInt32	unExtraParamHead;		//消息头标志
	UInt32	unExtraParamID;			//消息类型ID
	UInt32 	unExtraParamValue;		//块数据类型
	UInt32 	unExtraParamFirst;		//起始行
	UInt32 	unExtraParamTotal;		//总行数
	UInt32  unExtraDataLen;			//数据总长度
	UInt8	data[0];				//上载参数的数据结构体
} MsgFaultInfo;	//上载参数回复的结构体

/***************新增状态消息：（注：下发的状态消息都不带消息体）
***************************************/
//消息类型：	0x71 -- 下载对时，0x72 -- 上载对时
//消息体：
struct SAdjustTime
{
	unsigned long			ulGlobalTime;		// UTC时间
	unsigned int			unTimeZone;			//时差
};


//消息类型：0x159 -- 获取通道状态
//消息体：STRU_N_ChannelStatusGroup AscChannelStatusTable[4];
//通道状态组定义
struct STRU_N_ChannelStatusGroup
{
	/*2.9.4.1 channelStatusGroup表的行号，不能大于
maxChannelStatusGroups。(1..4)*/
	UInt8  byChannelStatusGroupNumber;

	/*2.9.4.2通道的红灯输出状态(bit=1表示活动，bit=0表示不活动)	Bit
7: 通道号 = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: 通道号 = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupReds;	
	/*2.9.4.3通道的黄灯输出状态(bit=1表示活动，bit=0表示不活动)	Bit
7: 通道号 = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: 通道号 = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupYellows;

	/*2.9.4.4通道的绿灯输出状态(bit=1表示活动，bit=0表示不活动)	Bit
7: 通道号 = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: 通道号 = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupGreens;
};

//消息类型：0x157 -- 获取相位状态
//消息体：STRU_N_PhaseStatusGroup AscPhaseStatusTable[2]; 
struct STRU_N_PhaseStatusGroup {
	UInt8	byPhaseStatusGroupNumber;/*2.2.4.1相位状态组号，不能超过最大
相位组值*/
	UInt8	byPhaseStatusGroupReds;	/*2.2.4.2对于每个组内8个相位的红灯状
态，置1为红活动*/
	UInt8	byPhaseStatusGroupYellows;/*2.2.4.3对于每个组内8个相位的黄灯
状态，置1为黄活动*/
	UInt8	byPhaseStatusGroupGreens;/*2.2.4.4对于每个组内8个相位的绿灯
状态，置1为绿活动*/
	UInt8	byPhaseStatusGroupDontWalks;/*2.2.4.5禁止行人状态，置1为行人
不放行*/
	UInt8	byPhaseStatusGroupPedClears; /*2.2.4.6行人清空状态，置1为行
人清空放行*/
	UInt8	byPhaseStatusGroupWalks;/*2.2.4.7行人状态，置1为行人放行*/
	UInt8	byPhaseStatusGroupVehCalls;/*2.2.4.8机动车请求状态，置1为该
相位有机动车请求*/
	UInt8	byPhaseStatusGroupPedCalls;/*2.2.4.9行人请求状态，置1为该相
位有行人请求*/
	UInt8	byPhaseStatusGroupPhaseOns;/*2.2.4.10相位活动状态，置1为该相
位处于活动状态*/
	UInt8	byPhaseStatusGroupPhaseNexts;/*2.2.4.11下一个相位的状态*/
};

//消息类型：0x2B -- 获取跟随相位状态
//消息体：STRU_N_OverlapStatusGroup AscOverlapStatusTable[2];
//重叠状态组定义
struct STRU_N_OverlapStatusGroup
{
	UInt8 byOverlapStatusGroupNumber;	/*2.10.4.1 Status Group号，
不能超过maxOverlapStatusGroups。1..2*/
	UInt8 byOverlapStatusGroupReds;		/*2.10.4.2  overlap红灯输出
状态标志。*/
	UInt8 byOverlapStatusGroupYellows;	/*2.10.4.3 overlap黄灯输出状
态标志。*/
	UInt8 byOverlapStatusGroupGreens;	/*2.10.4.4 overlap绿灯输出状
态标志。*/
};

//消息类型：0x15A -- 获取同步状态
//消息体：
struct STRU_CoordinateStatus
{
	UInt8	byCoordPatternStatus;		/*2.5.10 协调方案状态
				Not used（0）
				Pattern -（1-253）当前运行方案。
				Free - (254)感应
				Flash - (255)闪光*/
	UInt16	wCoordCycleStatus;		/* 2.5.12 协调方案的周期状态
（0-510sec），从周期长一直减少到0*/
	UInt16 	wCoordSyncStatus;			/*2.5.13协调同步状态
（0－510）从协调基准点到目前周期的时间，从0记录到下个周期基准点。可以超过周
期长*/

	UInt8	byUnitControlStatus;			/*2.4.5当前的信号机
控制状态
				Other(1): 未知模式。
				systemControl(2): 系统协调控制。
				systemStandby(3): 主机协调控制。
				backupMode(4):背光模式
				manual(5): 手动面板控制。
				Timebase(6): 时段表控制（按本地时段表运
行）。
				Interconnect(7): 有缆协调。
				interconnectBackup(8):无缆协调*/
};

//流量占有率定义
typedef struct STRU_N_VolumeOccupancy
{
    /*提供两位小数点精度支持，实际值乘以100后进行存储，给用户显示时，需要除以100才能是实际值*/
	BYTE    byDetectorVolume;			/*2.3.5.4.1一个检测器在一个周期中采集的流量数,单位是 辆/周期*/
	WORD    byDetectorOccupancy;		/*2.3.5.4.2一个检测器在一个周期中的占有率 这里我们提供的是时间占有率，除以10000，得到百分比值*/
	WORD	byVehicleSpeed;		        /*车速,按照*100存储的，所以实际值应该除以100后，单位才是km/h*/
	WORD	wQueueLengh;		        /*排队长度,按照*100存储，除以100后，才能得到实际的车辆数，单位是辆*/
	WORD    wGreenLost;                 /*绿损 损失绿灯时长/绿灯总时长 ，该值除以100即是*/
	WORD    wVehicleDensity;            /*车流密度，按照*100存储，除以100后单位是 辆/km*/
	WORD    wVehicleHeadDistance;       /*车头间距，同一车道、同一方向连续行驶前后相邻两辆车之间在某一瞬时的车头间距。符号：hd 单位：m/辆。*/
    WORD    wVehicleHeadTimeDistance;   /*车头时距：同一车道、同一方向连续行驶前后相邻的两辆车的车头通过某一点的时间间隔。符号：ht 单位：s/辆 */
} VolumeOccupancy;	

//历史流量与时间对应结构
typedef struct STRU_TimeAndHistoryVolume
{
    DWORD dwTime;                       //历史时间点
    VolumeOccupancy  struVolume[48]; //对应历史流量
} TimeAndHistoryVolume;

typedef struct
{
	UINT8 greenBlink;	//绿闪时间
	UINT8 yellow;		//黄灯时间
	UINT8 allred;		//全红时间
} PhaseTime;
//周期方案数据,该数据在每个周期的开始进行更新,包含信号机当前运行方案的所有信息。
typedef struct
{
	UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd0
	
	UINT8 nPatternId;						//当前运行方案号，以下ID均从从1开始，依次是4,7，不做转换。
	UINT8 nSplitId;							//当前运行绿信比表号
	UINT8 nPhaseTurnId;						//当前运行相序表号
	UINT16 nOffset;							//当前方案相位差	!!!!!!!!!!!!!!!!!!!!!
	PhaseTurnItem sPhaseTurn[4];			//当前运行的四个环的相序,下标0代表环1，依次类推
	UINT32 nCoordinatePhase;				//当前方案使用的协调相位,bit0代表相位1，该值为0表示不做协调相位，为1表示该相位作为协调相位。
	PhaseTime phaseTime[16];
	FollowPhaseItem stFollowPhase[16];      //跟随相位										
	UINT8 ucIsCurrentCycle;		//是否是当前周期,0:不是,1:是	++++++++++++++
	char phaseDesc[16][64];	//当前使用相位表的各相位描述
    UINT8 nPhaseTableId;
    UINT8 nChannelTableId;	
} MsgRealtimePattern;//方案数据

typedef struct
{
	UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd2
	UINT32 unExtraDataLen;					//剩余的数据长度
	UINT8 nPhaseArray[16];					//信号机配置的所有相位的相位号
	UINT8 nPatternArray[108];				//信号机配置的所有方案号，不做转换	
} MsgPhaseSchemeId;//供平台1049协议使用

//实时流量信息通过下面的结构体获取，历史流量信息通过sftp获取指定的文件即可,该文件的二进制格式是TimeAndHistoryVolume。
typedef struct
{
	UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd1
	
	TimeAndHistoryVolume volumeOccupancy;		//流量占有率等结构体	
} MsgRealtimeVolume;

//给触摸屏发送的通道倒计时信息
typedef struct {
    UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd3
    
    UINT8 nChannelStatus[32][2];            //32个通道的倒计时值和颜色状态    
                //第0列表示当前通道的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
                //第1列表示当前通道所属倒计时时间
    UINT8 nReserved[256];				//预留字节
}MsgChannelCountDown;

#define MAX_CHAN_LOCK_PERIODS	16
typedef struct 
{
	unsigned char	 ucChannelStatus[32];	   //32个通道状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪, 6：全红， 7：当前通道不起效， 					   
	unsigned char	 ucWorkingTimeFlag; 	   //时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
	unsigned char	 ucBeginTimeHour;		   //控制起效时间：小时
	unsigned char	 ucBeginTimeMin;		   //控制起效时间：分钟
	unsigned char	 ucBeginTimeSec;		   //控制起效时间：秒
	unsigned char	 ucEndTimeHour; 		   //控制结束时间：小时
	unsigned char	 ucEndTimeMin;		   //控制结束时间：分钟
	unsigned char	 ucEndTimeSec;		   //控制结束时间：秒
	unsigned char	 ucReserved;		   //预留
}STRU_CHAN_LOCK_PARAMS;

typedef struct 
{
	UINT8					cLockFlag;	
	STRU_CHAN_LOCK_PARAMS	chans[MAX_CHAN_LOCK_PERIODS];
}STU_MUL_PERIODS_CHAN_PARAMS;

/*此协议专门用来提供给SDK获取信号机的类型和所使用的通信端口，
 * 此协议只能通过udp 20000端口获取，所以以后的信号机主控程序都要保留20000端口专门用来实现此协议*/
struct STRU_N_PatternNew	//类型为223,即0xdf
{
	UINT16	wProtocol;		//信号机协议类型，1-NTCIP，2-HIK，3-GB
	UINT16	wDscType;		//信号机型号，0:500型，1:300-44型，2:300-22型
	UINT32	unPort;			//通讯端口
};


#pragma pack(pop)

#endif

