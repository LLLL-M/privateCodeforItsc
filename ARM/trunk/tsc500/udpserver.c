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
struct UDP_CUR_VALUE_INFO udp_cur_value_info;          //��Ƶ���ֵ�������ݰ�
extern struct SPECIAL_PARAMS s_special_params;         //�����������
extern unsigned char g_RedCurrentValue[32];
extern unsigned int g_failurenumber;                   //�����¼����
struct CURRENT_PARAMS_UDP current_params_udp_info;
extern struct CURRENT_PARAMS g_struRecCurrent[32];     //��������
extern PHASE_DESC_PARAMS phase_desc_params;            //��λ����      
extern CHANNEL_DESC_PARAMS channel_desc_params;        //ͨ������
extern PATTERN_NAME_PARAMS pattern_name_params;        //��������
extern PLAN_NAME_PARAMS plan_name_params;              //�ƻ�����
extern DATE_NAME_PARAMS date_name_params;		       //��������
DEVICE_VERSION_PARAMS device_version_params;           //�豸��Ӳ����Ϣ
extern CountDownVeh countdown_veh[16];                 //��������λ����ʱ����
extern CountDownPed countdown_ped[16];				   //������λ����ʱ����
extern struct Count_Down_Params g_struCountDown;      //����ʱ����
extern COM_PARAMS com_params;                         //�������ò���
extern int HardwareWatchdoghd;                      //Ӳ�����Ź����
extern int is_WatchdogEnabled;						//Ӳ�����Ź��Ƿ�ʹ�ܱ��
extern CHANNEL_LOCK_PARAMS gChannelLockedParams;
extern void *gHandle;
extern COM_PARAMS g_com_params[4];
extern void HardwareWatchdogInit();
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //����ʱ�ӿ���Ϣ
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //������udpserver���õĵ���ʱ����
extern pthread_rwlock_t gCountDownLock;

extern UInt8 gChannelLockFlag;   //0��ʾδ������1��ʾ����
extern UInt8 gSpecialControlSchemeId;       //������Ʒ�����

