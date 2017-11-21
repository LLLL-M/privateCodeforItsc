#include <stdio.h>
#include "canmsg.h"
#include <time.h>
#include "hik.h"
#include "its.h"
#include "configureManagement.h"

int g_fd_send = 0;  //CAN�׽���
//�ƿذ��ѹ������������
pthread_mutex_t g_MutexLamp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long   g_volt[6] = {0}; 	             //�ƿص�ѹ
unsigned char   g_cur[6][6] = {{0}};	             //�ƿص���
unsigned short boardInfoFromInput[6] = {0};      //������������
unsigned short boardInfoSend[3] = {0};           //������������鷢��
unsigned short signForSendVechileDataRequire = 0;//�������������(0�򳵼��1���ͣ�1�򳵼��2����)
UINT8 sendData[6] = {0x00};                      //��Ҫ��������������8 bit
UINT8 Car_On_Sign[6][8] ={{0}}; //��¼ĳ�������г�״̬����ʱ��
time_t lastTime[6][8] = {{0}};
IS_NEED_DATA vechileData[12][4];           //�źŻ���������,�ĸ�����
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;//�����������
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
	canits_set_bitrate("can0", 600000);
	INFO("set can0 bitrate 600K!!!\n");
	canits_can_start("can0");
	canits_create_recv_thread();
    	canits_create_socket_can_send();
}

void canits_send(struct can_frame *pcanfram)
{
	int ret = write(g_fd_send, pcanfram, sizeof(struct can_frame));
	if (ret == -1)
	{
		//ERR("canits_send error!\n");
	}
}

void parse_lamp_voltcur(struct can_frame *pframe)
{
	//int i = 0;
	pthread_mutex_lock(&g_MutexLamp);
	//��ѹ
	int boardNo = pframe->can_id&0x7;
	if (boardNo<1 || boardNo>6)
	{
		pthread_mutex_unlock(&g_MutexLamp);
		return;
	}
	g_volt[boardNo-1] =  (pframe->can_id>>3)&0x3ffff;
	//DBG("boardNo:%d, g_volt:%d\n", boardNo, g_volt[boardNo-1]);
	//����
	memcpy(g_cur[boardNo-1], pframe->data, 6);
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
	//int i = 0;
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
			DBG("frame.can_id=%x\n", frame.can_id&0x3fffff);
			
			if ((frame.can_id&0x3fffff) > 0x200000 && ((frame.can_id>>21)&0x1) == 1)//�ƿذ��������ѹ����
			{
#if 0
				INFO("��������ѹ����:\n");
				for (i = 0; i < frame.can_dlc; i++)
				{
					INFO("dlc=%d,data[%d]=%x\n", frame.can_dlc, i,frame.data[i]);
				}
#endif
				parse_lamp_voltcur(&frame);
			}
			//�������Ϣ����
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))
			{
                if(1 == gStructBinfileConfigPara.cCarDetectSwitch)
                {
				    DBG("�������Ϣ����:can_id=0x%x\n",frame.can_id);
				    save_Date_From_Vechile(&frame);//���������Ϣ
				}
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
	{
		return;
	}
	unsigned short *p = poutLamp;
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 8;
	m_frame_send.data[0] = 1;
	m_frame_send.data[1] = p[0]&0xff;
	m_frame_send.data[2] = (((p[1]&0xf)<<4) | ((p[0]>>8)&0xf));
	m_frame_send.data[3] = (p[1]>>4)&0xff;
	m_frame_send.data[4] = p[2]&0xff;
	m_frame_send.data[5] = (((p[3]&0xf)<<4) | ((p[2]>>8)&0xf));
	m_frame_send.data[6] = (p[3]>>4)&0xff;
	m_frame_send.data[7] = p[4]&0x3f;
	canits_send(&m_frame_send);
	
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 7;
	m_frame_send.data[0] = 2;
	m_frame_send.data[1] = (((p[5]&0x3)<<6) | ((p[4]>>6)&0x3f));
	m_frame_send.data[2] = (p[5]>>2)&0xff;
	m_frame_send.data[3] = (((p[6]&0x3f)<<2) | ((p[5]>>10)&0x3));
	m_frame_send.data[4] = (((p[7]&0x3)<<6) | ((p[6]>>6)&0x3f));
	m_frame_send.data[5] = (p[7]>>2)&0xff;
	m_frame_send.data[6] = (p[7]>>10)&0x3;
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
		*pboardInfo = (((g_volt[1]&0x3f)<<6) | ((g_volt[0]>>12)&0x3f));
		break;
	case 3:
		*pboardInfo = (g_volt[1]>>6)&0xfff;
		break;
	case 4:
		*pboardInfo = g_volt[2]&0xfff;
		break;
	case 5:
		*pboardInfo = (((g_volt[3]&0x3f)<<6) | ((g_volt[2]>>12)&0x3f));
		break;
	case 6:
		*pboardInfo = (g_volt[3]>>6)&0xfff;
		break;
	case 7:
		*pboardInfo = g_volt[4]&0xfff;
		break;
	case 8:
		*pboardInfo = (((g_volt[5]&0x3f)<<6) | ((g_volt[4]>>12)&0x3f));
		break;
	default:
		break;
	} 
	pthread_mutex_unlock(&g_MutexLamp);
}

