#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "canmsg.h"
#include "common.h"
#include "debug.h"
#include "io_ioctl.h"

#include "msg.h"

int g_fd_send_300 = 0;  //CAN套接字
extern int g_io_fd;
//灯控板电压及电流互斥锁
pthread_mutex_t g_MutexLamp_300 = PTHREAD_MUTEX_INITIALIZER;

#define BOARD_NUM	6
unsigned long   g_volt_300[BOARD_NUM] = {0}; 	//灯控电压
unsigned char   g_cur_300[BOARD_NUM][BOARD_NUM] = {{0}};	//灯控电流

extern unsigned short g_lampctrl;
extern unsigned char g_boardnumBit;

void KeyBoardSetLed(unsigned char led);

void canits_set_bitrate_300(const char *name, unsigned long bitrate)
{
	int err = can_set_bitrate(name, bitrate);
	if (err < 0)
	{
		ERR("set %s bitrate(%lu) failed\n", name, bitrate);
	}
}

void canits_can_start_300(const char *name)
{
	if (can_do_start(name) < 0)
	{
		ERR("%s start failed\n", name);
	}
}

void canits_create_recv_thread_300()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, canits_recv_thread_300, NULL))
	{
		ERR("create can receive thread fail");
		exit(1);
	}
}
void canits_create_socket_can_send_300()
{
    struct sockaddr_can addr;
    struct ifreq ifr;

    g_fd_send_300 = socket(AF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, "can0");
    ioctl(g_fd_send_300, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(g_fd_send_300, (struct sockaddr *)&addr, sizeof(addr));	
}

void canits_init_300()
{
	can_do_stop("can0");
	canits_set_bitrate_300("can0", 500000);
	//INFO("set can0 bitrate 500K!!!\n");
	canits_can_start_300("can0");
	canits_create_recv_thread_300();
    canits_create_socket_can_send_300();
}

void canits_send_300(struct can_frame *pcanfram)
{
	int ret = write(g_fd_send_300, pcanfram, sizeof(struct can_frame));
	if (ret == -1)
	{
		ERR("canits_send_300 error, error info:%s\n", strerror(errno));
	}
}

void parse_lamp_voltcur_300(struct can_frame *pframe)
{
	//int i = 0;
	pthread_mutex_lock(&g_MutexLamp_300);
	//电压
	int boardNo = pframe->can_id&0x7;
	if (boardNo<1 || boardNo>4)
	{
		pthread_mutex_unlock(&g_MutexLamp_300);
		return;
	}
	g_volt_300[boardNo-1] =  (pframe->can_id>>3)&0xfff;
	//INFO("boardNo:%d, g_volt:%#x\n", boardNo, g_volt[boardNo-1]);
	//电流
	memcpy(g_cur_300[boardNo-1], pframe->data, 4);
	pthread_mutex_unlock(&g_MutexLamp_300);
}

void *canits_recv_thread_300(void *p)
{
	//INFO("can_recv_thread is running\n");
	struct sockaddr_can addr;
    struct ifreq ifr;
	int __attribute__((unused)) i = 0;
	int m_fd_recv = socket(AF_CAN, SOCK_RAW, CAN_RAW);

	strcpy(ifr.ifr_name, "can0");
	ioctl(m_fd_recv, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	bind(m_fd_recv, (struct sockaddr *)&addr, sizeof(addr));
	struct can_frame frame;
	while (1)
	{
		if (read(m_fd_recv, &frame, sizeof(struct can_frame)) > 0)
		{
		    //INFO("===> can Id   0x%x\n",frame.can_id);
			if (GET_BIT(frame.can_id, 15) == 1 && frame.can_dlc == 4) {
			//第15位为1并且长度为4的can消息表示是灯控板电流及电压反馈
				/*INFO("电流及电压反馈:");
				for (i = 0; i < frame.can_dlc; i++)
				{
					INFO("dlc=%d,data[%d]=%d\n", frame.can_dlc, i,frame.data[i]);
				}*/
				parse_lamp_voltcur_300(&frame);
			}
			//前面板5个按键状态
			else if(frame.can_id == 0x401 && frame.can_dlc == 1) {
				//INFO("recv foreboard info, data:%#x", frame.data[0]);
				if ((g_io_fd != -1) && (frame.data[0] & 0x1f) != 0)
					ioctl(g_io_fd, IO_SET_BUTTON_STATUS, &frame.data[0]);
			}
		}
	}
}

#define BIT(v, n)	((v >> n) & 0x1)
static void key_led_test()
{
	struct can_frame m_frame_send;
	unsigned char key_data = 0;
	int arg = 0;
	int i;
	
	ioctl(g_io_fd, IO_GET_BUTTON_STATUS, &key_data);
	if (key_data == 0)
		return;
	
	memset(&m_frame_send, 0, sizeof(struct can_frame));
	m_frame_send.can_id = 0x110;
	m_frame_send.can_dlc = 1;
	m_frame_send.data[0] = key_data;
	canits_send_300(&m_frame_send);
	
	arg = BIT(key_data, 0) ? ARG_AUTO_LED_ON : ARG_AUTO_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = BIT(key_data, 1) ? ARG_MANUAL_LED_ON : ARG_MANUAL_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = BIT(key_data, 2) ? ARG_YELLOW_BLINK_LED_ON : ARG_YELLOW_BLINK_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = BIT(key_data, 3) ? ARG_ALL_RED_LED_ON : ARG_ALL_RED_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = BIT(key_data, 4) ? ARG_STEP_LED_ON : ARG_STEP_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
}

void *lampctrl_thread(void *p)
{
	unsigned short nLampStatus[8] = {0};
	int counter = 0;
	int i, bit = 0;
	while (1)
	{
		if (g_lampctrl == 0)//全部灭灯
		{
			memset((char*)nLampStatus, 0, sizeof(nLampStatus));
			counter = 6;
		}
		else if(g_lampctrl == 1)//全部点亮
		{
			memset((char*)nLampStatus, 0xff, sizeof(nLampStatus));
			counter = 0;
		}
		else if(g_lampctrl == 2)//点亮设置的灯控板的所有灯
		{
			memset((char*)nLampStatus, 0, sizeof(nLampStatus));
			for(i=0; i<6; i++)
			{
				if(GET_BIT(g_boardnumBit, i))
					nLampStatus[i] = 0xffff;
			}
		}
		else//(g_lampctrl == 3)
		{
			if(++counter > 4)//3s
			{
				if(bit > 11)
				{
					bit = 0;
					g_lampctrl = 0;//停止电灯
					continue;
				}

				memset((char*)nLampStatus, 0, sizeof(nLampStatus));
				for(i=0; i<6; i++)
				{
					if(GET_BIT(g_boardnumBit, i))
						SET_BIT(nLampStatus[i], bit);
				}
				// 1short = 2bytes =16bits = 4channels
				// first channel: bit 0,1,2 -> green,red,yellow(1-light, 0-off)
				// second channel: bit 3,4,5 -> green,red,yellow(1-light, 0-off)
				// ...
				//bit 11,12,13,14
				if(bit%3 == 2)// 2,5,8,11
					bit--;
				else
					bit += 2;

				if(bit == 11)// 11 -> for pedestrian, no yellow
					bit--;
	
				counter = 0;
			}
		}
		i_can_its_send_led_request_300(1, nLampStatus);
		key_led_test();
		usleep(300000);
	}
	pthread_exit(NULL);
}
/*****************************************************接口*************************************************/
void i_can_its_init_300()
{
	canits_init_300();
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, lampctrl_thread, NULL))
	{
		ERR("create light control thread fail");
		exit(1);
	}
	pthread_detach(thread);
}

