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

#include "common.h"
     

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

//灯控板
void i_can_its_init();
void i_can_its_send_led_request(int boardNum, unsigned short *poutLamp);
void i_can_its_get_Volt(int boardNum, unsigned short *pboardInfo);
unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen);


//车检板
unsigned short recv_date_from_vechile(int boardNum);

//内部接口
void canits_set_bitrate(const char *name, unsigned long bitrate);
void canits_start(const char *name);
void canits_create_recv_thread();
void canits_create_socket_can_send();
void canits_init();
void canits_send(struct can_frame *pcanfram);
void *canits_recv_thread(void *p);  //接收CAN消息线程


//视频车检器对接

void video_detece_main_thread_create();
int  video_detect_interface(void);
void recv_from_camera(int *client_fd);

#pragma pack(push, 1)

typedef unsigned int   DWORD; // 4, same as long
typedef unsigned short WORD;  // 2
typedef unsigned char  BYTE;  // 1

#define NAME_LEN				32				/* 用户名长度 */
#define MAX_TPS_RULE                  8				/*最大参数规则数目*/
#define SERIALNO_LEN        		48				/* 序列号长度 */
#define LOW_4bits                         0                       /*低四位*/
#define TALL_4bits                         4                       /*高四位*/
#define BuffLen                              2048                  /*接收buff长度*/
#define IsNeedSend                         1                      /*需要发送数据*/      
#define NoNeedSend                        0                      /*不需要发送*/
typedef struct
{		/* 24 bytes */
	struct in_addr	v4;							//< IPv4地址
	struct in6_addr	v6;							//< IPv6地址
	unsigned char	res[4];
}U_IN_ADDR;

typedef struct 
{
	DWORD dwLength;   						/*报文总长度*/
	BYTE    byRes1[2]; 						/* 保留 */
	BYTE    byCommand;						 /* 请求命令 0x3a*/
	BYTE    byIPType;		 				/*IP类型 0-IPV4 1-IPV6 */
	DWORD dwVersion; 						/* 设备版本信息 */
	BYTE   sDVRName[NAME_LEN];     			/* 设备名称 */
	BYTE   sSerialNumber[SERIALNO_LEN]; 		/* 设备序列号 */
	U_IN_ADDR  struIPAddr;              			/*设备IP地址*/ 
	WORD wPort; 							/* 设备端口号 */
	BYTE byMacAddr[6];	 					/* 设备MAC地址*/
	BYTE byRes3[8]; 							/* 保留 */
}INTER_DVR_REQUEST_HEAD_V30;

typedef struct _TPS_NET_VCA_POINT_
{	
 	UINT16 wX;								/*(0.000-1)*1000 X轴坐标 */
 	UINT16 wY;								/*(0.000-1)*1000 Y轴坐标 */
} TPS_NET_VCA_POINT;

typedef struct _NET_LANE_QUEUE_
{
    	TPS_NET_VCA_POINT   struHead;			/*队列头*/
    	TPS_NET_VCA_POINT   struTail;				/*队列尾*/
    	UINT32          dwlength;					/*实际队列长度 单位为米 浮点数*1000*/
}NET_LANE_QUEUE; 

typedef enum tagTRAFFIC_DATA_VARY_TYPE
{    
     ENUM_TRAFFIC_VARY_NO               	= 0x00,   //无变化
     ENUM_TRAFFIC_VARY_VEHICLE_ENTER  = 0x01,   //车辆进入虚拟线圈
     ENUM_TRAFFIC_VARY_VEHICLE_LEAVE  = 0x02,   //车辆离开虚拟线圈
     ENUM_TRAFFIC_VARY_QUEUE           	= 0x04,   //队列变化
     ENUM_TRAFFIC_VARY_STATISTIC        	= 0x08   //统计数据变化（每分钟变化一次包括平均速度，车道空间/时间占有率，交通状态）        
} TRAFFIC_DATA_VARY_TYPE;  

typedef struct _NET_LANE_PARAM
{
    	UINT8  byRuleName[NAME_LEN];  		/*车道规则名称 */
    	UINT8  byRuleID;                 			/*规则序号，为规则配置结构下标，0-7 */
	UINT8  byLaneType;			      		/*车道下行或下行*/
	UINT8  byTrafficState;					/*车道交通状态*/
	UINT8  byRes1;				        	/* 保留*/
	UINT32 dwVaryType;					/* 车道交通参数变化类型 参照   TRAFFIC_DATA_VARY_TYPE */
	UINT32 dwTpsType;					/* 数据变化类型标志，表示当前上传的统计参数中，哪些数据有效，其值为ITS_TPS_TYPE的任意组合*/
    	UINT32 dwLaneVolume;	                   /* 车道流量 ，统计有多少车子通过*/
    	UINT32 dwLaneVelocity;         			/*车道速度，公里计算	浮点数*1000*/
   	UINT32 dwTimeHeadway ;       			/*车头时距，以秒计算	浮点数*1000*/
    	UINT32 dwSpaceHeadway;       		/*车头间距，以米来计算浮点数*1000*/
    	UINT32 dwSpaceOccupyRation;   		/*车道占有率，百分比计算（空间上) 浮点数*1000*/
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
    	UINT32   dwLanNum;   				/* 交通参数的车道数目*/
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
    NET_VCA_DEV_INFO struDevInfo;			/* 前端设备信息*/
    NET_TPS_INFO  struTPSInfo;				/* 交通事件信息*/
    //UINT8  byRes1[128];					/* 保留字节*/
    UINT32 dwDeviceId; /*设备ID*/ 
    UINT8 byRes1[124]; /* 保留字节*/ 
}NET_TPS_ALARM;

//IS_NEED_DATA结构体内为信号机需要的信息
typedef struct _IS_NEED_DATA_
{
    UINT32 varyType;						/* 过车类型*/
    UINT32 laneSpeed;						/*车道速度*/
    UINT8  ruleID;							/* 车道编号*/
    UINT32 cameraID;   					 /*设备ID*/ 

}IS_NEED_DATA;

#pragma pack(pop)

void  deal_data_from_video(NET_TPS_ALARM  alarmData);
#ifdef __cplusplus
}
#endif
 
#endif
