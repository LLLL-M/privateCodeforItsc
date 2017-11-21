#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"

#include "common.h"
#include "HikConfig.h"

#define MSG_RED_SIGNAL_CHECK	111

#define MSGSIZE (sizeof(struct msgbuf) - sizeof(long))

typedef enum
{
	STEP_START = 0,	//步进开始
	STEP_EXCUTE,	//步进执行
	STEP_END,		//步进结束
} StepCmd;

struct msgbuf
{
	long mtype;
	StepCmd cmd;
	UInt8 stageNo;
	UInt16 lightArray[8];
};
/************* 新通道锁定相关命令定义**********************/
typedef struct Channel_Lock_Period
{
	unsigned char	 ucChannelStatus[32];	   //32个通道状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪 ,7:不起效（不对该通道控制）						   
	unsigned char	 ucWorkingTimeFlag; 	   //时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
	unsigned char	 ucBeginTimeHour;		   //控制起效时间：小时
	unsigned char	 ucBeginTimeMin;		   //控制起效时间：分钟
	unsigned char	 ucBeginTimeSec;		   //控制起效时间：秒
	unsigned char	 ucEndTimeHour; 		   //控制结束时间：小时
	unsigned char	 ucEndTimeMin;		   //控制结束时间：分钟
	unsigned char	 ucEndTimeSec;		   //控制结束时间：秒
	unsigned char	 ucReserved;		   //预留

}CHANNEL_LOCK_PERIOD_PARAMS;

//新通道锁定命令：
typedef struct STRU_Extra_Param_Channel_Lock_V2
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xD3表示开启通道锁定
     CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriod[16];  //时间段设置
     unsigned char    ucReserved[4];			//预留
}CHANNEL_LOCK_V2_PARAMS; 

//新通道锁定反馈：
typedef struct STRU_Extra_Param_Channel_Lock_Feedback_V2
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xD3表示开启通道锁定
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}CHANNEL_LOCK_FEEDBACK_V2_PARAMS;    


//新通道解锁命令：(和原通道解锁命令是同一个消息)
typedef struct STRU_Extra_Param_Channel_Unlock_V2
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xad表示关闭通道锁定
     unsigned int     unResult;				//1表示新通道锁定解锁，0表示原通道锁定解锁
}CHANNEL_UNLOCK_V2_PARAMS;    

//新通道解锁反馈：(和原通道解锁命令是同一个消息)
typedef struct STRU_Extra_Param_Channel_Unlock_Feedback_V2
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xad表示关闭通道锁定
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}CHANNEL_UNLOCK_FEEDBACK_V2_PARAMS;   

//新通道锁定状态查询命令：
typedef struct STRU_Extra_Param_Channel_Lock_Status_V2
{
     unsigned int     unExtraParamHead;        		//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0xD4表示关闭通道锁定
     unsigned int     unResult;				//返回值，1表示成功，0表示失败
}CHANNEL_LOCK_STATUS_V2_PARAMS;    

//新通道锁定状态查询反馈：
typedef struct STRU_Extra_Param_Channel_Lock_Status_Feedback_V2
{
    unsigned int     unExtraParamHead;        		//标志头,0x6e6e
    unsigned int     unExtraParamID;                	//类型,0xD4表示关闭通道锁定
    CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriod[16];  //时间段设置反馈
	unsigned char	  unLockStatus; 					//锁定状态，1表示通道锁定，0表示通道未锁定，2表示待锁定
	unsigned char	  unReserved[3]; //预留
}CHANNEL_LOCK_STATUS_FEEDBACK_V2_PARAMS;

typedef struct STRU_Channel_Lock_V2_INFO
{
	UINT8	uChannelLockFlag;							//新通道锁定标志，1表示通道锁定，0表示通道未锁定
	UINT8	uChannelLockStatus;							//锁定状态，1表示通道锁定，0表示通道未锁定，2表示待锁定
	CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriods[16];  //时间段设置	
}STRU_Channel_Lock_V2_INFO;

/********************************************************/

typedef enum
{
	COUNTDOWN_CONFIG_NO_NEW = 0,	//没有新的配置信息
	COUNTDOWN_CONFIG_HAVE_NEW,		//有新的配置信息
	COUNTDOWN_CONFIG_UPDATE,		//配置信息更新
} CountdownConfigFlag;

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
     unsigned char    stVehPhaseCountingDown[16][2];    //16个机动车相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     							//第1列表示当前机动车相位所属相位倒计时时间
     unsigned char    stPedPhaseCountingDown[16][2];    //16个行人相位的当前灯色及倒计时
     							//第0列表示当前机动车相位的状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
								//第1列表示当前行人相位所属相位倒计时时间
     unsigned char    ucPlanNo;				//当前运行方案号
     unsigned char    ucCurCycleTime;			//当前运行周期长
     unsigned char    ucCurRunningTime;		//当前运行时间
     unsigned char    ucChannelLockStatus;		//通道是否被锁定状态，1表示通道锁定，0表示通道未锁定
     unsigned char    ucCurPlanDsc[16];			//当前运行方案描述
     unsigned char    ucOverlap[16][2];                    //跟随相位状态，0:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪
     unsigned char    stPhaseRunningInfo[16][2];	//各相位运行时间和绿信比
							//第0列表示该相位绿信比；第1列表示该相位运行时间，绿灯亮起后第1列才有数值，否则为0
	unsigned char    ucChannelStatus[32]; //32个通道状态，7:灭灯， 1:绿灯，2:红灯，3:黄灯，4：绿闪，5：黄闪 ,0:不起效（不对该通道控制） 
     unsigned char    ucWorkingTimeFlag; //时间段控制起效标志，1表示以下时间段控制起效；0表示全部时间内起效，后续时间参数无效
     unsigned char    ucBeginTimeHour; //控制起效时间：小时
     unsigned char    ucBeginTimeMin; //控制起效时间：分钟
     unsigned char    ucBeginTimeSec; //控制起效时间：秒
     unsigned char    ucEndTimeHour; //控制结束时间：小时
     unsigned char    ucEndTimeMin; //控制结束时间：分钟
     unsigned char    ucEndTimeSec; //控制结束时间：秒
     unsigned char    ucReserved[9]; //预留
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



