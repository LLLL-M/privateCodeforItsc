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
 �� �� ��  : GetTimeIntervalID
 ��������  : �������ȱ����ݵ�ǰʱ�䣬ȷ����ǰ����ʱ�κ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : unsigned
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��7��30��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
static unsigned char GetTimeIntervalID(struct tm *now)
{
    int i = 0;

    for(i = 0 ; i < NUM_SCHEDULE ; i++) { //�����ǰʱ���ĳһ����ȱ�������򷵻ظõ��ȱ��ʱ�κ�
        if ((BIT(gSignalControlpara->stPlanSchedule[i].month, now->tm_mon + 1) == 1) 	//��Ϊtm_mon��Χ��[0,11]
            && (BIT(gSignalControlpara->stPlanSchedule[i].day, now->tm_mday) == 1)) {
			    return gSignalControlpara->stPlanSchedule[i].nTimeIntervalID;
		}
		if(BIT(gSignalControlpara->stPlanSchedule[i].week, (now->tm_wday == 0) ? 7 : now->tm_wday) == 1) {
														//tm_wday == 0����������
			return gSignalControlpara->stPlanSchedule[i].nTimeIntervalID;
		}
    }

	//DBG("Can't find Time interval list\n");
    return 0;
}

#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)
#define MAX_TIME_GAP	HoursToMinutes(23, 59)

/*****************************************************************************
 �� �� ��  : GetActionID
 ��������  : ����ʱ�α�ID��������ʱ�α��µ�ʱ�Σ����ݵ�ǰʱ�䣬�ж�Ӧ������
             ��ʱ�Σ�����ȷ����ǰʱ����Ҫִ�еĶ���
 �������  : unsigned short nTimeIntervalID
 �������  : ��
 �� �� ֵ  : unsigned
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��7��30��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
    nTimeIntervalID = GetTimeIntervalID(&now);//���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
    if(nTimeIntervalID == 0) {
        return 0;
    }
	//DBG("nTimeIntervalID = %d, index = %d\n", nTimeIntervalID, index);
	//DBG("nowTime: hour = %d, minute = %d\n", now.tm_hour, now.tm_min);
	nCurrentTime = HoursToMinutes(now.tm_hour, now.tm_min);
	oldTimeGap = MAX_TIME_GAP;	//Ԥ������һ�����Ĳ�ֵ
	while (index == -1) {	//ѭ���ҳ���ǰʱ����ʱ�α��в�ֵ��С��ʱ������Ӧ��actionID��Ϊ��ǰӦ�����е�actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) {
			if(gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) { //��˵����ʱ�ο���û�б�ʹ�ã�ֱ��continue��
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
		if (oldTimeGap == MAX_TIME_GAP) { //˵����ǰʱ�䴦��ʱ�α�����С��λ��
			nCurrentTime += HoursToMinutes(24, 0);	//�ѵ�ǰʱ������24СʱȻ���ٴ�ѭ��
		}
	}

	return gSignalControlpara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;
}

static void loadTimeIntervalFromIni()	//װ��ʱ�α�
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

static void loadPlanScheduleFromIni()	//װ�ص��ȱ�
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

#define SEND_TIMEINTERVAL_TIMES		16	//�ܹ�����16��ʱ�α���Ϣ
#define SEND_TIMEINTERVAL_NUM		48	//ÿ�η���48��
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
		del_section(section);	//ɾ��֮ǰ������
		if (item[i].nActionID == 0)	//��Ч�����þͲ���д�������ļ���
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

#define SEND_PLANSCHEDULE_TIMES		4	//�ܹ�����4�ε��ȱ���Ϣ
#define	SEND_PLANSCHEDULE_NUM		10	//ÿ�η���10��
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
* ��ת��Ӧģʽ
*
***********************************************************************************/

static void udp_send_jump_response()
{
	//����UDP������
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
//ʹ��161�˿�
	localAddr.sin_port = htons(161);
//�򱾻�161�˿ڷ���
	DBG("��ת��Ӧ����\n");
	int len = sendto(socketFd,responseSend,47,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		DBG("Send inductive control signal failed!!!\n");
	}
	close(socketFd);
	
}

/*********************************************************************************
*
* ���͸ı���Ʒ�ʽ����
*
***********************************************************************************/

static void udp_send_change_ctrl()
{
	//����UDP������
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
	//ʹ��161�˿�
    	localAddr.sin_port = htons (161);
	//�򱾻�161�˿ڷ���
	DBG("�ı���Ʒ�ʽ\n");
	int len = sendto(socketFd,responseSend,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		DBG("Send system control signal failed!!!\n");
	}
	close(socketFd);
	
}

//����ʱ���
static inline int timeGap(struct timeval *new, struct timeval *old)
{
	return (new->tv_sec - old->tv_sec) * 1000000 + new->tv_usec - old->tv_usec;
}

enum {	//����һ��ö��������������־ʹ��
	NOUSED = 0,		//����δʹ��
	INIT,			//�����ǳ�ʼ��
	CHANGE,			//������ʱ�����ڸı���Ʒ�ʽ
};

void adjustControlType()	//�������Ʒ�ʽ
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
    if (flag == INIT) {	//��ʼ��
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
	if (lastActionId == YELLOWBLINKID && nowActionId != YELLOWBLINKID) {//Ҫ�ӻ����л����������Ʒ�ʽ
		//gettimeofday(&nowTime, NULL);
		if (oldTime.tv_sec == 0 && oldTime.tv_usec == 0) {	//�����տ�ʼ�ı���Ʒ�ʽ
			oldTime = nowTime;	//��¼��ǰʱ��������ʱ
			flag = CHANGE;		//�ı��־˵����ʱ�����ڸı���Ʒ�ʽ
			udp_send_jump_response();	//�ȷ���һ�θ�Ӧ��Ϣ
		} else {
			//����ʱ������500msʱ����ϵͳ������Ϣ
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
