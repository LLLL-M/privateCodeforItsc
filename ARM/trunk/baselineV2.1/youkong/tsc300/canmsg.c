#include <stdio.h>
#include "canmsg.h"
#include <time.h>
#include <errno.h>
//#include "common.h"
#include "io_ioctl.h"
#include "hik.h"
#include "its.h"
#include "ykconfig.h"

#define IFUSE_VECHILE	 1	/* ���ݲ�ͨ�������*/
static int g_fd_send = 0;  //CAN�����׽���
static int g_fd_recv = 0;  //CAN�����׽���
//�ƿذ��ѹ������������
pthread_mutex_t g_MutexLamp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ped_lock = PTHREAD_MUTEX_INITIALIZER;

#define BOARD_NUM	6
unsigned long   g_volt[BOARD_NUM] = {0}; 	//�ƿص�ѹ
unsigned char   g_cur[BOARD_NUM][BOARD_NUM] = {{0}};	//�ƿص���
unsigned short boardInfoFromInput[6] = {0}; //������������
unsigned short boardInfoSend[3] = {0};           //������������鷢��
unsigned short signForSendVechileDataRequire = 0;//�������������(0�򳵼��1���ͣ�1�򳵼��2����)
UINT8 sendData[6] = {0x00};  /* ��Ҫ��������������8 bit*/

IS_NEED_DATA vechileData[12][4];           //�źŻ���������,�ĸ�����
UINT8 Car_On_Sign[6][8] ={{0}}; //��¼ĳ�������г�״̬����ʱ��
time_t lastTime[6][8] = {{0}};
unsigned short iPedOnState = 0;
UINT8 recvPedData[4];//���յ�����������

extern int g_io_fd;
extern YK_Config *gSignalControlpara;
//extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;//�����������
static UInt8 gCarDetectSwitch = 0;	//�����������0:������,1:����
//extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //���Ӳ���
static UINT8 gIsCanRestartHiktscAllowed = 0;

pthread_t g_thread_can_monitor = 0;	
pthread_t g_thread_can_recv = 0;

unsigned long long gCanRecvCount = 0;

void CanRestart();
void canits_can_start(const char *name);


#ifdef GB_TEST_DETECTOR
#include "../libits/gb.h"

extern GbConfig *gGbconfig;
STRU_DETECTOR_DATA gDetectorData;
STRU_DETECTOR_STATUS gDetectorStatus;
UINT8 gDetectorId = 0;

//val1 ��ȫ�ֱ���, val2�ǵ�ǰֵ
void UpdateDetectorAlarmStatusRecord(UINT8 val1,UINT8 val2)
{
    UINT8 i = 0;

    for(i = 0 ; i < 8; i++)
    {
        if(((val1 >> i)&0x1) == 0 && ((val2 >> i)&0x1) == 1)
        {
            switch(i)
            {
                case 0: GbWriteEventFile(DETECTOR_NOT_ACTIVITY_EVENT);break;
                case 1: GbWriteEventFile(DETECTOR_EXIST_TIMEOUT_EVENT);break;
                case 2: GbWriteEventFile(DETECTOR_NOT_STABLE_EVENT);break;
                case 3: GbWriteEventFile(DETECTOR_COMMUNICATION_FAULT_EVENT);break;
                case 4: GbWriteEventFile(DETECTOR_CONFIG_FAULT_EVENT);break;
                case 7: GbWriteEventFile(DETECTOR_UNKNOWN_FAULT_EVENT);break;
                default: GbWriteEventFile(DETECTOR_NOT_ACTIVITY_EVENT);break;
            }
        }
    }
}
//val1 ��ȫ�ֱ���, val2�ǵ�ǰֵ
void UpdateCoilAlarmStatusRecord(UINT8 val1,UINT8 val2)
{
    UINT8 i = 0;

    for(i = 0 ; i < 8; i++)
    {
        if(((val1 >> i)&0x1) == 0 && ((val2 >> i)&0x1) == 1)
        {
            switch(i)
            {
                case 0: GbWriteEventFile(INDUCTOR_COIL_UNKNOWN_FAULT_EVENT);break;
                case 1: GbWriteEventFile(INDUCTOR_COIL_WATCHDOG_FAULT_EVENT);break;
                case 2: GbWriteEventFile(INDUCTOR_COIL_OPEN_EVENT);break;
                case 3: GbWriteEventFile(INDUCTOR_COIL_INSUFFICIENT_EVENT);break;
                case 4: GbWriteEventFile(INDUCTOR_COIL_BEYOND_EVENT);break;
                default: GbWriteEventFile(INDUCTOR_COIL_UNKNOWN_FAULT_EVENT);break;
            }
        }
    }
}

