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
#include <unistd.h>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE	//对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <time.h>
#include "common.h"
#include "HikConfig.h"
#include "platform.h"
#include "its.h"
#include "configureManagement.h"
#include "binfile.h"
#include "gb.h"

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


extern int CreateCommunicationSocket(char *interface, UInt16 port);
extern void GPS_led_off();
extern void StoreBegin(void *arg);
extern void DownloadConfig(int type, void *arg);
extern int DownloadExtendConfig(struct STRU_Extra_Param_Response *response);
extern int UploadConfig(struct STRU_Extra_Param_Response *response);
extern void WriteLoaclConfigFile(ProtocolType type, void *config);
#ifdef USE_GB_PROTOCOL
extern void NtcipConvertToGb();
#endif


extern int gComPort;//各个信号机的通讯端口号，消息队列ID与此值相同。


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

	if((gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 0) && (p_SpecialParams->iGpsSwitch == 1))
	{
		//GPS从关闭到打开
		system("killall -9 GPS &");
		system("/root/GPS &");
	}
	else if((gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 1) && (p_SpecialParams->iGpsSwitch == 0))
	{
		//GPS从打开到关闭
		system("killall -9 GPS &");
		GPS_led_off();
	}

    memcpy(&gStructBinfileConfigPara.sSpecialParams,p_SpecialParams,sizeof(gStructBinfileConfigPara.sSpecialParams));
    SendSuccessMsg(socketFd,fromAddr);
}

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
static void DownloadIpAddress(int socketFd,struct sockaddr_in fromAddr)
{
	struct STRU_N_IP_ADDRESS ip_info;
    struct in_addr inaddr;
	char *name = NULL;
    char cmd[256] = {0};
	FILE *fp = NULL;
	int line = 0;
	
    if(udp_info.iType == 0x15d)
		name = "eth1";
    else if(udp_info.iType == 0x15f)
		name = "eth0";
    else if(udp_info.iType == 0x161)
		name = "wlan0";
	memset(&ip_info,0,sizeof(ip_info));
	inaddr.s_addr = (unsigned long)udp_info.iValue[0];
	strcpy(ip_info.address, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)udp_info.iValue[1];
	strcpy(ip_info.subnetMask, inet_ntoa(inaddr));
	inaddr.s_addr = (unsigned long)udp_info.iValue[2];
	strcpy(ip_info.gateway, inet_ntoa(inaddr));
	SetNetwork(name, udp_info.iValue[0], udp_info.iValue[1], udp_info.iValue[2]);
    SendSuccessMsg(socketFd,fromAddr);    

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
					line, line + 2, ip_info.address, ip_info.subnetMask, ip_info.gateway);
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

