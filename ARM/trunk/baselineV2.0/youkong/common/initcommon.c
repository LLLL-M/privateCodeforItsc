
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
#include "ykconfig.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define SA(x) ((struct sockaddr *)&x)
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void *DataCollectModule(void *arg);
extern void *CommunicationModule(void *arg);
extern void HardflashDogCtrl(); //硬黄闪输出
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
//信号机配置参数主结构全局变量指针，用来存放配置信息
YK_Config *gSignalControlpara = NULL;
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
	
	if (gSignalControlpara->wholeConfig.watchdogEnable == 0)
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

static void LoadLocalConfigFile(void *config)
{
	int fd;
	
	if (config == NULL)
		return;
	fd = open("/home/ykconfig.dat", O_RDONLY);
	if (fd == -1)
	{
		ERR("open /home/ykconfig fail");
		return;
	}
	if (read(fd, config, sizeof(YK_Config)) != sizeof(YK_Config))
		memset(config, 0, sizeof(YK_Config));
	close(fd);
}

void InitCommon()
{
	struct threadInfo threads[] = {
		{0, HardYellowFlashModule, "hard yellow flash", 0},
		{0, DataCollectModule, "data collect", 0},
		{0, CommunicationModule, "communication", 0},
	};

    gSignalControlpara = (YK_Config *)calloc(1, sizeof(YK_Config));
    if(!gSignalControlpara)
    {
        ERR("error to alloc memory for gSignalControlpara  : %s\n",strerror(errno));
		exit(1);
    }
	
	fclose(stdout);
	signal(SIGSEGV, SigHandle);
	signal(SIGINT, SigHandle);		//for ctrl + c
	signal(SIGTERM, SigHandle);    //for command 'kill' or 'killall'
	signal(SIGUSR1, SigHandle);	//for OFTEN print

	LoadLocalConfigFile(gSignalControlpara);
	ItsInit(threads, sizeof(threads) / sizeof(threads[0]), gSignalControlpara, sizeof(YK_Config));
}

#if 0
//输出红绿信号到倒计时器
void ItsCountDownOutput(LineQueueData *data)
{
    CountDownInterface(data);
}
#endif
