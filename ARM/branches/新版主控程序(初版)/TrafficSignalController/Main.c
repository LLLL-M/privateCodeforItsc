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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <time.h>
#include "common.h"
#include "Util.h"
#include "CPLD.h"
#include "canmsg.h"
#include "Database.h"


//�źŻ��������汾
#define SOFTWARE_VERSION_INFO "V1.0.0.1 build 2014.07.21"

typedef unsigned short  uint16;
static uint16 g_nLampStatus[8] = {0};

static int HardwareWatchdoghd = 0;
static int HardflashDogFlag = 0;
static int is_WatchdogEnabled = 0;
//static int g_yellowflash = 0; //������־��1������0����
static int g_errorstat = 0; //���ϱ�־ :1 :�й��ϣ� 0:�޹���
//static int g_closelampflag = 0; //�����ص�
//static uint16 g_yellowflashfreq = 1; //����Ƶ��
static int g_faultstatus[32] = {0}; //ͨ�����ϼ�¼���ڻָ�
static int CurDetectFreq = 0;
extern CountDownVeh countdown_veh[16];
extern CountDownPed countdown_ped[16];

unsigned char g_RedCurrentValue[32] = {0};   //32·��Ƶ���ֵ


struct FAILURE_INFO s_failure_info;       //������Ϣ�ṹ��

extern void get_fault_cfg(char *cfgName);

extern void udp_send_yellowflash();
extern void set_failure_number(unsigned int failurenumber);
extern void get_special_params();
extern void get_failure_number();
extern void get_current_params();
extern int udp_server_init();




static void RedgreenCollision(int boardNum, uint16 boardInfo);
static void RedExtinguish(); //��Ƹ�����������Ϊ���Ϩ��



#define TIMEOUT  20
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

extern int fd;
extern int auto_pressed;    			  //0:�Զ�����û�а���   1:�Զ������Ѱ���
extern int manual_pressed;				  //0:�ֶ�����û�а���   1:�ֶ������Ѱ���
extern int flashing_pressed;			  //0:��������û�а���   1:���������Ѱ���
extern int allred_pressed;				  //0:ȫ�찴��û�а���   1:ȫ�찴���Ѱ���
extern int step_by_step_pressed;		  //0:��������û�а���   1:���������Ѱ���
extern int globalWatchdogFlagDisable;     //0:Enable  1:Disable.//WatchDog ʹ�ܿ���. (����)
extern int RTC_USE_GPS_ENABLE;			  //��0:Enable  0:Disable //GPSʹ�ܿ���.  (����)
extern struct SPECIAL_PARAMS s_special_params;   //�����������
extern struct CURRENT_PARAMS g_struRecCurrent[32];   //��������
extern unsigned int g_failurenumber;          //�����¼����



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
		printf("������¼��Ϣ��Ч!\n");
		return -1;
	}
    
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("������¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("��ȡ������¼�ļ���Ϣʧ��!\n");
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
		printf("���ϼ�¼��Ϣ��Ч!\n");
		return -1;
	}
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("���ϼ�¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("��ȡ���ϼ�¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			printf("���ϼ�¼�ļ�������δ����ȷ��!\n");
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
    	printf("���ϼ�¼�ļ�δ����ȷ��!\n");
        return -1;
    }
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("��ȡ���ϼ�¼�ļ���Ϣʧ��!\n");
		return -1;
	}
	if(f_stat.st_size > 1024*1024)
	{
		fclose(pFileDebugInfo);
		pFileDebugInfo = fopen(pFile, "w+");
		if(pFileDebugInfo == NULL)
		{	
			printf("���ϼ�¼�ļ�������δ����ȷ��!\n");
			return -1;
		}
	}
	fwrite(&s_failure_info,sizeof(s_failure_info),1,pFileDebugInfo);
    fclose(pFileDebugInfo);
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
		printf("��FaultStatus.datʧ��,write\n");
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
		printf("��FaultStatus.datʧ��,read\n");
		return;
	}
	fread(g_faultstatus, sizeof(g_faultstatus), 1, pFile);
	fclose(pFile);
}



/*********************************************************************************
*
* 	Ӳ�����Ź���ʼ����
*
***********************************************************************************/

