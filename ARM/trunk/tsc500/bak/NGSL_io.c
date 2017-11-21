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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"
#include "HikConfig.h"

typedef unsigned short  uint16;
static uint16 g_nLampStatus[8] = {0};
uint16 temp_nLampStatus[8] = {0}; //��������ʽ��������ʱ��
char temp_Sign[16] = {0};//��־λ
char lastRedLight[16] = {0}; //�����һ��״̬
char nowRedLight[16] = {0}; //�������״̬
char stayTimes[16] = {0};//�ӳٷ�������ʱ��
int HardwareWatchdoghd = 0;
static int HardflashDogFlag = 0;
int is_WatchdogEnabled = 0;
//static int g_yellowflash = 0; //������־��1������0����
static int g_errorstat = 0; //���ϱ�־ :1 :�й��ϣ� 0:�޹���
//static int g_closelampflag = 0; //�����ص�
//static uint16 g_yellowflashfreq = 1; //����Ƶ��
static int g_faultstatus[32] = {0}; //ͨ�����ϼ�¼���ڻָ�
static int CurDetectFreq = 0;
//��ʱ�޸ģ�����Խ��
extern CountDownVeh countdown_veh[17];
extern CountDownPed countdown_ped[17];
static int SetFreeGreenTimeSign = 0;//��Ӧʱ���־λ
unsigned char g_RedCurrentValue[32] = {0};   //32·��Ƶ���ֵ
struct FAILURE_INFO s_failure_info;       //������Ϣ�ṹ��

#define TIMEOUT  20
#define _YELLOWFLASH_

PHASE_COUNTING_DOWN_PARAMS *gCountDownParams = NULL;        //����ʱ�ӿ���Ϣ
UInt8 gSpecialControlSchemeId = 0;       //������Ʒ�����
PhaseTurnItem gPhaseTurn[4];

extern CHANNEL_LOCK_PARAMS gChannelLockedParams;
extern UInt8 gChannelLockFlag ;


typedef enum
{
         TURN_OFF = 0,
         GREEN = 1,
         RED = 2,
         YELLOW = 3,
         GREEN_BLINK = 4,
         YELLOW_BLINK = 5,
         ALLRED = 6,
         INVALID = 7,
} PhaseChannelStatus;

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
extern struct Count_Down_Params g_struCountDown;    //����ʱ����
unsigned int g_printf_switch;                 //��ӡ��Ϣ�Ƿ�������
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
	if(1 == g_printf_switch)
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

extern void RedgreenCollision(int boardNum, uint16 boardInfo);
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
	i_can_its_get_Volt(boardNum, &boardInfo);
	//��ȡ���ʱ�׼ʱ��
	time(&now);  
	  
	//ת��Ϊ����ʱ��
	timenow = localtime(&now); 

	sprintf(msg,"%04d.%02d.%02d-%02d:%02d:%02d",
		timenow->tm_year + 1900,timenow->tm_mon + 1,timenow->tm_mday,
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
	printf("%s  INFO 100131: boardNum:%d, boardInfodes:%d, boardInfosrc:%d\n",msg, boardNum, boardInfo, g_nLampStatus[boardNum-1]&0xfff); 
	
	//���̳�ͻ���
	RedgreenCollision(boardNum, boardInfo);
	return ~boardInfo;
}


typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

