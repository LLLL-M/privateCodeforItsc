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

#define DEVICE_NAME "/dev/CPLD_IO"

//黄闪输出
#define YELLOW_CONTROL_OUTPUT1_1      0x1001
#define YELLOW_CONTROL_OUTPUT2_1      0x1002

#define YELLOW_CONTROL_OUTPUT1_0      0x0001
#define YELLOW_CONTROL_OUTPUT2_0      0x0002

//5个LED输出
#define LED_OUTPUT1_1             	  0x1010
#define LED_OUTPUT2_1             	  0x1011
#define LED_OUTPUT3_1             	  0x1012
#define LED_OUTPUT4_1             	  0x1013
#define LED_OUTPUT5_1             	  0x1014

#define LED_OUTPUT1_0             	  0x0010
#define LED_OUTPUT2_0             	  0x0011
#define LED_OUTPUT3_0             	  0x0012
#define LED_OUTPUT4_0             	  0x0013
#define LED_OUTPUT5_0             	  0x0014

//20个IO输出
#define IO_OUTPUT1_1              	  0x1020
#define IO_OUTPUT2_1              	  0x1021
#define IO_OUTPUT3_1              	  0x1022
#define IO_OUTPUT4_1              	  0x1023
#define IO_OUTPUT5_1              	  0x1024
#define IO_OUTPUT6_1              	  0x1025
#define IO_OUTPUT7_1              	  0x1026
#define IO_OUTPUT8_1              	  0x1027
#define IO_OUTPUT9_1              	  0x1028
#define IO_OUTPUT10_1             	  0x1029
#define IO_OUTPUT11_1             	  0x102a
#define IO_OUTPUT12_1             	  0x102b
#define IO_OUTPUT13_1             	  0x102c
#define IO_OUTPUT14_1             	  0x102d
#define IO_OUTPUT15_1             	  0x102e
#define IO_OUTPUT16_1             	  0x1030
#define IO_OUTPUT17_1             	  0x1031
#define IO_OUTPUT18_1             	  0x1032
#define IO_OUTPUT19_1             	  0x1033
#define IO_OUTPUT20_1             	  0x1034

#define IO_OUTPUT1_0              	  0x0020
#define IO_OUTPUT2_0              	  0x0021
#define IO_OUTPUT3_0              	  0x0022
#define IO_OUTPUT4_0              	  0x0023
#define IO_OUTPUT5_0              	  0x0024
#define IO_OUTPUT6_0              	  0x0025
#define IO_OUTPUT7_0              	  0x0026
#define IO_OUTPUT8_0             	  0x0027
#define IO_OUTPUT9_0              	  0x0028
#define IO_OUTPUT10_0             	  0x0029
#define IO_OUTPUT11_0             	  0x002a
#define IO_OUTPUT12_0             	  0x002b
#define IO_OUTPUT13_0             	  0x002c
#define IO_OUTPUT14_0             	  0x002d
#define IO_OUTPUT15_0             	  0x002e
#define IO_OUTPUT16_0             	  0x0030
#define IO_OUTPUT17_0             	  0x0031
#define IO_OUTPUT18_0             	  0x0032
#define IO_OUTPUT19_0             	  0x0033
#define IO_OUTPUT20_0             	  0x0034

//32个IO输入
#define IO_INPUT1_TO_INPUT8     	  0x2040  
#define IO_INPUT9_TO_INPUT16    	  0x2041 
#define IO_INPUT17_TO_INPUT24   	  0x2042 
#define IO_INPUT25_TO_INPUT32   	  0x2043 

//5个键盘板按键接收
#define KEYBOARD_INPUT1         	  0x2110
#define KEYBOARD_INPUT2         	  0x2111
#define KEYBOARD_INPUT3         	  0x2112
#define KEYBOARD_INPUT4         	  0x2113
#define KEYBOARD_INPUT5         	  0x2114

//5个键盘板按键等点亮
#define KEYBOARD_OUTPUT1_1        	  0x1100
#define KEYBOARD_OUTPUT2_1        	  0x1101
#define KEYBOARD_OUTPUT3_1        	  0x1102
#define KEYBOARD_OUTPUT4_1        	  0x1103
#define KEYBOARD_OUTPUT5_1        	  0x1104

#define KEYBOARD_OUTPUT1_0        	  0x0100
#define KEYBOARD_OUTPUT2_0        	  0x0101
#define KEYBOARD_OUTPUT3_0        	  0x0102
#define KEYBOARD_OUTPUT4_0        	  0x0103
#define KEYBOARD_OUTPUT5_0        	  0x0104

