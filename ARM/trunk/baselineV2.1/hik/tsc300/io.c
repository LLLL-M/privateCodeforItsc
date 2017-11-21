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
#include "common.h"
#include "its.h"
#include "LogSystem.h"

#define DEVICE_NAME "/dev/gpio"
#define WIRELESS_CONTROLLER_DELAY_TIME	30	//delay time of wireless controller  after system reboot, uint: second
#define WIRELESS_WRONG_TRIGGER_TIME		3	//same key pressed in 3s would not be handled.
#define WIRELESS_MANUAL_WAITING_TIME	10	//the waiting time of wireless manual control  before return to auto control (added for wrong trgger)

#define KEYBOARD_DELAY_TIME				30	//delay time of new keyboard, unit:second, to avoid wrong tirgger

#ifndef BIT
#define BIT(v, n) (((v) >> (n)) & 0x1)      //ȡv�ĵ� n bitλ
#endif

static int g_io_fd = 0;

int g_auto_pressed = 1;
int g_manual_pressed = 0;
int g_flashing_pressed = 0;
int g_allred_pressed = 0;
int g_step_by_step_pressed = 0;

static time_t system_begin_time = 0;

static void KeyBoardSetLed(unsigned char led);

extern void CanSend(struct can_frame *pframe);

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

//����ǰ��尴��
void Set_front_board_keys(UInt8 status)
{
	ioctl(g_io_fd, IO_SET_BUTTON_STATUS, &status);
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
	CanSend(&m_frame_send);
	
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

/*****************************************************
**	Desc:		Check if any key pressed from the wireless controller.
				It should be used in a loop with short interval.
**	param:		none
**	Return Value:	0-5
**	Author:		Kevin Pan
*****************************************************/
static unsigned char WirelessKeyCheck(void)
{
	FaultLogType type=0;
	unsigned char wireless_data = 0;
	int j = 0;
	char msg[128] = "";
	static unsigned char lastkeystatus[5] = {0};
	unsigned char currentkeystatus[5] = {0};
	unsigned char keyPressed=0; 
	
	//��������30s������ң�ز���Ч
	if((time(NULL)-system_begin_time) < WIRELESS_CONTROLLER_DELAY_TIME)
	{
		return V_KEY_INVALID;
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
				switch(j)
				{
					case 0:
						type = WIRELESS_KEY_AUTO;
						sprintf(msg,"WirelssController: Key %d[Auto] pressed!", j+1);
						break;
					case 1:
						type = WIRELESS_KEY_MANUAL;
						sprintf(msg,"WirelssController: Key %d[Manual] pressed!", j+1);
						break;
					case 2:
						type = WIRELESS_KEY_YELLOWBLINK;
						sprintf(msg,"WirelssController: Key %d[YellowBlink] pressed!", j+1);
						break;
					case 3:
						type = WIRELESS_KEY_ALLRED;
						sprintf(msg,"WirelssController: Key %d[AllRed] pressed!", j+1);
						break;
					case 4:
						type = WIRELESS_KEY_STEP;
						sprintf(msg,"WirelssController: Key %d[StepByStep] pressed!", j+1);
						break;
					default:
						sprintf(msg,"WirelssController: Key %d[unknown] pressed!", j+1);
						break;
				}
				keyPressed=j+1;
				//WriteKeyCtrlLogInfos("/home/Wireless.log",msg);
				ItsWriteFaultLog(type, keyPressed);
				log_debug(msg);
				INFO("%s",msg);
			}
			lastkeystatus[j] = currentkeystatus[j];
		}
	}

	return keyPressed;
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
static eKeyStatus keyCheck(eKeyStatus curPressed, int *lastPressed, unsigned char manualOvertimeFlag)
{
	eKeyStatus ret=V_KEY_INVALID;
	unsigned char led=0;
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
		if(manualOvertimeFlag == 1 && (*lastPressed == V_KEY_MANUAL) && ((curtime - lasttime) >= WIRELESS_MANUAL_WAITING_TIME))
		{
			INFO("ManualControl: Manualkey Overtime, Return to Autokey...");
			log_debug("ManualControl: Manualkey Overtime, Return to Autokey...");
			*lastPressed = V_KEY_AUTO;
			KeyBoardSetLed(V_KEY_AUTO);
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
				led = V_KEY_MANUAL;
			}
			break;
		case V_KEY_MANUAL:
			if(curPressed != V_KEY_MANUAL)
			{
				ret = curPressed;
				led = curPressed;
				*lastPressed = curPressed;
			}			
			break;
		case V_KEY_STEP:
			if(curPressed != V_KEY_MANUAL)
			{
				ret = curPressed;
				led = curPressed;
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
				led = curPressed;
				*lastPressed = curPressed;
			}
			break;
		default:
			break;

	}
	KeyBoardSetLed(led);
	return ret;
}
int lastKeyPressed = V_KEY_AUTO;//recorde the last key pressed from keyboard or wireless
static unsigned char gWirelessAutoFlag=0;
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
		ret = (gWirelessAutoFlag == 0)?WirelessKeyCheck():2;
		if(gWirelessAutoFlag != 0)
			gWirelessAutoFlag = 0;
	}
	return ret;
}

