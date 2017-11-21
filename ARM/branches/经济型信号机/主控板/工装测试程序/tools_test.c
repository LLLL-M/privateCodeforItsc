#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h> 
#include <termios.h>

#ifndef READLINE_LIBRARY
#define READLINE_LIBRARY
#endif
#include "readline.h"
#include "history.h"
#include "io_ioctl.h"
#include "debug.h"
#include "canmsg.h"

typedef void cmd_func(void);

typedef struct {
	char *name;
    cmd_func *func;
    char *doc;
}COMMAND;

int g_io_fd;
unsigned short g_lampctrl = 0;

void onekey_auto_test();
void usb_test();
void wifi_state_check();
void rs485_test();
void turn_on_all_lights();
void turn_off_all_lights();
void get_cur_and_volt();
void get_ped_key_status();
void turn_on_all_led();
void turn_off_all_led();
void print_help();
void exit_readline();
void yellow_blink_init();
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
		INFO("USB设备检测失败!");
	else
		INFO("USB设备检测通过!");
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
		INFO("Can't Open Serial Port %s", devname); 
		return -1; 
	}
	//恢复串口为阻塞状态
	if(fcntl(fd, F_SETFL, 0)<0) 
	{
		INFO("fcntl failed!"); 
	}
	//测试是否为终端设备 
	if(isatty(STDIN_FILENO)==0) 
	{
		INFO("standard input is not a terminal device"); 
	}
	
	//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
     if(tcgetattr(fd,&oldtio) != 0) 
	 {  
        INFO("SetupSerial 1"); 
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
      ERR("com set error"); 
      return -1; 
     } 
	return fd;
}

