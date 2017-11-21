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


extern MsgRealtimeVolume gStructMsgRealtimeVolume;
extern pthread_rwlock_t gLockRealtimeVol;                       //???????????��??
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
	NET_TPS_ALARM_HEAD *head = NULL;			//���ͷ��
	NET_TPS_REALTIME_INFO *rtinfo = NULL;		//ʵʱ��Ϣ
	//NET_TPS_INTER_TIME_INFO *itinfo = (NET_TPS_INTER_TIME_INFO *)(buf + offsetof(NET_TPS_ALARM_INTER_TIME, struTPSInterInfo));	//ʱ��ͳ����Ϣ
	UInt8 lane;	//������
	int count = 0;
	Stream *stream = StreamCreate(fd);
	
	if (stream == NULL)
	{
		StreamClose(fd);
		return;
	}
	//���ȶ�ȡ���Э��ͷ��
	if (StreamRead(stream, sizeof(NET_TPS_ALARM_HEAD)) == FALSE)
		return;
	head = (NET_TPS_ALARM_HEAD *)stream->buf;
	if (head->byType == 0xb6 && head->dwLength == sizeof(NET_TPS_ALARM_REALTIME))	//ʵʱ��Ϣ
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
		{//�����г�������
			count = (rtinfo->byRes[0] << 8) | rtinfo->byRes[1];
			OFTEN("^^^^^^^^^^^lane %d vehicle enter, count: %d ^^^^^^^^^^", lane, count);
		} else if (rtinfo->byCMD == 0x02)//�����г����뿪
			OFTEN("$$$$$$$$$$$lane %d vehicle leave$$$$$$$$$$", lane);
		else if (rtinfo->byCMD == 0x03)	//�����г���ӵ��
			OFTEN("$$$$$$$$$$$lane %d vehicle jam$$$$$$$$$$", lane);
		StreamReset(stream);
	}
	//else if (head->byType == 0xb7 && head->dwLength == sizeof(NET_TPS_ALARM_INTER_TIME))	//ʱ��ͳ����Ϣ
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
			{	//˵�����µ�������ӽ���
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
			else	//˵�����µ�������ݴ������
			{
				RecvCameraData(events[i].data.fd);
			}
		}
	}
	
	close(srvfd);
	close(epollfd);
	pthread_exit(NULL);
}

