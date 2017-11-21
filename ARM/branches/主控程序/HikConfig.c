#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "HikConfig.h"
#include "parse_ini.h"

static SignalControllerPara *gSignalControlpara;

#define NOUPDATE	0
#define UPDATE		1
static char gTimeIntervalUpdateFlag = NOUPDATE;
static char gPlanScheduleUpdateFlag = NOUPDATE;

#define YELLOWBLINKID	119

/*****************************************************************************
 函 数 名  : GetTimeIntervalID
 功能描述  : 遍历调度表，根据当前时间，确定当前运行时段号
 输入参数  : 无
 输出参数  : 无
 返 回 值  : unsigned
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年7月30日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char GetTimeIntervalID(struct tm *now)
{
    int i = 0;

    for(i = 0 ; i < NUM_SCHEDULE ; i++) { //如果当前时间和某一项调度表相符，则返回该调度表的时段号
        if ((BIT(gSignalControlpara->stPlanSchedule[i].month, now->tm_mon + 1) == 1) 	//因为tm_mon范围是[0,11]
            && (BIT(gSignalControlpara->stPlanSchedule[i].day, now->tm_mday) == 1)) {
			    return gSignalControlpara->stPlanSchedule[i].nTimeIntervalID;
		}
		if(BIT(gSignalControlpara->stPlanSchedule[i].week, (now->tm_wday == 0) ? 7 : now->tm_wday) == 1) {
														//tm_wday == 0代表星期日
			return gSignalControlpara->stPlanSchedule[i].nTimeIntervalID;
		}
    }

	//DBG("Can't find Time interval list\n");
    return 0;
}

#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)
#define MAX_TIME_GAP	HoursToMinutes(23, 59)

/*****************************************************************************
 函 数 名  : GetActionID
 功能描述  : 根据时段表ID，遍历该时段表下的时段，根据当前时间，判断应属于哪
             个时段，进而确定当前时段需要执行的动作
 输入参数  : unsigned short nTimeIntervalID
 输出参数  : 无
 返 回 值  : unsigned
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年7月30日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned char GetActionID()
{
    int i = 0;
	unsigned char nTimeIntervalID;
	int nCurrentTime, nTempTime;
	int nTimeGap, oldTimeGap, index = -1;
    time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);
    nTimeIntervalID = GetTimeIntervalID(&now);//根据当前时间，遍历调度表，得到时段表ID
    if(nTimeIntervalID == 0) {
        return 0;
    }
	//DBG("nTimeIntervalID = %d, index = %d\n", nTimeIntervalID, index);
	//DBG("nowTime: hour = %d, minute = %d\n", now.tm_hour, now.tm_min);
	nCurrentTime = HoursToMinutes(now.tm_hour, now.tm_min);
	oldTimeGap = MAX_TIME_GAP;	//预先设置一个最大的差值
	while (index == -1) {	//循环找出当前时间与时段表中差值最小的时段所对应的actionID即为当前应该运行的actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) {
			if(gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) { //这说明该时段可能没有被使用，直接continue掉
				continue;
			}

			nTempTime = HoursToMinutes(gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour,gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute);
			if (nCurrentTime == nTempTime) {
				return gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][i].nActionID;
			} else if (nCurrentTime > nTempTime) {
				nTimeGap = nCurrentTime - nTempTime;
				if (nTimeGap < oldTimeGap) {
					oldTimeGap = nTimeGap;
					index = i;
				}
			}
		}
		if (oldTimeGap == MAX_TIME_GAP) { //说明当前时间处于时段表中最小的位置
			nCurrentTime += HoursToMinutes(24, 0);	//把当前时间增加24小时然后再次循环
		}
	}

	return gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;
}

static void loadTimeIntervalFromIni()	//装载时段表
{
	int i, j;
	char cSection[256] = {0};

	
	if (parse_start("/home/config.ini") == False) {
		return;
	}
	memset((void *)(gSignalControlpara->stTimeInterval), 0, sizeof(gSignalControlpara->stTimeInterval));
	
	for(i = 0 ; i < NUM_TIME_INTERVAL; i++)
    {
        for(j = 0 ; j < NUM_TIME_INTERVAL_ID ; j++)
        {
            sprintf(cSection,"TimeIntervalItem%d_%d",i+1,j+1);

            gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID = get_one_value(cSection,"nTimeIntervalID");
            gSignalControlpara->stTimeInterval[i][j].nTimeID = get_one_value(cSection,"nTimeID");
            gSignalControlpara->stTimeInterval[i][j].cStartTimeHour = get_one_value(cSection,"cStartTimeHour");
            gSignalControlpara->stTimeInterval[i][j].cStartTimeMinute = get_one_value(cSection,"cStartTimeMinute");
            gSignalControlpara->stTimeInterval[i][j].nActionID = get_one_value(cSection,"nActionID");
#if 1
            if(gSignalControlpara->stTimeInterval[i][j].nActionID != 0)
                DBG("%s  stTimeInterval  nTimeIntervalID  %d  , nTimeID  %d , cStartTimeHour  %d , cStartTimeMinute  %d , nActionID  %d   \n",__func__,
                                    gSignalControlpara->stTimeInterval[i][j].nTimeIntervalID,
                                    gSignalControlpara->stTimeInterval[i][j].nTimeID,
                                    gSignalControlpara->stTimeInterval[i][j].cStartTimeHour,
                                    gSignalControlpara->stTimeInterval[i][j].cStartTimeMinute,
                                    gSignalControlpara->stTimeInterval[i][j].nActionID);
#endif                                    
            memset(cSection,0,sizeof(cSection));
        }
    }
    parse_end();
}

static void loadPlanScheduleFromIni()	//装载调度表
{
	int i;
	char cSection[256] = {0};

	if (parse_start("/home/config.ini") == False) {
		return;
	}

	memset((void *)(gSignalControlpara->stPlanSchedule), 0, sizeof(gSignalControlpara->stPlanSchedule));
    for(i = 0 ; i < NUM_SCHEDULE; i++)
    {
        sprintf(cSection,"PlanScheduleItem%d",i+1);

        gSignalControlpara->stPlanSchedule[i].nScheduleID = get_one_value(cSection,"nScheduleID");
        gSignalControlpara->stPlanSchedule[i].nTimeIntervalID = get_one_value(cSection,"nTimeIntervalID");
		gSignalControlpara->stPlanSchedule[i].month = get_one_value(cSection,"month");
		gSignalControlpara->stPlanSchedule[i].week = get_one_value(cSection,"week");
		gSignalControlpara->stPlanSchedule[i].day = get_one_value(cSection,"day");
#if 1
        if(gSignalControlpara->stPlanSchedule[i].nTimeIntervalID != 0)
            DBG("%s  stPlanSchedule  nScheduleID  %d  , nTimeIntervalID  %d, month = %d, week = %d, day = %u\n",__func__,
			gSignalControlpara->stPlanSchedule[i].nScheduleID, 
			gSignalControlpara->stPlanSchedule[i].nTimeIntervalID,
			gSignalControlpara->stPlanSchedule[i].month,
			gSignalControlpara->stPlanSchedule[i].week,
			gSignalControlpara->stPlanSchedule[i].day);
#endif
        memset(cSection,0,sizeof(cSection));
    }
	
	parse_end();
}

#define SEND_TIMEINTERVAL_TIMES		16	//总共发送16次时段表信息
#define SEND_TIMEINTERVAL_NUM		48	//每次发送48个
void storeTimeIntervalToIni(void *arg)
{
	TimeIntervalItem *item = (TimeIntervalItem *)arg;
    char section[128] = {0};
	char tmpstr[16] = {0};
	static int num = 0;
	int i;
	
	if (parse_start("/home/config.ini") == False) {
		return;
	}
	for (i = 0; i < SEND_TIMEINTERVAL_NUM; i++) {
		sprintf(section,"TimeIntervalItem%d_%d", item[i].nTimeIntervalID, item[i].nTimeID);
		del_section(section);	//删除之前的配置
		if (item[i].nActionID == 0)	//无效的配置就不必写入配置文件了
			continue;
		add_one_key(section, "nTimeIntervalID", item[i].nTimeIntervalID);
		add_one_key(section, "nTimeID", item[i].nTimeID);
		add_one_key(section, "cStartTimeHour", item[i].cStartTimeHour);
		add_one_key(section, "cStartTimeMinute", item[i].cStartTimeMinute);
		add_one_key(section, "nActionID", item[i].nActionID);

		DBG("[stTimeInterval]  nTimeIntervalID: %d, nTimeID: %d, cStartTimeHour: %d, cStartTimeMinute: %d, nActionID: %d\n",
				item[i].nTimeIntervalID,
				item[i].nTimeID,
				item[i].cStartTimeHour,
				item[i].cStartTimeMinute,
				item[i].nActionID);
	}
	parse_end();
	num++;
	if (num == SEND_TIMEINTERVAL_TIMES) {
		gTimeIntervalUpdateFlag = UPDATE;
		DBG("TimeInterval list update\n");
		num = 0;
	}
}

#define SEND_PLANSCHEDULE_TIMES		4	//总共发送4次调度表信息
#define	SEND_PLANSCHEDULE_NUM		10	//每次发送10个
void storePlanScheduleToIni(void *arg)
{
	PlanScheduleItem *item = (PlanScheduleItem *)arg;
	const char *file = "/home/config.ini" ;
    char section[128] = {0};
	char tmpstr[16] = {0};
	static int num = 0;
	int i;
	
	if (parse_start("/home/config.ini") == False) {
		return;
	}
	for (i = 0; i < SEND_PLANSCHEDULE_NUM; i++) {
		sprintf(section,"PlanScheduleItem%d", item[i].nScheduleID);
		del_section(section);
		if (item[i].nTimeIntervalID == 0)
			continue;
		add_one_key(section, "nScheduleID", item[i].nScheduleID);
		add_one_key(section, "month", item[i].month);
		add_one_key(section, "week", item[i].week);
		sprintf(tmpstr,"%u",item[i].day);
		add_key_string(section, "day", tmpstr);
		add_one_key(section, "nTimeIntervalID", item[i].nTimeIntervalID);

		DBG("[stPlanSchedule] nScheduleID: %d, nTimeIntervalID: %d, month: %d, week: %d, day: %u\n",
				item[i].nScheduleID, 
				item[i].nTimeIntervalID,
				item[i].month,
				item[i].week,
				item[i].day);
	}
	parse_end();
	num++;
	if (num == SEND_PLANSCHEDULE_TIMES) {
		gPlanScheduleUpdateFlag = UPDATE;
		DBG("PlanSchedule list update\n");
		num = 0;
	}
}

/*********************************************************************************
*
* 跳转感应模式
*
***********************************************************************************/

