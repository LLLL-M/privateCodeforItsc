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
#define _SVID_SOURCE	//����glibc2�汾,����stime()��Ҫ��������꣬
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

//MsgPhaseSchemeId gStructMsg1049;                            //��ƽ̨1049Э��ʹ��
pthread_rwlock_t gLockRealtimeVol = PTHREAD_RWLOCK_INITIALIZER;//����ʵʱ�����Ķ�д��
MsgRealtimeVolume gStructMsgRealtimeVolume;                 //ʵʱ������ֻ��������ʵʱ��

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //�����/home/config.dat�����е����нṹ�壬���������������ṹ��/����忪��/��־��ӡ����/�������к�/����������
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������
extern UInt8 gRedCurrentValue[32];
extern SignalControllerPara *gSignalControlpara;
extern CountDownCfg        g_CountDownCfg; 
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //���Ӳ���


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


extern int gComPort;//�����źŻ���ͨѶ�˿ںţ���Ϣ����ID���ֵ��ͬ��


/*****************************************************************************
 �� �� ��  : Set1049MsgContent
 ��������  : Ϊƽ̨1049Э��
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��11��16��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
__attribute__((unused)) static void Set1049MsgContent(MsgPhaseSchemeId *pStructMsg1049)
{
    int i = 0;
    int k = 0;
    int j = 0;

    pStructMsg1049->unExtraParamHead = 0x6e6e;
    pStructMsg1049->unExtraParamID = 0xd2;
    pStructMsg1049->unExtraDataLen = sizeof(pStructMsg1049->nPatternArray)+sizeof(pStructMsg1049->nPhaseArray);       
    
    //�źŻ����õ�������λ����λ��
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

    //�źŻ����õ����з����ţ�����ת��
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
        return;
    
    p_SpecialParams->iErrorDetectSwitch = GET_BIT(udp_info.iValue[0], 0);
    p_SpecialParams->iCurrentAlarmSwitch = GET_BIT(udp_info.iValue[0], 1);
    p_SpecialParams->iVoltageAlarmSwitch = GET_BIT(udp_info.iValue[0], 2);
    p_SpecialParams->iCurrentAlarmAndProcessSwitch = GET_BIT(udp_info.iValue[0], 3);
    p_SpecialParams->iVoltageAlarmAndProcessSwitch = GET_BIT(udp_info.iValue[0], 4);
    p_SpecialParams->iWatchdogSwitch = GET_BIT(udp_info.iValue[0], 5);
    p_SpecialParams->iGpsSwitch = GET_BIT(udp_info.iValue[0], 6);	
    p_SpecialParams->iPhaseTakeOverSwtich = GET_BIT(udp_info.iValue[0], 7);	

    //���¿��Ź���GPS����
    if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 0) && (p_SpecialParams->iWatchdogSwitch == 1))
    {
		//Ӳ�����Ź��ӹرյ���
		system("killall -9 watchdog &");
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 1;
    }
    else if((gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch == 1) && (p_SpecialParams->iWatchdogSwitch == 0))
    {	//Ӳ�����Ź��Ӵ򿪵��ر�
		gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 0;                
    }

	if((gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 0) && (p_SpecialParams->iGpsSwitch == 1))
	{
		//GPS�ӹرյ���
		system("killall -9 GPS &");
		system("/root/GPS &");
	}
	else if((gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 1) && (p_SpecialParams->iGpsSwitch == 0))
	{
		//GPS�Ӵ򿪵��ر�
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
	//����ip
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip;
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1)
	{
		ERR("set %s ip %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return FALSE;
	}
	INFO("set %s ip %s successful!", name, inet_ntoa(addr->sin_addr));
	//����netmask
	addr->sin_addr.s_addr = netmask;
	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1)
	{
		ERR("set %s netmask %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return FALSE;
	}
	INFO("set %s netmask %s successful!", name, inet_ntoa(addr->sin_addr));
	//��������
	addr->sin_addr.s_addr = gateway;	//��һ��ֻ��������ӡ��ûʲô������;
	memset(&rt, 0, sizeof(rt));
	memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
	rt.rt_dev = name;
	gateway_addr.sin_family = PF_INET;
	//inet_aton("0.0.0.0",&gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	ioctl(sockfd, SIOCDELRT, &rt); 	//��������֮ǰ��ɾ��֮ǰ��

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	gateway_addr.sin_addr.s_addr = gateway;
	memcpy(&rt.rt_gateway, &gateway_addr, sizeof(struct sockaddr_in));
	inet_aton("0.0.0.0", &gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	if(ioctl(sockfd, SIOCADDRT, &rt) == -1)		//��������µ�
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
	{	//���ҳ�Ҫ����������address����λ��/etc/network/interfaces��һ��
		fscanf(fp, "%d", &line);
		pclose(fp);
		if (line > 0)
		{	//ʹ��sed����������滻���滻��֮ǰ������
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "sed -i '%d,%dc address %s\\\nnetmask %s\\\ngateway %s' /etc/network/interfaces", 
					line, line + 2, ip_info.address, ip_info.subnetMask, ip_info.gateway);
			system(cmd);
		}
	}
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
	{	//��/etc/network/interfaces�ļ��л�ȡIP��Ϣ
		fscanf(fp, "address %s\nnetmask %s\ngateway %s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
		pclose(fp);
		INFO("ip=%s,netmask=%s,address=%s\n", ip_info.address, ip_info.subnetMask, ip_info.gateway);
		udp_info.iValue[0] = inet_addr(ip_info.address);
		udp_info.iValue[1] = inet_addr(ip_info.subnetMask);
		udp_info.iValue[2] = inet_addr(ip_info.gateway);
	}
    sendto(socketFd, &udp_info, 20, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
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
	DEVICE_VERSION_PARAMS device_version_params;           //�豸��Ӳ����Ϣ
	char *hv = HARDWARE_VERSION_INFO;
	char *sv = SOFTWARE_VERSION_INFO;

	memset(&device_version_params,0,sizeof(device_version_params));
	device_version_params.unExtraParamHead = udp_info.iHead;
	device_version_params.unExtraParamID = udp_info.iType;
	memcpy(device_version_params.hardVersionInfo, hv, strlen(hv));        //��32λ����Ӳ����Ϣ
	if (strcmp((char *)device_version_params.hardVersionInfo, "DS-TSC300") == 0)
	{
		if (gStructBinfileConfigPara.sSpecialParams.iSignalMachineType == 2)
			strcat((char *)device_version_params.hardVersionInfo, "-22");
		else
			strcat((char *)device_version_params.hardVersionInfo, "-44");
	}
	memcpy(device_version_params.softVersionInfo, sv, strlen(sv));       //��32λ���������Ϣ
	sendto(socketFd, &device_version_params, sizeof(device_version_params), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
}

#define UP_DOWN_LOAD_TIMEOUT	60		//�����س�ʱʱ��60s
static void UpAndDownLoadDeal(int socketFd, struct sockaddr_in fromAddr)	//�����ش���
{
	static UInt8 uploadNum = 0;
	static UInt32 downloadFlag = 0;//��ŵ��ǿ�ʼ����ʱSDK�������ľ�����Щ������Ҫ�����flag
	static struct timespec uploadStartTime = {0, 0}, downloadStartTime = {0, 0};
	struct timespec currentTime;
	struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)&udp_info;
	int ret = 12;

	if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb7)//���ز�����ʼ
	{
		log_debug("download config begin");
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
		{	//˵���������ͻ�����������
			udp_info.iValue[0] = 1;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		if (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)
		{	//˵���������ͻ�����������
			udp_info.iValue[0] = 2;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		downloadFlag = *(UInt32 *)(udp_info.iValue);
		downloadStartTime = currentTime;
		StoreBegin((void *)udp_info.iValue);
		udp_info.iValue[0] = 0;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//�����������
	{
		log_debug("download config over");
		if (downloadFlag > 0 && IsSignalControlparaLegal(gSignalControlpara) == 0)
		{	//��������Ƿ����
			ItsSetConfig(NTCIP, gSignalControlpara);
			if (BIT(downloadFlag, 16)) //��ʾ�Ƿ�дflash
				WriteLoaclConfigFile(NTCIP, gSignalControlpara);	//д�뱾�������ļ���
			log_debug("config information update!");
			ItsWriteFaultLog(LOCAL_CONFIG_UPDATE, 0);
		}
		else
			ItsGetConfig(NTCIP, gSignalControlpara);
		downloadFlag = 0;
		downloadStartTime.tv_sec = 0;
	}
	else if(udp_info.iHead == 0x6e6e && ((udp_info.iType >= 0xaa && udp_info.iType <= 0xb6) || udp_info.iType == 0xeeeeeeee))//����������Ϣ		0xeeeeeeee��ʾweb����
	{
		//log_debug("download config, type = %#x", udp_info.iType);
		DownloadConfig(udp_info.iType, (void *)udp_info.iValue);//0xaa 0xab 0xac 0xad 0xae 0xaf 0xb0 0xb1 0xb2 0xb3 0xb4 0xb5 0xb6 ֻ��Ϊ��ctrl+f��������!
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xce)	//���ؿ�ʼ
	{
		log_debug("upload config begin");
		clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (downloadFlag > 0 && (currentTime.tv_sec - downloadStartTime.tv_sec) <= UP_DOWN_LOAD_TIMEOUT)	//˵���������ͻ�����������
		{
			udp_info.iValue[0] = 1;
			sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			return;
		}
		//INFO("DEBUG: upload begin, downloadFlag = %#x, timegap = %d", downloadFlag, currentTime.tv_sec - downloadStartTime.tv_sec);
		//�������ͻ��������Ѿ���ʱ�������uploadNum=1,����ʱ�򶼼�1
		uploadNum = (uploadNum > 0 && (currentTime.tv_sec - uploadStartTime.tv_sec) > UP_DOWN_LOAD_TIMEOUT) ? 1 : (uploadNum + 1);
		uploadStartTime = currentTime;
		udp_info.iValue[0] = 0;
	}
	else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xcf)	//���ؽ���
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
	else if (udp_info.iHead == 0x6e6e && udp_info.iType == 0xc0)	//��������
	{
		ret = sizeof(struct STRU_Extra_Param_Response);
		if (response->unExtraParamValue == 0x15b)	//���ع�����Ϣ
		{
			log_debug("upload fault log, startline = %u, linenum = %u", response->unExtraParamFirst, response->unExtraParamTotal);
			ItsReadFaultLog(response->unExtraParamFirst, response->unExtraParamTotal, socketFd, (struct sockaddr *)&fromAddr);
			return;	//������Ϣ������ͳһ���ڹ�����־ģ��ظ�����������ֱ�ӷ��ز��ڴ˻ظ�
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
{	//���Ӽ��㷽ʽ��20,21,22����ͨ��1�ĺ죬�ƣ��̣�23,24,25����ͨ��2�ĺ죬�ƣ��̣��Դ�����
	if (nTerminal < 20 || nTerminal > MAX_TERMINAL_NUM)
		return;
	int channelId = (nTerminal - 20) / 3 + 1;
	int left = (nTerminal - 20) % 3;
	LightStatus status = (left == 0) ? RED : ((left == 1) ? YELLOW : GREEN);
	
	ItsChannelCheck(channelId, status);
}

//���͸�����������ͨ���ĵ���ʱֵ��״̬
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
	for (i = 0; i < 4; i++)	//�ܹ�4��״̬
	{
		gp->byChannelStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//ÿ�����8��ͨ����״̬��1bit����һ��ͨ��
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
	for (i = 0; i < 2; i++)	//�ܹ�2��״̬
	{
		gp->byPhaseStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//ÿ�����8����λ��״̬��1bit����һ����λ
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
	for (i = 0; i < 2; i++)	//�ܹ�2��״̬
	{
		gp->byOverlapStatusGroupNumber = i + 1;
		for (j = 0; j < 8; j++)	//ÿ�����8��������λ��״̬��1bit����һ��������λ
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

//����type���±����ļ�,Ŀǰ���������������ļ���ʵʱ���£������ļ���ʱ��������,TODO
static inline void UpdateLocalCfg(int socketFd, struct sockaddr_in fromAddr,int type)
{
    sleep(1);//����ȴ�һ��ʱ���ٽ��и��£���������ļ�û��׼����
    switch(type)
    {
        case 1://����hikconfig.dat
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
* 	udp�����̴߳��������
*
***********************************************************************************/

