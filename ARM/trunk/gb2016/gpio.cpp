#ifdef HARDWARE_MODEL
#include <cstring>
#include <cstdlib>
#include <termios.h>
#include <errno.h>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE    //对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <ctime>
#include <cstdlib>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <errno.h>
#include "gpio.h"
#include "can.h"
#include "singleton.h"
#include "log.h"

int Gpio::iofd = -1;

int Gpio::Opentty(int port, int baudRate, int dataBits, int stopBits, char parity)
{
	char device[12] = {0};
	struct termios options;
	int fd = -1;
	
	sprintf(device, "/dev/ttyS%d", port);
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		ERR("open %s fail!\n", device);
		return -1;
	}
	if (tcgetattr(fd, &options) != 0) 
	{  
        ERR("get serial options error\n");
		close(fd);
		return -1; 
    }
	//忽略CR回车符
	options.c_iflag |= IGNCR;
	//使用串口非规范模式
	options.c_lflag &= ~ICANON;
	//设置本地接收模式
	options.c_cflag |= CLOCAL | CREAD;
	//设置波特率
	switch (baudRate) 
 	{ 
    	case 2400: 
    		cfsetispeed(&options, B2400); 
    		cfsetospeed(&options, B2400); 
    		break; 
    	case 4800: 
    		cfsetispeed(&options, B4800); 
    		cfsetospeed(&options, B4800); 
    		break; 
    	case 9600: 
    		cfsetispeed(&options, B9600); 
    		cfsetospeed(&options, B9600); 
    		break; 
    	case 115200: 
    		cfsetispeed(&options, B115200); 
    		cfsetospeed(&options, B115200); 
    		break; 
    	case 460800: 
    		cfsetispeed(&options, B460800); 
    		cfsetospeed(&options, B460800); 
    		break; 
    	default: 
    		cfsetispeed(&options, B9600); 
    		cfsetospeed(&options, B9600); 
    		break; 
 	}
	//设置数据位
	options.c_cflag &= ~CSIZE;
 	switch (dataBits) 
 	{
		case 5: options.c_cflag |= CS5; break;
    	case 6: options.c_cflag |= CS6; break;
    	case 7: options.c_cflag |= CS7; break;
    	case 8: options.c_cflag |= CS8; break;
 	}
	//设置停止位
	switch (stopBits)
	{
		case 1: options.c_cflag &= ~CSTOPB; break;
		case 2: options.c_cflag |= CSTOPB; break;
	}
	//设置奇偶校验位 
 	switch (parity) 
 	{ 
    	case 'O': //奇数 
    		options.c_cflag |= PARENB; 
    		options.c_cflag |= PARODD; 
    		options.c_iflag |= (INPCK | ISTRIP); 
    		break; 
    	case 'E': //偶数 
    		options.c_iflag |= (INPCK | ISTRIP); 
    		options.c_cflag |= PARENB; 
    		options.c_cflag &= ~PARODD; 
    		break; 
    	case 'N':  //无奇偶校验位 
    		options.c_cflag &= ~PARENB; 
    		break; 
 	}
	/*设置超时等待时间和最小接收字符*/ 
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;
	/*处理未接收字符*/ 
 	tcflush(fd, TCIFLUSH);
	/*激活新配置*/ 
	if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
	{ 
		ERR("serial attr set failed!\n");
		close(fd);
		return -1;
	}
	else
	{
		//INFO("serial attr set successful!\n");
		return fd;
	}	
}

