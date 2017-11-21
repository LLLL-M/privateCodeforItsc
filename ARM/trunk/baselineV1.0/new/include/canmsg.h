#ifndef _CAN_INTERFACE_H
#define _CAN_INTERFACE_H

#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "libsocketcan.h"
//视频车检添加
#include<sys/un.h>
#include<unistd.h>
#include<stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hik.h"
     

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

//灯控板
void i_can_its_init();
void i_can_its_send_led_request(int boardNum, unsigned short *poutLamp);
void i_can_its_get_Volt(int boardNum, unsigned short *pboardInfo);
unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen);
unsigned short getPedONStateFromCanLib();
//车检板
unsigned short recv_date_from_vechile(int boardNum);
//内部接口
void canits_set_bitrate(const char *name, unsigned long bitrate);
void canits_start(const char *name);
void canits_create_recv_thread();
void canits_create_socket_can_send();
void canits_init();
void canits_send(struct can_frame *pcanfram);
//接收CAN消息线程
void *canits_recv_thread(void *p); 
//视频车检器对接
void video_detece_main_thread_create();
int  video_detect_interface(void);
void recv_from_camera(int *client_fd);

#pragma pack(push, 1)

#define NAME_LEN				32				/* 用户名长度 */
#define MAX_TPS_RULE            8				/*最大参数规则数目*/
#define SERIALNO_LEN            48				/* 序列号长度 */
#define LOW_4bits               0               /*低四位*/
#define TALL_4bits              4               /*高四位*/
#define BUFFLEN                 2048            /*接收buff长度*/
#define IsNeedSend              1               /*需要发送数据*/      
#define NoNeedSend              0               /*不需要发送*/
typedef struct
{		/* 24 bytes */
	struct in_addr	v4;							//< IPv4地址
	struct in6_addr	v6;							//< IPv6地址
	unsigned char	res[4];
}U_IN_ADDR;

typedef struct 
{
	DWORD dwLength;   						/*报文总长度*/
	BYTE  byRes1[2]; 						/* 保留 */
	BYTE  byCommand;						/* 请求命令 0x3a*/
	BYTE  byIPType;		 				    /*IP类型 0-IPV4 1-IPV6 */
	DWORD dwVersion; 						/* 设备版本信息 */
	BYTE  sDVRName[NAME_LEN];     			/* 设备名称 */
	BYTE  sSerialNumber[SERIALNO_LEN]; 		/* 设备序列号 */
	U_IN_ADDR  struIPAddr;              	/*设备IP地址*/ 
	WORD wPort; 							/* 设备端口号 */
	BYTE byMacAddr[6];	 					/* 设备MAC地址*/
	BYTE byRes3[8]; 						/* 保留 */
}INTER_DVR_REQUEST_HEAD_V30;

typedef struct _TPS_NET_VCA_POINT_
{	
 	UINT16 wX;								/*(0.000-1)*1000 X轴坐标 */
 	UINT16 wY;								/*(0.000-1)*1000 Y轴坐标 */
} TPS_NET_VCA_POINT;

typedef struct _NET_LANE_QUEUE_
{
    TPS_NET_VCA_POINT   struHead;	        /*队列头*/
    TPS_NET_VCA_POINT   struTail;		    /*队列尾*/
    UINT32              dwlength;		    /*实际队列长度 单位为米 浮点数*1000*/
}NET_LANE_QUEUE; 

typedef enum tagTRAFFIC_DATA_VARY_TYPE
{    
     ENUM_TRAFFIC_VARY_NO             = 0x00,   //无变化
     ENUM_TRAFFIC_VARY_VEHICLE_ENTER  = 0x01,   //车辆进入虚拟线圈
     ENUM_TRAFFIC_VARY_VEHICLE_LEAVE  = 0x02,   //车辆离开虚拟线圈
     ENUM_TRAFFIC_VARY_QUEUE          = 0x04,   //队列变化
     ENUM_TRAFFIC_VARY_STATISTIC      = 0x08,   //统计数据变化（每分钟变化一次包括平均速度，车道空间/时间占有率，交通状态）        
} TRAFFIC_DATA_VARY_TYPE;  

