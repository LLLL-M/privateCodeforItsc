#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <net/route.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE	//对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <time.h>
#include "its.h"
#include "ykconfig.h"

typedef struct
{
	int sockfd;
	struct sockaddr addr;
} UploadFaultLogNetArg;	//上载故障日志的网络参数

void UploadFaultLog(void *arg, void *data, int datasize)
{
	UploadFaultLogNetArg *netArg = (UploadFaultLogNetArg *)arg;
	char buf[2048] = {0};
	YK_ProtocolHead *protocol = (YK_ProtocolHead *)buf;

	protocol->iHead = 0x7e7e;
	protocol->iType = 0x7f0a;
	memcpy(protocol->iValue, data, datasize);
	sendto(netArg->sockfd, protocol, sizeof(YK_ProtocolHead) + datasize, 0, &netArg->addr, sizeof(struct sockaddr));
}

extern YK_Config *gSignalControlpara;
extern int CreateCommunicationSocket(char *interface, UInt16 port);


pthread_rwlock_t gLockRealtimeVol = PTHREAD_RWLOCK_INITIALIZER;//保护实时流量的读写锁
TimeAndHistoryVolume gRealtimeVolume;                 //实时流量，只有流量是实时的

static Boolean SetNetwork(char *name, UInt32 ip, UInt32 netmask, UInt32 gateway)
{
	struct ifreq ifr;
	struct rtentry rt;
	struct sockaddr_in gateway_addr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return FALSE;
	}
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, name);
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
	addr->sin_addr.s_addr = gateway;	//这一句只是用来打印，没什么具体用途
	memset(&rt, 0, sizeof(rt));
	memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
	rt.rt_dev = name;
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
	close(sockfd);
	return TRUE;
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
static void DownloadIpAddress(YK_NetworkParam *networkParam)
{
	char ipaddr[20] = {0}, netmaskaddr[20] = {0}, gatewayaddr[20] = {0};
    struct in_addr inaddr;
	char *name = NULL;
    char cmd[256] = {0};
	FILE *fp = NULL;
	int line = 0;

    if(networkParam->networkType == 0x15d)
		name = "eth1";
    else if(networkParam->networkType == 0x15f)
		name = "eth0";
    else if(networkParam->networkType == 0x161)
		name = "wlan0";
	else
	{
		ERR("Unknown set network interface type %#x", networkParam->networkType);
		return;
	}
	inaddr.s_addr = (unsigned long)networkParam->ip;
	strcpy(ipaddr, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)networkParam->netmask;
	strcpy(netmaskaddr, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)networkParam->gateway;
	strcpy(gatewayaddr, inet_ntoa(inaddr));
	SetNetwork(name, networkParam->ip, networkParam->netmask, networkParam->gateway);  

	sprintf(cmd, "grep 'iface %s' /etc/network/interfaces -n | cut -d ':' -f 1", name);
	sprintf(cmd, "grep 'iface %s' /etc/network/interfaces -A4 -n | grep address | cut -d '-' -f 1", name);
	fp = popen(cmd, "r");
	if (fp != NULL)
	{	//先找出要设置网卡的address配置位于/etc/network/interfaces哪一行
		fscanf(fp, "%d", &line);
		pclose(fp);
		if (line > 0)
		{	//使用sed命令进行行替换，替换掉之前的配置
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "sed -i '%d,%dc address %s\\\nnetmask %s\\\ngateway %s' /etc/network/interfaces", 
					line, line + 2, ipaddr, netmaskaddr, gatewayaddr);
			system(cmd);
		}
	}
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
static void UploadIpAddress(YK_NetworkParam *networkParam)
{
	char ipaddr[20] = {0}, netmaskaddr[20] = {0}, gatewayaddr[20] = {0};
    char *cmd = NULL;
	FILE *fp = NULL;

    if (networkParam->networkType == 0x15d)
		cmd = "grep 'iface eth1' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(networkParam->networkType == 0x15f)
		cmd = "grep 'iface eth0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
    else if(networkParam->networkType == 0x161)
		cmd = "grep 'iface wlan0' -A4 /etc/network/interfaces | sed -e '/iface/d' -e '/pre-up/d' -e '/^$/d'";
	else
	{
		ERR("Unknown get network interface type %#x", networkParam->networkType);
		networkParam->ip = 0xffffffff;
		networkParam->netmask = 0xffffffff;
		networkParam->gateway = 0xffffffff;
		return;
	}
	fp = popen(cmd, "r");
	if (fp != NULL)
	{	//从/etc/network/interfaces文件中获取IP信息
		fscanf(fp, "address %s\nnetmask %s\ngateway %s\n", ipaddr, netmaskaddr, gatewayaddr);
		pclose(fp);
		INFO("ip=%s,netmask=%s,address=%s\n", ipaddr, netmaskaddr, gatewayaddr);
		networkParam->ip = inet_addr(ipaddr);
		networkParam->netmask = inet_addr(netmaskaddr);
		networkParam->gateway = inet_addr(gatewayaddr);
	}
}