//��ȡ����
unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen)
{
	unsigned short curinfo = 0;
	unsigned char *p = g_cur[0];
	pthread_mutex_lock(&g_MutexLamp);
	curinfo = *(p + (boardNum-1)*4 + pahseNum - 1); //���� p[(boardNum-1)*4 + pahseNum-1];
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
		ERR("video detect recv thread creat failure!!\n");
	else
		INFO("video detect recv thread creat successful!\n");
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
	#ifdef KAKOU
	int i = 0;
	int j = 0;
	#endif
	int retval = -1;			//����ֵ
	int reuse0=1;
	struct timeval timeout;	//���ճ�ʱ
	
	/*�����������׽���*/
ReStart:
	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	//printf("server_sockfd=%d\n",server_sockfd);
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
		//ERR("sockef bind failed!!\n");
		close(server_sockfd);
		goto ReStart;
	}
	else
	{
        INFO("sockef bind Success!!\n");
	}
	/*��������*/
	retval = listen(server_sockfd,5);
	if(retval < 0)
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
		#ifdef KAKOU
		/*�Խӿ��ڵ�ʱ�����øù���*/
		/*�жϳ������г�״̬����ʱ�䣬����ʱ���ж�Ϊ�����뿪*/
		for(i=0;i<6;i++)
		{
			for(j=0;j<8;j++)
			{
				if((sendData[i]>>j) & 0x01)
				{
					if(0 == Car_On_Sign[i][j])
					{
						Car_On_Sign[i][j] = 1;
						lastTime[i][j] = time(NULL);
					}
					
					if (abs(time(NULL) - lastTime[i][j]) >= 3)
					{
						sendData[i] = sendData[i] & (~(1<< j));//���㣬�����뿪
						Car_On_Sign[i][j] = 0;
						INFO("�������볬ʱ\n");
					}
				}
				else
				{
					Car_On_Sign[i][j] = 0;
				}
			}
		}
		#endif
		/*���տͻ�������*/
		DBG("****************while**************\n");
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
		DBG("client_sockfd=%d\n",client_sockfd);
		if(client_sockfd>=0)
		{
			/*���������*/
			DBG("****************recv**************\n");
			recv_from_camera(&client_sockfd);//���ݽ����봦��
			//pthread_create(&recv_camera_thread,NULL,(void*)recv_from_camera,&client_sockfd);//�����߳�
		}
		else
		{
			DBG("client_sockfd creat error!\n");
			continue;
		}
	}
	return 0;	
}

