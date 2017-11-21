#if defined(__linux__) && defined(__arm__)
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE    //对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <time.h>

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
std::time_t Phasecontrol::GetGPSTime()
{
	char buf[1024] = {0};
	struct tm now;
	std::time_t timep = 0;
	char *start, *end;
	
	read(fd, buf, sizeof(buf));
	start = std::strstr(buf, "GPRMC");
	end = std::strchr(buf, '*');
	//fprintf(stderr, "GPS data:\n%s", buf);
	
	if(std::strstr(buf, "$GPGLL,,,,,,V,N*64") != NULL	//无法收到GPS信号
		|| start == NULL || std::strlen(start) < 68		//获取到的时间信息不完整
		|| end == NULL || (end - start) <= 10
		|| (start[16] != 'A' && start[16] != 'V'))
		return 0;

	if (3 != std::sscanf(end - 10, "%2d%2d%2d", &now.tm_mday, &now.tm_mon, &now.tm_year)//获取年月日
		|| 3 != std::sscanf(start+6, "%2d%2d%2d", &now.tm_hour, &now.tm_min, &now.tm_sec))//获取时分秒
		return 0;
	now.tm_year += 2000 - 1900;	//对年做一些调整
	now.tm_mon--;	//月份从0开始，所以要减1
	timep = std::mktime(&now) + 8 * 3600;	//增加8个时区变为北京时间
	if (time(NULL) != timep)
	{
		if (-1 == stime(&timep))	//设置系统时间
			ERR("GPS check time[%lu] error: %s", timep, strerror(errno));
		//system("hwclock -w");
	}
	return timep;
}

/* 串口设置为:
	波特率：9600， 8bit
	停止位：1
	奇偶校验位：N	*/
void Phasecontrol::OpenGpsPort() 
{ 
	struct termios options;
	
	fd = open( "/dev/ttyS6", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open /dev/ttyS6 fail!\n");
		return;
	}
#if 0
	//测试是否为终端设备 
	if(isatty(STDIN_FILENO) == 0) 
	{
		printf("standard input is not a terminal device\n"); 
		close(fd);
		fd = -1;
		return;
	}
#endif
	//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if(tcgetattr(fd, &options) != 0) 
	{  
        printf("get serial options error\n");
		close(fd);
		fd = -1;
		return; 
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
		printf("serial attr set failed!\n");
		close(fd);
		fd = -1;
	}
	else
	{
		printf("serial attr set successful!\n");
	}
}

void Phasecontrol::GetLocalTime()
{
	gCurTime = 0;

	if (ptl.enableGPS)
	{
		if (fd == -1)
			fd = OpenGpsPort();
	}
	else
	{
		if (fd > 0)
		{
			close(fd);
			fd = -1;
		}
	}
	
	if (fd > 0)
	{	//首先使用GPS获取时间
		gCurTime = GetGPSTime(fd);
		ledstatus = !ledstatus;
		ledstatus ? ptl.GPS_led_on() : ptl.GPS_led_off();
	}
	if (gCurTime == 0)	
	{	//如果GPS获取不到时间则使用系统时间
		gCurTime = std::time(NULL);
		ptl.GPS_led_off();
	}
}

#else
inline void Phasecontrol::GetLocalTime()
{
	gCurTime = std::time(NULL);
}
#endif