static void udp_send_jump_response()
{
	//创建UDP服务器
	int socketFd = -1;

	char responseSend[47]={0x30,0x2D,0x02,0x01,0x00,0x04,0x06,0x70,
						0x75,0x62,0x6C,0x69,0x63,0xA3,0x20,0x02,
						0x01,0x02,0x02,0x01,0x00,0x02,0x01,0x00,
						0x30,0x15,0x30,0x13,0x06,0x0D,0x2B,0x06,
						0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
						0x04,0x0E,0x00,0x02,0x02,0x00,0xFE};
	struct sockaddr_in localAddr;

	memset(&localAddr, 0, sizeof(localAddr));
	socklen_t localLen = sizeof(localAddr);
	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( -1 == socketFd )
	{
		DBG("socket udp init error!!!\n");
		return;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
//使用161端口
	localAddr.sin_port = htons(161);
//向本机161端口发送
	DBG("跳转感应控制\n");
	int len = sendto(socketFd,responseSend,47,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		DBG("Send inductive control signal failed!!!\n");
	}
	close(socketFd);
	
}

/*********************************************************************************
*
* 发送改变控制方式命令
*
***********************************************************************************/

static void udp_send_change_ctrl()
{
	//创建UDP服务器
	int socketFd = -1;

	char responseSend[46]={0x30,0x2C,0x02,0x01,0x00,0x04,0x06,0x70,
							0x75,0x62,0x6C,0x69,0x63,0xA3,0x1F,0x02,
							0x01,0x07,0x02,0x01,0x00,0x02,0x01,0x00,
							0x30,0x14,0x30,0x12,0x06,0x0D,0x2B,0x06,
							0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
							0x04,0x0E,0x00,0x02,0x01,0x00};
    struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
    	socklen_t localLen = sizeof(localAddr);
    	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    	if ( -1 == socketFd )
    	{
		DBG("socket udp init error!!!\n");
       	return;
    	}

    	localAddr.sin_family = AF_INET;
    	localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//使用161端口
    	localAddr.sin_port = htons (161);
	//向本机161端口发送
	DBG("改变控制方式\n");
	int len = sendto(socketFd,responseSend,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		DBG("Send system control signal failed!!!\n");
	}
	close(socketFd);
	
}

//计算时间差
static inline int timeGap(struct timeval *new, struct timeval *old)
{
	return (new->tv_sec - old->tv_sec) * 1000000 + new->tv_usec - old->tv_usec;
}

enum {	//定义一个枚举类型用来作标志使用
	NOUSED = 0,		//表明未使用
	INIT,			//表明是初始化
	CHANGE,			//表明此时正处于改变控制方式
};

void adjustControlType()	//调整控制方式
{
	static char flag = INIT;
	static unsigned char lastActionId = 0, nowActionId = 0;
	static struct timeval oldTime = {0, 0};
	struct timeval nowTime = {0, 0};
//#define DEBUG
#ifdef DEBUG
	static struct timeval tmpTime = {0, 0};
#endif
	gettimeofday(&nowTime, NULL);
    if (flag == INIT) {	//初始化
		if ((gSignalControlpara = (SignalControllerPara *)calloc(1, sizeof(SignalControllerPara))) == NULL) {
			DBG("memory is not enough to store the backup config information\n");
			return;
		}
		flag = NOUSED;
		loadTimeIntervalFromIni();
		loadPlanScheduleFromIni();
#ifdef DEBUG
		tmpTime = nowTime;
#endif
		return;
    }
	if (gTimeIntervalUpdateFlag == UPDATE) {
		gTimeIntervalUpdateFlag = NOUPDATE;
		loadTimeIntervalFromIni();
	}
	if (gPlanScheduleUpdateFlag == UPDATE) {
		gPlanScheduleUpdateFlag = NOUPDATE;
		loadPlanScheduleFromIni();
	}
	
	if (flag == NOUSED) {
		nowActionId = GetActionID();
#ifdef DEBUG
		if (timeGap(&nowTime, &tmpTime) > 1000000) {
			DBG("nowActionId = %d\n", nowActionId);
			tmpTime = nowTime;
		}
#endif
		if (nowActionId == YELLOWBLINKID) {
			lastActionId = YELLOWBLINKID;
		}
	}
	if (lastActionId == YELLOWBLINKID && nowActionId != YELLOWBLINKID) {//要从黄闪切换到其他控制方式
		//gettimeofday(&nowTime, NULL);
		if (oldTime.tv_sec == 0 && oldTime.tv_usec == 0) {	//表明刚开始改变控制方式
			oldTime = nowTime;	//记录当前时间用以延时
			flag = CHANGE;		//改变标志说明此时正处于改变控制方式
			udp_send_jump_response();	//先发送一次感应信息
		} else {
			//计算时间差大于500ms时发送系统控制信息
			if (timeGap(&nowTime, &oldTime) >= 500000) {
				udp_send_change_ctrl();
				oldTime.tv_sec = 0;
				oldTime.tv_usec = 0;
				flag = NOUSED;
				lastActionId = 0;
			}
		}
	}
}