static void DownloadConfig(UInt32 type, void *data)
{
	YK_TimeIntervalItem *timeIntervalItem = (YK_TimeIntervalItem *)data;
	YK_SchemeItem *schemeItem = (YK_SchemeItem *)data;
	YK_ChannnelLockParams *channnelLockParams = (YK_ChannnelLockParams *)data;
	ChannelLockParams channelLock;
	struct SAdjustTime *pTime = (struct SAdjustTime *)data;
	YK_NetworkParam *networkParam = (YK_NetworkParam *)data;
	
	switch (type)
	{
		case 0x7f01: 
			memcpy(&gSignalControlpara->scheduleTable, data, sizeof(YK_PlanScheduleTable));
			break;
		case 0x7f02:
			if (timeIntervalItem->nTimeIntervalID > 0 && timeIntervalItem->nTimeIntervalID <= NUM_TIME_INTERVAL)
			{
				memcpy(&gSignalControlpara->timeIntervalTable[timeIntervalItem->nTimeIntervalID - 1], timeIntervalItem, sizeof(YK_TimeIntervalTable));
			}
			break;
		case 0x7f03:
			memcpy(&gSignalControlpara->channelTable, data, sizeof(YK_ChannelTable));
#if 0
			YK_ChannelItem *channelItem = (YK_ChannelItem *)data;
			int i;
			for (i = 0; i < NUM_CHANNEL; i++)
			{
				INFO("nChannelID:%d, nControllerType:%d, conflictChannel:%#x", 
					channelItem[i].nChannelID, channelItem[i].nControllerType, channelItem[i].conflictChannel);
			}
#endif
			break;
		case 0x7f04:
			if (schemeItem->nSchemeId > 0 && schemeItem->nSchemeId <= NUM_SCHEME)
			{
				memcpy(&gSignalControlpara->schemeTable[schemeItem->nSchemeId - 1], schemeItem, sizeof(YK_SchemeItem));
			}
			else if (schemeItem->nSchemeId == SINGLE_ADAPT_SCHEMEID)
			{
				ItsSetSingleAdaptScheme(schemeItem);
			}
#if 0
			INFO("nSchemeId:%d, cycleTime:%d, totalPhaseNum:%d", 
				schemeItem->nSchemeId, schemeItem->cycleTime, schemeItem->totalPhaseNum);
			int i;
			for (i = 0; i < schemeItem->totalPhaseNum; i++)
			{
				INFO("phase%d:%d,%d,%d,%d,%d, channelBits:%#x", i + 1, 	
				schemeItem->phaseInfo[i].greenTime, schemeItem->phaseInfo[i].greenBlinkTime,
				schemeItem->phaseInfo[i].yellowTime, schemeItem->phaseInfo[i].redYellowTime,
				schemeItem->phaseInfo[i].allRedTime, schemeItem->phaseInfo[i].channelBits);
			}
#endif
			break;
		case 0x7f05:
			memcpy(&gSignalControlpara->wholeConfig, data, sizeof(YK_WholeConfig));
			break;
		case 0x7f07:
			if (channnelLockParams->lockFlag == 1)
			{
				memset(&channelLock, 0, sizeof(channelLock));
				channelLock.ucChannelLockStatus = 1;
				memcpy(channelLock.ucChannelStatus, channnelLockParams->ucChannelStatus, sizeof(channnelLockParams->ucChannelStatus));
				ItsChannelLock(&channelLock);
			}
			else
				ItsChannelUnlock();
			break;
		case 0x7f09:
			ItsCtl(TOOL_CONTROL, *(YK_ManualControl *)data, 0);
			break;
		case 0x7f0a:
			ItsClearFaultLog();
			break;
		case 0x7f0b:
			INFO("download UTC time: %#lx, timezone:%d", pTime->ulGlobalTime, pTime->unTimeZone);
			pTime->ulGlobalTime += pTime->unTimeZone;
			stime((time_t *)&(pTime->ulGlobalTime));
			system("hwclock -w");
			break;
		case 0x7f0c:
			DownloadIpAddress(networkParam);
		case 0x7f0d:
			INFO("!!!!!!receive reboot command");
			system("reboot");
			break;
	}
}

