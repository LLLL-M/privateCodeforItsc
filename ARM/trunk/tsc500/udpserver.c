#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "common.h"
#include "HikConfig.h"
#include "platform.h"

struct UDP_INFO udp_info;       
struct STRU_N_IP_ADDRESS ip_info;
struct UDP_CUR_VALUE_INFO udp_cur_value_info;          //红灯电流值返回数据包
extern struct SPECIAL_PARAMS s_special_params;         //特殊参数定义
extern unsigned char g_RedCurrentValue[32];
extern unsigned int g_failurenumber;                   //故障事件序号
struct CURRENT_PARAMS_UDP current_params_udp_info;
extern struct CURRENT_PARAMS g_struRecCurrent[32];     //电流参数
extern PHASE_DESC_PARAMS phase_desc_params;            //相位描述      
extern CHANNEL_DESC_PARAMS channel_desc_params;        //通道描述
extern PATTERN_NAME_PARAMS pattern_name_params;        //方案描述
extern PLAN_NAME_PARAMS plan_name_params;              //计划描述
extern DATE_NAME_PARAMS date_name_params;		       //日期描述
DEVICE_VERSION_PARAMS device_version_params;           //设备软硬件信息
extern CountDownVeh countdown_veh[16];                 //机动车相位倒计时参数
extern CountDownPed countdown_ped[16];				   //行人相位倒计时参数
extern struct Count_Down_Params g_struCountDown;      //倒计时参数
extern COM_PARAMS com_params;                         //串口设置参数
extern int HardwareWatchdoghd;                      //硬件看门狗句柄
extern int is_WatchdogEnabled;						//硬件看门狗是否被使能标记
extern CHANNEL_LOCK_PARAMS gChannelLockedParams;
extern void *gHandle;
extern COM_PARAMS g_com_params[4];
extern void HardwareWatchdogInit();
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //倒计时接口信息
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //用来供udpserver调用的倒计时缓存
extern pthread_rwlock_t gCountDownLock;

extern UInt8 gChannelLockFlag;   //0表示未锁定，1表示锁定
extern UInt8 gSpecialControlSchemeId;       //特殊控制方案号