/*��Ƶ���������պ���*/
void recv_from_camera(int *client_fd)
{
	INTER_DVR_REQUEST_HEAD_V30 alarmuphead;
    	NET_TPS_ALARM alarmParam;
	NET_DVR_TPS_REAL_TIME_ALARM realTimeAlarm;
	char recvtmpbuf[BUFFLEN] = {0};
	int retval = -1;
	long leftlen = 0;
	
	memset(&alarmuphead, 0, sizeof(INTER_DVR_REQUEST_HEAD_V30));
	memset(&alarmParam, 0, sizeof(NET_TPS_ALARM));
   	memset(&realTimeAlarm, 0, sizeof(NET_DVR_TPS_REAL_TIME_ALARM));
	
	/*��һ�ν���ͷ����*/
	if(recv(*client_fd, (INT8*)&alarmuphead, sizeof(INTER_DVR_REQUEST_HEAD_V30), 0)<=0)
	{
		ERR("recv failed!!\n");
	    	close(*client_fd);
	    	return ;
	} 
	//DBG("dwLength:%ld,byCommand=0x%x\n",alarmuphead.dwLength,alarmuphead.byCommand);//���Ȳ���������ת��
	if(alarmuphead.dwLength> sizeof(INTER_DVR_REQUEST_HEAD_V30))
	{
		leftlen = alarmuphead.dwLength - sizeof(INTER_DVR_REQUEST_HEAD_V30);
		//DBG("leftlen(%ld)=dwLength(%ld)-sizeof(INTER_DVR_REQUEST_HEAD_V30)(%ld)\n",leftlen,alarmuphead.dwLength,sizeof(INTER_DVR_REQUEST_HEAD_V30));
		DBG("sizeof(NET_TPS_ALARM)=%d,sizeof(NET_DVR_TPS_REAL_TIME_ALARM)=%d\n",sizeof(NET_TPS_ALARM),sizeof(NET_DVR_TPS_REAL_TIME_ALARM));
		/*�ڶ��ν��յ����ݰ�*/
		retval = recv(*client_fd,recvtmpbuf , leftlen, 0);
		if(retval<=0)
		{
			DBG("recv again failed!\n");
		}
		else
		{
			if(0x3a == alarmuphead.byCommand)
    		{
    			/* ���1.0Э��:�ѽ��յ������ݸ�ֵ���ṹ��*/
    			memcpy(&alarmParam, recvtmpbuf, sizeof(NET_TPS_ALARM));
    			deal_data_from_video_v10(&alarmParam);/* ���ݴ���*/
    		}
    		else if(0xb6 == alarmuphead.byCommand)
    		{
    			/*���2.0Э��:�ѽ��յ������ݸ�ֵ���ṹ��*/
    			memcpy(&realTimeAlarm, recvtmpbuf, sizeof(NET_DVR_TPS_REAL_TIME_ALARM));
    			deal_data_from_video_v20(&realTimeAlarm);
    		}
		}
   	 }
	else
	{
		ERR("recv Length failed!\n");
	}
	close(*client_fd);/*�ر��׽��֣��߳̽���*/
}
//��Ƶ���������ݴ���(Э��v1.0)
void  deal_data_from_video_v10(NET_TPS_ALARM  *alarmData)
{
    	int i = 0;
    	UINT32 iCameraId = 0;
    
	if(NULL == alarmData)
    	{
	        ERR("error:alarmData is NULL\n");
	        return;
    	}
	/*һ��������ĸ�����������Ч*/
	if((0 < ntohl(alarmData->dwDeviceId)) && (ntohl(alarmData->dwDeviceId) < 100 ))
	{
        	iCameraId = ntohl(alarmData->dwDeviceId);//����������
	}
	else
    	{
		ERR("v10 recv CameraID %d error\n",ntohl(alarmData->dwDeviceId));
		return;
    	}
	
	for(i=0;i<4;i++)
	{
		vechileData[iCameraId-1][i].varyType = ntohl(alarmData->struTPSInfo.struLaneParam[i].dwVaryType);
		vechileData[iCameraId-1][i].ruleID = alarmData->struTPSInfo.struLaneParam[i].byRuleID;//���ûת������
		DBG("V1.0:lane[%d] VaryType=0x%x,cameraID=%d,byRuleID=%d\n",i,
		vechileData[iCameraId-1][i].varyType,iCameraId,vechileData[iCameraId-1][i].ruleID);
    }
    data_send_by_can(1,0,iCameraId);//���ݴ�����ͨ��CAN����(��һ������Э��1.0���ڶ�����������)
}

//��Ƶ���������ݴ���(Э��v2.0)
void  deal_data_from_video_v20(NET_DVR_TPS_REAL_TIME_ALARM *realTimeData)
{
    int iLane = 0;//������
    UINT32 iCameraId = 0;
    if(NULL == realTimeData)
    {
        ERR("error:realTimeData is NULL\n");
        return;
    }
    
    if((0 < realTimeData->struTPSRealTimeInfo.byLane) &&(realTimeData->struTPSRealTimeInfo.byLane<50))
    {
		iLane = realTimeData->struTPSRealTimeInfo.byLane;//������
    }
    else
    {
		ERR("v20 recv iLane %d error\n",realTimeData->struTPSRealTimeInfo.byLane);
		return;
    }
    if((0 < ntohs(realTimeData->struTPSRealTimeInfo.wDeviceID)) && (ntohs(realTimeData->struTPSRealTimeInfo.wDeviceID) < 90 ))
    {
        	iCameraId = ntohs(realTimeData->struTPSRealTimeInfo.wDeviceID);
    }
    else
    {
		ERR("v20 recv CameraID %d error\n",ntohs(realTimeData->struTPSRealTimeInfo.wDeviceID));
		return;
    }
    
    vechileData[1][1].varyType = realTimeData->struTPSRealTimeInfo.byCMD;
    vechileData[1][1].ruleID = realTimeData->struTPSRealTimeInfo.byLane;
  
    DBG("V2.0:byCMD:%d,wDeviceID:%d,byLane:%d\n",vechileData[iCameraId-1][iLane-1].varyType,
    iCameraId,vechileData[iCameraId-1][iLane-1].ruleID);

    data_send_by_can(2,iLane,iCameraId);//���ݴ�����ͨ��CAN����(��һ������Э��1.0���ڶ�����������-1)
}