void UpdateGbVehDetectorAlarmList(STRU_DETECTOR_STATUS *data)
{
    if(!data)
    {
        return;
    }
    UINT8 temp = 0;
    UINT8 i = 0;
    
    GbVehDetectorAlarmList *list = (gGbconfig->vehDetectorAlarmTable + data->nDetectorId - 1);
    gDetectorId= data->nDetectorId;

    for(i = 0; i < 4; i++)
    {
        list->detectorNo = data->nDetectorId+i;
        temp = (data->noLive | (data->liveLong << 1) | (data->unStable << 2) | (data->commErr << 3) | (data->cfgErr << 4) | (data->unKown << 7));
        UpdateDetectorAlarmStatusRecord(list->detectorAlarmStatus,temp);
        list->detectorAlarmStatus = temp;

        temp = 0;
        temp = (data->nOther | (data->nWatchdog << 1) | (data->nOpen << 2) | (data->nLow << 3) | (data->nHigh << 4));
        UpdateCoilAlarmStatusRecord(list->inductiveCoilAlarmStatus,temp);
        list->inductiveCoilAlarmStatus = temp;

        if(list->detectorAlarmStatus != 0 || list->inductiveCoilAlarmStatus != 0)
        {
            gGbconfig->detectorStatusTable[(list->detectorNo -1 )/ 8].detectorStatusNo = (list->detectorNo -1 ) / 8;
            gGbconfig->detectorStatusTable[(list->detectorNo -1 ) / 8].detectorStatusAlarm |= (1 << (list->detectorNo - 1)%8);

            gGbconfig->deviceAlarmSummary |= (1 << 5);
            
            
        }
        else
        {
            gGbconfig->detectorStatusTable[(list->detectorNo -1 ) / 8].detectorStatusAlarm &= ~(1 << (list->detectorNo - 1)%8);
            gGbconfig->deviceAlarmSummary &= ~(1 << 5);
        }

        list += 1;
    }
}

void UpdateGbTrafficDetectDataList(STRU_DETECTOR_DATA *data)
{
    if(!data)
    {
        return;
    }

    UINT8 temp = 0;
    UINT8 i = 0;
    GbTrafficDetectDataList *list = (gGbconfig->trafficDetectDataTable + data->nDetectorId - 1);

    for(i = 0; i < 4; i++)
    {
        list->detectorNo = data->nDetectorId+i;
        list->totalFlow = data->nTotalFlow;
        list->largeVehFlow = data->nLargeFlow;
        list->smallVehFlow = data->nSmallFlow;
        list->rate = data->nPercent;
        list->speed = data->nSpeed;
        list->vehBodyLen = data->nLength;

        list += 1;
    }
    if(gDetectorId != 0)
    {
        gGbconfig->activeDetectorNum = 4;
    }


}

#endif

void ClearGbDetectorStatusList()
{
#ifdef GB_TEST_DETECTOR
    UINT8 i = 0;
    for(i = 0; i < 4; i++)
    {
        gGbconfig->detectorStatusTable[(gDetectorId+i -1 ) / 8].detectorStatus &= ~(1 << (gDetectorId+i - 1)%8);
        gGbconfig->detectorStatusTable[(gDetectorId+i -1 ) / 8].detectorStatusAlarm &= ~(1 << (gDetectorId+i - 1)%8);
        gGbconfig->deviceAlarmSummary &= ~(1 << 5);
        memset(&gGbconfig->trafficDetectDataTable[gDetectorId+i - 1],0,sizeof(GbTrafficDetectDataList));
        memset(&gGbconfig->vehDetectorAlarmTable[gDetectorId+i - 1],0,sizeof(GbVehDetectorAlarmList));
    
    }
    
    gGbconfig->activeDetectorNum = 0;
#endif
}


