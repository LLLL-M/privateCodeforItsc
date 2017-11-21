#ifndef DATA_H
#define DATA_H
//#include <sys/socket.h>
//#include <sys/types.h>
#include <inaddr.h>
#include <in6addr.h>
#include <winsock2.h>
//#include <netinet/in.h>
//#pragma comment(lib, "ws2_32.lib")

#define BYTE unsigned char
#define UINT8 unsigned char
#define UINT16	unsigned short
#define WORD unsigned short
#define DWORD unsigned int


#define CAMERA_PROTOCOL10	1
#define CAMERA_PROTOCOL20	2
#define CAMERA_PROTOCOL21	3

#define NAME_LEN		32
#define SERIALNO_LEN            48

#define MSG_REAL_TIME_ALARM_INFO	0
#define MSG_INTER_ALARM_INFO	1

#define ALARM_REAL_TIME_ENTER	1
#define ALARM_REAL_TIME_LEAVE	2
#define ALARM_REAL_TIME_CONGESTION	3
#define ALARM_STATISTICS	4

/**************************camera protocol 2.0 definition ******************************/
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
    char  sDVRName[NAME_LEN];     			/* 设备名称 */
    BYTE  sSerialNumber[SERIALNO_LEN]; 		/* 设备序列号 */
    U_IN_ADDR  struIPAddr;              	/*设备IP地址*/
    WORD wPort; 							/* 设备端口号 */
    BYTE byMacAddr[6];	 					/* 设备MAC地址*/
    BYTE byRes3[8]; 						/* 保留 */
}INTER_DVR_REQUEST_HEAD_V30;
/********************************************/
typedef struct _INTER_STRUCTHEAD
{
    UINT16 wLength;             /*结构长度*/
    UINT8 byVersion;            /*高低4位分别代表高低版本，后续根据版本和长度进行扩展，
                                不同版本的长度进行限制，第一个版本默认使用的是0.1版本*/
    UINT8 byRes;
}INTER_STRUCTHEAD;

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
/***********************************************/
#define TPS_MAX_LANE_NUM 8
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
}NET_DVR_LANE_PARAM;

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
}NET_DVR_TPS_INTER_TIME_INFO;

typedef struct tagNET_DVR_TPS_INTER_TIME_ALARM
{
    INTER_STRUCTHEAD struVerHead;                       //版本头
    DWORD dwChan;                                       //通道号
    NET_DVR_TPS_INTER_TIME_INFO struTPSInterTimeInfo;   //交通参数统计信息
    BYTE byRes[128];                                    //保留
}NET_DVR_TPS_INTER_TIME_ALARM;
/***********************************************/
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
}NET_DVR_TPS_REAL_TIME_INFO;
typedef struct tagNET_DVR_TPS_REAL_TIME_ALARM
{
    INTER_STRUCTHEAD struVerHead;                   //版本头
    DWORD dwSize;                                   //通道号
    INTER_TPS_TIME struTime;                        //检测时间
    NET_DVR_TPS_REAL_TIME_INFO struTPSRealTimeInfo; //交通参数统计信息
    BYTE byRes[24];                                 //保留
}NET_DVR_TPS_REAL_TIME_ALARM;

typedef struct _TPS_REAL_TIME_INFO20
{
    BYTE byLane;
    WORD wDeviceID;
}TPS_REAL_TIME_INFO20;
typedef struct _TPS_INTER_TIME_INFO20
{
   WORD wDeviceID;
   WORD wSamplePeriod;
}TPS_INTER_TIME_INFO20;

/******************************end of P2.0************************************/

/*************************camera protocal 2.1 definition**********************/
typedef struct _NET_TPS_ALARM_HEAD_	/*140bytes*/
{
    DWORD	dwLength;	//报文总长度:sizeof(NET_TPS_ALARM_REALTIME)
                        //或者sizeof(NET_TPS_ALARM_INTER_TIME)
                        //和byType有关，注意:此字段未使用网络字节序
    BYTE	byRes[2];	//保留
    BYTE	byType;		//消息类型: 0xB6实时数据，0xB7统计数据
    BYTE	byRes1[133];	//保留数据
}NET_TPS_ALARM_HEAD;

typedef NET_DVR_TPS_REAL_TIME_INFO NET_TPS_REALTIME_INFO;
typedef struct _NET_TPS_ALARM_REALTIME_
{
    NET_TPS_ALARM_HEAD	struHead;	//信息头
    INTER_TPS_TIME		struTime;	//检测时间
    NET_TPS_REALTIME_INFO	struTPSRealTimeInfo;	//交通参数实时信息
    BYTE				byRes[24];	//保留
}NET_TPS_ALARM_REALTIME;

typedef struct _NET_TPS_LANE_PARAM_
{
    BYTE	byLane;			//对应车道号
    BYTE	bySpeed;		//车道过车平均速度，0表示无效速度
    BYTE	byRes[2];		//保留
    DWORD	dwLightVehicle;	//小型车数量
    DWORD	dwMidVehicle;	//中型车数量
    DWORD	dwHeavyVehicle;	//重型车数量
    DWORD	dwTimeHeadway;	//车头时距，以秒计算
    DWORD	dwSpaceHeadway;	//车头间距，以米计算
    WORD	wSpaceOccupyRation;	//空间占有率，百分比计算，浮点数*1000,如341，表示34.1%
    WORD	wTimeOccupyRation;	//时间占有率，百分比计算，浮点数*1000,如341，表示34.1%
    BYTE	byRes1[16];		//保留
}NET_TPS_LANE_PARAM;

typedef struct _NET_TPS_INTER_TIME_INFO_
{
    BYTE byStart;                   //开始码,固定0xFE
    BYTE byCMD;                     //命令号， 08-定时成组数据指令
    BYTE byRes[2];                  //预留字节
    WORD wDeviceID;                 //设备ID
    WORD wDataLen;                  //数据长度:sizeof(NET_TPS_INTER_TIME_INFO)-8
    BYTE byTotalLaneNum;            //有效车道总数
    BYTE byRes1[15];                //预留字节
    INTER_TPS_TIME struStartTime;   //统计开始时间
    DWORD dwSamplePeriod;           //统计时间,单位秒
    NET_TPS_LANE_PARAM struLaneParam[TPS_MAX_LANE_NUM];
}NET_TPS_INTER_TIME_INFO;

typedef struct _NET_TPS_ALARM_INTER_TIMER_
{
    NET_TPS_ALARM_HEAD	struHead;	//信息头
    NET_TPS_INTER_TIME_INFO	struTPSInterInfo;	//交通参数统计信息
    BYTE				byRes[128];	//保留
}NET_TPS_ALARM_INTER_TIME;

typedef struct _TPS_PERIODS_TIME_INFO
{
   UINT8 uDeviceID;
   UINT8 uSamplePeriod;
   UINT8 uTotoalLaneNum;
}TPS_PERIODS_TIME_INFO;
/********************************end of CP2.1**********************************/

/**********************protocol 1.0 definition***************************/
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
#define MAX_TPS_RULE            8				/*最大参数规则数目*/
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

typedef struct _TPS_ALARM_INFO10
{
   UINT32 dwDeviceID;
   UINT8 byLane;
}TPS_ALARM_INFO10;
/****************************end of p1.0*********************************/

#endif // DATA_H