static void UploadConfig(YK_ProtocolHead *protocol, int socketFd, struct sockaddr *fromAddr)
{
	YK_TimeIntervalItem *timeIntervalItem = (YK_TimeIntervalItem *)protocol->iValue;
	YK_SchemeItem *schemeItem = (YK_SchemeItem *)protocol->iValue;
	struct SAdjustTime *pTime = (struct SAdjustTime *)protocol->iValue;
	YK_FaultLog *faultLog = (YK_FaultLog *)protocol->iValue;
	//TimeAndHistoryVolume *vehFlow = (TimeAndHistoryVolume *)protocol->iValue;
	UploadFaultLogNetArg netArg;
	int datasize = 0;
	
	switch (protocol->iType)
	{
		case 0x7f01: 
			memcpy(protocol->iValue, &gSignalControlpara->scheduleTable, sizeof(YK_PlanScheduleTable));
			datasize = sizeof(YK_PlanScheduleTable);
			break;
		case 0x7f02:
			//INFO("upload timeIntervalTable %d", timeIntervalItem->nTimeIntervalID);
			if (timeIntervalItem->nTimeIntervalID > 0 && timeIntervalItem->nTimeIntervalID <= NUM_TIME_INTERVAL)
			{
				memcpy(timeIntervalItem, &gSignalControlpara->timeIntervalTable[timeIntervalItem->nTimeIntervalID - 1], sizeof(YK_TimeIntervalTable));
				datasize = sizeof(YK_TimeIntervalTable);
			}
			break;
		case 0x7f03:
			memcpy(protocol->iValue, &gSignalControlpara->channelTable, sizeof(YK_ChannelTable));
			datasize = sizeof(YK_ChannelTable);
			break;
		case 0x7f04:
			if (schemeItem->nSchemeId > 0 && schemeItem->nSchemeId <= NUM_SCHEME)
			{
				memcpy(schemeItem, &gSignalControlpara->schemeTable[schemeItem->nSchemeId - 1], sizeof(YK_SchemeItem));
				datasize = sizeof(YK_SchemeItem);
#if 0
				INFO("nSchemeId:%d, cycleTime:%d, totalPhaseNum:%d", 
					schemeItem->nSchemeId, schemeItem->cycleTime, schemeItem->totalPhaseNum);
#endif
			}
			break;
		case 0x7f05:
			memcpy(protocol->iValue, &gSignalControlpara->wholeConfig, sizeof(YK_WholeConfig));
			datasize = sizeof(YK_WholeConfig);
			break;
		case 0x7f06:
			pthread_rwlock_rdlock(&gLockRealtimeVol);
			memcpy(protocol->iValue, &gRealtimeVolume, sizeof(TimeAndHistoryVolume));
			pthread_rwlock_unlock(&gLockRealtimeVol);
			datasize = sizeof(TimeAndHistoryVolume);
			break;
		case 0x7f08:
			ItsCountDownGet(protocol->iValue, sizeof(YK_RealTimeInfo));
			datasize = sizeof(YK_RealTimeInfo);
			break;
		case 0x7f0a:
			netArg.sockfd = socketFd;
			memcpy(&netArg.addr, fromAddr, sizeof(struct sockaddr));
			INFO("startLine = %d, linenum = %d", faultLog->startLine, faultLog->linenum);
			ItsReadFaultLog(faultLog->startLine, faultLog->linenum, &netArg, sizeof(UploadFaultLogNetArg), UploadFaultLog);
			return;
		case 0x7f0b:
			pTime->ulGlobalTime = time(NULL) - 28800;
			INFO("upload UTC time:%#lx", pTime->ulGlobalTime);
			pTime->unTimeZone = 28800;
			datasize = sizeof(struct SAdjustTime);
			break;
		case 0x7f0c:
			UploadIpAddress((YK_NetworkParam *)protocol->iValue);
			datasize = sizeof(YK_NetworkParam);
			break;
		default: datasize = 0; break;
	}
	sendto(socketFd, protocol, sizeof(YK_ProtocolHead) + datasize, 0, (struct sockaddr *)fromAddr, sizeof(struct sockaddr));
}

