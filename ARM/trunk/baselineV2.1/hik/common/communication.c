#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#ifndef _SVID_SOURCE
#define _SVID_SOURCE	//对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <time.h>
#include "common.h"
#include "HikConfig.h"
#include "platform.h"
#include "its.h"
#include "configureManagement.h"
//#include "binfile.h"
#include "sqlite3.h"
#include "sqlite_conf.h"

extern int gOftenPrintFlag;

static struct UDP_INFO udp_info;

//MsgPhaseSchemeId gStructMsg1049;                            //供平台1049协议使用
pthread_rwlock_t gLockRealtimeVol = PTHREAD_RWLOCK_INITIALIZER;//保护实时流量的读写锁
MsgRealtimeVolume gStructMsgRealtimeVolume;                 //实时流量，只有流量是实时的

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述
extern UInt8 gRedCurrentValue[32];
extern SignalControllerPara *gSignalControlpara;
extern CountDownCfg        g_CountDownCfg; 
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //混杂参数
extern UINT8 gInductiveDemotion;

unsigned int iIsSaveHikconfig;
unsigned int iIsSaveSpecialParams;
unsigned int iIsSaveCustomParams;
unsigned int iIsSaveDescParams;

struct STRU_N_IP_ADDRESS ip_infos[3] = {{{0},{0},{0}}};//eth0, eth1 and wlan0's ip infos

extern void StoreBegin(void *arg);
extern void DownloadConfig(int type, void *arg);
extern int DownloadExtendConfig(struct STRU_Extra_Param_Response *response);
extern int UploadConfig(struct STRU_Extra_Param_Response *response);
extern void WriteLoaclConfigFile(void *config);
extern void GetCurChanLockStatus(STRU_CHAN_LOCK_PARAMS *status);

extern char XMLMsgHandle(STRU_EXTEND_UDP_MSG *msg);

typedef struct
{
	UInt32 startline;
	UInt32 linenum;
    int sockfd;
    struct sockaddr addr;
} UploadFaultLogNetArg; //上载故障日志的网络参数

void UploadFaultLog(void *arg, void *data, int datasize)
{
    UploadFaultLogNetArg *netArg = (UploadFaultLogNetArg *)arg;
    char buf[2048] = {0};
    struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)buf;

    response->unExtraParamHead = 0x6e6e;        //消息头标志
    response->unExtraParamID = 0xc0;            //消息类型ID
    response->unExtraParamValue = 0x15b;        //块数据类型
	response->unExtraParamFirst = netArg->startline;
	response->unExtraParamTotal = datasize / sizeof(FaultLogInfo);
	response->unExtraDataLen = datasize;
    memcpy(response->data, data, datasize);
    sendto(netArg->sockfd, response, sizeof(struct STRU_Extra_Param_Response) + datasize, MSG_DONTWAIT, &netArg->addr, sizeof(struct sockaddr));
}