void rs485_test()
{
	int ttyS4Fd = open_port(5);
	int ttyS5Fd = open_port(6);
	char sendBuff[128] = "RS485 TEST START";
	char recvBuff[128] = {0};
	int arg;
	char succFlag0, succFlag1;
	succFlag0 = succFlag1 = 0;
	
	if (ttyS4Fd == -1 || ttyS5Fd == -1)
		return;
	//ttyS4发送，ttyS5接收
	arg = ARG_TTYS4_SEND_ENABLE;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_TTYS5_RECEIVE_ENABLE;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	write(ttyS4Fd, sendBuff, sizeof(sendBuff));
	//INFO("ttyS4 send: %s\n", sendBuff);
	sleep(1);
	read(ttyS5Fd, recvBuff, sizeof(recvBuff));
	//INFO("ttyS5 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
		succFlag0 = 1;
	else
		INFO("RS485正向测试失败！(ttyS4 send,ttyS5 recv).");
		
	//ttyS5发送，ttyS4接收
	memset(sendBuff, 0, sizeof(sendBuff));
	memset(recvBuff, 0, sizeof(recvBuff));
	strcpy(sendBuff, "RS485 TEST END");
	arg = ARG_TTYS5_SEND_ENABLE;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_TTYS4_RECEIVE_ENABLE;
	ioctl(g_io_fd, IO_SET_PIN_STATUS, &arg);
	write(ttyS5Fd, sendBuff, sizeof(sendBuff));
	//INFO("ttyS5 send: %s\n", sendBuff);
	sleep(1);
	read(ttyS4Fd, recvBuff, sizeof(recvBuff));
	//INFO("ttyS4 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(recvBuff)))
		succFlag1 = 1;
	else
		INFO("RS485反向测试失败!(ttyS5 send,ttyS4 recv).");
	
	if(succFlag0 == 1 && succFlag1 == 1)
		INFO("RS485发送接收数据测试成功!\n");
	
	close(ttyS4Fd);
	close(ttyS5Fd);
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
int cur_and_volt_check(Cur_Volt_Check_Param *curVoltParam, char isLight)
{
	unsigned short boardInfo = 0;
	unsigned short curInfo = 0;
	unsigned short curUpVal = 254;
	unsigned short curDownVal = 8;
	unsigned short curOffVal = 5;
	unsigned char isFail = 0;
	int i = 0;
	int j = 0;
	if(curVoltParam == NULL)
		return 0;

	for (i=0; i<8; i++)
	{
		for (j=0; j<4; j++)
		{
			curInfo = i_can_its_get_cur(i+1, j+1, 1);
			//INFO("第%02d通道红灯电流:%03d\n", i*4+j+1, curInfo);
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
			//INFO("第%02d通道电压状态(0:无;1:有),红灯:%d,黄灯:%d,绿灯:%d\n", i*4+j+1, (boardInfo&0x2)>>1, (boardInfo&0x4)>>2, boardInfo&0x1);
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
	return (curVoltParam->curSucFlag == 1 && curVoltParam->voltSucFlag == 1) ? 0 : 1;
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
	
	if(lightFlag != 0 && lightFlag != 1)
		return;
	memset(&curVoltParam, 0, sizeof(Cur_Volt_Check_Param));

	g_lampctrl = lightFlag;
	sleep(1);
	if(0 != cur_and_volt_check(&curVoltParam, lightFlag))
	{
		//puts("点亮灯控板所有灯电流电压检测失败！");	
		if(curVoltParam.curSucFlag != 1)
		{
			INFO("%s灯控板所有灯电流检测失败！", lightStr[lightFlag]);	
			for(i = 0; i < 32; i++)
			{
				if(curVoltParam.uCurValue[i] != 0)
					INFO("第%02d通道红灯电流检测失败，电流值:%03d\n", i, curVoltParam.uCurValue[i]);			
			}
		}
		if(curVoltParam.voltSucFlag != 1)
		{
			INFO("%s控板所有灯电压检测失败！",lightStr[lightFlag]);	
			for(i = 0; i < 32; i++)
			{
				if(curVoltParam.uVoltValue[i] != 0)
					INFO("第%02d通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:%d,黄灯:%d,绿灯:%d\n", i, (curVoltParam.uCurValue[i]&0x2)>>1, (curVoltParam.uCurValue[i]&0x4)>>2, curVoltParam.uCurValue[i]&0x1);
			}
		}
	}
	else
		INFO("%s灯控板所有灯电流电压检测成功！", lightStr[lightFlag]);	
}
void Cur_Volt_Test()
{
	cur_volt_Test_by_lightState(1);//亮灯测试
	cur_volt_Test_by_lightState(0);//灭灯测试
}

void wifi_state_check()
{
	char result[1024] = {0};
	int ret = 0;
	ret = executeCMD("ifconfig | grep -c wlan0", result, 1024);
	//system("ifconfig | grep -c wlan0");
	if(ret <= strlen("wlan0"))
		INFO("Wifi/3G模块检测失败!");
	else
		INFO("Wifi/3G模块检测通过!");
}
void get_ped_key_status()
{
	unsigned int cmd = IO_GET_PIN_STATUS;
	int arg, i;
	unsigned short value = 0;
	
	for (i = 1; i <= 8; i++)
	{
		arg = SET_ARG(i, 0);
		ioctl(g_io_fd, cmd, &arg);
		value |= ((arg & 0x1) << (i - 1));
	}
	//INFO("value = %#x", value);
	if(value == 0)
		INFO("当前无行人按钮按下！(请先按下行人按钮再输入命令获取按钮按下状态)");
	else
	{
		for(i = 0; i < 8; i++)
		{
			if(GET_BIT(value, i))		
			{
				INFO("行人按钮%d按下!", i+1);
			}
		}
	}
}

void onekey_auto_test()
{
	puts("USB测试结果:");
	usb_test();
	usleep(100000);
	puts("\nRS485-1/RS485-2测试结果:");
	rs485_test();
	usleep(100000);

	puts("\n开/关灯电流和电压测试结果:");
	Cur_Volt_Test();
	usleep(100000);

	puts("Wifi/3G模块测试结果:");
	wifi_state_check();
}
void light_on_inOrder()
{
	INFO("注意观察灯控板上的指示灯变化是否正确！");
	g_lampctrl = 2;
	while(g_lampctrl != 0)
	{
		sleep(1);	
	}
}
void font_board_led_Test()
{
	INFO("注意观察前面板上路口图中的指示灯、GPS和运行指示灯以及按键指示灯变化是否正确！");
	turn_on_all_led();
	g_lampctrl = 2;
	while(g_lampctrl != 0)
	{
		sleep(1);	
	}
	turn_off_all_led();
}
void wireless_check_on()
{
	INFO("按下遥控器上的按键观察屏幕输出是否正确！");
	set_wireless_check_flag(1);	
}
COMMAND cmds[] = {
	{"1", onekey_auto_test, "1: 一键检测（主要检测6、7、8、9项,对于检测失败的选项可以输入6/7/8/9进行单项测试）"},
	{"2", light_on_inOrder, "2: 灯控板上的指示灯依次点亮."},
	{"3", font_board_led_Test, "3: 点亮前面板上的指示灯(包括程序运行、GPS以及控制按键指示灯,前面板路口指示灯依次点亮)"},
	{"4", wireless_check_on, "4: 无线遥控器按键测试"},
	{"5", get_ped_key_status, "5: 8路行人按钮测试"},
	{"", NULL, " --------------------------------------"},

	{"6", usb_test, "6: USB测试"},
	{"7", rs485_test, "7: 2个485串口测试（一发一收）"},
	{"8", Cur_Volt_Test, "8: 开/关灯电流和电压测试"},
	{"9", wifi_state_check, "9: wifi/3G模块检测"},

	{"h", print_help, "h: 显示帮助信息"},
	{"q", exit_readline, "q: 退出工装测试程序"},
	{NULL, NULL, NULL}
};

void print_help()
{
	int n = sizeof(cmds) / sizeof(COMMAND) - 1;
	int i;
	for (i = 0; i < n; i++) {
		puts(cmds[i].doc);
	}
}

void exit_readline()
{
    exit(0);
}

static char* command_generator(const char *text, int state)
{
    const char *name;
    static int list_index, len;

    if (!state)
    {
        list_index = 0;
    }
        len = strlen(text);

    while (name = cmds[list_index].name)
    {
        list_index++;

        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    return ((char *)NULL);
}

char **command_completion(const char *text, int start, int end)
{
    char **matches = NULL;
    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    }

    return matches;
}

void execute_command(char *text)
{
    int i = 0, len = strlen(text);

    while (cmds[i].name) {
        if (strncmp(text, cmds[i].name, len) == 0)
            break;
        i++;
    }

    if (cmds[i].name)
	{
		puts(strchr(cmds[i].doc, ':') + 2);
        cmds[i].func();
		putchar('\n');
	}
    else
        INFO("输入有误，没有这个测试选项，请输入h查看帮助信息");
}

void readline_init(void)
{
    rl_readline_name = "工装测试";
    rl_attempted_completion_function = command_completion;
}

void clear_blank(char *text)
{
    char *p = text + strlen(text) - 1;
    while(isspace(*p))
        *p-- = '\0';
}

int main(void)
{
	system("killall hikTSC300");
	g_io_fd = open("/dev/gpio", O_RDWR);
	if (g_io_fd == -1) {
		perror("open /dev/gpio fail");
		return -1;
	}
	char *line = NULL;
    readline_init();
	yellow_blink_init();
	i_can_its_init();
	wireless_init();

	while(1) {
		line = readline("请输入: ");
		if (!line)
			break;

		if (*line)  {
            clear_blank(line);
            add_history(line);
            execute_command(line);
		}
		else
		{
			print_help();
			putchar('\n');
		}
		free(line);
	}
	return 0;
}
