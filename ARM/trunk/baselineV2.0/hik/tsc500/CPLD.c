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
#include <unistd.h>
#include <linux/ioctl.h>
#include "CPLD.h"
#include "hik.h"
#include "its.h"
#include "LogSystem.h"
#include "configureManagement.h"


#ifndef BIT
#define BIT(v, n) (((v) >> (n)) & 0x1)      //取v的第 n bit位
#endif

int g_io_fd = 0;
int auto_pressed = 1;
int manual_pressed = 0;
int flashing_pressed = 0;
int allred_pressed = 0;
int step_by_step_pressed = 0;
static int HiktcsRunningFlag = 0;
time_t lasttime = 0;
time_t system_begin_time = 0;
int backup_count = 0;

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;


//描述无线按键状态
static int wireless_auto_pressed = 1;
static int wireless_manual_pressed = 0;
static int wireless_flashing_pressed = 0;
static int wireless_allred_pressed = 0;
static int wireless_step_by_step_pressed = 0;

/*********************************************************************************
*
* 	CPU与CPLD交互初始化
*
***********************************************************************************/
void CPLD_IO_Init()
{
	//获取当前时间秒数
	time(&system_begin_time); 
	g_io_fd = open(DEVICE_NAME,O_RDONLY);
	if(g_io_fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
}

/*********************************************************************************
*
* 	熄灭GPS指示灯
*
***********************************************************************************/
void GPS_led_off()
{
	if(g_io_fd != -1)
	{
		//熄灭GPS指示灯
		ioctl(g_io_fd,LED_OUTPUT2_0);
	}
}
/*********************************************************************************
*
* 	点亮GPS指示灯
*
***********************************************************************************/
void GPS_led_on()
{
	if(g_io_fd != -1)
	{
		//点亮GPS指示灯
		ioctl(g_io_fd,LED_OUTPUT2_1);
	}
}

void Set_ttys4_send_enable()
{
    int arg = 0;
    ioctl(g_io_fd,TTYS4_SEND_ENABLE,&arg);
}
			

/*********************************************************************************
*
* 	CPU控制CPLD发送黄闪控制信号给电源板。
*   CPLD如果检测到高低高低脉冲，则输出高电平；如果没有检测到，则输出低电平。
*
***********************************************************************************/
void HardflashDogCtrl()
{
	static char hardFlashFlag = 0;
	hardFlashFlag = (~hardFlashFlag) & 0x1;
	if(hardFlashFlag == 1)
	{
		if(g_io_fd != -1)
		{
			//发送高电平
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT1_1);
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT2_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//发送低电平
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT1_0);
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
*	写主控板运行指示灯。
*
***********************************************************************************/

void Hiktsc_Running_Status(void)
{

	if (HiktcsRunningFlag == 1)
	{
		HiktcsRunningFlag = 0;
		if(g_io_fd != -1)
		{
			ioctl(g_io_fd,LED_OUTPUT1_1);
		}
	}
	else
	{
		HiktcsRunningFlag = 1;
		if(g_io_fd != -1)
		{
			ioctl(g_io_fd,LED_OUTPUT1_0);
		}
	}
	return;
}

/*********************************************************************************
*
* 	点亮或者熄灭步进灯
*
***********************************************************************************/
void SetStepbyStepLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//点灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT1_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//灭灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT1_0);
		}
	}
}

/*********************************************************************************
*
* 	点亮或者熄灭黄闪灯
*
***********************************************************************************/
void SetFlashingLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//点灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT2_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//灭灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
* 	点亮或者熄灭自动灯
*
***********************************************************************************/
void SetAutoLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//点灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT3_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//灭灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT3_0);
		}
	}
}

/*********************************************************************************
*
* 	点亮或者熄灭手动灯
*
***********************************************************************************/
void SetManualLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//点灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT4_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//灭灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT4_0);
		}
	}
}

/*********************************************************************************
*
* 	点亮或者熄灭全红灯
*
***********************************************************************************/
void SetAllRedLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//点灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT5_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//灭灯
			ioctl(g_io_fd,KEYBOARD_OUTPUT5_0);
		}
	}
}

