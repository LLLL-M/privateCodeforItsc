#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include "HikConfig.h"
#include "parse_ini.h"
#include "specialControl.h"

SignalControllerPara *gSignalControlpara = NULL;
static SignalControllerPara *gConfigPara = NULL;//�źŻ����ò�������
pthread_rwlock_t gConfigLock = PTHREAD_RWLOCK_INITIALIZER;	//��Ҫ�������̷߳���ȫ������
static int gHikConfigNum = 0;

#define YELLOWBLINKID	119

static void ReadHikConfigFile(void)
{
	int hikconfignum[2] = {0, 0};
	char *filePath[2] = {CFG_NAME, CFG_BAK_NAME};
	FILE *fp[2], *file;
	char *path;
	int i;
	
	for (i = 0; i < 2; i++)
	{
		fp[i] = fopen(filePath[i], "r");
		if (NULL == fp[i])
			continue;
		if (0 == fseek(fp[i], sizeof(SignalControllerPara), SEEK_CUR))
			fread(&hikconfignum[i], sizeof(int), 1, fp[i]);
		fclose(fp[i]);
	}
	gHikConfigNum = max(hikconfignum[0], hikconfignum[1]);
	if (gHikConfigNum == 0)
		return;
	if (hikconfignum[0] > hikconfignum[1])
	{
		system("cp "CFG_NAME" "CFG_BAK_NAME);
		path = filePath[0];
	}
	else if (hikconfignum[0] < hikconfignum[1])
	{
		system("cp "CFG_BAK_NAME" "CFG_NAME);
		path = filePath[1];
	}
	else
		path = filePath[0];
	sync();
	file = fopen(path, "r");
	if (NULL == file)
		return;
	fread(gSignalControlpara, sizeof(SignalControllerPara), 1, file);
	fclose(file);
}
void InitSignalMachineConfig(void)
{
	int ret = 0;
	if ((gSignalControlpara = (SignalControllerPara *)calloc(2, sizeof(SignalControllerPara))) == NULL)
	{
		ERR("memory is not enough to store the config information\n");
		exit(1);
	}
	gConfigPara = gSignalControlpara + 1;
	ReadHikConfigFile();
	ret = IsSignalControlparaLegal(gSignalControlpara);
	if (ret != 0)
	{
		ERR("the local config is invalid, ret = %d\n", ret);
		memset(gSignalControlpara, 0, sizeof(SignalControllerPara));
	}
	else
	{
		memmove(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
	}
}
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
UInt8 GetTimeIntervalID(struct tm *now)
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

	DBG("Current time has not config timeinterval!");
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
UInt8 GetActionID()
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

#define SEND_TIMEINTERVAL_TIMES		16	//�ܹ�����16��ʱ�α���Ϣ
#define SEND_TIMEINTERVAL_NUM		48	//ÿ�η���48��
static inline void StoreTimeInterval(void *arg)
{
	TimeIntervalItem *item = (TimeIntervalItem *)arg;
	int i;
	
	for (i = 0; i < SEND_TIMEINTERVAL_NUM; i++) 
	{
		if (item[i].nActionID == 0)	//��Ч�����þͲ���д�������ļ���
			continue;
		gConfigPara->stTimeInterval[item[i].nTimeIntervalID - 1][item[i].nTimeID - 1] = item[i];

		DBG("[stTimeInterval]  nTimeIntervalID: %d, nTimeID: %d, cStartTimeHour: %d, cStartTimeMinute: %d, nActionID: %d\n",
				item[i].nTimeIntervalID,
				item[i].nTimeID,
				item[i].cStartTimeHour,
				item[i].cStartTimeMinute,
				item[i].nActionID);
	}
}

#define SEND_PLANSCHEDULE_TIMES		4	//�ܹ�����4�ε��ȱ���Ϣ
#define	SEND_PLANSCHEDULE_NUM		10	//ÿ�η���10��
static inline void StorePlanSchedule(void *arg)
{
	PlanScheduleItem *item = (PlanScheduleItem *)arg;
	int i;
	
	for (i = 0; i < SEND_PLANSCHEDULE_NUM; i++) 
	{
		if (item[i].nTimeIntervalID == 0)
			continue;
		gConfigPara->stPlanSchedule[item[i].nScheduleID - 1] = item[i];
		
		DBG("[stPlanSchedule] nScheduleID: %d, nTimeIntervalID: %d, month: %d, week: %d, day: %u\n",
				item[i].nScheduleID, 
				item[i].nTimeIntervalID,
				item[i].month,
				item[i].week,
				item[i].day);
	}
}

static inline void StoreUnit(void *arg)
{
	memcpy(&gConfigPara->stUnitPara, arg, sizeof(UnitPara));
}
/*****************************************************************************
 �� �� ��  : StorePhase
 ��������  : ������λ����
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��18��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��λ����ȫ��ʱ�䡢��λ�ӳ��̡��Ƶ�ʱ��SDK�������ĵ�λ��1/10��
                 ���������ﱣ��ʱҪ����10���Ա�֤У����ȷ��
*****************************************************************************/
static inline void StorePhase(void *arg)
{
    int i = 0;
    
	memcpy(gConfigPara->stPhase, arg, sizeof(gConfigPara->stPhase));

    for(i = 0; i < NUM_PHASE; i++)
    {
        if(gConfigPara->stPhase[i].nCircleID != 0)
        {
            gConfigPara->stPhase[i].nAllRedTime = (gConfigPara->stPhase[i].nAllRedTime / 10);
            gConfigPara->stPhase[i].nYellowTime= (gConfigPara->stPhase[i].nYellowTime/ 10);
            gConfigPara->stPhase[i].nUnitExtendGreen= (gConfigPara->stPhase[i].nUnitExtendGreen/ 10);
        }
    }
	
}
static inline void StoreTransform(void *arg)
{
	memcpy(gConfigPara->AscSignalTransTable, arg, sizeof(gConfigPara->AscSignalTransTable));
}
#define SEND_PHASETURN_TIMES	16	//�ܹ�����16���������Ϣ
#define	SEND_PHASETURN_NUM		4	//ÿ�η���4��
static inline void StorePhaseTurn(void *arg)
{
    PhaseTurnItem_ *item = (PhaseTurnItem_ *)arg;
	
    int i = 0;

    for(i = 0 ; i < SEND_PHASETURN_NUM; i++)
    {
        memcpy((unsigned char *)&(gConfigPara->stPhaseTurn[item[i].nPhaseTurnID - 1][item[i].nCircleID - 1]), (unsigned char *)&item[i], sizeof(PhaseTurnItem));//�������λ��Ŀ����16������32��
    }

}
#define SEND_SPLIT_TIMES	36	//�ܹ�����36�����űȱ���Ϣ
#define	SEND_SPLIT_NUM		16	//ÿ�η���16��
static inline void StoreSplit(void *arg)
{
	GreenSignalRationItem *item = (GreenSignalRationItem *)arg;
	int i;
	
	for (i = 0; i < SEND_SPLIT_NUM; i++)
	{
		if (item[i].nGreenSignalRationTime == 0)
			continue;
		gConfigPara->stGreenSignalRation[item[i].nGreenSignalRationID - 1][item[i].nPhaseID - 1] = item[i];
	}
}
static inline void StoreChannel(void *arg)
{
	memcpy(gConfigPara->stChannel, arg, sizeof(gConfigPara->stChannel));
}
#define SEND_SCHEME_TIMES	6	//�ܹ�����6�η�������Ϣ
#define	SEND_SCHEME_NUM		18	//ÿ�η���18��
static inline void StoreScheme(void *arg)
{
	SchemeItem *item = (SchemeItem *)arg;
	int i;
	
	for (i = 0; i < SEND_SCHEME_NUM; i++)
	{
		if (item[i].nCycleTime == 0)
			continue;
		gConfigPara->stScheme[item[i].nSchemeID - 1] = item[i];
	}
}
#define SEND_ACTION_TIMES	17	//�ܹ�����17�ζ�������Ϣ
#define	SEND_ACTION_NUM		15	//ÿ�η���15��
static inline void StoreAction(void *arg)
{
	ActionItem *item = (ActionItem *)arg;
	int i;
	
	for (i = 0; i < SEND_ACTION_NUM; i++)
	{
		if (item[i].nSchemeID == 0)
			continue;
		gConfigPara->stAction[item[i].nActionID - 1] = item[i];
	}
}
#define SEND_VEHICLEDETECTOR_TIMES	4	//�ܹ�����4�γ������������Ϣ
#define	SEND_VEHICLEDETECTOR_NUM	18	//ÿ�η���18��
static inline void StoreVehicleDetector(void *arg)
{
	struct STRU_N_VehicleDetector *item = (struct STRU_N_VehicleDetector *)arg;
	int i;
	
	for (i = 0; i < SEND_VEHICLEDETECTOR_NUM; i++)
	{
		if (item[i].byVehicleDetectorCallPhase == 0)
			continue;
		gConfigPara->AscVehicleDetectorTable[item[i].byVehicleDetectorNumber - 1] = item[i];
	}
}
static inline void StorePedestrianDetector(void *arg)
{
	memcpy(gConfigPara->AscPedestrianDetectorTable, arg, sizeof(gConfigPara->AscPedestrianDetectorTable));
}
static inline void StoreFollowPhase(void *arg)
{
	memcpy(gConfigPara->stFollowPhase, arg, sizeof(gConfigPara->stFollowPhase));
}

static UInt32 g_DownloadFlag = 0;//��ŵ��ǿ�ʼ����ʱSDK�������ľ�����Щ������Ҫ�����flag

void StoreBegin(void *arg)
{
	UInt32 flag = *(UInt32 *)arg;
	int i;
	
	//INFO("StoreBegin flag = %x", flag);
	g_DownloadFlag = flag;
	for (i = 0; i < 32; i++)
	{
		if (BIT(flag, i) == 0)
			continue;
		switch (i)
		{
			case 2: memset(gConfigPara->stPhaseTurn, 0, sizeof(gConfigPara->stPhaseTurn)); break;
			case 3: memset(gConfigPara->stGreenSignalRation, 0, sizeof(gConfigPara->stGreenSignalRation)); break;
			case 4: memset(gConfigPara->AscVehicleDetectorTable, 0, sizeof(gConfigPara->AscVehicleDetectorTable)); break;
			case 5: memset(gConfigPara->AscPedestrianDetectorTable, 0, sizeof(gConfigPara->AscPedestrianDetectorTable)); break;
			case 7: memset(gConfigPara->stScheme, 0, sizeof(gConfigPara->stScheme)); break;
			case 8: memset(gConfigPara->stAction, 0, sizeof(gConfigPara->stAction)); break;
			case 9: memset(gConfigPara->stTimeInterval, 0, sizeof(gConfigPara->stTimeInterval)); break;
			case 10: memset(gConfigPara->stPlanSchedule, 0, sizeof(gConfigPara->stPlanSchedule)); break;
			default: break;
		}
	}
}

void StoreComplete()
{
	FILE *fp;
	int ret = IsSignalControlparaLegal(gConfigPara);
	if (ret != 0)
	{
		ERR("the local config is invalid, ret = %d\n", ret);
		//WriteConfigFile(gConfigPara,"./hikconfig_bak.ini");
		memmove(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
	}
	else
	{
		pthread_rwlock_wrlock(&gConfigLock);
		memmove(gSignalControlpara, gConfigPara, sizeof(SignalControllerPara));
		pthread_rwlock_unlock(&gConfigLock);
		
		if (BIT(g_DownloadFlag, 16) == 0)	//��16λΪ0��������Ҫд��flash�����Ҳ�Ͳ���д�������ļ�
			return;
		fp = fopen(CFG_NAME, "w");
		if (fp == NULL)
			return;
		gHikConfigNum++;
		fwrite(gSignalControlpara, sizeof(SignalControllerPara), 1, fp);
		fwrite(&gHikConfigNum, sizeof(int), 1, fp);
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);

		system("cp "CFG_NAME" "CFG_BAK_NAME);
		sync();
	}
}
void DownloadConfig(int type, void *arg)
{
	//INFO("receive download information!");
	switch (type)
	{
		case 0xaa: StoreTimeInterval(arg);  break;
		case 0xab: StorePlanSchedule(arg); break;
		case 0xac: StoreUnit(arg); break;
		case 0xad: StorePhase(arg); break;
		case 0xae: StorePedestrianDetector(arg); break;
		case 0xaf: StoreChannel(arg); break;
		case 0xb0: StoreVehicleDetector(arg); break;
		case 0xb1: StoreScheme(arg); break;
		case 0xb2: StoreSplit(arg); break;
		case 0xb3: StorePhaseTurn(arg); break;
		case 0xb4: StoreAction(arg); break;
		case 0xb5: StoreFollowPhase(arg); break;
		case 0xb6: StoreTransform(arg); break;
		//case 183: StoreBegin(arg); break;
		//case 184: StoreComplete(arg); break;
		default:  break;
	}
}

//����ʱ���
static inline int TimeGap(struct timeval *new, struct timeval *old)
{
	return (new->tv_sec - old->tv_sec) * 1000000 + new->tv_usec - old->tv_usec;
}

enum {	//����һ��ö��������������־ʹ��
	NOUSED = 0,		//����δʹ��
	CHANGE,			//������ʱ�����ڸı���Ʒ�ʽ
};

void adjustControlType()	//�������Ʒ�ʽ
{
	static char flag = NOUSED;
	static unsigned char lastActionId = 0, nowActionId = 0;
	static struct timeval oldTime = {0, 0};
	struct timeval nowTime = {0, 0};
//#define DEBUG
#ifdef DEBUG
	static struct timeval tmpTime = {0, 0};
#endif
	gettimeofday(&nowTime, NULL);
#ifdef DEBUG
	if (tmpTime.tv_sec == 0)
		tmpTime = nowTime;
#endif
	
	if (flag == NOUSED) {
		pthread_rwlock_rdlock(&gConfigLock);
		nowActionId = GetActionID();
		pthread_rwlock_unlock(&gConfigLock);
#ifdef DEBUG
		if (TimeGap(&nowTime, &tmpTime) > 1000000) {
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
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_INDUCTION);	//�ȷ���һ�θ�Ӧ��Ϣ
		} else {
			//����ʱ������500msʱ����ϵͳ������Ϣ
			if (TimeGap(&nowTime, &oldTime) >= 500000) {
				SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_SYSTEM);	//����ϵͳ������Ϣ
				oldTime.tv_sec = 0;
				oldTime.tv_usec = 0;
				flag = NOUSED;
				lastActionId = 0;
			}
		}
	}
}
