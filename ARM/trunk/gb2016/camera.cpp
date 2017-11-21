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
 * Ŀǰ������߰汾��Ϊ2.1������2.0ֻ��Ϊ������Ŀʹ�ã��������ֻʵ��2.1Э��
 * Hikvision ��ͨ����ͨѶЭ��V2.1  added by Jicky at 2016.09.05
 * note: �������ݽṹ�е�WORD, DWORD�������ݣ���Ҫת��Ϊ�����ֽ�������ʹ��BYTE��������ת��
 */
typedef struct
{
    WORD wYear;                 //�꣬eg��2014
    BYTE byMonth;               //�£�1~12
    BYTE byDay;                 //�գ�1~31
    BYTE byHour;                //ʱ��0~23
    BYTE byMinute;              //�֣�0~59
    BYTE bySecond;              //�룬0~59
    BYTE byRes;
    WORD wMilliSec;             //���룬0~999
    UINT8 byRes1[2];
} INTER_TPS_TIME;

typedef struct _NET_TPS_ALARM_HEAD_	/*140bytes*/
{
	DWORD	dwLength;	//�����ܳ���:sizeof(NET_TPS_ALARM_REALTIME)
						//����sizeof(NET_TPS_ALARM_INTER_TIME)
						//��byType�йأ�ע��:���ֶ�δʹ�������ֽ���
	BYTE	byRes[2];	//����
	BYTE	byType;		//��Ϣ����: 0xB6ʵʱ���ݣ�0xB7ͳ������
	BYTE	byRes1[133];	//��������
} NET_TPS_ALARM_HEAD;

//ʵʱ��Ϣ
typedef struct _NET_TPS_REALTIME_INFO_/*36 bytes*/
{
    BYTE byStart;               //��ʼ��
    BYTE byCMD;                 //����ţ�01-����ָ�02-�뿪ָ�03-ӵ�� 
                                //״ָ̬��(Ϊ03ʱ��ֻ��byLaneState��byQueueLen��Ч)
    BYTE byRes[2];              //Ԥ���ֽ�
    WORD wDeviceID;             //�豸ID
    WORD wDataLen;              //���ݳ���
    BYTE byLane;                //��Ӧ������
    BYTE bySpeed;               //��Ӧ����
    BYTE byLaneState;           //����״̬��0-��״̬��1-��ͨ��2-ӵ����3-����
    BYTE byQueueLen;            //����״̬���Ŷӳ���
    BYTE byRes1[24];            //����
} NET_TPS_REALTIME_INFO;
typedef NET_TPS_REALTIME_INFO NET_DVR_TPS_REAL_TIME_INFO;

typedef struct _NET_TPS_ALARM_REALTIME_
{
	NET_TPS_ALARM_HEAD	struHead;	//��Ϣͷ
	INTER_TPS_TIME		struTime;	//���ʱ��
	NET_TPS_REALTIME_INFO	struTPSRealTimeInfo;	//��ͨ����ʵʱ��Ϣ
	BYTE				byRes[24];	//����
} NET_TPS_ALARM_REALTIME;
typedef NET_TPS_ALARM_REALTIME NET_DVR_TPS_REAL_TIME_ALARM;

//ʱ����Ϣ
typedef struct _NET_TPS_LANE_PARAM_
{
	BYTE	byLane;			//��Ӧ������
	BYTE	bySpeed;		//��������ƽ���ٶȣ�0��ʾ��Ч�ٶ�
	BYTE	byRes[2];		//����
	DWORD	dwLightVehicle;	//С�ͳ�����
	DWORD	dwMidVehicle;	//���ͳ�����
	DWORD	dwHeavyVehicle;	//���ͳ�����
	DWORD	dwTimeHeadway;	//��ͷʱ�࣬�������
	DWORD	dwSpaceHeadway;	//��ͷ��࣬���׼���
	WORD	wSpaceOccupyRation;	//�ռ�ռ���ʣ��ٷֱȼ��㣬������*1000,��341����ʾ34.1%
	WORD	wTimeOccupyRation;	//ʱ��ռ���ʣ��ٷֱȼ��㣬������*1000,��341����ʾ34.1%
	BYTE	byRes1[16];		//����
} NET_TPS_LANE_PARAM;