/*����źŻ����й�����CAN�ҵ��Ľ������:
*
*1. ����һ���߳�thread_monitor_can_recvר����������Ƿ��յ�������ѹ��Ϣ.
*
*2. �������2s��û���յ���Ϣ������Ϊû�нӵƿذ��CANͨѶ�쳣������������CAN��
*
*3. ���������������CAN��û�лָ���������ִ��ifconfig can0 down
*
*4. ���ִ��ifconfig can0 downҲû��Ч�����͸���ȫ�ֲ�������������hikTSC
*
*/

//ִ��ifconfig can0 down
void ShutDownCan0()
{
    INFO("ShutDownCan0");
    system("ifconfig can0 down");
    sleep(5);
}

//hikTSC����
void HikTSCRestart()
{
    INFO("hikTSC will restart in 10s");
    sleep(10);
#if 0    
    system("echo \"#!/bin/sh \" > /root/restart.sh\n");
    system("echo \"cd /root/ \" >> /root/restart.sh\n");
	system("echo \"killall hikTSC\" >> /root/restart.sh\n");
	system("echo \"sleep 1\" >> /root/restart.sh\n");
	system("echo \"/root/hikTSC &\" >> /root/restart.sh\n");
	system("chmod 777 /root/restart.sh\n");
	system("/root/restart.sh\n");
#endif
}

//��������Ƿ��Ѿ���ȡ��������ѹ��Ϣ���������5sû���յ����Կذ��CAN��Ϣ��������CAN������CAN�ķ�ʽ��ifcon can0 down
void *thread_monitor_can_recv()
{
    unsigned long long nTempValue = 0;
    unsigned char nErrorCount = 0;
	INFO("thread_monitor_can_recv thread start ...");
    while(1)
    {
        nTempValue = gCanRecvCount;
        sleep(2);
        if(nTempValue == gCanRecvCount)//�������2���ӣ�CAN���ռ�����û�б仯����˵��CAN�п��ܹ����ˣ�������֮
        {
            log_error("I will restart can0 .");
            nErrorCount++;
            if(nErrorCount <= 2)
            {
                CanRestart();
            }
            else if(nErrorCount <= 4)
            {
                ShutDownCan0();
            }
            else
            {
                if(gIsCanRestartHiktscAllowed == 1)//Ĭ��������ǲ������������ģ������������ļ��н������á�
                {   
                    HikTSCRestart();
                }  
                else
                {
                    log_error("Can may be still error , unless you reboot system .");
                    nErrorCount = 0;
                }
            }
        }
        else
        {
            nErrorCount = 0;
        }
    }

    return NULL;
}

//�ڶ�������CAN���Լ�ʵ�ֵ�
void CanRestart()
{
#ifndef FOR_SERVER
    pthread_cancel(g_thread_can_recv);//shut down the thread for recving
    shutdown(g_fd_recv,SHUT_RDWR);//shut down the fd for recving
    shutdown(g_fd_send,SHUT_RDWR);//shut down the fd for sending
	close(g_fd_recv);	//������ִ��shutdown���ٴ�close������fd�ᱻһֱռ��,���ճ���ϵͳ���fd��������
	close(g_fd_send);
    //sleep(1);
	can_do_stop("can0");
	//sleep(1);
	canits_set_bitrate("can0", 500000);
	INFO("set can0 bitrate 500K!!!\n");
	canits_can_start("can0");
	//sleep(1);
	canits_create_recv_thread();
	//sleep(1);
    canits_create_socket_can_send();
    
    INFO("Can Restated ....   ===>      <====\n");
#endif
}



void canits_set_bitrate(const char *name, unsigned long bitrate)
{
#ifndef FOR_SERVER
	int err = can_set_bitrate(name, bitrate);
	if (err < 0)
	{
		ERR("set %s bitrate(%lu) failed\n", name, bitrate);
	}
#endif	
}

void canits_can_start(const char *name)
{
#ifndef FOR_SERVER
	if (can_do_start(name) < 0)
	{
		ERR("%s start failed\n", name);
	}
#endif
}