/*********************************************************************************
*
* 	获取步进按钮是否按下的状态，返回1表示按下，返回0表示未按下，返回-1表示异常
*
***********************************************************************************/
int GetStepByStepKeyStatus()
{
	int arg = -1;
	int ret = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,KEYBOARD_INPUT1,&arg);
		//printf("Get keyboard 1 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			ret = 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			ret = 1;
		}
		else
		{
			//异常状态
			ret = -1;
		}
		if (ret != 1 && wireless_step_by_step_pressed == 1)
			ret = 1;
		return ret;
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	获取黄闪按钮是否按下的状态，返回1表示按下，返回0表示未按下，返回-1表示异常
*
***********************************************************************************/
int GetFlashingKeyStatus()
{
	int arg = -1;
	int ret = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,KEYBOARD_INPUT2,&arg);
		//printf("Get keyboard 2 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			ret = 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			ret = 1;
		}
		else
		{
			//异常状态
			ret = -1;
		}
		if (ret != 1 && wireless_flashing_pressed == 1)
			ret = 1;
		return ret;
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	获取自动按钮是否按下的状态，返回1表示按下，返回0表示未按下，返回-1表示异常
*
***********************************************************************************/
int GetAutoKeyStatus()
{
	int arg = -1;
	int ret = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,KEYBOARD_INPUT3,&arg);
		//printf("Get keyboard 3 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			ret = 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			ret = 1;
		}
		else
		{
			//异常状态
			ret = -1;
		}
		if (ret != 1 && wireless_auto_pressed == 1)
			ret = 1;
		return ret;
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	获取手动按钮是否按下的状态，返回1表示按下，返回0表示未按下，返回-1表示异常
*
***********************************************************************************/
int GetManualKeyStatus()
{
	int arg = -1;
	int ret = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,KEYBOARD_INPUT4,&arg);
		//printf("Get keyboard 4 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			ret = 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			ret = 1;
		}
		else
		{
			//异常状态
			ret = -1;
		}
		if (ret != 1 && wireless_manual_pressed == 1)
			ret = 1;
		return ret;
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	获取全红按钮是否按下的状态，返回1表示按下，返回0表示未按下，返回-1表示异常
*
***********************************************************************************/
int GetAllRedKeyStatus()
{
	int arg = -1;
	int ret = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,KEYBOARD_INPUT5,&arg);
		//printf("Get keyboard 5 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			ret = 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			ret = 1;
		}
		else
		{
			//异常状态
			ret = -1;
		}
		if (ret != 1 && wireless_allred_pressed == 1)
			ret = 1;
		return ret;
	}
	else
	{
		return -1;
	}
}


/*********************************************************************************
*
* 	根据实际的键盘按钮按下的状态来点灯
*
***********************************************************************************/
void ProcessKeyBoardLight(void)
{
	
	SetStepbyStepLight(step_by_step_pressed);
	SetFlashingLight(flashing_pressed);
	SetAutoLight(auto_pressed);
	SetManualLight(manual_pressed);
	SetAllRedLight(allred_pressed);
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
	int tmpint = -1;
	int ret = 0;
	time_t curtime = 0;
	time(&curtime);
	//系统起来后12s内如果检测到黄闪按钮按下4秒以上时间，则删除配置文件并重启
	if(curtime - system_begin_time < 12)
	{
		if(GetFlashingKeyStatus() == 1)
		{
			//黄闪键按下，复位计数加1
			backup_count ++;
		}
		//达到键盘板计数阈值
		if(backup_count >= 10)
		{
			//熄灭自动按钮灯并删除配置文件
			SetAutoLight(0);
			system("rm -f /home/data/*");
			//重启信号机
			//system("reboot");
		}
	}
	else
	{
		if(auto_pressed == 1)       //自动键已经按下
		{
			//检测手动按钮是否按下
			tmpint = GetManualKeyStatus();
			if(tmpint == 1)
			{
				//检测到手动键按下
				auto_pressed = 0;
				manual_pressed = 1;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[Manual] pressed!");
				ret = 2;
			}
			else if(tmpint == 0)
			{
				//没有检测到手动键按下
				ret = 0;
			}	
		}
		else if(manual_pressed == 1)     //手动键已经按下
		{
			if(GetAutoKeyStatus() == 1)   //检测自动键是否按下
			{
				//检测到自动键按下
				auto_pressed = 1;
				manual_pressed = 0;
				flashing_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[Auto] pressed!");
				ret = 1;
			}
			else if(GetFlashingKeyStatus() == 1)   //检测黄闪键是否按下
			{
				//黄闪键已经按下
				flashing_pressed = 1;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[YellowBlink] pressed!");
				ret = 3;	
			}
			else if(GetAllRedKeyStatus() == 1)   //检测全红键是否按下
			{
				//全红键已经按下
				allred_pressed = 1;
				flashing_pressed = 0;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[AllRed] pressed!");
				ret = 4;			
			}
			else if((GetStepByStepKeyStatus() == 1) && flashing_pressed == 0 
				&& allred_pressed == 0)				//检测步进键是否按下，步进等级较黄闪和全红低
			{
				time(&curtime);
				//3秒内步进键无法多次触发
				if((curtime - lasttime) > 3)
				{
					//步进键起效
					lasttime = curtime;
					step_by_step_pressed = 1;
					allred_pressed = 0;
					flashing_pressed = 0;
					auto_pressed = 0;
					ProcessKeyBoardLight();
					log_debug("Keyboard: key[StepByStep] pressed!");
					ret = 5;
				}
			}
		}
	}
	wireless_auto_pressed = 0;
	wireless_manual_pressed = 0;
	wireless_flashing_pressed = 0;
	wireless_allred_pressed = 0;
	wireless_step_by_step_pressed = 0;
	return ret;
}

