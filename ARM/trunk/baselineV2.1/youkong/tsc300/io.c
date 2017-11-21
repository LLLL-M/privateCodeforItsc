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

#ifndef BIT
#define BIT(v, n) (((v) >> (n)) & 0x1)      //ȡv�ĵ� n bitλ
#endif

int g_io_fd = 0;
int g_auto_pressed = 1;
int g_manual_pressed = 0;
int g_flashing_pressed = 0;
int g_allred_pressed = 0;
int g_step_by_step_pressed = 0;

static time_t system_begin_time = 0;

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
void GPS_led_off()
{
	set_led_status(ARG_GPS_LED_OFF);
}
//����ttys4����ʹ��
void Set_ttys4_send_enable()
{
	int arg = ARG_TTYS4_SEND_ENABLE;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
}
//д���ذ�����ָʾ��
void Hiktsc_Running_Status(void)
{
	static int hiktcsRunningFlag = 0;
	hiktcsRunningFlag = (~hiktcsRunningFlag) & 0x1;
	set_led_status(hiktcsRunningFlag ? ARG_SYSTEM_RUNNING_LED_ON : ARG_SYSTEM_RUNNING_LED_OFF);
}

void HardflashDogCtrl()
{
	static char hardFlashFlag = 0;
	int value;
	hardFlashFlag = (~hardFlashFlag) & 0x1;
	value = (hardFlashFlag == 1) ? ARG_YF_CTRL_HIGH : ARG_YF_CTRL_LOW;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &value);
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
				//�����źŻ�
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
		return;
	}
	INFO("IO init successful!");
}
