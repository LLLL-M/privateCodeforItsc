#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "io_ioctl.h"
#include "canmsg.h"
#include "io.h"
#include "hik.h"


#define DEVICE_NAME "/dev/gpio"
#define WIRELESS_CONTROLLER_DELAY_TIME	30

int g_io_fd = 0;
int g_auto_pressed = 1;
int g_manual_pressed = 0;
int g_flashing_pressed = 0;
int g_allred_pressed = 0;
int g_step_by_step_pressed = 0;

static time_t system_begin_time = 0;
int WriteWirelessCtrlLogInfos(const char *pFile,const char *pchMsg);

//����LED��״̬
static void set_led_status(int param)
{
	int arg = param;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
}

/*��ȡkey����״̬
	return 0: û�а�������
	return 1: �а�������
*/
static int get_key_status(int param)
{
	int arg = param;
	(void) ioctl(g_io_fd, IO_GET_PIN_STATUS, &arg);
	return !(arg & 0x1);	//��Ϊ�͵�ƽ��ʾ�������£����ߵ�ƽ��ʾδ���£�����ʹ�÷�ֵ
}

//����GPSָʾ��
void GPS_led_on()
{
	set_led_status(ARG_GPS_LED_ON);
}
//�ر�GPSָʾ��
//void GPS_led_off()
void Set_LED2_OFF()
{
	set_led_status(ARG_GPS_LED_OFF);
}

//д���ذ�����ָʾ��
void Hiktsc_Running_Status(void)
{
	static int hiktcsRunningFlag = 0;
	hiktcsRunningFlag = (~hiktcsRunningFlag) & 0x1;
	set_led_status(hiktcsRunningFlag ? ARG_SYSTEM_RUNNING_LED_ON : ARG_SYSTEM_RUNNING_LED_OFF);
}

//���������߳�
static void *yellow_blink_thread(void *param)
{
	int high = ARG_YF_CTRL_HIGH;
	int low = ARG_YF_CTRL_LOW;
	while (1) {
		ioctl(g_io_fd, IO_SET_PIN_STATUS, &high);
		usleep(10000);
		ioctl(g_io_fd, IO_SET_PIN_STATUS, &low);
		usleep(10000);
	}
	pthread_exit(NULL);
}
static void yellow_blink_init()
{
	pthread_t threadId;
	if (pthread_create(&threadId, NULL, yellow_blink_thread, NULL)) {
		perror("create yellow light thread fail");
		exit(1);
	}
	pthread_detach(threadId);
	INFO("yellow light thread init successful!");
}

//���ſ����źţ�Ԥ��
unsigned short DoorCheck()
{
	return 0;
}

//�����˰�ť�ź�
unsigned short PedestrianCheck()
{
	unsigned short ret = 0;
	int i, arg;
	for (i = 0; i < 8; i++) {
		arg = SET_ARG(XRIO_0 + i, 0);
		ioctl(g_io_fd, IO_GET_PIN_STATUS, &arg);
		ret |= (arg & 0x1) << i;
	}
	return ret;
}


/*********************************************************************************
*
* 	����ʵ�ʵļ��̰�ť���µ�״̬�����
*
***********************************************************************************/
void ProcessKeyBoardLight(void)
{
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0, sizeof(struct can_frame));
	m_frame_send.can_id = 0x110;
	m_frame_send.can_dlc = 1;
	m_frame_send.data[0] = g_auto_pressed 
							| (g_manual_pressed << 1) 
							| (g_flashing_pressed << 2) 
							| (g_allred_pressed << 3)
							| (g_step_by_step_pressed << 4);
	canits_send(&m_frame_send);
	
	set_led_status(g_auto_pressed ? ARG_AUTO_LED_ON : ARG_AUTO_LED_OFF);	
	set_led_status(g_manual_pressed ? ARG_MANUAL_LED_ON : ARG_MANUAL_LED_OFF);
	set_led_status(g_flashing_pressed ? ARG_YELLOW_BLINK_LED_ON : ARG_YELLOW_BLINK_LED_OFF);
	set_led_status(g_allred_pressed ? ARG_ALL_RED_LED_ON : ARG_ALL_RED_LED_OFF);
	set_led_status(g_step_by_step_pressed ? ARG_STEP_LED_ON : ARG_STEP_LED_OFF);
}


/*********************************************************************************
*
*  ���ݰ�����״̬ȥ��������ָʾ��
	5�������ֱ�Ϊ5��������ǰ��״̬
	����ֵ:
	0: �޼�����.
	1:�Զ�������,���ѷſ�.
	2:�ֶ�������,���ѷſ�.
	3:����������,���ѷſ�.
	4:ȫ�������,���ѷſ�.
	5:����������,���ѷſ�.
*
***********************************************************************************/