/*****************************************************************************
 函 数 名  : Set1049MsgContent
 功能描述  : 为平台1049协议
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年11月16日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
__attribute__((unused)) static void Set1049MsgContent(MsgPhaseSchemeId *pStructMsg1049)
{
    int i = 0;
    int k = 0;
    int j = 0;

    pStructMsg1049->unExtraParamHead = 0x6e6e;
    pStructMsg1049->unExtraParamID = 0xd2;
    pStructMsg1049->unExtraDataLen = sizeof(pStructMsg1049->nPatternArray)+sizeof(pStructMsg1049->nPhaseArray);       
    
    //信号机配置的所有相位的相位号
    for(i = 0,k = 0; i < 4; i++)
    {
        for(j = 0; j < 16; j++)
        {
            if(gSignalControlpara->stPhaseTurn[0][i].nTurnArray[j] != 0)
            {
                pStructMsg1049->nPhaseArray[k++] = gSignalControlpara->stPhaseTurn[0][i].nTurnArray[j];
                
            }
            else
            {
                break;
            }
        }
    }

    //信号机配置的所有方案号，不做转换
    for(i = 0,j = 0; i < 108; i++)
    {
        if(gSignalControlpara->stScheme[i].nCycleTime != 0)
        {
            pStructMsg1049->nPatternArray[j] = gSignalControlpara->stScheme[i].nSchemeID;
            j++;
        }
    }
}



/*****************************************************************************
 函 数 名  : SendSuccessMsg
 功能描述  : 在下载或上传完成后，向客户端发送成功标示。
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static inline void SendSuccessMsg(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;

	udp_info.iValue[0] = 0;
	result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
	}

}
static inline void SendFailureMsg(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;

	udp_info.iValue[0] = 1;
	result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
	}

}

/*****************************************************************************
 函 数 名  : DownloadSpecialParams
 功能描述  : 下载特殊参数
 输入参数  : int socketFd                          
             struct sockaddr_in fromAddr           
             STRU_SPECIAL_PARAMS *p_SpecialParams  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void DownloadSpecialParams(int socketFd,struct sockaddr_in fromAddr,STRU_SPECIAL_PARAMS *p_SpecialParams)
{
    if(p_SpecialParams == NULL)
        return;
    
    p_SpecialParams->iErrorDetectSwitch = GET_BIT(udp_info.iValue[0], 0);
    p_SpecialParams->iCurrentAlarmSwitch = GET_BIT(udp_info.iValue[0], 1);
    p_SpecialParams->iVoltageAlarmSwitch = GET_BIT(udp_info.iValue[0], 2);
    p_SpecialParams->iCurrentAlarmAndProcessSwitch = GET_BIT(udp_info.iValue[0], 3);
    p_SpecialParams->iVoltageAlarmAndProcessSwitch = GET_BIT(udp_info.iValue[0], 4);
    p_SpecialParams->iWatchdogSwitch = GET_BIT(udp_info.iValue[0], 5);
    p_SpecialParams->iGpsSwitch = GET_BIT(udp_info.iValue[0], 6);	
    p_SpecialParams->iPhaseTakeOverSwtich = GET_BIT(udp_info.iValue[0], 7);	

    //更新看门狗和GPS进程
    if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 0) && (p_SpecialParams->iWatchdogSwitch == 1))
    {
		//硬件看门狗从关闭到打开
		system("killall -9 watchdog &");
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 1;
    }
    else if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 1) && (p_SpecialParams->iWatchdogSwitch == 0))
    {	//硬件看门狗从打开到关闭
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 0;                
    }

    memcpy(&gStructBinfileConfigPara.sSpecialParams,p_SpecialParams,sizeof(gStructBinfileConfigPara.sSpecialParams));
    SendSuccessMsg(socketFd,fromAddr);
}

static int IsIpInSameNet(UINT8 ieth, UINT32 ip, UINT32 netmask)
{
    struct in_addr addr = {0};
	struct in_addr mask = {0};

	if (ieth == 1)
	{
		inet_aton(ip_infos[0].address, &addr);
		inet_aton(ip_infos[0].subnetMask, &mask);
	}
	else if (ieth == 0)
	{
		inet_aton(ip_infos[1].address, &addr);
		inet_aton(ip_infos[1].subnetMask, &mask);
	}
	else 
		return 0;
	if ((ip & netmask) == (addr.s_addr & mask.s_addr))//same network range
		return 1;
	else 
		return 0;
}

static Boolean SetNetwork(const char *name, UInt32 ip, UInt32 netmask, UInt32 gateway, char *mac)
{
	struct ifreq ifr;
	struct rtentry rt;
	struct sockaddr_in gateway_addr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	unsigned char *macaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return FALSE;
	}
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, name);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1)
	{
		ERR("get netcard %s IFFLAGS failed: %s", name, strerror(errno));
		return FALSE;
	}
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1)
	{
		ERR("set netcard %s IFFLAGS failed: %s", name, strerror(errno));
		return FALSE;
	}
	
	//先获取mac地址
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
	{
		ERR("get mac address fail!");
		close(sockfd);
		return FALSE;
	}
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
		macaddr[0], macaddr[1], macaddr[2],
		macaddr[3], macaddr[4], macaddr[5]);

	//设置ip
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip;
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1)
	{
		ERR("set %s ip %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return FALSE;
	}
	INFO("set %s ip %s successful!", name, inet_ntoa(addr->sin_addr));
	//设置netmask
	addr->sin_addr.s_addr = netmask;
	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1)
	{
		ERR("set %s netmask %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return FALSE;
	}
	INFO("set %s netmask %s successful!", name, inet_ntoa(addr->sin_addr));
	//设置网关
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, name);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1)
	{
		ERR("get netcard %s IFFLAGS failed: %s", name, strerror(errno));
		return FALSE;
	}
	if (ifr.ifr_flags & IFF_RUNNING && (strcmp(name, "wlan0") != 0))
	{
	addr->sin_addr.s_addr = gateway;	//这一句只是用来打印，没什么具体用途
	memset(&rt, 0, sizeof(rt));
	memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
	rt.rt_dev = (char*)name;
	gateway_addr.sin_family = PF_INET;
	//inet_aton("0.0.0.0",&gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	ioctl(sockfd, SIOCDELRT, &rt); 	//设置网关之前先删除之前的

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	gateway_addr.sin_addr.s_addr = gateway;
	memcpy(&rt.rt_gateway, &gateway_addr, sizeof(struct sockaddr_in));
	inet_aton("0.0.0.0", &gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	if(ioctl(sockfd, SIOCADDRT, &rt) == -1)		//重新添加新的
	{
		ERR("set %s gateway %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return FALSE;
	}
	INFO("set %s gateway %s successful!", name, inet_ntoa(addr->sin_addr));
	}
	close(sockfd);
	return TRUE;
}

static int SetGateway(const char* dev_name, UInt32 gateway)
{
	struct rtentry rt;
	struct sockaddr_in gateway_addr;

	if (strcmp(dev_name, "wlan0") == 0)
		return 0;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return -1;
	}

	//设置网关
	//addr->sin_addr.s_addr = gateway;	
	memset(&rt, 0, sizeof(rt));
	memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
	rt.rt_dev = (char*)dev_name;
	gateway_addr.sin_family = PF_INET;
	//inet_aton("0.0.0.0",&gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	ioctl(sockfd, SIOCDELRT, &rt); 	//设置网关之前先删除之前的

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	gateway_addr.sin_addr.s_addr = gateway;
	memcpy(&rt.rt_gateway, &gateway_addr, sizeof(struct sockaddr_in));
	inet_aton("0.0.0.0", &gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	if(ioctl(sockfd, SIOCADDRT, &rt) == -1)		//重新添加新的
	{
		ERR("set %s gateway %s fail, error info: %s\n", dev_name, inet_ntoa(gateway_addr.sin_addr), strerror(errno));
		close(sockfd);
		return -1;
	}
	INFO("set %s gateway %s successful!", dev_name, inet_ntoa(gateway_addr.sin_addr));
	close(sockfd);
	return 0;
}

void SaveIpInfosToInterface(struct STRU_N_IP_ADDRESS* ip_infos)
{
	UINT32 ip = 0, mask = 0, gateway = 0;
	int fd = -1;
	char *hv = HARDWARE_VERSION_INFO;
	char hver[32] = {0};
	char buf[1024] = {0};
	
	strcpy(hver, hv);
	fd = open("/etc/network/interfaces", O_CREAT|O_WRONLY|O_TRUNC);
	if (fd <= 0)
		return;

	memset(buf, 0, 1024);
	sprintf(buf, "# Configure Loopback\n"
		"auto lo\n"
		"iface lo inet loopback\n"
		"\n\n");
	write(fd, buf, strlen(buf));

	if (strcmp(hver, "DS-TSC500") == 0)
	{
		memset(buf, 0, 1024);
		sprintf(buf, "auto eth0\n"
				"iface eth0 inet static\n"
				"pre-up ifconfig eth0 hw ether %s\n"
				"address %s\n"
				"netmask %s\n"
				"gateway %s\n"
				"\n\n"
				, ip_infos[0].mac, ip_infos[0].address, ip_infos[0].subnetMask, ip_infos[0].gateway);
		write(fd, buf, strlen(buf));
	}

	memset(buf, 0, 1024);
	sprintf(buf, "auto eth1\n"
		"iface eth1 inet static\n"
		"pre-up ifconfig eth1 hw ether %s\n"
		"address %s\n"
		"netmask %s\n"
		"gateway %s\n"
		"\n\n"
		, ip_infos[1].mac, ip_infos[1].address, ip_infos[1].subnetMask, ip_infos[1].gateway);
	write(fd, buf, strlen(buf));

	memset(buf, 0, 1024);
	sprintf(buf, "auto wlan0\n"
			"iface wlan0 inet static\n"
			"address %s\n"
			"netmask %s\n"
			"#gateway %s\n"

			, ip_infos[2].address, ip_infos[2].subnetMask, ip_infos[2].gateway);
	write(fd, buf, strlen(buf));

	memset(buf, 0, 1024);
	sprintf(buf, "#pre-up wpa_supplicant -iwlan0 -c/etc/wpa_supplicant.conf&\n"
						"#post-down killall -9 wpa_supplicant\n"
						"\n");
	write(fd, buf, strlen(buf));
	fdatasync(fd);
	close(fd);
}

void SetDefaultIp()
{
	int fd = -1;
	char* hv = HARDWARE_VERSION_INFO;
	char tsc_interface[2048] = {0};

	fd = open("/etc/network/interface", O_CREAT|O_WRONLY|O_TRUNC);
	if (fd <= 0)
		return;
	if (strcmp(hv, "DS-TSC500") == 0)
	{
		sprintf(tsc_interface, "# Configure Loopback"\
"auto lo"\
"iface lo inet loopback"\

"auto eth0"\
"iface eth1 inet static"\
"address 172.7.18.61"\
"netmask 255.255.255.0"\
"gateway 172.7.18.1"\

"auto eth1"\
"iface eth1 inet static"\
"address 192.168.1.101"\
"netmask 255.255.255.0"\
"gateway 192.168.1.1"\

"auto wlan0"\
"iface wlan0 inet static"\
"address 192.168.9.101"\
"netmask 255.255.255.0"\
"#gateway 192.168.9.1"\
"#pre-up wpa_supplicant -iwlan0 -c/etc/wpa_supplicant.conf&"\
"#post-down killall -9 wpa_supplicant");
	}
	else
	{
		sprintf(tsc_interface, "# Configure Loopback"\
"auto lo"\
"iface lo inet loopback"\

"auto eth1"\
"iface eth1 inet static"\
"address 192.168.1.101"\
"netmask 255.255.255.0"\
"gateway 192.168.1.1"\

"auto wlan0"\
"iface wlan0 inet static"\
"address 192.168.9.101"\
"netmask 255.255.255.0"\
"#gateway 192.168.9.1"\
"#pre-up wpa_supplicant -iwlan0 -c/etc/wpa_supplicant.conf&"\
"#post-down killall -9 wpa_supplicant");
	}
	
	write(fd, tsc_interface, strlen(tsc_interface));

	fdatasync(fd);
	close(fd);
	
}

typedef enum
	{
		SAVE_NONE = 0,
		SAVE_TO_DB = 1,
		SAVE_TO_INTERFACE = 2,
		SAVE_BOTH = 3,
	}NetCardSaveFlag;
//获取已运行网卡的网络字节序ip地址
static  in_addr_t GetNetworkCardIp(const char *interface)
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
		strncpy(ifr->ifr_name, interface, IFNAMSIZ - 1);
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

static struct in_addr GetGateway(const char *net_dev, char *gateway)
{
    FILE *fp;    
    char buf[1024];  
    char iface[16];    
    unsigned char tmp[100]={'\0'};
    unsigned int dest_addr=0, gate_addr=0;
	struct sockaddr_in Addr = {0};
	
    if(NULL == gateway)
    {
        INFO("gateway is NULL \n");
        return Addr.sin_addr; 
    }
    fp = fopen("/proc/net/route", "r");    
    if(fp == NULL){  
        INFO("fopen error \n");
        return Addr.sin_addr;   
    }
    
    fgets(buf, sizeof(buf), fp);    
    while(fgets(buf, sizeof(buf), fp)) 
    {    
        if((sscanf(buf, "%s\t%X\t%X", iface, &dest_addr, &gate_addr) == 3) 
            && (memcmp(net_dev, iface, strlen(net_dev)) == 0)
            && gate_addr != 0) 
        {
                memcpy(tmp, (unsigned char *)&gate_addr, 4);
                sprintf(gateway, "%d.%d.%d.%d", (unsigned char)*tmp, (unsigned char)*(tmp+1), (unsigned char)*(tmp+2), (unsigned char)*(tmp+3));
                break;    
        }
    }    
      
    fclose(fp);
	if (gateway[0] == 0)//don't get gateway,set default 0.0.0.0
		strcpy(gateway, "0.0.0.0");
	inet_aton(gateway, &Addr.sin_addr);
	return Addr.sin_addr;
}

static struct in_addr GetMask(const char *net_dev, char *mask)
{
    struct sockaddr_in Addr = {0};
    struct ifreq ifr;
    int sockfd;

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    memset(&ifr,0,sizeof(ifr));
    strncpy(ifr.ifr_name, net_dev, IFNAMSIZ - 1);
    
    if(ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0){
        close(sockfd);
        return Addr.sin_addr;
    }

	memcpy(&Addr, &(ifr.ifr_addr), sizeof(struct sockaddr_in));
    //pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
    strncpy(mask, (char *)(inet_ntoa(Addr.sin_addr)), 16);
    
    close(sockfd);
	return Addr.sin_addr;
}

static int GetMac(const char* net_dev, char* mac)
{
	struct ifreq ifr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	unsigned char *macaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	if (mac == NULL)
		return 0;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return FALSE;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, net_dev, IFNAMSIZ - 1);
	//先获取mac地址
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
	{
		ERR("get mac address fail!");
		close(sockfd);
		return FALSE;
	}
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
		macaddr[0], macaddr[1], macaddr[2],
		macaddr[3], macaddr[4], macaddr[5]);
	close(sockfd);
	return 1;
}

int IfUpDown(const char *ifname, int flag)
{
    int fd, rtn;
    struct ifreq ifr;        

    if (!ifname) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        ERR("socket called fail, error info: %s\n", strerror(errno));
        return -1;
    }
    
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1);

    if ( (rtn = ioctl(fd, SIOCGIFFLAGS, &ifr) ) == 0 ) 
	{
        if ( flag == 0 )
            ifr.ifr_flags &= ~IFF_UP;
        else if ( flag == 1 ) 
            ifr.ifr_flags |= IFF_UP;
        
		if ( (rtn = ioctl(fd, SIOCSIFFLAGS, &ifr) ) != 0)
		{
			ERR("SIOCSIFFLAGS failed , error=%s", strerror(errno));
		}
    }

    
    close(fd);

    return rtn;
}

static int SetMac(const char* net_dev, char* mac)
{
	struct ifreq ifr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	unsigned int macval[6] = {0};
	unsigned char *macaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	if (mac == NULL)
		return 0;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return 0;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_addr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, net_dev, IFNAMSIZ - 1);
	sscanf(mac, "%X:%X:%X:%X:%X:%X", &macval[0], &macval[1], &macval[2],
		&macval[3], &macval[4], &macval[5]);
	macaddr[0] = macval[0];
	macaddr[1] = macval[1];
	macaddr[2] = macval[2];
	macaddr[3] = macval[3];
	macaddr[4] = macval[4];
	macaddr[5] = macval[5];
	//set mac address
	if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) == -1)
	{
		ERR("get mac address fail!");
		close(sockfd);
		return 0;
	}
	close(sockfd);
	return 1;
}

void DelDefaultGateway(void)
{
	char *default_gateway[2] = {"172.7.18.1", "192.168.1.1"};//eth0, eth1
	struct ifreq ifreqs[8];
  	UINT8 net_dev_defgateway = 0;
	UINT8 net_dev_running = 0;
	UINT8 del_eth = 0;
	char cmd[128] = {0};
    struct ifreq *ifr = ifreqs;
    //struct in_addr addr = {0};
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return;
	}
	strncpy(ifr->ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(sockfd, SIOCGIFFLAGS, ifr);
	if (ifr->ifr_flags & IFF_RUNNING)
	{	
		INFO("network card eth0 is running.");
		SET_BIT(net_dev_running, 0);
	}
	strncpy(ifr->ifr_name, "eth1", IFNAMSIZ - 1);
	ioctl(sockfd, SIOCGIFFLAGS, ifr);
	if (ifr->ifr_flags & IFF_RUNNING)
	{	
		INFO("network card eth1 is running.");
		SET_BIT(net_dev_running, 1);
	}
	
	if (strcmp(ip_infos[0].gateway, default_gateway[0]) == 0)
	{
		SET_BIT(net_dev_defgateway, 0);
		
	}
	if (strcmp(ip_infos[1].gateway, default_gateway[1]) == 0)
	{
		SET_BIT(net_dev_defgateway, 1);
	}

	if (GET_BIT(net_dev_running, 0) == 1 && GET_BIT(net_dev_defgateway, 0) == 0)// eth0 RUNNING && not default
		del_eth = 2;
	else if (GET_BIT(net_dev_running, 1) == 1 && GET_BIT(net_dev_defgateway, 1) == 0)//eth1 RUNNING && not default
		del_eth = 1;
	else if (GET_BIT(net_dev_running, 0) == 1)// eth0 RUNNING && default
		del_eth = 2;
	else if (GET_BIT(net_dev_running, 1) == 1)//eth1 RUNNING && default
		del_eth = 1;
	else if (GET_BIT(net_dev_defgateway, 0) == 0)// eth0 UNRUNNING && not default
		del_eth = 2;
	else if (GET_BIT(net_dev_defgateway, 1) == 0)//eth1 UNRUNNING && not default
		del_eth = 1;
	else //both eth0 and eth1 are UNRUNNING && default, del eth0
		del_eth = 1;
	
	if (del_eth == 1)
		sprintf(cmd, "route del default eth0");
	else if (del_eth == 2)
		sprintf(cmd, "route del default eth1");
	INFO("del gateway =%s", cmd);
	system(cmd);
}

UINT8 ModifyDefaultMac(const char* net_dev, char* mac)
{
	char* ethmac[2] = {"02:C2:63:4F:18:6C", "82:87:F4:79:59:30"};//eth0,eth1
	struct ifreq ifr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	char strmac[24] = {0};
	unsigned char *macaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	unsigned int randval = 0;
	time_t rseed = 0;
	int net_dev_index = -1;
	sqlite3 *pdb = NULL;
	
	if (strcmp("eth0", net_dev) == 0)
		net_dev_index = 0;
	else if (strcmp("eth1", net_dev) == 0)
		net_dev_index = 1;
	if (mac == NULL || net_dev_index > 1 || net_dev_index < 0)
		return SAVE_NONE;
	
	if (strcmp(mac, ethmac[net_dev_index]) != 0 && 
		strcmp(ip_infos[net_dev_index].mac, ethmac[net_dev_index]) != 0)//mac isn't default && db's mac isn't default, don't need to modify
		return SAVE_NONE;

	time(&rseed);
	srandom(rseed);
	srandom(rseed + random());
	randval = random();

	macaddr[0] = 0x82;
	macaddr[1] = 0x77;
	macaddr[3] = ((char*)(&randval))[0];
	macaddr[4] = ((char*)(&randval))[1];
	macaddr[5] = ((char*)(&randval))[2];
	macaddr[6] = ((char*)(&randval))[3];
	//INFO("default mac=%s", mac);
	//INFO("MAC=%02X:%02X:%02X:%02X:%02X:%02X", macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
	if (IfUpDown(net_dev, 0) == 0)
	{
		sprintf(strmac, "%02X:%02X:%02X:%02X:%02X:%02X", 
		macaddr[0], macaddr[1], macaddr[2],
		macaddr[3], macaddr[4], macaddr[5]);
		SetMac(net_dev, strmac);
		usleep(1000000);//1000ms
		IfUpDown(net_dev, 1);
	}
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_blob_column(pdb, TABLE_NAME_IPINFOS, "mac", net_dev_index + 1, strmac, 24, 0);
	sqlite3_close_wrapper(pdb);
	strncpy(ip_infos[net_dev_index].mac, strmac, 24);
	strncpy(mac, strmac, 24);
	return SAVE_TO_INTERFACE|SAVE_TO_DB;
}

UINT8 CompareIpInfos(const char* net_dev, struct STRU_N_IP_ADDRESS* ip_infos)
{
	UINT32 ip = 0, mask = 0, gateway = 0;
	char gateway_addr[16] = {0};
	struct sockaddr_in hostgateway = {0};
	struct STRU_N_IP_ADDRESS host_ip_info = {{0},{0},{0},{0}};
	char mac[24] = {0};
	UINT8 net_dev_index = 0;
	struct sockaddr_in addr = {0};
	UINT8 save_flag = 0;
	struct STRU_N_IP_ADDRESS default_ip_infos[3] = {
			{"172.7.18.61", "255.255.255.0", "172.7.18.1", {0}},
			{"192.168.1.101", "255.255.255.0", "192.168.1.1", {0}},
			{"192.168.9.101", "255.255.255.0", "192.168.9.1", {0}}};
	char* ethmac[2] = {"02:C2:63:4F:18:6C", "82:87:F4:79:59:30"};//default eth0,eth1
	
	if (strcmp(net_dev, "eth0") == 0)
		net_dev_index = 0;
	else if (strcmp(net_dev, "eth1") == 0)
		net_dev_index = 1;
	else if(strcmp(net_dev, "wlan0") == 0)
		net_dev_index = 2;
	else
		return SAVE_NONE;

	
	inet_aton(ip_infos[net_dev_index].address, (struct in_addr*)&ip);
	inet_aton(ip_infos[net_dev_index].subnetMask, (struct in_addr*)&mask);
	inet_aton(ip_infos[net_dev_index].gateway, (struct in_addr*)&gateway);
	addr.sin_addr.s_addr = GetNetworkCardIp(net_dev);
	GetMac(net_dev, mac);
	save_flag |= ModifyDefaultMac(net_dev, mac);
	if (((UINT32)(addr.sin_addr.s_addr)) > 0 && ((UINT32)(addr.sin_addr.s_addr)) == ip && 
		(strlen(mac) >= 17 && strcmp(mac, ip_infos[net_dev_index].mac) == 0))
	{
		hostgateway.sin_addr = GetGateway(net_dev, gateway_addr);
		if (((UINT32)hostgateway.sin_addr.s_addr) == 0 && net_dev_index != 2)//host gateway is 0.0.0.0,need to set gateway
			SetGateway(net_dev, gateway);
		
		INFO("host == db , ip is ok.");
		return save_flag;
	}
	else if (((UINT32)(addr.sin_addr.s_addr)) == 0)//net card don't have ipaddr
	{
			if (ip != 0)//config database has ip infos, set ip
			{
				INFO("1set net %s", ip_infos[net_dev_index].address);
				SetNetwork(net_dev, ip, mask, gateway, mac);
				//write ipaddr infos to interface
				if (net_dev_index == 2)//wlan0
				{	
					save_flag |= SAVE_NONE;
					return save_flag;
				}
				save_flag |= SAVE_TO_INTERFACE;
			}
			else //both no ip, set default ipaddr
			{
				memcpy(&ip_infos[net_dev_index], &default_ip_infos[net_dev_index], sizeof(STRU_STRU_N_IP_ADDRESS));
				inet_aton(ip_infos[net_dev_index].address, (struct in_addr*)&ip);
				inet_aton(ip_infos[net_dev_index].subnetMask, (struct in_addr*)&mask);
				inet_aton(ip_infos[net_dev_index].gateway, (struct in_addr*)&gateway);
				INFO("2set default net %s", ip_infos[net_dev_index].address);
				SetNetwork(net_dev, ip, mask, gateway, mac);
				if (net_dev_index == 2)//wlan0
				{
					save_flag |= SAVE_NONE;
					return save_flag;
				}
				save_flag |= (SAVE_TO_DB | SAVE_TO_INTERFACE);
			}
	}
	else if (((UINT32)(addr.sin_addr.s_addr)) != ip) //current netcard ip == db's ip, return;
	{
			if (ip == 0)// database don't has ip config, save to db.
			{
				save_flag |= SAVE_TO_DB;
				strncpy(ip_infos[net_dev_index].address, inet_ntoa(addr.sin_addr), 16);
				GetMask(net_dev, ip_infos[net_dev_index].subnetMask);
				GetGateway(net_dev, ip_infos[net_dev_index].gateway);
				INFO("4set db net %s, %s, %s", ip_infos[net_dev_index].address, 
				ip_infos[net_dev_index].subnetMask, ip_infos[net_dev_index].gateway);
			}
			else //db's ip != current netcard's ip, set config ip to netcard
			{
				INFO("3set net %s", ip_infos[net_dev_index].address);
				SetNetwork(net_dev, ip, mask, gateway, mac);
				//save to interface
				save_flag |= SAVE_TO_INTERFACE;
			}
	}
	
	if (strcmp(mac, ip_infos[net_dev_index].mac) != 0)
	{
		if (strlen(ip_infos[net_dev_index].mac) >= 17)//XX:XX:XX:XX:XX:XX
		{
			save_flag |= SAVE_TO_INTERFACE; //need to save Mac addrs to interfaces
			if (IfUpDown(net_dev, 0) == 0)
			{
				INFO("set mac address to %s", ip_infos[net_dev_index].mac);
				SetMac(net_dev, ip_infos[net_dev_index].mac);
				usleep(800000);//800ms
				IfUpDown(net_dev, 1);
			}
		}
		else if (strlen(ip_infos[net_dev_index].mac) == 0)//extconfig's Mac addr is null
		{
			if (strlen(mac) >= 17)//host's Mac addr is ok, save to database.
			{
				memcpy(ip_infos[net_dev_index].mac, mac, 24);
				save_flag |= SAVE_TO_DB;
			}
		}
			
	}
	return save_flag;
}

void TscIpCheck()
{
	
	//struct STRU_N_IP_ADDRESS ip_infos[3] = {{{0},{0},{0}}};
		//{{"172.7.18.61", "255.255.255.0", "172.7.18.1"},
		//	{"192.168.1.101", "255.255.255.0", "192.168.1.1"},
		//	{"192.168.9.101", "255.255.255.0", "192.168.9.1"}};//eth0, eth1, wlan
	sqlite3 *pdb = NULL;
	char *hv = HARDWARE_VERSION_INFO;
	UINT8 save_flag = SAVE_NONE;
	UINT32 ip = 0;

	memset(ip_infos, 0, sizeof(STRU_STRU_N_IP_ADDRESS) * 3);
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_select_netcard_addr(pdb, ip_infos);
	sqlite3_close_wrapper(pdb);
	
	if (strcmp(hv, "DS-TSC500") == 0)// hardware id TSC500, eth1
	{
		save_flag |= CompareIpInfos("eth0", ip_infos);
	}
	
	save_flag |= CompareIpInfos("eth1", ip_infos);
	
	save_flag |= CompareIpInfos("wlan0", ip_infos);

	if (save_flag & SAVE_TO_INTERFACE)
	{
		SaveIpInfosToInterface(ip_infos);
		//system("service network restart");
	}
	if (save_flag & SAVE_TO_DB)
	{
		sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
		sqlite3_clear_table(pdb, TABLE_NAME_IPINFOS);
		sqlite3_insert_netcard_addr(pdb, ip_infos);
		//sqlite3_update_netcard_addr(pdb, &ip_infos[0], 1);
		//sqlite3_update_netcard_addr(pdb, &ip_infos[1], 2);
		//sqlite3_update_netcard_addr(pdb, &ip_infos[2], 3);
		sqlite3_close_wrapper(pdb);
	}
	if (strcmp(hv, "DS-TSC500") == 0)
		DelDefaultGateway();
	
}

/*****************************************************************************
 函 数 名  : DownloadIpAddress
 功能描述  : 下载本地IP地址
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void DownloadIpAddress(int socketFd,struct sockaddr_in fromAddr)
{
	struct STRU_N_IP_ADDRESS ip_info;
    struct in_addr inaddr;
	char *name = NULL;
    char cmd[512] = {0};
	char mac[24] = {0};
	sqlite3 *pdb = NULL;
	UINT8 index = 0;
	
    if(udp_info.iType == 0x15d)
	{	
		name = "eth1";
		index = 1;
	}
    else if(udp_info.iType == 0x15f)
	{
		name = "eth0";
		index = 0;
    }
    else if(udp_info.iType == 0x161)
	{
		name = "wlan0";
		index = 2;
    }
	if (IsIpInSameNet(index, udp_info.iValue[0], udp_info.iValue[1]) == 1)//same nentwork range, con't set it
	{
		SendFailureMsg(socketFd, fromAddr);
		return;
	}
	memset(&ip_info,0,sizeof(ip_info));
	inaddr.s_addr = (unsigned long)udp_info.iValue[0];
	strcpy(ip_info.address, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)udp_info.iValue[1];
	strcpy(ip_info.subnetMask, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)udp_info.iValue[2];
	strcpy(ip_info.gateway, inet_ntoa(inaddr));
	SetNetwork(name, udp_info.iValue[0], udp_info.iValue[1], udp_info.iValue[2], mac);
    SendSuccessMsg(socketFd,fromAddr);    

	memcpy(ip_info.mac, ip_infos[index].mac, 24);//keep mac not change
	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
	sqlite3_update_netcard_addr(pdb, &ip_info, index + 1);
	sqlite3_close_wrapper(pdb);
	memcpy(&ip_infos[index], &ip_info, sizeof(STRU_STRU_N_IP_ADDRESS));
	SaveIpInfosToInterface(ip_infos);
	//sprintf(cmd, "sed -i '/iface %s/{N;N;N;N;s/.*/iface %s inet static\\npre-up ifconfig %s hw ether %s\\naddress %s\\nnetmask %s\\ngateway %s/g}' /etc/network/interfaces",
		//name, name, name, mac, ip_info.address, ip_info.subnetMask, ip_info.gateway);
	//system(cmd);
}