void *CommunicationModule(void *arg)
{
	//����UDP������
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
	int specialControlSchemeId = 0;       //������Ʒ�����,�������ؿ��Ʒ�����ʱʹ��
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
			if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x3eb4)//������
				sendto(socketFd, &udp_info, result, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x94)//�������������
			{
				log_debug("download special check parameters, value = %#x", udp_info.iValue[0]);
                DownloadSpecialParams(socketFd,fromAddr,&s_SpecialParams);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x93)//�������������
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x88)//�������
			{
				log_debug("clear fault log information");
                SendSuccessMsg(socketFd,fromAddr);
				ItsClearFaultLog();
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x97)//���غ�Ƶ���
			{
				//log_debug("upload red light current values");
				memcpy(udp_cur_value_info->redCurrentValue, gRedCurrentValue, sizeof(gRedCurrentValue));
				result = sendto(socketFd, &udp_info, sizeof(struct UDP_CUR_VALUE_INFO), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x95)//���ص�������
			{
				log_debug("upload red light current parameters");
				memcpy(current_params_udp_info->struRecCurrent,gStructBinfileConfigPara.sCurrentParams,sizeof(gStructBinfileConfigPara.sCurrentParams));
				result = sendto(socketFd, &udp_info, sizeof(struct CURRENT_PARAMS_UDP), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x96)//���ص�������
			{
				log_debug("download red light current parameters");
				memcpy(gStructBinfileConfigPara.sCurrentParams, udp_info.iValue, sizeof(gStructBinfileConfigPara.sCurrentParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveSpecialParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15c)//�ָ�Ĭ�ϲ���
			{
				log_debug("recover default parameters");
				system("rm -rf /home/*");
				SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15d || udp_info.iType == 0x15f || udp_info.iType == 0x161))
			{		
				log_debug("set ip address");
                DownloadIpAddress(socketFd,fromAddr);//����eth1��eth0��wlan0��IP��ַ,eth1��ӦIP-1,eth0��ӦIP-2,wlan0��ӦIP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && (udp_info.iType == 0x15e || udp_info.iType == 0x160 || udp_info.iType == 0x162))
			{
				log_debug("get ip address");
                UploadIpAddress(socketFd,fromAddr);//����eth1��eth0��wlan0��IP��ַ,eth1��ӦIP-1,eth0��ӦIP-2,wlan0��ӦIP-WiFi
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9a)//������λ����
			{
				log_debug("download phase describe");
				memcpy(&gStructBinfileDesc.sPhaseDescParams,&udp_info,sizeof(gStructBinfileDesc.sPhaseDescParams));
                SendSuccessMsg(socketFd,fromAddr);
				memcpy(gStructBinfileDesc.phaseDescText[0], &gStructBinfileDesc.sPhaseDescParams.stPhaseDesc, sizeof(gStructBinfileDesc.sPhaseDescParams.stPhaseDesc));
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9b)//������λ����
			{
				log_debug("upload phase describe");
				gStructBinfileDesc.sPhaseDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPhaseDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPhaseDescParams, sizeof(gStructBinfileDesc.sPhaseDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9c)//����ͨ������
			{
				log_debug("download channel describe");
				memcpy(&gStructBinfileDesc.sChannelDescParams,&udp_info,sizeof(gStructBinfileDesc.sChannelDescParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9d)//����ͨ������
			{
				log_debug("upload channel describe");
				gStructBinfileDesc.sChannelDescParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sChannelDescParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sChannelDescParams, sizeof(gStructBinfileDesc.sChannelDescParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa4)//���ط�������
			{
				log_debug("download scheme describe");
				memcpy(&gStructBinfileDesc.sPatternNameParams,&udp_info,sizeof(gStructBinfileDesc.sPatternNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa5)//���ط�������
			{
				log_debug("upload scheme describe");
				gStructBinfileDesc.sPatternNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPatternNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPatternNameParams, sizeof(gStructBinfileDesc.sPatternNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa6)//���ؼƻ�����
			{
				log_debug("download plan describe");
				memcpy(&gStructBinfileDesc.sPlanNameParams,&udp_info,sizeof(gStructBinfileDesc.sPlanNameParams));
                SendSuccessMsg(socketFd,fromAddr);
                iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa7)//���ؼƻ�����
			{
				log_debug("upload plan describe");
				gStructBinfileDesc.sPlanNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sPlanNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sPlanNameParams, sizeof(gStructBinfileDesc.sPlanNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa8)//������������
			{
				log_debug("download date describe");
				memcpy(&gStructBinfileDesc.sDateNameParams,&udp_info,sizeof(gStructBinfileDesc.sDateNameParams));
				SendSuccessMsg(socketFd,fromAddr);
				iIsSaveDescParams = 1;
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa9)//������������
			{
				log_debug("upload date describe");
				gStructBinfileDesc.sDateNameParams.unExtraParamHead = udp_info.iHead;
				gStructBinfileDesc.sDateNameParams.unExtraParamID = udp_info.iType;
				result = sendto(socketFd, &gStructBinfileDesc.sDateNameParams, sizeof(gStructBinfileDesc.sDateNameParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa1)//������Ӳ���汾��Ϣ
			{
                UploadVersionInfo(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9f)//���ص���ʱ������
			{
				log_debug("download countdown set information");
				memcpy(&gStructBinfileCustom.sCountdownParams,udp_info.iValue,sizeof(gStructBinfileCustom.sCountdownParams));
				SendSuccessMsg(socketFd,fromAddr);
				WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
				ItsYellowLampFlash(GET_BIT(gStructBinfileCustom.sCountdownParams.option, 1) ? TRUE : FALSE);//���ûƵ�ʱ��˸
				ItsSetRedFlashSec((UInt8)gStructBinfileCustom.sCountdownParams.redFlashSec);//���ú�Ƶ���ʱ��˸������
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa0)//���ص���ʱ������
			{
				log_debug("upload countdown set information");
				memcpy(udp_info.iValue,&gStructBinfileCustom.sCountdownParams,sizeof(gStructBinfileCustom.sCountdownParams));
				result = sendto(socketFd, &udp_info, sizeof(gStructBinfileCustom.sCountdownParams) + 8, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa3)//���ش�������
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xa2)//���ش�������
			{
				log_debug("upload serial config");
			    if((udp_info.iValue[0] > 0) && (udp_info.iValue[0] <= 4))
			    {
                    gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1].unExtraParamHead = udp_info.iHead;
                    gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1].unExtraParamID = udp_info.iType;
			    	result = sendto(socketFd, &gStructBinfileCustom.sComParams[udp_info.iValue[0] - 1], sizeof(COM_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			    }
			    else
			    {
                    result = -1;
			    }
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x9e)//��ȡ����ʱ������Ϣ
			{
				ItsCountDownGet(&countDownParams);
				if(gStructBinfileCustom.cChannelLockFlag != 1 || gStructBinfileCustom.sChannelLockedParams.ucReserved ==1)//reserved==1 ��ʾ�ɱ䳵���ƿ���
					countDownParams.ucChannelLockStatus = 0;
				if (countDownParams.ucPlanNo > 0 && countDownParams.ucPlanNo <= 16)
					memcpy(countDownParams.ucCurPlanDsc, gStructBinfileDesc.sPatternNameParams.stPatternNameDesc[countDownParams.ucPlanNo - 1], sizeof(countDownParams.ucCurPlanDsc));	//��ӷ�������
                else if(countDownParams.ucPlanNo > 16 && countDownParams.ucPlanNo < 249)
                    snprintf((char *)countDownParams.ucCurPlanDsc,sizeof(countDownParams.ucCurPlanDsc),"���� %d",countDownParams.ucPlanNo);

				result = sendto(socketFd, &countDownParams, 
				/*����˽���ڵ���ʱ�ṹ�����������һЩ���ݣ���ƽ̨�Ǳ߲���Ҫ�������Ҫ������Щ�������ݳ���*/
						 offsetof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS, ucChannelRealStatus),
						 MSG_DONTWAIT, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); //������Ϣͷ����Ϣ����
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6b)//�źŻ�����
			{
				log_debug("System will restart");
				ItsWriteFaultLog(UNNORMAL_OR_SOFTWARE_REBOOT, 0);
				SendSuccessMsg(socketFd,fromAddr);
				sync();
				sleep(1);
				system("reboot");
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_LOCK)//����ͨ������
			{
				log_debug("enable channel lock");
				memcpy(&gStructBinfileCustom.sChannelLockedParams,&udp_info,sizeof(gStructBinfileCustom.sChannelLockedParams));
				//ItsChannelLock(&gStructBinfileCustom.sChannelLockedParams);
			    gStructBinfileCustom.cChannelLockFlag = 1;
			    WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_CHAN_UNLOCK)//�ر�ͨ������
			{
				switch(udp_info.iValue[0])
				{
					case 0://ԭͨ������
						log_debug("disable channel lock");
						if(gStructBinfileCustom.cChannelLockFlag == 1)
						{
							gStructBinfileCustom.cChannelLockFlag = 0;
							//ItsChannelUnlock();
						}
						break;
					case 1://��ʱ��ͨ����������
						log_debug("Disable mult periods channel lock");
						gStructBinfileCustom.sMulPeriodsChanLockParams.cLockFlag = 0;
						break;
					case 2://��ʱ��ͨ�������ָ�
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd7)//���ý�������
			{
				log_debug("set demotion parameters");
				memcpy(&gStructBinfileCustom.demotionParams,&udp_info,sizeof(gStructBinfileCustom.demotionParams));
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd8)//��ȡ��������
			{
				log_debug("get demotion parameters");
				gStructBinfileCustom.demotionParams.unExtraParamID = 0xd8;
				result = sendto(socketFd, &gStructBinfileCustom.demotionParams, sizeof(gStructBinfileCustom.demotionParams), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbb)//�ֶ������������
			{
				log_debug("manual special control command, schemeid = %d", udp_info.iValue[0]);
				ItsCtl(TOOL_CONTROL, (UInt8)udp_info.iValue[0], 0);
				specialControlSchemeId = udp_info.iValue[0];	//���·�������������ʱʹ��
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6d)//��ȡ�������źŻ��ͺ�
			{
			    udp_info.iValue[0] = specialControlSchemeId;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbc)//�źŻ���������
			{
				log_debug("excute step control, stageNO = %d", stepCtrlParams->unStepNo);
				ItsCtl(TOOL_CONTROL, STEP_SCHEMEID, stepCtrlParams->unStepNo);
				stepCtrlParams->unStepNo = 1;	//��ʾ�����ɹ�
				result = sendto(socketFd, stepCtrlParams, sizeof(STEP_CTRL_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr)); 
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbd)//�źŻ�ȡ������
			{
				log_debug("cancel step control");
				ItsCtl(TOOL_CONTROL, 0, 0); //����ϵͳ������Ϣ
				cancelStepFeedback->unValue = 1;
				result = sendto(socketFd, cancelStepFeedback, sizeof(CANCEL_STEP_FEEDBACK_PARAMS), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbe)//���þ������źŻ��ͺ�
			{
			    gStructBinfileConfigPara.sSpecialParams.iSignalMachineType = udp_info.iValue[0];
			    iIsSaveSpecialParams = 1;
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xbf)//��ȡ�������źŻ��ͺ�
			{
			    udp_info.iValue[0] = gStructBinfileConfigPara.sSpecialParams.iSignalMachineType;
				result = sendto(socketFd, &udp_info, 12, 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x71)//���ض�ʱ
			{
				log_debug("set system time, ulGlobalTime = %lu, unTimeZone = %d", timep->ulGlobalTime, timep->unTimeZone);
				timep->ulGlobalTime += timep->unTimeZone;
				stime((time_t *)&timep->ulGlobalTime);
				system("hwclock -w");
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x72)//���ض�ʱ
			{
				log_debug("get system time");
				timep->ulGlobalTime = time(NULL) - 8 * 3600;
				timep->unTimeZone = 8 * 3600;
				result = sendto(socketFd, &udp_info, 8 + sizeof(*timep), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x6c)//ͨ�����
			{
				log_debug("excute channel check, terminal = %d", udp_info.iValue[0]);
				ChannelCheckDeal(udp_info.iValue[0]);
			    SendSuccessMsg(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x159)//����ͨ��״̬
			{
			    UploadChannelStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x157)//������λ״̬
			{
			    UploadPhaseStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x2b)//���ظ�����λ״̬
			{
			    UploadFollowPhaseStatus(socketFd,fromAddr);
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0x15a)//����ͬ��״̬
			{
			    UploadSyncStatus(socketFd,fromAddr);
			}	
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd0)//ʵʱ���з�����Ϣ
			{
				log_debug("get real time running scheme information");
				MsgRealtimePattern *p = (MsgRealtimePattern *)&udp_info;
				ItsGetRealtimePattern(p);
                result = sendto(socketFd, p, sizeof(MsgRealtimePattern), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd1)//ʵʱ����ͳ�ƻ�ȡ�ӿ�
			{
                pthread_rwlock_rdlock(&gLockRealtimeVol);
                gStructMsgRealtimeVolume.unExtraParamHead = 0x6e6e;
                gStructMsgRealtimeVolume.unExtraParamID = 0xd1;

/*    INFO("����: %d ��, ʱ��ռ����: %0.2f%%, ƽ������: %0.2f km/h, �Ŷӳ���: %0.2f m, �����ܶ�: %0.2f ��/km, \n\t\t\t��ͷ���: %0.2f m, ��ͷʱ��: %0.2f s, ����: %d s\n"
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
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xd2)//ƽ̨1049Э��
			{
                //�����Ƕ�1049Э��ķ�װ
                MsgPhaseSchemeId cStructMsg1049;
                Set1049MsgContent(&cStructMsg1049);
                result = sendto(socketFd, &cStructMsg1049, sizeof(cStructMsg1049), 0, (struct sockaddr*)&fromAddr, sizeof(fromAddr));
			}
			else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeec)//�������巢��ͨ������ʱ��Ϣ
			{
                UploadChannelCountdown(socketFd,fromAddr);
                //INFO("get countdown information");
            }
            else if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xeeeeeeed)//����ָ���������ļ�
			{
                UpdateLocalCfg(socketFd,fromAddr,udp_info.iValue[0]);
            }
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_SET)//��ʱ��ͨ������(������ԭͨ����������һ����Ϣ)
			{
				if(gStructBinfileCustom.cChannelLockFlag == 1)//ԭͨ���Ѵ�������״̬����Ч
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
			else if(udp_info.iHead == COM_MSG_HEAD && udp_info.iType == MSG_MP_CHAN_LOCK_GET)//��ʱ��ͨ��������ѯ
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
				UpAndDownLoadDeal(socketFd, fromAddr);	//�����ش���
				if(udp_info.iHead == 0x6e6e && udp_info.iType == 0xb8)//�����������
				{
#ifdef USE_GB_PROTOCOL
					NtcipConvertToGb();
#endif				
					//��������������ļ�
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

