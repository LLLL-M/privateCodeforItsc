/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ItsMain.c
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��11��29��
  ����޸�   :
  ��������   : ��Ҫ��ʼ����ͨ��ģ�顢��ʱ��ģ�顢���Ե���ģ��ȵ�
  �����б�   :
              CommunicationModule
              createThread
              getMsgId
              InitGolobalVar
              init_communication_server
              itstaskmain
              ThreadHardYellowLight
              TimerModule
  �޸���ʷ   :
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "common.h"
#include "LogSystem.h"
#include "Util.h"
#include "HikConfig.h"
#include "HikMsg.h"
#include "Database.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define LOG_PATH    "./logDir/"
#define LOG_MAX_SIZE    200//M
#define LOG_MAX_SIZE_EACH   10//M

#define IPPORT	8888
#define SA(x) ((struct sockaddr *)&x)

#define FILE_LEN	(4096 * 16) //64k

#define FAULT_RED_GREEN_CONFLICT    0xb
#define FAULT_RED_GREEN_CLEAR       0xc
#define FAULT_RED_OFF               0xd
#define FAULT_RED_OFF_CLEAR         0xe
#define FAULT_GREEN_CONFLICT        0xf
#define FAULT_GREEN_CLEAR           0x10
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern struct FAILURE_INFO s_failure_info; 
extern struct SPECIAL_PARAMS s_special_params;
typedef unsigned short  uint16;
extern int PhaseLampOutput(int boardNum, uint16 outLamp); 
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void HardflashDogOutput(void);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
int msgid;
//�źŻ����ò������ṹȫ�ֱ���ָ�룬�������������Ϣ
SignalControllerPara *gSignalControlpara = NULL;
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : getMsgId
 ��������  : ����һ����Ϣ���в�����msgid������Ѿ�������ֱ�ӷ����Ѿ�������msgid
 �������  : int proj_id  ������һ�����һ���ַ�
 �� �� ֵ  : ���ش����õ���Ϣ���е�msgid
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int getMsgId(int proj_id)
{
	key_t key = ftok("/dev/null", proj_id);
	int id = -1;
	if (key == -1) 
	{
		return -1;
	}
	id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
	if (id == -1 && errno == EEXIST) 
	{
		id = msgget(key, 0666);
	}
	return id;
}


