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
#define BIT(v, n) (((v) >> (n)) & 0x1)      //ȡv�ĵ� n bitλ
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


//�������߰���״̬
static int wireless_auto_pressed = 1;
static int wireless_manual_pressed = 0;
static int wireless_flashing_pressed = 0;
static int wireless_allred_pressed = 0;
static int wireless_step_by_step_pressed = 0;

/*********************************************************************************
*
* 	CPU��CPLD������ʼ��
*
***********************************************************************************/
void CPLD_IO_Init()
{
	//��ȡ��ǰʱ������
	time(&system_begin_time); 
	g_io_fd = open(DEVICE_NAME,O_RDONLY);
	if(g_io_fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
}

/*********************************************************************************
*
* 	Ϩ��GPSָʾ��
*
***********************************************************************************/
void GPS_led_off()
{
	if(g_io_fd != -1)
	{
		//Ϩ��GPSָʾ��
		ioctl(g_io_fd,LED_OUTPUT2_0);
	}
}
/*********************************************************************************
*
* 	����GPSָʾ��
*
***********************************************************************************/
void GPS_led_on()
{
	if(g_io_fd != -1)
	{
		//����GPSָʾ��
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
* 	CPU����CPLD���ͻ��������źŸ���Դ�塣
*   CPLD�����⵽�ߵ͸ߵ����壬������ߵ�ƽ�����û�м�⵽��������͵�ƽ��
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
			//���͸ߵ�ƽ
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT1_1);
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT2_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���͵͵�ƽ
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT1_0);
			ioctl(g_io_fd,YELLOW_CONTROL_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
*	д���ذ�����ָʾ�ơ�
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
* 	��������Ϩ�𲽽���
*
***********************************************************************************/
void SetStepbyStepLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT1_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT1_0);
		}
	}
}

/*********************************************************************************
*
* 	��������Ϩ�������
*
***********************************************************************************/
void SetFlashingLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT2_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
* 	��������Ϩ���Զ���
*
***********************************************************************************/
void SetAutoLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT3_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT3_0);
		}
	}
}

/*********************************************************************************
*
* 	��������Ϩ���ֶ���
*
***********************************************************************************/
void SetManualLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT4_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT4_0);
		}
	}
}

/*********************************************************************************
*
* 	��������Ϩ��ȫ���
*
***********************************************************************************/
void SetAllRedLight(int value)
{
	if(value == 1)
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT5_1);
		}
	}
	else
	{
		if(g_io_fd != -1)
		{
			//���
			ioctl(g_io_fd,KEYBOARD_OUTPUT5_0);
		}
	}
}