#define TPS_MAX_LANE_NUM 8          //���ͳ�ƹ�������
typedef struct _NET_TPS_INTER_TIME_INFO_
{
    BYTE byStart;                   //��ʼ��,�̶�0xFE
    BYTE byCMD;                     //����ţ� 08-��ʱ��������ָ��
    BYTE byRes[2];                  //Ԥ���ֽ�
    WORD wDeviceID;                 //�豸ID
    WORD wDataLen;                  //���ݳ���:sizeof(NET_TPS_INTER_TIME_INFO)-8
    BYTE byTotalLaneNum;            //��Ч��������
    BYTE byRes1[15];                //Ԥ���ֽ�
    INTER_TPS_TIME struStartTime;   //ͳ�ƿ�ʼʱ��
    DWORD dwSamplePeriod;           //ͳ��ʱ��,��λ��
    NET_TPS_LANE_PARAM struLaneParam[TPS_MAX_LANE_NUM];
} NET_TPS_INTER_TIME_INFO;

typedef struct _NET_TPS_ALARM_INTER_TIMER_
{
	NET_TPS_ALARM_HEAD	struHead;	//��Ϣͷ
	NET_TPS_INTER_TIME_INFO	struTPSInterInfo;	//��ͨ����ͳ����Ϣ
	BYTE				byRes[128];	//����
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
	{//����������г���40��������ɾ���ʼ�������
		streams.pop_front();
	}
	streams.emplace_back(fd);	//ֱ����listĩβ�������
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
				StreamDelete(stream); //�����ֵ��÷�ʽ������
				//ERR("tcp stream occur error");
			}
			ret = false;
		}
		else if (n == 0)
		{	//˵���Է�tcp�����Ѿ��ر�
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
	NET_TPS_ALARM_HEAD *head = NULL;			//���ͷ��
	NET_TPS_REALTIME_INFO *rtinfo = NULL;		//ʵʱ��Ϣ
	//NET_TPS_INTER_TIME_INFO *itinfo = (NET_TPS_INTER_TIME_INFO *)(buf + offsetof(NET_TPS_ALARM_INTER_TIME, struTPSInterInfo));	//ʱ��ͳ����Ϣ
	UInt8 lane;	//������
	Camera::TscStream &stream = StreamCreate(fd);
	DetectorArray &detectorArray = Singleton<DetectorArray>::GetInstance();
	
	//���ȶ�ȡ���Э��ͷ��
	if (!StreamRead(stream, sizeof(NET_TPS_ALARM_HEAD)))
		return;
	head = (NET_TPS_ALARM_HEAD *)stream.buf;
	if (head->byType == 0xb6 && head->dwLength == sizeof(NET_TPS_ALARM_REALTIME))	//ʵʱ��Ϣ
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
		{	//�����г�������
			detectorArray.Enter(lane);
			//INFO("^^^^^^^^^^^lane %d vehicle enter, count: %d ^^^^^^^^^^", lane, count);
		}
		else if (rtinfo->byCMD == 0x02)
		{	//�����г����뿪
			detectorArray.Leave(lane);
			//INFO("$$$$$$$$$$$lane %d vehicle leave$$$$$$$$$$", lane);
		}
		else if (rtinfo->byCMD == 0x03)
		{	//�����г���ӵ��
			//INFO("$$$$$$$$$$$lane %d vehicle jam$$$$$$$$$$", lane);
		}
		memset(stream.buf, 0, stream.buflen);
		stream.offset = 0;
	}
#if 0
	else if (head->byType == 0xb7 && head->dwLength == sizeof(NET_TPS_ALARM_INTER_TIME))
	{//ʱ��ͳ����Ϣ
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
			{	//˵�����µ�������ӽ���
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
			else	//˵�����µ�������ݴ������
			{
				RecvCameraData(events[i].data.fd);
			}
		}
	}
	
	close(epollfd);
	return;
}