/*********************************************************************************
*
* 	保存网络参数
*
***********************************************************************************/
int SaveNetparam(const char *ifName, const char *chIP, const char *chMask, const char *chGateway)
{
	char g_strNetParam[64][64];       //网络配置参数
	int i = 0;
	int index = 0;
	FILE *fp = NULL;
	if (NULL == ifName || NULL == chIP || NULL == chMask || NULL == chGateway)
	{
		return -1;
	}
	memset(g_strNetParam,0,64*64);
	printf("ifName:%s,chIP:%s,chMask:%s,chGateway:%s\n", ifName,chIP,chMask,chGateway);
	fp = fopen("/etc/network/interfaces", "r");
	if (NULL == fp)
	{
		return -2;
	}
	
	while (NULL != fgets(g_strNetParam[i], 64, fp) && i<64)
	{
		//printf("%s",g_strNetParam[i]);
		if (0 == strcmp(ifName, "eth0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))          //找到iface eth0这行
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		else if (0 == strcmp(ifName, "eth1"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth1"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		else if (0 == strcmp(ifName, "wlan0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface wlan0"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					sprintf(g_strNetParam[i], "address %s\n", chIP);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					sprintf(g_strNetParam[i], "netmask %s\n", chMask);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					sprintf(g_strNetParam[i], "gateway %s\n", chGateway);
				}
			}
		}
		i++;
	}
	fflush(fp);
	fsync(fileno(fp));
	
	fclose(fp);
	if (index > 0)
	{
		fp = fopen("/etc/network/interfaces", "w");
		if (NULL == fp)
		{
			return -2;
		}
		for(i=0;i<64;i++)
		{
			fprintf(fp,"%s",g_strNetParam[i]);
		}
        fflush(fp);
        fsync(fileno(fp));
		fclose(fp);
		system("/etc/init.d/S40network restart");
		return 0;
	}
	return -3;
}


/*********************************************************************************
*
* 	获取网络参数
*
***********************************************************************************/
int GetNetparam(const char *ifName, char *chIP, char *chMask, char *chGateway)
{
	char g_strNetParam[64][64];       //网络配置参数
	int i = 0;
	int index = 0;
	FILE *fp = NULL;
	if (NULL == ifName || NULL == chIP || NULL == chMask || NULL == chGateway)
	{
		return -1;
	}
	memset(g_strNetParam,0,64*64);
	fp = fopen("/etc/network/interfaces", "r");
	if (NULL == fp)
	{
		return -2;
	}
	
	while (NULL != fgets(g_strNetParam[i], 64, fp) && i<64)
	{
		if (0 == strcmp(ifName, "eth0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))          //找到iface eth0这行
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					memcpy(chIP,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					memcpy(chMask,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					memcpy(chGateway,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
			}
		}
		else if (0 == strcmp(ifName, "eth1"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface eth1"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					memcpy(chIP,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					memcpy(chMask,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					memcpy(chGateway,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
			}
		}
		else if (0 == strcmp(ifName, "wlan0"))
		{
			if (NULL != strstr(g_strNetParam[i], "iface wlan0"))
			{
				index = i;
			}
			else if (index > 0)
			{
				if (i > index + 3)
				{
					i++;
					continue;
				}
				if (NULL != strstr(g_strNetParam[i], "address"))
				{
					memcpy(chIP,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "netmask"))
				{
					memcpy(chMask,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
				else if (NULL != strstr(g_strNetParam[i], "gateway"))
				{
					memcpy(chGateway,g_strNetParam[i]+8,strlen(g_strNetParam[i])-9);
				}
			}
		}
		i++;
	}
	fclose(fp);
	if(index > 0)
	{
		return 0;
	}
	else
	{
		return -3;
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
    g_failurenumber = 0;
    set_failure_number(0);			
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
static void UploadFaultInfo(int socketFd,struct sockaddr_in fromAddr, int startLineNo, int lineNum)
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
		size = fread(errorinfo + 8, 1, sizeof(struct FAILURE_INFO) * lineNum, pFile);
		//INFO("block size = %d, total size = %d", sizeof(struct FAILURE_INFO), size);
	}
	fclose(pFile);

    result = sendto(socketFd, errorinfo, 8 + ((size < 4) ? 4 : size), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型			
    if(result == -1)
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
    if((s_special_params.iWatchdogSwitch == 0) && (p_SpecialParams->iWatchdogSwitch == 1))
    {
		//硬件看门狗从关闭到打开
		system("killall -9 watchdog &");
		s_special_params.iWatchdogSwitch = 1;
		HardwareWatchdogInit();	
		s_special_params.iWatchdogSwitch = 0;                
    }
    else if((s_special_params.iWatchdogSwitch == 1) && (p_SpecialParams->iWatchdogSwitch == 0))
    {
		//硬件看门狗从打开到关闭
		is_WatchdogEnabled = 0;
		close(HardwareWatchdoghd);
		system("watchdog -t 1 -T 3 /dev/watchdog &");
    }

	if((s_special_params.iGpsSwitch == 0) && (p_SpecialParams->iGpsSwitch == 1))
	{
		//GPS从关闭到打开
		system("killall -9 GPS &");
		system("/root/GPS &");
	}
	else if((s_special_params.iGpsSwitch == 1) && (p_SpecialParams->iGpsSwitch == 0))
	{
		//GPS从打开到关闭
		system("killall -9 GPS &");
		Set_LED2_OFF();
	}

    memcpy(&s_special_params,p_SpecialParams,sizeof(s_special_params));
    
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
    system("rm -rf /home/data/TSC*");
    system("rm -rf /home/*.log");
    system("rm -rf /home/*.dat");
    system("cp /home/config.bak /home/config.ini -f");				

    SendSuccessMsg(socketFd,fromAddr);
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
    memset(&ip_info,0,sizeof(ip_info));
    struct in_addr inaddr;
    char * pAddr = NULL;
    char netname[8] ="";
    inaddr.s_addr = (unsigned long)udp_info.iValue[0];
    pAddr = inet_ntoa(inaddr);
    strcpy((char *)&ip_info.address,pAddr);

    inaddr.s_addr = (unsigned long)udp_info.iValue[1];
    pAddr = inet_ntoa(inaddr);
    strcpy((char *)&ip_info.subnetMask,pAddr);

    inaddr.s_addr = (unsigned long)udp_info.iValue[2];
    pAddr = inet_ntoa(inaddr);
    strcpy((char *)&ip_info.gateway,pAddr);

    if(udp_info.iType == 0x15d)
    {
        strcpy(netname,"eth1");
    }
    else if(udp_info.iType == 0x15f)
    {
        strcpy(netname,"eth0");
    }
    else if(udp_info.iType == 0x161)
    {
        strcpy(netname,"wlan0");
    }

    //将获取的IP信息写入/etc/network/interfaces文件中
    if(SaveNetparam(netname,ip_info.address,ip_info.subnetMask,ip_info.gateway) != 0)
    {
        //修改IP失败
        return;
    }

    SendSuccessMsg(socketFd,fromAddr);    
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
    ssize_t result = 0;

    memset(&ip_info,0,sizeof(ip_info));
    char netname[8] ="";
    if(udp_info.iType == 0x15e)
    {
        strcpy(netname,"eth1");
    }
    else if(udp_info.iType == 0x160)
    {
        strcpy(netname,"eth0");
    }
    else if(udp_info.iType == 0x162)
    {
        strcpy(netname,"wlan0");
    }

    //从/etc/network/interfaces文件中获取IP信息
    if(GetNetparam(netname,ip_info.address,ip_info.subnetMask,ip_info.gateway) != 0)
    {
        //修改IP失败
    }
    DBG("%s:ip=%s,netmask=%s,address=%s\n",netname,ip_info.address,ip_info.subnetMask,ip_info.gateway);
    udp_info.iValue[0] = inet_addr(ip_info.address);
    udp_info.iValue[1] = inet_addr(ip_info.subnetMask);
    udp_info.iValue[2] = inet_addr(ip_info.gateway);

    result = sendto(socketFd, &udp_info, 20, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
    if ( result == -1 )
    {
        DBG("sendto udp info error!!!\n");
    }
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
	memcpy(device_version_params.hardVersionInfo, HARDWARE_VERSION_INFO, sizeof(HARDWARE_VERSION_INFO));        //低32位放置硬件信息
	memcpy(device_version_params.softVersionInfo, SOFTWARE_VERSION_INFO, sizeof(SOFTWARE_VERSION_INFO));       //高32位放置软件信息
	result = sendto(socketFd, &device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
		
	}
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
//	DestAddressInfo dst;

    STRU_SPECIAL_PARAMS s_SpecialParams;
    unsigned char iIsSaveSpecialParams = 0;
    unsigned char iIsSaveCustomParams = 0;
    unsigned char iIsSaveDescParams = 0;

 	UInt8 stepFlag = 0;
	STEP_CTRL_PARAMS *stepCtrlParams = (STEP_CTRL_PARAMS *)&udp_info;
	CANCEL_STEP_FEEDBACK_PARAMS *cancelStepFeedback = (CANCEL_STEP_FEEDBACK_PARAMS *)&udp_info;
	
	struct STRU_Extra_Param_Block *faultInfo = (struct STRU_Extra_Param_Block *)&udp_info;
	int startLineNo = 0, lineNum = 0;
	
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
				udp_info.iValue[0] = s_special_params.iErrorDetectSwitch
					| (s_special_params.iCurrentAlarmSwitch << 1)
					| (s_special_params.iVoltageAlarmSwitch << 2)
					| (s_special_params.iCurrentAlarmAndProcessSwitch << 3)
					| (s_special_params.iCurrentAlarmAndProcessSwitch << 4)
					| (s_special_params.iWatchdogSwitch << 5)
					| (s_special_params.iGpsSwitch << 6);
				
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//故障清除
			{
                ClearFaultInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15b)//上载部分故障信息
			{
                UploadFaultInfo(socketFd, fromAddr, startLineNo, lineNum);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)//上载哪些故障信息
			{
                startLineNo = faultInfo->unExtraParamFirst;
				lineNum = faultInfo->unExtraParamTotal;
                SendSuccessMsg(socketFd,fromAddr);
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
				memcpy(current_params_udp_info.struRecCurrent,g_struRecCurrent,sizeof(struct CURRENT_PARAMS)*32);
				result = sendto(socketFd, &current_params_udp_info, sizeof(current_params_udp_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//下载电流参数
			{
				memcpy(g_struRecCurrent, udp_info.iValue, 32*8);
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
				memcpy(&phase_desc_params,&udp_info,sizeof(phase_desc_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//上载相位描述
			{
				phase_desc_params.unExtraParamHead = udp_info.iHead;
				phase_desc_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &phase_desc_params, sizeof(phase_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//下载通道描述
			{
				memcpy(&channel_desc_params,&udp_info,sizeof(channel_desc_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//上载通道描述
			{
				channel_desc_params.unExtraParamHead = udp_info.iHead;
				channel_desc_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &channel_desc_params, sizeof(channel_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//下载方案描述
			{
				memcpy(&pattern_name_params,&udp_info,sizeof(pattern_name_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//上载方案描述
			{
				pattern_name_params.unExtraParamHead = udp_info.iHead;
				pattern_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &pattern_name_params, sizeof(pattern_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//下载计划描述
			{
				memcpy(&plan_name_params,&udp_info,sizeof(plan_name_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//上载计划描述
			{
				plan_name_params.unExtraParamHead = udp_info.iHead;
				plan_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &plan_name_params, sizeof(plan_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//下载日期描述
			{
				memcpy(&date_name_params,&udp_info,sizeof(date_name_params));
				SendSuccessMsg(socketFd,fromAddr);
				iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//上载日期描述
			{
				date_name_params.unExtraParamHead = udp_info.iHead;
				date_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &date_name_params, sizeof(date_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)//上载软硬件版本信息
			{
                UploadVersionInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)//下载倒计时牌设置
			{
				memcpy(&g_struCountDown,udp_info.iValue,sizeof(g_struCountDown));
				SendSuccessMsg(socketFd,fromAddr);
				set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//上载倒计时牌设置
			{
				memcpy(udp_info.iValue,&g_struCountDown,sizeof(g_struCountDown));
				result = sendto(socketFd, &udp_info, sizeof(g_struCountDown) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//下载串口配置
			{
				memcpy(g_com_params,&udp_info,sizeof(g_com_params));
				SendSuccessMsg(socketFd,fromAddr);
				set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//上载串口配置
			{
				com_params.unExtraParamHead = udp_info.iHead;
				com_params.unExtraParamID = udp_info.iType;
				com_params.unExtraParamValue = udp_info.iValue[0];
				result = sendto(socketFd, &com_params, sizeof(com_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType >= 0xaa && udp_info.iType <= 0xb6))//下载配置信息
			{
				DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 只是为了ctrl+f搜索方便!
				//SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)//获取倒计时参数信息
			{
			    pthread_rwlock_rdlock(&gCountDownLock);
			    gCountDownParamsSend.ucChannelLockStatus = gChannelLockFlag;
			    gCountDownParamsSend.ucPlanNo = ((gSpecialControlSchemeId >= 251) ? gSpecialControlSchemeId : gCountDownParamsSend.ucPlanNo);
				result = sendto(socketFd, &gCountDownParamsSend, sizeof(gCountDownParamsSend), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
                pthread_rwlock_unlock(&gCountDownLock);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//信号机重启
			{
				system("sync");
				SendSuccessMsg(socketFd,fromAddr);
				ERR("System will restart in 1s .\n");
				sleep(1);
				system("reboot");
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb9)//开启通道锁定
			{
			    memcpy(&gChannelLockedParams,&udp_info,sizeof(gChannelLockedParams));
			    gChannelLockFlag = ((gChannelLockFlag == 0) ? 1 : gChannelLockFlag);
			    gCountDownParams->ucChannelLockStatus = gChannelLockFlag;
			    gCountDownParamsSend.ucChannelLockStatus = gChannelLockFlag;
			    CopyChannelLockInfoToCountDownParams(gCountDownParams,&gChannelLockedParams);
			    CopyChannelLockInfoToCountDownParams(&gCountDownParamsSend,&gChannelLockedParams);
			    set_custom_params();
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xba)//关闭通道锁定
			{
			    gChannelLockFlag = 0;
			    gCountDownParams->ucChannelLockStatus = gChannelLockFlag;
			    gCountDownParamsSend.ucChannelLockStatus = gChannelLockFlag;
			    SendSuccessMsg(socketFd,fromAddr);
			    set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//手动特殊控制命令
			{
			    gSpecialControlSchemeId = udp_info.iValue[0];
			    SendSuccessMsg(socketFd,fromAddr);
			    ERR("udp_server_process  control scheme id :  %d\n",gSpecialControlSchemeId);
			    set_custom_params();
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
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//信号机取消步进
			{
			    stepFlag = 0;
				StepCancel();
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//经济型信号机型号设置
			{
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//获取经济型信号机型号
			{
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//下载参数开始
			{
			    StoreBegin((void *)udp_info.iValue);
                SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//下载配置完成
			{
			    ERR("udp_server_process  recv completed . \n");
				StoreComplete();
				
                //保存参数到配置文件
                if(iIsSaveCustomParams == 1)
                {
                    iIsSaveCustomParams = 0;
                    set_custom_params();
                }
                if(iIsSaveDescParams == 1 )
                {
                    iIsSaveDescParams = 0;
                    set_desc_params();
                }
                if(iIsSaveSpecialParams == 1)
                {
                    iIsSaveSpecialParams = 0;
                    set_special_params();
                }
                SendSuccessMsg(socketFd,fromAddr);
                
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