static void WriteLoaclConfigFile(void *config, int size)
{
	int fd;
	
	if (config == NULL || size != sizeof(YK_Config))
		return;
	fd = open("/home/ykconfig.dat", O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1)
	{
		ERR("open /home/ykconfig fail");
		return;
	}
	write(fd, config, size);
	fdatasync(fd);
	close(fd);
}

#define UP_DOWN_LOAD_TIMEOUT	60		//上下载超时时间60s
void *CommunicationModule(void *arg)
{
	UInt8 uploadNum = 0, downloadFlag = 0;
	struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0};
	struct timespec currentTime;
	int socketFd = -1;
	struct sockaddr_in fromAddr;
	socklen_t fromLen = sizeof(fromAddr);
	char buf[2048] = {0};
	YK_ProtocolHead *udp_info = (YK_ProtocolHead *)buf;
	
    socketFd = CreateCommunicationSocket(NULL, 40000);
    if (-1 == socketFd)
    {
		printf("socket udp init error!!!\n");
       	pthread_exit(NULL);
    }
	
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        if (-1 == recvfrom(socketFd, buf, sizeof(buf), 0, (struct sockaddr *)&fromAddr, &fromLen))
		{
            ERR("############===>  Failed Error   %s\n",strerror(errno));
			continue;
		}
		if (udp_info->iHead == 0x8e8e && udp_info->iType == 0x7e00)//下载参数开始
		{
			log_debug("download config begin");
			clock_gettime(CLOCK_MONOTONIC, &currentTime);
			if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
			{	//说明有其他客户端正在下载
				udp_info->iValue[0] = 1;
				sendto(socketFd, udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				continue;
			}
			if (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
			{	//说明有其他客户端正在上载
				udp_info->iValue[0] = 2;
				sendto(socketFd, udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				continue;
			}
			downloadFlag = 1;
			downloadStartTime = currentTime;
			udp_info->iValue[0] = 0;
		}
		else if (udp_info->iHead == 0x8e8e && udp_info->iType == 0x7eff)//下载配置完成
		{
			log_debug("download config over");
			ItsSetConfig(gSignalControlpara, sizeof(YK_Config));
			WriteLoaclConfigFile(gSignalControlpara, sizeof(YK_Config));	//写入本地配置文件里
			INFO("config information update!");
			ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
			downloadFlag = 0;
			downloadStartTime.tv_sec = 0;
		}
		else if (udp_info->iHead == 0x8e8e && udp_info->iType >= 0x7f01 && udp_info->iType <= 0x7f0d)//下载配置信息
		{
			//log_debug("download config, type = %#x", udp_info->iType);
			DownloadConfig(udp_info->iType, (void *)udp_info->iValue);
			udp_info->iValue[0] = 0;	//默认所有下载都是成功的
		}
		else if (udp_info->iHead == 0x7e7e && udp_info->iType == 0x7e00)	//上载开始
		{
			log_debug("upload config begin");
			clock_gettime(CLOCK_MONOTONIC, &currentTime);
			if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)	//说明有其他客户端正在下载
			{
				udp_info->iValue[0] = 1;
				sendto(socketFd, udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				continue;
			}
			//INFO("DEBUG: upload begin, downloadFlag = %#x, timegap = %d", downloadFlag, currentTime.tv_sec - downloadStartTime.tv_sec);
			//有其他客户端上载已经超时的情况下uploadNum=1,其他时候都加1
			uploadNum = (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) > UP_DOWN_LOAD_TIMEOUT) ? 1 : (uploadNum + 1);
			uploadStartTime = currentTime;
			udp_info->iValue[0] = 0;
		}
		else if (udp_info->iHead == 0x7e7e && udp_info->iType == 0x7eff)	//上载结束
		{
			log_debug("upload config over");
			if (uploadNum > 1)
				uploadNum--;
			else if (uploadNum == 1)
			{
				uploadNum = 0;
				uploadStartTime.tv_sec = 0;
			}
			udp_info->iValue[0] = 0;
		}
		else if (udp_info->iHead == 0x7e7e && udp_info->iType >= 0x7f01 && udp_info->iType <= 0x7f0c)//下载配置信息
		{
			INFO("upload config, type = %#x", udp_info->iType);
			UploadConfig(udp_info, socketFd, (struct sockaddr *)&fromAddr);
			continue;
		}
		else
			continue;
		sendto(socketFd, udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
	}
}