/*	GPS模块输出的数据格式如下所示:
$GPRMC,085551.00,A,2505.71988,N,10454.79941,E,0.029,,111215,,,A*7B
$GPVTG,,T,,M,0.029,N,0.053,K,A*2E
$GPGGA,085551.00,2505.71988,N,10454.79941,E,1,11,1.03,1175.8,M,-26.7,M,,*4D
$GPGSA,A,3,15,31,10,24,14,18,25,21,20,22,12,,2.47,1.03,2.25*0C
$GPGSV,3,1,11,10,54,340,48,12,25,097,35,14,44,298,42,15,11,066,22*78
$GPGSV,3,2,11,18,72,053,39,20,18,121,23,21,43,195,38,22,48,329,50*73
$GPGSV,3,3,11,24,37,040,31,25,25,141,37,31,18,214,33*41
$GPGLL,2505.71988,N,10454.79941,E,085551.00,A,A*6E
*/
void Gpio::GetGPSTime(UInt32 timezone)
{
	char buf[1024] = {0};
	struct tm now;
	time_t timep = 0, gap = 0;
	char *start, *end;

	if (gpsfd == -1)
		return;
	GpsLedBlink();
	read(gpsfd, buf, sizeof(buf));
	start = strstr(buf, "GPRMC");
	end = strchr(buf, '*');
	//fprintf(stderr, "GPS data:\n%s", buf);
	
	if(strstr(buf, "$GPGLL,,,,,,V,N*64") != NULL	//无法收到GPS信号
		|| start == NULL || strlen(start) < 68		//获取到的时间信息不完整
		|| end == NULL || (end - start) <= 10
		|| (start[16] != 'A' && start[16] != 'V'))
		return;

	if (3 != sscanf(end - 10, "%2d%2d%2d", &now.tm_mday, &now.tm_mon, &now.tm_year)//获取年月日
		|| 3 != sscanf(start+6, "%2d%2d%2d", &now.tm_hour, &now.tm_min, &now.tm_sec))//获取时分秒
		return;

	now.tm_year += 2000 - 1900;	//对年做一些调整
	now.tm_mon--;	//月份从0开始，所以要减1
	timep = mktime(&now) + timezone;	//增加8个时区变为北京时间
	
	gap = abs(timep - time(NULL));
	if (gap != 0)
	{
		if (-1 == stime(&timep))	//设置系统时间
			ERR("GPS check time[%lu] error: %s", timep, strerror(errno));
		else
		{
			char out[64] = {0};
			localtime_r(&timep, &now);
			strftime(out, sizeof(out), "%F %T", &now);
			INFO("get GPS time %s successful!", out);
			if (gap >= 60)
				system("hwclock -w");
		}
	}
}

inline void Gpio::GpsLedBlink()
{
#if (HARDWARE_MODEL == 500)		//TSC500
	int cmd = gpsLedOn ? LED_OUTPUT2_1 : LED_OUTPUT2_0;
	ioctl(iofd, cmd);
#elif (HARDWARE_MODEL == 300)	//TSC300
	int arg = gpsLedOn ? ARG_GPS_LED_ON : ARG_GPS_LED_OFF;
	ioctl(iofd, IO_SET_PIN_STATUS, &arg);
#endif
	gpsLedOn = !gpsLedOn;
}

inline void Gpio::RunningLedBlink()
{
#if (HARDWARE_MODEL == 500)		//TSC500
	int cmd = runLedOn ? LED_OUTPUT1_1 : LED_OUTPUT1_0;
	ioctl(iofd, cmd);
#elif (HARDWARE_MODEL == 300)	//TSC300
	int arg = runLedOn ? ARG_SYSTEM_RUNNING_LED_ON : ARG_SYSTEM_RUNNING_LED_OFF;
	ioctl(iofd, IO_SET_PIN_STATUS, &arg);
#endif
	runLedOn = !runLedOn;
}

inline void Gpio::HardFlashCtrl()	//硬黄闪控制
{
#if (HARDWARE_MODEL == 500)		//TSC500
	if (hardFlashHigh)
	{
		ioctl(iofd, YELLOW_CONTROL_OUTPUT1_1);
		ioctl(iofd, YELLOW_CONTROL_OUTPUT2_1);
	}
	else
	{
		ioctl(iofd, YELLOW_CONTROL_OUTPUT1_0);
		ioctl(iofd, YELLOW_CONTROL_OUTPUT2_0);
	}
#elif (HARDWARE_MODEL == 300)	//TSC300
	int arg = hardFlashHigh ? ARG_YF_CTRL_HIGH : ARG_YF_CTRL_LOW;
	ioctl(iofd, IO_SET_PIN_STATUS, &arg);
#endif
	hardFlashHigh = !hardFlashHigh;
}

void Gpio::SetPedKeyStatus()
{
	UInt8 value = 0;
	int arg = 0;
#if (HARDWARE_MODEL == 500)		//TSC500
	ioctl(iofd, IO_INPUT1_TO_INPUT8, &arg);
	value = (~arg) & 0xff;
#elif (HARDWARE_MODEL == 300)	//TSC300
	for (int i = 1; i <= 8; i++)
	{
		arg = SET_ARG(i, 0);
		ioctl(iofd, IO_GET_PIN_STATUS, &arg);
		value |= ((arg & 0x1) << (i - 1));
	}
#endif
	if (value)
	{
		pedKeyStatus |= value;
		INFO("pedKeyStatus = %#x", value);
	}
}