/*****************************************************************************
 函 数 名  : UploadIpAddress
 功能描述  : 上传本地IP信息
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void UploadIpAddress(int socketFd,struct sockaddr_in fromAddr)
{
	struct STRU_N_IP_ADDRESS ip_info;
    char *cmd = NULL;
	FILE *fp = NULL;

    memset(&ip_info,0,sizeof(ip_info));
    if(udp_info.iType == 0x15e)
		cmd = "grep 'iface eth1' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(udp_info.iType == 0x160)
		cmd = "grep 'iface eth0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(udp_info.iType == 0x162)
		cmd = "grep 'iface wlan0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
	fp = popen(cmd, "r");
	if (fp != NULL)
	{	//从/etc/network/interfaces文件中获取IP信息
		fscanf(fp, "address %s\nnetmask %s\ngateway %s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
		pclose(fp);
		INFO("ip=%s,netmask=%s,address=%s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
		udp_info.iValue[0] = inet_addr(ip_info.address);
		udp_info.iValue[1] = inet_addr(ip_info.subnetMask);
		udp_info.iValue[2] = inet_addr(ip_info.gateway);
	}
    sendto(socketFd, &udp_info, 20, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
}

/*****************************************************************************
 函 数 名  : UploadVersionInfo
 功能描述  : 上传版本信息
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void UploadVersionInfo(int socketFd,struct sockaddr_in fromAddr)
{
	DEVICE_VERSION_PARAMS device_version_params;           //设备软硬件信息
	char *hv = HARDWARE_VERSION_INFO;
	char *sv = SOFTWARE_VERSION_INFO;

	memset(&device_version_params,0,sizeof(device_version_params));
	device_version_params.unExtraParamHead = udp_info.iHead;
	device_version_params.unExtraParamID = udp_info.iType;
	memcpy(device_version_params.hardVersionInfo, hv, strlen(hv));        //低32位放置硬件信息
	if (strcmp((char *)device_version_params.hardVersionInfo, "DS-TSC300") == 0)
	{
		if (gStructBinfileConfigPara.sSpecialParams.iSignalMachineType == 2)
			strcat((char *)device_version_params.hardVersionInfo, "-22");
		else
			strcat((char *)device_version_params.hardVersionInfo, "-44");
	}
	memcpy(device_version_params.softVersionInfo, sv, strlen(sv));       //高32位放置软件信息
	sendto(socketFd, &device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
}

void save_conf_to_database(void* config)
{
	int i = 0;
	STRUCT_BINFILE_CONFIG *pconfig = config;
	sqlite3 *pdatabase = NULL;
	if (pconfig == NULL)
		return;

	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
	sqlite3_begin(pdatabase);
	for (i = E_TABLE_NAME_CONF_SPECIALPRM; i < E_TABLE_NAME_CONF_SYS_INFOS + 1; i++)
	{
		if (GET_BIT(iIsSaveSpecialParams, i) == 1)
		{	
			switch(i)
			{
				case E_TABLE_NAME_CONF_SPECIALPRM:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_SPECIALPRM);
					sqlite3_insert_specialparams(pdatabase, &(pconfig->sSpecialParams));
					break;
				case E_TABLE_NAME_CONF_WIRELESS: 
					sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_WIRELESS);
					sqlite3_insert_wireless_controller(pdatabase, &(pconfig->stWirelessController));					
					break;
				case E_TABLE_NAME_CONF_FRONTBOARD:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_FRONTBOARD);
					sqlite3_insert_frontboardkey(pdatabase, &(pconfig->sFrontBoardKeys));
					break;
				case E_TABLE_NAME_CONF:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CONF);
					sqlite3_insert_config_pieces(pdatabase, pconfig);
					break;
				case E_TABLE_NAME_CONF_SYS_INFOS: 
					sqlite3_clear_table(pdatabase, TABLE_NAME_CONF_SYS_INFOS);
					sqlite3_insert_config_sys_infos(pdatabase, pconfig);
					break;
				
				default:
					break;
			}
		}
	}
	sqlite3_commit(pdatabase);
	sqlite3_close_wrapper(pdatabase);
}
void save_custom_to_database(void* custom)
{
	int i = 0;
	STRUCT_BINFILE_CUSTOM *pcustom = custom;
	sqlite3 *pdatabase = NULL;
	if (pcustom == NULL)
		return;

	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
	sqlite3_begin(pdatabase);
	for (i = E_TABLE_NAME_CUSTOM_COUNTDOWN; i < E_TABLE_NAME_CUSTOM_CAMERASERVER + 1; i++)
	{
		if (GET_BIT(iIsSaveCustomParams, i) == 1)
		{	
			switch(i)
			{
				case E_TABLE_NAME_CUSTOM_COUNTDOWN:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_COUNTDOWN);
					sqlite3_insert_countdown_prm(pdatabase, &(pcustom->sCountdownParams));
					sqlite3_update_column(pdatabase, TABLE_NAME_CUSTOM_COUNTDOWN, "cIsCountdownValueLimit", 1, &(pcustom->cIsCountdownValueLimit), 1, SQLITE_INTEGER);
					break;
				case E_TABLE_NAME_CUSTOM_COM: 
					sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_COM);
					sqlite3_insert_com_prm(pdatabase, pcustom->sComParams);					
					break;
				case E_TABLE_NAME_CUSTOM_CHANNELLOCK:
					//sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_CHANNELLOCK);
					//sqlite3_insert_chlock_prm(pdatabase, &(pcustom->sChannelLockedParams));
					//sqlite3_update_column(pdatabase, TABLE_NAME_CUSTOM_CHANNELLOCK, "cChannelLockFlag", 1, &(pcustom->cChannelLockFlag),  1, SQLITE_INTEGER);
					break;
				case E_TABLE_NAME_CUSTOM_MULCHANNELLOCK:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_MULCHANNELLOCK);
					sqlite3_insert_mp_chlock_prm(pdatabase, &(pcustom->sMulPeriodsChanLockParams));
					break;
				case E_TABLE_NAME_CUSTOM_CAMERASERVER:
					sqlite3_clear_table(pdatabase, TABLE_NAME_CUSTOM_CAMERASERVER);
					sqlite3_insert_cameraServer_prm(pdatabase, pcustom->sCameraFlowServer);
					break;
				default:
					break;
			}
		}
	}
	sqlite3_commit(pdatabase);
	sqlite3_close_wrapper(pdatabase);
}

void save_desc_to_database(void* desc)
{
	int i = 0;
	STRUCT_BINFILE_DESC *pdesc = desc;
	sqlite3 *pdatabase = NULL;
	if (pdesc == NULL)
		return;

	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdatabase);
	sqlite3_begin(pdatabase);
	for (i = E_TABLE_NAME_DESC_PHASE; i < E_TABLE_NAME_DESC_DATE + 1; i++)
	{
		if (GET_BIT(iIsSaveDescParams, i) == 1)
		{	
			switch(i)
			{
				case E_TABLE_NAME_DESC_PHASE:
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PHASE);
					sqlite3_insert_phase_desc(pdatabase, pdesc->phaseDescText);
					break;
				case E_TABLE_NAME_DESC_FOLLOW_PHASE:
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_FOLLOW_PHASE);
					sqlite3_insert_followphase_desc(pdatabase, pdesc->followPhaseDescText);
					break;
				case E_TABLE_NAME_DESC_CHANNEL: 
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_CHANNEL);
					sqlite3_insert_channel_desc(pdatabase, &(pdesc->sChannelDescParams));					
					break;
				case E_TABLE_NAME_DESC_PATTERN:
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PATTERN);
					sqlite3_insert_pattern_name_desc(pdatabase, &(pdesc->sPatternNameParams));
					break;
				case E_TABLE_NAME_DESC_PLAN:
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_PLAN);
					sqlite3_insert_plan_name(pdatabase, &(pdesc->sPlanNameParams));
					break;
				case E_TABLE_NAME_DESC_DATE:
					sqlite3_clear_table(pdatabase, TABLE_NAME_DESC_DATE);
					sqlite3_insert_plan_date(pdatabase, &(pdesc->sDateNameParams));
					break;
				default:
					break;
			}
		}
	}
	sqlite3_commit(pdatabase);
	sqlite3_close_wrapper(pdatabase);
}


#define UP_DOWN_LOAD_TIMEOUT	3		//上下载超时时间60s
static void UpAndDownLoadDeal(int socketFd, struct sockaddr_in fromAddr)	//上下载处理
{
	static UInt8 uploadNum = 0, lastUploadNum = 0, firstUploadFlagCheck = 1;
	static UInt32 downloadFlag = 0;//存放的是开始下载时SDK传送来的具体哪些参数需要保存的flag
	static struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0}, uploadFinishTime = {0, 0};
	struct timespec currentTime;
	struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)&udp_info;
	int ret = 12;
	UploadFaultLogNetArg netArg;

	if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//下载参数开始
	{
		log_debug("Recv From %s : download config begin", inet_ntoa(fromAddr.sin_addr));
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
		{	//说明有其他客户端正在下载
			udp_info.iValue[0] = 1;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		if (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
		{	//说明有其他客户端正在上载
			udp_info.iValue[0] = 2;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		downloadFlag = *(UInt32 *)(udp_info.iValue);
		downloadStartTime = currentTime;
		StoreBegin((void *)udp_info.iValue);
		udp_info.iValue[0] = 0;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
	{
		udp_info.iValue[0] = IsSignalControlparaLegal(gSignalControlpara);
		log_debug("Recv From %s : download config over, check result %d", inet_ntoa(fromAddr.sin_addr), udp_info.iValue[0]);
		if (downloadFlag > 0 && udp_info.iValue[0] == 0)
		{	//检查配置是否合理
			ItsSetConfig(gSignalControlpara, sizeof(SignalControllerPara));
			if (BIT(downloadFlag, 16)) //表示是否写flash
				WriteLoaclConfigFile(gSignalControlpara);	//写入本地配置文件里
			log_debug("config information update!");
			ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
		}
		else
			ItsGetConfig(gSignalControlpara, sizeof(SignalControllerPara));
		downloadFlag = 0;
		downloadStartTime.tv_sec = 0;
	}
	else if(udp_info.iHead == 0x6e6e && ((udp_info.iType >= 0xaa && udp_info.iType <= 0xb6) 
		|| udp_info.iType == 0xdd || udp_info.iType == 0xeeeeeeee))//下载配置信息,0xdd代表新的方案下载,0xeeeeeeee表示web下载
	{
		if (gOftenPrintFlag)
			log_debug("download config, type = %#x", udp_info.iType);
		DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 只是为了ctrl+f搜索方便!
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xce)	//上载开始
	{
		//log_debug("upload config begin");
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)	//说明有其他客户端正在下载
		{
			udp_info.iValue[0] = 1;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		//INFO("DEBUG: upload begin, downloadFlag = %#x, timegap = %d", downloadFlag, currentTime.tv_sec - downloadStartTime.tv_sec);
		//有其他客户端上载已经超时的情况下uploadNum=1,其他时候都加1
		uploadNum = (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) > UP_DOWN_LOAD_TIMEOUT) ? 1 : (uploadNum + 1);
		uploadStartTime = currentTime;
		udp_info.iValue[0] = 0;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xcf)	//上载结束
	{
		//log_debug("upload config over");
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (uploadNum > 1)
			uploadNum--;
		else if (uploadNum == 1)
		{
			uploadNum = 0;
			uploadStartTime.tv_sec = 0;
		}
		if (uploadNum > 0 && lastUploadNum > 0)
		{		
			if(firstUploadFlagCheck == 1)
			{
				clock_gettime(CLOCK_MONOTONIC, &uploadFinishTime);
				firstUploadFlagCheck = 0;
			}
			if((currentTime.tv_sec - uploadFinishTime.tv_sec > UP_DOWN_LOAD_TIMEOUT)
				&&(firstUploadFlagCheck == 0))
			{
				INFO("Upload overtime :%ld,set uploadNum to 0",
					currentTime.tv_sec - uploadFinishTime.tv_sec);
				uploadNum = 0;
			}
		}
		else if ((uploadNum && lastUploadNum) == 0 && firstUploadFlagCheck == 0)
		{
			//如果开始和结束标志数量一致，则从新计时
			firstUploadFlagCheck = 1;
		}
		lastUploadNum = uploadNum;
	}
	else if (udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)	//上载配置
	{
		ret = sizeof(struct STRU_Extra_Param_Response);
		if (response->unExtraParamValue == 0x15b)	//上载故障信息
		{
			//INFO("upload fault log, startline = %u, linenum = %u", response->unExtraParamFirst, response->unExtraParamTotal);
			netArg.startline = response->unExtraParamFirst;
			netArg.linenum = response->unExtraParamTotal;
			netArg.sockfd = socketFd;
            memcpy(&netArg.addr, &fromAddr, sizeof(struct sockaddr));
            ItsReadFaultLog(response->unExtraParamFirst, response->unExtraParamTotal, &netArg, sizeof(UploadFaultLogNetArg), UploadFaultLog);
			return;	//故障信息的上载统一都在故障日志模块回复，所以这里直接返回不在此回复
		}
		else if (response->unExtraParamValue == 68 || response->unExtraParamValue == 70
				|| response->unExtraParamValue == 78 || response->unExtraParamValue == 125
				|| response->unExtraParamValue == 154)
			ret = DownloadExtendConfig(response);
		else
		{
			//log_debug("upload config, unExtraParamValue = %u", response->unExtraParamValue);
			ret = UploadConfig(response);
		}
	}
	sendto(socketFd, &udp_info, ret, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}


#define MAX_TERMINAL_NUM	(NUM_CHANNEL * 3 + 19)
static inline void ChannelCheckDeal(int nTerminal)
{	//端子计算方式：20,21,22代表通道1的红，黄，绿；23,24,25代表通道2的红，黄，绿；以此类推
	if (nTerminal < 20 || nTerminal > MAX_TERMINAL_NUM)
		return;
	int channelId = (nTerminal - 20) / 3 + 1;
	int left = (nTerminal - 20) % 3;
	LightStatus status = (left == 0) ? RED : ((left == 1) ? YELLOW : GREEN);
	
	ItsChannelCheck(channelId, status);
}

//发送给触摸屏各个通道的倒计时值和状态
static inline void UploadChannelCountdown(int socketFd, struct sockaddr_in fromAddr)
{
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gp = (PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *)&udp_info;

	memset(udp_info.iValue, 0, sizeof(udp_info.iValue));
	ItsCountDownGet(gp, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	gp->unExtraParamHead = 0x6e6e;
	gp->unExtraParamID = 0xeeeeeeec;
	sendto(socketFd, &udp_info,sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));

	//INFO("UploadChannelCountdown  ->  %s\n",inet_ntoa(fromAddr.sin_addr));
}


static inline void UploadChannelStatus(int socketFd, struct sockaddr_in fromAddr)
{
	struct STRU_N_ChannelStatusGroup *gp = (struct STRU_N_ChannelStatusGroup *)udp_info.iValue;
	int i, j, datasize = sizeof(*gp) * 4;
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

	memset(gp, 0, datasize);
	ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	for (i = 0; i < 4; i++)	//总共4组状态
	{
		gp->byChannelStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//每组包含8个通道的状态，1bit代表一个通道
		{
			switch (countDownParams.ucChannelStatus[i * 8 + j])
			{
				case GREEN_BLINK:
				case GREEN: gp->byChannelStatusGroupGreens |= (1 << j); break;
				case YELLOW_BLINK: 
				case YELLOW: gp->byChannelStatusGroupYellows |= (1 << j); break;
				case RED_BLINK: 
				case ALLRED:
				case RED: gp->byChannelStatusGroupReds |= (1 << j); break;
			}
		}
		gp++;
	}
	sendto(socketFd, &udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

static inline void UploadPhaseStatus(int socketFd, struct sockaddr_in fromAddr)
{
	struct STRU_N_PhaseStatusGroup *gp = (struct STRU_N_PhaseStatusGroup *)udp_info.iValue;
	int i, j, datasize = sizeof(*gp) * 2;
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

	memset(gp, 0, datasize);
	ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	for (i = 0; i < 2; i++)	//总共2组状态
	{
		gp->byPhaseStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//每组包含8个相位的状态，1bit代表一个相位
		{
			switch (countDownParams.stVehPhaseCountingDown[i * 8 + j][0])
			{
				case GREEN_BLINK:
				case GREEN: gp->byPhaseStatusGroupGreens |= (1 << j); break;
				case YELLOW_BLINK: 
				case YELLOW: gp->byPhaseStatusGroupYellows |= (1 << j); break;
				case RED_BLINK: 
				case ALLRED:
				case RED: gp->byPhaseStatusGroupReds |= (1 << j); break;
			}
		}
		gp++;
	}
	sendto(socketFd, &udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

static inline void UploadFollowPhaseStatus(int socketFd, struct sockaddr_in fromAddr)
{
	struct STRU_N_OverlapStatusGroup *gp = (struct STRU_N_OverlapStatusGroup *)udp_info.iValue;
	int i, j, datasize = sizeof(*gp) * 2;
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

	memset(gp, 0, datasize);
	ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	for (i = 0; i < 2; i++)	//总共2组状态
	{
		gp->byOverlapStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//每组包含8个跟随相位的状态，1bit代表一个跟随相位
		{
			switch (countDownParams.ucOverlap[i * 8 + j][0])
			{
				case GREEN_BLINK:
				case GREEN: gp->byOverlapStatusGroupGreens |= (1 << j); break;
				case YELLOW_BLINK: 
				case YELLOW: gp->byOverlapStatusGroupYellows |= (1 << j); break;
				case RED_BLINK: 
				case ALLRED:
				case RED: gp->byOverlapStatusGroupReds |= (1 << j); break;
			}
		}
		gp++;
	}
	sendto(socketFd, &udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

static inline void UploadSyncStatus(int socketFd, struct sockaddr_in fromAddr)
{
	struct STRU_CoordinateStatus *gp = (struct STRU_CoordinateStatus *)udp_info.iValue;
	int datasize = sizeof(*gp);
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

	memset(gp, 0, datasize);
	ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	gp->byCoordPatternStatus = countDownParams.ucPlanNo;
	gp->wCoordCycleStatus = countDownParams.ucCurCycleTime - countDownParams.ucCurRunningTime;
	gp->wCoordSyncStatus = countDownParams.ucCurRunningTime;
	gp->byUnitControlStatus = ItsControlStatusGet();
	sendto(socketFd, &udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}


#define SA(x) ((struct sockaddr *)&x)
static int CreateUdpSocket(char *interface, UInt16 port)
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

/*********************************************************************************
*
* 	udp接收线程处理函数入口
*
***********************************************************************************/

