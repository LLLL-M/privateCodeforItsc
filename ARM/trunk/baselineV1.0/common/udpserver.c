#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <net/route.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "common.h"
#include "HikConfig.h"
#include "platform.h"
#include "specialControl.h"
#include "binfile.h"
#include "configureManagement.h"

struct UDP_INFO udp_info;       
struct STRU_N_IP_ADDRESS ip_info;
struct UDP_CUR_VALUE_INFO udp_cur_value_info;          //红灯电流值返回数据包
extern unsigned char g_RedCurrentValue[32];
struct CURRENT_PARAMS_UDP current_params_udp_info;
DEVICE_VERSION_PARAMS device_version_params;           //设备软硬件信息
extern CountDownVeh countdown_veh[16];                 //机动车相位倒计时参数
extern CountDownPed countdown_ped[16];				   //行人相位倒计时参数
extern int HardwareWatchdoghd;                      //硬件看门狗句柄
extern int is_WatchdogEnabled;						//硬件看门狗是否被使能标记
extern void HardwareWatchdogInit();
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //用来供udpserver调用的倒计时缓存
extern pthread_rwlock_t gCountDownLock;
extern CountdownConfigFlag gCountdownConfigFlag;
extern MsgPhaseSchemeId gStructMsg1049;                            //供平台1049协议使用
extern pthread_rwlock_t gLockRealtimeVol ;//保护实时流量的读写锁
extern MsgRealtimeVolume gStructMsgRealtimeVolume;                 //实时流量，只有流量是实时的
extern SignalControllerPara *gSignalControlpara;
extern unsigned char g_cCurrentActionId;//当前运行动作号,系统控制时，可能会进行感应、黄闪、全红.
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //存放在/home/config.dat配置中的所有结构体，包括特殊参数定义结构体/车检板开关/日志打印开关/错误序列号/电流参数表
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //存放在/home/desc.dat配置中的所有结构体，包括相位描述/通道描述/方案描述/计划描述/日期描述

extern UInt8 gStepFlag;
extern UInt8 outPutCount; 	//倒计时接口调用计数

extern void HardwareWatchdogInit();
void RecordNewFault(int type);
void RecordTimeintervalSpecialAction(int actionId);
static void RecordKeyLog(int result);
int KeyDefaultOperation(int key);

extern MsgRealtimePattern gStructMsgRealtimePatternInfo;           //在倒计时接口中，每个周期开始时进行更新
extern unsigned char finalChannelStatus[32];         


