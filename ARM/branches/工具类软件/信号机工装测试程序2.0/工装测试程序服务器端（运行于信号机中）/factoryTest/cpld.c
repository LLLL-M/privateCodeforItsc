#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "common.h"
#include "cpld.h"
#include "debug.h"
#include "msg.h"

int g_CPLDFd = -1;
unsigned short g_nLampStatus[8] = {0};
time_t system_begin_time = 0;
static unsigned char gIoInputCheck = 0;

int auto_pressed = 1;
int manual_pressed = 0;
int flashing_pressed = 0;
int allred_pressed = 0;
int step_by_step_pressed = 0;

extern struct sockaddr_in g_keyboard_servaddr;
extern unsigned char g_boardnumBit;
/*********************************************************************************
*
* 	CPU与CPLD交互初始化
*
***********************************************************************************/
void CPLD_IO_Init()
{
	//获取当前时间秒数
	time(&system_begin_time); 
	g_CPLDFd = open(DEVICE_NAME,O_RDONLY);
	if(g_CPLDFd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
}

int RS232_test(char *result)
{
	int fd = 0; 
	int len = 0;
	char sendBuff[32] = "RS232 TEST SEND";
    char recvBuff[32] = {0}; 
	int ret = 0;
	char *p = result;
	//打开串口 
   	if(p == NULL || (fd = open_port(2))<0)
	{
	    perror("open_port error"); 
        return ret; 
    } 

   	if(len = write(fd, sendBuff, strlen(sendBuff))>0)
	{
		printf("串口232输出成功\n");
		ret = 1;
	}
	else
	{
		memcpy(result, "串口232输出数据失败!\n", sizeof("串口232输出数据失败!\n"));
		p += sizeof("串口232输出数据失败!\n");
		return 0;
	}
	sleep(1);
	len = read(fd, recvBuff, strlen(sendBuff));
	printf("232 len:%d, recv:%s\n", len, recvBuff);
	if(len && !strncmp(sendBuff, recvBuff, strlen(sendBuff)))
	{
		printf("RS232接收成功!\n");
		memcpy(result, "RS232测试成功!\n", sizeof("RS232测试成功!\n"));
		ret = 1;
	}
	else
	{
		memcpy(p, "RS232读取数据失败!\n", sizeof("RS232读取数据失败!\n"));
		ret = 0;
	}
	//	printf("RS232测试失败!\n");
	close(fd);
	return ret;
}
int RS422_test(char* result)
{
	int fd = 0; 
 	int len = 0;
	char sendBuff[32] = "RS422 TEST SEND";
   	char recvBuff[32]= {0}; 
	int ret = 0;
	char *p = result;
	
	//打开串口 
   	if(NULL == p || (fd=open_port(4))<0)
	{
        perror("open_port error"); 
    	return ret; 
   	} 
	if(set_opt(fd, 9600, 8, 'N', 1) < 0)
	{
       	printf("set_opt(422) error"); 
		close(fd);
        return 0; 
    }
	if(len = write(fd, sendBuff, strlen(sendBuff))>0)
	{
		printf("串口422输出成功\n");
		ret = 1;
	}
	else
	{
		memcpy(result, "串口422输出数据失败!\n", sizeof("串口422输出数据失败!\n"));
		p += sizeof("串口422输出数据失败!\n");
		return 0;
	}
	sleep(1);
	len = read(fd, recvBuff, strlen(sendBuff));
	printf("422 recv len:%d, Str:%s\n", len, recvBuff);
	if(len && !strncmp(sendBuff, recvBuff, strlen(sendBuff)))
	{
		printf("RS422接收成功!\n");
		memcpy(result, "RS422测试成功!\n", sizeof("RS422测试成功!\n"));
		ret = 1;
	}
	else
	{
		memcpy(result, "RS422读取数据失败!\n", sizeof("RS422读取数据失败!\n"));
		ret = 0;
	}		
	close(fd);
	return ret;
}

int rs485CommunicationTest(int sendfd, int recvfd, char *result)
{
	
	int ret = 0;
	int len = 0;
	int ttySend = -1;
	int ttyRecv = -1;
	char sendBuff[32] = "RS485 Test ttyS4 SEND";
	char recvBuff[32] = {0};
	int arg;
	char *pos = result;
	
	if(!((sendfd == 5 && recvfd == 6)|| (sendfd == 6 && recvfd == 5)))
		return ret;
	
	ttySend = open_port(sendfd);
	ttyRecv = open_port(recvfd);
	if (ttySend < 0 || ttyRecv < 0 || g_CPLDFd == -1)
	{
		printf("open port error!\n");
		return ret;
	}
	if((ret = set_opt(ttySend, 9600, 8, 'N', 1)) < 0)
	{
       	printf("set_opt(ttysend:%d) error",sendfd); 
		close(ttySend);
		close(ttyRecv);
        return 0; 
    }
	if((ret = set_opt(ttyRecv, 9600, 8, 'N', 1)) < 0)
	{
       	printf("set_opt(ttyrecv:%d) error",recvfd); 
		close(ttySend);
		close(ttyRecv);
        return 0; 
    }
	//ttyS4发送，ttyS5接收
	ioctl(g_CPLDFd, sendfd == 5 ? TTYS4_SEND_ENABLE : TTYS5_SEND_ENABLE, &arg);
	ioctl(g_CPLDFd, recvfd == 6 ? TTYS5_RECEIVE_ENABLE : TTYS4_RECEIVE_ENABLE, &arg);
	if(sendfd == 6)
		sendBuff[15] = '5';
	if((len = write(ttySend, sendBuff, strlen(sendBuff)))>0)
	{
		printf("RS485: len: %d, ttySend: %s\n", len, sendBuff);
		//tmpret = 1;
	}
	else
	{
		printf("RS485: ttySend failed!\n");
		sprintf(pos, "RS485发送测试失败！(ttyS%d发送).\n", sendfd-1);
		pos += strlen("RS485发送测试失败！(ttyS4发送).\n");
		return 0;
	}
	sleep(1);
	len = read(ttyRecv, recvBuff, strlen(sendBuff));
	printf("RS485: len:%d, ttyRecv: %s\n", len, recvBuff);
	if(len && !strncmp(sendBuff, recvBuff, strlen(sendBuff)))
	{
		sprintf(pos, "RS485测试成功！(ttyS%d发送，ttyS%d接收).\n", sendfd-1, recvfd-1);
		ret = 1;
	}
	else
	{
		sprintf(pos, "RS485接收测试失败！(ttyS%d接收:%s).\n", recvfd-1, recvBuff);
		ret = 0;
	}	
	close(ttySend);
	close(ttyRecv);
	
	return ret;
}
int RS485_test500(char *result)
{
	char *p = result;
	int ret = 0;
	//ttys4 send. ttys5 recv
	ret = rs485CommunicationTest(5, 6, p);
	p += strlen(result);
	
	//ttys5 send. ttys4 recv
	if(rs485CommunicationTest(6, 5, p) == 1 && ret == 1) 
		return 1;
	
	return 0;
}
int IO_OUT_High_TEST(void)
{
	int fd = 0;
	int arg = -1;
	
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
		return -1;
	}

	ioctl(fd,IO_OUTPUT1_0,&arg);
	ioctl(fd,IO_OUTPUT2_0,&arg);
	ioctl(fd,IO_OUTPUT3_0,&arg);
	ioctl(fd,IO_OUTPUT4_0,&arg);
	ioctl(fd,IO_OUTPUT5_1,&arg);
	ioctl(fd,IO_OUTPUT6_1,&arg);
	ioctl(fd,IO_OUTPUT7_1,&arg);
	ioctl(fd,IO_OUTPUT8_1,&arg);
	ioctl(fd,IO_OUTPUT9_1,&arg);
	ioctl(fd,IO_OUTPUT10_1,&arg);
	ioctl(fd,IO_OUTPUT11_1,&arg);
	ioctl(fd,IO_OUTPUT12_1,&arg);
	ioctl(fd,IO_OUTPUT13_1,&arg);
	ioctl(fd,IO_OUTPUT14_1,&arg);
	ioctl(fd,IO_OUTPUT15_1,&arg);
	ioctl(fd,IO_OUTPUT16_1,&arg);
	ioctl(fd,IO_OUTPUT17_1,&arg);
	ioctl(fd,IO_OUTPUT18_1,&arg);
	ioctl(fd,IO_OUTPUT19_1,&arg);
	ioctl(fd,IO_OUTPUT20_1,&arg);
	close(fd);
	
	return 0;
}
int IO_OUT_Low_TEST(void)
{
	int fd = 0;
	int arg = -1;
	
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
		return -1;
	}

	ioctl(fd,IO_OUTPUT1_1,&arg);
	ioctl(fd,IO_OUTPUT2_1,&arg);
	ioctl(fd,IO_OUTPUT3_1,&arg);
	ioctl(fd,IO_OUTPUT4_1,&arg);
	ioctl(fd,IO_OUTPUT5_0,&arg);
	ioctl(fd,IO_OUTPUT6_0,&arg);
	ioctl(fd,IO_OUTPUT7_0,&arg);
	ioctl(fd,IO_OUTPUT8_0,&arg);
	ioctl(fd,IO_OUTPUT9_0,&arg);
	ioctl(fd,IO_OUTPUT10_0,&arg);
	ioctl(fd,IO_OUTPUT11_0,&arg);
	ioctl(fd,IO_OUTPUT12_0,&arg);
	ioctl(fd,IO_OUTPUT13_0,&arg);
	ioctl(fd,IO_OUTPUT14_0,&arg);
	ioctl(fd,IO_OUTPUT15_0,&arg);
	ioctl(fd,IO_OUTPUT16_0,&arg);
	ioctl(fd,IO_OUTPUT17_0,&arg);
	ioctl(fd,IO_OUTPUT18_0,&arg);
	ioctl(fd,IO_OUTPUT19_0,&arg);
	ioctl(fd,IO_OUTPUT20_0,&arg);
	close(fd);
	return 0;
}

