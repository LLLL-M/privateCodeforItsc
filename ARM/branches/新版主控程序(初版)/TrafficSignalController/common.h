#ifndef _COMMON_H_
#define _COMMON_H_

#pragma pack(push, 1)

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long   UINT32;
typedef char 	INT8;
typedef short INT16;
typedef long INT32;

typedef struct SPECIAL_PARAMS
{
	int iErrorDetectSwitch;                         //故障检测开关
	int iCurrentAlarmSwitch;						//电流检测开关，报警不处理
	int iVoltageAlarmSwitch;						//电压检测开关，报警不处理
	int iCurrentAlarmAndProcessSwitch;				//电流报警并处理开关
	int iVoltageAlarmAndProcessSwitch;				//电压报警并处理开关
	int iWatchdogSwitch;							//软件看门狗开关
	int iGpsSwitch;									//GPS开关
}SPECIAL_PARAMS;

typedef struct 
{
    int nIsControlRecord;//这里是控制器故障开关，主要存储的是红绿冲突等
    int nIsLogRecord;//这里是日志开关，主要存储的是开机日志、控制面板操作日志等
}FAULT_CFG;


typedef struct STRU_N_IP_ADDRESS
{
	int address; 					//地址
	int subnetMask; 				//子网掩码
	int gateway;					//网关
}STRU_N_IP_ADDRESS;	

typedef struct UDP_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	int iValue[64];						//
	//unsigned int iReserved;                  //预留
}UDP_INFO;

typedef struct UDP_CUR_VALUE_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	unsigned char redCurrentValue[32];		//32个通道电流值
	//unsigned int iReserved;                  //预留
}UDP_CUR_VALUE_INFO;


typedef struct FAILURE_INFO
{
	int nNumber;    		//序列号
	int nID;                //消息类型ID
	long nTime;        		//发生时间
	int nValue;         	//时间值(通道号)
}FAILURE_INFO;

typedef struct CURRENT_PARAMS
{
int RedCurrentBase;         //基准值
int RedCurrentDiff;         //差值
}CURRENT_PARAMS;

typedef struct CURRENT_PARAMS_UDP
{
	int iHead;                      
	int iType;	
	struct CURRENT_PARAMS struRecCurrent[32];
}CURRENT_PARAMS_UDP;

typedef struct _Count_Down_Veh_
{
	unsigned char veh_phase;                      
	unsigned char veh_color;	
	unsigned char veh_phaseTime;
}CountDownVeh;

typedef struct _Count_Down_Ped_
{
	unsigned char ped_phase;                      
	unsigned char ped_color;	
	unsigned char ped_phaseTime;
}CountDownPed;

typedef struct _Count_Down_Head_
{
	UINT8 head1;
	UINT8 head2;
}CountDownHead;

#pragma pack(pop)


#endif

