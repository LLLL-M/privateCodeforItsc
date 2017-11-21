#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h> 


unsigned short g_nLampStatus[8] = {0};
unsigned short g_lampctrl = -1;

//rs485 ttyS4 收发使能
#define TTYS4_RECEIVE_ENABLE          0x4001   
#define TTYS4_SEND_ENABLE			  0x4000 

//rs485 ttyS5 收发使能
#define TTYS5_RECEIVE_ENABLE   		  0x5001
#define TTYS5_SEND_ENABLE             0x5000
//32个IO输入
#define IO_INPUT1_TO_INPUT8     	  0x2040  
#define IO_INPUT9_TO_INPUT16    	  0x2041 
#define IO_INPUT17_TO_INPUT24   	  0x2042 
#define IO_INPUT25_TO_INPUT32   	  0x2043 
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
typedef unsigned short  uint16;
#define DEVICE_NAME "/dev/CPLD_IO"
#define SET_BIT(v, bit) ({(v) |= (1 << (bit));}) //设置某位 
#define CLR_BIT(v, bit)	({(v) &= (~(1 << (bit)));})	//清零v的某一bit位

int cpld_io_fd;
void light_Test(void);
void Car_Detect_Test(void);
void USB_Test(void);
void main_Test(void);
static int open_port(int comport) ;
void RS232_Test(void);
void RS422_Test(void);
void RS485_1Test(void);
void RS485_2Test(void);
void IO_OUT_High_TEST(void);
void IO_OUT_Low_TEST(void);
void IO_IN_TEST(void);
/***********************************************
和调用system函数效果相同，
不同的是可以获取执行后的输出结果
cmd: 要执行命令字符串
result: 存储执行后的输出结果的buff
buffsize: 存储空间result的大小
************************************************/
int executeCMD(const char *cmd, char *result, int buffsize)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
	if(cmd == NULL || result == NULL)
	{
		printf("cmd or result is NULL\n");
		return -1;
	}
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r")) != NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
           if((strlen(result) + strlen(buf_ps)) >= buffsize)
               break;
           strncat(result, buf_ps, 1024);
        } 
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
	return strlen(result);
}