void Get_CarDetector_state(int *result)
{
	unsigned short boardInfo = 0;
	int boardNum = 0;//取值1,2,3
	int ibit = 0;//位移量
	int Car_State = 0;
	for(boardNum = 1; boardNum < 4; boardNum++)
	{
		boardInfo = recv_date_from_vechile(boardNum);
		for(ibit = 0; ibit < 16; ibit++)
		{
			Car_State = ((boardInfo>>ibit) & 0x01);
			if(Car_State)
				printf("第%d通道,过车状态:%d\n",(boardNum-1)*16+ibit+1, Car_State);
			result[(boardNum-1)*16+ibit] = Car_State;
		}
	}
}
//依次点亮每块灯控板上的灯
void PhaseLampOutput(int boardNum)
{
	static int iChanLevel = 0;//代表每块板子的第几通道，取值范围1~6 
	static int iChannel = 0;//通道号
	static int lightCloor = 0;//红1黄2绿0
	int bnum = 0; //灯控板号
	for(iChanLevel = 0; iChanLevel < 6; iChanLevel++)
	{
		do{
			lightCloor++;
			if(lightCloor > 2)
			{
				lightCloor = 0;
			}
			
			for(bnum = 0; bnum < 6; bnum++)
			{
				if(GET_BIT(g_boardnumBit, bnum))
				{
					iChannel = iChanLevel + 6*bnum;
					SET_BIT(g_nLampStatus[iChannel/4], (iChannel%4)*3 + lightCloor);
					printf("所有灯控板第%d通道点亮(%d)\n", iChanLevel, iChannel);
				}
			}
			i_can_its_send_led_request(boardNum, g_nLampStatus);
			usleep(1000 * 500);
			//灭灯
			memset((char*)g_nLampStatus, 0, sizeof(g_nLampStatus));	
			i_can_its_send_led_request(boardNum, g_nLampStatus);
			usleep(1000 * 500);
		}while(lightCloor);
		lightCloor = 0;
	}
}