/*********************************************************************************
*
* 	��ȡ������ť�Ƿ��µ�״̬������1��ʾ���£�����0��ʾδ���£�����-1��ʾ�쳣
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
			//�ü�û�а���
			ret = 0;
		}
		else if(arg == 0)
		{
			//�ü��Ѿ�����
			ret = 1;
		}
		else
		{
			//�쳣״̬
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
* 	��ȡ������ť�Ƿ��µ�״̬������1��ʾ���£�����0��ʾδ���£�����-1��ʾ�쳣
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
			//�ü�û�а���
			ret = 0;
		}
		else if(arg == 0)
		{
			//�ü��Ѿ�����
			ret = 1;
		}
		else
		{
			//�쳣״̬
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
* 	��ȡ�Զ���ť�Ƿ��µ�״̬������1��ʾ���£�����0��ʾδ���£�����-1��ʾ�쳣
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
			//�ü�û�а���
			ret = 0;
		}
		else if(arg == 0)
		{
			//�ü��Ѿ�����
			ret = 1;
		}
		else
		{
			//�쳣״̬
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
* 	��ȡ�ֶ���ť�Ƿ��µ�״̬������1��ʾ���£�����0��ʾδ���£�����-1��ʾ�쳣
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
			//�ü�û�а���
			ret = 0;
		}
		else if(arg == 0)
		{
			//�ü��Ѿ�����
			ret = 1;
		}
		else
		{
			//�쳣״̬
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
* 	��ȡȫ�찴ť�Ƿ��µ�״̬������1��ʾ���£�����0��ʾδ���£�����-1��ʾ�쳣
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
			//�ü�û�а���
			ret = 0;
		}
		else if(arg == 0)
		{
			//�ü��Ѿ�����
			ret = 1;
		}
		else
		{
			//�쳣״̬
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
* 	����ʵ�ʵļ��̰�ť���µ�״̬�����
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
	int tmpint = -1;
	int ret = 0;
	time_t curtime = 0;
	time(&curtime);
	//ϵͳ������12s�������⵽������ť����4������ʱ�䣬��ɾ�������ļ�������
	if(curtime - system_begin_time < 12)
	{
		if(GetFlashingKeyStatus() == 1)
		{
			//���������£���λ������1
			backup_count ++;
		}
		//�ﵽ���̰������ֵ
		if(backup_count >= 10)
		{
			//Ϩ���Զ���ť�Ʋ�ɾ�������ļ�
			SetAutoLight(0);
			system("rm -f /home/data/*");
			//�����źŻ�
			//system("reboot");
		}
	}
	else
	{
		if(auto_pressed == 1)       //�Զ����Ѿ�����
		{
			//����ֶ���ť�Ƿ���
			tmpint = GetManualKeyStatus();
			if(tmpint == 1)
			{
				//��⵽�ֶ�������
				auto_pressed = 0;
				manual_pressed = 1;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[Manual] pressed!");
				ret = 2;
			}
			else if(tmpint == 0)
			{
				//û�м�⵽�ֶ�������
				ret = 0;
			}	
		}
		else if(manual_pressed == 1)     //�ֶ����Ѿ�����
		{
			if(GetAutoKeyStatus() == 1)   //����Զ����Ƿ���
			{
				//��⵽�Զ�������
				auto_pressed = 1;
				manual_pressed = 0;
				flashing_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[Auto] pressed!");
				ret = 1;
			}
			else if(GetFlashingKeyStatus() == 1)   //���������Ƿ���
			{
				//�������Ѿ�����
				flashing_pressed = 1;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[YellowBlink] pressed!");
				ret = 3;	
			}
			else if(GetAllRedKeyStatus() == 1)   //���ȫ����Ƿ���
			{
				//ȫ����Ѿ�����
				allred_pressed = 1;
				flashing_pressed = 0;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				ProcessKeyBoardLight();
				log_debug("Keyboard: key[AllRed] pressed!");
				ret = 4;			
			}
			else if((GetStepByStepKeyStatus() == 1) && flashing_pressed == 0 
				&& allred_pressed == 0)				//��ⲽ�����Ƿ��£������ȼ��ϻ�����ȫ���
			{
				time(&curtime);
				//3���ڲ������޷���δ���
				if((curtime - lasttime) > 3)
				{
					//��������Ч
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
* 	�����˰�ť�źš�
*
***********************************************************************************/
int PedestrianCheck()
{
	int tmpint = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT1_TO_INPUT8,&tmpint);
		//��8λȡ��
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
* 	���ſ����źš�
*
***********************************************************************************/
int DoorCheck()
{
	int tmpint = -1;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//���λȡ��
		//�������߰���״̬
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
* 	����ң�ذ������
*
***********************************************************************************/

static unsigned char WirelessKeyCheck(void)
{
	unsigned char key=0;
	FaultLogType type=0;
	char *desc[]={"Auto","Manual","YellowBlink","AllRed","StepByStep"};
	int tmpint = -1;
	static int lastkeystatus[5] = {1,1,1,1,1};  //��һ�λ�ȡ��5������״̬
	int curkeystatus[5] = {1,1,1,1,1};       //��ǰ��ȡ����5������״̬
	int i = 0;
	if(g_io_fd != -1)
	{
		ioctl(g_io_fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//�������߰���״̬
		for(i = 0; i < 5; i++)
		{
			curkeystatus[i] = BIT(tmpint, i);   //��ȡ5��������ֵ��1��ʾδ������0��ʾ����
			if(curkeystatus[i] == 1 && lastkeystatus[i] == 0)    //�����ɿ�ʱ���������ź�(�����ر�ʾ����)
			{
				switch(i)
				{
					case 0:   //ȫ�찴ť����
						key = 4;
						type = WIRELESS_KEY_ALLRED;
						break;
					case 1:   //������ť����
						key = 3;
						type = WIRELESS_KEY_YELLOWBLINK;
						break;
					case 2:   //������ť����
						key = 5;
						type = WIRELESS_KEY_STEP;
						break;
					case 3:   //�Զ���ť����
						key = 1;
						type = WIRELESS_KEY_AUTO;
						break;
					case 4:   //�ֶ���ť����
						key = 2;
						type = WIRELESS_KEY_MANUAL;
						break;
					default:
						break;
				}		
			}
			lastkeystatus[i] = curkeystatus[i];       //������һ�ΰ�����ֵ
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
