/*hdr
**
**	
**
**	FILE NAME:	
**
**	AUTHOR:		
**
**	DATE:		
**
**	FILE DESCRIPTION:
**			
**			
**
**	FUNCTIONS:	
**			
**			
**			
**	NOTES:		
*/  

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "canmsg.h"
#include "io.h"
#include "hik.h"
#include "platform.h"
#include "HikConfig.h"
#include "specialControl.h"
#include "binfile.h"
#include "configureManagement.h"

int gOftenPrintFlag = 0;
typedef unsigned short  uint16;
static uint16 g_nLampStatus[8] = {0};
uint16 temp_nLampStatus[8] = {0}; //��������ʽ��������ʱ��
char temp_Sign[16] = {0};//��־λ
char lastRedLight[16] = {0}; //�����һ��״̬
char nowRedLight[16] = {0}; //�������״̬
char stayTimes[16] = {0};//�ӳٷ�������ʱ��
int HardwareWatchdoghd = 0;
//static int HardflashDogFlag = 0;
int is_WatchdogEnabled = 0;
//static int g_yellowflash = 0; //������־��1������0����
static int g_errorstat = 0; //���ϱ�־ :1 :�й��ϣ� 0:�޹���
//static int g_closelampflag = 0; //�����ص�
//static uint16 g_yellowflashfreq = 1; //����Ƶ��
static int g_faultstatus[32] = {0}; //ͨ�����ϼ�¼���ڻָ�
static int CurDetectFreq = 0;

//added by liujie 20160506
unsigned char finalChannelStatus[32] = {0};       //�����źŵ����״̬
static uint16 nLampStatus[4][8] = {0};         //1S4�ε��ֵ�洢






//��ʱ�޸ģ�����Խ��
extern CountDownVeh countdown_veh[17];
extern CountDownPed countdown_ped[17];
static int SetFreeGreenTimeSign = 0;//��Ӧʱ���־λ
unsigned char g_RedCurrentValue[32] = {0};   //32·��Ƶ���ֵ
time_t pedLastTime[4] = {0};//���˼�������ʱʱ��
int theFirstTimeSign = 0;//��һ�α�־λ
//static int pedOnState = 0;//���˽���״̬
struct FAILURE_INFO s_failure_info;       //������Ϣ�ṹ��

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //�����/home/config.dat�����е����нṹ�壬���������������ṹ��/����忪��/��־��ӡ����/�������к�/����������
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������pthread_t thread;//���ܵ�����ѹ��Ϣ��CAN���
extern CountDownCfg        g_CountDownCfg;              //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //���Ӳ���

extern pthread_rwlock_t gCountDownLock;
extern pthread_rwlock_t gLockRealtimeVol ;//����ʵʱ�����Ķ�д��
extern MsgRealtimeVolume gStructMsgRealtimeVolume;                 //ʵʱ������ֻ��������ʵʱ��


void ItsCustom(void);
void CountDownPulse(void);
void WirelessChanLock(void);

static void LockByChan(unsigned char flag, unsigned char *chanConfig, unsigned char *delayFlag,unsigned char *chanChangedFlag,unsigned char *cChanStatus,unsigned char *cArrayBlinkStatus,unsigned char *cArrayIsTimeEnd);
static int transitionOnChanLockStart(unsigned char *chan, unsigned char *chanConfig);
static int transitionOnChanLockEnd(unsigned char *chan);
static unsigned char transitionControl(unsigned char *chan, unsigned char *config);
static int trainsitionOnChanChange(unsigned char *cur, unsigned char *config);

extern UINT8 gFollowPhaseGreenBlinkFlag[NUM_PHASE];//������λ������־
extern UINT8 gFollowPhaseMotherPhase[NUM_PHASE];//������λ���ڸ����ĸ��λ
extern int gWirelessCtrlLockChanId;
int gLastWirelessCtrlId=0;

#define TIMEOUT  15
#define _YELLOWFLASH_


#define N 32
#define M 1<<(N-1)
void print_data(unsigned int c)    
{       
	int i;    
	for (i = 0; i < N; i++)	{       
	   if(i % 8 == 0) printf(" ");  
	   putchar(((c & M) == 0) ? '0':'1'); 
	   c <<= 1;    
	}       
	printf("\n");    
}
/*********************************************************************************
*
* 	�趨��Ӧ���ʱ�䡣
*
***********************************************************************************/
extern void SeFreeGreenTime(unsigned char time);
void Count_Down_pulse_All(uint16 *g_nLampStatus);
void Count_Down_pulse_Half(uint16 *g_nLampStatus);


extern int g_auto_pressed;    			  //0:�Զ�����û�а���   1:�Զ������Ѱ���
extern int g_manual_pressed;				  //0:�ֶ�����û�а���   1:�ֶ������Ѱ���
extern int g_flashing_pressed;			  //0:��������û�а���   1:���������Ѱ���
extern int g_allred_pressed;				  //0:ȫ�찴��û�а���   1:ȫ�찴���Ѱ���
extern int g_step_by_step_pressed;		  //0:��������û�а���   1:���������Ѱ���
extern int globalWatchdogFlagDisable;     //0:Enable  1:Disable.//WatchDog ʹ�ܿ���. (����)
extern int RTC_USE_GPS_ENABLE;			  //��0:Enable  0:Disable //GPSʹ�ܿ���.  (����)
extern PedDetectParams g_struPedDetect; //���˼�����

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //�����/home/config.dat�����е����нṹ�壬���������������ṹ��/����忪��/��־��ӡ����/�������к�/����������
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
extern STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������

/* add by Jicky */
extern SignalControllerPara *gSignalControlpara;    //ȫ��������Ϣ
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;       //����ʱ�ӿ���Ϣ

extern int msgid;
extern UInt8 gStepFlag;    //������־��0����ʾδ������1����ʾ����
extern UInt16 gLightArray[8];	//����ʱʹ�õĵ������
extern pthread_rwlock_t gLightArrayLock;    //��������������
extern unsigned short g_nLightStatus[8];

extern int RedSignalCheckInit(void);
extern void CollectCountDownParams();
extern void StepPthreadInit(void);

/* add over */