void HardwareWatchdogInit()
{
	//����Ӳ�����Ź�ι��
	is_WatchdogEnabled = s_special_params.iWatchdogSwitch;
	if(is_WatchdogEnabled == 1)
	{
		//ʹ��Ӳ�����Ź���ͨ���������ι��
		//�ر��ⲿι������
		system("killall watchdog");
		//��Ӳ�����Ź�
		HardwareWatchdoghd = open("/dev/watchdog", O_WRONLY);
		if(HardwareWatchdoghd == -1) 
		{
			printf("Open watchdog error!!!\n");
			return;
		}
		//����Ӳ�����Ź�
		ioctl(HardwareWatchdoghd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);	
		printf("Watchdog is enabled!!!\n");
	}
	else
	{
		//�ⲿι�������������(�ⲿι�������Ѿ��ڿ�������ʱ�Զ�����)
		printf("Watchdog is disabled!!!\n");
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
	freopen("/dev/tty","w",stdout);
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
	if(s_special_params.iGpsSwitch == 0)
	{
		//GPS����û�д�
		printf("GPS is disabled!!!\n");
		system("killall -9 GPS");
		Set_LED2_OFF();
	}
	else 
	{
		//GPS�����Ѿ���
		printf("GPS is enabled!!!\n");
		system("killall -9 GPS");
		//Ϩ��GPSָʾ��
		Set_LED2_OFF();
		system("/root/GPS&");
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
		printf("boardNum error:%d\n", boardNum);
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
#ifdef ARM_PLATFORM	
	i_can_its_get_Volt(boardNum, &boardInfo);
#endif	
	//��ȡ���ʱ�׼ʱ��
	time(&now);  
	  
	//ת��Ϊ����ʱ��
	timenow = localtime(&now); 

	sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
	//printf("%s  INFO 100131: boardNum:%d, boardInfodes:%d, boardInfosrc:%d\n",msg, boardNum, boardInfo, g_nLampStatus[boardNum-1]&0xfff); 
	
	//���̳�ͻ���
	RedgreenCollision(boardNum, boardInfo);
	return ~boardInfo;
}


/*********************************************************************************
*
*  д�ƿذ��źŵơ�
*
***********************************************************************************/
int PhaseLampOutput(int boardNum, uint16 outLamp)
{

	//printf("INFO 100130:%d:0x%x\n",boardNum, outLamp); 
	//print_data(outLamp);
	
	if (boardNum<1 || boardNum>8)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
#ifdef ARM_PLATFORM		
		i_can_its_send_led_request(boardNum, g_nLampStatus);
#endif		
		//���ذ�����ָʾ��
		Hiktsc_Running_Status();
		//Ӳ�����Ź�ι������
		HardwareWatchdogKeepAlive(boardNum);
	}

	//��ѹ���
	if(s_special_params.iVoltageAlarmSwitch == 1)
	{
		PhaseLampVoltInput(boardNum);
	}
	
	
	return 0;
}


/*********************************************************************************
*
* 	��������źš�
*
***********************************************************************************/

uint16 VehicleCheckInput(int boardNum)
{
	uint16 boardInfo = 0;

	if (boardNum<1 || boardNum>3)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}
#ifdef ARM_PLATFORM		
	boardInfo = recv_date_from_vechile(boardNum);
	printf("INFO 100140:%d,boardInfo:%d\n",boardNum,boardInfo); 
#endif

	return boardInfo;
}


/*********************************************************************************
*
*	дӲ�����źš�
*
***********************************************************************************/

void HardflashDogOutput(void)
{
	if (HardflashDogFlag == 1)
	{
		HardflashDogFlag = 0;
	}
	else
	{
		HardflashDogFlag = 1;
	}
	
	HardflashDogCtrl(HardflashDogFlag);

	//printf("INFO 100150 Ӳ�����ź�:%d\n", HardflashDogFlag); 

	CurDetectFreq++;
	if( CurDetectFreq > 2 )
	{
		//�������
		if(s_special_params.iCurrentAlarmSwitch == 1 )
		{
			RedExtinguish();
		}
		CurDetectFreq = 0;
	}
	return;
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
	printf("INFO 100142 �ſ����ź�: 0x%x\n",input); 


	return input;
}

