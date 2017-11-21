#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "io_ioctl.h"
#include "debug.h"

#include "ftest.h"
#include "common.h"
#include "canmsg.h"
#include "cpld.h"
#include "gps.h"

int g_io_fd;
unsigned short g_lampctrl = 0;
unsigned char g_boardnumBit = 0;
extern int gTsctype;
extern int g_CPLDFd;

int OpenSysDev(void)
{
	g_io_fd = open(GPIO_DEV, O_RDWR);
	if(-1 == g_io_fd)
	{
		perror("open gpio device failed.");
		return -1;
	}
	return 0;
}
void TscProcessCtrl(char cmd)
{
	char *cmds[] = {"killall hikTSC300", "/root/hikTSC300 &", "killall hikTSC500", "/root/hikTSC500 &"};
	int cmdIndex = (gTsctype == TSC_TYPE_300 ? cmd : cmd +2);
	
	if(0 == cmd || 1 == cmd)
	{
		system(cmds[cmdIndex]);
	}
}
int usb_test(char *result)
{
	//system("ls -l /mnt");
	char buff[1024] = {0};
	int ret = 0;
	ret = executeCMD("ls /mnt/usb/", buff, 1024);
	if(ret <= 0 || strstr(buff, "No such file") != NULL)
	{
		memcpy(result, "USB设备检测失败!\n", sizeof("USB设备检测失败!\n"));
		ret = 0;
	}
	else
	{
		memcpy(result, "USB设备检测通过!\n", sizeof("USB设备检测通过!\n"));
		ret = 1;
	}
	usleep(100*1000);
	return ret;
}
int rs485_test(char *result)
{
	int ret = 0;
	int len = 0;
	int fd =  g_io_fd;
	int ttyS4Fd = open_port(5);
	int ttyS5Fd = open_port(6);
	char sendBuff[128] = "RS485 TEST01 SEND";
	char recvBuff[128] = {0};
	int arg;
	char succFlag0, succFlag1;
	succFlag0 = succFlag1 = 0;
	char *pos = result;
	
	if (ttyS4Fd < 0 || ttyS5Fd < 0)
		return ret;
	//ttyS4发送，ttyS5接收
	arg = ARG_TTYS4_SEND_ENABLE;
	ioctl(fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_TTYS5_RECEIVE_ENABLE;
	ioctl(fd, IO_SET_PIN_STATUS, &arg);
	write(ttyS4Fd, sendBuff, strlen(sendBuff));
	INFO("ttyS4 send: %s\n", sendBuff);
	sleep(1);
	while(!(len = read(ttyS5Fd, recvBuff, strlen(sendBuff))));
	recvBuff[len] = '\0';
	//read(ttyS5Fd, recvBuff, sizeof(recvBuff));
	INFO("ttyS5 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(sendBuff)))
		succFlag0 = 1;
	else
	{
		len = strlen("RS485正向测试失败！（ttyS4发送，ttyS5接收）\n");
		memcpy(pos, "RS485正向测试失败！（ttyS4发送，ttyS5接收）\n", len);
		pos += len;
	}
		
	//ttyS5发送，ttyS4接收
	memset(sendBuff, 0, sizeof(sendBuff));
	memset(recvBuff, 0, sizeof(recvBuff));
	strcpy(sendBuff, "RS485 TEST02 SEND");
	arg = ARG_TTYS5_SEND_ENABLE;
	ioctl(fd, IO_SET_PIN_STATUS, &arg);
	arg = ARG_TTYS4_RECEIVE_ENABLE;
	ioctl(fd, IO_SET_PIN_STATUS, &arg);
	write(ttyS5Fd, sendBuff, strlen(sendBuff));
	INFO("ttyS5 send: %s\n", sendBuff);
	sleep(1);
	while(!(len = read(ttyS4Fd, recvBuff, strlen(sendBuff))));
	recvBuff[len] = '\0';
	//read(ttyS4Fd, recvBuff, sizeof(recvBuff));
	INFO("ttyS4 recv: %s\n", recvBuff);
	if(strlen(recvBuff) && !strncmp(sendBuff, recvBuff, strlen(sendBuff)))
		succFlag1 = 1;
	else
	{
		memcpy(pos, "RS485反向测试失败！（ttyS5发送,ttyS4接收）\n", strlen("RS485反向测试失败！（ttyS5发送,ttyS4接收）\n"));
		//len += strlen("RS485反向测试失败！（ttyS5发送,ttyS4接收）\n");
		//INFO("RS485鍙嶅悜娴嬭瘯澶辫触!(ttyS5 send,ttyS4 recv).");
	}
	
	if(succFlag0 == 1 && succFlag1 == 1)
	{
		memcpy(pos, "RS485发送接收数据测试成功！\n", strlen("RS485发送接收数据测试成功！\n"));
		//INFO("RS485鍙戦€佹帴鏀舵暟鎹祴璇曟垚鍔?\n");
		ret = 1;
	}
	
	close(ttyS4Fd);
	close(ttyS5Fd);
	return ret;
}
int TSC_RS485_Test(char *result)
{
	return (gTsctype == TSC_TYPE_300 ? rs485_test(result) : RS485_test500(result));
}

/*************** cur and volt test ***************/
typedef struct cur_volt_check{
	char curSucFlag; //电流检测成功标志
	char voltSucFlag; //电压检测成功标志
	int curFailChans;	//电流检测出失败的通道,每一位代表一个通道
	int voltFailChans;  //电压检测出失败的通道
	unsigned short uCurValue[32];	//检测失败的电流值
	unsigned short uVoltValue[32];	//检测失败的电压值
}Cur_Volt_Check_Param;
/***************************************************************
对32个通道进行电流和电压值检测
curVoltParam: 外部传入，用于存储检测失败的电流电压值
isLight: 当前灯的状态:点亮还是熄灭，用于检测判断
返回值： 检测成功 - 0	检测有失败通道 -  1
****************************************************************/
static int cur_and_volt_check(Cur_Volt_Check_Param *curVoltParam, char isLight)
{
	int chanNum = (gTsctype == TSC_TYPE_300 ? 4 : 6);
	unsigned short boardInfo = 0;
	unsigned short curInfo = 0;
	unsigned short curUpVal = 254;
	unsigned short curDownVal = 8;
	unsigned short curOffVal = 5;
	unsigned char isFail = 0;
	int i = 0;
	int j = 0;
	int bn = 0;
	int ch = 0;
	int start = 0;
	int end = 0;
	int index0 = 0;
	int index1 = 0;
	if(curVoltParam == NULL)
		return 0;

	for (bn=0; bn<MAX_BOARD_NUM; bn++)	// max 6 boards 
	{
		if(!GET_BIT(g_boardnumBit, bn))
			continue;
		//i=bn*chanNum/4;
		for (ch=0; ch<chanNum; ch++)
		{
			if((bn * chanNum + ch) > 31)
				continue;
			i = (bn*chanNum + ch)/4;
			j = (bn*chanNum + ch)%4;
			curInfo = (gTsctype == TSC_TYPE_300 ? i_can_its_get_cur_300(i+1, j+1, 1) : i_can_its_get_cur(i+1, j+1, 1));
			INFO("第%02d通道红灯电流:%03d\n", i*4+j+1, curInfo);
			if((isLight == 1 && (curInfo > curUpVal || curInfo < curDownVal))
					|| (isLight == 0 && (curInfo > curOffVal)))
			{
				isFail = 1;	
				//SET_BIT(curVoltParam->curFailChans, bn*chanNum + ch);
				//curVoltParam->uCurValue[bn*chanNum + ch] = curInfo;
				SET_BIT(curVoltParam->curFailChans, i*4+j);
				curVoltParam->uCurValue[i*4+j] = curInfo;
			}
		}
	}
	if(isFail != 1)
		curVoltParam->curSucFlag = 1;
	isFail = 0;
	for (bn=0; bn<MAX_BOARD_NUM; bn++) 
	{
		if(!GET_BIT(g_boardnumBit, bn))
			continue;
		//gTsctype == TSC_TYPE_300 ? i_can_its_get_Volt_300(i+1, &boardInfo) : i_can_its_get_Volt(i+1, &boardInfo);
//		for (ch=0; ch<chanNum; ch++)
		index0 = (gTsctype == TSC_TYPE_300 ? bn : (bn*chanNum/4));
		index1 = (gTsctype == TSC_TYPE_300 ? index0 : (index0 + 1));
		for (i=index0; i<=index1; i++)
		{
			//i = (bn*chanNum + ch)/4;
			if(i == 8)
				continue;
			gTsctype == TSC_TYPE_300 ? i_can_its_get_Volt_300(i+1, &boardInfo) : i_can_its_get_Volt(i+1, &boardInfo);
			if(gTsctype == TSC_TYPE_500 && (i==1 || i==4 || i==7))
			{
				if(bn%2 == 0)
				{
					start = 0;
					end = 2;
				}
				else
				{
					start = 2;
					end = 4;
					boardInfo >>= 3;
					boardInfo >>= 3;
				}
			}
			else
			{
				start = 0;
				end = 4;
			}
			for(j=start; j<end; j++)
			{
				if((isLight == 1 && (((boardInfo&0x2)>>1 == 0) /*|| ((boardInfo&0x4)>>2 == 0) */|| (boardInfo&0x1 == 0)))
					|| (isLight == 0 && (((boardInfo&0x2)>>1 == 1) /*|| ((boardInfo&0x4)>>2 == 1) */|| (boardInfo&0x1 == 1))))//黄灯不检测，默认为1
				{
					isFail = 1;
					SET_BIT(curVoltParam->voltFailChans, i*4 + j);
					curVoltParam->uVoltValue[i*4+j] = boardInfo;
				}
				INFO("第%02d通道电压状态(0:无;1:有),红灯:%d,黄灯:%d,绿灯:%d\n", i*4+j+1, (boardInfo&0x2)>>1, (boardInfo&0x4)>>2, boardInfo&0x1);
				boardInfo >>= 3;
			}
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
static int cur_volt_Test_by_lightState(char *result, char lightFlag)
{
	Cur_Volt_Check_Param curVoltParam;
	int i = 0;
	char *lightStr[2]={"熄灭", "点亮"};
	//int length = 0;
	int ret = 0;
	char *p = result;
	
	if(lightFlag != 0 && lightFlag != 1)
		return ret;
	memset(&curVoltParam, 0, sizeof(Cur_Volt_Check_Param));

	if(TSC_TYPE_300 == gTsctype)
	{
		g_lampctrl = (lightFlag == 1 ? 2: 0);
	}
	else
	{
		Lamp_light_ctrl(lightFlag);
	}
	sleep(3);
	if(0 != cur_and_volt_check(&curVoltParam, lightFlag))
	{
		//puts("点亮灯控板所有灯电流电压检测失败！");	
		if(curVoltParam.curSucFlag != 1)
		{
			//INFO("%s灯控板所有灯电流检测失败！", lightStr[lightFlag]);	
			sprintf(p, "%s灯控板所有灯电流检测失败！\n", lightStr[lightFlag]);	
			p += strlen(p);
			for(i = 0; i < 32; i++)
			{
				//if(curVoltParam.uCurValue[i] != 0)
				if(GET_BIT(curVoltParam.curFailChans, i))
				{
					sprintf(p, "第%02d通道红灯电流检测失败，电流值:%03d\n", i+1, curVoltParam.uCurValue[i]);			
					//p += strlen("第01通道红灯电流检测失败，电流值:003\n");
					//INFO("one fail cur len: %d", strlen(p));
					p += strlen(p);
					//INFO("第%02d通道红灯电流检测失败，电流值:%03d\n", i, curVoltParam.uCurValue[i]);			
					//INFO("cur one len: %d\n", strlen("第02通道红灯电流检测失败，电流值:001\n"));
				}
			}
		}
		if(curVoltParam.voltSucFlag != 1)
		{
			sprintf(p, "%s灯控板所有灯电压检测失败！\n",lightStr[lightFlag]);	
			p += strlen(p);
			//length += strlen("熄灭控板所有灯电压检测失败！")
			//INFO("%s控板所有灯电压检测失败！",lightStr[lightFlag]);	
			for(i = 0; i < 32; i++)
			{
				//if(curVoltParam.uVoltValue[i] != 0)
				if(GET_BIT(curVoltParam.voltFailChans, i))
				{
					sprintf(p, "第%02d通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:%d,黄灯:%d,绿灯:%d\n", i+1, (curVoltParam.uVoltValue[i]&0x2)>>1, (curVoltParam.uVoltValue[i]&0x4)>>2, curVoltParam.uVoltValue[i]&0x1);
					//INFO("on fail volt len: %d", strlen(p));
					p += strlen(p);
					//p += strlen("第02通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:1,黄灯:1,绿灯:1\n");
					//INFO("第%02d通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:%d,黄灯:%d,绿灯:%d\n", i, (curVoltParam.uCurValue[i]&0x2)>>1, (curVoltParam.uCurValue[i]&0x4)>>2, curVoltParam.uCurValue[i]&0x1);
					//INFO("volt one len: %d\n", strlen("第02通道电压状态(0:无;1:有)检测失败, 电压状态值：红灯:1,黄灯:1,绿灯:0\n"));
				}
			}
		}
	}
	else
	{
		sprintf(p, "%s灯控板所有灯电流电压检测成功！\n", lightStr[lightFlag]);	
		ret = 1;
		//INFO("%s灯控板所有灯电流电压检测成功！", lightStr[lightFlag]);	
	}
	return ret;
}
int updateBoardNum(char *data)
{
	int i = 0;
	int *states = (int*)data;
	//获取需测试的灯控板编号
	g_boardnumBit = 0;
	for(i=0; i<6; i++)
	{
		if(states[i])
			SET_BIT(g_boardnumBit, i);
	}
	return g_boardnumBit;
}
int cur_Volt_Test(char *data)
{
	char *result = data +4;
	int ret = 1;
	char buff[1024*5]={0};
	int len = 0;

	if(0 == updateBoardNum(data))
	{
		usleep(100*1000);
		return ret;
	}
	ret &= cur_volt_Test_by_lightState(buff, 1);//亮灯测试
	len = strlen(buff);
	memcpy(result, buff, len);
	//printf("len: %d, retStr0: %s\n", strlen(buff), buff);
	sleep(1);
	memset(buff, 0, 1024*5);
	ret &= cur_volt_Test_by_lightState(buff, 0);//灭灯测试
	memcpy(result + len,  buff, strlen(buff));
	//printf("len: %d, retStr1: %s\n", strlen(buff), buff);
	
	printf("retStr: %s\n", result);
	return ret;
}
/**************end of cur and volt test************/
int wifi_state_check(char *result)
{
	char outputbuff[1024] = {0};
	int ret = 0;
	ret = executeCMD("ifconfig | grep -c wlan0", outputbuff, 1024);
	//system("ifconfig | grep -c wlan0");
	if(ret <= strlen("wlan0"))
	{
		memcpy(result, "Wifi/3G模块检测失败!\n", strlen("Wifi/3G模块检测失败!\n"));
		ret = 0;
	}
	else
	{
		memcpy(result, "Wifi/3G模块检测通过!\n", strlen("Wifi/3G模块检测通过!\n"));
		ret = 1;
	}
	usleep(100 * 1000);//100ms
	return ret;
}
void light_on_inOrder(char *data)
{
	if(0 == updateBoardNum(data))
	{
		usleep(100*1000);
		return;
	}
	if(TSC_TYPE_300 == gTsctype)
	{
		//INFO("注意观察灯控板上的指示灯变化是否正确！");
		g_lampctrl = 3;
		while(g_lampctrl != 0)
		{
			sleep(1);	
		}
	}
	else
		Lamp_light_ctrl(2);
//		PhaseLampOutput(1);
}
void font_board_led_Test()
{
	//INFO("注意观察前面板上路口图中的指示灯、GPS和运行指示灯以及按键指示灯变化是否正确！");
	sleep(2);//应产线需求，开始测试前延迟2s
	turn_on_all_led();
	g_boardnumBit = 0xff;
	g_lampctrl = 3;
	while(g_lampctrl != 0)
	{
		sleep(1);	
	}
	turn_off_all_led();
}
void wireless_check_ctrl(int flag)
{
	//INFO("按下遥控器上的按键观察屏幕输出是否正确！");
	if(WIRELESS_CHECK_ON == flag || WIRELESS_CHECK_OFF == flag)
		set_wireless_check_flag(flag);	
}
void get_ped_key_status(int *result)
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
	//if(value == 0)
		//INFO("当前无行人按钮按下！(请先按下行人按钮再输入命令获取按钮按下状态)");
	//else
	if(0 != value)
	{
		for(i = 0; i < 8; i++)
		{
			if(GET_BIT(value, i))		
			{
				INFO("行人按钮%d按下!", i+1);
				result[i] = 1;
			}
		}
	}
}
int get_tsc_type(void)
{
	int i = -1;
	char buff[1024] = {0};
	int ret = 0;
	char *pos = NULL;
	char *driverStr[] = {"io", "CPLD"};
	ret = executeCMD("lsmod", buff, 1024);
	if(ret != 0 && (pos = strstr(buff, "Not tainted")) != NULL)
	{
		pos += strlen("Not tainted") + 1;
		
		if(strstr(pos, driverStr[0]))
			ret = TSC_TYPE_300;
		else if(strstr(pos, driverStr[1]))
			ret = TSC_TYPE_500;
		else
		{
			ret = 0;
			printf("Can't get the type of TSC!\n");
		}
	}
	
	return ret;
}
//黄闪心跳：黄闪板收到心跳信号就不会输出信号控制灯控板，如果没有心跳信号就会强制控制灯控板黄闪
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
void YellowBlink_init300()
{
	pthread_t threadId;
	if (pthread_create(&threadId, NULL, yellow_blink_thread, NULL)) {
		perror("create yellow light thread fail");
		exit(1);
	}
	pthread_detach(threadId);
	//INFO("yellow light thread init successful!");
}
void Key_board_check_ctrl(int val)
{
	if(TSC_TYPE_300 == gTsctype)
		set_keyboard_check_flag(val);
	else
		set_keyboard500_check_flag(val);
}


static int initFlag = 0;
void Ftest_init(void)
{
	
	if(initFlag == 1)
		return;
	TscProcessCtrl(TSC_PROCESS_SHUTDOWN);//关闭信号机程序
	if(TSC_TYPE_300 == gTsctype)
	{
		OpenSysDev();//打开gpio设备
		i_can_its_init_300();//can通信初始化
		wireless_init();//无线遥控器初始化
		YellowBlink_init300();
		KeyBoardInit();
	}
	else{
		canits_init();
		CPLD_IO_Init();
		IO_Input_init();
		YelloBlink_init500();
		KeybardInit500();
	}
	GPS_init();
	initFlag = 1;
}
void Ftest_finished(void)
{
	
	TscProcessCtrl(TSC_PROCESS_BOOT);//启动信号机程序
}