/*****************************************************************************
 �� �� ��  : InitGolobalVar
 ��������  : ��ʼ��ȫ�ֲ�������־���������ò�����CANͨ��
 �������  : ��
 �� �� ֵ  : 

 �޸���ʷ      :
  1.��    ��   : 2014��7��30��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void InitGolobalVar()
{
    //��ʼ��������־ϵͳ        ��PC���Ǵ�,��Ƕ��ʽ�忨��Ĭ���ǹرյġ�
#ifndef ARM_PLATFORM
    //��ʼ��������־ϵͳ
    InitLogSys(LOG_PATH, LOG_MAX_SIZE, LOG_MAX_SIZE_EACH);
#endif

    log_debug("===============System  Start===============\n");

    //��ʼ��ȫ�ֵ��źŻ����ò���
    gSignalControlpara = (PSignalControllerPara)malloc(sizeof(SignalControllerPara));
    if(!gSignalControlpara)
    {
        log_error("error to alloc memory for gSignalControlpara  : %s\n",strerror(errno));
		exit(1);
    }
    memset(gSignalControlpara,0,sizeof(SignalControllerPara));
    
}

/*****************************************************************************
 �� �� ��  : init_communication_server
 ��������  : ͨ��ģ���udp��������ʼ��
 �������  : void  
 �� �� ֵ  : ��ȷ����sockfd�����󷵻�-1
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int init_communication_server(void)
{
	int sockfd;
	struct sockaddr_in addr = 
	{
		.sin_family = PF_INET,
		.sin_port = htons(IPPORT),
		.sin_zero = {0},
	};
	socklen_t len = sizeof(struct sockaddr);
	struct ifreq ifr;
	int opt = 1;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		log_error("create socket fail, error info: %s\n", strerror(errno));
		return -1;
	}
	//��ȡeth0��IP��ַ
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) 
	{
		log_error("get eth0 ip fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	} 
	else 
	{
		addr.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
		log_debug("eth0 ip: %s\n", inet_ntoa(addr.sin_addr));
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(int));
	if (bind(sockfd, SA(addr), len) == -1) 
	{
		log_error("bind socket fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	return sockfd;
}

/*****************************************************************************
 �� �� ��  : CommunicationModule
 ��������  : ͨ��ģ����̴߳���������Ҫ��ʹ��udp�����������ⲿ��������Ϣ��
             Ȼ��ͨ����Ϣ���д�����Ϣ�����Ե���ģ��
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *CommunicationModule(void *arg)
{
	struct msgbuf msg = {MSG_CONFIG_UPDATE, {0}};	//���͸����Կ���ģ�����Ϣ
	int sockfd = init_communication_server();
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr);
	char buf[FILE_LEN];
	ssize_t recvsize, size = sizeof(SignalControllerPara);

	if (sockfd == -1) 
	{
		pthread_exit(NULL);
	}

	LoadDataFromCfg(gSignalControlpara, NULL);	//װ������
	if (IsSignalControlparaLegal(gSignalControlpara) == 0) 
	{	//��������Ƿ����
		msgsnd(msgid, &msg, MSGSIZE, 0);
	}
	//��Gohead������ͨ��
	while (1) 
	{
		memset(buf, 0, FILE_LEN);
		recvsize = recvfrom(sockfd, buf, FILE_LEN, 0, SA(addr), &len);
		if (recvsize >= size) 
		{
			memcpy(gSignalControlpara, buf, size);
			if (IsSignalControlparaLegal(gSignalControlpara) == 0) 
			{	//��������Ƿ����
				msgsnd(msgid, &msg, MSGSIZE, 0);
				WriteConfigFile(gSignalControlpara, NULL);
				log_debug("config file is update!\n");
				break;
			}
		}
	}

	pthread_exit(NULL);
}

/*****************************************************************************
 �� �� ��  : TimerModule
 ��������  : ��ʱ��ģ����̴߳���������Ҫ�������ղ��Ե���ģ��Ķ�ʱ��Ϣ��
             Ȼ��ʱ1s������ٸ�����ģ�鷢�Ͷ�ʱ��ɵ���Ϣ
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *TimerModule(void *arg)
{
	struct msgbuf msg = {MSG_TIMER_START, {0}};
	struct timespec start = {0, 0}, end = {0, 0};
	long gap;
	
	//�������ն�ʱ����������Ϣ
	while (msgrcv(msgid, &msg, MSGSIZE, MSG_TIMER_START, 0) == -1) 
	{
		log_error("timer receive MSG_TIMER_START error:%s\n", strerror(errno));
		usleep(10);
	}
	clock_gettime(CLOCK_MONOTONIC, &start);	//��¼����ʼʱ��
	msg.mtype = MSG_TIMER_COMPLETE;
	while (1)	//ÿDELAY_TIME msѭ��һ��
	{
		usleep(DELAY_TIME_USECS_BEFORE);	//����ʱDELAY_TIME_USECS_BEFORE us,������1ms�ھ�ȷ��ʱ
		while (1)
		{
			usleep(50);	//ÿ����ʱ50us
			clock_gettime(CLOCK_MONOTONIC, &end);
			gap = (end.tv_sec - start.tv_sec) * ONE_SECOND_NSECS + end.tv_nsec - start.tv_nsec;
			if (gap >= DELAY_TIME_NSECS)
			{	//���Ͷ�ʱ�����Ϣ
				msgsnd(msgid, &msg, MSGSIZE, 0);
				start.tv_nsec += DELAY_TIME_NSECS;
				if (start.tv_nsec >= ONE_SECOND_NSECS)
				{
					start.tv_nsec -= ONE_SECOND_NSECS;
					start.tv_sec += 1;
				}
			}
		}
	}
	pthread_exit(NULL);
}

/*****************************************************************************
 �� �� ��  : Light
 ��������  : ���ģ����̴߳���������Ҫ���յ����Ϣ��Ȼ����Ϣ���ݽ��е��
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *Light(void *arg)
{
	struct msgbuf msg;
	unsigned short *group = (unsigned short *)(msg.mtext);
	int i;
	while (1)
	{
		memset(group, 0, MSGSIZE);
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_LIGHT, 0) == -1)
		{
			usleep(10);
			continue;
		}
		for (i = 0; i < NUM_BOARD; i++)
		{
			PhaseLampOutput(i + 1, group[i]);//call the old interface
		}
	}
}
 
/*****************************************************************************
 �� �� ��  : ThreadHardYellowLight
 ��������  : ��������ģ����̴߳�����
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *ThreadHardYellowLight(void *arg)
{

    while(1)
    {
        usleep(300000);
        if(((FAULT_RED_GREEN_CONFLICT == s_failure_info.nID) || (FAULT_GREEN_CONFLICT == s_failure_info.nID)) && (s_special_params.iVoltageAlarmAndProcessSwitch == 1))
        {
            log_debug("%s   Id  %d\n",__func__,s_failure_info.nID);
            continue;
        }

        if((FAULT_RED_OFF == s_failure_info.nID) && (s_special_params.iCurrentAlarmAndProcessSwitch == 1))
        {
            log_debug("%s   Id  %d\n",__func__,s_failure_info.nID);
            continue;
        }

        HardflashDogOutput();

    }

    return NULL;
}

/*****************************************************************************
 �� �� ��  : createThread
 ��������  : �̴߳�����Ҫ���õĺ�������Ҫ��������һ���߳�
 �������  : pthread_t *id            �̺߳ŵ�ָ�룬�����ɹ����̺߳Ż�ͨ��ָ�뷵��
             void *(*fun)(void *arg)  �̵߳Ĵ�����ָ��
             char *moduleName         �̵߳�ģ������
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void createThread(pthread_t *id,
                                void *(*fun)(void *arg),
                                char *moduleName)
{
	if (pthread_create(id, NULL, fun, NULL) == -1) 
	{
		log_error("create %s module fail, error info:%s\n", moduleName, strerror(errno));
		return;
	}
	pthread_detach(*id);
	log_debug("create %s pthread successful!\n", moduleName);
}

/*****************************************************************************
 �� �� ��  : itstaskmain
 ��������  : ��ģ��ĳ�ʼ��������ڣ���Ҫ������ʼ������ģ����߳�
 �������  : int argc      
             char *argv[]  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
int itstaskmain(int argc, char *argv[])
{
    InitGolobalVar();

	msgid = getMsgId('a');
	if (msgid == -1) 
	{
		perror("get msgid fail, error info:");
		return -1;
	}
	log_debug("msgid = %d\n", msgid);

	pthread_t communication, timer, signalRun;
	pthread_t light;
	pthread_t t_database;
    pthread_t t_hard_yellow;

	//����ͨ��ģ��
	createThread(&communication, CommunicationModule, "communication");
	//������ʱ��ģ��
	createThread(&timer, TimerModule, "timer");
	//�����źŻ�����ģ��
	createThread(&signalRun, SignalControllerRun, "signalRun");
	//�������ģ��
	createThread(&light, Light, "light");

    createThread(&t_database,ThreadCheckCfgChanged,"ThreadCheckCfgChanged");

	createThread(&t_hard_yellow, ThreadHardYellowLight,"ThreadHardYellowLight");

	while (1) 
	{
		if (pthread_kill(communication, 0) == ESRCH) 
		{
			createThread(&communication, CommunicationModule, "communication");
		}
		if (pthread_kill(timer, 0) == ESRCH) 
		{
			createThread(&timer, TimerModule, "timer");
		}
		if (pthread_kill(signalRun, 0) == ESRCH) 
		{
			createThread(&signalRun, SignalControllerRun, "signalRun");
		}
		if (pthread_kill(light, 0) == ESRCH) 
		{
			createThread(&light, Light, "light");
		}
		if (pthread_kill(t_database, 0) == ESRCH) 
		{
			createThread(&t_database, ThreadCheckCfgChanged, NULL);
		}		
		if (pthread_kill(t_hard_yellow, 0) == ESRCH) 
		{
			createThread(&t_hard_yellow, ThreadHardYellowLight, NULL);
		}	
		//���ϵͳ����
        system("sync");
        system("echo 1 > /proc/sys/vm/drop_caches");
        system("echo \"0\" >> /proc/sys/vm/overcommit_memory");
		sleep(1);
	}

    return 0;
}