int fd = 0;
int auto_pressed = 1;
int manual_pressed = 0;
int flashing_pressed = 0;
int allred_pressed = 0;
int step_by_step_pressed = 0;
static int HiktcsRunningFlag = 0;
time_t lasttime = 0;
time_t system_begin_time = 0;
int backup_count = 0;

/*********************************************************************************
*
* 	CPU与CPLD交互初始化
*
***********************************************************************************/
void CPLD_IO_Init()
{
	//获取当前时间秒数
	time(&system_begin_time); 
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
}

/*********************************************************************************
*
* 	熄灭GPS指示灯
*
***********************************************************************************/
void Set_LED2_OFF()
{
	if(fd != -1)
	{
		//熄灭GPS指示灯
		ioctl(fd,LED_OUTPUT2_0);
	}
}


			

/*********************************************************************************
*
* 	CPU控制CPLD发送黄闪控制信号给电源板。
*   CPLD如果检测到高低高低脉冲，则输出高电平；如果没有检测到，则输出低电平。
*
***********************************************************************************/
void HardflashDogCtrl(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//发送高电平
			ioctl(fd,YELLOW_CONTROL_OUTPUT1_1);
			ioctl(fd,YELLOW_CONTROL_OUTPUT2_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//发送低电平
			ioctl(fd,YELLOW_CONTROL_OUTPUT1_0);
			ioctl(fd,YELLOW_CONTROL_OUTPUT2_0);
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
		if(fd != -1)
		{
			ioctl(fd,LED_OUTPUT1_1);
		}
	}
	else
	{
		HiktcsRunningFlag = 1;
		if(fd != -1)
		{
			ioctl(fd,LED_OUTPUT1_0);
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
		if(fd != -1)
		{
			//点灯
			ioctl(fd,KEYBOARD_OUTPUT1_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//灭灯
			ioctl(fd,KEYBOARD_OUTPUT1_0);
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
		if(fd != -1)
		{
			//点灯
			ioctl(fd,KEYBOARD_OUTPUT2_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//灭灯
			ioctl(fd,KEYBOARD_OUTPUT2_0);
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
		if(fd != -1)
		{
			//点灯
			ioctl(fd,KEYBOARD_OUTPUT3_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//灭灯
			ioctl(fd,KEYBOARD_OUTPUT3_0);
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
		if(fd != -1)
		{
			//点灯
			ioctl(fd,KEYBOARD_OUTPUT4_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//灭灯
			ioctl(fd,KEYBOARD_OUTPUT4_0);
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
		if(fd != -1)
		{
			//点灯
			ioctl(fd,KEYBOARD_OUTPUT5_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//灭灯
			ioctl(fd,KEYBOARD_OUTPUT5_0);
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
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT1,&arg);
		//printf("Get keyboard 1 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			return 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			return 1;
		}
		else
		{
			//异常状态
			return -1;
		}
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
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT2,&arg);
		//printf("Get keyboard 2 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			return 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			return 1;
		}
		else
		{
			//异常状态
			return -1;
		}
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
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT3,&arg);
		//printf("Get keyboard 3 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			return 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			return 1;
		}
		else
		{
			//异常状态
			return -1;
		}
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
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT4,&arg);
		//printf("Get keyboard 4 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			return 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			return 1;
		}
		else
		{
			//异常状态
			return -1;
		}
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
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT5,&arg);
		//printf("Get keyboard 5 value =%d\n",arg);
		if(arg == 1)
		{
			//该键没有按下
			return 0;
		}
		else if(arg == 0)
		{
			//该键已经按下
			return 1;
		}
		else
		{
			//异常状态
			return -1;
		}
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
			system("reboot");
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
				return 2;
			}
			else if(tmpint == 0)
			{
				//没有检测到手动键按下
				return 0;
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
				return 1;
			}
			else if(GetFlashingKeyStatus() == 1)   //检测黄闪键是否按下
			{
				//黄闪键已经按下
				flashing_pressed = 1;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				return 3;	
			}
			else if(GetAllRedKeyStatus() == 1)   //检测全红键是否按下
			{
				//全红键已经按下
				allred_pressed = 1;
				flashing_pressed = 0;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				ProcessKeyBoardLight();
				return 4;			
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
					return 5;
				}
			}
		}
	}
	return 0;
}

/*********************************************************************************
*
* 	读行人按钮信号。
*
***********************************************************************************/
int PedestrianCheck()
{
	int tmpint = -1;
	if(fd != -1)
	{
		ioctl(fd,IO_INPUT1_TO_INPUT8,&tmpint);
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
	if(fd != -1)
	{
		ioctl(fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//最低位取反
		tmpint = ((tmpint ^ 0xFFFF)&0x01);
		return tmpint;
	}
	else
	{
		return 0;
	}
}