static void lampCtrlLight(void)
{
	int chanNo = 0;//channum of every board: 1-6
	int chanNum = 0;//chan number: 0-31
	int colorbit = 0; //green-0 red-1 yellow-2 
	int bnum = 0;//board number
	//char *tStr[]={"green", "red", "yellow"};
	if(g_boardnumBit == 0)
	{
		usleep(100*1000);
		return;
	}
	
	for(chanNo = 0; chanNo < 6; chanNo++)
	{
		while(colorbit != 3)
		{
			for(bnum = 0; bnum < 6; bnum++)
			{
				if(GET_BIT(g_boardnumBit, bnum))
				{
					chanNum = chanNo + bnum*6;
					SET_BIT(g_nLampStatus[chanNum/4], (chanNum%4)*3 + colorbit);
					//printf("channum%02d: %s\n", chanNum, tStr[colorbit]);
				}
			}
			//printf("=================\n");
			i_can_its_send_led_request(1, g_nLampStatus);
			usleep(1000 * 800);		
			
			memset((char*)g_nLampStatus, 0, sizeof(g_nLampStatus));	
			if(colorbit == 2)
				colorbit = 1;
			else
				colorbit += 2;
		}
		colorbit = 0;
	}
	i_can_its_send_led_request(1, g_nLampStatus);//lights off
}

