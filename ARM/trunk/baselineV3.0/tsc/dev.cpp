#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifndef _SVID_SOURCE
#define _SVID_SOURCE    //对于glibc2版本,函数stime()需要定义这个宏，
#endif
#include <ctime>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/can.h>
#include <sys/time.h>
#include "libsocketcan.h"
#include "linux.h"
#include "dev.h"

bool hik::dev::parse_gps(int gpsfd, const char *headname, int zone, const char *rtcname)
{
	if (gpsfd == -1 || headname == nullptr)
		return false;
	char buf[1024] = {0};
	if (0 >= read(gpsfd, buf, sizeof(buf)))
		return false;

	const char *p = strstr(buf, headname);
	if (p == nullptr || strchr(p, '\n') == nullptr)
		return false;

//$GPRMC,085551.00,A,2505.71988,N,10454.79941,E,0.029,,111215,,,A*7B
//$GNRMC,065407.000,V,3012.695983,N,12012.973370,E,0.000,24.054,101117,,E,N*1C

	struct tm now = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	char flag = 'V';
	if (7 != sscanf(p, "%*[^,],%2d%2d%2d.%*d,%c,%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%2d%2d%2d%*s\n", 
	/*获取时分秒*/&now.tm_hour, &now.tm_min, &now.tm_sec, &flag,
	/*获取年月日*/&now.tm_mday, &now.tm_mon, &now.tm_year) || flag == 'V')
		return false;
	//printf("flag: %c, time: %d-%d-%d %02d:%02d:%02d\n", flag, now.tm_year + 2000, now.tm_mon, now.tm_mday, now.tm_hour + 8, now.tm_min, now.tm_sec);
	now.tm_year += 2000 - 1900;	//对年做一些调整
	now.tm_mon--;	//月份从0开始，所以要减1
	time_t timep = mktime(&now) + zone * 3600;	//增加8个时区变为北京时间
	time_t gap = abs(timep - time(NULL));
	if (gap != 0)
	{
		stime(&timep);
		if (gap >= 60 && rtcname != nullptr)
		{
			rtc hwclock(rtcname);
			hwclock.set(timep);
		}
	}
	return true;
}

int hik::dev::can_init(unsigned int bitrate, const char *canname/* = "can0"*/)
{
	struct sockaddr_can addr;
    struct ifreq ifr;
	struct timeval timeout = {2, 0};
	int fd = -1;

	if (canname == nullptr)
		exit(1);
	can_do_stop(canname);
	if (can_set_bitrate(canname, bitrate) < 0)
	{
		//ERR("set can0 bitrate(%u) fail!", bitrate);
		exit(1);
	}
	if (can_do_start(canname) < 0)
	{
		//ERR("can0 start fail!");
		exit(1);
	}
    fd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
	if (fd == -1)
	{
		//perror("create can socket fail!");
		exit(1);
	}
    strcpy(ifr.ifr_name, canname);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
	{
		//perror("ioctl get can index fail!");
		exit(1);
	}
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		//perror("bind can socket fail!");
		exit(1);
	}
	//设置接收超时时间为2s
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	return fd;
}

void hik::dev::setlamps(UInt8 ctrlMode)
{
	TscStatus status;
	ChannelArray lamps = cache;
	switch (ctrlMode)
	{
		case TURNOFF_MODE: status = TURN_OFF; break;
		case YELLOWBLINK_MODE: status = YELLOW_BLINK; break;
		case ALLRED_MODE: status = ALLRED; break;
		default: status = INVALID;
	}
	for (auto &i : lamps)
	{
		if (i.inuse())
			i.status = status;
	}
	cache = lamps;
}

#endif