void Gpio::SetWirelessKeyStatus()
{
	UInt8 value = 0;
	int arg = -1;
#if (HARDWARE_MODEL == 500)		//TSC500
	ioctl(iofd, IO_INPUT9_TO_INPUT16, &arg);
	bitset<5> bits = (~arg) & 0x1f;
	value = (bits[0] << 3) | (bits[1] << 2) | (bits[2] << 4) | (bits[3]) | (bits[4] << 1);
#elif (HARDWARE_MODEL == 300)	//TSC300
	arg = 0;
	ioctl(iofd, IO_GET_WIRELESS_STATUS, &arg);
	value = (UInt8)arg;
#endif
	if (value)
	{
		wirelessKeyStatus |= value;
		INFO("wirelessKeyStatus = %#x", value);
	}
}

void Gpio::SetCtrlKeyLed()
{
#if (HARDWARE_MODEL == 500)		//TSC500
	ioctl(iofd, ctrlKey[0] ? KEYBOARD_OUTPUT3_1 : KEYBOARD_OUTPUT3_0);	//点亮或熄灭自动LED
	ioctl(iofd, ctrlKey[1] ? KEYBOARD_OUTPUT4_1 : KEYBOARD_OUTPUT4_0);	//点亮或熄灭手动LED
	ioctl(iofd, ctrlKey[2] ? KEYBOARD_OUTPUT2_1 : KEYBOARD_OUTPUT2_0);	//点亮或熄灭黄闪LED
	ioctl(iofd, ctrlKey[3] ? KEYBOARD_OUTPUT5_1 : KEYBOARD_OUTPUT5_0);	//点亮或熄灭全红LED
	ioctl(iofd, ctrlKey[4] ? KEYBOARD_OUTPUT1_1 : KEYBOARD_OUTPUT1_0);	//点亮或熄灭步进LED
#elif (HARDWARE_MODEL == 300)	//TSC300
	Singleton<Can>::GetInstance().SetCtrlKeyLedByCan(ctrlKey);	//点亮前面板led
	//点亮侧面板led
	char buf[8] = "Button";
	for (int i = 0; i < 5; i++)
	{
		if (ctrlKey[i])
		{
			buf[6] = (char)(i + 1);
			write(keyfd, buf, 7);
		}
	}
#endif
}

void Gpio::ProcessCtrlKey(bitset<8> &old, bitset<8> &now)
{
	if (now.none())
		return;
	if (old[0] && now[1])
	{
		old = 0x2;	//由自动到手动
		ctrlKeyStatus = MANUAL_KEY;
		SetCtrlKeyLed();
		//log.Write("auto key pressed, recovery auto control");
	}
	else if (old[1])
	{
		if (now[0])
		{
			old = 0x1;	//由手动到自动
			ctrlKeyStatus = AUTO_KEY;
			SetCtrlKeyLed();
			//log.Write("manual key pressed!");
			return;
		}
		if (!old[2] && now[2])
		{
			old = 0x6;	//执行黄闪
			ctrlKeyStatus = YELLOWBLINK_KEY;
			SetCtrlKeyLed();
			//log.Write("yellow flash key pressed, excute manual yellow flash");
			return;
		}
		if (!old[3] && now[3])
		{
			old = 0xa;	//执行全红
			ctrlKeyStatus = ALLRED_KEY;
			SetCtrlKeyLed();
			//log.Write("allred key pressed, excute manual allred");
			return;
		}
		if (!old[2] && !old[3] && now[4])
		{
			old = 0x12;	//执行步进
			ctrlKeyStatus = STEP_KEY;
			SetCtrlKeyLed();
			//log.Write("step key pressed, excute manual step");
		}
	}
}