void Lamp_light_ctrl(int lightFlag)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int chanNum = 0;
	
	if(lightFlag == 1)
	{
		memset((char*)g_nLampStatus, 0, sizeof(g_nLampStatus));	
		//点亮
		for(i=0; i<6; i++)//boardnum
		{
			if(GET_BIT(g_boardnumBit, i))
			{
				//memset(((char*)g_nLampStatus)+i*3, 0xff, sizeof(char)*3);	
				for(j=0; j<6; j++)//channel number
				{
					chanNum = j + i*6;
					for(k=0; k<3; k++)//color
						SET_BIT(g_nLampStatus[chanNum/4], (chanNum%4)*3 + k);				
				}
			}
		}
		//memset((char*)g_nLampStatus, 0xff, sizeof(g_nLampStatus));	
		i_can_its_send_led_request(1, g_nLampStatus);
//		i_can_its_send_led_request(1, g_nLampStatus);
		sleep(2);
	}
	else if(lightFlag == 0)
	{
		//灭灯
		memset((char*)g_nLampStatus, 0, sizeof(short)*8);	
		i_can_its_send_led_request(1, g_nLampStatus);
//		i_can_its_send_led_request(1, g_nLampStatus);
		sleep(2);
	}
	else if(lightFlag == 2)//依次点亮
		lampCtrlLight();
	else
		return;
	
}