/*********************************************************************************
*
* 	ÿ�ο�������������ʱ���¼��ָ���ļ���
*
***********************************************************************************/
int WriteLogInfos(const char *pFile,const char *pchMsg)
{
	time_t now;  
	struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	
	if(pchMsg == NULL)
	{
		ERR("������¼��Ϣ��Ч!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("������¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("��ȡ������¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	
	if(f_stat.st_size > 100*1024)
	{
		fclose(pFileDebugInfo);
		//���ļ���������0������д
		pFileDebugInfo = fopen(pFile, "w+");
	}

	//��ȡ���ʱ�׼ʱ��
	time(&now);  
	  
	//ת��Ϊ����ʱ��
	timenow = localtime(&now); 

	fprintf(pFileDebugInfo,"%04d.%02d.%02d-%02d:%02d:%02d %s\n",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    
    fclose(pFileDebugInfo);
    return 0;
}

/*********************************************************************************
*
* 	д������־��
*
***********************************************************************************/
int WriteErrorInfos(const char *pFile,const char *pchMsg)
{
	time_t now;  
	struct tm *timenow; 
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;
	if(pchMsg == NULL)
	{
		ERR("���ϼ�¼��Ϣ��Ч!\n");
		return -1;
	}
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("���ϼ�¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("��ȡ���ϼ�¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			ERR("���ϼ�¼�ļ�������δ����ȷ��!\n");
			return -1;
		}
	}
	time(&now);  
	timenow = localtime(&now); 
	fprintf(pFileDebugInfo,"%04d.%02d.%02d-%02d:%02d:%02d %s\n",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,
		pchMsg);
    fclose(pFileDebugInfo);
    return 0;
}


/*********************************************************************************
*
* 	д�����ƹ�����־��
*
***********************************************************************************/
int WriteFailureInfos(const char *pFile,struct FAILURE_INFO s_failure_info)
{
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;

    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	ERR("���ϼ�¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		ERR("��ȡ���ϼ�¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			ERR("���ϼ�¼�ļ�������δ����ȷ��!\n");
			return -1;
		}
	}
	fwrite(&s_failure_info,sizeof(s_failure_info),1,pFileDebugInfo);
    fclose(pFileDebugInfo);

    if(gStructBinfileConfigPara.cFailureNumber >= 0xffff)
    {
        gStructBinfileConfigPara.cFailureNumber = 0;//����������кų�����65536�������㡣
    }

    WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
    return 0;
}

/*********************************************************************************
*
* 	д����״̬��־
*
***********************************************************************************/
void WriteFaultStatus()
{
	FILE *pFile = NULL;
	pFile = fopen("/home/FaultStatus.dat", "wb");
	if (pFile == NULL)
	{
		ERR("��FaultStatus.datʧ��,write\n");
		return;
	}
	fwrite(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}

/*********************************************************************************
*
* 	������״̬��־
*
***********************************************************************************/
void ReadFaultStatus()
{
	FILE *pFile = NULL;
	pFile = fopen("/home/FaultStatus.dat", "rb");
	if (pFile == NULL)
	{
		ERR("��FaultStatus.datʧ��,read\n");
		return;
	}
	fread(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}

/*********************************************************************************
*
*	��ȡ�������ͨ��״̬(1Sˢ��һ�Σ�������ʱ�ӿ�ʹ��)
*
***********************************************************************************/

void GetFinalChannelStatus()
{
	int i = 0;
	int j = 0;
	UInt16 tmp1LampStatus[8] = {0};     //������˸ͨ����Ϣ
	UInt16 tmp2LampStatus[8] = {0};		//��������ͨ����Ϣ
	UInt16 tmp3LampStatus[8] = {0};		//�������ͨ����Ϣ
	
	for(j = 0;j<8;j++)
	{
		tmp1LampStatus[j] = /*nLampStatus[0][j]^*/nLampStatus[1][j]^nLampStatus[2][j]/*^nLampStatus[3][j]*/;
		tmp2LampStatus[j] = nLampStatus[0][j]&nLampStatus[1][j]&nLampStatus[2][j]&nLampStatus[3][j];
		tmp3LampStatus[j] = ~(nLampStatus[0][j]|nLampStatus[1][j]|nLampStatus[2][j]|nLampStatus[3][j]);
		for(i = 0;i<4;i++)
		{
			//��˸ͨ����ֵ
			if(BIT(tmp1LampStatus[j],i*3+0) == 1)
			{
				finalChannelStatus[j*4+i] = GREEN_BLINK;
			}
			if(BIT(tmp1LampStatus[j],i*3+1) == 1)
			{
				finalChannelStatus[j*4+i] = RED_BLINK;
			}
			if(BIT(tmp1LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = YELLOW_BLINK;
			}	
			
			//����ͨ����ֵ
			if(BIT(tmp2LampStatus[j],i*3+0) == 1)
			{
				finalChannelStatus[j*4+i] = GREEN;
			}
			if(BIT(tmp2LampStatus[j],i*3+1) == 1)
			{
				finalChannelStatus[j*4+i] = RED;
			}
			if(BIT(tmp2LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = YELLOW;
			}
			//���ͨ����ֵ
			if(BIT(tmp3LampStatus[j],i*3+0) == 1 &&BIT(tmp3LampStatus[j],i*3+1) == 1&&BIT(tmp3LampStatus[j],i*3+2) == 1)
			{
				finalChannelStatus[j*4+i] = TURN_OFF;
			}
			
		}
			
	}
	
	//���Դ�ӡ
	
	/*INFO("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		finalChannelStatus[0],finalChannelStatus[1],finalChannelStatus[2],finalChannelStatus[3],
		finalChannelStatus[4],finalChannelStatus[5],finalChannelStatus[6],finalChannelStatus[7],
		finalChannelStatus[8],finalChannelStatus[9],finalChannelStatus[10],finalChannelStatus[11],
		finalChannelStatus[12],finalChannelStatus[13],finalChannelStatus[14],finalChannelStatus[15],
		finalChannelStatus[16],finalChannelStatus[17],finalChannelStatus[18],finalChannelStatus[19],
		finalChannelStatus[20],finalChannelStatus[21],finalChannelStatus[22],finalChannelStatus[23],
		finalChannelStatus[24],finalChannelStatus[25],finalChannelStatus[26],finalChannelStatus[27],
		finalChannelStatus[28],finalChannelStatus[29],finalChannelStatus[30],finalChannelStatus[31]);
	*/
	
}


/*********************************************************************************
*
* 	Ӳ�����Ź���ʼ����
*
***********************************************************************************/

void HardwareWatchdogInit()
{
	//����Ӳ�����Ź�ι��
	is_WatchdogEnabled = gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch;
	if(is_WatchdogEnabled == 1)
	{
		//ʹ��Ӳ�����Ź���ͨ���������ι��
		//�ر��ⲿι������
		system("killall watchdog");
		sleep(1);
		//��Ӳ�����Ź�
		HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
		if(HardwareWatchdoghd == -1) 
		{
			ERR("Open watchdog error,retry!!!\n");
			HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
			//return;
		}
		//����Ӳ�����Ź�
		ioctl(HardwareWatchdoghd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);	
		INFO("Watchdog is enabled!!!\n");
	}
	else
	{
		//�ⲿι�������������(�ⲿι�������Ѿ��ڿ�������ʱ�Զ�����)
		INFO("Watchdog is disabled!!!\n");
	}
}

/*********************************************************************************
*
* 	Ӳ�����Ź�ι����
*
***********************************************************************************/

unsigned int HardwareWatchdogKeepAlive(int boardNum)
{
	int dummy = 0;
	//Ӳ�����Ź�ʹ��ʱ����boardNumΪ8ʱι��
	if(boardNum == 8 && is_WatchdogEnabled == 1 )
	{
		//����ι������
		ioctl(HardwareWatchdoghd, WDIOC_KEEPALIVE, &dummy);
	}
	//����־
	if(1 == gStructBinfileConfigPara.cPrintfLogSwitch)
    {
        freopen("/dev/tty","w",stdout);//����־
    }
    else
    {
        freopen("/dev/tty","w",stderr);//�ر���־
    }
	return 1;
}

/*********************************************************************************
*
* 	GPS�Ƿ����ÿ��ƺ�����
*
***********************************************************************************/

void GPSInit()
{
	
	//GPS�����Ƿ��
	if(gStructBinfileConfigPara.sSpecialParams.iGpsSwitch == 0)
	{
		//GPS����û�д�
		INFO("GPS is disabled!!!\n");
		system("killall -9 GPS");
		Set_LED2_OFF();
	}
	else 
	{
		//GPS�����Ѿ���
		INFO("GPS is enabled!!!\n");
		system("killall -9 GPS");
		//Ϩ��GPSָʾ��
		Set_LED2_OFF();
		system("/root/GPS&");
	}
}

/*****************************************************************************
 �� �� ��  : RecordVehicleFlowData 
             �������������ͳ�����ݱ��浽�ļ��У��ļ�Ĭ�ϴ�С��10M��ѭ�����ǣ��ļ��ĵ�һ���ֽڴ�ŵ�������һ����¼���ļ��ײ���ƫ��
             �ṹ������
 �������  : TimeAndHistoryVolume *timeAndHistoryVolume  
             int size                                    
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int RecordVehicleFlowData(TimeAndHistoryVolume *timeAndHistoryVolume)
{
	FILE *fp = NULL;
	UINT64 offset = 0;//offset��¼�ˣ�������������µ�һ����¼���ļ�ͷ��TimeAndHistoryVolume����,��ֵ�洢���ļ���ͷ��4�ֽ�������
	
	fp = fopen(FILE_VEHICLE_DAT,"r+");//
	if(NULL == fp)//�������ļ������ڣ����½�֮
	{
	    if(2 == errno)//errno ���� 2�������ļ�������
	    {
            if((fp = fopen(FILE_VEHICLE_DAT,"a+")) == NULL)
            {
                INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
                return 0;
            }
	    }
	    else
	    {
    		INFO("open %s failed : %s , errno %d .\n",FILE_VEHICLE_DAT,strerror(errno),errno);
    		return 0;
	    }
	}

    if(fread(&offset,sizeof(UINT64),1,fp) < 1)//�Ȼ�ȡƫ����
    {
        offset = 1;
    }
    if(offset >= 10*1024*1024/sizeof(TimeAndHistoryVolume))//�ļ�Ĭ�ϴ�С��10M������10M���Ҫ��ͷ��ʼ����,offset��4��ʼ
    {
        offset = 1;
    }
    fseek(fp,0,SEEK_SET);
    offset += 1;
    fwrite(&offset,sizeof(UINT64),1,fp);//д��ƫ����

    fseek(fp,sizeof(UINT64)+(offset-1)*sizeof(TimeAndHistoryVolume),SEEK_SET);//����ȷ��λ��д��
	if(fwrite(timeAndHistoryVolume,sizeof(TimeAndHistoryVolume),1,fp) < 1)
	{
		INFO("read %s failed : %s .\n",FILE_VEHICLE_DAT,strerror(errno));
		fclose(fp);
		return 0;
	}
    fflush(fp);
	fclose(fp);
	return 1; 
}

/*****************************************************************************
 �� �� ��  : CalcStatData
 ��������  : ���ݳ�������������������糵��ռ���ʵ�ͳ������
����ռ���ʵĹ�ʽ���:
http://wenku.baidu.com/view/8c2e3951a417866fb84a8e22.html

���ٵļ��㹫ʽ��:
���⽻ͨ�����µĳ���--����ͨ��ģ��Ϊ: 
��Ƴ���Us/(km/h)	ͨ������C (������)/(Pcu/h)	 
120	2200	0.93	1.88	4.85
100	2200	0.95	1.88	4.86
80	2000	1.00	1.88	4.90
60	1800	1.20	1.88	4.88



�Ŷӳ��ȵļ��������:
�ó���û�з���Ȩ��ʱ����Ըó�����ƽ�����٣����Ǹó����ں��ʱ�Ĵ����Ŷӳ���

�����ܶ�k��ĳһ˲�䣬��λ·�γ����ڵĳ�����,��λ����/km�����ݸ���ϣ���Σ�Greenshields���ٶ�-�ܶ�ģ��ģ�ͣ����Եõ��ٶ����ܶȵĹ�ϵ��:
Q = KUf(1- K/Kj),����Qָ���ǳ��٣�K���ǳ����ܶȣ�Uf�����ɳ��٣�Kj�������ܶ�.���������ʽ�����Է��Ƶõ�������k�복��Q�Ĺ�ϵΪ:
K = (Kj-sqrt(Kj* Kj  - 4* Kj*Q/Uf))/2

ʱ��ռ����o: ��������ʱ���ܼ��ȣ���һ���Ĺ۲�ʱ���ڣ�����ͨ�������ʱ��ռ�õ�ʱ����۲���ʱ��ı�ֵ�����չ�ʽ��ʱ��ռ�����복���ȼ����
����֮�ͳ����ȼ�o = (l+d)*k.

��ͷʱ���뽻ͨ���Ĺ�ϵ  ht=3600/Q 
��ͷ����뽻ͨ���ܶȵĹ�ϵhs=1000/K

 �������  : TimeAndHistoryVolume *timeAndHistoryVolume  nCycle Ϊ�������ڳ�
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcStatData(TimeAndHistoryVolume *timeAndHistoryVolume,UINT32 nCycle)
{
    int i = 0;
    float tmp = 0;
    UINT8 nPhaseId = 0;
    UINT16 nWaitTime = 0;
    const UINT8 Kj = 120;//�����ܶ�--���������޷��ƶ�����������ͨ����ʱ�ĳ����ܶ�,��λ����/km.
    const UINT8 Uf = 72;//���ɳ����ٶȣ��������Ϊ��·����.

    //�����Ǽ���ƽ��������Ҫ�õ��Ĳ���
    const UINT8 Us = 60;//�ٶ��õ�·���ʱ����60km/h
    const float a1 = 1.2;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    const float a2 = 1.88;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    const float a3 = 4.88;//��1 ���ٹ�·����-����ͨ��ģ�Ͳ������е�����
    float b = 0;//�м�ֵ
    
    if(nCycle <= 0)
    {
        return;
    }   
    pthread_rwlock_rdlock(&gCountDownLock);
    for(i = 0; i < 48; i++)
    {
        if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)//�������������0����ôƽ�����پ���0�����ټ���
        {
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = 0;
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 0;
        }
        else
        {
            //����ƽ������
            tmp = 1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle;//�������������һСʱ�Ĺ�����
            b = a2 + a3*pow(1.0*tmp/1800,3);
            timeAndHistoryVolume->struVolume[i].byVehicleSpeed = (int)(100*a1*Us/(1+powf(tmp/1800,b)));//Ϊ��ȷ��λС�������Ǵ洢ʱ���ճ���100������ʵ��ʹ��ʱ��Ҫ����100.

            //���㳵���ܶȺ�ʱ��ռ����
            timeAndHistoryVolume->struVolume[i].wVehicleDensity = 100*Kj*(1 - timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100/Uf);

            //���㳵ͷ���
            if(timeAndHistoryVolume->struVolume[i].wVehicleDensity != 0)
            {
                timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance = 100*1000/(1.0*timeAndHistoryVolume->struVolume[i].wVehicleDensity/100);
            }

            //���㳵ͷʱ��
            timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance = 100*3600/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume*3600/nCycle);

            //�����Ŷӳ���,��λ�ĺ�Ƶȴ�ʱ����Գ��������õ��ĳ��������Ǹó����ĺ���Ŷӳ���
            nPhaseId = gSignalControlpara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase; //�ҵ���������Ӧ����λ
            nWaitTime = 0;
            if(nPhaseId >= 1 && nPhaseId <= 16)
            {
                nWaitTime = gCountDownParams->ucCurCycleTime - gCountDownParams->stPhaseRunningInfo[nPhaseId - 1][0];
            }
            if(timeAndHistoryVolume->struVolume[i].byDetectorVolume == 0)
            {
                timeAndHistoryVolume->struVolume[i].wQueueLengh = 0;
                continue;
            }
            timeAndHistoryVolume->struVolume[i].wQueueLengh = 100*nWaitTime/(1.0*timeAndHistoryVolume->struVolume[i].byDetectorVolume/nCycle);
            
        }
    }
    pthread_rwlock_unlock(&gCountDownLock);
    
    /*
    i = 0;
    INFO("����: %d ��, ʱ��ռ����: %0.2f%%, ƽ������: %0.2f km/h, �Ŷӳ���: %0.2f m, �����ܶ�: %0.2f ��/km, \n\t\t\t��ͷ���: %0.2f m, ��ͷʱ��: %0.2f s, ����: %d s\n"
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorVolume
                                                ,timeAndHistoryVolume->struVolume[i].byDetectorOccupancy/100.0
                                                ,timeAndHistoryVolume->struVolume[i].byVehicleSpeed/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wQueueLengh/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleDensity/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wVehicleHeadTimeDistance/100.0
                                                ,timeAndHistoryVolume->struVolume[i].wGreenLost/100);
                                              
    */
}



/*****************************************************************************
 �� �� ��  : VehicleFlowStat
 ��������  : ͳ�Ƴ�����������ȡ���ǵ�Ԫ�����еĲɼ����ڣ���λ�ǵ�Ԫ�����е�
             �ɼ���λ��ÿ������ͳ��һ�����ݣ������浽ָ�����ļ��У��ļ���С
             Ĭ����10M,�Ҳ��ɸ���,����10M���ѭ�����ǡ�
 �������  : boardNumֻ����1 2 3
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��9��24��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void VehicleFlowStat(UINT8 boardNum,UINT16 boardInfo)
{
    static UINT16 oldBoardInfo[3] = {0};  //������һ�εĹ������ݣ������ж��Ƿ��й����������bitλ��0���1����Ϊ�Ǹó�������һ�ι�����Ϣ
    static time_t startTime = 0;     //���μ�¼�Ŀ�ʼʱ�䣬Ҳ����Ҫ���浽�������ļ��е�������ʼʱ��
    static time_t oldTime = 0;//���������һ�μ��������ʱ��
    static UINT8 nFlag = 0;
    static struct timespec calOccupancyStartTime[48];//����ʱ��ռ������ʼʱ��
    static UINT8 nFlagIsCalcOccupancy[48] = {0};//�Ƿ�ʼ����ʱ��ռ���ʵı�־
    static struct timespec greenStartTime[48] ; //�̵ƿ�ʼʱ��
    static UINT32 nTimeOccupy[48] = {0};//��ǰ���������ڣ�����ռ�ó�������ʱ�䣬��λ��ms.
    struct timespec currentTime;//��ǰʱ��
    UINT64 nTempLong = 0;

    UINT16 nCycleTime = gSignalControlpara->stUnitPara.byFluxCollectCycle * ((gSignalControlpara->stUnitPara.byCollectCycleUnit == 0) ? 1 : 60);//�ɼ����ڣ���λ����
    UINT8 nIsHaveCar = 0;//�Ƿ��й�����1�����й�����0�����޹���
    UINT8 nPhaseId = 0;//�������Ӧ����λ
    
    int i = 0;
    time_t nowTime = time(NULL);            

    if(startTime == 0)
    {
        startTime = nowTime;
    }
    pthread_rwlock_wrlock(&gLockRealtimeVol);
    nCycleTime = (nCycleTime == 0 ? 5 : nCycleTime);

    //���ж���û�й�������
    for(i = 0; i < 16; i++)
    {
        nIsHaveCar = 0;
        if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))//����ǰһ������λΪ0����һ������λΪ1����Ϊ�г�������
        {
            gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorVolume++;
            nIsHaveCar = 1;
        }

        nPhaseId = gSignalControlpara->AscVehicleDetectorTable[(boardNum - 1)*16 + i].byVehicleDetectorCallPhase;
        //�����ǰ�������Ӧ����λ���̵ƣ����й������ͼ�������ʱ��
        if((nPhaseId >=1) && (nPhaseId < 16)&&(gCountDownParams->stVehPhaseCountingDown[nPhaseId - 1][0] == 1))
        {
            if(nFlag == 0)
            {
                nFlag = 1;
                oldTime = nowTime;
            }
            if(nIsHaveCar == 0)//�����λ���̵ƣ���û�й�������ô����ʱ���Ҫ��1��
            {
                if(nowTime - oldTime >= 1)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].wGreenLost += 1*100;
                    nFlag = 0;
                }
            }
            else
            {
                oldTime += 1;
            }

            //��λ�̵�,��һ��������0��Ȼ����1�������г���ʼͨ������������ʼͳ��ʱ��ռ����
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 0) && (((boardInfo >> i) & 0x01) == 1))
            {
                clock_gettime(CLOCK_MONOTONIC, &calOccupancyStartTime[(boardNum - 1)*16 + i]);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 0)
                {
                    nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 1;
                    greenStartTime[(boardNum - 1)*16 + i] = calOccupancyStartTime[(boardNum - 1)*16 + i];
                }
            }            
            if((((oldBoardInfo[boardNum - 1] >> i) & 0x01) == 1) && (((boardInfo >> i) & 0x01) == 0))//��һ��������1��Ȼ����0���������������뿪������������Ҫ��¼��ʱ��            {
            {
                clock_gettime(CLOCK_MONOTONIC, &currentTime);

                if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
                    nTimeOccupy[(boardNum - 1)*16 + i] += (currentTime.tv_sec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - calOccupancyStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;
                //if(i == 0)
                 //   INFO("%d  %p\n",nTimeOccupy[0],&nTimeOccupy[0]);
            }        
        }
        else//���ʱ�������Ƿ��Ѿ�ͳ�ƹ���ʼʱ��������ʱ��ռ����
        {
            if(nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] == 1)
            {
                nFlagIsCalcOccupancy[(boardNum - 1)*16 + i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                //nTempLong��ʵ����ľ��Ǹó�������Ӧ����λ�̵�ʱ��
                nTempLong = (currentTime.tv_sec - greenStartTime[(boardNum - 1)*16 + i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[(boardNum - 1)*16 + i].tv_nsec)/1000000;

                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[(boardNum - 1)*16 + i].byDetectorOccupancy = 100.0*nTimeOccupy[(boardNum - 1)*16 + i]/nTempLong;
                }
                memset(nTimeOccupy,0,sizeof(nTimeOccupy));
            }
        }
    }

    gStructMsgRealtimeVolume.volumeOccupancy.dwTime = startTime;
    CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
    
    //�ж������Ƿ��ѵ��ÿ�����ڼ�¼һ�����ݣ�ÿ��ͳ��һ������
    if(nowTime - startTime >= nCycleTime)
    {
        for(i = 0; i < 48; i++)
        {
            if(nFlagIsCalcOccupancy[i] == 1)
            {
                nFlagIsCalcOccupancy[i] = 0;
                
                clock_gettime(CLOCK_MONOTONIC, &currentTime);
                nTempLong = (currentTime.tv_sec - greenStartTime[i].tv_sec)*1000+(currentTime.tv_nsec - greenStartTime[i].tv_nsec)/1000000;
                if(nTempLong != 0)
                {
                    gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy = 10000.0*nTimeOccupy[i]/(1.0*nTempLong);
                
                   /* if(i == 0)
                    {
                        INFO("===>   %0.2f%% \n",gStructMsgRealtimeVolume.volumeOccupancy.struVolume[i].byDetectorOccupancy/100.0);
                    }*/
                }
                nTimeOccupy[i] = 0;
            }
        }
    
       // CalcStatData(&gStructMsgRealtimeVolume.volumeOccupancy,nowTime - startTime);
        RecordVehicleFlowData(&gStructMsgRealtimeVolume.volumeOccupancy);

        startTime = nowTime;
        nFlag = 0;
        memset(&gStructMsgRealtimeVolume,0,sizeof(gStructMsgRealtimeVolume));
    }
    oldBoardInfo[boardNum - 1] = boardInfo;

    pthread_rwlock_unlock(&gLockRealtimeVol);
}


