#include <stdio.h>
#include "canmsg.h"
#include <time.h>
#include <errno.h>
#include "common.h"
#include "debug.h"
#include "io_ioctl.h"

#define IFUSE_VECHILE	 1	/* ���ݲ�ͨ�������*/
int g_fd_send = 0;  //CAN�׽���
//�ƿذ��ѹ������������
pthread_mutex_t g_MutexLamp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ped_lock = PTHREAD_MUTEX_INITIALIZER;

#define GET_BIT(data, bit) ((data >> bit) & 0x01)	//ȡdata��bitλ����
#define BOARD_NUM	6
unsigned long   g_volt[BOARD_NUM] = {0}; 	//�ƿص�ѹ
unsigned char   g_cur[BOARD_NUM][BOARD_NUM] = {{0}};	//�ƿص���
unsigned short boardInfoFromInput[6] = {0}; //������������
unsigned short boardInfoSend[3] = {0};           //������������鷢��
unsigned short signForSendVechileDataRequire = 0;//�������������(0�򳵼��1���ͣ�1�򳵼��2����)
UINT8 sendData[6] = {0x00};  /* ��Ҫ��������������8 bit*/
UINT8 Car_On_Sign[6][8] ={{0}}; //��¼ĳ�������г�״̬����ʱ��
time_t lastTime[6][8] = {{0}};
unsigned short iPedOnState = 0;
UINT8 recvPedData[4];//���յ�����������

extern int g_io_fd;
extern struct SPECIAL_PARAMS s_special_params;   //�����������

void canits_set_bitrate(const char *name, unsigned long bitrate)
{
	int err = can_set_bitrate(name, bitrate);
	if (err < 0)
	{
		ERR("set %s bitrate(%lu) failed\n", name, bitrate);
	}
}

void canits_can_start(const char *name)
{
	if (can_do_start(name) < 0)
	{
		ERR("%s start failed\n", name);
	}
}

void canits_create_recv_thread()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, canits_recv_thread, NULL))
	{
		//log error
	}
}
void canits_create_socket_can_send()
{
    struct sockaddr_can addr;
    struct ifreq ifr;

    g_fd_send = socket(AF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, "can0");
    ioctl(g_fd_send, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(g_fd_send, (struct sockaddr *)&addr, sizeof(addr));	
}

void canits_init()
{
	can_do_stop("can0");
	canits_set_bitrate("can0", 500000);
	INFO("set can0 bitrate 500K!!!\n");
	canits_can_start("can0");
	canits_create_recv_thread();
    	canits_create_socket_can_send();
}

void canits_send(struct can_frame *pcanfram)
{
	int ret = write(g_fd_send, pcanfram, sizeof(struct can_frame));
	if (ret == -1)
	{
		ERR("canits_send error, error info:%s\n", strerror(errno));
	}
}

void parse_lamp_voltcur(struct can_frame *pframe)
{
	//int i = 0;
	pthread_mutex_lock(&g_MutexLamp);
	//��ѹ
	int boardNo = pframe->can_id&0x7;
	if (boardNo<1 || boardNo>4)
	{
		pthread_mutex_unlock(&g_MutexLamp);
		return;
	}
	g_volt[boardNo-1] =  (pframe->can_id>>3)&0xfff;
	//INFO("boardNo:%d, g_volt:%#x\n", boardNo, g_volt[boardNo-1]);
	//����
	memcpy(g_cur[boardNo-1], pframe->data, 4);
	pthread_mutex_unlock(&g_MutexLamp);
}
/*�����������ݱ���*/
void save_Date_From_Vechile(struct can_frame *pframe)
{
	int i = 0;
	pthread_mutex_lock(&count_lock);
	if ((0x201 == pframe->can_id))
	{
		for (i = 0; i < 3; i++)//ֻȡǰ��λ������Ϣ
		{
			boardInfoFromInput[i] = pframe->data[i];
			DBG("vechile_data[%d]=0x%x\n",i,pframe->data[i]);
		}
	}
	else if ((0x202 == pframe->can_id))
	{
		for (i = 0; i < 3; i++) //ֻȡǰ��λ������Ϣ
		{
			boardInfoFromInput[i+3] = pframe->data[i];
			DBG("vechile_data[%d]=0x%x\n", i+3,pframe->data[i]);
		}
	}
	else
	{
		ERR("can_id is error\n");
	}
	pthread_mutex_unlock(&count_lock);
}
void *canits_recv_thread(void *p)
{
	INFO("can_recv_thread is running\n");
	struct sockaddr_can addr;
    struct ifreq ifr;
	int __attribute__((unused)) i = 0;
	int m_fd_recv = socket(AF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, "can0");
	ioctl(m_fd_recv, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	bind(m_fd_recv, (struct sockaddr *)&addr, sizeof(addr));
	struct can_frame frame;
	while (1)
	{
		if (read(m_fd_recv, &frame, sizeof(struct can_frame)) > 0)
		{
		    //INFO("===> can Id   0x%x\n",frame.can_id);
			if (GET_BIT(frame.can_id, 15) == 1 && frame.can_dlc == 4) {
			//��15λΪ1���ҳ���Ϊ4��can��Ϣ��ʾ�ǵƿذ��������ѹ����
				/*INFO("��������ѹ����:");
				for (i = 0; i < frame.can_dlc; i++)
				{
					INFO("dlc=%d,data[%d]=%d\n", frame.can_dlc, i,frame.data[i]);
				}*/
				parse_lamp_voltcur(&frame);
			}
			//�������Ϣ����
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))
			{
				DBG("�������Ϣ����:can_id=0x%x\n",frame.can_id);
				//save_Date_From_Vechile(&frame);//���������Ϣ
			}
			//ǰ���5������״̬
			else if(frame.can_id == 0x401 && frame.can_dlc == 1) {
				INFO("recv foreboard info, data:%#x", frame.data[0]);
				if ((g_io_fd != -1) && (frame.data[0] & 0x1f) != 0)
					ioctl(g_io_fd, IO_SET_BUTTON_STATUS, &frame.data[0]);
			}
		}
	}
}


