#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>       /* basic system data types */
#include <sys/socket.h>      /* basic socket definitions */
#include <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "canmsg.h"
#include "platform.h"
#include "HikConfig.h"
#include "configureManagement.h"
#include "stream.h"

#pragma pack(push, 1)
/************************************************************************
 * 目前相机基线版本都为2.1，至于2.0只作为定制项目使用，因此我们只实现2.1协议
 * Hikvision 交通流量通讯协议V2.1  added by Jicky at 2016.09.05
 * note: 对于数据结构中的WORD, DWORD类型数据，需要转换为本地字节序后才能使用BYTE类型无需转换
 */
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
} INTER_TPS_TIME;

typedef struct _NET_TPS_ALARM_HEAD_	/*140bytes*/
{
	DWORD	dwLength;	//报文总长度:sizeof(NET_TPS_ALARM_REALTIME)
						//或者sizeof(NET_TPS_ALARM_INTER_TIME)
						//和byType有关，注意:此字段未使用网络字节序
	BYTE	byRes[2];	//保留
	BYTE	byType;		//消息类型: 0xB6实时数据，0xB7统计数据
	BYTE	byRes1[133];	//保留数据
} NET_TPS_ALARM_HEAD;

//实时信息
typedef struct _NET_TPS_REALTIME_INFO_/*36 bytes*/
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
} NET_TPS_REALTIME_INFO;
typedef NET_TPS_REALTIME_INFO NET_DVR_TPS_REAL_TIME_INFO;

typedef struct _NET_TPS_ALARM_REALTIME_
{
	NET_TPS_ALARM_HEAD	struHead;	//信息头
	INTER_TPS_TIME		struTime;	//检测时间
	NET_TPS_REALTIME_INFO	struTPSRealTimeInfo;	//交通参数实时信息
	BYTE				byRes[24];	//保留
} NET_TPS_ALARM_REALTIME;
typedef NET_TPS_ALARM_REALTIME NET_DVR_TPS_REAL_TIME_ALARM;

//时段信息
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
} NET_TPS_LANE_PARAM;

#define TPS_MAX_LANE_NUM 8          //最大统计规则数据
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
} NET_TPS_INTER_TIME_INFO;

typedef struct _NET_TPS_ALARM_INTER_TIMER_
{
	NET_TPS_ALARM_HEAD	struHead;	//信息头
	NET_TPS_INTER_TIME_INFO	struTPSInterInfo;	//交通参数统计信息
	BYTE				byRes[128];	//保留
} NET_TPS_ALARM_INTER_TIME;


extern MsgRealtimeVolume gStructMsgRealtimeVolume;
extern pthread_rwlock_t gLockRealtimeVol;                       //???????????д??
extern SignalControllerPara *gSignalControlpara;

void ConvertVehicleFlowInfo(NET_TPS_LANE_PARAM* tps_lane_prm)
{
	VolumeOccupancy* pVolOccupancy = NULL;
	if (tps_lane_prm->byLane <= 0 || tps_lane_prm->byLane > 48)// illegal lane num
		return;

	pthread_rwlock_wrlock(&gLockRealtimeVol);
	pVolOccupancy = gStructMsgRealtimeVolume.volumeOccupancy.struVolume;
	memset(&pVolOccupancy[tps_lane_prm->byLane - 1], 0, sizeof(VolumeOccupancy));
	pVolOccupancy[tps_lane_prm->byLane - 1].byDetectorOccupancy = tps_lane_prm->wTimeOccupyRation;
	pVolOccupancy[tps_lane_prm->byLane - 1].wVehicleHeadDistance = tps_lane_prm->dwSpaceHeadway;
	pVolOccupancy[tps_lane_prm->byLane - 1].wVehicleHeadTimeDistance = tps_lane_prm->dwTimeHeadway;
	pVolOccupancy[tps_lane_prm->byLane - 1].byDetectorVolume = tps_lane_prm->dwLightVehicle + tps_lane_prm->dwMidVehicle + tps_lane_prm->dwHeavyVehicle;
	pVolOccupancy[tps_lane_prm->byLane - 1].byVehicleSpeed = tps_lane_prm->bySpeed;
	pthread_rwlock_unlock(&gLockRealtimeVol);
}



/********************************end of P2.1***************************************/
#pragma pack(pop)

#define MAX_CAMERA_NUM	40

static int CreateTCPSocket(UInt16 port)
{
	struct sockaddr_in addr = 
    {   
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
        .sin_zero = {0},
    };
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	
	if (sockfd == -1)
	{
		perror("can't create tcp socket!");
		return -1;
	}
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt SO_REUSEADDR fail");
		close(sockfd);
        return -1;
	}
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1) 
	{
		perror("set tcp sockfd nonblock fail!");
		close(sockfd);
        return -1;
    }
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)
    {
        perror("tcp sockfd bind error!");
		close(sockfd);
        return -1;
    }
	if (listen(sockfd, MAX_CAMERA_NUM) == -1) 
    {
        perror("listen error");
		close(sockfd);
        return -1;
    }
	return sockfd;
}