/*********************************************************************************
*
* 	�����������
*
***********************************************************************************/
int SaveNetparam(const char *ifName, const char *chIP, const char *chMask, const char *chGateway)
{
	char g_strNetParam[64][64];       //�������ò���
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
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))          //�ҵ�iface eth0����
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
* 	��ȡ�������
*
***********************************************************************************/
int GetNetparam(const char *ifName, char *chIP, char *chMask, char *chGateway)
{
	char g_strNetParam[64][64];       //�������ò���
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
			if (NULL != strstr(g_strNetParam[i], "iface eth0"))          //�ҵ�iface eth0����
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
 �� �� ��  : SendSuccessMsg
 ��������  : �����ػ��ϴ���ɺ���ͻ��˷��ͳɹ���ʾ��
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : ClearFaultInfo
 ��������  : ���������Ϣ
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void ClearFaultInfo(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;
    FILE * pFile = NULL;
    //���������־
    pFile = fopen("/home/FailureLog.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("���ϼ�¼�ļ�δ����ȷ��!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    pFile = fopen("/home/FaultStatus.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("/home/FaultStatus.dat�ļ�δ����ȷ��!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    pFile = fopen("/home/FaultLog.dat", "w+");
    if(pFile == NULL)
    {
    	DBG("/home/FaultLog.dat�ļ�δ����ȷ��!\n");
    	sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    }
    fclose(pFile);	
    udp_info.iValue[0] = 0;
    //��������־�����Ϊ0
    g_failurenumber = 0;
    set_failure_number(0);			
    //��������󷵻ع�������ɹ���Ϣ�������ù���
    result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
    if ( result == -1 )
    {
    	DBG("sendto udp info error!!!\n");
    }

}

/*****************************************************************************
 �� �� ��  : UploadFaultInfo
 ��������  : �ϴ�������Ϣ
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
    	DBG("��FailureLog.datʧ��\n");
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

    result = sendto(socketFd, errorinfo, 8 + ((size < 4) ? 4 : size), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����			
    if(result == -1)
    {
    	DBG("sendto udp info error!!!\n");
    }
}

/*****************************************************************************
 �� �� ��  : DownloadSpecialParams
 ��������  : �����������
 �������  : int socketFd                          
             struct sockaddr_in fromAddr           
             STRU_SPECIAL_PARAMS *p_SpecialParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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

    //���¿��Ź���GPS����
    if((s_special_params.iWatchdogSwitch == 0) && (p_SpecialParams->iWatchdogSwitch == 1))
    {
		//Ӳ�����Ź��ӹرյ���
		system("killall -9 watchdog &");
		s_special_params.iWatchdogSwitch = 1;
		HardwareWatchdogInit();	
		s_special_params.iWatchdogSwitch = 0;                
    }
    else if((s_special_params.iWatchdogSwitch == 1) && (p_SpecialParams->iWatchdogSwitch == 0))
    {
		//Ӳ�����Ź��Ӵ򿪵��ر�
		is_WatchdogEnabled = 0;
		close(HardwareWatchdoghd);
		system("watchdog -t 1 -T 3 /dev/watchdog &");
    }

	if((s_special_params.iGpsSwitch == 0) && (p_SpecialParams->iGpsSwitch == 1))
	{
		//GPS�ӹرյ���
		system("killall -9 GPS &");
		system("/root/GPS &");
	}
	else if((s_special_params.iGpsSwitch == 1) && (p_SpecialParams->iGpsSwitch == 0))
	{
		//GPS�Ӵ򿪵��ر�
		system("killall -9 GPS &");
		Set_LED2_OFF();
	}

    memcpy(&s_special_params,p_SpecialParams,sizeof(s_special_params));
    
    SendSuccessMsg(socketFd,fromAddr);
}

/*****************************************************************************
 �� �� ��  : ResetAllParams
 ��������  : �ָ�Ĭ�ϲ���
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void ResetAllParams(int socketFd,struct sockaddr_in fromAddr)
{
    //�������ļ�����Ŀ¼ɾ��
    system("rm -rf /home/data/TSC*");
    system("rm -rf /home/*.log");
    system("rm -rf /home/*.dat");
    system("cp /home/config.bak /home/config.ini -f");				

    SendSuccessMsg(socketFd,fromAddr);
}

/*****************************************************************************
 �� �� ��  : DownloadIpAddress
 ��������  : ���ر���IP��ַ
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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

    //����ȡ��IP��Ϣд��/etc/network/interfaces�ļ���
    if(SaveNetparam(netname,ip_info.address,ip_info.subnetMask,ip_info.gateway) != 0)
    {
        //�޸�IPʧ��
        return;
    }

    SendSuccessMsg(socketFd,fromAddr);    
}

/*****************************************************************************
 �� �� ��  : UploadIpAddress
 ��������  : �ϴ�����IP��Ϣ
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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

    //��/etc/network/interfaces�ļ��л�ȡIP��Ϣ
    if(GetNetparam(netname,ip_info.address,ip_info.subnetMask,ip_info.gateway) != 0)
    {
        //�޸�IPʧ��
    }
    DBG("%s:ip=%s,netmask=%s,address=%s\n",netname,ip_info.address,ip_info.subnetMask,ip_info.gateway);
    udp_info.iValue[0] = inet_addr(ip_info.address);
    udp_info.iValue[1] = inet_addr(ip_info.subnetMask);
    udp_info.iValue[2] = inet_addr(ip_info.gateway);

    result = sendto(socketFd, &udp_info, 20, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
    if ( result == -1 )
    {
        DBG("sendto udp info error!!!\n");
    }
}

/*****************************************************************************
 �� �� ��  : UploadVersionInfo
 ��������  : �ϴ��汾��Ϣ
 �������  : int socketFd                 
             struct sockaddr_in fromAddr  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��11��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void UploadVersionInfo(int socketFd,struct sockaddr_in fromAddr)
{
    ssize_t result = 0;

	memset(&device_version_params,0,sizeof(device_version_params));
	device_version_params.unExtraParamHead = udp_info.iHead;
	device_version_params.unExtraParamID = udp_info.iType;
	memcpy(device_version_params.hardVersionInfo, HARDWARE_VERSION_INFO, sizeof(HARDWARE_VERSION_INFO));        //��32λ����Ӳ����Ϣ
	memcpy(device_version_params.softVersionInfo, SOFTWARE_VERSION_INFO, sizeof(SOFTWARE_VERSION_INFO));       //��32λ���������Ϣ
	result = sendto(socketFd, &device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
	if ( result == -1 )
	{
		DBG("sendto udp info error!!!\n");
		
	}
}


/*********************************************************************************
*
* 	udp�����̴߳��������
*
***********************************************************************************/

void udp_server_process()
{
	printf("udp server process started!\n");
	//����UDP������
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
	//ʹ��20000�˿�
    localAddr.sin_port = htons (20000);

	//���ö˿ڸ��� 
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
        // ��������  
        //fprintf(stderr,"udp_server_process   recving ...\n");
        result = recvfrom(socketFd, &udp_info, sizeof(struct UDP_INFO), 0, (struct sockaddr *)&fromAddr, &fromLen);
		//fprintf(stderr,"recv size = %d  type=0x%x  head 0x%x  data[0] 0x%x \n", result,udp_info.iType,udp_info.iHead,udp_info.iValue[0]);

		if(-1 == result)
		{
            DBG("############===>  Failed Error   %s\n",strerror(errno));
		}
        else
        {
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//�������������
			{
                DownloadSpecialParams(socketFd,fromAddr,&s_SpecialParams);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//�������������
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//�������
			{
                ClearFaultInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15b)//���ز��ֹ�����Ϣ
			{
                UploadFaultInfo(socketFd, fromAddr, startLineNo, lineNum);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)//������Щ������Ϣ
			{
                startLineNo = faultInfo->unExtraParamFirst;
				lineNum = faultInfo->unExtraParamTotal;
                SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)//���غ�Ƶ���
			{
				udp_cur_value_info.iHead = udp_info.iHead;
				udp_cur_value_info.iType = udp_info.iType;
				memcpy(udp_cur_value_info.redCurrentValue, g_RedCurrentValue, 32);
				result = sendto(socketFd, &udp_cur_value_info, sizeof(udp_cur_value_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)//���ص�������
			{
				current_params_udp_info.iHead = udp_info.iHead;
				current_params_udp_info.iType = udp_info.iType;
				memcpy(current_params_udp_info.struRecCurrent,g_struRecCurrent,sizeof(struct CURRENT_PARAMS)*32);
				result = sendto(socketFd, &current_params_udp_info, sizeof(current_params_udp_info), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//���ص�������
			{
				memcpy(g_struRecCurrent, udp_info.iValue, 32*8);
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//�ָ�Ĭ�ϲ���
			{
			    ResetAllParams(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
			{		
                DownloadIpAddress(socketFd,fromAddr);//����eth1��eth0��wlan0��IP��ַ,eth1��ӦIP-1,eth0��ӦIP-2,wlan0��ӦIP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{			
                UploadIpAddress(socketFd,fromAddr);//����eth1��eth0��wlan0��IP��ַ,eth1��ӦIP-1,eth0��ӦIP-2,wlan0��ӦIP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//������λ����
			{
				memcpy(&phase_desc_params,&udp_info,sizeof(phase_desc_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//������λ����
			{
				phase_desc_params.unExtraParamHead = udp_info.iHead;
				phase_desc_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &phase_desc_params, sizeof(phase_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//����ͨ������
			{
				memcpy(&channel_desc_params,&udp_info,sizeof(channel_desc_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//����ͨ������
			{
				channel_desc_params.unExtraParamHead = udp_info.iHead;
				channel_desc_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &channel_desc_params, sizeof(channel_desc_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//���ط�������
			{
				memcpy(&pattern_name_params,&udp_info,sizeof(pattern_name_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//���ط�������
			{
				pattern_name_params.unExtraParamHead = udp_info.iHead;
				pattern_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &pattern_name_params, sizeof(pattern_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//���ؼƻ�����
			{
				memcpy(&plan_name_params,&udp_info,sizeof(plan_name_params));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//���ؼƻ�����
			{
				plan_name_params.unExtraParamHead = udp_info.iHead;
				plan_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &plan_name_params, sizeof(plan_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//������������
			{
				memcpy(&date_name_params,&udp_info,sizeof(date_name_params));
				SendSuccessMsg(socketFd,fromAddr);
				iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//������������
			{
				date_name_params.unExtraParamHead = udp_info.iHead;
				date_name_params.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &date_name_params, sizeof(date_name_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)//������Ӳ���汾��Ϣ
			{
                UploadVersionInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)//���ص���ʱ������
			{
				memcpy(&g_struCountDown,udp_info.iValue,sizeof(g_struCountDown));
				SendSuccessMsg(socketFd,fromAddr);
				set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//���ص���ʱ������
			{
				memcpy(udp_info.iValue,&g_struCountDown,sizeof(g_struCountDown));
				result = sendto(socketFd, &udp_info, sizeof(g_struCountDown) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//���ش�������
			{
				memcpy(g_com_params,&udp_info,sizeof(g_com_params));
				SendSuccessMsg(socketFd,fromAddr);
				set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//���ش�������
			{
				com_params.unExtraParamHead = udp_info.iHead;
				com_params.unExtraParamID = udp_info.iType;
				com_params.unExtraParamValue = udp_info.iValue[0];
				result = sendto(socketFd, &com_params, sizeof(com_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType >= 0xaa && udp_info.iType <= 0xb6))//����������Ϣ
			{
				DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 ֻ��Ϊ��ctrl+f��������!
				//SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)//��ȡ����ʱ������Ϣ
			{
			    pthread_rwlock_rdlock(&gCountDownLock);
			    gCountDownParamsSend.ucChannelLockStatus = gChannelLockFlag;
			    gCountDownParamsSend.ucPlanNo = ((gSpecialControlSchemeId >= 251) ? gSpecialControlSchemeId : gCountDownParamsSend.ucPlanNo);
				result = sendto(socketFd, &gCountDownParamsSend, sizeof(gCountDownParamsSend), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
                pthread_rwlock_unlock(&gCountDownLock);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//�źŻ�����
			{
				system("sync");
				SendSuccessMsg(socketFd,fromAddr);
				ERR("System will restart in 1s .\n");
				sleep(1);
				system("reboot");
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb9)//����ͨ������
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xba)//�ر�ͨ������
			{
			    gChannelLockFlag = 0;
			    gCountDownParams->ucChannelLockStatus = gChannelLockFlag;
			    gCountDownParamsSend.ucChannelLockStatus = gChannelLockFlag;
			    SendSuccessMsg(socketFd,fromAddr);
			    set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//�ֶ������������
			{
			    gSpecialControlSchemeId = udp_info.iValue[0];
			    SendSuccessMsg(socketFd,fromAddr);
			    ERR("udp_server_process  control scheme id :  %d\n",gSpecialControlSchemeId);
			    set_custom_params();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//�źŻ���������
			{
			    if (IsStepValid((UInt8)stepCtrlParams->unStepNo, stepFlag))
				{
					stepFlag++;
					stepCtrlParams->unStepNo = 1;	//��ʾ�����ɹ�
				}
				else
					stepCtrlParams->unStepNo = 0;	//��ʾ����ʧ��
				result = sendto(socketFd, stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); 
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//�źŻ�ȡ������
			{
			    stepFlag = 0;
				StepCancel();
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//�������źŻ��ͺ�����
			{
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//��ȡ�������źŻ��ͺ�
			{
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//���ز�����ʼ
			{
			    StoreBegin((void *)udp_info.iValue);
                SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//�����������
			{
			    ERR("udp_server_process  recv completed . \n");
				StoreComplete();
				
                //��������������ļ�
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
* udp�̳߳�ʼ����
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
