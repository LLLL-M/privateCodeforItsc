#include <cstring>
#include <unistd.h>
#include <sys/types.h>       /* basic system data types */
#include <sys/socket.h>      /* basic socket definitions */
#include <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <cstdlib>
#include <algorithm>
#include "sock.h"
#include "camera.h"
#include "singleton.h"
#include "detector.h"

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

/********************************end of P2.1***************************************/
#pragma pack(pop)

int Camera::epollfd = -1;

inline Camera::TscStream::TscStream(int _fd, int _buflen) : fd(_fd)
{
	buflen = max(_buflen, 2048);
	buf = new char[buflen];
	if (buf != nullptr)
		memset(buf, 0, buflen);
}
inline Camera::TscStream::~TscStream()
{
	if (buf != nullptr)
	{
		delete [] buf;
		buf = nullptr;
	}
	if (fd != -1)
	{
		epoll_ctl(Camera::epollfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		fd = -1;
	}
}

Camera::TscStream & Camera::StreamCreate(int fd)
{
	for (auto & stream : streams)
	{
		if (stream.fd == fd)
			return stream;
	}
	if (streams.size() == MAX_CAMERA_NUM)
	{//如果流链表中超过40个流，则删除最开始插入的流
		streams.pop_front();
	}
	streams.emplace_back(fd);	//直接在list末尾构造添加
	return streams.back();
}

inline void Camera::StreamDelete(int fd)
{
	streams.remove_if([&fd](Camera::TscStream &ts)->bool{return ts.fd == fd;});
}

inline void Camera::StreamDelete(Camera::TscStream &stream)
{
	StreamDelete(stream.fd);
}

bool Camera::StreamRead(Camera::TscStream &stream, int size)
{
	int n = 0;
	bool ret = true;
	
	if (stream.offset < size)
	{
		n = read(stream.fd, stream.buf + stream.offset, size - stream.offset);
		if (n == -1)
		{
			if (!(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				StreamDelete(stream); //这两种调用方式都可以
				//ERR("tcp stream occur error");
			}
			ret = false;
		}
		else if (n == 0)
		{	//说明对方tcp连接已经关闭
			StreamDelete(stream);
			ret = false;
			//INFO("tcp stream has been closed!");
		}
		else
		{
			stream.offset += n;
			ret = (stream.offset == size) ? true : false;
		}	
	}
	return ret;
}

void Camera::RecvCameraData(int fd)
{
	NET_TPS_ALARM_HEAD *head = NULL;			//相机头部
	NET_TPS_REALTIME_INFO *rtinfo = NULL;		//实时信息
	//NET_TPS_INTER_TIME_INFO *itinfo = (NET_TPS_INTER_TIME_INFO *)(buf + offsetof(NET_TPS_ALARM_INTER_TIME, struTPSInterInfo));	//时段统计信息
	UInt8 lane;	//车道号
	Camera::TscStream &stream = StreamCreate(fd);
	DetectorArray &detectorArray = Singleton<DetectorArray>::GetInstance();
	
	//首先读取相机协议头部
	if (!StreamRead(stream, sizeof(NET_TPS_ALARM_HEAD)))
		return;
	head = (NET_TPS_ALARM_HEAD *)stream.buf;
	if (head->byType == 0xb6 && head->dwLength == sizeof(NET_TPS_ALARM_REALTIME))	//实时信息
	{
		if (!StreamRead(stream, sizeof(NET_TPS_ALARM_REALTIME)))
			return;
		rtinfo = (NET_TPS_REALTIME_INFO *)(stream.buf + offsetof(NET_TPS_ALARM_REALTIME, struTPSRealTimeInfo));
		lane = rtinfo->byLane;
		if (lane > MAX_LANE_NUM || lane == 0)
		{
			ERR("lane %d is invalid!", lane);
			StreamDelete(stream);
			return;
		}
		if (rtinfo->byCMD == 0x01)	
		{	//表明有车辆进入
			detectorArray.Enter(lane);
			//INFO("^^^^^^^^^^^lane %d vehicle enter, count: %d ^^^^^^^^^^", lane, count);
		}
		else if (rtinfo->byCMD == 0x02)
		{	//表明有车辆离开
			detectorArray.Leave(lane);
			//INFO("$$$$$$$$$$$lane %d vehicle leave$$$$$$$$$$", lane);
		}
		else if (rtinfo->byCMD == 0x03)
		{	//表明有车辆拥堵
			//INFO("$$$$$$$$$$$lane %d vehicle jam$$$$$$$$$$", lane);
		}
		memset(stream.buf, 0, stream.buflen);
		stream.offset = 0;
	}
#if 0
	else if (head->byType == 0xb7 && head->dwLength == sizeof(NET_TPS_ALARM_INTER_TIME))
	{//时段统计信息
	}
#endif
	else
	{
		StreamDelete(stream);
	}
}

void Camera::run(void *arg)
{
	Sock tcp;
	struct epoll_event ev, events[MAX_CAMERA_NUM];
	int i, nfds, clifd;
	struct sockaddr clientaddr;
	socklen_t socklen = sizeof(struct sockaddr);
	
	if (!tcp.CreateTcpSocket(7200))
		return;
	tcp.SetNonblock();
	tcp.Listen(MAX_CAMERA_NUM);
	epollfd = epoll_create(MAX_CAMERA_NUM);
	if (epollfd == -1)
	{
		perror("create epoll fail!");
		return;
	}
	ev.data.fd = tcp.sockfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, tcp.sockfd, &ev) == -1)
	{
		perror("add srvfd to epoll error!");
		close(epollfd);
		return;
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
				StreamDelete(events[i].data.fd);
			}
			else if (events[i].data.fd == tcp.sockfd)
			{	//说明有新的相机连接建立
				clifd = tcp.Accept(&clientaddr, &socklen);
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
			}
			else	//说明有新的相机数据传输过来
			{
				RecvCameraData(events[i].data.fd);
			}
		}
	}
	
	close(epollfd);
	return;
}