typedef struct _NET_LANE_PARAM
{
    UINT8  byRuleName[NAME_LEN];  		    /*车道规则名称 */
    UINT8  byRuleID;                 		/*规则序号，为规则配置结构下标，0-7 */
	UINT8  byLaneType;			      		/*车道下行或下行*/
	UINT8  byTrafficState;					/*车道交通状态*/
	UINT8  byRes1;				        	/* 保留*/
	UINT32 dwVaryType;					    /* 车道交通参数变化类型 参照   TRAFFIC_DATA_VARY_TYPE */
	UINT32 dwTpsType;					    /* 数据变化类型标志，表示当前上传的统计参数中，哪些数据有效，其值为ITS_TPS_TYPE的任意组合*/
    UINT32 dwLaneVolume;	                /* 车道流量 ，统计有多少车子通过*/
    UINT32 dwLaneVelocity;         			/*车道速度，公里计算	浮点数*1000*/
   	UINT32 dwTimeHeadway ;       			/*车头时距，以秒计算	浮点数*1000*/
    UINT32 dwSpaceHeadway;       		    /*车头间距，以米来计算浮点数*1000*/
    UINT32 dwSpaceOccupyRation;   		    /*车道占有率，百分比计算（空间上) 浮点数*1000*/
	UINT32 dwTimeOccupyRation;          	/*时间占有率*/
	UINT32 dwLightVehicle;       			/* 小型车数量*/
	UINT32 dwMidVehicle;        			/* 中型车数量*/
	UINT32 dwHeavyVehicle;      			/* 重型车数量*/
    NET_LANE_QUEUE struLaneQueue;       	/*车道队列长度*/
    TPS_NET_VCA_POINT  struRuleLocation;	/*规则位置*/
    UINT8    byRes2[64];
}NET_LANE_PARAM;

//交通统计参数信息结构体
typedef struct _NET_TPS_INFO_
{
    UINT32   dwLanNum;   				    /* 交通参数的车道数目*/
    NET_LANE_PARAM struLaneParam[MAX_TPS_RULE];
	UINT8     byRes[32];    				/*保留*/
}NET_TPS_INFO;

typedef struct _NET_VCA_DEV_INFO_
{
	U_IN_ADDR  struDevIP;				/*前端设备地址*/ 
	UINT16 wPort; 						/*前端设备端口号*/ 
	UINT8 byChannel;  					/*前端设备通道*/
	UINT8 byIvmsChannel;				/*作为IVMS通道号*/ 
} NET_VCA_DEV_INFO;

typedef struct _NET_TPS_ALARM_
{
    UINT32 dwSize;						/* 结构体大小*/
    UINT32 dwRelativeTime;				/* 相对时标*/
    UINT32 dwAbsTime;					/* 绝对时标*/
    NET_VCA_DEV_INFO struDevInfo;		/* 前端设备信息*/
    NET_TPS_INFO  struTPSInfo;			/* 交通事件信息*/
    //UINT8  byRes1[128];				/* 保留字节*/
    UINT32 dwDeviceId; /*设备ID*/ 
    UINT8 byRes1[124]; /* 保留字节*/ 
}NET_TPS_ALARM;

/*************************Hikvision 交通流量通讯协议V2.0************************

**************************Add by lxp 2014-12-26**************************/

#define COMM_ALARM_TPS_REAL_TIME 0xb6  //交通参数统计报警实时信息
#define COMM_ALARM_TPS_INTER_TIME 0xb7 //交通参数统计报警时间段信息

typedef struct
{
    WORD wYear;                 //年，eg：2014
    BYTE byMonth;               //月，1~12
    BYTE byDay;                 //日，1~31
    BYTE byHour;                //时，0~23
    BYTE byMinute;              //分，0~59
    BYTE bySecond;              //秒，0~59
    BYTE byRes;
    WORD wMilliSec;             //毫秒，0~999
    UINT8 byRes1[2];
}INTER_TPS_TIME;

typedef struct _INTER_STRUCTHEAD
{
    UINT16 wLength;             /*结构长度*/
    UINT8 byVersion;            /*高低4位分别代表高低版本，后续根据版本和长度进行扩展，
                                不同版本的长度进行限制，第一个版本默认使用的是0.1版本*/
    UINT8 byRes;
}INTER_STRUCTHEAD, *LPINTER_STRUCTHEAD;

//实时信息
typedef struct tagNET_DVR_TPS_REAL_TIME_INFO
{
    BYTE byStart;               //开始码
    BYTE byCMD;                 //命令号，01-进入指令，02-离开指令，03-拥堵 
                                //状态指令(为03时，只有byLaneState和byQueueLen有效)
    BYTE byRes[2];              //预留字节
    WORD wDeviceID;             //设备ID
    WORD wDataLen;              //数据长度
    BYTE byLane;                //对应车道号
    BYTE bySpeed;               //对应车速
    BYTE byLaneState;           //车道状态；0-无状态，1-畅通，2-拥挤，3-堵塞
    BYTE byQueueLen;            //堵塞状态下排队长度
    BYTE byRes1[24];            //保留
}NET_DVR_TPS_REAL_TIME_INFO, *LPNET_DVR_TPS_REAL_TIME_INFO;