/*****************************************************�ӿ�*************************************************/
void i_can_its_init()
{
	canits_init();
	//�����Ƶ�����������߳�
	video_detece_main_thread_create();
}

//���͵������
void i_can_its_send_led_request(int boardNum, unsigned short *poutLamp)
{
	if (NULL == poutLamp)
		return;
	unsigned short *p = poutLamp;
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 7;
	m_frame_send.data[0] = s_special_params.iSignalMachineType & 0xff;
	m_frame_send.data[1] = p[0]&0xff;
	m_frame_send.data[2] = ((p[1]&0xf)<<4) | ((p[0]>>8)&0xf);
	m_frame_send.data[3] = (p[1]>>4)&0xff;
	m_frame_send.data[4] = p[2]&0xff;
	m_frame_send.data[5] = ((p[3]&0xf)<<4) | ((p[2]>>8)&0xf);
	m_frame_send.data[6] = (p[3]>>4)&0xff;
	canits_send(&m_frame_send);
}

//��ȡ��ѹ
void i_can_its_get_Volt(int boardNum, unsigned short *pboardInfo)
{
	if (NULL == pboardInfo)
	{	
		return;
	}
	pthread_mutex_lock(&g_MutexLamp);
	switch (boardNum)
	{
	case 1:
		*pboardInfo = g_volt[0]&0xfff;
		break;
	case 2:
		*pboardInfo = g_volt[1]&0xfff;//((g_volt[1]&0x3f)<<6) | ((g_volt[0]>>12)&0x3f);
		break;
	case 3:
		*pboardInfo = g_volt[2]&0xfff;//(g_volt[1]>>6)&0xfff;
		break;
	case 4:
		*pboardInfo = g_volt[3]&0xfff;//g_volt[2]&0xfff;
		break;
	/*case 5:
		*pboardInfo = ((g_volt[3]&0x3f)<<6) | ((g_volt[2]>>12)&0x3f);
		break;
	case 6:
		*pboardInfo = (g_volt[3]>>6)&0xfff;
		break;
	case 7:
		*pboardInfo = g_volt[4]&0xfff;
		break;
	case 8:
		*pboardInfo = ((g_volt[5]&0x3f)<<6) | ((g_volt[4]>>12)&0x3f);
		break;*/
	default:
		break;
	} 
	pthread_mutex_unlock(&g_MutexLamp);
}

//��ȡ����
unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen)
{
	unsigned short curinfo = 0;
	//unsigned char *p = g_cur[0];
	pthread_mutex_lock(&g_MutexLamp);
	curinfo = g_cur[boardNum - 1][pahseNum - 1];
	pthread_mutex_unlock(&g_MutexLamp);
	return curinfo;
}
// ���չ�����Ϣ
unsigned short recv_date_from_vechile(int iboardNo)
{
	pthread_mutex_lock(&count_lock);
	DBG("###TEST2###boardInfoFromInput[%d] = %d\n",(iboardNo-1),boardInfoFromInput[(iboardNo-1)] );
	boardInfoSend[iboardNo-1] = ((0xff00 & (boardInfoFromInput[(iboardNo-1)*2 +1]<<8)) | (0x00ff & boardInfoFromInput[(iboardNo-1)*2 ]));
	DBG("boardInfoSend[%d] = 0x%x\n",(iboardNo-1),boardInfoSend[iboardNo-1]);
	
	pthread_mutex_unlock(&count_lock);
	return boardInfoSend[iboardNo-1];	
}