void usb_test()
{
	//system("ls -l /mnt");
	char result[1024] = {0};
	int ret = 0;
	ret = executeCMD("ls -l /mnt", result, 1024);
	if(ret <= strlen("total 0")+1)
		printf("USB设备检测失败!\n");
	else
		printf("USB设备检测通过!\n");
}
void PhaseLampOutput(int boardNum, unsigned short Lampstate)
{
	static int iChanLevel = 0;//代表每块板子的第几通道，取值范围1~6 
	static int iChannel = 0;//通道号
	static int lightCloor = 0;//红1黄2绿0
	for(iChanLevel = 0; iChanLevel < 6; iChanLevel++)
	{
		do{
			lightCloor++;
			if(lightCloor > 2)
			{
				lightCloor = 0;
			}
			for(iChannel = iChanLevel; iChannel < 32; iChannel += 6)
			{
				SET_BIT(g_nLampStatus[iChannel/4], (iChannel%4)*3 + lightCloor);
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

typedef struct cur_volt_check{
	char curSucFlag; //电流检测成功标志
	char voltSucFlag; //电压检测成功标志
	unsigned short uCurValue[32];	//检测失败的电流值
	unsigned short uVoltValue[32];	//检测失败的电压值
}Cur_Volt_Check_Param;
/***************************************************************
对32个通道进行电流和电压值检测
curVoltParam: 外部传入，用于存储检测失败的电流电压值
isLight: 当前灯的状态:点亮还是熄灭，用于检测判断
返回值： 检测成功 - 0	检测有失败通道 -  1
****************************************************************/
int cur_and_volt_check(Cur_Volt_Check_Param *curVoltParam, const char isLight)
{
	unsigned short boardInfo = 0;
	unsigned short curInfo = 0;
	const unsigned short curUpVal = 254;
	const unsigned short curDownVal = 8;
	const unsigned short curOffVal = 5;
	unsigned char isFail = 0;
	int i = 0;
	int j = 0;
	if(curVoltParam == NULL)
	{
		printf("参数错误\n");
		return -1;
	}	

	for (i=0; i<8; i++)
	{
		for (j=0; j<4; j++)
		{
			curInfo = i_can_its_get_cur(i+1, j+1, 1);
			//printf("第%02d通道红灯电流:%03d\n", i*4+j+1, curInfo);
			if((isLight == 1 && (curInfo > curUpVal || curInfo < curDownVal))
					|| (isLight == 0 && (curInfo > curOffVal)))
			{
				isFail = 1;	
				curVoltParam->uCurValue[i*4+j] = curInfo;
			}
		}
	}
	if(isFail != 1)
		curVoltParam->curSucFlag = 1;
	isFail = 0;
	for (i=0; i<8; i++)
	{
		i_can_its_get_Volt(i+1, &boardInfo);
		for (j=0; j<4; j++)
		{
			//printf("第%02d通道电压状态(0:无;1:有),红灯:%d,黄灯:%d,绿灯:%d\n", i*4+j+1, (boardInfo&0x2)>>1, (boardInfo&0x4)>>2, boardInfo&0x1);
			if((isLight == 1 && (((boardInfo&0x2)>>1 == 0) /*|| ((boardInfo&0x4)>>2 == 0) */|| (boardInfo&0x1 == 0)))
					|| (isLight == 0 && (((boardInfo&0x2)>>1 == 1) /*|| ((boardInfo&0x4)>>2 == 1) */|| (boardInfo&0x1 == 1))))//黄灯不检测，默认为1
			{
				isFail = 1;
				curVoltParam->uVoltValue[i*4+j] = boardInfo;
			}
			boardInfo >>= 3;
		}
	}
	if(isFail != 1)
		curVoltParam->voltSucFlag = 1;
	return isFail;
}
/**************************************
  灯控板开关灯测试函数
lightFlag: 开关灯标志，0 - 灭灯测试，1 - 亮灯测试
  ************************************/
void cur_volt_Test_by_lightState(char lightFlag)
{
	Cur_Volt_Check_Param curVoltParam;
	int i = 0;
	char *lightStr[2]={"熄灭", "点亮"};

	memset(&curVoltParam, 0, sizeof(Cur_Volt_Check_Param));

	if(lightFlag == 1)
	{
		//点亮
		memset((char*)g_nLampStatus, 0xffff, sizeof(g_nLampStatus));	
		i_can_its_send_led_request(1, g_nLampStatus);
	}
	else if(lightFlag == 0)
	{
		//灭灯
		memset((char*)g_nLampStatus, 0, sizeof(g_nLampStatus));	
		i_can_its_send_led_request(1, g_nLampStatus);
	}
	else
		return;
	sleep(1);
	if(0 != cur_and_volt_check(&curVoltParam, lightFlag))
	{
		if(curVoltParam.curSucFlag != 1)
		{
			printf("%s灯控板所有灯电流检测失败！\n", lightStr[lightFlag]);	
			for(i = 0; i < 32; i++)
			{
				if(curVoltParam.uCurValue[i] != 0)
					printf("第%02d通道红灯电流检测失败，电流值:%03d\n", i, curVoltParam.uCurValue[i]);			
			}
		}
		if(curVoltParam.voltSucFlag != 1)
		{
			printf("%s控板所有灯电压检测失败！\n",lightStr[lightFlag]);	
			for(i = 0; i < 32; i++)
			{
				if(curVoltParam.uVoltValue[i] != 0)
					printf("第%02d通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:%d,黄灯:%d,绿灯:%d\n", i, (curVoltParam.uCurValue[i]&0x2)>>1, (curVoltParam.uCurValue[i]&0x4)>>2, curVoltParam.uCurValue[i]&0x1);
			}
		}
	}
	else
		printf("%s灯控板所有灯电流电压检测成功！\n", lightStr[lightFlag]);	
}
void Cur_Volt_Test()
{
	cur_volt_Test_by_lightState(1);//亮灯测试
	cur_volt_Test_by_lightState(0);//灭灯测试
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

void create_flash_thread()
{
	pthread_t thread;
	if (0 != pthread_create(&thread, NULL, flash_thread, NULL))
	{
		//log error
	}
}

void Car_Detect_Test(void)
{
	uint16 boardInfo = 0;
	int boardNum = 0;//取值1,2,3
	int ibit = 0;//位移量
	int Car_State = 0;
	printf("1为有车，0为无车\n");
	for(boardNum = 1;boardNum<4;boardNum++)
	{
		boardInfo = recv_date_from_vechile(boardNum);
		for(ibit=0;ibit<16;ibit++)
		{
			Car_State = ((boardInfo>>ibit) & 0x01);
			printf("第%d通道,过车状态:%d\n",(boardNum-1)*16+ibit+1,Car_State);
		}
	}
}

/* 串口设置为:
	波特率：9600， 8bit
	停止位：1
	奇偶校验位：N	*/
static int open_port(int comport) 
{ 
	char devname[40]={0}; 
	struct termios newtio,oldtio;
	int fd = -1;
	
	sprintf(devname, "/dev/ttyS%d", comport - 1);
	fd = open(devname, O_RDWR|O_NOCTTY|O_NDELAY); 
	if (-1 == fd)
	{ 
		printf("Can't Open Serial Port %s", devname); 
		return -1; 
	}
	//恢复串口为阻塞状态
	if(fcntl(fd, F_SETFL, 0)<0) 
	{
		printf("fcntl failed!"); 
	}
	//测试是否为终端设备 
	if(isatty(STDIN_FILENO)==0) 
	{
		printf("standard input is not a terminal device"); 
	}
	
	//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
     if(tcgetattr(fd,&oldtio) != 0) 
	 {  
        printf("SetupSerial 1"); 
		return -1; 
     } 
     memset(&newtio, 0, sizeof(newtio)); 
	 //步骤一，设置字符大小
     newtio.c_cflag  |=  CLOCAL | CREAD;  
     newtio.c_cflag &= ~CSIZE; 
	 newtio.c_cflag |= CS8;	//设置停止位 
	 newtio.c_cflag &= ~PARENB;	//设置奇偶校验位 
	 //设置波特率
	 cfsetispeed(&newtio, B9600); 
	 cfsetospeed(&newtio, B9600); 	 
	 /*设置停止位*/ 
      newtio.c_cflag &=  ~CSTOPB; 
	/*设置等待时间和最小接收字符*/ 
     newtio.c_cc[VTIME]  = 0; 
     newtio.c_cc[VMIN] = 0; 
	/*处理未接收字符*/ 
     tcflush(fd,TCIFLUSH); 
	/*激活新配置*/ 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0) 
     { 
      printf("com set error"); 
      return -1; 
     } 
	return fd;
}
void RS232_Test(void)
{
	int fd = 0; 
	int nwrite = 0;
	char sendBuff[1024] = "RS232 TEST SEND";
    char recvBuff[1024]=""; 
	//打开串口 
   	if((fd=open_port(2))<0)
	{
	    perror("open_port error"); 
        return ; 
    } 

   	if(nwrite = write(fd,sendBuff,1024)>0)
	{
		//printf("串口232输出成功\n");
	}
	else
	{
		printf("串口232输出失败\n");
	}
	sleep(1);
	read(fd, recvBuff, sizeof(recvBuff));
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
	{
		//printf("RS232接收成功!\n");
	}
	else
		printf("RS232测试失败!\n");
}
void RS422_Test(void)
{
	int fd = 0; 
 	int nwrite = 0;
	char sendBuff[1024] = "RS422 TEST SEND";
   	char recvBuff[1024]=""; 
	//打开串口 
   	if((fd=open_port(4))<0)
	{
        perror("open_port error"); 
    	return ; 
   	} 

	if(nwrite = write(fd,sendBuff,1024)>0)
	{
		//printf("串口422输出成功\n");
	}
	else
	{
		printf("串口422输出失败\n");
	}
	sleep(1);
	read(fd, recvBuff, sizeof(recvBuff));
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
	{
		//printf("RS422接收成功!\n");
	}
	else
		printf("RS422测试失败!\n");
	close(fd);
}
void rs485_test()
{
	int ttyS4Fd = open_port(5);
	int ttyS5Fd = open_port(6);
	char sendBuff[128] = "RS485 TEST START";
	char recvBuff[128] = {0};
	int arg;
	int g_io_fd = 0;
	char succFlag0, succFlag1;
	succFlag0 = succFlag1 = 0;
	
	if (ttyS4Fd == -1 || ttyS5Fd == -1)
		return;
	//ttyS4发送，ttyS5接收
	arg = TTYS4_SEND_ENABLE;
	ioctl(g_io_fd, TTYS4_SEND_ENABLE, &arg);
	arg = TTYS5_RECEIVE_ENABLE;
	ioctl(g_io_fd, TTYS5_RECEIVE_ENABLE, &arg);
	write(ttyS4Fd, sendBuff, sizeof(sendBuff));
	//INFO("ttyS4 send: %s\n", sendBuff);
	sleep(1);
	read(ttyS5Fd, recvBuff, sizeof(recvBuff));
	//INFO("ttyS5 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
		succFlag0 = 1;
	else
		printf("RS485正向测试失败！(ttyS4 send,ttyS5 recv)\n");
		
	//ttyS5发送，ttyS4接收
	memset(sendBuff, 0, sizeof(sendBuff));
	memset(recvBuff, 0, sizeof(recvBuff));
	strcpy(sendBuff, "RS485 TEST END");
	arg = TTYS5_SEND_ENABLE;
	ioctl(g_io_fd, TTYS5_SEND_ENABLE, &arg);
	arg = TTYS4_RECEIVE_ENABLE;
	ioctl(g_io_fd, TTYS4_RECEIVE_ENABLE, &arg);
	write(ttyS5Fd, sendBuff, sizeof(sendBuff));
	//INFO("ttyS5 send: %s\n", sendBuff);
	sleep(1);
	read(ttyS4Fd, recvBuff, sizeof(recvBuff));
	//INFO("ttyS4 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
		succFlag1 = 1;
	else
		printf("RS485反向测试失败!(ttyS5 send,ttyS4 recv)\n");
	
	if(succFlag0 == 1 && succFlag1 == 1)
		printf("RS485发送接收数据测试成功!\n");
	
	close(ttyS4Fd);
	close(ttyS5Fd);
}
void wifi_state_check()
{
	char result[1024] = {0};
	int ret = 0;
	ret = executeCMD("ifconfig | grep -c wlan0", result, 1024);
	//system("ifconfig | grep -c wlan0");
	if(ret <= strlen("wlan0"))
		printf("Wifi/3G模块检测失败!\n");
	else
		printf("Wifi/3G模块检测通过!\n");
}
void IO_OUT_High_TEST(void)
{
	int fd = 0;
	int arg = -1;
	
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
		return;
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
}
void IO_OUT_Low_TEST(void)
{
	int fd = 0;
	int arg = -1;
	
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
		return;
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
}

void IO_IN_TEST(void)
{
	int fd = 0;
	int arg = -1;	
	int bit0 = 0;
	int bit1 = 0;
	int bit2 = 0;
	int bit3 = 0;
	int bit4 = 0;
	int bit5 = 0;
	int bit6 = 0;
	int bit7 = 0;

	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		printf("Open device %s error!\n",DEVICE_NAME);
	}
	ioctl(fd,IO_INPUT1_TO_INPUT8,&arg);
	bit0 = arg & 0x01;
	bit1 = (arg & 0x02)>>1;
	bit2 = (arg & 0x04)>>2;
	bit3 = (arg & 0x08)>>3;
	bit4 = (arg & 0x10)>>4;
	bit5 = (arg & 0x20)>>5;
	bit6 = (arg & 0x40)>>6;
	bit7 = (arg & 0x80)>>7;
	printf("IO1-IO8   Input: %d %d %d %d %d %d %d %d\n",bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7);
	ioctl(fd,IO_INPUT9_TO_INPUT16,&arg);
	bit0 = arg & 0x01;
	bit1 = (arg & 0x02)>>1;
	bit2 = (arg & 0x04)>>2;
	bit3 = (arg & 0x08)>>3;
	bit4 = (arg & 0x10)>>4;
	bit5 = (arg & 0x20)>>5;
	bit6 = (arg & 0x40)>>6;
	bit7 = (arg & 0x80)>>7;
	printf("IO9-IO16  Input: %d %d %d %d %d %d %d %d\n",bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7);
	if(!bit0)
	{
		printf("全红\n");
	}
	else if(!bit1)
	{
		printf("黄闪\n");
	}
	else if(!bit2)
	{
		printf("步进\n");
	}
	else if(!bit3)
	{
		printf("自动\n");
	}
	else if(!bit4)
	{
		printf("手动\n");
	}
	ioctl(fd,IO_INPUT25_TO_INPUT32,&arg);
	bit0 = arg & 0x01;
	bit1 = (arg & 0x02)>>1;
	bit2 = (arg & 0x04)>>2;
	bit3 = (arg & 0x08)>>3;
	bit4 = (arg & 0x10)>>4;
	bit5 = (arg & 0x20)>>5;
	bit6 = (arg & 0x40)>>6;
	bit7 = (arg & 0x80)>>7;
	printf("IO25-IO32 Input: %d %d %d %d %d %d %d %d\n",bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7);
	close(fd);
}
void onekey_auto_test(void)
{
	printf("[USB 测试]\n");
	usb_test();
	usleep(1000*100);
	printf("RS232检测\n");
	RS232_Test();
	usleep(1000*100);
	printf("RS422检测\n");
	RS422_Test();
	usleep(1000*100);
	printf("[RS485检测]\n");
	rs485_test();
	usleep(1000*100);
	printf("[20输出低电平]\n");
	IO_OUT_Low_TEST();
	usleep(1000*100);
	printf("[20输出高电平]\n");
	IO_OUT_High_TEST();
	usleep(1000*100);
	printf("[WIFI/3G测试]\n");
	wifi_state_check();
	usleep(1000*100);
	printf("[电压电流测试]\n");
	Cur_Volt_Test();
	usleep(1000*100);
}

int main()
{
	system("killall hikTSC500");
	canits_init();
	CPLD_IO_Init();
	create_flash_thread();
	
	while (1)
	{
		int inChar = 0;
		printf("\n请输入\n 1:一键测试(包含测试项5,6,7,8,9,a,b,c);\n 2:灯控板指示灯依次点亮;\n 3:32路输入测试(9-13为遥控器接收，17-24无)\n 4:车检板测试;\
		\n...............................\n 5:USB测试;\n 6:RS232测试;\n 7:RS422测试;\n 8:RS485测试\n 9:20个IO输出低电平\
		\n a:20IO输出高电平;\n b:WIFI/3G测试;\n c:电压电流测试\n...............................\n q:退出\n");
		inChar = getchar();
		while (getchar() != '\n'); 
		printf("你输入的命令是: %c\n",inChar);
		switch(inChar)
		{
			case '1':
				printf("一键测试(包含测试项5,6,7,8,9,a,b,c)\n");
				onekey_auto_test();
				break;
			case '2':
				printf("灯控板指示灯依次点亮\n");
				PhaseLampOutput(1, g_lampctrl);
				break;
			case '3':
				printf("[32输入(9-13遥控器接收，17-24无),0有效，1无效]\n");
				IO_IN_TEST();
				break;
			case '4':
				printf("车检板测试\n");
				Car_Detect_Test();
				break;
			case '5':
				printf("[USB 测试]\n");
				usb_test();
				break;
			case '6':
				printf("[RS232检测]\n");
				RS232_Test();
				break;
			case '7':
				printf("[RS422检测]\n");
				RS422_Test();
				break;
			case '8':
				printf("[RS485检测]\n");
				rs485_test();
				break;
			case '9':
				printf("[20IO输出低电平]\n");
				IO_OUT_Low_TEST();
				break;
			case 'a':
				printf("[20IO输出高电平]\n");
				IO_OUT_High_TEST();
				break;
			case 'b':
				printf("[WIFI/3G测试]\n");
				wifi_state_check();
				break;
			case 'c':
				printf("[电压电流测试]\n");
				Cur_Volt_Test();
				break;
			case 'q':
				printf("程序已退出！\n");
				exit(0);
				break;
			default:
				break;
		}
		
		
	}
	return 0;
}
