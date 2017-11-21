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
PHASE_COUNTING_DOWN_PARAMS phase_counting_down_params; //相位倒计时参数
extern CountDownVeh countdown_veh[16];                 //机动车相位倒计时参数
extern CountDownPed countdown_ped[16];				   //行人相位倒计时参数
extern struct Count_Down_Params g_struCountDown;      //倒计时参数
extern COM_PARAMS com_params;                         //串口设置参数
extern int HardwareWatchdoghd;                      //硬件看门狗句柄
extern int is_WatchdogEnabled;						//硬件看门狗是否被使能标记



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
	int i = 0;
    struct sockaddr_in localAddr;
	struct sockaddr_in fromAddr;
	memset((char *)&localAddr, 0, (int)sizeof(localAddr));
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);
	
	memset(&udp_info,0,sizeof(UDP_INFO));

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

	//fcntl(socketFd, F_SETFL, O_NONBLOCK);	//设置为非阻塞模式
    while(1)
    {
        // 接收数据  
        DBG("udp_server_process   recving ...\n");
        ssize_t result = recvfrom(socketFd, &udp_info, sizeof(UDP_INFO), 0, (struct sockaddr *)&fromAddr, &fromLen);
		DBG("recv size = %d\n", result);

		if(-1 == result)
		{
            DBG("############===>  Failed Error   %s\n",strerror(errno));
		}
        else
        {
			//DBG("Udp head:%x\nUdp type:%x\nUdp value:%0x\n",udp_info.iHead,udp_info.iType,udp_info.iValue[0]);
			//下载特殊检测参数
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)
			{
				s_special_params.iErrorDetectSwitch = udp_info.iValue[0]& 0x1;
				s_special_params.iCurrentAlarmSwitch = (udp_info.iValue[0] & 0x2) >> 1;
				s_special_params.iVoltageAlarmSwitch = (udp_info.iValue[0] & 0x4) >> 2;
				s_special_params.iCurrentAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x8) >> 3;
				s_special_params.iVoltageAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x10) >> 4;
				if(s_special_params.iWatchdogSwitch == 0 && ((udp_info.iValue[0] & 0x20) >> 5) == 1)
				{
					//硬件看门狗从关闭到打开
					system("killall -9 watchdog &");
					s_special_params.iWatchdogSwitch = 1;
					HardwareWatchdogInit();	
					s_special_params.iWatchdogSwitch = 0;
				}
				else if(s_special_params.iWatchdogSwitch == 1 && ((udp_info.iValue[0] & 0x20) >> 5) == 0)
				{
					//硬件看门狗从打开到关闭
					is_WatchdogEnabled = 0;
					close(HardwareWatchdoghd);
					system("watchdog -t 1 -T 3 /dev/watchdog &");
				}
				s_special_params.iWatchdogSwitch = ((udp_info.iValue[0] & 0x20) >> 5);
				
				if(s_special_params.iGpsSwitch == 0 && ((udp_info.iValue[0] & 0x40) >> 6) == 1)
				{
					//GPS从关闭到打开
					system("killall -9 GPS &");
					system("/root/GPS &");
				}
				else if(s_special_params.iGpsSwitch == 1 && ((udp_info.iValue[0] & 0x40) >> 6) == 0)
				{
					//GPS从打开到关闭
					system("killall -9 GPS &");
					Set_LED2_OFF();
				}
				s_special_params.iGpsSwitch = ((udp_info.iValue[0] & 0x40) >> 6);	
				
				//将特殊参数保存到ini配置文件
				set_special_params(s_special_params);
				//下载成功后返回下载成功消息包给配置工具
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
        		}
			}
			//上载特殊检测参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)
			{
				udp_info.iValue[0] = s_special_params.iErrorDetectSwitch
					| (s_special_params.iCurrentAlarmSwitch << 1)
					| (s_special_params.iVoltageAlarmSwitch << 2)
					| (s_special_params.iCurrentAlarmAndProcessSwitch << 3)
					| (s_special_params.iCurrentAlarmAndProcessSwitch << 4)
					| (s_special_params.iWatchdogSwitch << 5)
					| (s_special_params.iGpsSwitch << 6);
				
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				if ( result == -1 )
        		{
					DBG("sendto udp info error!!!\n");
        		}
			}
			//故障清除
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)
			{
				FILE * pFile = NULL;
				//清除故障日志
			    pFile = fopen("/home/FailureLog.dat", "w+");
			    if(pFile == NULL)
			    {
			    	DBG("故障记录文件未能正确打开!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			        continue;
			    }
				fclose(pFile);	
				pFile = fopen("/home/FaultStatus.dat", "w+");
			    if(pFile == NULL)
			    {
			    	DBG("/home/FaultStatus.dat文件未能正确打开!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			        continue;
			    }
				fclose(pFile);	
				pFile = fopen("/home/FaultLog.dat", "w+");
			    if(pFile == NULL)
			    {
			    	DBG("/home/FaultLog.dat文件未能正确打开!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			        continue;
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
			//上载故障信息
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15b)
			{
				FILE *pFile = NULL;
				struct stat f_stat;
				char tmpstr[1024*1024] = {};
				pFile = fopen("/home/FailureLog.dat", "rb");
				if (pFile == NULL)
				{
					DBG("打开FailureLog.dat失败\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
					continue;
				}
				
				if( stat("/home/FailureLog.dat", &f_stat ) == -1 )
				{
			        fclose(pFile);
					DBG("获取故障记录文件信息失败!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
					continue ;
				}
				fread(tmpstr, f_stat.st_size, 1, pFile);
				fclose(pFile);
				char errorinfo[1024*1024 + 8] = {};
				memcpy(errorinfo,&(udp_info.iHead),4);
				memcpy(errorinfo+4,&(udp_info.iType),4);
				memcpy(errorinfo+8,tmpstr,f_stat.st_size);
				result = sendto(socketFd, errorinfo, 8 + f_stat.st_size, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型			
				if(result == -1)
				{
            		DBG("sendto udp info error!!!\n");
        		}
				
			}
			//上载红灯电流
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)
			{
				udp_cur_value_info.iHead = udp_info.iHead;
				udp_cur_value_info.iType = udp_info.iType;
				memcpy(udp_cur_value_info.redCurrentValue, g_RedCurrentValue, 32);
				result = sendto(socketFd, &udp_cur_value_info, sizeof(udp_cur_value_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载电流参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)
			{
				current_params_udp_info.iHead = udp_info.iHead;
				current_params_udp_info.iType = udp_info.iType;
				get_current_params();
				for(i=0;i<32;i++)
				{
					current_params_udp_info.struRecCurrent[i].RedCurrentBase = g_struRecCurrent[i].RedCurrentBase;
					current_params_udp_info.struRecCurrent[i].RedCurrentDiff = g_struRecCurrent[i].RedCurrentDiff;
					DBG("上传通道%02d红灯电流基准值:%03d\n",i+1,current_params_udp_info.struRecCurrent[i].RedCurrentBase);
					DBG("上传通道%02d红灯电流基准值:%03d\n",i+1,current_params_udp_info.struRecCurrent[i].RedCurrentDiff);
				}			
				result = sendto(socketFd, &current_params_udp_info, sizeof(current_params_udp_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
				}
			}
			//下载电流参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)
			{
				for(i=0;i<32;i++)
				{
					DBG("通道%02d红灯电流基准值:%03d\n",i+1,udp_info.iValue[2*i]);
					DBG("通道%02d红灯电流偏差值:%03d\n",i+1,udp_info.iValue[2*i+1]);
				}
				memcpy(g_struRecCurrent, udp_info.iValue, 32*8);
				
				for(i=0;i<32;i++)
				{
					DBG("保存通道%02d红灯电流基准值:%03d\n",i+1,g_struRecCurrent[i].RedCurrentBase);
					DBG("保存通道%02d红灯电流偏差值:%03d\n",i+1,g_struRecCurrent[i].RedCurrentDiff);
				}
				set_current_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
				}
			}
			//恢复默认参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)
			{
				//将配置文件保存目录删除
				system("rm -rf /home/data/TSC*");
				system("rm -rf /home/*.log");
				system("rm -rf /home/*.dat");
				system("cp /home/config.bak /home/config.ini -f");				
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
				}
			}
			//下载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
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
					continue;
				}
				
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
				}
			}
           	//上载eth1、eth0、wlan0的IP地址,eth1对应IP-1,eth0对应IP-2,wlan0对应IP-WiFi
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{			
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
					continue;
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
			//下载相位描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)
			{
				memcpy(&phase_desc_params,&udp_info,sizeof(phase_desc_params));
				set_phase_desc_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载相位描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)
			{
				phase_desc_params.unExtraParamHead = udp_info.iHead;
				phase_desc_params.unExtraParamID = udp_info.iType;
				get_phase_desc_params();
				result = sendto(socketFd, &phase_desc_params, sizeof(phase_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//下载通道描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)
			{
				memcpy(&channel_desc_params,&udp_info,sizeof(channel_desc_params));
				set_channel_desc_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载通道描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)
			{
				channel_desc_params.unExtraParamHead = udp_info.iHead;
				channel_desc_params.unExtraParamID = udp_info.iType;
				get_channel_desc_params();
				result = sendto(socketFd, &channel_desc_params, sizeof(channel_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//下载方案描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)
			{
				memcpy(&pattern_name_params,&udp_info,sizeof(pattern_name_params));
				set_pattern_name_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载方案描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)
			{
				pattern_name_params.unExtraParamHead = udp_info.iHead;
				pattern_name_params.unExtraParamID = udp_info.iType;
				get_pattern_name_params();
				result = sendto(socketFd, &pattern_name_params, sizeof(pattern_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//下载计划描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)
			{
				memcpy(&plan_name_params,&udp_info,sizeof(plan_name_params));
				set_plan_name_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载计划描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)
			{
				plan_name_params.unExtraParamHead = udp_info.iHead;
				plan_name_params.unExtraParamID = udp_info.iType;
				get_plan_name_params();
				result = sendto(socketFd, &plan_name_params, sizeof(plan_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//下载日期描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)
			{
				memcpy(&date_name_params,&udp_info,sizeof(date_name_params));
				set_date_name_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载日期描述
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)
			{
				date_name_params.unExtraParamHead = udp_info.iHead;
				date_name_params.unExtraParamID = udp_info.iType;
				get_date_name_params();
				result = sendto(socketFd, &date_name_params, sizeof(date_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载软硬件版本
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)
			{
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
			//上载相位倒计时信息
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)
			{
				phase_counting_down_params.unExtraParamHead = udp_info.iHead;
				phase_counting_down_params.unExtraParamID = udp_info.iType;
				for(i = 0;i < 16;i ++)
				{
					phase_counting_down_params.stVehPhaseCountingDown[i][0] = countdown_veh[i+1].veh_color;
					phase_counting_down_params.stVehPhaseCountingDown[i][1] = countdown_veh[i+1].veh_phaseTime;
					phase_counting_down_params.stPedPhaseCountingDown[i][0] = countdown_ped[i+1].ped_color;
					phase_counting_down_params.stPedPhaseCountingDown[i][1] = countdown_ped[i+1].ped_phaseTime;
				}
				
				result = sendto(socketFd, &phase_counting_down_params, sizeof(phase_counting_down_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//下载倒计时牌设置
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)
			{
				memcpy(&g_struCountDown,udp_info.iValue,sizeof(g_struCountDown));
				set_count_down_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载倒计时牌设置
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)
			{
				get_count_down_params();
				memcpy(udp_info.iValue,&g_struCountDown,sizeof(g_struCountDown));
				result = sendto(socketFd, &udp_info, sizeof(g_struCountDown) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
				//exit(0);
			}
			//下载串口配置
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)
			{
				memcpy(&com_params,&udp_info,sizeof(com_params));
				set_com_params(com_params.unExtraParamValue);
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}
			//上载串口配置
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)
			{
				com_params.unExtraParamHead = udp_info.iHead;
				com_params.unExtraParamID = udp_info.iType;
				com_params.unExtraParamValue = udp_info.iValue[0];
				get_com_params(udp_info.iValue[0]);
				result = sendto(socketFd, &com_params, sizeof(com_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
			}

			//下载时段表信息
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xaa)
			{
				//DBG("receive timeInterval information\n");
				storeTimeIntervalToIni((void *)udp_info.iValue);
			}
			//下载调度表信息
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xab)
			{
				//DBG("receive planSchedule information\n");
				storePlanScheduleToIni((void *)udp_info.iValue);
			}			

			//信号机重启
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)
			{
				system("sync");
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		DBG("sendto udp info error!!!\n");
					
				}
				system("reboot");
			}

        }
        
    }
}

/*********************************************************************************
*
* 向主控模块发送黄闪控制信号
*
***********************************************************************************/

void udp_send_yellowflash()
{
	//创建UDP服务器
	int socketFd = -1;
	char yellowflash_msg[47] =   {0x30, 0x2D, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
    							  0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x20, 0x02,
    							  0x01, 0x03, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
    							  0x30, 0x15, 0x30, 0x13, 0x06, 0x0D, 0x2B, 0x06,
    							  0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 0x01,
    							  0x04, 0x0E, 0x00, 0x02, 0x02, 0x00, 0xFF};
    struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = sizeof(localAddr);
    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	return;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//使用161端口
    localAddr.sin_port = htons (161);
	//向本机161端口发送黄闪信号
	int len = sendto(socketFd,yellowflash_msg,47,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		printf("Send flashing signal failed!!!\n");
	}
	close(socketFd);
	
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