void Gpio::SetCtrlKeyStatus()
{
	bitset<8> value;
	int arg = 1;
#if (HARDWARE_MODEL == 500)		//TSC500
	ioctl(iofd, KEYBOARD_INPUT3, &arg);	//获取自动按键状态
	value[0] = (arg == 0);
	ioctl(iofd, KEYBOARD_INPUT4, &arg);	//获取手动按键状态
	value[1] = (arg == 0);
	ioctl(iofd, KEYBOARD_INPUT2, &arg);	//获取黄闪按键状态
	value[2] = (arg == 0);
	ioctl(iofd, KEYBOARD_INPUT5, &arg);	//获取全红按键状态
	value[3] = (arg == 0);
	ioctl(iofd, KEYBOARD_INPUT1, &arg);	//获取步进按键状态
	value[4] = (arg == 0);
#if 0
	value |= GetWirelessKeyStatus();
	if (value.any())
		cout << "key board: " << value << endl;
#endif
#elif (HARDWARE_MODEL == 300)	//TSC300
	ioctl(iofd, IO_GET_BUTTON_STATUS, &arg);
	if (arg > 0)
		value = arg;
	else
	{
		char buf[32] = {0};
		int n = read(keyfd, buf, sizeof(buf));
		if (n > 0)
		{
			string keystr(buf, n);
			auto it = button.find(keystr);
			if (it != button.end() && it->second > 0)
				value[it->second - 1] = 1;
		}
	}
	//value |= GetWirelessKeyStatus();
#endif
	ProcessCtrlKey(ctrlKey, value);
}

Gpio::Gpio() : log(Singleton<Log>::GetInstance())
{
#if (HARDWARE_MODEL == 500)		//TSC500
	iofd = open("/dev/CPLD_IO", O_RDONLY);
#elif (HARDWARE_MODEL == 300)	//TSC300
	iofd = open("/dev/gpio", O_RDONLY);
	keyfd = Opentty(3, 9600, 8, 1, 'N');	//打开侧面板串口
	button["Auto"] = 1;		//这几个按键对应的值必须是1,2,3,4,5，刚好与前面板按键值一样
	button["Manual"] = 2;
	button["YellowBlink"] = 3;
	button["AllRed"] = 4;
	button["StepByStep"] = 5;
	#if 0
	other["East"] = 6;
	other["South"] = 7;
	other["West"] = 8;
	other["North"] = 9;
	other["EastAndWestStraight"] = 10;
	other["SouthAndNorthStraight"] = 11;
	other["EastAndWestLeft"] = 12;
	other["SouthAndNorthLeft"] = 13;
	#endif
#endif
	gpsfd = Opentty(6, 9600, 8, 1, 'N');
	watchdogfd = -1;
	gpsSwitch = true;		//默认开启GPS
	watchdogSwitch = false;	//默认关闭watchdog
	gpsLedOn = true;
	runLedOn = true;
	hardFlashHigh = true;
	pedKeyStatus = 0;
	ctrlKeyStatus = NONE_KEY;
	ctrlKey = 1;	//初始化默认自动按键按下
	SetCtrlKeyLed();
}

inline void Gpio::FeedWatchdog()
{
	if (watchdogfd != -1)
	{
		int food = 0;
		ioctl(watchdogfd, WDIOC_KEEPALIVE, &food);
	}
}

void Gpio::SetWatchdog(bool flag)
{
	bool old = watchdogSwitch;
	if (!old && flag)
	{	//watchdog从关闭到打开
		watchdogSwitch = true;
		system("killall watchdog");
		int fd = open("/dev/watchdog", O_WRONLY);
		if (fd == -1)
		{
			ERR("open /dev/watchdog fail, error info: %s", strerror(errno));
			system("watchdog -t 1 -T 3 /dev/watchdog");
		}
		else
			watchdogfd = fd;
	}
	else if (old && !flag)
	{	//watchdog从打开到关闭
		watchdogSwitch = false;
		close(watchdogfd);
		watchdogfd = -1;
		system("watchdog -t 1 -T 3 /dev/watchdog");
	}
}

void Gpio::run(void *a)
{
	UInt8 value = 0;
	int count = 0;
	
	while (true)
	{
		SetPedKeyStatus();
		SetWirelessKeyStatus();
		SetCtrlKeyStatus();
		if (++count >= 5)
		{
			RunningLedBlink();
			count = 0;
			if (gpsSwitch)
				GetGPSTime();
			FeedWatchdog();
		}
		HardFlashCtrl();
		msleep(100);
	}
}

#endif