typedef struct tagNET_DVR_TPS_REAL_TIME_ALARM
{
    INTER_STRUCTHEAD struVerHead;                   //版本头
    DWORD dwSize;                                   //通道号
    INTER_TPS_TIME struTime;                        //检测时间
    NET_DVR_TPS_REAL_TIME_INFO struTPSRealTimeInfo; //交通参数统计信息
    BYTE byRes[24];                                 //保留
}NET_DVR_TPS_REAL_TIME_ALARM, *LPNET_DVR_TPS_REAL_TIME_ALARM;

//时段信息
#define TPS_MAX_LANE_NUM 8          //最大统计规则数据
typedef struct tagNET_DVR_LANE_PARAM
{
    BYTE byLane;                    //对应车道号
    BYTE bySpeed;                   //车道过车平均速度
    BYTE byRes[2];                  //保留
    DWORD dwLaneVolume;             //车道流量，统计有多少车子通过
    DWORD dwTimeHeadway ;           //车头时距，以秒计算
    DWORD dwSpaceHeadway;           //车头间距，以米来计算
    WORD wSpaceOccupyRation;        //空间占有率，百分比计算,浮点数*1000，如341，表示34.1%
    WORD wTimeOccupyRation;         //时间占有率，百分比计算,浮点数*1000，如341，表示34.1%
    BYTE byRes1[8];                 //保留
}NET_DVR_LANE_PARAM, *LPNET_DVR_LANE_PARAM;

typedef struct tagNET_DVR_TPS_INTER_TIME_INFO
{
    BYTE byStart;                   //开始码
    BYTE byCMD;                     //命令号， 08-定时成组数据指令
    BYTE byRes[2];                  //预留字节
    WORD wDeviceID;                 //设备ID
    WORD wDataLen;                  //数据长度
    BYTE byTotalLaneNum;            //有效车道总数
    BYTE byRes1[15];                //预留字节
    INTER_TPS_TIME struStartTime;   //统计开始时间
    DWORD dwSamplePeriod;           //统计时间,单位秒
    NET_DVR_LANE_PARAM struLaneParam[TPS_MAX_LANE_NUM];
}NET_DVR_TPS_INTER_TIME_INFO, *LPNET_DVR_TPS_INTER_TIME_INFO;

typedef struct tagNET_DVR_TPS_INTER_TIME_ALARM
{
    INTER_STRUCTHEAD struVerHead;                       //版本头
    DWORD dwChan;                                       //通道号
    NET_DVR_TPS_INTER_TIME_INFO struTPSInterTimeInfo;   //交通参数统计信息
    BYTE byRes[128];                                    //保留
}NET_DVR_TPS_INTER_TIME_ALARM, *LPNET_DVR_TPS_INTER_TIME_ALARM;

/************************************************************************/

//IS_NEED_DATA结构体内为信号机需要的信息
typedef struct _IS_NEED_DATA_
{
    UINT32 varyType;					/* 过车类型*/
    UINT32 laneSpeed;					/*车道速度*/
    UINT8  ruleID;						/* 车道编号*/
    UINT32 cameraID;   					/*设备ID*/ 

}IS_NEED_DATA;

//这里是国标测试中需要用的模拟车检器结构体
//#define GB_TEST_DETECTOR
#ifdef GB_TEST_DETECTOR
typedef struct
{
	UINT8 nDetectorId;
	UINT8 nTotalFlow;
	UINT8 nLargeFlow;
	UINT8 nSmallFlow;
	UINT8 nPercent;
	UINT8 nSpeed;
	UINT8 nLength;
}STRU_DETECTOR_DATA;

typedef struct
{
	UINT8 nDetectorId;

	UINT8 noLive:1;
	UINT8 liveLong:1;
	UINT8 unStable:1;
	UINT8 commErr:1;
	UINT8 cfgErr:1;
	UINT8 :2;
	UINT8 unKown:1;

	UINT8 nOther:1;
	UINT8 nWatchdog:1;
	UINT8 nOpen:1;
	UINT8 nLow:1;
	UINT8 nHigh:1;
	UINT8 :3;
}STRU_DETECTOR_STATUS;
#endif


#pragma pack(pop)

//函数声明
void  deal_data_from_video_v10(NET_TPS_ALARM  *alarmData);
void  deal_data_from_video_v20(NET_DVR_TPS_REAL_TIME_ALARM *realTimeData);
void data_send_by_can(char iProtocol,int iNum,UINT32 iCamId);
#ifdef __cplusplus
}
#endif
 
#endif