//以下是为解决平台和配置工具冲突而新设计的协议,added by xiaowh


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
	POWER_ON = 0x0A,								//上电
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
	POW_ON_REBOOT = 0x19,							//上电重启
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

//流量占有率定义
typedef struct STRU_N_VolumeOccupancy
{
    /*提供两位小数点精度支持，实际值乘以100后进行存储，给用户显示时，需要除以100才能是实际值*/
	BYTE    byDetectorVolume;			/*2.3.5.4.1一个检测器在一个周期中采集的流量数,单位是 辆/周期*/
	WORD    byDetectorOccupancy;		/*2.3.5.4.2一个检测器在一个周期中的占有率 这里我们提供的是时间占有率，除以100，得到百分比值*/
	WORD	byVehicleSpeed;		        /*车速,按照*100存储的，所以实际值应该除以100后，单位才是km/h*/
	WORD	wQueueLengh;		        /*排队长度,按照*100存储，除以100后，才能得到实际的车辆数，单位是辆*/
	WORD    wGreenLost;                 /*绿损 损失绿灯时长/绿灯总时长 ，该值除以100即是*/
	WORD    wVehicleDensity;            /*车流密度，按照*100存储，除以100后单位是 辆/km*/
	WORD    wVehicleHeadDistance;       /*车头间距，同一车道、同一方向连续行驶前后相邻两辆车之间在某一瞬时的车头间距。符号：hd 单位：m/辆。*/
    WORD    wVehicleHeadTimeDistance;   /*车头时距：同一车道、同一方向连续行驶前后相邻的两辆车的车头通过某一点的时间间隔。符号：ht 单位：s/辆 */
}VolumeOccupancy;	

//历史流量与时间对应结构
typedef struct STRU_TimeAndHistoryVolume
{
    DWORD dwTime;                       //历史时间点
    VolumeOccupancy  struVolume[48];    //对应历史流量
}TimeAndHistoryVolume;

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
	UINT8 nOffset;							//当前方案相位差
	PhaseTurnItem sPhaseTurn[4];			//当前运行的四个环的相序,下标0代表环1，依次类推
	UINT32 nCoordinatePhase;				//当前方案使用的协调相位,bit0代表相位1，该值为0表示不做协调相位，为1表示该相位作为协调相位。
	PhaseTime phaseTime[16];
	FollowPhaseItem stFollowPhase[16];      //跟随相位										
	
	UINT8 nReserved[256];					//预留字节
}MsgRealtimePattern;//方案数据

typedef struct
{
	UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd2
	UINT32 unExtraDataLen;					//剩余的数据长度
	UINT8 nPhaseArray[16];					//信号机配置的所有相位的相位号
	UINT8 nPatternArray[108];				//信号机配置的所有方案号，不做转换	
}MsgPhaseSchemeId;//供平台1049协议使用

//故障信息,其中包含信号机记录的包括信号灯故障、控制器故障在内的所有故障信息。该结构体兼容新库。
typedef struct STRU_Extra_Param_Response
{
	UInt32	unExtraParamHead;				//消息头标志，默认为0x6e6e
	UInt32	unExtraParamID;					//消息类型ID，该值为0xc0
	UInt32 	unExtraParamValue;				//块数据类型，该值为0x15b
	UInt32 	unExtraParamFirst;				//起始行，起始故障号，从1开始
	UInt32 	unExtraParamTotal;				//总行数，总共获取的故障数
	UInt32  unExtraDataLen;					//数据总长度，以下数据段总长度
	STRU_FAILURE_INFO data[0];				//实际的故障具体内容
}MsgFaultInfo;

//实时流量信息通过下面的结构体获取，历史流量信息通过sftp获取指定的文件即可,该文件的二进制格式是TimeAndHistoryVolume。
typedef struct
{
	UINT32 unExtraParamHead;				//消息头，默认为0x6e6e
	UINT32 unExtraParamID;					//消息类型，该信息方案数据的的值为0xd1
	
	TimeAndHistoryVolume volumeOccupancy;		//流量占有率等结构体	
}MsgRealtimeVolume;


#pragma pack(pop)


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
	RED_YELLOW = 9
} PhaseChannelStatus;

typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

/*****************************************************************************
 函 数 名  : put_lamp_value
 功能描述  : 主要用来设置一组灯中某个灯的状态值
 输入参数  : volatile unsigned short *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
             unsigned short value             要设置的灯的状态值
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}
/*****************************************************************************
 函 数 名  : get_lamp_value
 功能描述  : 主要用来获取一组灯中具体某个灯的状态值
 输入参数  : UInt16 *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
 返 回 值  : 返回某个灯的状态值
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline UInt16 get_lamp_value(UInt16 *lights, int n)
{
	lamp_t *p = (lamp_t *)(lights);
	UInt16 value = 0;
	switch (n) 
	{
		case 0:	value = p->L0; break;
		case 1:	value = p->L1; break;
		case 2:	value = p->L2; break;
		case 3:	value = p->L3; break;
		default: break;
	}
	
	return value;
}

#endif