/*************************************************/
static int g_key_pressed=0;//current key pressed from frontkeyboard, update every 500ms
static int g_key_fd=-1;		//the file descriptor of keyboard device
/*****************************************
**	open device ttyS4 and set its attribute
******************************************/
static char KeyDevInit(void)
{
	if((g_key_fd = open_port(4)) <= 0)
	{
		ERR("Keyboard: open port error \n"); 
		return -1; 
	} 
	if(set_opt(g_key_fd,9600,8,'N',1)<0)
	{
		ERR("Keyboard: set_opt error"); 
		return -1; 
	}
	return 0;
}
/**********************************************************
**	handle the event of key pressed from frontKeyBoard, update every 500ms
***********************************************************/
static void KeyBoardRecvProcess(void)
{
	FaultLogType type=0;
	int nread = 0; 
	char recvKey=0;
	int i;
	char recvBuff[32]="";
	char *keyString[]={
		"Auto", "Manual", "YellowBlink", "AllRed", "StepByStep",
		"East", "South", "West", "North",
		"EastAndWestStraight", "SouthAndNorthStraight", "EastAndWestLeft", "SouthAndNorthLeft"};

	while(1)
	{
		g_key_pressed=0;
		/*if((time(NULL)-system_begin_time) < KEYBOARD_DELAY_TIME)
		{
			usleep(500000);//500ms
			continue;
		}*/
		if((nread = read(g_key_fd, &recvBuff, 32))>0)
		{
			if(nread>0)
			{
				//INFO("Keyboard recv: %s...", recvBuff);
				for(i=0; i<13; i++)
				{
					if(!strncmp(recvBuff, keyString[i], nread))
						break;
				}

				if(i == 13)
					recvKey = 0;
				else
					recvKey = i+1;
				memset(recvBuff, 0, 32);
			}
			if(recvKey >0 && recvKey <14)
			{
				switch(recvKey)
				{
					case V_KEY_AUTO:
						type = MANUAL_PANEL_AUTO;
						break;
					case V_KEY_MANUAL:
						type = MANUAL_PANEL_MANUAL;
						break;
					case V_KEY_STEP:
						type = MANUAL_PANEL_STEP;
						break;
					case V_KEY_YELLOWBLINK:
						type = MANUAL_PANEL_FLASH;
						break;
					case V_KEY_ALLRED:
						type = MANUAL_PANEL_ALL_RED;
						break;
					case V_KEY_EAST:
						type = MANUAL_PANEL_EAST;
						break;
					case V_KEY_SOUTH:
						type = MANUAL_PANEL_SOUTH;
						break;
					case V_KEY_WEST:
						type = MANUAL_PANEL_WEST;
						break;
					case V_KEY_NORTH:
						type = MANUAL_PANEL_NORTH;
						break;
					case V_D_EAST_WEST:
						type = MANUAL_PANEL_EASTWEST_DIRECT;
						break;
					case V_D_SOUTH_NORTH:
						type = MANUAL_PANEL_SOUTHNORTH_DIRECT;
						break;
					case V_L_EAST_WEST:
						type = MANUAL_PANEL_EASTWEST_LEFT;
						break;
					case V_L_SOUTH_NORTH:
						type = MANUAL_PANEL_SOUTHNORTH_LEFT;
						break;
					default:
						break;
				}
				
				g_key_pressed = recvKey;
				ItsWriteFaultLog(type, recvKey);
				log_debug("Keyboard: key%d[%s] pressed!", g_key_pressed, keyString[recvKey-1]);
			}
		}
		usleep(500000);//500ms
	}
	pthread_exit(NULL);
}

/*****************************************
**	create a thread for receiving keyboard event
******************************************/
static void key_board_thread_create(void)
{
	//�����������������߳�
	pthread_t keyBoard_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 2<<20);//2M

	if (0 != pthread_create(&keyBoard_thread, &attr, (void*)KeyBoardRecvProcess, NULL))
	{
		//log error
		ERR("Keyboard recv thread creat failure!!\n");
	} else {
		INFO("****KeyBoard detect thread creat successful!**********");
	}
	pthread_detach(keyBoard_thread);
	pthread_attr_destroy(&attr);
}
/*****************************************
**	Desc: Light the led of the keyboard
** 	led:		id of the led, range: 1-13
** 	return value:	none
******************************************/
void KeyBoardSetLed(unsigned char led)
{
	char buff[16]="Button";
	if(g_key_fd == -1 || led == 0)
		return;

	buff[6]=led;
	//INFO("--->Keyboard light led: %s..", buff);
	if(write(g_key_fd, buff, 7) < 0)
		ERR("set led %d error...", led);
}
static unsigned char gOutControlKey=0;
void KeyBoardSetKey(unsigned char keyv)
{
	if(keyv >= V_KEY_AUTO && keyv <= V_L_SOUTH_NORTH)
		gOutControlKey = keyv;
}
unsigned char GetKeyBoardStatus(void)
{
	unsigned char key=0;

	key = (gOutControlKey == 0)? keyCheck(g_key_pressed, &lastKeyPressed, 1) : keyCheck(gOutControlKey, &lastKeyPressed, 1);
	if(gOutControlKey != 0)
	{
		gOutControlKey = 0;
	}
	return key;
}
void KeyBoardInit(void)
{
	if(KeyDevInit() != -1)
	{
		key_board_thread_create();
		KeyBoardSetLed(V_KEY_AUTO);//defalut auto control
		g_key_pressed = V_KEY_AUTO;
	}
}

/************************************************/