void canits_create_recv_thread()
{
	pthread_t thread;
	if (0 != pthread_create(&g_thread_can_recv, NULL, canits_recv_thread, NULL))
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
#ifndef FOR_SERVER

	can_do_stop("can0");
	canits_set_bitrate("can0", 500000);
	INFO("set can0 bitrate 500K!!!\n");
	canits_can_start("can0");
	canits_create_recv_thread();
    canits_create_socket_can_send();
    pthread_create(&g_thread_can_monitor, NULL, thread_monitor_can_recv, NULL);
#endif
}

void canits_send(struct can_frame *pcanfram)
{
	int ret = write(g_fd_send, pcanfram, sizeof(struct can_frame));
	if (ret == -1)
	{
		//ERR("canits_send error, error info:%s\n", strerror(errno));
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
	g_fd_recv = socket(AF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, "can0");
	ioctl(g_fd_recv, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	bind(g_fd_recv, (struct sockaddr *)&addr, sizeof(addr));
	struct can_frame frame;
	while (1)
	{
		pthread_testcancel();
		if (read(g_fd_recv, &frame, sizeof(struct can_frame)) > 0)
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
				gCanRecvCount++;
			}
			//�������Ϣ����
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))
			{
				DBG("�������Ϣ����:can_id=0x%x\n",frame.can_id);
				save_Date_From_Vechile(&frame);//���������Ϣ
			}
			//ǰ���5������״̬
			else if(frame.can_id == 0x401 && frame.can_dlc == 1) {
				INFO("recv foreboard info, data:%#x", frame.data[0]);
				if ((g_io_fd != -1) && (frame.data[0] & 0x1f) != 0)
					ioctl(g_io_fd, IO_SET_BUTTON_STATUS, &frame.data[0]);
			}
		}
		pthread_testcancel();
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
	m_frame_send.data[0] = gSignalControlpara->wholeConfig.signalMachineType & 0x3;
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
		*pboardInfo = 0; break;
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
			/*���������*/
			//INFO("****************recv**************\n");
			recv_from_camera(&client_sockfd);//���ݽ����봦��
			//pthread_create(&recv_camera_thread,NULL,(void*)recv_from_camera,&client_sockfd);//�����߳�
		}
		else
		{
			//ERR("client_sockfd creat error!\n");
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
	if((retval = recv(*client_fd, (INT8*)&alarmuphead, sizeof(INTER_DVR_REQUEST_HEAD_V30), 0)) <= 0)
	{
		ERR("recv failed!!\n");
    	close(*client_fd);
    	return ;
	} 
#ifdef GB_TEST_DETECTOR
    if(retval == 1)
    {
        ClearGbDetectorStatusList();
    	close(*client_fd);
    	return ;    
    }
#endif
    UINT8 xx = 0;
	DBG("dwLength:%ld,byCommand=0x%x\n",alarmuphead.dwLength,alarmuphead.byCommand);//���Ȳ���������ת��
	if(alarmuphead.dwLength> sizeof(INTER_DVR_REQUEST_HEAD_V30))
	{
		leftlen = alarmuphead.dwLength - sizeof(INTER_DVR_REQUEST_HEAD_V30);
		DBG("leftlen(%ld)=dwLength(%ld)-sizeof(INTER_DVR_REQUEST_HEAD_V30)(%ld)\n",leftlen,alarmuphead.dwLength,sizeof(INTER_DVR_REQUEST_HEAD_V30));
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
#ifdef GB_TEST_DETECTOR
                if(recv(*client_fd,&gDetectorData,sizeof(gDetectorData),0) <= 0)
                {
        	    	close(*client_fd);
        	    	return ;                    
                }
                if(recv(*client_fd,&gDetectorStatus,sizeof(gDetectorStatus),0) <= 0)
                {
        	    	close(*client_fd);
        	    	return ;                    
                }

                UpdateGbTrafficDetectDataList(&gDetectorData);
                UpdateGbVehDetectorAlarmList(&gDetectorStatus);

                for(xx = 0; xx < 4; xx++)
                    gGbconfig->detectorStatusTable[(gDetectorId+xx -1 ) / 8].detectorStatus |= (1 << (gDetectorId+xx - 1)%8);
#endif    		
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
  
//   	INFO("V2.0:byCMD:%d,wDeviceID:%d,byLane:%d\n",vechileData[iCameraId-1][iLane-1].varyType,
//    iCameraId,vechileData[iCameraId-1][iLane-1].ruleID);

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
		if(1 == gCarDetectSwitch)
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