void data_send_by_can(char iProtocol,int iLan,UINT32 iCamId)
{
    	int i = 0;
    	int dateType = 0;                   //��������(�Ƿ���Ҫ����)
	UINT16 boardID = 0;                 //�������ֳ����
	int deviceID = 0;                   //λ�ڵ�Ƭ��(0,1,2,3,4,5)
	int deviceType = 0;                 //�ߵ�λ(����λ������λ0,4)
	int iAmount = 0;                    //��������(Э��1.0�ĸ���Э��2.0һ��)
	struct can_frame m_video_send;
    	/*�ж������ź�����*/
    	if(1 == iProtocol)
    	{
        	iAmount = 4;
        	i = 0;
    	}
    	else if(2 == iProtocol)
    	{
       		 iAmount = iLan;
       		 i = iLan - 1;
    	}
	for(;i<iAmount;i++)
	{
       	//����λ���ĸ���Ƭ��
       	deviceID = (iLan - 1)/8;
		
       	//����λ�ڵ�Ƭ���ĸߵ�λ
	   	if(0 == (iLan - (deviceID * 8))/4)
	   	{
           	deviceType = LOW_4bits;
	   	}
	   	else if(0 < (iLan - (deviceID * 8))/4)
	   	{
           	deviceType = TALL_4bits;
	   	}
       	//���ֳ����
    	if((iLan <= 24) && (iLan > 0))
    	{
    		boardID = 0x205;//��һ�鳵���
    	}
    	else if((iLan > 24) && (iLan <= 48))
    	{
    		boardID = 0x206;//�ڶ��鳵���
    	}
    	DBG("deviceID = %d,deviceType = %d,boardID = 0x%x,byLane = %d\n",deviceID,deviceType,boardID,vechileData[iCamId-1][i].ruleID);
    	//״̬����������֮һ��������Ϊ��Ҫ����
    	if(2 == iProtocol)
    	{
			if(ENUM_TRAFFIC_VARY_VEHICLE_ENTER == vechileData[1][1].varyType)
	    	{
	    		dateType = IsNeedSend;
	    		sendData[deviceID] = sendData[deviceID] | (1<<(iLan - (deviceID * 8) - 1));//������Ϊ0x01,����������1
	    	}
	    	else if(ENUM_TRAFFIC_VARY_VEHICLE_LEAVE == vechileData[1][1].varyType)
	    	{
	    		dateType = IsNeedSend;
	    		sendData[deviceID] = sendData[deviceID] & (~(1<<(iLan - (deviceID * 8) - 1)));//������Ϊ0x02,�����뿪��0
	    	}
		}
		else if(1 == iProtocol)
		{
			if(ENUM_TRAFFIC_VARY_VEHICLE_ENTER == vechileData[iCamId-1][i].varyType)
	    	{
	    		dateType = IsNeedSend;
	    		sendData[deviceID] = sendData[deviceID] | (1<<(i+deviceType));//������Ϊ0x01,����������1
	    	}
	    	else if(ENUM_TRAFFIC_VARY_VEHICLE_LEAVE == vechileData[iCamId-1][i].varyType)
	    	{
	    		dateType = IsNeedSend;
	    		sendData[deviceID] = sendData[deviceID] & (~(1<<(i+deviceType)));//������Ϊ0x02,�����뿪��0
	    	}
		}
    	
	}
	
	if(dateType)
	{
		dateType = NoNeedSend;                  //״̬����Ϊ����Ҫ����
		memset(&m_video_send, 0 , sizeof(struct can_frame));
		m_video_send.can_id = boardID;          //ID����Ϊ0x203������Ƶ ����
		m_video_send.can_dlc = 3;
		m_video_send.data[0] = deviceID;        //�������ֶ�Ӧ�ĳ��쵥Ƭ��
		m_video_send.data[1] = deviceType;      //�������ָߵ�λ
		m_video_send.data[2] = sendData[deviceID];
		//����������ͨ��CAN���ݷ��͵�����壬��������ֱ���ϴ������س���
		if(1 == gStructBinfileConfigPara.cCarDetectSwitch)
		{
    			DBG("****send***\nboardID:0x%x,send.can_id=0x%x,deviceID:%d,deviceType=%d,sendData[%d]:0x%x\n",boardID,m_video_send.can_id,deviceID,deviceType,deviceID,sendData[deviceID]);
		    	canits_send(&m_video_send);
		}
		else
		{
		    	pthread_mutex_lock(&count_lock);
    			boardInfoFromInput[deviceID] = sendData[deviceID];//�������ֱ���ϴ����س���
    			DBG("###TEST1###boardInfoFromInput[%d] = %d\n",deviceID,boardInfoFromInput[deviceID] );
    			pthread_mutex_unlock(&count_lock);
		}
		
	}
}

