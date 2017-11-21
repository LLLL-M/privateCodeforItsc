
/*----------------------------------------------*
 * 包含头文件                                   *
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
#include "countDown.h"

#ifdef USE_GB_PROTOCOL
#include "gb.h"
GbConfig *gGbconfig = NULL;					//全局国标配置指针
#endif
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define SA(x) ((struct sockaddr *)&x)
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara; 
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void *DataCollectModule(void *arg);
extern void *CommunicationModule(void *arg);
#ifdef USE_GB_PROTOCOL
extern void *GbUDPCommunicationModule(void *arg);
//extern void *GbTCPCommunicationModule(void *arg);
#endif
extern void get_all_params_from_config_file();
extern void GPS_led_off(void);
extern void HardflashDogCtrl(); //硬黄闪输出
extern void LoadLocalConfigFile(ProtocolType type, void *config);
extern void SendRunInfoTOBoard(LineQueueData *data, SignalControllerPara *para);

static void ChannelControl(void);

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
//信号机配置参数主结构全局变量指针，用来存放配置信息
SignalControllerPara *gSignalControlpara = NULL;
int gOftenPrintFlag = 0;

static inline void GPSInit()
{
#ifdef FOR_SERVER
return;
#endif
	//GPS开关是否打开
	if(gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 0)
	{
		//GPS开关没有打开
		INFO("GPS is disabled!!!\n");
		system("killall -9 GPS");
		GPS_led_off();
	}
	else 
	{
		//GPS开关已经打开
		INFO("GPS is enabled!!!\n");
		system("killall -9 GPS");
		//熄灭GPS指示灯
		GPS_led_off();
		system("/root/GPS &> /dev/null &");
	}
}

__attribute__((destructor)) void ProgramExit(void)
{
	ItsExit();
	if (gSignalControlpara != NULL)
	{
		free(gSignalControlpara);
		gSignalControlpara = NULL;
	}
#ifdef USE_GB_PROTOCOL
	if (gGbconfig != NULL)
	{
		free(gGbconfig);
		gGbconfig = NULL;
	}
#endif
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
//根据当前时间判断是否需要执行降级
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
	{	//当前时间不在设置的降级时间范围内不进行降级
		return FALSE;	
	}
	else
	{
		*demotionSchemeId = demotionParams->ucWorkingTimeFlag;
		return TRUE;
	}
}
//降级处理
void *DemotionDeal(void *arg)
{
	char netRunFlag = 1;	//网络运行标志,1:网络畅通, 0:网络中断, 初始化默认为1
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
			{	//网络畅通后恢复到系统控制
				netRunFlag = 1;
				ItsCtl(TOOL_CONTROL, 0, 0);
				log_debug("the network card is recover running!");
				ItsWriteFaultLog(COMMUNICATION_CONNECT, 0);
			}
		}
		else
		{	//如果网络中断并且当前时间可以执行降级，则执行降级方案
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

//获取已运行网卡的网络字节序ip地址
in_addr_t GetNetworkCardIp(char *interface)
{
	struct ifreq ifreqs[8];
    struct ifconf conf;
    int i, ifc_num;
    struct ifreq *ifr = ifreqs;
    struct in_addr addr = {0};
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	if (sockfd == -1) 
	{
		ERR("create socket fail, error info: %s\n", strerror(errno));
		return addr.s_addr;
	}
	memset(ifr, 0, sizeof(ifreqs));
	if (interface != NULL)
	{
		strcpy(ifr->ifr_name, interface);
		if (ioctl(sockfd, SIOCGIFADDR, ifr) != 0)
			return addr.s_addr;
		addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
		INFO("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
		return addr.s_addr;
	}
	//interface为NULL时，获取正在运行的网卡的ip地址
    conf.ifc_len = sizeof(ifreqs);
    conf.ifc_req = ifreqs;
    if (ioctl(sockfd, SIOCGIFCONF, &conf))
    {   
        ERR("get conf fail, error info: %s\n", strerror(errno));
        return addr.s_addr;
    }   

    ifc_num = conf.ifc_len / sizeof(struct ifreq);
    for (i = 0; i < ifc_num; i++, ifr++)
	{
		if ((strncmp(ifr->ifr_name, "lo", 2) == 0)
            || (strncmp(ifr->ifr_name, "can", 3) == 0)) 
            continue;
		ioctl(sockfd, SIOCGIFFLAGS, ifr);
		if (ifr->ifr_flags & IFF_RUNNING)
		{
			addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
			INFO("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
			break;
		}
	}
	return addr.s_addr;
}

/*****************************************************************************
 函 数 名  : CreateCommunicationSocket
 功能描述  : 通信模块的udp服务器初始化
 输入参数  : char *interface, UInt16 port  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年04月22日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
int CreateCommunicationSocket(char *interface, UInt16 port)
{
	struct sockaddr_in addr = 
	{
		.sin_family = PF_INET,
		.sin_addr = {(interface == NULL) ? INADDR_ANY : GetNetworkCardIp(interface)},
		.sin_port = htons(port),
		.sin_zero = {0},
	};
	socklen_t len = sizeof(struct sockaddr);
	int opt = 1;
	int sockfd = -1;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		ERR("create socket fail, error info: %s\n", strerror(errno));
		return -1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(int));
	if (bind(sockfd, SA(addr), len) == -1) 
	{
		ERR("bind socket fail, error info:%s\n", strerror(errno));
		close(sockfd);
		return -1;
	}
	return sockfd;
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

void ItsLight(int boardNum, unsigned short *poutLamp)
{
	i_can_its_send_led_request(boardNum, poutLamp);
}

extern void ReadFaultStatus();
void InitCommon()
{
	struct threadInfo threads[] = {
		{0, HardYellowFlashModule, "hard yellow flash", 0},
		{0, DataCollectModule, "data collect", 0},
		{0, CommunicationModule, "communication", 0},
		{0, DemotionDeal, "demotion", 0},
#ifdef USE_GB_PROTOCOL
	  //{0, GbUDPCommunicationModule, "GB udp communication", 0},
	  //{0, GbTCPCommunicationModule, "GB tcp communication", 0},
#endif
	};

	ReadFaultStatus();	//读取故障检测的状态,用于恢复使用
    gSignalControlpara = (PSignalControllerPara)calloc(1, sizeof(SignalControllerPara));
    if(!gSignalControlpara)
    {
        ERR("error to alloc memory for gSignalControlpara  : %s\n",strerror(errno));
		exit(1);
    }
#ifdef USE_GB_PROTOCOL
	gGbconfig = (GbConfig *)calloc(1, sizeof(GbConfig));
	if (NULL == gGbconfig)
	{
		ERR("error to alloc memory for gGbconfig pointer  : %s\n",strerror(errno));
		exit(1);
	}
#endif
	fclose(stdout);
	ReadBinAllCfgParams();
	if (BIT(gStructBinfileCustom.sCountdownParams.option, 1))
		ItsYellowLampFlash(TRUE);//设置黄灯时闪烁
	if (gStructBinfileCustom.sCountdownParams.redFlashSec) 
	{
		ItsSetRedFlashSec((UInt8)gStructBinfileCustom.sCountdownParams.redFlashSec);//设置红灯倒计时闪烁的秒数
		INFO("red flash second %d", gStructBinfileCustom.sCountdownParams.redFlashSec);
	}
	GPSInit();

	signal(SIGSEGV, SigHandle);
	signal(SIGINT, SigHandle);		//for ctrl + c
	signal(SIGTERM, SigHandle);    //for command 'kill' or 'killall'
	signal(SIGUSR1, SigHandle);	//for OFTEN print

	LoadLocalConfigFile(NTCIP, gSignalControlpara);
	ItsInit(threads, sizeof(threads) / sizeof(threads[0]), NTCIP, gSignalControlpara);
}

void PrintVehCountDown(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n\n",\
		GET_COLOR(phaseInfos[0].phaseStatus), phaseInfos[0].phaseLeftTime,
		GET_COLOR(phaseInfos[1].phaseStatus), phaseInfos[1].phaseLeftTime,
		GET_COLOR(phaseInfos[2].phaseStatus), phaseInfos[2].phaseLeftTime,
		GET_COLOR(phaseInfos[3].phaseStatus), phaseInfos[3].phaseLeftTime,
		GET_COLOR(phaseInfos[4].phaseStatus), phaseInfos[4].phaseLeftTime,
		GET_COLOR(phaseInfos[5].phaseStatus), phaseInfos[5].phaseLeftTime,
		GET_COLOR(phaseInfos[6].phaseStatus), phaseInfos[6].phaseLeftTime,
		GET_COLOR(phaseInfos[7].phaseStatus), phaseInfos[7].phaseLeftTime);
}

void PrintRunInfo(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintRunInfo>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d",\
		phaseInfos[0].splitTime - phaseInfos[0].phaseSplitLeftTime + 1, phaseInfos[0].splitTime,
		phaseInfos[1].splitTime - phaseInfos[1].phaseSplitLeftTime + 1, phaseInfos[1].splitTime,
		phaseInfos[2].splitTime - phaseInfos[2].phaseSplitLeftTime + 1, phaseInfos[2].splitTime,
		phaseInfos[3].splitTime - phaseInfos[3].phaseSplitLeftTime + 1, phaseInfos[3].splitTime,
		phaseInfos[4].splitTime - phaseInfos[4].phaseSplitLeftTime + 1, phaseInfos[4].splitTime,
		phaseInfos[5].splitTime - phaseInfos[5].phaseSplitLeftTime + 1, phaseInfos[5].splitTime,
		phaseInfos[6].splitTime - phaseInfos[6].phaseSplitLeftTime + 1, phaseInfos[6].splitTime,
		phaseInfos[7].splitTime - phaseInfos[7].phaseSplitLeftTime + 1, phaseInfos[7].splitTime,
		data->cycleTime - data->leftTime + 1, data->cycleTime);
}
//此处只是举一个ItsCustom函数实现的例子,可以模仿去做一些其他的事情
void ItsCustom(LineQueueData *data)
{
	SendRunInfoTOBoard(data, gSignalControlpara);
	ChannelControl();
	//PrintRunInfo(data);
	//PrintVehCountDown(data);
}

//输出红绿信号到倒计时器
void ItsCountDownOutput(LineQueueData *data)
{
    CountDownInterface(data);
}

static void StartLockChan(UINT8 *chanStatus)
{
	CHANNEL_LOCK_PARAMS chan;
	
	if(chanStatus == NULL)
		return;

	memset(&chan, 0x0, sizeof(CHANNEL_LOCK_PARAMS));
	chan.unExtraParamHead = COM_MSG_HEAD;
	chan.unExtraParamID = MSG_CHAN_LOCK;
	chan.ucWorkingTimeFlag = 0;//时段由上一层调用控制
	memcpy(chan.ucChannelStatus, chanStatus, NUM_CHANNEL);
	
	ItsChannelLock(&chan);
}
static char RealTimeChanLock(UINT8 *chan)
{
	int i;

	if(chan == NULL)
		return 0;
	
	if(gStructBinfileCustom.cChannelLockFlag == 0)
		return 0;
	for(i=0; i<NUM_CHANNEL; i++)
	{
		if(gStructBinfileCustom.sChannelLockedParams.ucChannelStatus[i] != INVALID)
			chan[i] = gStructBinfileCustom.sChannelLockedParams.ucChannelStatus[i];
	}
	return 1;
}
static char MulPeriodsChanLock(UINT8 *chan)
{
	int i;
	time_t start,end,now;
	struct tm tmp;
	char ret=0;

	if(chan == NULL)
		return 0;

	if(gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag == 0)
		return 0;
	
	time(&now);
	localtime_r(&now, &tmp);
	for(i=0; i<MAX_CHAN_LOCK_PERIODS; i++)
	{
		if(gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucWorkingTimeFlag == 0)
			continue;
		
		tmp.tm_hour = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeHour;
		tmp.tm_min = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeMin;
		tmp.tm_sec = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucBeginTimeSec;
		start = mktime(&tmp);
		tmp.tm_hour = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucEndTimeHour;
		tmp.tm_min = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucEndTimeMin;
		tmp.tm_sec = gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucEndTimeSec;
		end = mktime(&tmp);
		if((start <= end) && (now >= start && now < end))
		{
			break;
		}
		else if((start > end) && (now >= start || now < end))
		{
			break;
		}
	}
	if(i != MAX_CHAN_LOCK_PERIODS)
	{
		memcpy(chan, gStructBinfileCustom.sMulPeriodsChanLockParams.chans[i].ucChannelStatus, NUM_CHANNEL);
		ret = 1;
	}
	return ret;
}
static void ChannelControl(void)
{
	char lockFlag=0;
	UINT8 chan[NUM_CHANNEL] = {INVALID};
	static UINT8 lastChan[NUM_CHANNEL] = {INVALID};
	
	lockFlag = MulPeriodsChanLock(chan);
	lockFlag |= RealTimeChanLock(chan);

	if(lockFlag == 0)//unlock
	{
		ItsChannelUnlock();
		memset(lastChan, 0, NUM_CHANNEL);
	}
	else
	{
		if(strncmp((char*)chan, (char*)lastChan, NUM_CHANNEL)!=0)//channel values update
		{
			StartLockChan(chan);
			memcpy(lastChan, chan, NUM_CHANNEL);
		}
	}
}