extern struct sockaddr_in g_ioInput_servaddr;
extern void sendMessage(char msgtype, char *data, int datalen, struct sockaddr_in *addr);
void IOinput_Check_Ctrl(unsigned char flag)
{
	if(flag == 0 || flag == 1)
		gIoInputCheck = flag;
}
void IO_IN_Check(int fd)
{
	int arg = -1;	
	int i = 0;
	int base = 0x01;
	int bits[8] = {1};
	char buff[512] = {0};
	char tmp[48] = {0};
	int index = 0;
	char *pos = &buff[24];
	char *wirelessStr[] = {"全红键按下", "黄闪键按下", "步进键按下", "自动键按下", "手动键按下"};
//	static char last_state[32] = {1};
	
	//printf("IO1-32  State:\n");
	memset(buff, 1, 24);
	memcpy(pos, "IO01-24输入口状态:\n", strlen("IO01-24输入口状态:\n"));
	pos += strlen("IO01-24输入口状态:\n");
	// IO 1-8
	ioctl(fd, IO_INPUT1_TO_INPUT8, &arg);
	//printf("IO1-IO8   Input:");
	for(i = 0; i < 8; i++, index++)
	{
		//bits[i] = (arg & base)>>i;
		buff[index] = (arg & base)>>i;
		base *= 2;
		//printf("%d-%s ", i+1, bits[i] == 0 ? "有" : "无");
		memset(tmp, 0, 48);
		sprintf(tmp, "%02d-%s ", i+1, buff[index] == 0 ? "有效" : "无效");
		memcpy(pos, tmp, strlen(tmp));
		pos += strlen(tmp);
	}
	*pos = '\n';
	pos++;
	//putchar('\n');
	
	// IO 9-16
	ioctl(fd,IO_INPUT9_TO_INPUT16,&arg);
	base = 0x01;
	for(i = 0; i < 8; i++, index++)
	{
		//bits[i] = (arg & base)>>i;
		buff[index] = (arg & base)>>i;
		base *= 2;
		memset(tmp, 0, 48);
		if(i < 5)
		{
			//printf("%d-%s ", i+1+8, bits[i]==0?wirelessStr[i]:"无");
			sprintf(tmp, "%02d-(无线遥控器)%s ", i+1+8, buff[index] == 0 ? wirelessStr[i] : "无效");
		}
		else
			sprintf(tmp, "%02d-%s ", i+1+8, buff[index] == 0 ? "有效" : "无效");
			//printf("%d-%s ", i+1+8, bits[i]==0?"有":"无");
		memcpy(pos, tmp, strlen(tmp));
		pos += strlen(tmp);
	}
	*pos = '\n';
	pos++;
	//putchar('\n');
	
	// IO 17-24
/*	ioctl(fd, IO_INPUT17_TO_INPUT24, &arg);
	base = 0x01;
	for(i = 0; i < 8; i++, index++)
	{
		buff[index] = (arg & base)>>i;
		base *= 2;
		//printf("%d-%s ", i+1, bits[i] == 0 ? "有" : "无");
		memset(tmp, 0, 48);
		sprintf(tmp, "%02d-%s ", i+1+16, buff[index] == 0 ? "有效" : "无效");
		memcpy(pos, tmp, strlen(tmp));
		pos += strlen(tmp);	
	}
	*pos = '\n';
	pos++;
	*/
	//putchar('\n');
	// IO 25-32
	ioctl(fd, IO_INPUT25_TO_INPUT32, &arg);
	base = 0x01;
	for(i = 0; i < 8; i++, index++)
	{
		buff[index] = (arg & base)>>i;
		base *= 2;
		//printf("%d-%s ", i+1, bits[i] == 0 ? "有" : "无");
		memset(tmp, 0, 48);
		sprintf(tmp, "%02d-%s ", i+1+24, buff[index] == 0 ? "有效" : "无效");
		memcpy(pos, tmp, strlen(tmp));
		pos += strlen(tmp);		
	}
	*pos = '\n';
	pos++;

	
	sendMessage(FTEST_MSG_IO_INPUT_DATA, buff, 24+strlen(pos), &g_ioInput_servaddr);
}

void *IOinput_State_Check(void *arg)
{
	int fd = -1;
	if((fd = open(DEVICE_NAME, O_RDONLY)) == -1)
	{
		printf("Open device %s error", DEVICE_NAME);
		return;
	}
	
	while(1)
	{
		if(gIoInputCheck)
		{
			IO_IN_Check(fd);
		}
		
		usleep(1000 * 1000);
	}
	close(fd);
}
void IO_Input_init()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, IOinput_State_Check, NULL))
	{
		ERR("create IO Input check thread fail");
		exit(1);
	}
	pthread_detach(thread);
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
		if(g_CPLDFd != -1)
		{
			//发送高电平
			ioctl(g_CPLDFd,YELLOW_CONTROL_OUTPUT1_1);
			ioctl(g_CPLDFd,YELLOW_CONTROL_OUTPUT2_1);
		}
	}
	else
	{
		if(g_CPLDFd != -1)
		{
			//发送低电平
			ioctl(g_CPLDFd,YELLOW_CONTROL_OUTPUT1_0);
			ioctl(g_CPLDFd,YELLOW_CONTROL_OUTPUT2_0);
		}
	}
}
void *flash_thread(void *p)
{
	while (1)
	{
		HardflashDogCtrl(1);
		usleep(1000*50);
		HardflashDogCtrl(0);
		usleep(1000*50);
	}
}