void *CommunicationModule(void *arg)
{
	//创建UDP服务器
	int socketFd = -1;
	struct sockaddr_in fromAddr;
	ssize_t result = 0;
	sqlite3 *pdb = NULL;
	static time_t vehflow_time = 0;
	
    STRU_SPECIAL_PARAMS s_SpecialParams;
    

	struct STRU_N_PatternNew *communication = (struct STRU_N_PatternNew *)udp_info.iValue;
	struct UDP_CUR_VALUE_INFO *udp_cur_value_info = (struct UDP_CUR_VALUE_INFO *)&udp_info;
	struct CURRENT_PARAMS_UDP *current_params_udp_info = (struct CURRENT_PARAMS_UDP *)&udp_info;
	STEP_CTRL_PARAMS *stepCtrlParams = (STEP_CTRL_PARAMS *)&udp_info;
	CANCEL_STEP_FEEDBACK_PARAMS *cancelStepFeedback = (CANCEL_STEP_FEEDBACK_PARAMS *)&udp_info;
	
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;
	int specialControlSchemeId = 0;       //特殊控制方案号,用来上载控制方案号时使用
	struct SAdjustTime *timep = (struct SAdjustTime *)udp_info.iValue;
	UInt8 schemeId = 0;
	
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);
	
	memset(&udp_info,0,sizeof(struct UDP_INFO));

    socketFd = CreateUdpSocket(NULL, 20000);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	pthread_exit(NULL);
    }

	//init_db_file_tables();
	
    while(1)
    {
        memset(&udp_info,0,sizeof(udp_info));
        result = recvfrom(socketFd, &udp_info, sizeof(struct UDP_INFO), 0, (struct sockaddr *)&fromAddr, &fromLen);
		if(-1 == result)
		{
            ERR("############===>  Failed Error   %s\n",strerror(errno));
		}
        else
        {
//            INFO("CommunicationModule  0x%x \n",udp_info.iType);
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x3eb4)//心跳包
				sendto(socketFd, &udp_info, result, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xdf)
			{
				communication->wProtocol = 2;	//此协议是HIK协议
				communication->wDscType = gStructBinfileConfigPara.sSpecialParams.iSignalMachineType;
				communication->unPort = 20000;
				sendto(socketFd, &udp_info, sizeof(struct STRU_N_PatternNew) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//下载特殊检测参数
			{
				log_debug("Recv From %s : download special check parameters, value = %#x", inet_ntoa(fromAddr.sin_addr), udp_info.iValue[0]);
                DownloadSpecialParams(socketFd,fromAddr,&s_SpecialParams);
                //iIsSaveSpecialParams = 1;
				SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF_SPECIALPRM);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//上载特殊检测参数
			{
				//log_debug("upload special check parameters");
				udp_info.iValue[0] = gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch
					| (gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch << 1)
					| (gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch << 2)
					| (gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch << 3)
					| (gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch << 4)
					| (gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch << 5)
					| (gStructBinfileConfigPara.sSpecialParams.iGpsSwitch << 6)
					| (gStructBinfileConfigPara.sSpecialParams.iPhaseTakeOverSwtich << 7);
				
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//故障清除
			{
				//log_debug("clear fault log information");
                SendSuccessMsg(socketFd,fromAddr);
				ItsClearFaultLog();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)//上载红灯电流
			{
				//log_debug("upload red light current values");
				memcpy(udp_cur_value_info->redCurrentValue, gRedCurrentValue, sizeof(gRedCurrentValue));
				result = sendto(socketFd, &udp_info, sizeof(struct UDP_CUR_VALUE_INFO), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)//上载电流参数
			{
				//log_debug("upload red light current parameters");
				memcpy(current_params_udp_info->struRecCurrent,gStructBinfileConfigPara.sCurrentParams,sizeof(gStructBinfileConfigPara.sCurrentParams));
				result = sendto(socketFd, &udp_info, sizeof(struct CURRENT_PARAMS_UDP), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//下载电流参数
			{
				log_debug("Recv From %s : download red light current parameters", inet_ntoa(fromAddr.sin_addr));
				memcpy(gStructBinfileConfigPara.sCurrentParams, udp_info.iValue, sizeof(gStructBinfileConfigPara.sCurrentParams));
                SendSuccessMsg(socketFd,fromAddr);
                //iIsSaveSpecialParams = 1;
				SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//恢复默认参数
			{
				log_debug("Recv From %s : recover default parameters", inet_ntoa(fromAddr.sin_addr));
				system("rm -rf /home/*");
				system("rm -f /usr/hikconfig.db.bak /usr/extconfig.db.bak");
				SendSuccessMsg(socketFd,fromAddr);
				init_db_file_tables();
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
			{		
				log_debug("Recv From %s : set ip address", inet_ntoa(fromAddr.sin_addr));
#if defined(__linux__) && defined(__arm__)	//这是arm交叉编译gcc内置的宏定义
                DownloadIpAddress(socketFd,fromAddr);//下载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
#endif
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{
				//log_debug("get ip address");
                UploadIpAddress(socketFd,fromAddr);//上载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//下载相位描述
			{
				log_debug("REcv From %s : download phase describe", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileDesc.sPhaseDescParams,&udp_info,sizeof(gStructBinfileDesc.sPhaseDescParams));				
				memcpy(gStructBinfileDesc.phaseDescText[0], &gStructBinfileDesc.sPhaseDescParams.stPhaseDesc, sizeof(gStructBinfileDesc.sPhaseDescParams.stPhaseDesc));
                SendSuccessMsg(socketFd,fromAddr);
                //iIsSaveDescParams = 1;
				SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PHASE);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//上载相位描述
			{
				//log_debug("upload phase describe");
				gStructBinfileDesc.sPhaseDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPhaseDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPhaseDescParams, sizeof(gStructBinfileDesc.sPhaseDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//下载通道描述
			{
				log_debug("Recv From %s : download channel describe", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileDesc.sChannelDescParams,&udp_info,sizeof(gStructBinfileDesc.sChannelDescParams));
                SendSuccessMsg(socketFd,fromAddr);
                //iIsSaveDescParams = 1;
                SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_CHANNEL);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//上载通道描述
			{
				//log_debug("upload channel describe");
				gStructBinfileDesc.sChannelDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sChannelDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sChannelDescParams, sizeof(gStructBinfileDesc.sChannelDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//下载方案描述
			{
				log_debug("Recv From %s : download scheme describe", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileDesc.sPatternNameParams,&udp_info,sizeof(gStructBinfileDesc.sPatternNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                //iIsSaveDescParams = 1;
				SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PATTERN);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//上载方案描述
			{
				//log_debug("upload scheme describe");
				gStructBinfileDesc.sPatternNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPatternNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPatternNameParams, sizeof(gStructBinfileDesc.sPatternNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//下载计划描述
			{
				log_debug("Recv From %s : download plan describe", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileDesc.sPlanNameParams,&udp_info,sizeof(gStructBinfileDesc.sPlanNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                //iIsSaveDescParams = 1;
                SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_PLAN);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//上载计划描述
			{
				//log_debug("upload plan describe");
				gStructBinfileDesc.sPlanNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPlanNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPlanNameParams, sizeof(gStructBinfileDesc.sPlanNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//下载日期描述
			{
				log_debug("Recv From %s : download date describe", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileDesc.sDateNameParams,&udp_info,sizeof(gStructBinfileDesc.sDateNameParams));
				SendSuccessMsg(socketFd,fromAddr);
				//iIsSaveDescParams = 1;
				SET_BIT(iIsSaveDescParams, E_TABLE_NAME_DESC_DATE);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//上载日期描述
			{
				//log_debug("upload date describe");
				gStructBinfileDesc.sDateNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sDateNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sDateNameParams, sizeof(gStructBinfileDesc.sDateNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)//上载软硬件版本信息
			{
                UploadVersionInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)//下载倒计时牌设置
			{
				log_debug("Recv From %s : download countdown set information", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileCustom.sCountdownParams,udp_info.iValue,sizeof(gStructBinfileCustom.sCountdownParams));
				//iIsSaveCustomParams = 1;
				SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_COUNTDOWN);
				SendSuccessMsg(socketFd,fromAddr);
				//WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			/*	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
				sqlite3_clear_table(pdb, TABLE_NAME_CUSTOM_COUNTDOWN);
				sqlite3_insert_countdown_prm(pdb, &gStructBinfileCustom.sCountdownParams);
				sqlite3_close_wrapper(pdb); pdb = NULL;*/
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//上载倒计时牌设置
			{
				//log_debug("upload countdown set information");
				memcpy(udp_info.iValue,&gStructBinfileCustom.sCountdownParams,sizeof(gStructBinfileCustom.sCountdownParams));
				result = sendto(socketFd, &udp_info, sizeof(gStructBinfileCustom.sCountdownParams) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//下载串口配置
			{
				log_debug("Recv From %s : download serial config", inet_ntoa(fromAddr.sin_addr));
                if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
			    {
    				memcpy(&gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1],&udp_info,sizeof(COM_PARAMS));
					//iIsSaveCustomParams = 1;
					SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_COM);
    				SendSuccessMsg(socketFd,fromAddr);
    				//WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
    				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
					sqlite3_clear_table(pdb, TABLE_NAME_CUSTOM_COM);
					sqlite3_insert_com_prm(pdb, gStructBinfileCustom.sComParams);
					sqlite3_close_wrapper(pdb); pdb = NULL;
					sqlite3_bak_db(DATABASE_EXTCONFIG);
			    }
			    else
			    {
                    result = -1;
			    }
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//上载串口配置
			{
				//log_debug("upload serial config");
			    if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
			    {
                    gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1].unExtraParamHead = udp_info.iHead;
                    gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1].unExtraParamID = udp_info.iType;
			    	result = sendto(socketFd, &gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1], sizeof(COM_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			    }
			    else
			    {
                    result = -1;
			    }
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)//获取倒计时参数信息
			{
				ItsCountDownGet(&countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
				if(gStructBinfileCustom.cChannelLockFlag != 1 || gStructBinfileCustom.sChannelLockedParams.ucReserved == 1)//reserved==1 表示可变车道牌控制
					countDownParams.ucChannelLockStatus = 0;
                if(countDownParams.ucPlanNo > 0 && countDownParams.ucPlanNo <= NUM_SCHEME)
                {
                    if((countDownParams.ucPlanNo-1)%3 == 0)//普通定周期方案是可以让用户自定义描述的
                    {
                        if((countDownParams.ucPlanNo <= 47) && (strlen((char *)gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]) > 0))
					        memcpy(countDownParams.ucCurPlanDsc, gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3], sizeof(countDownParams.ucCurPlanDsc));	//添加方案描述
                        else
                            snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"定周期 %d",(countDownParams.ucPlanNo+2)/3);
                    }
                    else if((countDownParams.ucPlanNo-2)%3 == 0)//感应
                    {
                        if((countDownParams.ucPlanNo <= 47) && (strlen((char *)gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]) > 0))
                            snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"感应 %s",gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3]);
                        else
                            snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"感应 %d",(countDownParams.ucPlanNo+1)/3);
						countDownParams.ucPlanNo = INDUCTIVE_SCHEMEID;	//感应控制时倒计时默认返回方案254
                    }
                    else if(countDownParams.ucPlanNo%3 == 0) //协感
                    {
                        if((countDownParams.ucPlanNo <= 47) && (strlen((char *)gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3 - 1]) > 0))
                            snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"协感 %s",gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo/3 - 1]);
                        else
                            snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"协感 %d",countDownParams.ucPlanNo/3);
						countDownParams.ucPlanNo = INDUCTIVE_COORDINATE_SCHEMEID;//协感控制时倒计时默认返回方案250
                    }
                }

				result = sendto(socketFd, &countDownParams, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS),
						 MSG_DONTWAIT, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//信号机重启
			{
				log_debug("Recv From %s : System will restart", inet_ntoa(fromAddr.sin_addr));
				ItsWriteFaultLog(UNNORMAL_OR_SOFTWARE_REBOOT, 0);
				SendSuccessMsg(socketFd,fromAddr);
				sync();
				sleep(1);
#if defined(__linux__) && defined(__arm__)	//这是arm交叉编译gcc内置的宏定义
				system("reboot");
#endif
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK)//开启通道锁定
			{
				log_debug("Recv From %s : enable channel lock", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileCustom.sChannelLockedParams,&udp_info,sizeof(gStructBinfileCustom.sChannelLockedParams));
			    gStructBinfileCustom.cChannelLockFlag = 1;
			    //WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    //iIsSaveCustomParams = 1;
				SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_CHANNELLOCK);
			/*	sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
				sqlite3_clear_table(pdb, TABLE_NAME_CUSTOM_CHANNELLOCK);
				sqlite3_insert_chlock_prm(pdb, &gStructBinfileCustom.sChannelLockedParams);
				sqlite3_update_column(pdb, TABLE_NAME_CUSTOM_CHANNELLOCK, "cChannelLockFlag", 1, &(gStructBinfileCustom.cChannelLockFlag),  1, SQLITE_INTEGER);
				sqlite3_close_wrapper(pdb); pdb = NULL;*/
				SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_UNLOCK)//关闭通道锁定
			{
				switch(udp_info.iValue[0])
				{
					case 0://原通道解锁
						log_debug("Recv From %s : disable channel lock", inet_ntoa(fromAddr.sin_addr));
						if(gStructBinfileCustom.cChannelLockFlag == 1)
						{
							gStructBinfileCustom.cChannelLockFlag = 0;
							//ItsChannelUnlock();
						}
						break;
					case 1://多时段通道锁定解锁
						log_debug("Recv From %s : Disable mult periods channel lock", inet_ntoa(fromAddr.sin_addr));
						gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 0;
						break;
					case 2://多时段通道锁定恢复
						log_debug("Recv From %s : Recover mult periods channel lock", inet_ntoa(fromAddr.sin_addr));
						gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 1;
						break;
					default:
						log_debug("Recv From %s : Unknown Msg type in ChanUnlock!", inet_ntoa(fromAddr.sin_addr));
						break;
				}
				//iIsSaveCustomParams = 1;
				
			    SendSuccessMsg(socketFd,fromAddr);
			    //WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
				//sqlite3_update_column(pdb, TABLE_NAME_CUSTOM_CHANNELLOCK, "cChannelLockFlag", 1, &(gStructBinfileCustom.cChannelLockFlag),  1, SQLITE_INTEGER);
				sqlite3_update_column(pdb, TABLE_NAME_CUSTOM_MULCHANNELLOCK, "cLockFlag", 1, &(gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag),  1, SQLITE_INTEGER);
				sqlite3_close_wrapper(pdb); pdb = NULL;
				sqlite3_bak_db(DATABASE_EXTCONFIG);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd7)//设置降级参数
			{
				log_debug("Recv From %s : set demotion parameters", inet_ntoa(fromAddr.sin_addr));
				memcpy(&gStructBinfileCustom.demotionParams,&udp_info,sizeof(gStructBinfileCustom.demotionParams));
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd8)//获取降级参数
			{
				//log_debug("get demotion parameters");
				gStructBinfileCustom.demotionParams.unExtraParamID = 0xd8;
				result = sendto(socketFd, &gStructBinfileCustom.demotionParams, sizeof(gStructBinfileCustom.demotionParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//手动特殊控制命令
			{
				schemeId = udp_info.iValue[0];
				log_debug("Recv From %s : manual special control command, schemeid = %d", inet_ntoa(fromAddr.sin_addr), schemeId);
				if ((schemeId > 0) && (schemeId <= NUM_SCHEME) && (2 == schemeId % 3))	//手动感应方案控制
					ItsCtlNonblock(TOOL_CONTROL, INDUCTIVE_SCHEMEID, schemeId);
				else if ((schemeId > 0) && (schemeId <= NUM_SCHEME) && (0 == schemeId % 3))	//手动协调感应方案控制
					ItsCtlNonblock(TOOL_CONTROL, INDUCTIVE_COORDINATE_SCHEMEID, schemeId);
				else	//其他黄闪、全红、关灯、步进或手动执行定周期方案控制
					ItsCtlNonblock(TOOL_CONTROL, schemeId, schemeId);
				specialControlSchemeId = schemeId;	//存下方案号用来上载时使用
				gInductiveDemotion = 0;//reset demotion flag
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6d)//获取当前控制方案号
			{
			    udp_info.iValue[0] = specialControlSchemeId;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//信号机步进控制
			{
				log_debug("Recv From %s : excute step control, stageNO = %d", inet_ntoa(fromAddr.sin_addr), stepCtrlParams->unStepNo);
				ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, stepCtrlParams->unStepNo);
				stepCtrlParams->unStepNo = 1;	//表示步进成功
				result = sendto(socketFd, stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); 
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//信号机取消步进
			{
				log_debug("Recv From %s : cancel step control", inet_ntoa(fromAddr.sin_addr));
				ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, -1);	//取消步进时步进号为-1
				specialControlSchemeId = 0;	//取消步进时默认进行系统控制
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//设置经济型信号机型号
			{
			    gStructBinfileConfigPara.sSpecialParams.iSignalMachineType = udp_info.iValue[0];
			    //iIsSaveSpecialParams = 1;
				SET_BIT(iIsSaveSpecialParams, E_TABLE_NAME_CONF_SPECIALPRM);
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//获取经济型信号机型号
			{
			    udp_info.iValue[0] = gStructBinfileConfigPara.sSpecialParams.iSignalMachineType;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x71)//下载对时
			{
				struct tm settime;
				char timebuf[64] = {0};

                gStructBinfileMisc.time_zone_gap = timep->unTimeZone;
//                INFO("**********************************\n");
//                INFO("recv ulGlobalTime = %ld, unTimeZone = %d\n",timep->ulGlobalTime,timep->unTimeZone);
//                INFO("time_zone_gap = %d\n",gStructBinfileMisc.time_zone_gap);
//                INFO("**********************************\n");
                //WRITE_BIN_CFG_PARAMS(FILE_MISC_DAT,&gStructBinfileMisc,sizeof(STRUCT_BINFILE_MISC));
                sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
				sqlite3_update_column(pdb, TABLE_NAME_MISC, "time_zone_gap", 1, &(gStructBinfileMisc.time_zone_gap),  4, SQLITE_INTEGER);
				sqlite3_close_wrapper(pdb); pdb = NULL;
				sqlite3_bak_db(DATABASE_EXTCONFIG);
				timep->ulGlobalTime += timep->unTimeZone;
				stime((time_t *)&timep->ulGlobalTime);
				system("hwclock -w");
				localtime_r((time_t *)&timep->ulGlobalTime, &settime);
				strftime(timebuf, sizeof(timebuf), "%F %T", &settime);
				log_debug("Recv From %s : set system time: %s", inet_ntoa(fromAddr.sin_addr), timebuf);
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x72)//上载对时
			{
				//log_debug("get system time");
//				timep->ulGlobalTime = time(NULL) - 8 * 3600;
//				timep->unTimeZone = 8 * 3600;

                timep->ulGlobalTime = time(NULL) - gStructBinfileMisc.time_zone_gap;
				timep->unTimeZone = gStructBinfileMisc.time_zone_gap;
//                INFO("upload unTimeZone = %d\n",timep->unTimeZone);
				result = sendto(socketFd, &udp_info, 8 + sizeof(*timep), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6c)//通道检测
			{
				log_debug("Recv From %s : excute channel check, terminal = %d", inet_ntoa(fromAddr.sin_addr), udp_info.iValue[0]);
				ChannelCheckDeal(udp_info.iValue[0]);
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x159)//上载通道状态
			{
			    UploadChannelStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x157)//上载相位状态
			{
			    UploadPhaseStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x2b)//上载跟随相位状态
			{
			    UploadFollowPhaseStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15a)//上载同步状态
			{
			    UploadSyncStatus(socketFd,fromAddr);
			}	
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd0)//实时运行方案信息
			{
				//log_debug("get real time running scheme information");
				ItsGetRealtimePattern(&udp_info);
                result = sendto(socketFd, &udp_info, sizeof(MsgRealtimePattern), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd1)//实时流量统计获取接口
			{
                pthread_rwlock_rdlock(&gLockRealtimeVol);
                gStructMsgRealtimeVolume.unExtraParamHead = 0x6e6e;
                gStructMsgRealtimeVolume.unExtraParamID = 0xd1;
				time(&vehflow_time);
				if (gStructMsgRealtimeVolume.volumeOccupancy.dwTime == 0)
				{
					gStructMsgRealtimeVolume.volumeOccupancy.dwTime = vehflow_time;
				}
				else
				{
					UINT32 vehflow_cycletime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);
					if (vehflow_time - gStructMsgRealtimeVolume.volumeOccupancy.dwTime > vehflow_cycletime)
					{
						gStructMsgRealtimeVolume.volumeOccupancy.dwTime = vehflow_time - (vehflow_time - gStructMsgRealtimeVolume.volumeOccupancy.dwTime) % vehflow_cycletime;
						memset(gStructMsgRealtimeVolume.volumeOccupancy.struVolume, 0, sizeof(gStructMsgRealtimeVolume.volumeOccupancy.struVolume));
					}
				}

/*    INFO("总数: %d 辆, 时间占有率: %0.2f%%, 平均车速: %0.2f km/h, 排队长度: %0.2f m, 车流密度: %0.2f 辆/km, \n\t\t\t车头间距: %0.2f m, 车头时距: %0.2f s, 绿损: %d s\n"
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].byDetectorVolume
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].byDetectorOccupancy/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].byVehicleSpeed/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].wQueueLengh/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].wVehicleDensity/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].wVehicleHeadDistance/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].wVehicleHeadTimeDistance/100.0
                                                ,gStructMsgRealtimeVolume.volumeOccupancy.struVolume[0].wGreenLost/100);
*/
                result = sendto(socketFd, &gStructMsgRealtimeVolume, sizeof(gStructMsgRealtimeVolume), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
                pthread_rwlock_unlock(&gLockRealtimeVol);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd2)//平台1049协议
			{
                //以下是对1049协议的封装
                MsgPhaseSchemeId cStructMsg1049;
                Set1049MsgContent(&cStructMsg1049);
                result = sendto(socketFd, &cStructMsg1049, sizeof(cStructMsg1049), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeec)//给触摸板发送通道倒计时信息
			{
                UploadChannelCountdown(socketFd,fromAddr);
                //INFO("get countdown information");
            }
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_SET)//多时段通道锁定(解锁和原通道锁定共用一个消息)
			{
				if(gStructBinfileCustom.cChannelLockFlag == 1)//原通道已处于锁定状态不生效
				{
					log_debug("Recv From %s : Can't override realtime channel lock", inet_ntoa(fromAddr.sin_addr));
					SendFailureMsg(socketFd,fromAddr);
				}
				else
				{
					log_debug("Recv From %s : Enable mult periods channel lock", inet_ntoa(fromAddr.sin_addr));
					memcpy((char*)gStructBinfileCustom.sMulPeriodsChanLockParams.chans,(char*)udp_info.iValue,sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans));
				    gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 1;
				    //WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				  /*  sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
					sqlite3_clear_table(pdb, TABLE_NAME_CUSTOM_MULCHANNELLOCK);
					sqlite3_insert_mp_chlock_prm(pdb, &gStructBinfileCustom.sMulPeriodsChanLockParams);
					sqlite3_close_wrapper(pdb); pdb = NULL;*/
					//iIsSaveCustomParams = 1;
					SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_MULCHANNELLOCK);
				    SendSuccessMsg(socketFd,fromAddr);
					//log_debug("Mult periods channel lock");
				}
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_GET)//多时段通道锁定查询
			{
				//log_debug("Get mult periods channel lock info");
				memcpy(&udp_info.iValue[0], (char*)gStructBinfileCustom.sMulPeriodsChanLockParams.chans, sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans));
				*((char *)udp_info.iValue + sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans)) = gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag; 	
				result = sendto(socketFd, &udp_info, 12+sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK_STATUS_GET)//通道锁定状态获取
			{
				STRU_CHAN_LOCK_PARAMS curLockStatus;
				memset(&curLockStatus, 0, sizeof(STRU_CHAN_LOCK_PARAMS));
				GetCurChanLockStatus(&curLockStatus);
				memcpy(&udp_info.iValue[0], (char*)&curLockStatus, sizeof(STRU_CHAN_LOCK_PARAMS));
				result = sendto(socketFd, &udp_info, 8+sizeof(STRU_CHAN_LOCK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == EXTEND_MSG_HEAD)
			{
				STRU_EXTEND_UDP_MSG *msg = (STRU_EXTEND_UDP_MSG*)&udp_info;
				//INFO("-->udp recv: %x,%x,%x,%x", udp_info.iHead, udp_info.iType, udp_info.iValue[0],udp_info.iValue[1]);
				//INFO("-->udp recv xml: %s", msg->xml);
				XMLMsgHandle(msg);
				//INFO("-->udp send xml: %s", msg->xml);
				result = sendto(socketFd, &udp_info, msg->len+8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				sqlite3_bak_db(DATABASE_EXTCONFIG);
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CAMERA_FLOW_SERVER_SET)//获取相机流量上传服务器信息
			{
				int i;
				switch(udp_info.iValue[7])
				{
					case MSG_ORDERED:
						for(i = 0; i < MAX_SDK_SERVER; i++)
						{
							if((0 == strncmp(gStructBinfileCustom.sCameraFlowServer[i].sAddress, (char*)udp_info.iValue, 16)
								&& (udp_info.iValue[4] == gStructBinfileCustom.sCameraFlowServer[i].iPort))
								|| (NO_MSG_ORDERED == gStructBinfileCustom.sCameraFlowServer[i].uIsOrder))
								break;
						}
						if(MAX_SDK_SERVER > i)
						{
							memcpy(gStructBinfileCustom.sCameraFlowServer[i].sAddress, (char*)udp_info.iValue, 16);
							gStructBinfileCustom.sCameraFlowServer[i].iPort = udp_info.iValue[4];
							gStructBinfileCustom.sCameraFlowServer[i].iSDKID = udp_info.iValue[5];
							gStructBinfileCustom.sCameraFlowServer[i].uMsgType =  udp_info.iValue[6];
							gStructBinfileCustom.sCameraFlowServer[i].uIsOrder = udp_info.iValue[7];
							INFO("New Server added for camera flow, ip:%s, port:%d...", gStructBinfileCustom.sCameraFlowServer[i].sAddress, gStructBinfileCustom.sCameraFlowServer[i].iPort);
						}
						else
							INFO("No space left for new record!...");
						break;
					case NO_MSG_ORDERED:
						for(i = 0; i < MAX_SDK_SERVER; i++)
						{
							if(0 == strncmp(gStructBinfileCustom.sCameraFlowServer[i].sAddress, (char*)udp_info.iValue, 16)
								&& (udp_info.iValue[4] == gStructBinfileCustom.sCameraFlowServer[i].iPort))
							{
								INFO("Delete Server record of camera flow, ip:%s, port:%d...", gStructBinfileCustom.sCameraFlowServer[i].sAddress, gStructBinfileCustom.sCameraFlowServer[i].iPort);
								memset(&gStructBinfileCustom.sCameraFlowServer[i], 0, sizeof(STRU_CAMERA_FLOW_SERVER_PARAMS));
							}
						}
						break;
					default:
						break;
				}
				SendSuccessMsg(socketFd, fromAddr);
				SET_BIT(iIsSaveCustomParams, E_TABLE_NAME_CUSTOM_CAMERASERVER);

				sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
				sqlite3_clear_table(pdb, TABLE_NAME_CUSTOM_CAMERASERVER);
				sqlite3_insert_cameraServer_prm(pdb, gStructBinfileCustom.sCameraFlowServer);
				sqlite3_close_wrapper(pdb); pdb = NULL;
				sqlite3_bak_db(DATABASE_EXTCONFIG);
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_TOUCH_CONFIG_APP)//触摸屏配置工具消息
			{
				STRU_TOUCH_BOARD_MSG *tcmsg = (STRU_TOUCH_BOARD_MSG *)udp_info.iValue;
				//INFO("Touchconfig: download/upload config ....\n");
				result = TouchConfig_msgHandle(tcmsg);
				result = sendto(socketFd, &udp_info, result+8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else
			{
				UpAndDownLoadDeal(socketFd, fromAddr);	//上下载处理
				if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
				{
#ifdef USE_GB_PROTOCOL
					NtcipConvertToGb();
#endif				
					//保存参数到配置文件
					if(iIsSaveCustomParams > 0)
					{
						save_custom_to_database(&gStructBinfileCustom);
						//log_debug("update custom to database done");
						iIsSaveCustomParams = 0;
					/*	//WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
						
						sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
						write_custom(pdb, &gStructBinfileCustom);
						sqlite3_close_wrapper(pdb); pdb = NULL;*/
					}
					if(iIsSaveDescParams > 0)
					{
						save_desc_to_database(&gStructBinfileDesc);
						//log_debug("update desc to database done");
						iIsSaveDescParams = 0;
						/*//WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC));
						
						sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
						write_desc(pdb, &gStructBinfileDesc);
						sqlite3_close_wrapper(pdb); pdb = NULL;*/
					}
					if(iIsSaveSpecialParams > 0)
					{
						save_conf_to_database(&gStructBinfileConfigPara);
						//log_debug("update config to database done");
						iIsSaveSpecialParams = 0;
					/*	//WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
						
						sqlite3_open_wrapper(DATABASE_EXTCONFIG, &pdb);
						write_config(pdb, &gStructBinfileConfigPara);
						sqlite3_close_wrapper(pdb); pdb = NULL;*/
					}
					sqlite3_bak_db(DATABASE_EXTCONFIG);
				}
			}
			
			if (result == -1)
    		{
				ERR("sendto udp info error!!!\n");
    		}
			usleep(10000);
        }
        
    }
}

/******************************************************
**	单点自适应功能信号机设备端定制
**   上传消息内容为:   json 文本
**   处理:   将json文本中的时间改为当前时间
******************************************************/
int gCameraFlowUploadSocket = -1;
void CameraFlowUploadInit(void)
{
	gCameraFlowUploadSocket = CreateUdpSocket(NULL, 50000);
	if ( -1 == gCameraFlowUploadSocket )
    {
		printf("socket udp init error!!!\n");
    }
}
void MsgUpdateTime(char *jsonText)
{
	struct tm * timelocal = NULL;
	time_t secTime = time(NULL);
	char *pos = NULL;
	char buff[32] = {0};
	if(NULL == jsonText)
		return;

	timelocal = localtime(&secTime);
	if(NULL != (pos = strstr(jsonText, "\"time\":")))
	{
		sprintf(buff, "%04d-%02d-%02dT%02d:%02d:%02d", timelocal->tm_year + 1900, timelocal->tm_mon + 1, timelocal->tm_mday,
				timelocal->tm_hour, timelocal->tm_min, timelocal->tm_sec);
		//INFO("--->Time String: %s", buff);
		memcpy(pos + 8, buff, 19); // 8 -> sizeof("time":) , exclude timezone
	}
	else
		ERR("Can't resolove 'time' string in json text!");
	//INFO("--> Done: %s ",jsonText);
}
void SendCameraFlow(char *data, int len)
{
	struct sockaddr_in servAddr;
	char buff[1024*15] = {0};
	STRU_EXTEND_UDP_MSG *msg = (STRU_EXTEND_UDP_MSG*)buff;
	int i;
	
	if(data == NULL)
		return;
	
	while(-1 == gCameraFlowUploadSocket)
	{
		INFO("Upload socket creating for camera flow info...");
		CameraFlowUploadInit();
		sleep(1);
	}
	
	msg->head = 0x6e6d;
	msg->len = len;
	MsgUpdateTime(data);
	memcpy(msg->xml, data, len);
	for(i = 0; i < MAX_SDK_SERVER; i++)
	{
		if(MSG_ORDERED == gStructBinfileCustom.sCameraFlowServer[i].uIsOrder
			&& MSG_UPLOAD_CAMERA_FLOW == gStructBinfileCustom.sCameraFlowServer[i].uMsgType)
		{
			memset(&servAddr, 0, sizeof(struct sockaddr_in));
			msg->checksum = gStructBinfileCustom.sCameraFlowServer[i].iSDKID;
			servAddr.sin_family = AF_INET;
			servAddr.sin_addr.s_addr = inet_addr(gStructBinfileCustom.sCameraFlowServer[i].sAddress);
			servAddr.sin_port = htons(gStructBinfileCustom.sCameraFlowServer[i].iPort);
			sendto(gCameraFlowUploadSocket, msg, msg->len + 8, 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
		}
	}
}
int TouchConfig_uploadAll(STRU_TOUCH_BOARD_MSG *msg)
{
	int i,j;
	SignalControllerPara config;
	int offset = 0;
	int tmp= MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry);
	//char *p = (char *)&udp_info.iValue[2];
	char *p = (char *)msg->data;

	INFO("start to get config info...\n");
	memset(&config, 0, sizeof(SignalControllerPara));
	ItsGetConfig(&config, sizeof(SignalControllerPara));
	//相位配置
	//memcpy(msg->data, (char*)config.stPhase, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem));
	offset += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem);
	memcpy(p, (char*)config.stPhase, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem));
	p += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem);
	INFO("phase info done...%d,%d\n",offset, tmp);
/*
    for(i=0; i<MAX_PHASE_TABLE_COUNT; i++)
    {
        INFO("相位表%d:\n",i);
        for(j=0; j<NUM_PHASE; j++)
        {
            PhaseItem *item = &config.stPhase[i][j];
            INFO("==相位%d: ID=%d,Ring=%d,Y=%d,AR=%d,MinG=%d,MaxG1=%d,MaxG2=%d,ExtG=%d,PedGo=%d,PedClear=%d, Auto[veh:%d, ped:%d]\n",
                   j, item->nPhaseID, item->nCircleID, item->nYellowTime, item->nAllRedTime, item->nMinGreen, item->nMaxGreen_1, item->nMaxGreen_2,
                   item->nUnitExtendGreen,item->nPedestrianPassTime, item->nPedestrianClearTime, GET_BIT(item->wPhaseOptions, 9), GET_BIT(item->wPhaseOptions, 11));
        }
    }
	*/
	//信号转换序列
	//memcpy(msg->data + offset, (char*)config.AscSignalTransTable, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
	offset += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry);
	memcpy(p, (char*)config.AscSignalTransTable, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
	p += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry);
	INFO("signaltrans info done...%d\n",offset);
	//绿信比配置
	//memcpy(msg->data + offset, (char*)config.stGreenSignalRation, NUM_GREEN_SIGNAL_RATION*NUM_PHASE*sizeof(GreenSignalRationItem));
	offset += /*NUM_GREEN_SIGNAL_RATION*/MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(GreenSignalRationItem);
	memcpy(p, (char*)config.stGreenSignalRation, /*NUM_GREEN_SIGNAL_RATION*/MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(GreenSignalRationItem));
	p += /*NUM_GREEN_SIGNAL_RATION*/MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(GreenSignalRationItem);
	INFO("GreenSignalRation info done...%d\n",offset);
	//通道配置
	//memcpy(msg->data+offset, (char*)config.stChannel, MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem));
	offset += MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem);
	memcpy(p, (char*)config.stChannel, MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem));
	p += MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem);
	INFO("stChannel info done...%d\n",offset);
	//方案配置
	//memcpy(msg->data+offset, (char*)config.stScheme, NUM_SCHEME*sizeof(SchemeItem));
	offset += NUM_SCHEME*sizeof(SchemeItem);
	memcpy(p, (char*)config.stScheme, NUM_SCHEME*sizeof(SchemeItem));
	p += NUM_SCHEME*sizeof(SchemeItem);
	INFO("stScheme info done...%d\n",offset);
	//动作配置
	//memcpy(msg->data + offset, (char*)config.stAction, NUM_ACTION*sizeof(ActionItem));
	offset += NUM_ACTION*sizeof(ActionItem);
	memcpy(p, (char*)config.stAction, NUM_ACTION*sizeof(ActionItem));
	p += NUM_ACTION*sizeof(ActionItem);
	INFO("stAction info done...%d\n",offset);
	//相序配置
	//memcpy(msg->data + offset, (char*)config.stPhaseTurn, NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem));
	offset += NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem);
	memcpy(p, (char*)config.stPhaseTurn, NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem));
	p += NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem);
	INFO("stPhaseTurn info done...\n%d",offset);

	//INFO("---------------------------------");
	//INFO("touchUPLOAD PhaseTurn0: ring1:%d,%d,%d. ring2:%d,%d,%d\n", config.stPhaseTurn[0][0].nTurnArray[0], config.stPhaseTurn[0][0].nTurnArray[1], config.stPhaseTurn[0][0].nTurnArray[2],
	//		config.stPhaseTurn[0][1].nTurnArray[0], config.stPhaseTurn[0][1].nTurnArray[1], config.stPhaseTurn[0][1].nTurnArray[2]);
	//INFO("---------------------------------");
	
	//计划配置
	//memcpy(msg->data + offset, (char*)config.stTimeInterval, NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem));
	offset += NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem);
	memcpy(p, (char*)config.stTimeInterval, NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem));
	p += NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem);
	INFO("stTimeInterval info done...%d\n",offset);

	/*for(i=0; i<16; i++)
		for(j=0; j<48; j++)
		{
			TimeIntervalItem *item = &config.stTimeInterval[i][j];
            INFO("==时段表%d 时段%d: time=%d:%d,intervalID=%d,timeID=%d, actionID=%d\n",
                   i,j, item->cStartTimeHour, item->cStartTimeMinute, item->nTimeIntervalID, item->nTimeID,item->nActionID);	
		}
	*/
	//日期调度配置
	//memcpy(msg->data + offset, (char*)config.stPlanSchedule, NUM_SCHEDULE*sizeof(PlanScheduleItem));
	offset += /*NUM_SCHEDULE*/10*sizeof(PlanScheduleItem);
	memcpy(p, (char*)config.stPlanSchedule, /*NUM_SCHEDULE*/10*sizeof(PlanScheduleItem));
	p += /*NUM_SCHEDULE*/10*sizeof(PlanScheduleItem);
	INFO("stPlanSchedule info done...%d\n",offset);
	//车检器配置
	//memcpy(msg->data + offset, (char*)config.AscVehicleDetectorTable, MAX_VEHICLEDETECTOR_COUNT*sizeof(struct STRU_N_VehicleDetector));
	offset += /*MAX_VEHICLEDETECTOR_COUNT*/48*sizeof(struct STRU_N_VehicleDetector);
	memcpy(p, (char*)config.AscVehicleDetectorTable, /*MAX_VEHICLEDETECTOR_COUNT*/48*sizeof(struct STRU_N_VehicleDetector));
	p += /*MAX_VEHICLEDETECTOR_COUNT*/48*sizeof(struct STRU_N_VehicleDetector);
	INFO("VehicleDetector info done...%d\n",offset);
	//单元参数
	//memcpy(msg->data+offset, (char*)&config.stUnitPara, sizeof(UnitPara));
	offset += sizeof(UnitPara);
	memcpy(p, (char*)&config.stUnitPara, sizeof(UnitPara));
	p += sizeof(UnitPara);
	INFO("UnitPara info done...%d\n",offset);
	msg->len = offset;	

	INFO("Get config info done!\n");
	return offset+8;
}
void TouchConfig_downloadAll(STRU_TOUCH_BOARD_MSG *msg)
{
	int offset = 0;
	int i = 0;

	//相位配置
	memcpy(gSignalControlpara->stPhase, msg->data, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem));
	memcpy(gSignalControlpara->stOldPhase, gSignalControlpara->stPhase[0], NUM_PHASE*sizeof(PhaseItem));
	offset = MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem);
	//信号转换序列
	memcpy(gSignalControlpara->AscSignalTransTable, msg->data + offset,	MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
	memcpy(gSignalControlpara->OldAscSignalTransTable, gSignalControlpara->AscSignalTransTable[0], NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
	offset += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry);
	//绿信比配置
	memcpy(gSignalControlpara->stGreenSignalRation, msg->data + offset,  16/*NUM_GREEN_SIGNAL_RATION*/*NUM_PHASE*sizeof(GreenSignalRationItem));
	offset += 16/*NUM_GREEN_SIGNAL_RATION*/*NUM_PHASE*sizeof(GreenSignalRationItem);
	//通道配置
	memcpy(gSignalControlpara->stChannel, msg->data+offset, MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem));
	memcpy(gSignalControlpara->stOldChannel, gSignalControlpara->stChannel[0], NUM_CHANNEL*sizeof(ChannelItem));
	offset += MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem);

	//方案配置
	SchemeItem *item = NULL;
	memcpy(gSignalControlpara->stScheme, msg->data+offset, NUM_SCHEME*sizeof(SchemeItem));
	item = gSignalControlpara->stScheme;
	for (i = 0; i < NUM_SCHEME; i++)
	{
		if (item[i].nSchemeID > 0 && item[i].nSchemeID <= NUM_SCHEME)
		{
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nSchemeID = item[i].nSchemeID;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nCycleTime = item[i].nCycleTime;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nOffset = item[i].nOffset;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nGreenSignalRatioID = item[i].nGreenSignalRatioID;
			gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nPhaseTurnID = item[i].nPhaseTurnID;
			INFO("SchemeID:%d, cycletime:%d, greenRatioID:%d, phaseTurnID:%d.", item[i].nSchemeID, item[i].nCycleTime, item[i].nGreenSignalRatioID, item[i].nPhaseTurnID);
		}
	}
	offset += NUM_SCHEME*sizeof(SchemeItem);
	//动作配置
	//memcpy(gSignalControlpara->stAction, msg.data + offset,  NUM_ACTION*sizeof(ActionItem));
	ActionItem *item1 = (ActionItem *)(msg->data + offset);
	for (i = 0; i < NUM_ACTION; i++)
	{
		if (item1[i].nActionID > 0 && item1[i].nActionID <= NUM_ACTION)
		{
			if (item1[i].nPhaseTableID == 0 || item1[i].nPhaseTableID > MAX_PHASE_TABLE_COUNT)
				item1[i].nPhaseTableID = 1;
			if (item1[i].nChannelTableID == 0 || item1[i].nChannelTableID > MAX_CHANNEL_TABLE_COUNT)
				item1[i].nChannelTableID = 1;
			gSignalControlpara->stAction[item1[i].nActionID - 1] = item1[i];
		}
	}
	offset += NUM_ACTION*sizeof(ActionItem);
	
	//相序配置
	memcpy(gSignalControlpara->stPhaseTurn, msg->data + offset,	NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem));
	offset += NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem);

	//INFO("---------------------------------");
	//INFO("touchDOWNLOAD PhaseTurn0: ring1:%d,%d,%d. ring2:%d,%d,%d\n", gSignalControlpara->stPhaseTurn[0][0].nTurnArray[0], gSignalControlpara->stPhaseTurn[0][0].nTurnArray[1], gSignalControlpara->stPhaseTurn[0][0].nTurnArray[2],
	//		gSignalControlpara->stPhaseTurn[0][1].nTurnArray[0], gSignalControlpara->stPhaseTurn[0][1].nTurnArray[1], gSignalControlpara->stPhaseTurn[0][1].nTurnArray[2]);
	//INFO("---------------------------------");


	//计划配置
	memcpy(gSignalControlpara->stTimeInterval, msg->data + offset, NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem));
	offset += NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem);

	//日期调度配置
	memcpy(gSignalControlpara->stPlanSchedule, msg->data+offset, 10/*NUM_SCHEDULE*/*sizeof(PlanScheduleItem));
	offset += 10/*NUM_SCHEDULE*/*sizeof(PlanScheduleItem);
	//车检器配置
	memcpy(gSignalControlpara->AscVehicleDetectorTable, msg->data + offset,  48/*MAX_VEHICLEDETECTOR_COUNT*/*sizeof(struct STRU_N_VehicleDetector));
	offset += 48/*MAX_VEHICLEDETECTOR_COUNT*/*sizeof(struct STRU_N_VehicleDetector);
	//单元参数
	memcpy((char*)&gSignalControlpara->stUnitPara, msg->data+offset,  sizeof(UnitPara));

	return;
}
int TouchConfig_upload(STRU_TOUCH_BOARD_MSG *msg)
{
	int ret = 0;
	if(NULL == msg)
		return 0;

	switch(msg->type)
	{
		case TC_MSG_CONFIG_PHASE:
			break;
		case TC_MSG_CONFIG_CHANNEL:
			break;
		case TC_MSG_CONFIG_SCHEME:
			break;
		case TC_MSG_CONFIG_PLAN:
			break;
		case TC_MSG_CONFIG_DATE:
			break;
		case TC_MSG_CONFIG_CARDET:
			break;
		case TC_MSG_CONFIG_UNIT:
			break;
		case TC_MSG_CONFIG_ALL:
			ret =TouchConfig_uploadAll(msg);
			break;
		default:
			break;
	}
	return ret;
}
int TouchConfig_download(STRU_TOUCH_BOARD_MSG *msg)
{
	static UINT16 downloadFlag = 0;
	int offset = 0;
	int i = 0;
	int ret = -1;
	if(NULL == msg)
		return 0;

	switch(msg->type)
	{
		case TC_MSG_CONFIG_PHASE:
			INFO("TouchBoard: Download Phase params.");
			//相位配置
			memcpy(gSignalControlpara->stPhase, msg->data, MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem));
			INFO("==Phase:0-%d,1-%d,2-%d,3-%d\n", gSignalControlpara->stPhase[0][0].nPhaseID, gSignalControlpara->stPhase[0][1].nPhaseID, gSignalControlpara->stPhase[0][2].nPhaseID, gSignalControlpara->stPhase[0][3].nPhaseID);
			memcpy(gSignalControlpara->stOldPhase, gSignalControlpara->stPhase[0], NUM_PHASE*sizeof(PhaseItem));
			offset = MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(PhaseItem);
			//信号转换序列
			memcpy(gSignalControlpara->AscSignalTransTable, msg->data + offset,  MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
			memcpy(gSignalControlpara->OldAscSignalTransTable, gSignalControlpara->AscSignalTransTable[0], NUM_PHASE*sizeof(struct STRU_SignalTransEntry));
			offset += MAX_PHASE_TABLE_COUNT*NUM_PHASE*sizeof(struct STRU_SignalTransEntry);
			//绿信比配置
			memcpy(gSignalControlpara->stGreenSignalRation, msg->data + offset,  16/*NUM_GREEN_SIGNAL_RATION*/*NUM_PHASE*sizeof(GreenSignalRationItem));
			break;
		case TC_MSG_CONFIG_CHANNEL:
			INFO("TouchBoard: Download Channel params.");
			memcpy(gSignalControlpara->stChannel, msg->data, MAX_CHANNEL_TABLE_COUNT*NUM_CHANNEL*sizeof(ChannelItem));
			memcpy(gSignalControlpara->stOldChannel, gSignalControlpara->stChannel[0], NUM_CHANNEL*sizeof(ChannelItem));
			break;
		case TC_MSG_CONFIG_SCHEME:
			{
				INFO("TouchBoard: Download Scheme params.");
				//方案配置
				SchemeItem *item = NULL;
				memcpy(gSignalControlpara->stScheme, msg->data, NUM_SCHEME*sizeof(SchemeItem));
				item = gSignalControlpara->stScheme;
				for (i = 0; i < NUM_SCHEME; i++)
				{
					if (item[i].nSchemeID > 0 && item[i].nSchemeID <= NUM_SCHEME)
					{
						gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nSchemeID = item[i].nSchemeID;
						gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nCycleTime = item[i].nCycleTime;
						gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nOffset = item[i].nOffset;
						gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nGreenSignalRatioID = item[i].nGreenSignalRatioID;
						gSignalControlpara->stOldScheme[item[i].nSchemeID - 1].nPhaseTurnID = item[i].nPhaseTurnID;
					}
				}
				offset = NUM_SCHEME*sizeof(SchemeItem);
				//动作配置
				//memcpy(gSignalControlpara->stAction, msg.data + offset,  NUM_ACTION*sizeof(ActionItem));
				ActionItem *item1 = (ActionItem *)(msg->data + offset);
				for (i = 0; i < NUM_ACTION; i++)
				{
					if (item1[i].nActionID > 0 && item1[i].nActionID <= NUM_ACTION)
					{
						if (item1[i].nPhaseTableID == 0 || item1[i].nPhaseTableID > MAX_PHASE_TABLE_COUNT)
							item1[i].nPhaseTableID = 1;
						if (item1[i].nChannelTableID == 0 || item1[i].nChannelTableID > MAX_CHANNEL_TABLE_COUNT)
							item1[i].nChannelTableID = 1;
						gSignalControlpara->stAction[item1[i].nActionID - 1] = item1[i];
					}
				}
				offset += NUM_ACTION*sizeof(ActionItem);
				//相序配置
				memcpy(gSignalControlpara->stPhaseTurn, msg->data + offset,  NUM_PHASE_TURN*NUM_RING_COUNT*sizeof(PhaseTurnItem));
			}
			break;
		case TC_MSG_CONFIG_PLAN://时段配置
			INFO("TouchBoard: Download Interval params.");
			memcpy(gSignalControlpara->stTimeInterval, msg->data, NUM_TIME_INTERVAL*NUM_TIME_INTERVAL_ID*sizeof(TimeIntervalItem));
			break;
		case TC_MSG_CONFIG_DATE://日期调度配置
			INFO("TouchBoard: Download Date params.");
			memcpy(gSignalControlpara->stPlanSchedule, msg->data, 10/*NUM_SCHEDULE*/*sizeof(PlanScheduleItem));
			break;
		case TC_MSG_CONFIG_CARDET:
			INFO("TouchBoard: Download CarDet params.");
			memcpy(gSignalControlpara->AscVehicleDetectorTable, msg->data,  48*sizeof(struct STRU_N_VehicleDetector));
			break;
		case TC_MSG_CONFIG_UNIT:
			INFO("TouchBoard: Download Unit params.recv_len:%d",msg->len);
			memcpy((char*)&gSignalControlpara->stUnitPara, msg->data, sizeof(UnitPara));
			break;
		case TC_MSG_CONFIG_ALL:
			INFO("TouchBoard: Download All params.");
			TouchConfig_downloadAll(msg);
			iIsSaveHikconfig = 0x1fff; //E_TABLE_NAME_UNIT(0) ~ E_TABLE_NAME_SIGNAL_TRANS(12)
			break;
		default:
			break;
	}
	INFO("TouchBoard: Download params done.");
	ret = IsSignalControlparaLegal(gSignalControlpara);
	log_debug("download touchconfig over, check result %d", ret);
	if (ret == 0)
	{	//检查配置是否合理
		ItsSetConfig(gSignalControlpara, sizeof(SignalControllerPara));
		
		WriteLoaclConfigFile(gSignalControlpara);	//写入本地配置文件里
		log_debug("Tsc config information update!");
		ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
		msg->data[0] = TOUCH_CONFIG_OP_SUCC;
	}
	else
	{
		ItsGetConfig(gSignalControlpara, sizeof(SignalControllerPara));
		msg->data[0] = TOUCH_CONFIG_OP_FAIL;
	}
	msg->len = 1;
	INFO("TouchBoard: Check params %s!", ret == 0 ? "successful":"failed");
	
	return 8+msg->len;
}
int TouchConfig_msgHandle(STRU_TOUCH_BOARD_MSG *msg)
{
	int ret = 0;
	if(NULL == msg)
		return 0;
	switch(msg->flag)
	{
		case TOUCH_CONFIG_UPLOAD:
			ret = TouchConfig_upload(msg);
			break;
		case TOUCH_CONFIG_DOWNLOAD:
			ret = TouchConfig_download(msg);
			break;
		default:
			break;
	}
	return ret;
}