/*****************************************************************************
 �� �� ��  : put_lamp_value
 ��������  : ��Ҫ��������һ�����ĳ���Ƶ�״ֵ̬
 �������  : volatile unsigned short *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
             unsigned short value             Ҫ���õĵƵ�״ֵ̬
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}


#define Time_To_Minutes(hour,minute,second) (hour*3600+minute*60+second)

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
    else//����ʼʱ��Ƚ���ʱ��Сʱ
    {
        if((iNow > iBeginTime) && (iNow < iEndTime))
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
    int i = 0;
    unsigned short tmp, value;
    unsigned char cFlag = 0;
    static unsigned char cArrayBlinkStatus[32] = {0};//����������������ʱ��һ�ε�ɫ��״̬������������ʱ�Ƿ�Ӧ�����
    static unsigned char cArrayIsTimeEnd[32] = {0};//����ʱ��Ҫ��֤״̬����ʱ���500ms.
    
    if(gChannelLockFlag == 0)//���û��ͨ�����������ֱ�ӷ��ء�
    {
        return;
    }
    //��������˹������򵫵�ǰʱ�䲢���ڹ��������ڣ���ֱ�ӷ��ء�
    if((pChannnelLockedParams->ucWorkingTimeFlag == 1) && (IsDateInControlArea(pChannnelLockedParams) == 0))
    {
        return;
    }

    for(i = 0 ; i < 32; i++)
    {
        cFlag = 0;
        
        switch(pChannnelLockedParams->ucChannelStatus[i])
        {
            case GREEN:             value = 1;    break;
            case RED:               value = 2;    break;
            case YELLOW:            value = 4;    break;
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
            case TURN_OFF:          value = 0;    break;
            case INVALID:           cFlag = 1;    break;
            default :               cFlag = 1;    break;
        }

        if(cFlag == 0)//����Ǻ��Ի���Ĭ������£��������øýӿڡ�
        {   
            put_lamp_value(&g_nLampStatus[i/4], i % 4, value);
        }
    }
}


/*********************************************************************************
*
*  д�ƿذ��źŵơ�
*
***********************************************************************************/
int PhaseLampOutput(int boardNum, uint16 outLamp)
{
	char *temp_LampStatus = NULL;
	//fprintf(stderr,"PhaseLampOutput %d:0x%x \n",boardNum, outLamp); 
	//print_data(outLamp);
	
	if (boardNum<1 || boardNum>8)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}

	g_nLampStatus[boardNum-1] = outLamp;
	if (boardNum == 8)
	{
	    //fprintf(stderr,"PhaseLampOutput  0x%x || 0x%x  || 0x%x  ||  0x%x  || 0x%x\n",g_nLampStatus[0],g_nLampStatus[1],g_nLampStatus[2],g_nLampStatus[3],g_nLampStatus[4]);
		if(1 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_All(g_nLampStatus);
		}
		else if(2 == g_struCountDown.iCountDownMode)
		{
			Count_Down_pulse_Half(g_nLampStatus);
		}
		LockChannel(&gChannelLockedParams);
		i_can_its_send_led_request(boardNum, g_nLampStatus);

		//���ذ�����ָʾ��
		Hiktsc_Running_Status();
		//Ӳ�����Ź�ι������
		HardwareWatchdogKeepAlive(boardNum);
	}

	//��ѹ���
	if(s_special_params.iVoltageAlarmSwitch == 1 && s_special_params.iErrorDetectSwitch == 1)
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
	int iPedNO = 0;//������
	
	if (boardNum<1 || boardNum>3)
	{
		printf("boardNum error:%d\n", boardNum);
		return -1;
	}
	boardInfo = recv_date_from_vechile(boardNum);
	
	printf("INFO 100140:%d,boardInfo:%d\n",boardNum,boardInfo); 


	return boardInfo;
}


