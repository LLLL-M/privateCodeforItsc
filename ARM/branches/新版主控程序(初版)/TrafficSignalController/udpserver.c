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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "common.h"



struct UDP_INFO udp_info;       
struct UDP_CUR_VALUE_INFO udp_cur_value_info;     //红灯电流值返回数据包
extern struct SPECIAL_PARAMS s_special_params;   //特殊参数定义
extern unsigned char g_RedCurrentValue[32];
extern unsigned int g_failurenumber;          //故障事件序号
struct CURRENT_PARAMS_UDP current_params_udp_info;
extern struct CURRENT_PARAMS g_struRecCurrent[32];   //电流参数


extern void set_failure_number(unsigned int failurenumber);
extern void get_special_params();
extern void get_failure_number();
extern void get_current_params();
extern void set_special_params(struct SPECIAL_PARAMS special_params);
extern void set_current_params();



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

    while(1)
    {
        // 接收数据      
        ssize_t result = recvfrom(socketFd, &udp_info, sizeof(UDP_INFO), 0, (struct sockaddr *)&fromAddr, &fromLen);    
        if ( -1 == result )
        {
			printf("recvfrom udp info error!!!\n");
            continue;
        }
        else
        {
			printf("Udp head:%x\nUdp type:%x\nUdp value:%0x\n",udp_info.iHead,udp_info.iType,udp_info.iValue[0]);
			//下载特殊检测参数
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)
			{
				s_special_params.iErrorDetectSwitch = udp_info.iValue[0]& 0x1;
				s_special_params.iCurrentAlarmSwitch = (udp_info.iValue[0] & 0x2) >> 1;
				s_special_params.iVoltageAlarmSwitch = (udp_info.iValue[0] & 0x4) >> 2;
				s_special_params.iCurrentAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x8) >> 3;
				s_special_params.iVoltageAlarmAndProcessSwitch = (udp_info.iValue[0] & 0x10) >> 4;
				s_special_params.iWatchdogSwitch = (udp_info.iValue[0] & 0x20) >> 5;
				s_special_params.iGpsSwitch = (udp_info.iValue[0] & 0x40) >> 6;	
				//将特殊参数保存到ini配置文件
				set_special_params(s_special_params);
				//下载成功后返回下载成功消息包给配置工具
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
				if ( result == -1 )
        		{
            		printf("sendto udp info error!!!\n");
        		}
				//exit(0);
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
					printf("sendto udp info error!!!\n");
        		}
				//exit(0);
			}
			//故障清除
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)
			{
				FILE * pFile = NULL;
				//清除故障日志
			    pFile = fopen("/home/FailureLog.dat", "w+");
			    if(pFile == NULL)
			    {
			    	printf("故障记录文件未能正确打开!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			        continue;
			    }
				fclose(pFile);	
				pFile = fopen("/home/FaultStatus.dat", "w+");
			    if(pFile == NULL)
			    {
			    	printf("/home/FaultStatus.dat文件未能正确打开!\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			        continue;
			    }
				fclose(pFile);	
				pFile = fopen("/home/FaultLog.dat", "w+");
			    if(pFile == NULL)
			    {
			    	printf("/home/FaultLog.dat文件未能正确打开!\n");
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
            		printf("sendto udp info error!!!\n");
        		}
				//exit(0);
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
					printf("打开FailureLog.dat失败\n");
					sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
					continue;
				}
				
				if( stat("/home/FailureLog.dat", &f_stat ) == -1 )
				{
			        fclose(pFile);
					printf("获取故障记录文件信息失败!\n");
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
            		printf("sendto udp info error!!!\n");
        		}
				//exit(0);
				
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
            		printf("sendto udp info error!!!\n");
					
				}
				//exit(0);
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
					printf("上传通道%02d红灯电流基准值:%03d\n",i+1,current_params_udp_info.struRecCurrent[i].RedCurrentBase);
					printf("上传通道%02d红灯电流基准值:%03d\n",i+1,current_params_udp_info.struRecCurrent[i].RedCurrentDiff);
				}			
				result = sendto(socketFd, &current_params_udp_info, sizeof(current_params_udp_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		printf("sendto udp info error!!!\n");
				}
				//exit(0);
			}
			//下载电流参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)
			{
				for(i=0;i<32;i++)
				{
					printf("通道%02d红灯电流基准值:%03d\n",i+1,udp_info.iValue[2*i]);
					printf("通道%02d红灯电流偏差值:%03d\n",i+1,udp_info.iValue[2*i+1]);
				}
				memcpy(g_struRecCurrent, udp_info.iValue, 32*8);
				
				for(i=0;i<32;i++)
				{
					printf("保存通道%02d红灯电流基准值:%03d\n",i+1,g_struRecCurrent[i].RedCurrentBase);
					printf("保存通道%02d红灯电流偏差值:%03d\n",i+1,g_struRecCurrent[i].RedCurrentDiff);
				}
				set_current_params();
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		printf("sendto udp info error!!!\n");
				}
				//exit(0);
			}
			//恢复默认参数
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)
			{
				//将配置文件保存目录删除
				system("rm -rf /home/data/TSC*");
				
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		printf("sendto udp info error!!!\n");
				}
			}
			//下载eth0的IP地址
			/*else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15d)
			{
				
				
				udp_info.iValue[0] = 0;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //发送消息头和消息类型
				if ( result == -1 )
        		{
            		printf("sendto udp info error!!!\n");
				}
				//system("reboot");
				//exit(0);
			}*/
           
        }
        
        usleep(100);
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
	return 1;

}