static void RecvCameraData(int fd)
{
	NET_TPS_ALARM_HEAD *head = NULL;			//相机头部
	NET_TPS_REALTIME_INFO *rtinfo = NULL;		//实时信息
	//NET_TPS_INTER_TIME_INFO *itinfo = (NET_TPS_INTER_TIME_INFO *)(buf + offsetof(NET_TPS_ALARM_INTER_TIME, struTPSInterInfo));	//时段统计信息
	UInt8 lane;	//车道号
	int count = 0;
	Stream *stream = StreamCreate(fd);
	
	if (stream == NULL)
	{
		StreamClose(fd);
		return;
	}
	//首先读取相机协议头部
	if (StreamRead(stream, sizeof(NET_TPS_ALARM_HEAD)) == FALSE)
		return;
	head = (NET_TPS_ALARM_HEAD *)stream->buf;
	if (head->byType == 0xb6 && head->dwLength == sizeof(NET_TPS_ALARM_REALTIME))	//实时信息
	{
		if (StreamRead(stream, sizeof(NET_TPS_ALARM_REALTIME)) == FALSE)
			return;
		rtinfo = (NET_TPS_REALTIME_INFO *)(stream->buf + offsetof(NET_TPS_ALARM_REALTIME, struTPSRealTimeInfo));
		lane = rtinfo->byLane;
		if (lane > MAX_LANE_NUM || lane == 0)
		{
			ERR("lane %d is invalid!", lane);
			StreamDelete(stream);
			return;
		}
		DealVehPassData(lane, rtinfo->byCMD == 0x01);
		if (rtinfo->byCMD == 0x01)	
		{//表明有车辆进入
			count = (rtinfo->byRes[0] << 8) | rtinfo->byRes[1];
			OFTEN("^^^^^^^^^^^lane %d vehicle enter, count: %d ^^^^^^^^^^", lane, count);
		} else if (rtinfo->byCMD == 0x02)//表明有车辆离开
			OFTEN("$$$$$$$$$$$lane %d vehicle leave$$$$$$$$$$", lane);
		else if (rtinfo->byCMD == 0x03)	//表明有车辆拥堵
			OFTEN("$$$$$$$$$$$lane %d vehicle jam$$$$$$$$$$", lane);
		StreamReset(stream);
	}
	//else if (head->byType == 0xb7 && head->dwLength == sizeof(NET_TPS_ALARM_INTER_TIME))	//时段统计信息
	else
	{
		StreamDelete(stream);	
	}
}

void *CameraCommunication(void *arg)
{
	int srvfd, clifd;
	struct epoll_event ev, events[MAX_CAMERA_NUM];
	int i, nfds;
	struct sockaddr clientaddr;
	socklen_t socklen = sizeof(struct sockaddr);
	
	srvfd = CreateTCPSocket(7200);
	if (srvfd == -1)
		pthread_exit(NULL);
	epollfd = epoll_create(MAX_CAMERA_NUM);
	if (epollfd == -1)
	{
		perror("create epoll fail!");
		close(srvfd);
		pthread_exit(NULL);
	}
	ev.data.fd = srvfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, srvfd, &ev) == -1)
	{
		perror("add srvfd to epoll error!");
		close(srvfd);
		close(epollfd);
	}
	memset(events, 0, sizeof(events));
	while (1)
	{
		nfds = epoll_wait(epollfd, events, MAX_CAMERA_NUM, -1);
		if (nfds == -1)
		{
			usleep(100000);
			continue;
		}
		for (i = 0; i < nfds; i++)
		{
			if((events[i].events & EPOLLRDHUP)
				|| (!(events[i].events & EPOLLIN)))
			{
				ERR("epoll delete fd %d, events: %#x", events[i].data.fd, events[i].events);
				StreamClose(events[i].data.fd);
			}
			else if (events[i].data.fd == srvfd)
			{	//说明有新的相机连接建立
				clifd = accept(srvfd, &clientaddr, &socklen);
				if (clifd == -1)
					continue;
				fcntl(clifd, F_SETFL, fcntl(clifd, F_GETFL, 0) | O_NONBLOCK);
				ev.data.fd = clifd;
				ev.events = EPOLLIN;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clifd, &ev) == -1)
				{
					perror("add clifd to epoll error!");
					close(clifd);
				}
				else
					OFTEN("epoll add fd %d", clifd);
			}
			else	//说明有新的相机数据传输过来
			{
				RecvCameraData(events[i].data.fd);
			}
		}
	}
	
	close(srvfd);
	close(epollfd);
	pthread_exit(NULL);
}