/*********************************************************************************
*
*	дӲ�����źš�
*
***********************************************************************************/
extern void RedExtinguish(); //��Ƹ�����������Ϊ���Ϩ��
void HardflashDogOutput(void)
{
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

	CurDetectFreq++;
	if( CurDetectFreq > 2 )
	{
		//1����ͬ��һ��nandflash����ֹ�ϵ��������ļ���ʧ
		//system("sync &");
		//�������
		if(s_special_params.iCurrentAlarmSwitch == 1 && s_special_params.iErrorDetectSwitch == 1)
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
	uint16 i = 0;
	uint16 j = 0;
	static uint16 errorcountred[32] = {0};
	static uint16 normalcountred[32] = {0};
	static uint16 errorcountgreen[32] = {0};
	static uint16 normalcountgreen[32] = {0};
	time_t now;  
	struct tm *timenow;
	
	char msg[128] = "";

	printf("**************************boardno:%d,g_nLampStatus:%d,boardinfo:%d\n", boardNum, g_nLampStatus[boardNum-1], boardInfo);
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
			pcurInfo = i_can_its_get_cur(i+1, j+1, 1);   //�ڣ�������д����,ֻ��ȡ��Ƶ���
			//pcurInfo = 100;
			g_RedCurrentValue[i*4+j] = pcurInfo;
			printf("ͨ��%02d��Ƶ���ֵ:%03d\n",i*4+j+1,pcurInfo);
			
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
	curInfo = i_can_its_get_cur(boardNum, pahseNum, redGreen);
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
	char msg[128] = "";
	sprintf(msg,"�źŻ�����ʼ����,��ǰ����汾��:%s",SOFTWARE_VERSION_INFO);
	WriteLogInfos("/home/StartUp.log",msg);
	ReadFaultStatus();
}

int itstaskmain(int argc, char *argv[]);

void TestPhase()
{
    gCountDownParams = (PHASE_COUNTING_DOWN_PARAMS *)malloc(sizeof(PHASE_COUNTING_DOWN_PARAMS));
    memset(gCountDownParams,0,sizeof(PHASE_COUNTING_DOWN_PARAMS));


    memset(gPhaseTurn,0,4*sizeof(PhaseTurnItem));

    gPhaseTurn[0].nPhaseTurnID = 1;
    gPhaseTurn[0].nCircleID = 1;

    int i = 0; 
    for(i = 0 ; i < 4; i++)
    {
        gPhaseTurn[0].nTurnArray[i] = i+1;
    }
    
    gPhaseTurn[1].nPhaseTurnID = 1;
    gPhaseTurn[1].nCircleID = 2;

    for(i = 0 ; i < 4; i++)
    {
        gPhaseTurn[1].nTurnArray[i] = i+5;
    }    

    gChannelLockFlag = 1;

    gChannelLockedParams.ucWorkingTimeFlag = 0;

    gChannelLockedParams.ucBeginTimeHour = 5;
    gChannelLockedParams.ucEndTimeHour = 5;
    gChannelLockedParams.ucEndTimeMin = 6;
    
    for(i = 0 ; i < 32 ; i++)
    {
        gChannelLockedParams.ucChannelStatus[i] = INVALID;
    }

    gChannelLockedParams.ucChannelStatus[13] = GREEN;
    gChannelLockedParams.ucChannelStatus[1] = GREEN_BLINK;
    gChannelLockedParams.ucChannelStatus[15] = YELLOW;
    gChannelLockedParams.ucChannelStatus[16] = YELLOW_BLINK;
    gChannelLockedParams.ucChannelStatus[17] = RED;
    gChannelLockedParams.ucChannelStatus[18] = YELLOW;
}


/*********************************************************************************
*
* 	���������
*
***********************************************************************************/
int main(int argc, char *argv[])
{	
	printf("********************************main()****************************\n");
	TestPhase();

	//д����������־����¼����汾��
	write_running_log();

	//�������ļ��л�ȡ���в�������
	get_all_params_from_config_file();
	

	//canͨ�ų�ʼ��
	i_can_its_init();
	
	//CPUͨ��CPLD�����IO�ڳ�ʼ��
	CPLD_IO_Init();

	//�������udp��������
	udp_server_init();
	
	//����ʱ�ƽ���
	//count_down_thread_create();

	//Ĭ�ϵ����Զ���
	ProcessKeyBoardLight();

	//������Ź���ʼ��
	HardwareWatchdogInit();

	//GPS��ʼ��
	GPSInit();

	//��sadp����������
	system("killall -9 RTSC_sadp");
	system("sleep 5 && /root/RTSC_sadp &");


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

/*****************************************************************************
 �� �� ��  : UpdateCountDownTime
 ��������  : ��֤��Ӧ����ʱ��������λ�ĵ���ʱ���ܱ�����ͬ�����ı仯
 �������  : int iDiffValue                                
             unsigned char *pPhaseArray                    
             int *pOldPhaseCountDownVaule                  
             PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void UpdateCountDownTime(int iDiffValue,unsigned char *pPhaseArray,int *pOldPhaseCountDownVaule,PHASE_COUNTING_DOWN_PARAMS *pCountDownParams)
{
    int i = 0;
    unsigned char cPhaseId = 0;
    
    if((iDiffValue < 0) || (pPhaseArray == NULL) || (pCountDownParams == NULL) || (pOldPhaseCountDownVaule == NULL))
    {
        return;
    }

    for(i = 0 ; i < 16; i++)
    {
        cPhaseId = pPhaseArray[i];
        if(cPhaseId == 0)
        {
            break;
        }
        //����û��ڣ�����λ�ĵ���ʱ����ֵ�������̵���λ�����ֵ�����ֶ������䵹��ʱʱ��

        if(pOldPhaseCountDownVaule[cPhaseId - 1] == 0)//�����ǵ�һ�ε��øú�������һ�εĵ���ʱ���ݻ�û�б�����������ֱ�ӱ���
        {
            pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
            continue;
        }
        
        if(((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - pOldPhaseCountDownVaule[cPhaseId - 1]) != iDiffValue) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))
        {
            //fprintf(stderr,"Changed %d , old  %d Diff %d \n\n",cPhaseId,pOldPhaseCountDownVaule[cPhaseId - 1],iDiffValue);
            pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] = pOldPhaseCountDownVaule[cPhaseId - 1] + iDiffValue;

           // pOldPhaseCountDownVaule[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];
        }
    }

}

/*****************************************************************************
 �� �� ��  : StoreOldCountDownData
 ��������  : ������һ�εĵ���ʱ��Ϣ
 �������  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             int *nArray                                   
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��6��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void StoreOldCountDownData(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,int *nArray)
{
    int i = 0;

    if((nArray == NULL) || (pCountDownParams == NULL))
    {

    }

    for(i = 0 ; i < 16; i++)
    {

        nArray[i] = pCountDownParams->stVehPhaseCountingDown[i][1];
    }


}

/*****************************************************************************
 �� �� ��  : CalcGreenSplit
 ��������  : ������λ�����ű�
 �������  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             unsigned char cCircleNo                       
             PhaseTurnItem *pPhaseTurn                     
             unsigned char uIndex                          
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcGreenSplit(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,unsigned char cCircleNo,PhaseTurnItem *pPhaseTurn,unsigned char uIndex)
{
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;

    for(k = 0 ; k < uIndex - 2; k++)//������
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ

        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            break;
        }
		
		if(((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0)) && 
			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
		{
			pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
																	((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);		
		}

    }

    for(k = uIndex; k < NUM_PHASE - 1; k++)//��ǰ����
    {
        cPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k];//�õ���ǰ����λ��
        iNextPhaseId = pPhaseTurn[cCircleNo].nTurnArray[k+1];//�õ�����������һ����λ
        if((0 == cPhaseId) || (0 == iNextPhaseId))
        {
            if((uIndex >= 2) && (cPhaseId != 0))//
            {
				if(((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] != 0)) && 
					((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
				{				
					pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] == 0) ?  pCountDownParams->stPedPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1] : pCountDownParams->stVehPhaseCountingDown[pPhaseTurn[cCircleNo].nTurnArray[0] - 1][1]) - 
																			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]);
				
				}
            }
            
            break;
        }

		if(((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0)) && 
			((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0) || (pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] != 0)))
		{		
			pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) - 
																	((pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1]); 
		}
    }
}

/*****************************************************************************
 �� �� ��  : CalcCircleTime
 ��������  : �����������ڳ�
 �������  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             PhaseTurnItem *pPhaseTurn                     
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void CalcCircleTime(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn)
{
    int i = 0;
    int j = 0;
    
    int iTempVal = 0;
    
    for(i = 0 ; i < 4; i++)
    {
        if(0 == pPhaseTurn[i].nTurnArray[0])//���ĳ����û�����ã��򲻼�������
        {
            continue;
        } 
        iTempVal = 0;
        
        for(j = 0 ; j < NUM_PHASE; j++)
        {
            if(0 == pPhaseTurn[i].nTurnArray[j])
            {
                break;
            }
            iTempVal += pCountDownParams->stPhaseRunningInfo[pPhaseTurn[i].nTurnArray[j] - 1][0];
        }

        pCountDownParams->ucCurCycleTime = iTempVal;
        break;
    }
}

/*****************************************************************************
 �� �� ��  : AlterPhaseGreenLightTime
 ��������  : ������λ����ʱ�䣬���ض�ʱ��ε��̵�״̬��Ϊ����
 �������  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  
             unsigned char *pPhaseGreenBlink               
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��10��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void AlterPhaseGreenLightTime(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,unsigned char *pPhaseGreenBlink)
{
    int i = 0;

    for(i = 0 ; i < 16 ; i++)
    {
        if((pCountDownParams->stVehPhaseCountingDown[i][0] == GREEN) && (pCountDownParams->stVehPhaseCountingDown[i][1] <= pPhaseGreenBlink[i]))
        {
            pCountDownParams->stVehPhaseCountingDown[i][0] = GREEN_BLINK;
           // fprintf(stderr,"===>  Phase  Green Blink %d\n",i+1);
        }
    }
}


/*****************************************************************************
 �� �� ��  : CalcPhaseRunTimeAndSplit
 ��������  : �ڵ���ʱ�м������λ�����ű�ʱ�䡢����ʱ�估��������ʱ�䡢����ʱ��
 �������  : PHASE_COUNTING_DOWN_PARAMS *pCountDownParams  ����ʱ�ṹ��ָ��
             PhaseTurnItem *pPhaseTurn                     ��ǰ���е������         ��ά����[4][16]
             unsigned char *pPhaseGreenLight                ��λ����ʱ��ָ��        ������16
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��9��
    ��    ��   : Ф�Ļ�
    �޸�����   : �ݲ��������˸�Ӧ�����
*****************************************************************************/
void CalcPhaseRunTimeAndSplit(PHASE_COUNTING_DOWN_PARAMS *pCountDownParams,PhaseTurnItem *pPhaseTurn,unsigned char *pPhaseGreenBlink)
{
    int i = 0;
    int j = 0;
    int k = 0;
    unsigned char cPhaseId = 0;
    int iNextPhaseId = 0;
    int index = 0;
    int iTempVal = 0 ;
    int iDiffVal = 0;//������¼��Ӧʱ��ǰ��λ���̵�����ֵ
    static int nArrayCurrentPhaseId[4] = {-1,-1,-1,-1};//��¼��ǰ�̵���λ�ţ����û��ڣ��̵���λ�����䶯ʱ�����ϸ���λ����λ����ʱ������
    static int nArrayCurrentPhaseCountDown[16] = {0};//���ؼ�¼һ�ݸ�����λ�ĵ���ʱ��Ϣ��������Ӧʱ����֤������λ�ĵ���ʱ��ͬʱ����
    
    if((NULL == pCountDownParams) || (NULL == pPhaseTurn))
    {
        return;
    }

   // fprintf(stderr,"===>  %d  ||  %d  ||  %d ||  %d\n",pCountDownParams->stVehPhaseCountingDown[0][1],pCountDownParams->stPedPhaseCountingDown[1][1],
                                                      //  pCountDownParams->stPedPhaseCountingDown[2][1],pCountDownParams->stVehPhaseCountingDown[3][1]);

    //��ѯ��������㵥���и�����λ�����ű�
    for(i = 0 ; i < 4 ; i++)
    {
        if(0 >= pPhaseTurn[i].nTurnArray[0])//���ĳ����û�����ã��򲻼�������
        {
            continue;
        }
        for(j = 0 ; j < NUM_PHASE; j++)//��ѯ�û��ڵ���λ
        {
            cPhaseId = pPhaseTurn[i].nTurnArray[j];
            if(0 >= cPhaseId)//����û������ҵ���λIDΪ0����λ�����������ѯ������ֱ��break�������ټ���û���
            {
                break;
            }

            if(pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] != 0)//�����ǰ��λ����ʱ�䲻Ϊ0���������������
            {
                pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1]++;
            }

            if((GREEN == pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][0]) || (GREEN == pCountDownParams->stPedPhaseCountingDown[cPhaseId - 1][0]))//�ҵ���ɫΪ��ɫ����λ
            {
                if(nArrayCurrentPhaseId[i] == -1)
                {
                    nArrayCurrentPhaseId[i] = cPhaseId;//��¼��ǰ�̵���λ
                }
                if(cPhaseId != nArrayCurrentPhaseId[i])//���������λ�����䶯��������ϸ���λ������ʱ��
                {
                    pCountDownParams->stPhaseRunningInfo[nArrayCurrentPhaseId[i] - 1][1] = 0;
                    nArrayCurrentPhaseId[i] = cPhaseId;//���µ�ǰ�̵���λ
                }

                if(0 == pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1])
                {
                    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] = 1;//�������ʱ����0��������ʱ���Ϊ1
                }

                index = j+1;//��¼�̵���λ��˳���Դ�Ϊ��㣬��������ֵ���ɵó���ĩ��λ������ű�

                //�ڸ�Ӧʱ��Ҫ��֤һ������������λ�ĵ���ʱʱ�䶼Ҫͬ��������ֵͬ
                if(nArrayCurrentPhaseCountDown[cPhaseId - 1] == 0)
                {
                    nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];//�տ�ʼʱ��¼
                }
                iDiffVal = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] - nArrayCurrentPhaseCountDown[cPhaseId - 1];


                if((iDiffVal >= 0) && (nArrayCurrentPhaseCountDown[cPhaseId - 1] != 1 ) && (pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1] != 0))//ֻ������λδ�����ı��ҵ���ʱ����ʱ�ŵ��á�
                {
                    //fprintf(stderr,"===>phase %d  countdown   %d\n",cPhaseId,nArrayCurrentPhaseCountDown[cPhaseId - 1]);
                    UpdateCountDownTime(iDiffVal,pPhaseTurn[i].nTurnArray, nArrayCurrentPhaseCountDown,pCountDownParams);
                }

                nArrayCurrentPhaseCountDown[cPhaseId - 1] = pCountDownParams->stVehPhaseCountingDown[cPhaseId - 1][1];

                //���㵱ǰ��λ�����ű�ʱ��,������һ����λ�ĵ���ʱʱ����ϵ�ǰ��λ������ʱ��
                iNextPhaseId = pPhaseTurn[i].nTurnArray[index];
                if(iNextPhaseId == 0)//�������λ����һ����λ�����ڣ�����һ����λΪ�������ʼֵ
                {
                    iNextPhaseId = pPhaseTurn[i].nTurnArray[0];
                }
				
				if((pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] != 0) || (pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] != 0))
				{
				    pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][0] = ((pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1] == 0) ? pCountDownParams->stPedPhaseCountingDown[iNextPhaseId - 1][1] : pCountDownParams->stVehPhaseCountingDown[iNextPhaseId - 1][1]) + pCountDownParams->stPhaseRunningInfo[cPhaseId - 1][1] - 1;
				}
				

                //������ֵ�ķ�ʽ��������һ����λ��������ű�
                CalcGreenSplit(pCountDownParams,i,pPhaseTurn,index);
            }
        }
    }
    
    //����ʱ����ڻ�������ں���Ҫ���㣬���¿�ʼ��һ������
    if((pCountDownParams->ucCurRunningTime >= pCountDownParams->ucCurCycleTime) && (pCountDownParams->ucCurCycleTime != 0))
    {
        pCountDownParams->ucCurRunningTime = 0;
    }
    //��������ʱ��
    pCountDownParams->ucCurRunningTime++;

    //������������
    CalcCircleTime(pCountDownParams,pPhaseTurn);

    //store old data
    StoreOldCountDownData(pCountDownParams,nArrayCurrentPhaseCountDown);

    //�޸�����״̬
    AlterPhaseGreenLightTime(pCountDownParams,pPhaseGreenBlink);
}