//发送点灯命令
void i_can_its_send_led_request_300(int boardNum, unsigned short *poutLamp)
{
	if (NULL == poutLamp)
		return;
	unsigned short *p = poutLamp;
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 7;
	m_frame_send.data[0] = 1;
	m_frame_send.data[1] = p[0]&0xff;
	m_frame_send.data[2] = ((p[1]&0xf)<<4) | ((p[0]>>8)&0xf);
	m_frame_send.data[3] = (p[1]>>4)&0xff;
	m_frame_send.data[4] = p[2]&0xff;
	m_frame_send.data[5] = ((p[3]&0xf)<<4) | ((p[2]>>8)&0xf);
	m_frame_send.data[6] = (p[3]>>4)&0xff;
	canits_send_300(&m_frame_send);
}

//获取电压
void i_can_its_get_Volt_300(int boardNum, unsigned short *pboardInfo)
{
	if (NULL == pboardInfo)
	{	
		return;
	}
	pthread_mutex_lock(&g_MutexLamp_300);
	switch (boardNum)
	{
	case 1:
		*pboardInfo = g_volt_300[0]&0xfff;
		break;
	case 2:
		*pboardInfo = g_volt_300[1]&0xfff;//((g_volt[1]&0x3f)<<6) | ((g_volt[0]>>12)&0x3f);
		break;
	case 3:
		*pboardInfo = g_volt_300[2]&0xfff;//(g_volt[1]>>6)&0xfff;
		break;
	case 4:
		*pboardInfo = g_volt_300[3]&0xfff;//g_volt[2]&0xfff;
		break;
	default:
		break;
	} 
	pthread_mutex_unlock(&g_MutexLamp_300);
}

