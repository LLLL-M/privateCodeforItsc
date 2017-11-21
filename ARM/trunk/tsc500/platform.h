#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"
	   
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

struct STRU_Extra_Param_Block
{
	unsigned int	unExtraParamHead;		//消息头标志
	unsigned int	unExtraParamID;			//消息类型ID
	unsigned int 	unExtraParamValue;		//块数据类型
	unsigned int 	unExtraParamFirst;		//起始行
	unsigned int 	unExtraParamTotal;		//总行数
};

#pragma pack(pop)

typedef struct
{
	int sockfd;
	struct sockaddr addr;
} DestAddressInfo;

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