int iRecvPhase = 0;//�Ѿ��յ�����λ����
unsigned char gGreenBlink[16] = {0};
/*����color: 1:�̵ƣ�2:��ƣ�3:�Ƶ�*/
void CountDownOutVeh(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    //fprintf(stderr,"CountDownOutVeh  phase %d  %s phaseTime %d \n\n\n", phase, ((color == 1) ? "�̵�" : (color == 2) ? "���" : "�Ƶ�"), phaseTime);
    gCountDownParams->stVehPhaseCountingDown[phase - 1][0] = color;
    gCountDownParams->stVehPhaseCountingDown[phase - 1][1] = phaseTime;

    gGreenBlink[phase - 1] = 3;
   
    iRecvPhase++;
#if 1
	if(iRecvPhase == 4)
	{  // iRecvPhase = 0;
       // fprintf(stderr,"<phase%d %s phaseTime %d >\n\n", phase, ((color == 1) ? "�̵�" : (color == 2) ? "���" : "�Ƶ�"), phaseTime);
        
	}
	else
	{
        //fprintf(stderr,"<phase%d %s phaseTime %d >  ", phase, ((color == 1) ? "�̵�" : (color == 2) ? "���" : "�Ƶ�"), phaseTime);
	}
	//return;
    if(iRecvPhase == 7)
    {
        iRecvPhase = 0;
        //return;
        CalcPhaseRunTimeAndSplit(gCountDownParams,gPhaseTurn,gGreenBlink);

        fprintf(stderr,"Phase>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d\n\n",
                                                            gCountDownParams->stPhaseRunningInfo[0][1],gCountDownParams->stPhaseRunningInfo[0][0],
                                                            gCountDownParams->stPhaseRunningInfo[1][1],gCountDownParams->stPhaseRunningInfo[1][0],
                                                            gCountDownParams->stPhaseRunningInfo[2][1],gCountDownParams->stPhaseRunningInfo[2][0],
                                                            gCountDownParams->stPhaseRunningInfo[3][1],gCountDownParams->stPhaseRunningInfo[3][0],
                                                            gCountDownParams->stPhaseRunningInfo[4][1],gCountDownParams->stPhaseRunningInfo[4][0],
                                                            gCountDownParams->stPhaseRunningInfo[5][1],gCountDownParams->stPhaseRunningInfo[5][0],
                                                            gCountDownParams->stPhaseRunningInfo[6][1],gCountDownParams->stPhaseRunningInfo[6][0],
                                                            gCountDownParams->stPhaseRunningInfo[7][1],gCountDownParams->stPhaseRunningInfo[7][0],
                                                            
                                                            gCountDownParams->ucCurRunningTime,
                                                            gCountDownParams->ucCurCycleTime);



    }

#endif    
	countdown_veh[phase].veh_color = color;
	countdown_veh[phase].veh_phaseTime = phaseTime;
	/*
	if(SetFreeGreenTimeSign == 0)
	{
		SeFreeGreenTime(3);//��Ӧ���ʱ��
		SetFreeGreenTimeSign = 1;
	}*/
}