//获取电流
unsigned short i_can_its_get_cur_300(int boardNum, int pahseNum, int redGreen)
{
	unsigned short curinfo = 0;
	//unsigned char *p = g_cur[0];
	pthread_mutex_lock(&g_MutexLamp_300);
	curinfo = g_cur_300[boardNum - 1][pahseNum - 1];
	pthread_mutex_unlock(&g_MutexLamp_300);
	return curinfo;
}

void turn_on_all_led()
{
	struct can_frame m_frame_send;
	int i, arg;
	//点亮程序运行和GPS指示灯
	arg = ARG_SYSTEM_RUNNING_LED_ON;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_GPS_LED_ON;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	//点亮键盘板5个按键指示灯
	arg = ARG_AUTO_LED_ON;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_MANUAL_LED_ON;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_YELLOW_BLINK_LED_ON;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_ALL_RED_LED_ON;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_STEP_LED_ON;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	//点亮前面板5个按键指示灯
	memset(&m_frame_send, 0, sizeof(struct can_frame));
	m_frame_send.can_id = 0x110;
	m_frame_send.can_dlc = 1;
	m_frame_send.data[0] = 0x1f;
	canits_send_300(&m_frame_send);
}

void turn_off_all_led()
{
	struct can_frame m_frame_send;
	int i, arg;
	//熄灭程序运行和GPS指示灯
	arg = ARG_SYSTEM_RUNNING_LED_OFF;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_GPS_LED_OFF;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	//熄灭键盘板5个按键指示灯
	arg = ARG_AUTO_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_MANUAL_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_YELLOW_BLINK_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_ALL_RED_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_STEP_LED_OFF;
	(void) ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	//熄灭前面板5个按键指示灯
	memset(&m_frame_send, 0, sizeof(struct can_frame));
	m_frame_send.can_id = 0x110;
	m_frame_send.can_dlc = 1;
	m_frame_send.data[0] = 0;
	canits_send_300(&m_frame_send);
}

//黄闪控制线程
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
void yellow_blink_init()
{
	pthread_t threadId;
	if (pthread_create(&threadId, NULL, yellow_blink_thread, NULL)) {
		perror("create yellow light thread fail");
		exit(1);
	}
	pthread_detach(threadId);
	//INFO("yellow light thread init successful!");
}