/*********************************************************************************
*
* 	将平台操作都存在相应日志文件中。
*
***********************************************************************************/
int WritePlatfomCtrlInfos(const char *pFile,const char *pchMsg)
{
	time_t now;  
	struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	
	if(pchMsg == NULL)
	{
		printf("平台操作记录消息无效!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("平台操作记录文件未能正确打开!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("获取平台记录文件信息失败!\n");
		return -1;
	}
	
	if(f_stat.st_size > 100*1024)
	{
		fclose(pFileDebugInfo);
		//把文件的内容清0，即重写
		pFileDebugInfo = fopen(pFile, "w+");
	}

	//获取国际标准时间
	time(&now);  
	  
	//转换为本地时间
	timenow = localtime(&now); 

	fprintf(pFileDebugInfo,"%04d.%02d.%02d-%02d:%02d:%02d %s\n",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    
    fclose(pFileDebugInfo);
    return 0;
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
static void SendSuccessMsg(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;

	udp_info.iValue[0] = 1;
	result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
	}

}

static void SendFailureMsg(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;

	udp_info.iValue[0] = 0;
	result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
	}

}

/*****************************************************************************
 函 数 名  : ClearFaultInfo
 功能描述  : 清除故障信息
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void ClearFaultInfo(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;
    FILE * pFile = NULL;
    //清除故障日志
    pFile = fopen("/home/FailureLog.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("故障记录文件未能正确打开!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    pFile = fopen("/home/FaultStatus.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("/home/FaultStatus.dat文件未能正确打开!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    pFile = fopen("/home/FaultLog.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("/home/FaultLog.dat文件未能正确打开!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    udp_info.iValue[0] = 0;
    //将故障日志序号置为0
    gStructBinfileConfigPara.cFailureNumber = 0;
    WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));			
    //故障清除后返回故障清除成功消息包给配置工具
    result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    if ( result == -1 )
    {
    	DBG("sendto udp info error!!!\n");
    }

}

/*****************************************************************************
 函 数 名  : UploadFaultInfo
 功能描述  : 上传故障信息
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
__attribute__((unused)) static void UploadFaultInfo(int socketFd,struct sockaddr_in fromAddr, int startLineNo, int lineNum)
{
    ssize_t result = 0;
    FILE *pFile = NULL;
	size_t size = 4, offset = 0;
    char errorinfo[1024*1024 + 8] = {0};

	if (startLineNo == 0 || lineNum == 0)
		return;
    pFile = fopen("/home/FailureLog.dat", "rb");
    if (pFile == NULL)
    {
    	DBG("打开FailureLog.dat失败\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		return;
    }

	memcpy(errorinfo,&(udp_info.iHead),4);
    memcpy(errorinfo+4,&(udp_info.iType),4);
	fseek(pFile, 0, SEEK_END);
	offset = sizeof(struct FAILURE_INFO) * (startLineNo - 1);
	//INFO("FailureLog.dat size is %ld", ftell(pFile));
	if (offset < ftell(pFile))
	{
		rewind(pFile);
		fseek(pFile, offset, SEEK_CUR);
		size = fread(errorinfo + 8, sizeof(struct FAILURE_INFO), lineNum, pFile);
		size *= sizeof(struct FAILURE_INFO);
		//INFO("block size = %d, total size = %d", sizeof(struct FAILURE_INFO), size);
	}
	fclose(pFile);

    result = sendto(socketFd, errorinfo, 8 + ((size < 4) ? 4 : size), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型			
    if(result == -1)
    {
    	DBG("sendto udp info error!!!\n");
    }
}

//使用新协议上载故障信息
static int UploadFaultInfoNew(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;
    FILE *pFile = NULL;
	size_t size = 4, offset = 0;
    char buf[1024*1024 + 24] = {0};//故障文件大小是1024*1024，加上前面8个INT组合成包发送给上层应用。
    int startLineNo = 0;
    int lineNum = 0;
    
    MsgFaultInfo *info = (MsgFaultInfo *)&udp_info;

    startLineNo = info->unExtraParamFirst;
    lineNum = info->unExtraParamTotal;
    
	if (startLineNo == 0 || lineNum == 0)
    {
        sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
        return -1;
    }		

    info = (MsgFaultInfo *)buf;

    //填充头
    info->unExtraParamID = 0xc0;
    info->unExtraParamHead = 0x6e6e;
    info->unExtraParamFirst = startLineNo;
    info->unExtraParamValue = 0x15b;
		
    pFile = fopen("/home/FailureLog.dat", "rb");
    if (pFile == NULL)
    {
    	DBG("打开FailureLog.dat失败\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		return -1;
    }

	fseek(pFile, 0, SEEK_END);
	offset = sizeof(struct FAILURE_INFO) * (startLineNo - 1);
	if (offset < ftell(pFile))//如果上层应用需要获取的ID超出了日志文件的最大值，则认为无效
	{
		rewind(pFile);
		fseek(pFile, offset, SEEK_CUR);
		size = fread(info->data, sizeof(struct FAILURE_INFO), lineNum, pFile);
		info->unExtraParamTotal = size;
		size *= sizeof(struct FAILURE_INFO);
		info->unExtraDataLen = size;
	}
	else
	{
	    fclose(pFile);
	    info->unExtraParamTotal = 0;
	    info->unExtraDataLen = 0;
    	//sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
		//return -1;
	}

    result = sendto(socketFd, info,info->unExtraDataLen+24, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型			

    return result;
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
    {
        return;
    }
    
    p_SpecialParams->iErrorDetectSwitch = udp_info.iValue[0]& 0x1;
    p_SpecialParams->iCurrentAlarmSwitch = (udp_info.iValue[0] & 0x2) >> 1;
    p_SpecialParams->iVoltageAlarmSwitch = (udp_info.iValue[0] & 0x4) >> 2;
    p_SpecialParams->iCurrentAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x8) >> 3;
    p_SpecialParams->iVoltageAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x10) >> 4;
    p_SpecialParams->iWatchdogSwitch = ((udp_info.iValue[0] & 0x20) >> 5);
    p_SpecialParams->iGpsSwitch = ((udp_info.iValue[0] & 0x40) >> 6);	

    //更新看门狗和GPS进程
    if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 0) && (p_SpecialParams->iWatchdogSwitch == 1))
    {
		//硬件看门狗从关闭到打开
		//system("killall -9 watchdog &");
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 1;
		HardwareWatchdogInit();	
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 0;                
    }
    else if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 1) && (p_SpecialParams->iWatchdogSwitch == 0))
    {
		//硬件看门狗从打开到关闭
		is_WatchdogEnabled = 0;
		close(HardwareWatchdoghd);
		system("watchdog -t 1 -T 3 /dev/watchdog &");
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
		Set_LED2_OFF();
	}

    memcpy(&gStructBinfileConfigPara.sSpecialParams,p_SpecialParams,sizeof(gStructBinfileConfigPara.sSpecialParams));
    
    SendSuccessMsg(socketFd,fromAddr);
}

/*****************************************************************************
 函 数 名  : ResetAllParams
 功能描述  : 恢复默认参数
 输入参数  : int socketFd                 
             struct sockaddr_in fromAddr  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月11日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void ResetAllParams(int socketFd,struct sockaddr_in fromAddr)
{
    //将配置文件保存目录删除
    system("rm -f /home/*");
    system("rm -f /home/data/*");
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
    ssize_t result = 0;

	memset(&device_version_params,0,sizeof(device_version_params));
	device_version_params.unExtraParamHead = udp_info.iHead;
	device_version_params.unExtraParamID = udp_info.iType;
	if(gStructBinfileConfigPara.sSpecialParams.iSignalMachineType == 2)
	{
		memcpy(device_version_params.hardVersionInfo, "DS-TSC300-22", sizeof("DS-TSC300-22"));        //低32位放置硬件信息
	}
	else
	{
		memcpy(device_version_params.hardVersionInfo, HARDWARE_VERSION_INFO, sizeof(HARDWARE_VERSION_INFO));        //低32位放置硬件信息
	}
	memcpy(device_version_params.softVersionInfo, SOFTWARE_VERSION_INFO, sizeof(SOFTWARE_VERSION_INFO));       //高32位放置软件信息
	result = sendto(socketFd, &device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
		
	}
}


#define UP_DOWN_LOAD_TIMEOUT	120		//上下载超时时间120s
static void UpAndDownLoadDeal(int socketFd, struct sockaddr_in fromAddr)	//上下载处理
{
	static UInt8 uploadNum = 0,lastUploadNum = 0, firstUploadFlagCheck = 1;
	static UInt32 downloadFlag = 0;//存放的是开始下载时SDK传送来的具体哪些参数需要保存的flag
	static struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0},downloadFinishTime = {0, 0};
	struct timespec currentTime;
	struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)&udp_info;
	int ret = 12;

	if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//下载参数开始
	{
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
		downloadFlag = 0;
		downloadStartTime.tv_sec = 0;
	}
	else if(udp_info.iHead == 0x6e6e && ((udp_info.iType >= 0xaa && udp_info.iType <= 0xb6) || udp_info.iType == 0xeeeeeeee))//下载配置信息		0xeeeeeeee表示web下载
	{
		DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 只是为了ctrl+f搜索方便!
        //SendSuccessMsg(socketFd,fromAddr);//不再反馈。
		return;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xce)	//上载开始
	{
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
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (uploadNum > 1)
			uploadNum--;
		else if (uploadNum == 1)
		{
			uploadNum = 0;
			uploadStartTime.tv_sec = 0;
		}
		//INFO("Upload finished,uploadNum=%d",uploadNum);
		//added by liujieyf for  上载结束标志未收到时异常处理
		if( uploadNum > 0 && lastUploadNum > 0 )
		{		
			if(firstUploadFlagCheck == 1)
			{
				clock_gettime(CLOCK_MONOTONIC, &downloadFinishTime);
				firstUploadFlagCheck = 0;
			}
			if((currentTime.tv_sec - downloadFinishTime.tv_sec > UP_DOWN_LOAD_TIMEOUT)
				&&(firstUploadFlagCheck == 0))
			{
				INFO("Upload overtime :%ld,set uploadNum to 0",
					currentTime.tv_sec - downloadFinishTime.tv_sec);
				uploadNum = 0;
			}
		}
		else if((uploadNum && lastUploadNum) == 0 && firstUploadFlagCheck == 0)
		{
			//如果开始和结束标志数量一致，则从新计时
			firstUploadFlagCheck = 1;
		}
		lastUploadNum = uploadNum;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)	//上载配置
	{
		ret = sizeof(struct STRU_Extra_Param_Response);
		if (response->unExtraParamValue == 0x15b)	//上载故障信息
		{
			//uploadFaultInfo(socketFd, fromAddr, response->unExtraParamFirst, response->unExtraParamTotal);
			UploadFaultInfoNew(socketFd, fromAddr);
			return;
		}
#if 0	//此处为上载配置使用，然老库不从这个端口进行配置的上载，只会收到上载开始和结束，所以用不上注释掉
		else
			ret = UploadConfig(response);
#endif
	}
	sendto(socketFd, &udp_info, ret, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
}

//获取上一次的流量统计信息
static int GetVehicleFlowData(TimeAndHistoryVolume *timeAndHistoryVolume)
{
	FILE *fp = NULL;
	UINT64 offset = 0;//offset记录了，即将插入的最新的一条记录距文件头的TimeAndHistoryVolume个数,该值存储在文件开头的4字节区域内
	
	fp = fopen(FILE_VEHICLE_DAT,"r");//
	if(NULL == fp)//可能是文件不存在，就新建之
	{
        return 0;
	}

    if(fread(&offset,sizeof(UINT64),1,fp) < 1)//先获取偏移量
    {
        fclose(fp);
        return 0;
    }
    
    fseek(fp,sizeof(UINT64)+(offset-1)*sizeof(TimeAndHistoryVolume),SEEK_SET);//在正确的位置写入
	if(fread(timeAndHistoryVolume,sizeof(TimeAndHistoryVolume),1,fp) < 1)
	{
		INFO("read %s failed : %s .\n",FILE_VEHICLE_DAT,strerror(errno));
		fclose(fp);
		return 0;
	}
	timeAndHistoryVolume->dwTime += gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);
	fclose(fp);
	return 1; 
}


/*********************************************************************************
*
* 	udp接收线程处理函数入口
*
***********************************************************************************/

void udp_server_process()
{
	printf("udp server process started!\n");
	//创建UDP服务器
	int socketFd = -1;
	int opt = 1; 
    struct sockaddr_in localAddr;
	struct sockaddr_in fromAddr;
	ssize_t result = 0;

    STRU_SPECIAL_PARAMS s_SpecialParams;
    unsigned char iIsSaveSpecialParams = 0;
    unsigned char iIsSaveCustomParams = 0;
    unsigned char iIsSaveDescParams = 0;

	char msg[256] = "";

    int nIsVerifyOK = 0;//下载配置参数是否校验成功，0表示成功，1表示失败。

 	UInt8 stepFlag = 0;
	STEP_CTRL_PARAMS *stepCtrlParams = (STEP_CTRL_PARAMS *)&udp_info;
	CANCEL_STEP_FEEDBACK_PARAMS *cancelStepFeedback = (CANCEL_STEP_FEEDBACK_PARAMS *)&udp_info;
	
	memset((char *)&localAddr, 0, (int)sizeof(localAddr));
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);
	
	memset(&udp_info,0,sizeof(struct UDP_INFO));

    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	return;
    }
	
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
	//使用20000端口
    localAddr.sin_port = htons (20000);

	//设置端口复用 
 	setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));  

    int bindResult = bind(socketFd, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if ( -1 == bindResult )
    {
		printf("bind 20000 port error!!!\n");
        close(socketFd);
        return;
    }

    while(1)
    {
        // 接收数据  
        //fprintf(stderr,"udp_server_process   recving ...\n");
        memset(&udp_info,0,sizeof(udp_info));
        result = recvfrom(socketFd, &udp_info, sizeof(struct UDP_INFO), 0, (struct sockaddr *)&fromAddr, &fromLen);
		//fprintf(stderr,"recv size = %d  type=0x%x  head 0x%x  data[0] 0x%x \n", result,udp_info.iType,udp_info.iHead,udp_info.iValue[0]);
		if(-1 == result)
		{
            DBG("############===>  Failed Error   %s\n",strerror(errno));
		}
        else
        {
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//下载特殊检测参数
			{
                DownloadSpecialParams(socketFd,fromAddr,&s_SpecialParams);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//上载特殊检测参数
			{
				udp_info.iValue[0] = gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch
					| (gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch << 1)
					| (gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch << 2)
					| (gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch << 3)
					| (gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch << 4)
					| (gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch << 5)
					| (gStructBinfileConfigPara.sSpecialParams.iGpsSwitch << 6);
				
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//故障清除
			{
                ClearFaultInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)//上载红灯电流
			{
				udp_cur_value_info.iHead = udp_info.iHead;
				udp_cur_value_info.iType = udp_info.iType;
				memcpy(udp_cur_value_info.redCurrentValue, g_RedCurrentValue, 32);
				result = sendto(socketFd, &udp_cur_value_info, sizeof(udp_cur_value_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)//上载电流参数
			{
				current_params_udp_info.iHead = udp_info.iHead;
				current_params_udp_info.iType = udp_info.iType;
				memcpy(current_params_udp_info.struRecCurrent,gStructBinfileConfigPara.sCurrentParams,sizeof(struct CURRENT_PARAMS)*32);
				result = sendto(socketFd, &current_params_udp_info, sizeof(current_params_udp_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//下载电流参数
			{
				memcpy(gStructBinfileConfigPara.sCurrentParams, udp_info.iValue, 32*8);
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//恢复默认参数
			{
			    ResetAllParams(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
			{		
                DownloadIpAddress(socketFd,fromAddr);//下载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{			
                UploadIpAddress(socketFd,fromAddr);//上载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//下载相位描述
			{
				memcpy(&gStructBinfileDesc.sPhaseDescParams,&udp_info,sizeof(gStructBinfileDesc.sPhaseDescParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//上载相位描述
			{
				gStructBinfileDesc.sPhaseDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPhaseDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPhaseDescParams, sizeof(gStructBinfileDesc.sPhaseDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//下载通道描述
			{
				memcpy(&gStructBinfileDesc.sChannelDescParams,&udp_info,sizeof(gStructBinfileDesc.sChannelDescParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//上载通道描述
			{
				gStructBinfileDesc.sChannelDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sChannelDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sChannelDescParams, sizeof(gStructBinfileDesc.sChannelDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//下载方案描述
			{
				memcpy(&gStructBinfileDesc.sPatternNameParams,&udp_info,sizeof(gStructBinfileDesc.sPatternNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//上载方案描述
			{
				gStructBinfileDesc.sPatternNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPatternNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPatternNameParams, sizeof(gStructBinfileDesc.sPatternNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//下载计划描述
			{
				memcpy(&gStructBinfileDesc.sPlanNameParams,&udp_info,sizeof(gStructBinfileDesc.sPlanNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//上载计划描述
			{
				gStructBinfileDesc.sPlanNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPlanNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPlanNameParams, sizeof(gStructBinfileDesc.sPlanNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//下载日期描述
			{
				memcpy(&gStructBinfileDesc.sDateNameParams,&udp_info,sizeof(gStructBinfileDesc.sDateNameParams));
				SendSuccessMsg(socketFd,fromAddr);
				iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//上载日期描述
			{
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
				memcpy(&gStructBinfileCustom.sCountdownParams,udp_info.iValue,sizeof(gStructBinfileCustom.sCountdownParams));
				SendSuccessMsg(socketFd,fromAddr);
				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//上载倒计时牌设置
			{
				memcpy(udp_info.iValue,&gStructBinfileCustom.sCountdownParams,sizeof(gStructBinfileCustom.sCountdownParams));
				result = sendto(socketFd, &udp_info, sizeof(gStructBinfileCustom.sCountdownParams) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//下载串口配置
			{
                if((udp_info.iValue[0] >= 0) && (udp_info.iValue[0] <= 4))
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
			    if((udp_info.iValue[0] >= 0) && (udp_info.iValue[0] <= 4))
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
			    pthread_rwlock_rdlock(&gCountDownLock);
				if(stepFlag == 0)//如果当前不是步进状态，那么就根据特殊控制方案号来给倒计时参数中方案描述等赋值。
				{
					UpdateCountdownParams();
				}
				else     //如果当前是步进状态，那么，在步进线程里会单独给倒计时接口赋值，这里只需要进行拷贝即可。
				{
					memcpy(&gCountDownParamsSend,gCountDownParams,sizeof(gCountDownParamsSend));
					gCountDownParamsSend.ucReserved[0] = 1;//这里其实就是步进标识。
				}
                gCountDownParamsSend.ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;//只更新发送的。
				
				memcpy(gCountDownParamsSend.ucChannelStatus,finalChannelStatus,sizeof(gCountDownParamsSend.ucChannelStatus));
				result = sendto(socketFd, &gCountDownParamsSend, sizeof(gCountDownParamsSend), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
                pthread_rwlock_unlock(&gCountDownLock);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//信号机重启
			{
				system("sync");
				SendSuccessMsg(socketFd,fromAddr);
				INFO("System will restart in 1s .\n");
				sprintf(msg,"重启信号机设备");
				WritePlatfomCtrlInfos("/home/Platform.log",msg);
				RecordNewFault(UNNORMAL_OR_SOFTWARE_REBOOT);
				sleep(1);
				system("reboot");
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb9)//开启通道锁定
			{
			    memcpy(&gStructBinfileCustom.sChannelLockedParams,&udp_info,sizeof(gStructBinfileCustom.sChannelLockedParams));
			    gStructBinfileCustom.cChannelLockFlag = ((gStructBinfileCustom.cChannelLockFlag == 0) ? 1 : gStructBinfileCustom.cChannelLockFlag);
			    gCountDownParams->ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;
			    gCountDownParamsSend.ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;
			    CopyChannelLockInfoToCountDownParams(gCountDownParams,&gStructBinfileCustom.sChannelLockedParams);
			    CopyChannelLockInfoToCountDownParams(&gCountDownParamsSend,&gStructBinfileCustom.sChannelLockedParams);
			    //INFO("udp_server_process  %d  %d \n",gCountDownParamsSend.ucChannelStatus[0],gCountDownParamsSend.ucChannelStatus[1]);
			    WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    SendSuccessMsg(socketFd,fromAddr);
				sprintf(msg,"开启通道锁定");
				WritePlatfomCtrlInfos("/home/Platform.log",msg);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xba)//关闭通道锁定
			{
				//0表示原通道锁定解锁, 1表示新通道锁定解锁, 2表示新通道锁定恢复
				INFO("Channel Locked Operaton:%d !\n", udp_info.iValue[0]);
				switch(udp_info.iValue[0])
				{
					case 0:
						{
					   	 	gStructBinfileCustom.cChannelLockFlag = 0;
						    gCountDownParams->ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;
						    gCountDownParamsSend.ucChannelLockStatus = gStructBinfileCustom.cChannelLockFlag;
							SendSuccessMsg(socketFd,fromAddr);
						    INFO("Channel Locked Operaton Canceled !\n");
							sprintf(msg,"关闭通道锁定");
						}
						break;
					case 1:
						{
							gStructBinfileCustom.sNewChannelLockedParams.uChannelLockFlag = 0;
							gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = 0;							
							SendSuccessMsg(socketFd,fromAddr);
							INFO("New Channel Locked Operaton Canceled !\n");
							sprintf(msg,"关闭新通道锁定");
						}
						break;
					case 2:
						{
							gStructBinfileCustom.sNewChannelLockedParams.uChannelLockFlag = 1;
							gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = 0;
							SendSuccessMsg(socketFd,fromAddr);
							INFO("New Channel Locked Operaton Recovered !\n");
							sprintf(msg,"恢复新通道锁定");
						}
						break;
					default:
						ERR("Wrong type for Channel Locked Operaton ! type:%d\n", udp_info.iValue[0]);
						udp_info.iValue[0]=0;
						result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
						break;
				}
				
				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				WritePlatfomCtrlInfos("/home/Platform.log",msg);

			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd3)//开启新通道锁定
			{
				gStructBinfileCustom.sNewChannelLockedParams.uChannelLockFlag = 1;
				gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = 0;
			    memcpy(&gStructBinfileCustom.sNewChannelLockedParams.stChannelLockPeriods,(char *)&udp_info+8,sizeof(gStructBinfileCustom.sNewChannelLockedParams.stChannelLockPeriods));

				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				SendSuccessMsg(socketFd,fromAddr);
				sprintf(msg,"开启新通道锁定");
				WritePlatfomCtrlInfos("/home/Platform.log",msg);

			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd4)//新通道锁定查询
			{
				memcpy(&udp_info.iValue[0], gStructBinfileCustom.sNewChannelLockedParams.stChannelLockPeriods, sizeof(gStructBinfileCustom.sNewChannelLockedParams.stChannelLockPeriods));
				*((char *)udp_info.iValue + sizeof(gStructBinfileCustom.sNewChannelLockedParams.stChannelLockPeriods)) = gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus; 	
				result = sendto(socketFd, &udp_info, 12+sizeof(CHANNEL_LOCK_PERIOD_PARAMS)*16, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//手动特殊控制命令
			{
#if 0	//changed by Jicky
				if(gStructBinfileCustom.cSpecialControlSchemeId == 0)//如果当前是系统控制(包括可能运行普通方案号，也可能是运行特殊控制方案号)
				{
                    ucPlanNo = gCountDownParams->ucPlanNo;
                    strcpy((char *)ucCurPlanDsc,(char *)gCountDownParams->ucCurPlanDsc);
				}
#endif
				if (gStructBinfileCustom.cSpecialControlSchemeId != udp_info.iValue[0])
				{
					gStructBinfileCustom.cSpecialControlSchemeId = udp_info.iValue[0];
					if (gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_YELLOW_BLINK 
						|| gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_TURN_OFF
						|| gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_ALL_RED)
					{
			    		pthread_rwlock_wrlock(&gCountDownLock);
						gCountDownParams->ucCurRunningTime = gCountDownParams->ucCurCycleTime;
			    		pthread_rwlock_unlock(&gCountDownLock);
						outPutCount = 0;	//清空倒计时调用计数
					}
					if (gStepFlag == 1)
					{
						StepCancel();
						stepFlag = 0;
					}
                    gStructMsgRealtimePatternInfo.nPatternId = gStructBinfileCustom.cSpecialControlSchemeId;//根据平台要求，需要将特殊方案号上传

					INFO("udp_server_process  control scheme id :  %d\n",gStructBinfileCustom.cSpecialControlSchemeId);
					sprintf(msg,"特殊控制，控制号:%d",gStructBinfileCustom.cSpecialControlSchemeId);
					WritePlatfomCtrlInfos("/home/Platform.log",msg);
					WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));					

				}
			    SendSuccessMsg(socketFd,fromAddr);
#if 0	//changed by Jicky
                if(gStructBinfileCustom.cSpecialControlSchemeId == 0)//当恢复系统控制时，将已保存的方案号及方案描述填写到倒计时接口中。
                {
                    gCountDownParams->ucPlanNo = ucPlanNo;
                    strcpy((char *)gCountDownParams->ucCurPlanDsc,(char *)ucCurPlanDsc);
                    //INFO("udp_server_process  resave  ID  %d\n",gCountDownParams->ucPlanNo);
                }
#endif
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//信号机步进控制
			{
			    if (IsStepValid((UInt8)stepCtrlParams->unStepNo, stepFlag))
				{
					stepFlag++;
					stepCtrlParams->unStepNo = 1;	//表示步进成功
				}
				else
					stepCtrlParams->unStepNo = 0;	//表示步进失败
				result = sendto(socketFd, stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); 
				sprintf(msg,"开启步进控制，阶段号:%d",gStructBinfileCustom.cSpecialControlSchemeId);
				INFO("----config Plan:%d, current Plan: %d stepCtrlParams->unStepNo %d  stepFlag %d\n", gStructBinfileCustom.cSpecialControlSchemeId, gStructMsgRealtimePatternInfo.nPatternId,stepCtrlParams->unStepNo,stepFlag);
				gStructMsgRealtimePatternInfo.nPatternId= 249;
				WritePlatfomCtrlInfos("/home/Platform.log",msg);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//信号机取消步进
			{
			    stepFlag = 0;
				StepCancel();
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				sprintf(msg,"取消步进");
				WritePlatfomCtrlInfos("/home/Platform.log",msg);
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd0)//实时运行方案信息
			{
                pthread_rwlock_rdlock(&gCountDownLock);
                gStructMsgRealtimePatternInfo.unExtraParamHead = 0x6e6e;
                gStructMsgRealtimePatternInfo.unExtraParamID = 0xd0;
                result = sendto(socketFd, &gStructMsgRealtimePatternInfo, sizeof(gStructMsgRealtimePatternInfo), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
                pthread_rwlock_unlock(&gCountDownLock);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd1)//实时流量统计获取接口,其实查询的是上一个统计周期的数据
			{
                MsgRealtimeVolume vol;
                memset(&vol,0,sizeof(vol));
                vol.unExtraParamHead = 0x6e6e;
                vol.unExtraParamID = 0xd1;
                GetVehicleFlowData(&vol.volumeOccupancy);
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
                result = sendto(socketFd, &vol, sizeof(vol), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd2)//平台1049协议
			{
                pthread_rwlock_rdlock(&gCountDownLock);
                //以下是对1049协议的封装
                gStructMsg1049.unExtraParamHead = 0x6e6e;
                gStructMsg1049.unExtraParamID = 0xd2;
                gStructMsg1049.unExtraDataLen = sizeof(gStructMsg1049.nPatternArray)+sizeof(gStructMsg1049.nPhaseArray);                
                result = sendto(socketFd, &gStructMsg1049, sizeof(gStructMsg1049), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
                pthread_rwlock_unlock(&gCountDownLock);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd5)//无线遥控模块设置
			{
				INFO("Wireless Control. switch: %d, overtime: %d", udp_info.iValue[0],udp_info.iValue[1]);
				switch(udp_info.iValue[0])
				{
					case WIRELESS_CTRL_OFF://关闭无线遥控
					{
						gStructBinfileConfigPara.stWirelessController.iSwitch = WIRELESS_CTRL_OFF;
						gStructBinfileConfigPara.stWirelessController.iOvertime = 0;
						WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
						SendSuccessMsg(socketFd,fromAddr);
					}
					break;
					case WIRELESS_CTRL_ON://开启无线遥控
					{
						
						gStructBinfileConfigPara.stWirelessController.iSwitch = WIRELESS_CTRL_ON;
						if(udp_info.iValue[1] <= 0)
							gStructBinfileConfigPara.stWirelessController.iOvertime = 300;//default value
						else
							gStructBinfileConfigPara.stWirelessController.iOvertime = udp_info.iValue[1];
						if(udp_info.iValue[2]>=0 && udp_info.iValue[2]<=1)
							gStructBinfileConfigPara.stWirelessController.iCtrlMode = udp_info.iValue[2];
						else
							gStructBinfileConfigPara.stWirelessController.iCtrlMode = WIRELESS_CTRL_DEFAULT;
						memcpy(gStructBinfileConfigPara.stWirelessController.key, &udp_info.iValue[3], sizeof(gStructBinfileConfigPara.stWirelessController.key));
						WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
						SendSuccessMsg(socketFd,fromAddr);
					}
					break;
					default:
						ERR("Wrong msg type in Wireless Controller, type: %d", udp_info.iValue[0]);
						SendFailureMsg(socketFd,fromAddr);
					break;
				}
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd6)//无线控制模块查询
			{
				INFO("Get Wireless controller Info.");
				udp_info.iValue[0] = gStructBinfileConfigPara.stWirelessController.iSwitch;
				udp_info.iValue[1] = gStructBinfileConfigPara.stWirelessController.iOvertime;
				result = sendto(socketFd, &udp_info, 16, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == E_COMMON_MSG_HEAD && udp_info.iType == E_DSC_PROTOCOL_TYPE_GET)//信号机协议信息获取
			{
				STRU_DSC_PROTOCOL_INFO dp;
				int i;
				char *dsctype[]={"DS-TSC500","DS-TSC300-44","DS-TSC300-22"};
				dp.wProtocol = E_PROTOCOL_NTCIP;
				dp.wPort = 161;
				INFO("HardWare version info:%s\n", HARDWARE_VERSION_INFO);
				for(i=0; i<3; i++)
				{
					if(0==strcmp(dsctype[i], HARDWARE_VERSION_INFO))
						break;
				}
				if(i != 3)
					dp.wDscType = i;
				else
					dp.wDscType = 3;//unkown
				memcpy((char*)udp_info.iValue, &dp, sizeof(dp));
				result = sendto(socketFd, &udp_info, sizeof(dp)+8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else 
			{
				UpAndDownLoadDeal(socketFd, fromAddr);	//上下载处理
				if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
				{
				//	log_debug("udp_server_process  recv completed . \n");
					nIsVerifyOK = StoreComplete();
					if (nIsVerifyOK == 0)
					{
						gCountdownConfigFlag = COUNTDOWN_CONFIG_UPDATE;
					}
					
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
#if 0	//下载完成的回复信息已经在UpAndDownLoadDeal函数中实现了，注释掉此块
					udp_info.iValue[0] = nIsVerifyOK;
					result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
#endif					
				}
			}
	
			if ( result == -1 )
    		{
				DBG("sendto udp info error!!!\n");
    		}
        }
        
    }
}

void* thread_adjust_control_type()
{
    while(1)
    {
		adjustControlType();
		usleep(100000);
    }

    return NULL;
}

/*********************************************************************************
*
* udp线程初始化。
*
***********************************************************************************/
int udp_server_init()
{
	int result = 0;
	pthread_t thread_id;
	result = pthread_create(&thread_id,NULL,(void *) udp_server_process,NULL);	
	if(result != 0 )
	{
		printf("Create udp server pthread error!\n");
		return 0;
	}

	result = pthread_create(&thread_id,NULL,(void *) thread_adjust_control_type,NULL);	
	if(result != 0 )
	{
		printf("Create thread_adjust_control_type pthread error!\n");
		return 0;
	}
	
	return 1;

}

//根据类型，记录新故障
void RecordNewFault(int type)
{
    time_t now;  
    struct FAILURE_INFO info; 
    
    //获取国际标准时间
    time(&now); 
    info.nNumber = (++gStructBinfileConfigPara.cFailureNumber);     
    info.nTime = now;
	info.nValue = 0;     
	info.nID = type;

    WriteFailureInfos("/home/FailureLog.dat",info);
}

//根据动作号，记录特殊动作
void RecordTimeintervalSpecialAction(int actionId)
{
    time_t now;  
    struct FAILURE_INFO info; 
    
    //获取国际标准时间
    time(&now); 
    info.nNumber = (++gStructBinfileConfigPara.cFailureNumber);     
    info.nTime = now;
	info.nValue = 0;     

    switch(actionId)
    {
        case 115: info.nID = TIMEINTERVAL_TURN_OFF; break;
        case 116: info.nID = TIMEINTERVAL_ALL_RED; break;
        //case 118: info.nID = MANUAL_PANEL_FLASH; break;
        case 119: info.nID = TIMEINTERVAL_FLASH; break;
        default:return;
    }

    WriteFailureInfos("/home/FailureLog.dat",info);
}


//记录手动按键
static void RecordKeyLog(int result)
{
    time_t now;  
    struct FAILURE_INFO info; 
    
    //获取国际标准时间
    time(&now); 
    info.nNumber = (++gStructBinfileConfigPara.cFailureNumber);     
    info.nTime = now;
	info.nValue = 0;     //通道号   

    switch(result)
    {
        case 1: info.nID = AUTO_TO_MANUAL; break;
        case 2: info.nID = MANUAL_TO_AUTO; break;
        case 3: info.nID = MANUAL_PANEL_FLASH; break;
        case 4: info.nID = MANUAL_PANEL_ALL_RED; break;
        case 5: info.nID = MANUAL_PANEL_STEP; break;
        default:return;
    }
	
    WriteFailureInfos("/home/FailureLog.dat",info);
}

/*********************************************************************************
*
*  键盘处理
	返回值:
	0: 无键按下.
	1:自动键按下,且已放开.
	2:手动键按下,且已放开.
	3:黄闪键按下,且已放开.
	4:全红键按下,且已放开.
	5:步进键按下,且已放开.
*
***********************************************************************************/
int gWirelessCtrlLockChanId=0;
int KeyControlDeal(int key)
{
	int ret=0;

	if(key==0)
		return 0;

	INFO("KeyControlDeal switch:%d, ctrlMode:%d, key:%d", gStructBinfileConfigPara.stWirelessController.iSwitch, gStructBinfileConfigPara.stWirelessController.iCtrlMode, key);
	if((WIRELESS_CTRL_ON==gStructBinfileConfigPara.stWirelessController.iSwitch) && (WIRELESS_CTRL_SELFDEF==gStructBinfileConfigPara.stWirelessController.iCtrlMode))
	{
		INFO("Wireless start to lock channel: id:%d", key);
		if(key>0 && key<=MAX_WIRELESS_KEY)
		{
			if(key == 2)//default key2 --> auto
			{
				KeyDefaultOperation(1);
				key = 0;
			}
			else if(key != 1)
				key = key -1;
			gWirelessCtrlLockChanId=key;
		}
	}
	else
	{
		ret=key;
		if(WIRELESS_CTRL_ON == gStructBinfileConfigPara.stWirelessController.iSwitch)
			ret=keyCheck(key);	
		if(ret != 0)
			KeyDefaultOperation(ret);
	}

	return 0;
}
int KeyDefaultOperation(int key)
{
	struct UDP_INFO udpdata;
	//创建UDP服务器
	int socketFd = -1;
    struct sockaddr_in localAddr;
	
	if (key == 0 || key == 2)
		return key;
	memset(&udpdata, 0, sizeof(udpdata));
	udpdata.iHead = 0x6e6e;
	udpdata.iType = 0xbb;

	RecordKeyLog(key);
	
	switch (key)
	{
		case 1:
			udpdata.iValue[0] = SPECIAL_CONTROL_SYSTEM;
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_INDUCTION);
			usleep(1000);
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_SYSTEM);
			break;
		case 3:
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
			udpdata.iValue[0] = SPECIAL_CONTROL_YELLOW_BLINK;
			break;
		case 4: 
			udpdata.iValue[0] = SPECIAL_CONTROL_ALL_RED;
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_ALL_RED);
			break;
		case 5: udpdata.iValue[0] = 0; 
		udpdata.iType = 0xbc; 
		key = 0; break;	//单步步进
		default:
			return key;
	}
	
	memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = sizeof(localAddr);

    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    localAddr.sin_port = htons (20000);
	localLen = sendto(socketFd,&udpdata,sizeof(udpdata),0,(struct sockaddr *)&localAddr,localLen);
	close(socketFd);
	
	return key;
}