void CountDownOutPed(unsigned char phase, unsigned char color, unsigned char phaseTime)
{
    gCountDownParams->stPedPhaseCountingDown[phase - 1][0] = color;
    gCountDownParams->stPedPhaseCountingDown[phase - 1][1] = phaseTime;
    iRecvPhase++;
    //fprintf(stderr,"CountDownOutPed <phase%d %s phaseTime %d >  ", phase, ((color == 1) ? "�̵�" : (color == 2) ? "���" : "�Ƶ�"), phaseTime);
	countdown_ped[phase].ped_color = color;
	countdown_ped[phase].ped_phaseTime = phaseTime;
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
						if(g_struCountDown.iPhaseOfChannel[iChannel].iphase == iPhase)
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
			if(((countdown_veh[iPhase].veh_phaseTime == g_struCountDown.iPulseGreenTime) && (countdown_veh[iPhase].veh_color == 1))
			||((countdown_veh[iPhase].veh_phaseTime == g_struCountDown.iPulseRedTime) && (countdown_veh[iPhase].veh_color == 2)))//��̵���ʱ
			{
				if(temp_Sign[iPhase] == 0)
				{
					for(iChannel=0; iChannel<32; iChannel++)
					{
						if(g_struCountDown.iPhaseOfChannel[iChannel].iphase== iPhase)
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