#define BIT(v, n)	((v >> n) & 0x1)
static int ProcessKeyByKeyStatus(unsigned char data)
{
	int autoKeyStatus = BIT(data, 0);
	int manualKeyStatus = BIT(data, 1);
	int flashingKeyStatus = BIT(data, 2);
	int allredKeyStatus = BIT(data, 3);
	int stepKeyStatus = BIT(data, 4);
	static time_t lasttime = 0;
	int ret = 0;	//Ĭ�Ϸ���0����û�а�������
	time_t curtime = 0;
	time(&curtime);
	
	if(g_auto_pressed == 1) {      //�Զ����Ѿ�����
		//����ֶ���ť�Ƿ���
		if (1 == manualKeyStatus) {
			//��⵽�ֶ�������
			g_auto_pressed = 0;
			g_manual_pressed = 1;			
			ret = 2;
		}
	} else if(g_manual_pressed == 1) {    //�ֶ����Ѿ�����
		if(autoKeyStatus == 1) {  //����Զ����Ƿ���
			//��⵽�Զ�������
			g_auto_pressed = 1;
			g_manual_pressed = 0;
			g_flashing_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 1;
		} else if(flashingKeyStatus == 1) {  //���������Ƿ���
			//�������Ѿ�����
			g_flashing_pressed = 1;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 3;	
		} else if(allredKeyStatus == 1) {  //���ȫ����Ƿ���
			//ȫ����Ѿ�����
			g_allred_pressed = 1;
			g_flashing_pressed = 0;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			ret = 4;			
		} else if(stepKeyStatus == 1 && g_flashing_pressed == 0 
				&& g_allred_pressed == 0) {	//��ⲽ�����Ƿ��£������ȼ��ϻ�����ȫ���
			time(&curtime);
			//3���ڲ������޷���δ���
			if((curtime - lasttime) > 3) {
				//��������Ч
				lasttime = curtime;
				g_step_by_step_pressed = 1;
				g_allred_pressed = 0;
				g_flashing_pressed = 0;
				g_auto_pressed = 0;
				ret = 5;
			}
		}
	}

	if (ret != 0) {	//�а�������
		ProcessKeyBoardLight();
		INFO("io.c: ���̴��� return %d,g_manual_pressed=%d,g_auto_pressed=%d,g_flashing_pressed=%d,g_allred_pressed=%d,g_step_by_step_pressed=%d \n",
				ret,g_manual_pressed,g_auto_pressed,
				g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);
	}
	return ret;
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
int ProcessKeyBoard()
{
	static int backup_count = 0;
	static time_t starttime = 0;
	time_t curtime = 0;
	unsigned char key_data = 0;

	if (starttime == 0)
	{
		time(&starttime);
	}
	ioctl(g_io_fd, IO_GET_BUTTON_STATUS, &key_data);
	//�������12�ڼ�⵽������������10�Σ���ɾ�������ļ�������
	if(key_data == 4 || get_key_status(ARG_YELLOW_BLINK_KEY_STATUS) == 1) {
		time(&curtime);
		if (curtime - starttime < 12)
		{
			backup_count++;
			if (backup_count == 10)
			{
				backup_count = 0;
				//Ϩ���Զ���ť�Ʋ�ɾ�������ļ�
				set_led_status(ARG_AUTO_LED_OFF);
				system("rm -f /home/data/*");
				//ȥ�������źŻ����ܣ���Ҫ�ֶ�����
				//system("reboot");
				return 0;
			}
		}
	}
	
	return (key_data == 0) ? 0 : ProcessKeyByKeyStatus(key_data);
}

void IO_Init()
{
	//��ȡ��ǰʱ������
	time(&system_begin_time); 
	g_io_fd = open(DEVICE_NAME,O_RDONLY);
	if(g_io_fd == -1) {
		ERR("Open device %s error!\n",DEVICE_NAME);
		exit(1);
	}
	
	yellow_blink_init();
	INFO("IO init successful!");
}
void WirelessKeyCheck(void)
{
	unsigned char wireless_data = 0;
	int j = 0;
	char msg[128] = "";
	static unsigned char lastkeystatus[5] = {0};
	unsigned char currentkeystatus[5] = {0};
	unsigned char keyPressed=0; 
	
	//��������30s������ң�ز���Ч
	if((time(NULL)-system_begin_time) < WIRELESS_CONTROLLER_DELAY_TIME)
	{
		return;
	}
	
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd, IO_GET_WIRELESS_STATUS, &wireless_data);
		//�������߰���״̬
		for (j = 0; j < 5; j++)
		{
			if(BIT(wireless_data, j) == 1)
			{
				currentkeystatus[j] = 1;
//				INFO("KEY %d ...", j);
			}
			else
			{
				currentkeystatus[j] = 0;
			}
			
			if((lastkeystatus[j]==1) && (currentkeystatus[j]==0))
			{
				//��⵽��ť����������Ϊ����������
				//keystatus[j] = 1;
				switch(j)
				{
					case 0:
						//wireless_auto_pressed = 1;
						sprintf(msg,"Key auto pressed!");
						break;
					case 1:
						//wireless_manual_pressed = 1;
						sprintf(msg,"Key Manual pressed!");
						break;
					case 2:
						//wireless_flashing_pressed = 1;
						sprintf(msg,"Key yellowBlink pressed!");
						break;
					case 3:
						//wireless_allred_pressed = 1;
						sprintf(msg,"Key AllRed pressed!");
						break;
					case 4:
						//wireless_step_by_step_pressed = 1;
						sprintf(msg,"Key StepByStep pressed!");
						break;
					default:
						sprintf(msg,"unknown key pressed!");
						break;
				}
				keyPressed=j+1;
				WriteWirelessCtrlLogInfos("/home/Wireless.log",msg);
				INFO("%s",msg);
			}
			lastkeystatus[j] = currentkeystatus[j];
		}
	}

	if (keyPressed != 0)
	{
		KeyControlDeal(keyPressed);
		ProcessKeyBoardLight();
		INFO("KEY %d pressed ...", keyPressed);
	}
	return;
}
/*********************************************************************************
*
*ÿ�ν�����ң�ز�����¼��ָ���ļ���
*
***********************************************************************************/
int WriteWirelessCtrlLogInfos(const char *pFile,const char *pchMsg)
{
	time_t now;
	struct tm *timenow;
    FILE *pFileDebugInfo = NULL;
	struct stat f_stat;

	if(pchMsg == NULL)
	{
		printf("����ң�ر����ļ���Ч!\n");
		return -1;
	}
    pFileDebugInfo = fopen(pFile, "a");
    if(pFileDebugInfo == NULL)
    {
    	printf("����ң�ؼ�¼�ļ�δ����ȷ��!\n");
        return -1;
	}
	
	if( stat( pFile, &f_stat ) == -1 )
	{
        fclose(pFileDebugInfo);
		printf("��ȡ����ң�ؿ��Ƽ�¼�ļ���Ϣʧ��!\n");
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
		timenow->tm_hour,timenow->tm_min,timenow->tm_sec,pchMsg);

	fclose(pFileDebugInfo);
	return 0;
}
char keyCheck(int key)
{
	char ret=0;
	time_t curtime = 0;
	static time_t lasttime = 0;

	if(key > 5 || key < 1)
	{
		ERR("unknown key: %d", key);
		return 0;
	}

	if(g_auto_pressed == 1)//�Զ����Ѿ�����
	{
		//����ֶ���ť�Ƿ���
		if (2 == key)
		{
			//��⵽�ֶ�������	
			g_auto_pressed = 0; 
			g_manual_pressed = 1;	
			ret = 2; 		
			//sprintf(msg,"����ң���ֶ�����������Ч");
		}		
	}
	else if(g_manual_pressed == 1)	 //�ֶ����Ѿ�����
	{ 
		if(1 == key)
		{
			g_auto_pressed = 1;
			g_manual_pressed = 0;
			g_flashing_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 1;
			//sprintf(msg,"����ң���Զ�����������Ч");
		} 
		else if(3 == key)
		{
			g_flashing_pressed = 1;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 3; 
			//sprintf(msg,"����ң�ػ�������������Ч");
		}
		else if(4 == key)
		{
			g_allred_pressed = 1;
			g_flashing_pressed = 0;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			ret = 4;
			//sprintf(msg,"����ң��ȫ�찴��������Ч");
		}
		else if(5==key && g_flashing_pressed == 0 && g_allred_pressed == 0)
		{
			time(&curtime);
			if((curtime - lasttime) > 3)
			{
				lasttime = curtime;
				g_step_by_step_pressed = 1;
				g_allred_pressed = 0;
				g_flashing_pressed = 0;
				g_auto_pressed = 0;
				ret = 5;
				//sprintf(msg,"����ң�ز�������������Ч");
			}
		}
	}
	return ret;
}