#define UP_DOWN_LOAD_TIMEOUT	60		//上下载超时时间60s
static void UpAndDownLoadDeal(int socketFd, struct sockaddr_in fromAddr)	//上下载处理
{
	static UInt8 uploadNum = 0;
	static UInt32 downloadFlag = 0;//存放的是开始下载时SDK传送来的具体哪些参数需要保存的flag
	static struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0};
	struct timespec currentTime;
	struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)&udp_info;
	int ret = 12;

	if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//下载参数开始
	{
		log_debug("download config begin");
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
		log_debug("download config over");
		if (downloadFlag > 0 && IsSignalControlparaLegal(gSignalControlpara) == 0)
		{	//检查配置是否合理
			ItsSetConfig(NTCIP, gSignalControlpara);
			if (BIT(downloadFlag, 16)) //表示是否写flash
				WriteLoaclConfigFile(NTCIP, gSignalControlpara);	//写入本地配置文件里
			log_debug("config information update!");
			ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
		}
		else
			ItsGetConfig(NTCIP, gSignalControlpara);
		downloadFlag = 0;
		downloadStartTime.tv_sec = 0;
	}
	else if(udp_info.iHead == 0x6e6e && ((udp_info.iType >= 0xaa && udp_info.iType <= 0xb6) || udp_info.iType == 0xeeeeeeee))//下载配置信息		0xeeeeeeee表示web下载
	{
		//log_debug("download config, type = %#x", udp_info.iType);
		DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 只是为了ctrl+f搜索方便!
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xce)	//上载开始
	{
		log_debug("upload config begin");
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
		log_debug("upload config over");
		if (uploadNum > 1)
			uploadNum--;
		else if (uploadNum == 1)
		{
			uploadNum = 0;
			uploadStartTime.tv_sec = 0;
		}
	}
	else if (udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)	//上载配置
	{
		ret = sizeof(struct STRU_Extra_Param_Response);
		if (response->unExtraParamValue == 0x15b)	//上载故障信息
		{
			log_debug("upload fault log, startline = %u, linenum = %u", response->unExtraParamFirst, response->unExtraParamTotal);
			ItsReadFaultLog(response->unExtraParamFirst, response->unExtraParamTotal, socketFd, (struct sockaddr *)&fromAddr);
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
	ItsCountDownGet(gp);
	gp->unExtraParamHead = 0x6e6e;
	gp->unExtraParamID = 0xeeeeeeec;
	sendto(socketFd, &udp_info,sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}


static inline void UploadChannelStatus(int socketFd, struct sockaddr_in fromAddr)
{
	struct STRU_N_ChannelStatusGroup *gp = (struct STRU_N_ChannelStatusGroup *)udp_info.iValue;
	int i, j, datasize = sizeof(*gp) * 4;
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;

	memset(gp, 0, datasize);
	ItsCountDownGet(&countDownParams);
	for (i = 0; i < 4; i++)	//总共4组状态
	{
		gp->byChannelStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//每组包含8个通道的状态，1bit代表一个通道
		{
			switch (countDownParams.ucChannelRealStatus[i * 8 + j])
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
	ItsCountDownGet(&countDownParams);
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
	ItsCountDownGet(&countDownParams);
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
	ItsCountDownGet(&countDownParams);
	gp->byCoordPatternStatus = countDownParams.ucPlanNo;
	gp->wCoordCycleStatus = countDownParams.ucCurCycleTime - countDownParams.ucCurRunningTime;
	gp->wCoordSyncStatus = countDownParams.ucCurRunningTime;
	gp->byUnitControlStatus = ItsControlStatusGet();
	sendto(socketFd, &udp_info, 8 + datasize, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

//根据type更新本地文件,目前仅仅做核心配置文件的实时更新，其他文件暂时不做处理,TODO
static inline void UpdateLocalCfg(int socketFd, struct sockaddr_in fromAddr,int type)
{
    sleep(1);//必须等待一段时间再进行更新，否则可能文件没有准备好
    switch(type)
    {
        case 1://更新hikconfig.dat
        {
            SignalControllerPara para;
            memset(&para,0,sizeof(para));

            READ_BIN_CFG_PARAMS(FILE_TSC_CFG_DAT, &para, sizeof(para));
            if(IsSignalControlparaLegal(&para) == 0)
            {
                ItsSetConfig(NTCIP, &para);
                INFO("UpdateLocalCfg hikconfig \n");
            }
            break ;
        }
        case 9://config.dat
        {
            READ_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));

            INFO("UpdateLocalCfg config \n");
            break;
        }
        case 3://desc.dat
        {
            READ_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC));

            INFO("UpdateLocalCfg desc \n");
            break;
        }
        case 4://custom.dat
        {
            READ_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
            INFO("UpdateLocalCfg custom \n");
            break;
        }
        case 5://countdown.dat
        {
            READ_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,&g_CountDownCfg,sizeof(CountDownCfg));
            INFO("UpdateLocalCfg countdown \n");
            break;
        }
        case 6://misc.dat
        {
            READ_BIN_CFG_PARAMS(FILE_MISC_DAT,&gStructBinfileMisc,sizeof(STRUCT_BINFILE_MISC));
            INFO("UpdateLocalCfg misc \n");
            break;
        }
        case 2://gbconfig.dat
        {
            GbConfig gbCfg;
            memset(&gbCfg,0,sizeof(gbCfg));
            READ_BIN_CFG_PARAMS("/home/gbconfig.dat",&gbCfg,sizeof(gbCfg));
            ItsSetConfig(GB2007, &gbCfg);

            INFO("UpdateLocalCfg gbconfig \n");
            break;
        }
        default:
        {
            break;
        }
    }
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

    STRU_SPECIAL_PARAMS s_SpecialParams;
    unsigned char iIsSaveSpecialParams = 0;
    unsigned char iIsSaveCustomParams = 0;
    unsigned char iIsSaveDescParams = 0;

	struct UDP_CUR_VALUE_INFO *udp_cur_value_info = (struct UDP_CUR_VALUE_INFO *)&udp_info;
	struct CURRENT_PARAMS_UDP *current_params_udp_info = (struct CURRENT_PARAMS_UDP *)&udp_info;
	STEP_CTRL_PARAMS *stepCtrlParams = (STEP_CTRL_PARAMS *)&udp_info;
	CANCEL_STEP_FEEDBACK_PARAMS *cancelStepFeedback = (CANCEL_STEP_FEEDBACK_PARAMS *)&udp_info;
	
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS countDownParams;
	int specialControlSchemeId = 0;       //特殊控制方案号,用来上载控制方案号时使用
	struct SAdjustTime *timep = (struct SAdjustTime *)udp_info.iValue;
	
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);
	
	memset(&udp_info,0,sizeof(struct UDP_INFO));

    socketFd = CreateCommunicationSocket(NULL, gComPort);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	pthread_exit(NULL);
    }
	
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//下载特殊检测参数
			{
				log_debug("download special check parameters, value = %#x", udp_info.iValue[0]);
                DownloadSpecialParams(socketFd,fromAddr,&s_SpecialParams);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//上载特殊检测参数
			{
				log_debug("upload special check parameters");
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
				log_debug("clear fault log information");
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
				log_debug("upload red light current parameters");
				memcpy(current_params_udp_info->struRecCurrent,gStructBinfileConfigPara.sCurrentParams,sizeof(gStructBinfileConfigPara.sCurrentParams));
				result = sendto(socketFd, &udp_info, sizeof(struct CURRENT_PARAMS_UDP), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//下载电流参数
			{
				log_debug("download red light current parameters");
				memcpy(gStructBinfileConfigPara.sCurrentParams, udp_info.iValue, sizeof(gStructBinfileConfigPara.sCurrentParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//恢复默认参数
			{
				log_debug("recover default parameters");
				system("rm -rf /home/*");
				SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
			{		
				log_debug("set ip address");
                DownloadIpAddress(socketFd,fromAddr);//下载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{
				log_debug("get ip address");
                UploadIpAddress(socketFd,fromAddr);//上载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//下载相位描述
			{
				log_debug("download phase describe");
				memcpy(&gStructBinfileDesc.sPhaseDescParams,&udp_info,sizeof(gStructBinfileDesc.sPhaseDescParams));
                SendSuccessMsg(socketFd,fromAddr);
				memcpy(gStructBinfileDesc.phaseDescText[0], &gStructBinfileDesc.sPhaseDescParams.stPhaseDesc, sizeof(gStructBinfileDesc.sPhaseDescParams.stPhaseDesc));
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//上载相位描述
			{
				log_debug("upload phase describe");
				gStructBinfileDesc.sPhaseDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPhaseDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPhaseDescParams, sizeof(gStructBinfileDesc.sPhaseDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//下载通道描述
			{
				log_debug("download channel describe");
				memcpy(&gStructBinfileDesc.sChannelDescParams,&udp_info,sizeof(gStructBinfileDesc.sChannelDescParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//上载通道描述
			{
				log_debug("upload channel describe");
				gStructBinfileDesc.sChannelDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sChannelDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sChannelDescParams, sizeof(gStructBinfileDesc.sChannelDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//下载方案描述
			{
				log_debug("download scheme describe");
				memcpy(&gStructBinfileDesc.sPatternNameParams,&udp_info,sizeof(gStructBinfileDesc.sPatternNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//上载方案描述
			{
				log_debug("upload scheme describe");
				gStructBinfileDesc.sPatternNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPatternNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPatternNameParams, sizeof(gStructBinfileDesc.sPatternNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//下载计划描述
			{
				log_debug("download plan describe");
				memcpy(&gStructBinfileDesc.sPlanNameParams,&udp_info,sizeof(gStructBinfileDesc.sPlanNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//上载计划描述
			{
				log_debug("upload plan describe");
				gStructBinfileDesc.sPlanNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPlanNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPlanNameParams, sizeof(gStructBinfileDesc.sPlanNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//下载日期描述
			{
				log_debug("download date describe");
				memcpy(&gStructBinfileDesc.sDateNameParams,&udp_info,sizeof(gStructBinfileDesc.sDateNameParams));
				SendSuccessMsg(socketFd,fromAddr);
				iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//上载日期描述
			{
				log_debug("upload date describe");
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
				log_debug("download countdown set information");
				memcpy(&gStructBinfileCustom.sCountdownParams,udp_info.iValue,sizeof(gStructBinfileCustom.sCountdownParams));
				SendSuccessMsg(socketFd,fromAddr);
				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				ItsYellowLampFlash(GET_BIT(gStructBinfileCustom.sCountdownParams.option, 1) ? TRUE : FALSE);//设置黄灯时闪烁
				ItsSetRedFlashSec((UInt8)gStructBinfileCustom.sCountdownParams.redFlashSec);//设置红灯倒计时闪烁的秒数
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//上载倒计时牌设置
			{
				log_debug("upload countdown set information");
				memcpy(udp_info.iValue,&gStructBinfileCustom.sCountdownParams,sizeof(gStructBinfileCustom.sCountdownParams));
				result = sendto(socketFd, &udp_info, sizeof(gStructBinfileCustom.sCountdownParams) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//下载串口配置
			{
				log_debug("download serial config");
                if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
			    {
    				memcpy(&gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1],&udp_info,sizeof(COM_PARAMS));
    				SendSuccessMsg(socketFd,fromAddr);
    				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    }
			    else
			    {
                    result = -1;
			    }
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//上载串口配置
			{
				log_debug("upload serial config");
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
				ItsCountDownGet(&countDownParams);
				if(gStructBinfileCustom.cChannelLockFlag != 1 || gStructBinfileCustom.sChannelLockedParams.ucReserved ==1)//reserved==1 表示可变车道牌控制
					countDownParams.ucChannelLockStatus = 0;
				if (countDownParams.ucPlanNo > 0 && countDownParams.ucPlanNo <= 16)
					memcpy(countDownParams.ucCurPlanDsc, gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo - 1], sizeof(countDownParams.ucCurPlanDsc));	//添加方案描述
                else if(countDownParams.ucPlanNo > 16 && countDownParams.ucPlanNo < 249)
                    snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"方案 %d",countDownParams.ucPlanNo);

				result = sendto(socketFd, &countDownParams, 
				/*由于私自在倒计时结构体后面增加了一些内容，但平台那边不需要，因此需要减掉这些新增内容长度*/
						 offsetof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS, ucChannelRealStatus),
						 MSG_DONTWAIT, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//信号机重启
			{
				log_debug("System will restart");
				ItsWriteFaultLog(UNNORMAL_OR_SOFTWARE_REBOOT, 0);
				SendSuccessMsg(socketFd,fromAddr);
				sync();
				sleep(1);
				system("reboot");
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK)//开启通道锁定
			{
				log_debug("enable channel lock");
				memcpy(&gStructBinfileCustom.sChannelLockedParams,&udp_info,sizeof(gStructBinfileCustom.sChannelLockedParams));
				//ItsChannelLock(&gStructBinfileCustom.sChannelLockedParams);
			    gStructBinfileCustom.cChannelLockFlag = 1;
			    WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_UNLOCK)//关闭通道锁定
			{
				switch(udp_info.iValue[0])
				{
					case 0://原通道解锁
						log_debug("disable channel lock");
						if(gStructBinfileCustom.cChannelLockFlag == 1)
						{
							gStructBinfileCustom.cChannelLockFlag = 0;
							//ItsChannelUnlock();
						}
						break;
					case 1://多时段通道锁定解锁
						log_debug("Disable mult periods channel lock");
						gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 0;
						break;
					case 2://多时段通道锁定恢复
						log_debug("Recover mult periods channel lock");
						gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 1;
						break;
					default:
						log_debug("Unknown Msg type in ChanUnlock!");
						break;
				}
			    SendSuccessMsg(socketFd,fromAddr);
			    WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd7)//设置降级参数
			{
				log_debug("set demotion parameters");
				memcpy(&gStructBinfileCustom.demotionParams,&udp_info,sizeof(gStructBinfileCustom.demotionParams));
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd8)//获取降级参数
			{
				log_debug("get demotion parameters");
				gStructBinfileCustom.demotionParams.unExtraParamID = 0xd8;
				result = sendto(socketFd, &gStructBinfileCustom.demotionParams, sizeof(gStructBinfileCustom.demotionParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//手动特殊控制命令
			{
				log_debug("manual special control command, schemeid = %d", udp_info.iValue[0]);
				ItsCtl(TOOL_CONTROL, (UInt8)udp_info.iValue[0], 0);
				specialControlSchemeId = udp_info.iValue[0];	//存下方案号用来上载时使用
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6d)//获取经济型信号机型号
			{
			    udp_info.iValue[0] = specialControlSchemeId;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//信号机步进控制
			{
				log_debug("excute step control, stageNO = %d", stepCtrlParams->unStepNo);
				ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, stepCtrlParams->unStepNo);
				stepCtrlParams->unStepNo = 1;	//表示步进成功
				result = sendto(socketFd, stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); 
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//信号机取消步进
			{
				log_debug("cancel step control");
				ItsCtl(TOOL_CONTROL, 0, 0); //发送系统控制消息
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//设置经济型信号机型号
			{
			    gStructBinfileConfigPara.sSpecialParams.iSignalMachineType = udp_info.iValue[0];
			    iIsSaveSpecialParams = 1;
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//获取经济型信号机型号
			{
			    udp_info.iValue[0] = gStructBinfileConfigPara.sSpecialParams.iSignalMachineType;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x71)//下载对时
			{
				log_debug("set system time, ulGlobalTime = %lu, unTimeZone = %d", timep->ulGlobalTime, timep->unTimeZone);
				timep->ulGlobalTime += timep->unTimeZone;
				stime((time_t *)&timep->ulGlobalTime);
				system("hwclock -w");
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x72)//上载对时
			{
				log_debug("get system time");
				timep->ulGlobalTime = time(NULL) - 8 * 3600;
				timep->unTimeZone = 8 * 3600;
				result = sendto(socketFd, &udp_info, 8 + sizeof(*timep), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6c)//通道检测
			{
				log_debug("excute channel check, terminal = %d", udp_info.iValue[0]);
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
				log_debug("get real time running scheme information");
				MsgRealtimePattern *p = (MsgRealtimePattern *)&udp_info;
				ItsGetRealtimePattern(p);
                result = sendto(socketFd, p, sizeof(MsgRealtimePattern), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd1)//实时流量统计获取接口
			{
                pthread_rwlock_rdlock(&gLockRealtimeVol);
                gStructMsgRealtimeVolume.unExtraParamHead = 0x6e6e;
                gStructMsgRealtimeVolume.unExtraParamID = 0xd1;

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
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeed)//更新指定的配置文件
			{
                UpdateLocalCfg(socketFd,fromAddr,udp_info.iValue[0]);
            }
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_SET)//多时段通道锁定(解锁和原通道锁定共用一个消息)
			{
				if(gStructBinfileCustom.cChannelLockFlag == 1)//原通道已处于锁定状态不生效
				{
					log_debug("Can't override realtime channel lock");
					SendFailureMsg(socketFd,fromAddr);
				}
				else
				{
					log_debug("Enable mult periods channel lock");
					memcpy((char*)gStructBinfileCustom.sMulPeriodsChanLockParams.chans,(char*)udp_info.iValue,sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans));
					//ItsChannelLock(&gStructBinfileCustom.sChannelLockedParams);
				    gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 1;
				    WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				    SendSuccessMsg(socketFd,fromAddr);
					//log_debug("Mult periods channel lock");
				}
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_GET)//多时段通道锁定查询
			{
				log_debug("Get mult periods channel lock info");
				memcpy(&udp_info.iValue[0], (char*)gStructBinfileCustom.sMulPeriodsChanLockParams.chans, sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans));
				*((char *)udp_info.iValue + sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans)) = gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag; 	
				result = sendto(socketFd, &udp_info, 12+sizeof(gStructBinfileCustom.sMulPeriodsChanLockParams.chans), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0xffeeffee)//exit
			{
                exit(0);
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
					if(iIsSaveCustomParams == 1)
					{
						iIsSaveCustomParams = 0;
						WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
					}
					if(iIsSaveDescParams == 1 )
					{
						iIsSaveDescParams = 0;
						WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC));
					}
					if(iIsSaveSpecialParams == 1)
					{
						iIsSaveSpecialParams = 0;
						WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
					}
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

