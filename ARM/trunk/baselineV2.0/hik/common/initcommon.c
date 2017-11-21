
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/watchdog.h>

#include "its.h"
#include "common.h"
#include "configureManagement.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara; 
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void *CanRecvModule(void *arg);
extern void *CameraCommunication(void *arg);
extern void *DataCollectModule(void *arg);
extern void *CommunicationModule(void *arg);
extern void get_all_params_from_config_file();
extern void HardflashDogCtrl(); //Ӳ�������
extern void LoadLocalConfigFile(void *config);


/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
//�źŻ����ò������ṹȫ�ֱ���ָ�룬�������������Ϣ
SignalControllerPara *gSignalControlpara = NULL;
int gOftenPrintFlag = 0;

__attribute__((destructor)) void ProgramExit(void)
{
	ItsExit();
	if (gSignalControlpara != NULL)
	{
		free(gSignalControlpara);
		gSignalControlpara = NULL;
	}
	INFO("The program exit!");
}

static void SigHandle(int sigNum)
{
	switch (sigNum)
	{
		case SIGSEGV:	log_error("This program has sigmentfault"); break;
		case SIGINT:	INFO("This program has been interrupted by ctrl+C"); break;
		case SIGTERM:	INFO("This program has been interrupted by command 'kill' or 'killall'"); break;
		case SIGUSR1: 	gOftenPrintFlag = !gOftenPrintFlag; return;
	}
	exit(1);
}
//���ݵ�ǰʱ���ж��Ƿ���Ҫִ�н���
static Boolean IsExcuteDemotion(UInt8 *demotionSchemeId)
{
	time_t timeNow, timeStart, timeEnd;    
    struct tm start, end;
	DemotionParams *demotionParams = &gStructBinfileCustom.demotionParams;

	timeNow = time(NULL);
	localtime_r(&timeNow, &start);
	memcpy(&end, &start, sizeof(end));
	start.tm_hour = demotionParams->ucBeginTimeHour;
	start.tm_min = demotionParams->ucBeginTimeMin;
	start.tm_sec = 0;//demotionParams->ucBeginTimeSec;
	timeStart = mktime(&start);
	end.tm_hour = demotionParams->ucEndTimeHour;
	end.tm_min = demotionParams->ucEndTimeMin;
	end.tm_sec = 0;//demotionParams->ucEndTimeSec;
	timeEnd = mktime(&end);
	if (((timeStart > timeEnd) && (timeNow < timeStart && timeNow > timeEnd))
			|| ((timeStart < timeEnd) && (timeNow < timeStart || timeNow > timeEnd)))
	{	//��ǰʱ�䲻�����õĽ���ʱ�䷶Χ�ڲ����н���
		return FALSE;	
	}
	else
	{
		*demotionSchemeId = demotionParams->ucWorkingTimeFlag;
		return TRUE;
	}
}
//��������
void *DemotionDeal(void *arg)
{
	char netRunFlag = 1;	//�������б�־,1:���糩ͨ, 0:�����ж�, ��ʼ��Ĭ��Ϊ1
	struct ifreq ifr;
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	UInt8 demotionSchemeId = 0;
	
	if (sockfd == -1) 
	{
		ERR("function[%s] create socket fail, error info: %s\n", __func__, strerror(errno));
		pthread_exit(NULL);
	}
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth1");
	while (1)
	{
		ioctl(sockfd, SIOCGIFFLAGS, &ifr);
		if (ifr.ifr_flags & IFF_RUNNING)
		{
			if (netRunFlag == 0)
			{	//���糩ͨ��ָ���ϵͳ����
				netRunFlag = 1;
				ItsCtl(TOOL_CONTROL, 0, 0);
				log_debug("the network card is recover running!");
				ItsWriteFaultLog(COMMUNICATION_CONNECT, 0);
			}
		}
		else
		{	//��������жϲ��ҵ�ǰʱ�����ִ�н�������ִ�н�������
			if (netRunFlag == 1)
			{
				netRunFlag = 0;
				ItsWriteFaultLog(COMMUNICATION_DISCONNECT, 0);
				if (IsExcuteDemotion(&demotionSchemeId))
				{
					ItsCtl(TOOL_CONTROL, demotionSchemeId, 0);
					log_debug("the network card is disconnect, demotionSchemeId = %d!", demotionSchemeId);
				}
			}
		}
		sleep(1);
	}
	pthread_exit(NULL);
}

static inline void FeedWatchDog(void)
{
	static int fd = -1;
	int food = 0;
	
	if (gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 0)
	{
		if (fd != -1)
		{
			close(fd);
			fd = -1;
			system("watchdog -t 1 -T 3 /dev/watchdog");
		}
		return;
	}
	if (fd == -1)
	{
		system("killall watchdog");
		fd = open("/dev/watchdog", O_WRONLY);
		if (fd == -1)
		{
			ERR("open /dev/watchdog fail, error info: %s", strerror(errno));
			system("watchdog -t 1 -T 3 /dev/watchdog");
		}
	}
	else
		ioctl(fd, WDIOC_KEEPALIVE, &food);
}

static void *HardYellowFlashModule(void *arg)
{
	int i = 0;
	while (1) {
		HardflashDogCtrl();
		usleep(10000);
		if (++i == 10)
		{
			i = 0;
			FeedWatchDog();
		}
	}
	pthread_exit(NULL);
}

extern void ReadFaultStatus();
void InitCommon()
{
	struct threadInfo threads[] = {
#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
		{0, CanRecvModule, "can receive", 0},
		{0, CameraCommunication, "camera communication", 0},
		{0, HardYellowFlashModule, "hard yellow flash", 0},
		{0, DataCollectModule, "data collect", 0},
		{0, DemotionDeal, "demotion", 0},
#endif
		{0, CommunicationModule, "communication", 0},
	};

#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
	ReadFaultStatus();	//��ȡ���ϼ���״̬,���ڻָ�ʹ��
#endif
    gSignalControlpara = (PSignalControllerPara)calloc(1, sizeof(SignalControllerPara));
    if(!gSignalControlpara)
    {
        ERR("error to alloc memory for gSignalControlpara  : %s\n",strerror(errno));
		exit(1);
    }
	fclose(stdout);
	init_db_file_tables();
	ReadBinAllCfgParams();
	TscIpCheck();//��ʼУ���������ip�Ƿ��������ã�������������
	signal(SIGSEGV, SigHandle);
	signal(SIGINT, SigHandle);		//for ctrl + c
	signal(SIGTERM, SigHandle);    //for command 'kill' or 'killall'
	signal(SIGUSR1, SigHandle);	//for OFTEN print

	LoadLocalConfigFile(gSignalControlpara);
	if (TRUE == ItsInit(threads, sizeof(threads) / sizeof(threads[0]), gSignalControlpara, sizeof(SignalControllerPara)))
		log_debug("software version: %s, compile time: %s, %s", SOFTWARE_VERSION_INFO, __DATE__, __TIME__);
}