/*********************************************************************************
*
* 	��������źš�
*
***********************************************************************************/

uint16 VehicleCheckInput(int boardNum)
{
	uint16 boardInfo = 0;
	//int iPedNO = 0;//������
	
	if (boardNum<1 || boardNum>3)
	{
		INFO("boardNum error:%d\n", boardNum);
		return -1;
	}
	boardInfo = recv_date_from_vechile(boardNum);

	VehicleFlowStat(boardNum,boardInfo);
	DBG("INFO 100140:%d,boardInfo:%d\n",boardNum,boardInfo); 

	return boardInfo;
}

/*********************************************************************************
*
* 	���ſ����źš�
*
***********************************************************************************/

uint16 DoorCheckInput(void)
{
	uint16 input = 0;

	input = DoorCheck();
	DBG("INFO 100142 �ſ����ź�: 0x%x\n",input); 


	return input;
}

/*********************************************************************************
*
* 	���̳�ͻ���̳�ͻ��⡣
*
***********************************************************************************/
void RedgreenCollision(int boardNum, uint16 boardInfo) 
{
	//uint16 i = 0;
	uint16 j = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	static uint16 errorcountgreen[32] = {0};
	static uint16 normalcountgreen[32] = {0};
	time_t now;  
	struct tm *timenow;
	
	char msg[128] = "";

	if(boardNum > 4)
	{
		//ֻ����16ͨ��
		return ;
	}

	//INFO("**************************boardno:%d,g_nLampStatus:%x,boardinfo:%x\n", boardNum, g_nLampStatus[boardNum-1], boardInfo);
	for (j=0; j<4; j++)
	{
		//���̳�ͻ���
		if ((((g_nLampStatus[boardNum-1]>>(j*3+1))&0x1) == 0)  && (((boardInfo>>(j*3+1))&0x1) == 1)) 
		{
			//��Ʋ�����������Ϊ���̳�ͻ
			//��ȡ���ʱ�׼ʱ��
			time(&now);   
			//ת��Ϊ����ʱ��
			timenow = localtime(&now); 

			sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
				timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
				timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
			//INFO("%s ��%dͨ����Ƶ�ѹ�쳣(���̳�ͻ����)��errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
			
			if (++errorcountred[(boardNum-1)*4+j] > TIMEOUT)
			{
				errorcountred[(boardNum-1)*4+j] = 0;
				normalcountred[(boardNum-1)*4+j] = 0;
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				if(g_faultstatus[(boardNum-1)*4+j] == 0)
				{
					gStructBinfileConfigPara.cFailureNumber ++;
					ERR("%s ��%dͨ�����̳�ͻ,errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
					sprintf(msg, "��%dͨ�����̳�ͻ", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = RED_GREEN_CONFLICT;          //��Ϣ����ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
					s_failure_info.nTime = now;
	  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					
					g_faultstatus[(boardNum-1)*4+j] = 2;
					//��g_faultstatus����д���ļ�
					WriteFaultStatus();
				}
			}		
		}
		else if ((((g_nLampStatus[boardNum-1]>>(j*3+1))&0x1) == 0)  && (((boardInfo>>(j*3+1))&0x1) == 0)) 
		{
			//����ĺ�ƶ�Ϩ���ˣ�����
			if(++normalcountred[(boardNum-1)*4+j] > TIMEOUT)
			{	
				normalcountred[(boardNum-1)*4+j] = 0;
				errorcountred[(boardNum-1)*4+j] = 0;
				if(g_faultstatus[(boardNum-1)*4+j] == 2)
				{
					//��ȡ���ʱ�׼ʱ��
					time(&now); 
					gStructBinfileConfigPara.cFailureNumber++;
					sprintf(msg, "��%dͨ�����̳�ͻ��������", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = RED_GREEN_CONFLICT_CLEAR;                      //���̳�ͻ�����Ϣ����ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}

		//�̳�ͻ���
		if ((((g_nLampStatus[boardNum-1]>>(j*3))&0x1) == 0) && (((boardInfo>>(j*3))&0x1) == 1)) //�̵Ʋ�����������Ϊ�̳�ͻ
		{
			//��ȡ���ʱ�׼ʱ��
			time(&now);  
			  
			//ת��Ϊ����ʱ��
			timenow = localtime(&now); 

			sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
				timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
				timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
			//INFO("%s ��%dͨ���̵Ƶ�ѹ�쳣��(�̳�ͻ����),errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
			if (++errorcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{		
				errorcountgreen[(boardNum-1)*4+j] = 0;
				normalcountgreen[(boardNum-1)*4+j] = 0;
				
				if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
				}
				gStructBinfileConfigPara.cFailureNumber++;
				ERR("%s ��%dͨ���̳�ͻ,errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
				sprintf(msg, "��%dͨ���̳�ͻ", (boardNum-1)*4+j+1);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				s_failure_info.nID = GREEN_CONFLICT;                      //�̳�ͻ��Ϣ����ID
				s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
				s_failure_info.nTime = now;
  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
				WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				g_faultstatus[(boardNum-1)*4+j] = 1;
				//��g_faultstatus����д���ļ�
				WriteFaultStatus();				
			}		
		}
		else if((((g_nLampStatus[boardNum-1]>>(j*3))&0x1) == 0) && (((boardInfo>>(j*3))&0x1) == 0))
		{
			//������̵ƶ����ˣ�����
			if(++normalcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{	
				normalcountgreen[(boardNum-1)*4+j] = 0;
				errorcountgreen[(boardNum-1)*4+j] = 0;
				if(g_faultstatus[(boardNum-1)*4+j] == 1)
				{
					//��ȡ���ʱ�׼ʱ��
					time(&now); 
					gStructBinfileConfigPara.cFailureNumber++;
					sprintf(msg, "��%dͨ���̳�ͻ��������", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = GREEN_CONFLICT_CLEAR;                      //���̳�ͻ�����Ϣ����ID
					s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}
	}

}

/*********************************************************************************
*
* ����λ���ѹ�źš�
*
***********************************************************************************/

uint16 PhaseLampVoltInput(int boardNum)
{
	if (boardNum<1 || boardNum>8)
	{
		ERR("boardNum error:%d\n", boardNum);
		return 0;
	}
	char msg[128] = "";
	time_t now;  
	struct tm *timenow;
	uint16 boardInfo = 0;
	if (g_errorstat == 1)
	{
		return ~boardInfo;
	}
	i_can_its_get_Volt(boardNum, &boardInfo);
	//��ȡ���ʱ�׼ʱ��
	time(&now);  
	  
	//ת��Ϊ����ʱ��
	timenow = localtime(&now); 

	sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
	DBG("%s  INFO 100131: boardNum:%d, boardInfodes:%d, boardInfosrc:%d\n",msg, boardNum, boardInfo, g_nLampStatus[boardNum-1]&0xfff); 
	
	//���̳�ͻ���
	RedgreenCollision(boardNum, boardInfo);
	return ~boardInfo;
}

#define Time_To_Minutes(hour,minute,second) ((hour)*3600+(minute)*60+second)

/*****************************************************************************
 �� �� ��  : IsDateInControlArea
 ��������  : �жϵ�ǰʱ���Ƿ���ָ��ʱ�����
 �������  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static unsigned char IsDateInControlArea(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);

    unsigned int iBeginTime = 0;
    unsigned int iEndTime = 0;
    unsigned int iNow = 0;

    iBeginTime = Time_To_Minutes(pChannnelLockedParams->ucBeginTimeHour,pChannnelLockedParams->ucBeginTimeMin,pChannnelLockedParams->ucBeginTimeSec);
    iEndTime = Time_To_Minutes(pChannnelLockedParams->ucEndTimeHour,pChannnelLockedParams->ucEndTimeMin,pChannnelLockedParams->ucEndTimeSec);
    iNow = Time_To_Minutes(now.tm_hour,now.tm_min,now.tm_sec);

    //normal 
    if(iBeginTime < iEndTime)
    {
        if((iNow > iBeginTime) && (iNow < iEndTime))
        {
            return 1;
        }
    }
    else//����ʼʱ��Ƚ���ʱ���ʱ
    {
        if((iNow > iEndTime ) && (iNow < iBeginTime))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/*****************************************************************************
 �� �� ��  : IsStartLockChannel
 ��������  : �ж��Ƿ�ʼ����ͨ�������û���յ����������򷵻�0�����յ�������
             ���ǰʱ����ڲ��������򷵻ش����������򷵻�����
 �������  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
unsigned char IsStartLockChannel(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
    if(gStructBinfileCustom.cChannelLockFlag == 0)//���û��ͨ�����������ֱ�ӷ��ء�
    {
        return 0;//������
    }

    //��������˹������򵫵�ǰʱ�䲢���ڹ��������ڣ���ֱ�ӷ���2��
    if((pChannnelLockedParams->ucWorkingTimeFlag == 1) && (IsDateInControlArea(pChannnelLockedParams) == 0))
    {
        gStructBinfileCustom.cChannelLockFlag = 2;
        return 2;//������
    }

    gStructBinfileCustom.cChannelLockFlag = 1;//����
    return 1;
}

/*****************************************************************************
 �� �� ��  : LockChannel
 ��������  : �����Ƿ�����ͨ���ı�ͨ��״̬
 �������  : CHANNEL_LOCK_PARAMS *pChannnelLockedParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void LockChannel(CHANNEL_LOCK_PARAMS *pChannnelLockedParams)
{
	unsigned char lock=0;
	static unsigned char delayFlag=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//����������������ʱ��һ�ε�ɫ��״̬������������ʱ�Ƿ�Ӧ�����    
	static unsigned char cArrayIsTimeEnd[32] = {0};//����ʱ��Ҫ��֤״̬����ʱ���500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};
	
	if(pChannnelLockedParams == NULL)//���û��ͨ�����������ֱ�ӷ��ء�    
	{
		return;
	}
	if(IsStartLockChannel(pChannnelLockedParams) != 1)
		lock=0;
	else
		lock=1;
	LockByChan(lock, pChannnelLockedParams->ucChannelStatus, &delayFlag, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
}
static UINT8 IsDateInControlAreas(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	int i=0;
	Boolean timeFlag = FALSE;
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);

    unsigned int iBeginTime = 0;
    unsigned int iEndTime = 0;
    unsigned int iNow = 0;

    iNow = Time_To_Minutes(now.tm_hour,now.tm_min,now.tm_sec);

	for(i=0; i<16; i++)
	{
		if(pNewChannnelLockedParams->stChannelLockPeriods[i].ucWorkingTimeFlag != 1)
			continue;
		iBeginTime = Time_To_Minutes(pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeHour, pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeMin,pNewChannnelLockedParams->stChannelLockPeriods[i].ucBeginTimeSec);
		iEndTime = Time_To_Minutes(pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeHour,pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeMin,pNewChannnelLockedParams->stChannelLockPeriods[i].ucEndTimeSec);
		if(iBeginTime <= iEndTime)
		{
			if(iNow >=iBeginTime && iNow < iEndTime)
				return i+16;	//����,����ʱ�κ�+16
		}
		else
		{
			if(iNow >= iBeginTime || iNow < iEndTime)
				return i+16;	//����
		}
		if(timeFlag == FALSE)
			timeFlag = TRUE;
	}
	if(timeFlag == TRUE)
		return 2;		//��ǰʱ�䲻��ʱ���ڣ�������
	return 0;//δ����
}

static UINT8 IsStartPeriodLockChannel(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	UINT8 ret=0;
	if(gStructBinfileCustom.sNewChannelLockedParams.uChannelLockFlag == 0 || gStructBinfileCustom.cChannelLockFlag == 1)//ԭͨ����������
		return 0;
	ret = IsDateInControlAreas(pNewChannnelLockedParams);
	if(ret >= 16)
		gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = 1;
	else
		gStructBinfileCustom.sNewChannelLockedParams.uChannelLockStatus = ret;
	
	return ret;
}
void PeriodLockChannel(STRU_Channel_Lock_V2_INFO *pNewChannnelLockedParams)
{
	unsigned char lock=0;
	UINT8 iPeriodNum = 0;
	unsigned char *pchan=NULL;
	static unsigned char delayFlag=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//����������������ʱ��һ�ε�ɫ��״̬������������ʱ�Ƿ�Ӧ�����    
	static unsigned char cArrayIsTimeEnd[32] = {0};//����ʱ��Ҫ��֤״̬����ʱ���500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};

	//����ԭͨ������ʱ����ͨ��������������
	if(pNewChannnelLockedParams == NULL)
	{
		return;
	}
	if((iPeriodNum = IsStartPeriodLockChannel(pNewChannnelLockedParams)) <16)
	{
		lock = 0;
	}
	else
	{
		iPeriodNum -=  16;//ת��Ϊ����,��Ϊʱ�κ�
		if(iPeriodNum<0 || iPeriodNum >= 16)
		{
			ERR("Wrong period number: %d", iPeriodNum);
			return;
		}
		lock = 1;
		pchan = pNewChannnelLockedParams->stChannelLockPeriods[iPeriodNum].ucChannelStatus;
	}
	

	LockByChan(lock, pchan, &delayFlag, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
	return;
}


/*********************************************************************************
*
*  д�ƿذ��źŵơ�
*
***********************************************************************************/
int PhaseLampOutput(int boardNum, uint16 outLamp)
{
	struct msgbuf msg;
	static int nCount = 0;
	//char *temp_LampStatus = NULL;
	DBG("INFO 100130:%d:0x%x|",boardNum, outLamp); 
#ifdef DEBUG
	print_data(outLamp);
#endif
	
	if (boardNum<1 || boardNum>8)
	{
		ERR("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
	g_nLightStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
		nCount ++;
		ItsCustom();
		CountDownPulse();
		PeriodLockChannel(&gStructBinfileCustom.sNewChannelLockedParams);
		WirelessChanLock();
		LockChannel(&gStructBinfileCustom.sChannelLockedParams);
		msg.mtype = MSG_RED_SIGNAL_CHECK;
		if (gStepFlag == 0)
		{
			memcpy(msg.lightArray, g_nLampStatus, sizeof(msg.lightArray));
			i_can_its_send_led_request(boardNum, g_nLampStatus);
		}
		else
		{
		    pthread_rwlock_rdlock(&gLightArrayLock);
			memcpy(msg.lightArray, gLightArray, sizeof(msg.lightArray));
            i_can_its_send_led_request(boardNum, gLightArray);
		    pthread_rwlock_unlock(&gLightArrayLock);
		}
		if (gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch == 1)
			msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
			
		//��1s4�εĵ��ֵ�浽nLampStatus��ά������
		memcpy(nLampStatus[nCount-1],msg.lightArray,sizeof(msg.lightArray));
		if(nCount == 4)
		{
			//��ȡ�������ͨ��״̬(1Sһ�Σ�������ʱ�ӿ����ʹ��)
			GetFinalChannelStatus();
			nCount = 0;
		}
		
		//���ذ�����ָʾ��
		Hiktsc_Running_Status();
		//Ӳ�����Ź�ι������
		HardwareWatchdogKeepAlive(boardNum);
	}

	//��ѹ���
	if(gStructBinfileConfigPara.sSpecialParams.iVoltageAlarmSwitch == 1)
	{
		PhaseLampVoltInput(boardNum);
	}
	
	
	return 0;
}


/*********************************************************************************
*
* 	���Ϩ���⡣
*
***********************************************************************************/
void RedExtinguish() //��Ƹ�����������Ϊ���Ϩ��
{
	uint16 i = 0;
	uint16 j = 0;
	uint16 pcurInfo = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	char msg[128] = "";
	time_t now; 
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //�ڣ�������д����,ֻ��ȡ��Ƶ���
			g_RedCurrentValue[i*4+j] = pcurInfo;

			//if(pcurInfo > 0)
			//{
			//	INFO("ͨ��%02d��Ƶ���ֵ:%03d\n",i*4+j+1,pcurInfo);
			//}
			if ((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo < (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff) )
			{	
				//INFO("ͨ��%02d��������Ҽ������쳣,errorcountred[%d]=%d pcurInfo=%d\n",i*4+j+1,i*4+j+1,errorcountred[i*4+j]+1,pcurInfo);
				//��������ҵ�������쳣
				if (++errorcountred[i*4+j] > 5)
				{
					errorcountred[i*4+j] = 0;
					normalcountred[i*4+j] = 0;
					if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmAndProcessSwitch == 1)
					{
						//g_yellowflash = 1;
						//���Ϊ�й���
						g_errorstat = 1;
						SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_YELLOW_BLINK);
					}
					if(g_faultstatus[i*4+j] == 0)
					{
						//��ȡ���ʱ�׼ʱ��
						time(&now); 
						
						
						gStructBinfileConfigPara.cFailureNumber ++;
						ERR("��%dͨ�����Ϩ��\n", i*4+j+1);
						sprintf(msg, "��%dͨ�����Ϩ��", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						s_failure_info.nID = RED_LIGHT_OFF;                      //���Ϩ����Ϣ����ID
						s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
						s_failure_info.nTime = now;
	  					s_failure_info.nValue = i*4+j+1;     //ͨ����   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_faultstatus[i*4+j] = 3;
						//��g_faultstatus����д���ļ�
						WriteFaultStatus();
						//return;
					}
				}
				//return;
			}
			else if((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo > (gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentBase - gStructBinfileConfigPara.sCurrentParams[i*4+j].RedCurrentDiff))
			{
				
				//��ͨ��������������ҵ���ֵ����
				if (++normalcountred[i*4+j] > 5)
				{
					normalcountred[i*4+j] = 0;
					errorcountred[i*4+j] = 0;
					//��ͨ�����д�����������
					if (g_faultstatus[i*4+j] == 3)
					{
						time(&now); 
						sprintf(msg, "��%dͨ�����Ϩ���������", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						gStructBinfileConfigPara.cFailureNumber ++;
						s_failure_info.nID = RED_LIGHT_OFF_CLEAR;                      //���Ϩ��������Ϣ����ID
						s_failure_info.nNumber = gStructBinfileConfigPara.cFailureNumber;       //���к�
						s_failure_info.nTime = now;
			  			s_failure_info.nValue = i*4+j+1;     //ͨ����   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
									s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_faultstatus[i*4+j] = 0;
						WriteFaultStatus();
					}
				}
			}
		}
	}
}

/*********************************************************************************
*
*	дӲ�����źš�
*
***********************************************************************************/

void HardflashDogOutput(void)
{
#if 0	//changed by Jicky
	uint16 pcurInfo = 0;
	if (HardflashDogFlag == 1)
	{
		HardflashDogFlag = 0;
	}
	else
	{
		HardflashDogFlag = 1;
	}
	
	HardflashDogCtrl(HardflashDogFlag);

	printf("INFO 100150 Ӳ�����ź�:%d\n", HardflashDogFlag); 
#endif
	CurDetectFreq++;
	if( CurDetectFreq > 2 )
	{
		//�������
		if(gStructBinfileConfigPara.sSpecialParams.iCurrentAlarmSwitch == 1
			&& gStructBinfileConfigPara.sSpecialParams.iErrorDetectSwitch == 1)
		{
			RedExtinguish();
		}
		CurDetectFreq = 0;
	}
	return;
}

unsigned char VehicleCheckFaultInput(unsigned char phase)
{
	return  0; 
}


/*********************************************************************************
*
* 	����λ�����ֵ��(����)
*
***********************************************************************************/

unsigned int PhaseLampCurInput(int boardNum, int pahseNum, int redGreen)
{
	if (boardNum<1 || boardNum>8 || pahseNum<1 || pahseNum>4)
	{
		ERR("boardNum or pahseNum error:%d, %d\n", boardNum, pahseNum);
		return 0;
	}
	uint16 curInfo = 0;
	if (g_errorstat == 1)
	{
		return curInfo;
	}
	DBG("INFO 100132:%d  %d   %d\n",boardNum, pahseNum, redGreen); 
	curInfo = i_can_its_get_cur(boardNum, pahseNum, redGreen);
	DBG("��λ�����ֵ:%d\n", curInfo);

	//RedExtinguish();

	curInfo &= 0x00ff;
	if (curInfo == 0xff)
	{
		curInfo = 0;
	}

	return curInfo;
}

/*********************************************************************************
*
* 	�����˰�ť�źš�
*
***********************************************************************************/

uint16 PedestrianCheckInput(void)
{
	uint16 input = PedestrianCheck();

	DBG("INFO 100141 ���˰�ť: 0x%x\n",input); 
	//INFO("---PedestrianCheckInput...");
	if(WIRELESS_CTRL_ON == gStructBinfileConfigPara.stWirelessController.iSwitch)
		WirelessKeyCheck();

	return input;
}


/*********************************************************************************
*
*  ���̴���
	����ֵ:
	0: �޼�����.
	1:�Զ�������,���ѷſ�.
	2:�ֶ�������,���ѷſ�.
	3:����������,���ѷſ�.
	4:ȫ�������,���ѷſ�.
	5:����������,���ѷſ�.
*
***********************************************************************************/
extern int KeyControlDeal(int result);
int ProcessKey(void)
{
	int result = 0;
	char tmpstr[256] = "";
	result = ProcessKeyBoard();
	
	DBG("INFO 100600: ���̴��� return %d,g_manual_pressed=%d,g_auto_pressed=%d,g_flashing_pressed=%d,g_allred_pressed=%d,g_step_by_step_pressed=%d \n",
		result,g_manual_pressed,g_auto_pressed,
		g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);

	if(result != 0)
	{
		sprintf(tmpstr,"���̴��� return %d,�ֶ�=%d,�Զ�=%d,����=%d,ȫ��=%d,����=%d \n",
			result,g_manual_pressed,g_auto_pressed,
			g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);
	
		WriteLogInfos("/home/Keyboard.log",tmpstr);
	}
	
	return KeyControlDeal(result);
	//return (0);
}

/*********************************************************************************
*
* 	д����������־��
*
***********************************************************************************/
void write_running_log()
{
	//д������־(ע:�ļ����ϼ�Ŀ¼�����Ѿ����ڣ������޷�����д������־)
	char msg[128] = "";
	sprintf(msg,"�źŻ�����ʼ����,��ǰ����汾��:%s",SOFTWARE_VERSION_INFO);
	WriteLogInfos("/home/StartUp.log",msg);
	ReadFaultStatus();
}

int itstaskmain(int argc, char *argv[]);

void SigHandler(int signum) //kill or killall will be called to free memory!
{
    INFO("signum = %d", signum);
	if (signum == SIGUSR1)
	{
		gOftenPrintFlag = !gOftenPrintFlag;
		return;
	}
	else
    	exit(0);
}
//It can be called after main() or exit() automatically!
__attribute__((destructor)) void ProgramExit(void)
{
    if (gSignalControlpara == NULL)
        free(gSignalControlpara);
    if (gCountDownParams == NULL)
        free(gCountDownParams);
    INFO("The program exit normaly!");
}
/*********************************************************************************
*
* 	���������
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	INFO("********************************main()****************************\n");
	INFO("compile time: %s, %s", __DATE__, __TIME__);
	//ע��һЩ�ź������ͷ�һЩ��Դ����������;
	signal(SIGTERM, SigHandler);    //for command 'kill' or 'killall'
	signal(SIGINT, SigHandler);		//for ctrl + C
	signal(SIGUSR1, SigHandler);	//for OFTEN print
	//д����������־����¼����汾��
	write_running_log();

	//�������ļ��л�ȡ���в�������
	InitCountDownParams();//����Ҫ�ȸ�����ʱ����ռ䣬�ٶ��ļ����������ɶδ���
	ReadBinAllCfgParams(&gStructBinfileConfigPara,&gStructBinfileCustom,&gStructBinfileDesc,&g_CountDownCfg,&gStructBinfileMisc);
	InitSignalMachineConfig();
	StepPthreadInit();
	//if (gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch == 1)
		gStructBinfileConfigPara.sSpecialParams.iRedSignalCheckSwitch = RedSignalCheckInit();
	
	//��sadp����������
	system("killall -9 RTSC_sadp");
	system("sleep 5 && /root/RTSC_sadp  >& /dev/null &");
	system("killall transfer");
	system("/root/transfer  >& /dev/null &");

	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	IO_Init();
		
	//canͨ�ų�ʼ��
	i_can_its_init();

	//�������udp��������
	udp_server_init();
	
	//Ĭ�ϵ����Զ���
	ProcessKeyBoardLight();

	//������Ź���ʼ��
	HardwareWatchdogInit();

	//GPS��ʼ��
	GPSInit();
	
    RecordNewFault(POW_ON_REBOOT);
	return itstaskmain(argc, argv);
}


/*********************************************************************************
*
* 	����ʱ�䵽Ӳ��RTC
*
***********************************************************************************/

void RTC_HW_fc(void)
{
	system("hwclock -w&");
	INFO("rtc hw fc.\n");
}

/*����color: 1:�̵ƣ�2:��ƣ�3:�Ƶ�*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //��20151225�𣬲������ٸ��Ͽⷢ�������ǰ���ڲ�������˴����ٱ�����
    if(gStepFlag != 0)
    {
        return;
    }    

	//OFTEN("veh:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;

	if(!(gCountDownParams->stVehPhaseCountingDown[phase - 1][0] == GREEN_BLINK
	    && color == GREEN))//�������ڼ��㵹��ʱ�ӿ��м�������ģ������λ���������Ͽ⴫������״̬���̵ƣ������޸�����ɫ״̬
	{
    	gCountDownParams->stVehPhaseCountingDown[phase - 1][0] = color;
	}

	gCountDownParams->stVehPhaseCountingDown[phase - 1][1] = phaseTime;
	CollectCountDownParams();

	/*���ø�Ӧ���ʱ��*/
	if((SetFreeGreenTimeSign == 0)&&(gStructBinfileCustom.sCountdownParams.iFreeGreenTime > 3))
	{
		SeFreeGreenTime(gStructBinfileCustom.sCountdownParams.iFreeGreenTime);//��Ӧ���ʱ��,ȱʡΪ9��.
		//printf("#Set freeGreenTime %d\n",gStructBinfileCustom.sCountdownParams.iFreeGreenTime);
		SetFreeGreenTimeSign = 1;
	}
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //��20151225�𣬲������ٸ��Ͽⷢ�������ǰ���ڲ�������˴����ٱ�����
    if(gStepFlag != 0)
    {
        return;
    }    

	//INFO("ped:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
	countdown_ped[phase].ped_color = color;
	countdown_ped[phase].ped_phaseTime = phaseTime;
	gCountDownParams->stPedPhaseCountingDown[phase - 1][0] = color;
	gCountDownParams->stPedPhaseCountingDown[phase - 1][1] = phaseTime;
	CollectCountDownParams();
}
//����ȫ�̵���ʱ
void Count_Down_pulse_All(uint16 *g_nLampStatus)
{
	int iPhase = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;

	memcpy(temp_nLampStatus,g_nLampStatus,16);
	for(iPhase = 1; iPhase<16; iPhase++)
	{
		if(countdown_veh[iPhase].veh_color > 0)
		{
			if(countdown_veh[iPhase].veh_color == 2)
			{
				nowRedLight[iPhase] = 1;//���
			}
			else
			{
				nowRedLight[iPhase] = 0;//���Ǻ��
			}
			if(nowRedLight[iPhase] != lastRedLight[iPhase])
			{
				stayTimes[iPhase]++;
				if(stayTimes[iPhase] == 2)
				{
					lastRedLight[iPhase] = nowRedLight[iPhase];
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(gStructBinfileCustom.sCountdownParams.iPhaseOfChannel[iChannel].iphase == iPhase)
						{
							dataNum1 = iChannel/4;
							dataNum2 = iChannel%4;
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+1)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+2)));
						}
					}
				}
			}
			else
			{
				stayTimes[iPhase] = 0;//������λ
			}
		}
	}
	memcpy(g_nLampStatus,temp_nLampStatus,16);
}
//�����̵���ʱ
void Count_Down_pulse_Half(uint16 *g_nLampStatus)
{
	int iPhase = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;

	memcpy(temp_nLampStatus,g_nLampStatus,16);
	for(iPhase = 1; iPhase<16; iPhase++)
	{
		if(countdown_veh[iPhase].veh_color > 0)
		{
			if(((countdown_veh[iPhase].veh_phaseTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) && (countdown_veh[iPhase].veh_color == 1))
			||((countdown_veh[iPhase].veh_phaseTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) && (countdown_veh[iPhase].veh_color == 2)))//��̵���ʱ
			{
				if(temp_Sign[iPhase] == 0)
				{
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(gStructBinfileCustom.sCountdownParams.iPhaseOfChannel[iChannel].iphase== iPhase)
						{
							dataNum1 = iChannel/4;
							dataNum2 = iChannel%4;
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+1)));
							temp_nLampStatus[dataNum1] = temp_nLampStatus[dataNum1] &( ~(1<<(dataNum2*3+2)));
						}
					}
					temp_Sign[iPhase] = 1;
				}
			}
			else
			{
				temp_Sign[iPhase] = 0;//��־λ��λ
			}
		}
	}
	memcpy(g_nLampStatus,temp_nLampStatus,16);
}
void CountDownPulse(void)
{
	if(1 == gStructBinfileCustom.sCountdownParams.iCountDownMode)
	{
		Count_Down_pulse_All(g_nLampStatus);
	}
	else if(2 == gStructBinfileCustom.sCountdownParams.iCountDownMode)
	{
		Count_Down_pulse_Half(g_nLampStatus);
	}
}
static void LockByChan(unsigned char flag, unsigned char *chanConfig, unsigned char *delayFlag,unsigned char *chanChangedFlag,unsigned char *cChanStatus,unsigned char *cArrayBlinkStatus,unsigned char *cArrayIsTimeEnd)
{
	int i = 0;
	unsigned short  value;
	unsigned char cFlag = 0;
/*
	time:	1s	3s	3s	---------	3s	---
	isLock:	0	0	0	1			1	0
	action:	lib	GB	Y	chanstatus	Y	lib
*/
	int ret=0;
	
	if(flag==0)
	{
		if(*delayFlag!=0)//lock just stop
		{
			if(transitionOnChanLockEnd(cChanStatus) == 1)//lock end transition finished
			{
				*delayFlag=0;
				memset(cChanStatus, 0, 32);
				return;
			}
		}
		else//no lock
		{
			return;
		}
	}
	if(!(*delayFlag))
	{
		ret = transitionOnChanLockStart(cChanStatus, chanConfig);
		if(ret == 0)
			return;
		else if(ret == 2)
		{
			memcpy(cChanStatus, chanConfig, 32);
			*delayFlag=1;
		}
	}
	else
	{
		if((flag == 1)&&(strncmp((char*)cChanStatus, (char*)chanConfig, 32)!=0))//channel value changed
		{
			//INFO("channel value changed!");
			*chanChangedFlag=1;
		}
	}
	if(*chanChangedFlag)
	{
		if(trainsitionOnChanChange(cChanStatus, chanConfig) == 1)
		{
			*chanChangedFlag=0;
			memcpy(cChanStatus, chanConfig, 32);
		}
	}
	for(i = 0 ; i < 32; i++)
	{
		cFlag = 0;
		
//		  switch(pChannnelLockedParams->ucChannelStatus[i])
		switch(cChanStatus[i])
		{
			case GREEN: 			value = 1;	  break;
			case RED:				value = 2;	  break;
			case YELLOW:			value = 4;	  break;
			case GREEN_BLINK:		
									if(++cArrayIsTimeEnd[i] < 2)
									{
										value = cArrayBlinkStatus[i];
										break;
									}
									cArrayIsTimeEnd[i] = 0;
									value = (cArrayBlinkStatus[i] == 0) ? 1 : 0;	
									cArrayBlinkStatus[i] = value;
									break;
			case YELLOW_BLINK:		
									if(++cArrayIsTimeEnd[i] < 2)
									{
										value = cArrayBlinkStatus[i];
										break;
									}
									cArrayIsTimeEnd[i] = 0; 								   
									value = (cArrayBlinkStatus[i] == 0) ? 4 : 0;  
									cArrayBlinkStatus[i] = value;
									break;
			case TURN_OFF:			value = 0;	  break;
			case INVALID:			cFlag = 1;	  break;
			default :				cFlag = 1;	  break;
		}

		if(cFlag == 0)//����Ǻ��Ի���Ĭ������£��������øýӿڡ�
		{
			put_lamp_value(&g_nLampStatus[i/4], i % 4, value);
		}
	}
}

#define SECS_OF_BLINK 	3
static int transitionOnChanLockStart(unsigned char *chan, unsigned char *chanConfig)
{
	int i,ret;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	static unsigned char chanLockCounter=0;
	static unsigned char locktime=0;
	static unsigned char lastChans[3][32]={{0},{0},{0}};
	unsigned short lightv[3]={0};//red yellow green
	unsigned short lights[3]={GREEN, RED, YELLOW};

	if(chan==NULL || chanConfig==NULL)
	{
		ERR("Channel value can't be NULL!");
		return 2;
	}
	
	chanLockCounter++;
	if(chanLockCounter < 4)
	{
		ret=0;
		for(iChannel=0; iChannel<32; iChannel++)
		{
			dataNum1 = iChannel/4;
			dataNum2 = iChannel%4;
			for(i=0; i<3; i++)
			{
				lightv[i] = (0x0001<<(dataNum2*3+i));
				if((g_nLampStatus[dataNum1] & lightv[i]) == lightv[i])
				{
					lastChans[chanLockCounter-1][iChannel]=lights[i];
					break;
				}
			}
			if(i == 3)
				lastChans[chanLockCounter-1][iChannel] = 0;	
		}
	
		if(3 == chanLockCounter)
		{
			for(i=0; i<32; i++)
			{
				//INFO("chan:%d, val: %d-%d-%d", i,lastChans[0][i],lastChans[1][i],lastChans[2][i]);
				if(chanConfig[i]==INVALID || chanConfig[i]==TURN_OFF)
				{
					chan[i]=chanConfig[i];
					continue;
				}
				if((lastChans[0][i]==lastChans[1][i]) && (lastChans[1][i]==lastChans[2][i]))
				{
					if(lastChans[0][i]==0)
						chan[i]=TURN_OFF;
					else
						chan[i]=lastChans[0][i];
					if(chan[i] == GREEN)
						INFO("chan:%d state green",i);
					continue;
				}
				if(((lastChans[0][i]&lastChans[1][i])&lastChans[2][i]) ==0)//blink
				{
					if(lastChans[0][i]==0)
						chan[i]=lastChans[2][i];
					else
						chan[i]=lastChans[0][i];
					chan[i]=((chan[i]==GREEN)?GREEN_BLINK : YELLOW_BLINK);
					INFO("---> Blink: chan:%d,value:%d...",i, chan[i]);
				}
				else//no blink 
				{
					chan[i]=lastChans[2][i];
				}
			}
			locktime = transitionControl(chan, chanConfig);
			INFO("--->lockstart: locktime: %d", locktime);
			memset(lastChans, 0, 3*32);
			if(locktime == 0)
			{
				chanLockCounter = 0;
				ret=2;
			}
			else
				ret=1;
		}
	}
	else
	{
		ret=1;//lock
		if(chanLockCounter%4==0)
			INFO("-->lockstart: lock 1s...");
		if((locktime>=(SECS_OF_BLINK*2)) && (chanLockCounter==(SECS_OF_BLINK*4+3)))
		{
			INFO("-->Start to Yellow...");
			for(i=0; i<32; i++)
			{
				if(chan[i]==GREEN_BLINK)
					chan[i]=YELLOW;
			}
		}
		if(chanLockCounter >= (locktime*4+3))
		{
			chanLockCounter = 0;
			ret=2;// release lock
		}
	}
	
	return ret;
}
static int transitionOnChanLockEnd(unsigned char *chan)
{
	int iChannel,ret=0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	unsigned short lightv=0;//red
	static unsigned char chanLockCounter=0;
	static unsigned char locktime=0;//SECS_OF_BLINK;

	chanLockCounter++;	
	if(chan == NULL)
		return ret;

	if(chanLockCounter == 1)
	{
		for(iChannel=0; iChannel<32; iChannel++)
		{
			if(chan[iChannel] == INVALID || chan[iChannel] == TURN_OFF)
				continue;
			
			dataNum1 = iChannel/4;
			dataNum2 = iChannel%4;
			lightv=(0x0001<<(dataNum2*3+1));//red
			if((g_nLampStatus[dataNum1] & lightv) == lightv)
			{
				if(chan[iChannel]==GREEN || chan[iChannel]==GREEN_BLINK || chan[iChannel]==YELLOW_BLINK)
				{// 	green/greenblink/yellowblink --> yellow
					chan[iChannel] = YELLOW;
					locktime = SECS_OF_BLINK;
				}
			}
		}
		INFO("---lockend: locktime %d", locktime);
		if(locktime == 0)
		{
			chanLockCounter = 0;
			ret = 1;
		}
		INFO("---chan 1/2/3/4: %d,%d,%d,%d", chan[0], chan[1], chan[2], chan[3]);
	}
	else
	{
		if(chanLockCounter%4==0)
			INFO("---lockend: lock 1s...");
		if(chanLockCounter >= (locktime*4))
		{
			chanLockCounter = 0;
			ret = 1;
		}
	}
	return ret;
}
static unsigned char transitionControl(unsigned char *chan, unsigned char *config)
{
	int i,time=0;
	if(chan==NULL || config==NULL)
		return 0;

	for(i=0; i<32; i++)
	{
		if(config[i] == INVALID || config[i] == TURN_OFF || config[i] != RED)
		{
			continue;
		}
		
		if(chan[i]==GREEN || chan[i]==GREEN_BLINK)
		{
			chan[i]=GREEN_BLINK;
			if(time < SECS_OF_BLINK*2)
				time = SECS_OF_BLINK*2;
		}
	}
	return time;
}
static int trainsitionOnChanChange(unsigned char *cur, unsigned char *config)
{
	int i,ret=0;
	static unsigned char counter=0;
	static unsigned char time=0;
	
	if(cur == NULL || config == NULL)
		return 0;

	counter++;
	if(counter == 1)
	{
		for(i=0; i<32; i++)
		{
			if(config[i] == INVALID || config[i] == TURN_OFF || config[i] != RED)
			{
				continue;
			}

			if(cur[i]==GREEN || cur[i] == GREEN_BLINK)
			{
				cur[i]=YELLOW;
				if(time==0)
					time=SECS_OF_BLINK;
			}
		}
		if(time==0)
		{
			counter=0;
			ret=1;
		}
	}
	else
	{
		if(counter >= (time*4))
		{
			counter=0;
			ret=1;
			time=0;
		}
	}

	return ret;
}
void FollowPhaseGreenBlink(UINT8 *chan)
{
	int i;
	int ctrlId=0;

	for(i=0; i<NUM_CHANNEL; i++)
	{
		if((gSignalControlpara->stChannel[i].nControllerID==0) || (gSignalControlpara->stChannel[i].nControllerType!=FOLLOW))
			continue;
		
		ctrlId = gSignalControlpara->stChannel[i].nControllerID-1;
		if(ctrlId >= NUM_PHASE)
		{
			ERR("Wrong phase id!");
			return;
		}
		if(!gFollowPhaseGreenBlinkFlag[ctrlId])
			continue;
		chan[i] = GREEN_BLINK;
	}
}
void WirelessChanLock(void)
{
	UINT8 flag=1;
	static unsigned int wirelessCounter=0;
	static unsigned char transitionLock=0;
	static unsigned char cArrayBlinkStatus[32] = {0};//����������������ʱ��һ�ε�ɫ��״̬������������ʱ�Ƿ�Ӧ�����    
	static unsigned char cArrayIsTimeEnd[32] = {0};//����ʱ��Ҫ��֤״̬����ʱ���500ms.	
	static unsigned char chanChanged=0;
	static unsigned char cChanStatus[32] = {0};
	unsigned char *chan=NULL;
	
	if(0 == gWirelessCtrlLockChanId)
	{
		flag = 0;
	}
	else
	{
	
		wirelessCounter++;
		if(gLastWirelessCtrlId != gWirelessCtrlLockChanId)
		{
			wirelessCounter = 0;
			gLastWirelessCtrlId = gWirelessCtrlLockChanId;
		}
		if((gStructBinfileConfigPara.stWirelessController.iOvertime!=0)&&(wirelessCounter>=(4*gStructBinfileConfigPara.stWirelessController.iOvertime)))
		{
			gWirelessCtrlLockChanId=0;
			flag = 0;
			wirelessCounter=0;
		}
		chan = gStructBinfileConfigPara.stWirelessController.key[gWirelessCtrlLockChanId-1].ucChan;
	}
	LockByChan(flag, chan, &transitionLock, &chanChanged, cChanStatus,cArrayBlinkStatus,cArrayIsTimeEnd);
}
void GetChanStatus(UINT16 *lampStatus, UINT8 *chanStatus)
{
	int ibit = 0;
	int iChannel = 0;
	int dataNum1 = 0;
	int dataNum2 = 0;
	unsigned short lightv[3]={0};//red yellow green
	unsigned char lights[3]={GREEN, RED, YELLOW};

	if(lampStatus == NULL || chanStatus == NULL)
		return;
	
	for(iChannel=0; iChannel<NUM_CHANNEL; iChannel++)
	{
		dataNum1 = iChannel/4;
		dataNum2 = iChannel%4;
		for(ibit=0; ibit<3; ibit++)
		{
			lightv[ibit] = (0x0001<<(dataNum2*3+ibit));
			if((lampStatus[dataNum1] & lightv[ibit]) == lightv[ibit])
			{
				chanStatus[iChannel]=lights[ibit];
				break;
			}
		}
		if(ibit == 3)
			chanStatus[iChannel] = TURN_OFF; 
	}
}
void SetLampStatus(uint16 *lampStatus, UINT8 *chanStatus)
{
	int i;
	unsigned short  value;
	unsigned char cFlag = 0;
    static unsigned char cArrayBlinkStatus[NUM_CHANNEL] = {0};//����������������ʱ��һ�ε�ɫ��״̬������������ʱ�Ƿ�Ӧ�����    
    static unsigned char cArrayIsTimeEnd[NUM_CHANNEL] = {0};//����ʱ��Ҫ��֤״̬����ʱ���500ms.

	if(lampStatus == NULL || chanStatus == NULL)
		return;
	for(i = 0 ; i < NUM_CHANNEL; i++)
	{
		cFlag = 0;	
		switch(chanStatus[i])
		{
			case GREEN: 	value = 1;	  break;
			case RED:		value = 2;	  break;
			case YELLOW:	value = 4;	  break;
			case GREEN_BLINK:		
								if(++cArrayIsTimeEnd[i] < 2)
								{
									value = cArrayBlinkStatus[i];
									break;
								}
								cArrayIsTimeEnd[i] = 0;
								value = (cArrayBlinkStatus[i] == 0) ? 1 : 0;	
								cArrayBlinkStatus[i] = value;
								break;
			case YELLOW_BLINK:		
								if(++cArrayIsTimeEnd[i] < 2)
								{
									value = cArrayBlinkStatus[i];
									break;
								}
								cArrayIsTimeEnd[i] = 0; 								   
								value = (cArrayBlinkStatus[i] == 0) ? 4 : 0;  
								cArrayBlinkStatus[i] = value;
								break;
			case TURN_OFF:	value = 0;	  break;
			case INVALID:	cFlag = 1;	  break;
			default :		cFlag = 1;	  break;
		}

		if(cFlag == 0)//����Ǻ��Ի���Ĭ������£��������øýӿڡ�
		{
			put_lamp_value(&lampStatus[i/4], i % 4, value);
		}
	}
}
void PedestrianFollowCtrl(UINT8 *chan)
{
	int i;
	UINT8 itime=0;
	int followPhase=0;
	int motherPhase=0;

	if(SPECIAL_CONTROL_SYSTEM != gStructBinfileCustom.cSpecialControlSchemeId)
		return;
	
	for(i=0; i<NUM_CHANNEL; i++)
	{
		if(gSignalControlpara->stChannel[i].nControllerType == OTHER)//ͨ������Դ����OTHER��ʾ���˸���
		{
			if(chan[i] == YELLOW || chan[i] == YELLOW_BLINK)//������λû�лƵƺͻ���
				chan[i] = RED;

			followPhase = gSignalControlpara->stChannel[i].nControllerID-1;
			if(followPhase >= NUM_PHASE || followPhase<0)
			{
				ERR("Wrong follow phase id!");
				continue;
			}
			motherPhase = gFollowPhaseMotherPhase[followPhase]-1;
			if(motherPhase < 0)//��һ��λҲ�ڸ�����λ����
				continue;
			//�������������Ƶƺ�ȫ��ʱ��
			itime = gCountDownParams->stPhaseRunningInfo[motherPhase][0]
				-(gSignalControlpara->stPhase[motherPhase].nYellowTime + 
				gSignalControlpara->stPhase[motherPhase].nAllRedTime + 
				gSignalControlpara->AscSignalTransTable[motherPhase].nGreenLightTime);

			if(gCountDownParams->ucOverlap[followPhase][0]==GREEN_BLINK)//ĸ��λ����ʱ��������λΪ���
				chan[i] = RED;

			if(chan[i] == GREEN && 
				gCountDownParams->stPhaseRunningInfo[motherPhase][1] > (itime-gSignalControlpara->stPhase[motherPhase].nPedestrianClearTime))
				chan[i] = GREEN_BLINK;
		}
	}
}
void ItsCustom(void)
{
	unsigned char chan[NUM_CHANNEL]={INVALID};
	GetChanStatus(g_nLampStatus, chan);//��ȡ���ǵ�ǰͨ���ĵ�ɫ(no blink status)

	/******start custom*******/
	FollowPhaseGreenBlink(chan);
	PedestrianFollowCtrl(chan);
	
	/******end custom*******/
	SetLampStatus(g_nLampStatus, chan);
}