extern struct sockaddr_in g_wireless_servaddr;
extern void sendMessage(char msgtype, char *data, int datalen, struct sockaddr_in *addr);
unsigned char g_wireless_check_flag = 0;
static unsigned char g_wireless_key_pressed = 0;
static void *wireless_key_check(void *arg)
{
	//int tmp = 0;
	char buff[256] = {0};
	unsigned char wireless_data = 0;
	unsigned char lastkeystatus[5] = {0};
	unsigned char currentkeystatus[5] = {0};
	char *keyStr[5] = {"自动[南北直行]","手动[自动]","黄闪[东西直行]","全红[南北左转]","步进[东西左转]"};
	int i = 0;

	while(1)
	{
		if(g_io_fd != -1 && g_wireless_check_flag == 1)	
		{
			ioctl(g_io_fd, IO_GET_WIRELESS_STATUS, &wireless_data);
			for(i = 0; i < 5; i++)
			{
				if(GET_BIT(wireless_data, i) == 1)	
					currentkeystatus[i] = 1;
				else
					currentkeystatus[i] = 0;

				if(lastkeystatus[i] == 1 && currentkeystatus[i] == 0)
				{
					INFO("无线遥控器：按键 %s 按下！", keyStr[i]);
					memset(buff, 0, 256);
					((int *)buff)[0] = i+1;
					g_wireless_key_pressed = i+1;
					KeyBoardSetLed(i+1);
					sprintf(buff + 4, "无线遥控器：按键 %s 按下！\n", keyStr[i]);
					//tmp = i+1;
					sendMessage(FTEST_MSG_WIRELESS_DATA, buff, 4 + strlen(buff+4), &g_wireless_servaddr);
					//INFO("Wireless: key down!");
				}
				lastkeystatus[i] = currentkeystatus[i];
			}
		}
		usleep(200000); //200ms

	}
}
void wireless_init()
{
	pthread_t thread;
	g_wireless_check_flag = 0;
	if(0 != pthread_create(&thread, NULL, wireless_key_check, NULL))
	{
		perror("create wireless check thread failed!");
		exit(1);
	}
	pthread_detach(thread);
}
void set_wireless_check_flag(char v)
{
	if(v == 0 || v == 1)
	{
		g_wireless_check_flag = v;
		//INFO("--> wireless check %s...", (v==1 ? "on" : "off"));
	}
}

/*************************************************/
unsigned char g_keyboard_check_flag = 0;
extern struct sockaddr_in g_keyboard_servaddr;
static int g_key_pressed=0;//current key pressed from frontkeyboard, update every 500ms
static int g_key_fd=-1;		//the file descriptor of keyboard device

void set_keyboard_check_flag(int v)
{
	if(v == 1 || v == 2)
	{
		g_keyboard_check_flag = (v == 2 ? 0 : 1);
		INFO("--> keyboard check %s...", (v==1 ? "on" : "off"));
	}
}
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
	//FaultLogType type=0;
	char buff[256] = {0};
	int nread = 0; 
	char recvKey=0;
	int i;
	char recvBuff[32]="";
	char *keyString[]={
		"Auto", "Manual", "YellowBlink", "AllRed", "StepByStep",
		"East", "South", "West", "North",
		"EastAndWestStraight", "SouthAndNorthStraight", "EastAndWestLeft", "SouthAndNorthLeft"};
	char *keyString2[] = {
		"自动","手动","黄闪","全红","步进",
		"东放行","南放行","西放行","北放行",
		"东西直行","南北直行","东西左转","南北左转"
	};

	while(1)
	{
		if(g_key_fd != -1 && g_keyboard_check_flag == 1)
		{
			g_key_pressed=0;
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
					g_key_pressed = recvKey;
					KeyBoardSetLed(g_key_pressed);
					//ItsWriteFaultLog(type, recvKey);
					INFO("手动控制面板：按键%02d[%s] 按下！\n", g_key_pressed, keyString2[recvKey-1]);
					memset(buff, 0, 256);
					((int *)buff)[0] = 1;
					sprintf(buff+4, "手动控制面板：按键%02d[%s] 按下！\n", g_key_pressed, keyString2[recvKey-1]);
					//tmp = i+1;
					sendMessage(FTEST_MSG_KEYBOARD_DATA, buff, 4 + strlen(buff+4), &g_keyboard_servaddr);				
				}
			}
		}
		else
		{
			if(g_wireless_key_pressed != 0)
				KeyBoardSetLed(g_wireless_key_pressed);
			else
				KeyBoardSetLed(V_KEY_AUTO);
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
	//创建按键面板接收主线程
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
void KeyBoardInit(void)
{
	if(KeyDevInit() != -1)
	{
		key_board_thread_create();
		//KeyBoardSetLed(V_KEY_AUTO);//defalut auto control
		g_key_pressed = V_KEY_INVALID;
	}
}

/************************************************/