/*********************************************************************************
*
* 	���̳�ͻ���̳�ͻ��⡣
*
***********************************************************************************/
void RedgreenCollision(int boardNum, uint16 boardInfo) 
{
	uint16 j = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	static uint16 errorcountgreen[32] = {0};
	static uint16 normalcountgreen[32] = {0};
	time_t now;  
	struct tm *timenow;
	
	char msg[128] = "";

	//printf("**************************boardno:%d,g_nLampStatus %d,boardinfo:%d\n", boardNum, g_nLampStatus[boardNum-1], boardInfo);
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
			printf("%s ��%dͨ����Ƶ�ѹ�쳣(���̳�ͻ����)��errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
			
			if (++errorcountred[(boardNum-1)*4+j] > TIMEOUT)
			{
				errorcountred[(boardNum-1)*4+j] = 0;
				normalcountred[(boardNum-1)*4+j] = 0;
				if(s_special_params.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					udp_send_yellowflash();
				}
				if(g_faultstatus[(boardNum-1)*4+j] == 0)
				{
					g_failurenumber ++;
					printf("%s ��%dͨ�����̳�ͻ,errorcountredgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountred[(boardNum-1)*4+j]);
					sprintf(msg, "��%dͨ�����̳�ͻ", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x0b;          //��Ϣ����ID
					s_failure_info.nNumber = g_failurenumber;       //���к�
					s_failure_info.nTime = now;
	  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
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
					g_failurenumber++;
					sprintf(msg, "��%dͨ�����̳�ͻ��������", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x0c;                      //���̳�ͻ�����Ϣ����ID
					s_failure_info.nNumber = g_failurenumber;       //���к�
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
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
			printf("%s ��%dͨ���̵Ƶ�ѹ�쳣��(�̳�ͻ����),errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
			if (++errorcountgreen[(boardNum-1)*4+j] > TIMEOUT)
			{		
				errorcountgreen[(boardNum-1)*4+j] = 0;
				normalcountgreen[(boardNum-1)*4+j] = 0;
				
				if(s_special_params.iVoltageAlarmAndProcessSwitch == 1)
				{
					g_errorstat = 1;
					udp_send_yellowflash();
				}
				g_failurenumber++;
				printf("%s ��%dͨ���̳�ͻ,errorcountgreen:%d\n", msg, (boardNum-1)*4+j+1, errorcountgreen[(boardNum-1)*4+j]);
				sprintf(msg, "��%dͨ���̳�ͻ", (boardNum-1)*4+j+1);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				s_failure_info.nID = 0x0f;                      //�̳�ͻ��Ϣ����ID
				s_failure_info.nNumber = g_failurenumber;       //���к�
				s_failure_info.nTime = now;
  				s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
				WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
				WriteErrorInfos("/home/FaultLog.dat", msg);
				set_failure_number(g_failurenumber);
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
					g_failurenumber++;
					sprintf(msg, "��%dͨ���̳�ͻ��������", (boardNum-1)*4+j+1);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					s_failure_info.nID = 0x10;                      //���̳�ͻ�����Ϣ����ID
					s_failure_info.nNumber = g_failurenumber;       //���к�
					s_failure_info.nTime = now;
		  			s_failure_info.nValue = (boardNum-1)*4+j+1;     //ͨ����   
					WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
					sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
					WriteErrorInfos("/home/FaultLog.dat", msg);
					set_failure_number(g_failurenumber);
					g_faultstatus[(boardNum-1)*4+j] = 0;
					WriteFaultStatus();
				}
				
			}
			
		}
	}

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
	for (i=0; i<8; i++)
	{
		for (j=0; j<4; j++)
		{
		#ifdef ARM_PLATFORM	
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //�ڣ�������д����,ֻ��ȡ��Ƶ���
        #endif
			//pcurInfo = 100;
			g_RedCurrentValue[i*4+j] = pcurInfo;
			//printf("ͨ��%02d��Ƶ���ֵ:%03d\n",i*4+j+1,pcurInfo);
			
			if ((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo < (g_struRecCurrent[i*4+j].RedCurrentBase - g_struRecCurrent[i*4+j].RedCurrentDiff) )
			{	
				printf("ͨ��%02d��������Ҽ������쳣,errorcountred[%d]=%d\n",i*4+j+1,i*4+j+1,errorcountred[i*4+j]+1);
				//��������ҵ�������쳣
				if (++errorcountred[i*4+j] > 10)
				{
					errorcountred[i*4+j] = 0;
					normalcountred[i*4+j] = 0;
					if(s_special_params.iCurrentAlarmAndProcessSwitch == 1)
					{
						//g_yellowflash = 1;
						//���Ϊ�й���
						g_errorstat = 1;
						udp_send_yellowflash();
					}
					if(g_faultstatus[i*4+j] == 0)
					{
						//��ȡ���ʱ�׼ʱ��
						time(&now); 
						
						
						g_failurenumber ++;
						printf("��%dͨ�����Ϩ��\n", i*4+j+1);
						sprintf(msg, "��%dͨ�����Ϩ��", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						s_failure_info.nID = 0x0d;                      //���Ϩ����Ϣ����ID
						s_failure_info.nNumber = g_failurenumber;       //���к�
						s_failure_info.nTime = now;
	  					s_failure_info.nValue = i*4+j+1;     //ͨ����   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
							s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						set_failure_number(g_failurenumber);
						g_faultstatus[i*4+j] = 3;
						//��g_faultstatus����д���ļ�
						WriteFaultStatus();
						//return;
					}
				}
				//return;
			}
			else if((g_nLampStatus[i] & (1<<(1+j*3))) > 0 && pcurInfo > (g_struRecCurrent[i*4+j].RedCurrentBase - g_struRecCurrent[i*4+j].RedCurrentDiff))
			{
				
				//��ͨ��������������ҵ���ֵ����
				if (++normalcountred[i*4+j] > 10)
				{
					normalcountred[i*4+j] = 0;
					errorcountred[i*4+j] = 0;
					//��ͨ�����д�����������
					if (g_faultstatus[i*4+j] == 3)
					{
						time(&now); 
						sprintf(msg, "��%dͨ�����Ϩ���������", i*4+j+1);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						g_failurenumber ++;
						s_failure_info.nID = 0x0e;                      //���Ϩ��������Ϣ����ID
						s_failure_info.nNumber = g_failurenumber;       //���к�
						s_failure_info.nTime = now;
			  			s_failure_info.nValue = i*4+j+1;     //ͨ����   
						WriteFailureInfos("/home/FailureLog.dat",s_failure_info);
						sprintf(msg, "���:%d,����:0x%x,ʱ��:%ld,ͨ��:%d",
									s_failure_info.nNumber,s_failure_info.nID,s_failure_info.nTime,s_failure_info.nValue);
						WriteErrorInfos("/home/FaultLog.dat", msg);
						set_failure_number(g_failurenumber);
						g_faultstatus[i*4+j] = 0;
						WriteFaultStatus();
					}
				}
			}
		}
	}
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
		printf("boardNum or pahseNum error:%d, %d\n", boardNum, pahseNum);
		return 0;
	}
	uint16 curInfo = 0;
	if (g_errorstat == 1)
	{
		return curInfo;
	}
	printf("INFO 100132:%d  %d   %d\n",boardNum, pahseNum, redGreen); 
	#ifdef ARM_PLATFORM	
	curInfo = i_can_its_get_cur(boardNum, pahseNum, redGreen);
    #endif
	printf("��λ�����ֵ:%d\n", curInfo);

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

	printf("INFO 100141 ���˰�ť: 0x%x\n",input); 


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
int ProcessKey(void)
{
	int result = 0;
	char tmpstr[256] = "";
	result = ProcessKeyBoard();
	
	printf("INFO 100600: ���̴��� return %d,manual_pressed=%d,auto_pressed=%d,flashing_pressed=%d,allred_pressed=%d,step_by_step_pressed=%d \n",
		result,manual_pressed,auto_pressed,
		flashing_pressed,allred_pressed,step_by_step_pressed);

	if(result != 0)
	{
		sprintf(tmpstr,"���̴��� return %d,�ֶ�=%d,�Զ�=%d,����=%d,ȫ��=%d,����=%d \n",
			result,manual_pressed,auto_pressed,
			flashing_pressed,allred_pressed,step_by_step_pressed);
	
		WriteLogInfos("/home/Keyboard.log",tmpstr);
	}
	
	return result;
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


	char msg[256] = "";
	memset(msg,0,sizeof(msg));
	sprintf(msg,"�źŻ�����ʼ����,��ǰ����汾��: %s",SOFTWARE_VERSION_INFO);
	//printf("===>   %s\n",msg);
	WriteLogInfos("/home/StartUp.log",msg);
	ReadFaultStatus();
}

int itstaskmain(int argc, char *argv[]);

/*********************************************************************************
*
* 	���������
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	printf("********************************main()****************************\n");
	//д����������־����¼����汾��
	write_running_log();
	
	//�������ļ��л�ȡ�������
	get_special_params(PATH_LOGIN);

	//�������ļ��л�ȡ������־���
	get_failure_number();

	//��ȡ��������
	get_current_params(PATH_LOGIN);

    get_fault_cfg(PATH_LOGIN);

	
#ifdef ARM_PLATFORM	
	//canͨ�ų�ʼ��
	i_can_its_init();
	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	CPLD_IO_Init();
#endif	

	//�������udp��������
	//udp_server_init();
	
	//����ʱ�ƽ���
	/*SeFreeGreenTime(0);//��Ӧ���ʱ��
	memset((char*)countdown_veh, 0, sizeof(countdown_veh));
	memset((char*)countdown_ped, 0, sizeof(countdown_ped));
	count_down_thread_create();*/
	
	//Ĭ�ϵ����Զ���
	ProcessKeyBoardLight();

	//������Ź���ʼ��
	HardwareWatchdogInit();

	//GPS��ʼ��
	GPSInit();


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
	printf("rtc hw fc.\n");
}

/*����color: 1:�̵ƣ�2:��ƣ�3:�Ƶ�*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
	printf("veh:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;
	
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
	printf("ped:phase %d color %d phaseTime %d \n", phase, color, phaseTime);
}
