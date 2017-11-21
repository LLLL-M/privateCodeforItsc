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
#define BIT(v, n) (((v) >> (n)) & 0x1)      //取v的第 n bit位
#endif

int g_io_fd = 0;
int g_auto_pressed = 1;
int g_manual_pressed = 0;
int g_flashing_pressed = 0;
int g_allred_pressed = 0;
int g_step_by_step_pressed = 0;

static time_t system_begin_time = 0;

//设置LED灯状态
static void set_led_status(int param)
{
	int arg = param;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
}

/*获取key按键状态
	return 0: 没有按键按下
	return 1: 有按键按下
*/
static int get_key_status(int param)
{
	int arg = param;
	(void) ioctl(g_io_fd, IO_GET_PIN_STATUS, &arg);
	return !(arg & 0x1);	//因为低电平表示按键按下，而高电平表示未按下，所以使用非值
}

//点亮GPS指示灯
void GPS_led_on()
{
	set_led_status(ARG_GPS_LED_ON);
}
//关闭GPS指示灯
void GPS_led_off()
{
	set_led_status(ARG_GPS_LED_OFF);
}
//设置ttys4发送使能
void Set_ttys4_send_enable()
{
	int arg = ARG_TTYS4_SEND_ENABLE;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
}
//写主控板运行指示灯
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

//读门开关信号，预留
unsigned short DoorCheck()
{
	return 0;
}

//读行人按钮信号
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
* 	根据实际的键盘按钮按下的状态来点灯
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
*  根据按键的状态去处理按键的指示灯
	5个参数分别为5个按键当前的状态
	返回值:
	0: 无键按下.
	1:自动键按下,且已放开.
	2:手动键按下,且已放开.
	3:黄闪键按下,且已放开.
	4:全红键按下,且已放开.
	5:步进键按下,且已放开.
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
	int ret = 0;	//默认返回0，即没有按键按下
	time_t curtime = 0;
	time(&curtime);
	
	if(g_auto_pressed == 1) {      //自动键已经按下
		//检测手动按钮是否按下
		if (1 == manualKeyStatus) {
			//检测到手动键按下
			g_auto_pressed = 0;
			g_manual_pressed = 1;			
			ret = 2;
		}
	} else if(g_manual_pressed == 1) {    //手动键已经按下
		if(autoKeyStatus == 1) {  //检测自动键是否按下
			//检测到自动键按下
			g_auto_pressed = 1;
			g_manual_pressed = 0;
			g_flashing_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 1;
		} else if(flashingKeyStatus == 1) {  //检测黄闪键是否按下
			//黄闪键已经按下
			g_flashing_pressed = 1;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			g_allred_pressed = 0;
			ret = 3;	
		} else if(allredKeyStatus == 1) {  //检测全红键是否按下
			//全红键已经按下
			g_allred_pressed = 1;
			g_flashing_pressed = 0;
			g_auto_pressed = 0;
			g_step_by_step_pressed = 0;
			ret = 4;			
		} else if(stepKeyStatus == 1 && g_flashing_pressed == 0 
				&& g_allred_pressed == 0) {	//检测步进键是否按下，步进等级较黄闪和全红低
			time(&curtime);
			//3秒内步进键无法多次触发
			if((curtime - lasttime) > 3) {
				//步进键起效
				lasttime = curtime;
				g_step_by_step_pressed = 1;
				g_allred_pressed = 0;
				g_flashing_pressed = 0;
				g_auto_pressed = 0;
				ret = 5;
			}
		}
	}

	if (ret != 0) {	//有按键按下
		ProcessKeyBoardLight();
		INFO("io.c: 键盘处理 return %d,g_manual_pressed=%d,g_auto_pressed=%d,g_flashing_pressed=%d,g_allred_pressed=%d,g_step_by_step_pressed=%d \n",
				ret,g_manual_pressed,g_auto_pressed,
				g_flashing_pressed,g_allred_pressed,g_step_by_step_pressed);
	}
	return ret;
}

/*********************************************************************************
*
*  键盘处理
	返回值:
	0: 无键按下.
	1:自动键按下,且已放开.
	2:手动键按下,且已放开.
	3:黄闪键按下,且已放开.
	4:全红键按下,且已放开.
	5:步进键按下,且已放开.
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
	//如果开机12内检测到黄闪按键按下10次，则删除配置文件并重启
	if(key_data == 4 || get_key_status(ARG_YELLOW_BLINK_KEY_STATUS) == 1) {
		time(&curtime);
		if (curtime - starttime < 12)
		{
			backup_count++;
			if (backup_count == 10)
			{
				backup_count = 0;
				//熄灭自动按钮灯并删除配置文件
				set_led_status(ARG_AUTO_LED_OFF);
				system("rm -f /home/data/*");
				//重启信号机
				//system("reboot");
				return 0;
			}
		}
	}
	
	return (key_data == 0) ? 0 : ProcessKeyByKeyStatus(key_data);
}

void IO_Init()
{
	//获取当前时间秒数
	time(&system_begin_time); 
	g_io_fd = open(DEVICE_NAME,O_RDONLY);
	if(g_io_fd == -1) {
		ERR("Open device %s error!\n",DEVICE_NAME);
		return;
	}
	INFO("IO init successful!");
}
