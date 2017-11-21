#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include "ftest.h"
#include "gps.h"
#include "cpld.h"
#include "debug.h"

extern int gTsctype;
extern int g_CPLDFd;
extern int g_io_fd;

typedef void (*ArgFunc)(void);

int g_fd = 0;
ArgFunc funcGPSLedOn;
ArgFunc funcGPSLedOff;

void GPS_led_on_300();
void GPS_led_on_500();
void GPS_led_off_300();
void GPS_led_off_500();

void fdInit()
{
	if(gTsctype == TSC_TYPE_300)
	{
		g_fd = g_io_fd;
		funcGPSLedOn = GPS_led_on_300;
		funcGPSLedOff = GPS_led_off_300;
	}
	else
	{
		g_fd = g_CPLDFd;
		funcGPSLedOn = GPS_led_on_500;
		funcGPSLedOff = GPS_led_off_500;	
	}
	
}

//设置LED灯状态
static void set_led_status(int param)
{
	int arg = param;
	(void) ioctl(g_fd, IO_SET_PIN_STATUS, &arg);
}
//点亮GPS指示灯
void GPS_led_on_300()
{
	set_led_status(ARG_GPS_LED_ON);
}
void GPS_led_on_500()
{
	if(g_fd != -1)
	{
		//点亮GPS指示灯
		ioctl(g_fd,LED_OUTPUT2_1);
	}
}
//关闭GPS指示灯
void GPS_led_off_300()
{
	set_led_status(ARG_GPS_LED_OFF);
}
void GPS_led_off_500()
{
	if(g_fd != -1)
	{
		//熄灭GPS指示灯
		ioctl(g_fd,LED_OUTPUT2_0);
	}
}
/* 串口设置为:
	波特率：9600， 8bit
	停止位：1
	奇偶校验位：N	*/
static int OpenGpsPort() 
{ 
	struct termios options;
	int fd = open( "/dev/ttyS6", O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (fd == -1)
	{
		ERR("open /dev/ttyS6 fail!\n");
		return -1;
	}
#if 0
	//测试是否为终端设备 
	if(isatty(STDIN_FILENO) == 0) 
	{
		ERR("standard input is not a terminal device\n"); 
		close(fd);
		return -1;
	}
#endif
	//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if(tcgetattr(fd, &options) != 0) 
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
	//设置8bit数据位
	options.c_cflag &= ~CSIZE; 
	options.c_cflag |= CS8;
	//设置无奇偶校验位 
	options.c_cflag &= ~PARENB;
	//设置9600波特率
	cfsetispeed(&options, B9600); 
	cfsetospeed(&options, B9600); 	 
	/*设置停止位1*/
	options.c_cflag &= ~CSTOPB; 
	/*设置超时等待时间和最小接收字符*/ 
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;
	/*激活新配置*/ 
	if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
	{ 
		ERR("serial attr set failed!\n");
		close(fd);
		return -1;
	}
	else
	{
		INFO("serial attr set successful!\n");
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
static void GetGPSTime(unsigned char useGPS, unsigned int timezone) 
{
	static int fd = -1;
	static unsigned char ledstatus = 0;
	char buf[1024] = {0};
	struct tm now;
	time_t timep = 0, gap = 0;
	char *start, *end;

	if (!useGPS)
	{
		//GPS_led_off();
		funcGPSLedOff();
		return;
	}
	if (fd == -1)
	{
		fd = OpenGpsPort();
		if (fd == -1)
		{
			//GPS_led_off();
			funcGPSLedOff();
			return;
		}
	}
	ledstatus = !ledstatus;
	//ledstatus ? GPS_led_on() : GPS_led_off();
	ledstatus ? funcGPSLedOn() : funcGPSLedOff();
		
	read(fd, buf, sizeof(buf));
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
unsigned char g_GpsFlag=0;
static void *GPS_thread(void *param)
{
	fdInit();
	while (1) {
		GetGPSTime(g_GpsFlag, 161);
		usleep(500*1000);//500ms
	}
	pthread_exit(NULL);
}
void GPS_init()
{
	pthread_t threadId;
	if (pthread_create(&threadId, NULL, GPS_thread, NULL)) {
		perror("create GPS thread fail");
		exit(1);
	}
	pthread_detach(threadId);
	//INFO("yellow light thread init successful!");
}