/*********************************************************************************
*
* 	读行人按钮信号。
*
***********************************************************************************/
int PedestrianCheck()
{
	int tmpint = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT1_TO_INPUT8,&tmpint);
		//低8位取反
		tmpint = ((tmpint ^ 0xFFFF)&0xFF);
		return tmpint;
	}
	else
	{
		return 0;
	}
}


/*********************************************************************************
*
* 	读门开关信号。
*
***********************************************************************************/
int DoorCheck()
{
	int tmpint = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//最低位取反
		//处理无线按键状态
		if (BIT(tmpint, 1) == 0)
			wireless_allred_pressed = 1;
		else if (BIT(tmpint, 2) == 0)
			wireless_flashing_pressed = 1;
		else if (BIT(tmpint, 3) == 0)
			wireless_step_by_step_pressed = 1;
		else if (BIT(tmpint, 4) == 0)
			wireless_auto_pressed = 1;
		else if (BIT(tmpint, 5) == 0)
			wireless_manual_pressed = 1;
		tmpint = ((tmpint ^ 0xFFFF)&0x01);
		return tmpint;
	}
	else
	{
		return 0;
	}
}

/*********************************************************************************
*
* 	无线遥控按键检测
*
***********************************************************************************/

static unsigned char WirelessKeyCheck(void)
{
	unsigned char key=0;
	FaultLogType type=0;
	char *desc[]={"Auto","Manual","YellowBlink","AllRed","StepByStep"};
	int tmpint = -1;
	static int lastkeystatus[5] = {1,1,1,1,1};  //上一次获取到5个按键状态
	int curkeystatus[5] = {1,1,1,1,1};       //当前获取到的5个按键状态
	int i = 0;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//处理无线按键状态
		for(i = 0; i < 5; i++)
		{
			curkeystatus[i] = BIT(tmpint, i);   //获取5个按键键值，1表示未触发，0表示触发
			if(curkeystatus[i] == 1 && lastkeystatus[i] == 0)    //按键松开时触发控制信号(上升沿表示触发)
			{
				switch(i)
				{
					case 0:   //全红按钮按下
						key = 4;
						type = WIRELESS_KEY_ALLRED;
						break;
					case 1:   //黄闪按钮按下
						key = 3;
						type = WIRELESS_KEY_YELLOWBLINK;
						break;
					case 2:   //步进按钮按下
						key = 5;
						type = WIRELESS_KEY_STEP;
						break;
					case 3:   //自动按钮按下
						key = 1;
						type = WIRELESS_KEY_AUTO;
						break;
					case 4:   //手动按钮按下
						key = 2;
						type = WIRELESS_KEY_MANUAL;
						break;
					default:
						break;
				}		
			}
			lastkeystatus[i] = curkeystatus[i];       //更新上一次按键键值
		}	
	}
	if(key != 0 && key < 6)
	{
		INFO("WirelssController: Key %d[%s] pressed!", key, desc[key-1]);
		ItsWriteFaultLog(type, key);
		log_debug("WirelssController: Key %d[%s] pressed!", key, desc[key-1]);
	}
	return key;	
}
/**********************************************************************************************
**	desc:				to check if the key pressed should be handled as a effecitve control, rules as follow:
						1. for special control include YellowBlink,AllRed and StepByStep, you must press key manual first.
						2. when you pressed key YellowBlink (or AllRed), you can't switch to step controll by Pressing Key
							StepByStep directly.The right way is to press key auto first, then key manual, key StepByStep
							in order.But when you are in StepByStep control mode, you can switch to YellowBlink(or AllRed)
							mode by simply pressing key YellowBlink.
						3. In any special control mode, you can return to auto by pressing the key.
**	curPressed:  			the key pressed now
**	lastPressed: 			the effective key(auto-step)  pressed last time
**	manualOvertimeFlag: 	if the state return to auto after manual key pressed more than 10s with no key pressed ever.
						1 means yes, 0 means no
** 	return value:			0-13(KEY_INVALID - L_SOUTH_NORTH)
**	author:				Kevin Pan
*********************************************************************************************/
static eKeyStatus keyCheck(eKeyStatus curPressed, int *lastPressed, char manualOvertimeFlag)
{
	eKeyStatus ret=V_KEY_INVALID;
	//unsigned char led=0;
	time_t curtime=0;
	static time_t lasttime=0;

	//params illegal
	if(lastPressed == NULL)
	{
		ERR("No key status input!");
		return V_KEY_INVALID;
	}
	//params out of range
	if(curPressed < V_KEY_INVALID || curPressed > V_KEY_MAX)
		return V_KEY_INVALID;

	time(&curtime);
	//same key pressed in 3s will be considered as wrong trigger
	if((curPressed == *lastPressed) && ((curtime - lasttime) <= WIRELESS_WRONG_TRIGGER_TIME))
	{
		//INFO("keyCheck: cur:%u, last:%u...", (UINT32)curtime, (UINT32)lasttime);
		return V_KEY_INVALID;
	}

	if(curPressed == V_KEY_INVALID)
	{
		if(manualOvertimeFlag == 1 && *lastPressed == V_KEY_MANUAL && ((curtime - lasttime) >= WIRELESS_MANUAL_WAITING_TIME))
		{
			INFO("ManualControl: Manualkey Overtime, Return to Autokey...");
			log_debug("ManualControl: Manualkey Overtime, Return to Autokey...");
			*lastPressed = V_KEY_AUTO;
		}
		return V_KEY_INVALID;
	}

	//INFO("effect: cur:%u, last:%u...", (UINT32)curtime, (UINT32)lasttime);
	lasttime = curtime;
	switch(*lastPressed)
	{
		case V_KEY_AUTO:
			if(curPressed == V_KEY_MANUAL)
			{
				*lastPressed = V_KEY_MANUAL;
				//led = V_KEY_MANUAL;
			}
			break;
		case V_KEY_MANUAL:
			if(curPressed != V_KEY_MANUAL)
			{
				ret = curPressed;
				//led = curPressed;
				*lastPressed = curPressed;
			}			
			break;
		case V_KEY_STEP:
			if(curPressed != V_KEY_MANUAL)
			{
				ret = curPressed;
				//led = curPressed;
				*lastPressed = curPressed;
			}
			break;
		case V_KEY_YELLOWBLINK:
		case V_KEY_ALLRED:
		case V_KEY_EAST:
		case V_KEY_SOUTH:
		case V_KEY_WEST:
		case V_KEY_NORTH:
		case V_D_EAST_WEST:
		case V_D_SOUTH_NORTH:
		case V_L_EAST_WEST:
		case V_L_SOUTH_NORTH:
			if(curPressed != *lastPressed && curPressed != V_KEY_MANUAL && curPressed != V_KEY_STEP)
			{
				ret = curPressed;
				//led = curPressed;
				*lastPressed = curPressed;
			}
			break;
		default:
			break;

	}
	//KeyBoardSetLed(led);
	return ret;
}
static unsigned char gWirelessAutoFlag=0;
int lastKeyPressed = V_KEY_AUTO;

void WirelessSetAuto(void)
{
	gWirelessAutoFlag = 1;
}
/********************************************************
**	desc: 		Get the state of wireless key.
** 	checkFlag:	if the key needs to be checked by inner rules befroe use.
				1-check, 0-no check
** 	return value:	0-13(KEY_INVALID - L_SOUTH_NORTH)
**	author:		Kevin Pan
*********************************************************/
unsigned char GetWirelessKeyStatus(unsigned char checkFlag)
{
	unsigned char ret=0;
	if(checkFlag)
		ret = keyCheck(WirelessKeyCheck(), &lastKeyPressed, 1);
	else
	{
		ret = (gWirelessAutoFlag == 0)?WirelessKeyCheck() : 2;
		if(gWirelessAutoFlag != 0)
			gWirelessAutoFlag = 0;
	}
	return ret;
}

/********************for new keyboard*************************/
unsigned char GetKeyBoardStatus(void)
{
	return 0;
}
void KeyBoardSetKey(unsigned char keyv)
{
}