void video_detece_main_thread_create()
{
	//������Ƶ�������������߳�
	pthread_t video_thread;
	
	if (0 != pthread_create(&video_thread, NULL, (void*)video_detect_interface, NULL))
	{
		//log error
		ERR("video detect recv thread creat failure!!\n");
	} else {
		INFO("****video detece creat successful!**********");
	}
}

/*��Ƶ�������ӿ�*/

int  video_detect_interface(void)
{
	int server_sockfd = 0;
	int client_sockfd = 0;
	socklen_t  server_len = sizeof(struct sockaddr_in);
	socklen_t  client_len = sizeof(struct sockaddr_in);
	struct sockaddr_in server_address;
	memset(&server_address, 0 , sizeof(struct sockaddr_in));
	struct sockaddr_in client_address;
	memset(&client_address, 0 , sizeof(struct sockaddr_in));
	//pthread_t recv_camera_thread = 0;
	//int i = 0;
	//int j = 0;
	int retval = -1;			//����ֵ
	int reuse0=1;
	struct timeval timeout;	//���ճ�ʱ
	
	/*�����������׽���*/
ReStart:
	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	DBG("server_sockfd=%d\n",server_sockfd);
	/*����׽���*/
	bzero(&server_address,sizeof(server_address)); 
	/*�����׽���*/
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(7200);
	server_len = sizeof(server_address);
	/*�����׽��ָ���*/	
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0))==-1) 
	{
		return -1;
	}
	/*���ý��ճ�ʱʱ��*/
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0)
	{	 
		perror("setsockopt() error");
		return -1;
	}

	/*���׽���*/
	retval = bind(server_sockfd,(struct sockaddr *)&server_address, server_len);
	if(retval < 0)
	{
		ERR("sockef bind failed!!\n");
		close(server_sockfd);
		goto ReStart;
	}
	else
	{
        ERR("sockef bind Success!!\n");
	}
	/*��������*/
	retval = listen(server_sockfd,5);
	if(retval <0)
	{
		ERR("sockef listen failed!!\n");
		close(server_sockfd);
		return -1;
	}
	DBG("\nserver waiting\n");
	client_len = sizeof(client_address);
	bzero(&client_address,sizeof(client_address)); 
	while(1)
	{
		/*���տͻ�������*/
		DBG("****************while**************\n");
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
		DBG("client_sockfd=%d\n",client_sockfd);
		if(client_sockfd>=0)
		{
			/*��������ӣ��ʹ���һ�������Ľ����߳�*/
			INFO("****************recv**************\n");
			recv_from_camera(&client_sockfd);//���ݽ����봦��
			//pthread_create(&recv_camera_thread,NULL,(void*)recv_from_camera,&client_sockfd);//�����߳�
		}
		else
		{
			ERR("client_sockfd creat error!\n");
			continue;
		}
	}
	return 0;	
}
/*��Ƶ���������պ���*/
void recv_from_camera(int *client_fd)
{
	INTER_DVR_REQUEST_HEAD_V30 alarmuphead;
    	memset(&alarmuphead, 0, sizeof(INTER_DVR_REQUEST_HEAD_V30));
    	NET_TPS_ALARM alarmParam;
	memset(&alarmParam, 0, sizeof(NET_TPS_ALARM));
	
	char recvtmpbuf[BuffLen] = {0};
	int retval = -1;
	long leftlen = 0;
	/*��һ�ν���ͷ����*/
	if(recv(*client_fd, (INT8*)&alarmuphead, sizeof(INTER_DVR_REQUEST_HEAD_V30), 0)<0)
	{
		ERR("recv failed!!\n");
		close(*client_fd);
		return ;
	} 
	DBG("dwLength:%ld,byCommand=0x%x\n",alarmuphead.dwLength,alarmuphead.byCommand);//���Ȳ���������ת��
	if(alarmuphead.dwLength> sizeof(INTER_DVR_REQUEST_HEAD_V30))
	{
		leftlen = alarmuphead.dwLength - sizeof(INTER_DVR_REQUEST_HEAD_V30);
		DBG("leftlen(%ld)=dwLength(%ld)-sizeof(INTER_DVR_REQUEST_HEAD_V30)(%ld)\n",leftlen,alarmuphead.dwLength,sizeof(INTER_DVR_REQUEST_HEAD_V30));
		DBG("sizeof(NET_TPS_ALARM)=%d\n",sizeof(NET_TPS_ALARM));
		/*�ڶ��ν��յ����ݰ�*/
		retval = recv(*client_fd,recvtmpbuf , leftlen, 0);
		/*�ѽ��յ������ݸ�ֵ���ṹ��*/
		memcpy(&alarmParam, recvtmpbuf, sizeof(NET_TPS_ALARM));
		if(retval<=0)
		{
			DBG("recv again failed!\n");
		}
		else
		{
			deal_data_from_video(alarmParam);/* ���ݴ���*/
		}
      	}
	else
	{
		ERR("recv Length failed!\n");
	}
	close(*client_fd);/*�ر��׽��֣��߳̽���*/
}
//��Ƶ���������ݴ���
void  deal_data_from_video(NET_TPS_ALARM  alarmData)
{
	int i = 0;
	int deviceID = 0;//������
	int deviceType = 0; //�������(1���������λ��2�Ÿ���λ)
	IS_NEED_DATA vechileData[4];
	int dateType = 0;//��������(�Ƿ���Ҫ����)
	UINT16 boardID = 0;     /*�������ֳ����*/
	/*һ��������ĸ�����������Ч*/
	for(i=0;i<4;i++)
	{
		vechileData[i].varyType = ntohl(alarmData.struTPSInfo.struLaneParam[i].dwVaryType);
		vechileData[i].laneSpeed = ntohl(alarmData.struTPSInfo.struLaneParam[i].dwLaneVelocity);
		vechileData[i].ruleID = alarmData.struTPSInfo.struLaneParam[i].byRuleID;//���ûת������
		vechileData[i].cameraID = ntohl(alarmData.dwDeviceId);//����������
		DBG("lane[%d] VaryType=0x%x,speed=%d,cameraID=%d,byRuleID=%d\n",i,vechileData[i].varyType,vechileData[i].laneSpeed,vechileData[i].cameraID,vechileData[i].ruleID);

		/*�ж������ź�����*/
		if(vechileData[i].cameraID == 1)
		{
			deviceID = 0;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 2)
		{
			deviceID = 0;
			deviceType = TALL_4bits;
		}
		else if(vechileData[i].cameraID == 3)
		{
			deviceID = 1;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 4)
		{
			deviceID = 1;
			deviceType = TALL_4bits;
		}
		else if(vechileData[i].cameraID == 5)
		{
			deviceID = 2;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 6)
		{
			deviceID = 2;
			deviceType = TALL_4bits;
		}
		else if(vechileData[i].cameraID == 7)
		{
			deviceID = 3;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 8)
		{
			deviceID = 3;
			deviceType = TALL_4bits;
		}
		else if(vechileData[i].cameraID == 9)
		{
			deviceID = 4;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 10)
		{
			deviceID = 4;
			deviceType = TALL_4bits;
		}
		else if(vechileData[i].cameraID == 11)
		{
			deviceID = 5;
			deviceType = LOW_4bits;
		}
		else if(vechileData[i].cameraID == 12)
		{
			deviceID = 5;
			deviceType = TALL_4bits;
		}

		if(deviceID<3)
		{
			boardID = 0x205;//��һ�鳵���
		}
		else
		{
			boardID = 0x206;//�ڶ��鳵���
		}
		//״̬����������֮һ��������Ϊ��Ҫ����
		if(ENUM_TRAFFIC_VARY_VEHICLE_ENTER == vechileData[i].varyType)
		{
			dateType = IsNeedSend;
			sendData[deviceID] = sendData[deviceID] | (1<<( i+deviceType));//������Ϊ0x01,����������1
		}
		else if(ENUM_TRAFFIC_VARY_VEHICLE_LEAVE == vechileData[i].varyType)
		{
			dateType = IsNeedSend;
			sendData[deviceID] = sendData[deviceID] & (~(1<< (i+deviceType)));//������Ϊ0x02,�����뿪��0
		}
	}
	if(dateType)
	{
		dateType = NoNeedSend;//״̬����Ϊ����Ҫ����
		#if 1
		pthread_mutex_lock(&count_lock);
		boardInfoFromInput[deviceID] = sendData[deviceID];//�������ֱ���ϴ����س���
		INFO("###TEST1###boardInfoFromInput[%d] = %d\n",deviceID,boardInfoFromInput[deviceID] );
		pthread_mutex_unlock(&count_lock);
		#else
		struct can_frame m_video_send;
		memset(&m_video_send, 0 , sizeof(struct can_frame));
		m_video_send.can_id = boardID;//ID����Ϊ0x203������Ƶ ����
		m_video_send.can_dlc = 3;
		m_video_send.data[0] = deviceID;//�������ֶ�Ӧ�ĳ��쵥Ƭ��
		m_video_send.data[1] = deviceType;//�������ָߵ�λ
		m_video_send.data[2] = sendData[deviceID];
		
		DBG("****send***\nboardID:0x%x,send.can_id=0x%x,deviceID:%d,deviceType=%d,sendData[%d]:0x%x\n",boardID,m_video_send.can_id,deviceID,deviceType,deviceID,sendData[deviceID]);
		canits_send(&m_video_send);
		#endif
	}
}