void YelloBlink_init500()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, flash_thread, NULL))
	{
		perror("create yellow light thread fail");
		exit(1);
	}
}


/*********************************************************************************
*
* 	根据实际的键盘按钮按下的状态来点灯
*
***********************************************************************************/
void ProcessKeyBoardLight(int curkey)
{
	//auto, mannual, yellowblink, allred, step
	short ioKeyLightOn[5]={KEYBOARD_OUTPUT3_1, KEYBOARD_OUTPUT4_1, KEYBOARD_OUTPUT2_1, KEYBOARD_OUTPUT5_1, KEYBOARD_OUTPUT1_1};
	short ioKeyLightOff[5]={KEYBOARD_OUTPUT3_0, KEYBOARD_OUTPUT4_0, KEYBOARD_OUTPUT2_0, KEYBOARD_OUTPUT5_0, KEYBOARD_OUTPUT1_0};
	int i = 0;
	if(g_CPLDFd == -1)
		return;
	//全灭灯
	if(curkey == 0)
	{
		for(i=0; i<5; i++)
			ioctl(g_CPLDFd,ioKeyLightOff[i]);
		return;
	}
	//点灯
	ioctl(g_CPLDFd, ioKeyLightOn[curkey - 1]);
	//printf("点亮灯%d\n", curkey);
	//灭灯
	for(i=0; i<5; i++)
	{
		if(curkey != (i+1))
			ioctl(g_CPLDFd,ioKeyLightOff[i]);
	}
}
static unsigned char g_keyBoard_check_flag = 0;
void set_keyboard500_check_flag(int v)
{
	if(v > 0 && v < 3)
		g_keyBoard_check_flag = (v == 2 ? 0 : 1);
}

void *KeyBoard_Check(void *p)
{
	static int lastkeypressed = 0;
	int keypressed = 0;
	int curKey = 0;
	char *keyString[] = {"自动","手动","黄闪","全红","步进"};
	short ioKeyVal[5] = {KEYBOARD_INPUT3, KEYBOARD_INPUT4, KEYBOARD_INPUT2, KEYBOARD_INPUT5, KEYBOARD_INPUT1};
	int arg = 1;
	int i = 0;
	char buff[256] = {0};
	
	while (1)
	{
		if(g_keyBoard_check_flag && g_CPLDFd != -1)
		{
			for(i = 0; i<5; i++)
			{
				arg = 1;
				ioctl(g_CPLDFd, ioKeyVal[i], &arg);
				curKey = (arg == 0 ? i+1: 0);
				if(arg == 0)
				{
					//printf("===> key %s pressed!\n", keyString[i]);
					//keypressed = curKey;
					break;
				}
			}
			
			if(curKey != 0 && curKey != lastkeypressed)
			{
				INFO("手动控制面板：按键%02d[%s] 按下！\n",curKey, keyString[curKey-1]);
				memset(buff, 0, 256);
				((int *)buff)[0] = 1;
				sprintf(buff+4, "手动控制面板：按键%02d[%s] 按下！\n", curKey, keyString[curKey-1]);
				sendMessage(FTEST_MSG_KEYBOARD_DATA, buff, 4 + strlen(buff+4), &g_keyboard_servaddr);			
				keypressed = curKey;
				//lastkeypressed = keypressed;
			}
		}
		else
			keypressed = 0;
		if(keypressed != lastkeypressed)
		{
			ProcessKeyBoardLight(keypressed);
			lastkeypressed = keypressed;
		}
		
		usleep(1000*300);
	}
}

void KeybardInit500()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, KeyBoard_Check, NULL))
	{
		perror("create yellow light thread fail");
		exit(1);
	}
}

