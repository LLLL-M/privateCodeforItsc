/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
#include "ykconfig.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //��ʱ��ת���ɷ�
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //һ���е����ʱ���ֵ����24Сʱ

#define MOTOR		2	//������ͨ������
#define PEDESTRIAN	3	//����ͨ������

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
 
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static YK_Config *gRunConfigPara = NULL;//�źŻ����е����ò���
static UInt8 gSingleAdaptRunCycleNum = 0;	//��������Ӧ�������ڸ���
static YK_SchemeItem gSingleAdaptScheme;	//��������Ӧ����

static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static YK_RealTimeInfo gRealTimeInfo;		//ʵʱ��Ϣ
static LineQueueData gCurRunData;		//��ǰ��1s����������

void ItsSetSingleAdaptScheme(YK_SchemeItem *pScheme)
{
	if (pScheme == NULL)
		return;
	memcpy(&gSingleAdaptScheme, pScheme, sizeof(YK_SchemeItem));
	gSingleAdaptRunCycleNum = gRunConfigPara->wholeConfig.adaptCtlEndRunCycleNum;	//Ĭ������3�����ں����
}

void *ItsAllocConfigMem(void *config, int configSize)
{
	gRunConfigPara = calloc(1, configSize);
	if(gRunConfigPara != NULL)
	{
		memcpy(gRunConfigPara, config, configSize);
	}
	return gRunConfigPara;
}

void ItsUnitInit(void (*initFunc)(UInt8, UInt8))
{
	initFunc(gRunConfigPara->wholeConfig.bootYellowBlinkTime, gRunConfigPara->wholeConfig.bootAllRedTime);
}

static void SetFollowPhase(CalInfo *calInfo, UInt16 *channelIncludePhaseBits)
{
	YK_ChannelItem *pChannelTable = (YK_ChannelItem *)&gRunConfigPara->channelTable;
	FollowPhaseInfo *followPhaseInfos = calInfo->followPhaseInfos;
	int ch, ph, fo;
	int channelIncludePhaseNum;	//ͨ����������λ����
	
	if (channelIncludePhaseBits == NULL)
		return;
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		if (channelIncludePhaseBits[ch] == 0)
			continue;
		channelIncludePhaseNum = 0;
		for (ph = 0; ph < MAX_PHASE_NUM; ph++)
		{
			if (GET_BIT(channelIncludePhaseBits[ch], ph) == 1)
				channelIncludePhaseNum++;
		}
		if (channelIncludePhaseNum <= 1)
			continue;
		//˵��һ��ͨ���ڶ����λ�з���������Ϊ������λ
		for (fo = 0; fo < MAX_FOLLOWPHASE_NUM; fo++)
		{
			if (followPhaseInfos[fo].phaseBits == 0 
				|| followPhaseInfos[fo].phaseBits == channelIncludePhaseBits[ch])
			{
				followPhaseInfos[fo].phaseBits = channelIncludePhaseBits[ch];
				if (pChannelTable[ch].nControllerType == MOTOR)
					SET_BIT(followPhaseInfos[fo].motorChannelBits, ch);
				else if (pChannelTable[ch].nControllerType == PEDESTRIAN)
					SET_BIT(followPhaseInfos[fo].pedChannelBits, ch);
			}
		}
	}
}	

static Boolean SetChannelBits(CalInfo *calInfo, YK_SchemeItem *pScheme)
{
	YK_ChannelItem *pChannelTable = (YK_ChannelItem *)&gRunConfigPara->channelTable;
	YK_PhaseInfo *phaseInfo;
	UInt16 channelIncludePhaseBits[NUM_CHANNEL] = {0};	//ͨ����������λbit
	int ch, ph;
	UInt32 conflictChannelBits = 0;
	
	if (pScheme == NULL)
		return FALSE;
	phaseInfo = pScheme->phaseInfo;
	//����ͨ������λ�Ķ�Ӧ��ϵ,������Ƿ���ͨ���̳�ͻ
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		if (pChannelTable[ch].nChannelID == 0 || pChannelTable[ch].nChannelID > NUM_CHANNEL 
			|| (pChannelTable[ch].nControllerType != MOTOR && pChannelTable[ch].nControllerType != PEDESTRIAN))
			continue;
		for (ph = 0; ph < pScheme->totalPhaseNum; ph++)
		{
			if (GET_BIT(phaseInfo[ph].channelBits, ch) == 1)
			{
				conflictChannelBits = phaseInfo[ph].channelBits & pChannelTable[ch].conflictChannel;
				if (conflictChannelBits == 0)
				{	//��ͨ����ͻ
					if (pChannelTable[ch].nControllerType == MOTOR)
						SET_BIT(calInfo->phaseTimes[ph].motorChannelBits, ch);
					else if (pChannelTable[ch].nControllerType == PEDESTRIAN)
						SET_BIT(calInfo->phaseTimes[ph].pedChannelBits, ch);
					SET_BIT(channelIncludePhaseBits[ch], ph);
					if (pChannelTable[ch].nVehDetectorNum > 0 && pChannelTable[ch].nVehDetectorNum <= 48)
						SET_BIT(calInfo->phaseTimes[ph].vehicleDetectorBits, pChannelTable[ch].nVehDetectorNum - 1);
					//INFO("!!!!!!set channel %d type %d", ch + 1, pChannelTable[ch].nControllerType);
				}
				else
				{	//��һ����λ�а��������ͻ��ͨ��ʱ�������̳�ͻ
					SET_BIT(conflictChannelBits, ch);
					ItsCtl(FAULT_CONTROL, YELLOWBLINK_SCHEMEID, 0);
					ItsWriteFaultLog(GREEN_CONFLICT, conflictChannelBits);
					log_error("scheme %d occur green conflict, conflictChannelBits: %#x", pScheme->nSchemeId, conflictChannelBits);
					return FALSE;
				}
			}
				
		}
	}
	SetFollowPhase(calInfo, channelIncludePhaseBits);
	return TRUE;
}

static Boolean SetPhaseTime(CalInfo *calInfo, YK_SchemeItem *pScheme)
{  
    int i = 0;
	YK_PhaseInfo *phaseInfo;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	UInt16 cycleTime = 0;
	
	if (pScheme == NULL || pScheme->totalPhaseNum > MAX_PHASE_NUM)
		return FALSE;
	phaseInfo = pScheme->phaseInfo;
	calInfo->cycleTime = pScheme->cycleTime;
    for (i = 0; i < pScheme->totalPhaseNum; i++)
    {
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = phaseInfo[i].greenTime + phaseInfo[i].greenBlinkTime + phaseInfo[i].yellowTime + phaseInfo[i].redYellowTime + phaseInfo[i].allRedTime;
		passTimeInfo->greenTime = phaseInfo[i].greenTime;
		passTimeInfo->greenBlinkTime = phaseInfo[i].greenBlinkTime;
		passTimeInfo->yellowTime = phaseInfo[i].yellowTime;
		passTimeInfo->redYellowTime = phaseInfo[i].redYellowTime;
		passTimeInfo->allRedTime = phaseInfo[i].allRedTime;
		phaseTimes[i].pedestrianClearTime = phaseInfo[i].pedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = 1;
		phaseTimes[i].unitExtendGreen = phaseInfo[i].unitExtendTime;
		phaseTimes[i].maxExtendGreen = phaseInfo[i].maxGreenTime - phaseInfo[i].minGreenTime;
		cycleTime += phaseTimes[i].splitTime;
		
		calInfo->stageInfos[i].runTime = phaseTimes[i].splitTime;
		calInfo->stageInfos[i].includeNum = 1;
		calInfo->stageInfos[i].includePhases[0] = i + 1;
		calInfo->includeNums[i] = 1;
		calInfo->phaseIncludeStage[i][0] = i + 1;
    }
	calInfo->maxStageNum = i;
	if (cycleTime != calInfo->cycleTime || calInfo->maxStageNum < 2)
	{
		log_error("all phase time sum[%d] of the scheme %d isn't eqult to cycleTime %d, maxStageNum = %d", cycleTime, calInfo->cycleTime, calInfo->maxStageNum);
		return FALSE;
	}
	else
		return SetChannelBits(calInfo, pScheme);
}

/*****************************************************************************
 �� �� ��  : GetTimeIntervalID
 ��������  : �������ȱ����ݵ�ǰʱ�䣬ȷ����ǰ����ʱ�α��
 �������  : struct tm *now     ��ǰʱ��ָ��
 �������  : 
 �� �� ֵ  : ��ǰ���е�ʱ�α��
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static UInt8 GetTimeIntervalID(struct tm *now)
{
    int i = 0;

	if (gRunConfigPara->wholeConfig.weekPreemptSchedule == 1 && now->tm_wday >= 0 && now->tm_wday < 7)	//�������ȱ�����
	{	//tm_wday == 0����������
		if (gRunConfigPara->scheduleTable.week[now->tm_wday] > 0 && gRunConfigPara->scheduleTable.week[now->tm_wday] <=NUM_TIME_INTERVAL)
			return gRunConfigPara->scheduleTable.week[now->tm_wday];
	}
	for(i = 0; i < NUM_SPECIAL_DATE; i++) 
	{	//Ĭ������ʹ���������ڵ�ʱ�κ�
		if ((GET_BIT(gRunConfigPara->scheduleTable.specialDate[i].month, now->tm_mon + 1) == 1)//tm_mon��Χ��[0,11]
			&& (GET_BIT(gRunConfigPara->scheduleTable.specialDate[i].day, now->tm_mday) == 1))
			return gRunConfigPara->scheduleTable.specialDate[i].nTimeIntervalID;
	}
	return (now->tm_wday >= 0 && now->tm_wday < 7) ? gRunConfigPara->scheduleTable.week[now->tm_wday] : 0;
}



/*****************************************************************************
 �� �� ��  : GetSchemeIdAndTimeGap
 ��������  : ���ݵ�ǰʱ�䣬�ж�Ӧ�����ĸ�ʱ�Σ�����ȷ����ǰʱ����Ҫִ�еķ���
 �������  : struct tm *now			��ǰʱ��ָ��
			 int *ret				���ڷ��ص�ǰʱ��������ʱ����ʼʱ��Ĳ�ֵ
 �������  : ��
 �� �� ֵ  : ��ǰ���еĶ�����

 �޸���ʷ      :
  1.��    ��   : 2014��11��20��
    ��    ��   : Jicky
    �޸�����   : �޸�������������������޸��˺���ʵ��

*****************************************************************************/
UInt8 GetSchemeIdAndTimeGap(CalInfo *calInfo, struct tm *now, int *ret)
{
	UInt8 hour, minute;
    int i = 0, index = -1;
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nSchemeID = 0;
	YK_TimeIntervalItem *item = NULL;
	
	nTimeIntervalID = GetTimeIntervalID(now);//���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > NUM_TIME_INTERVAL)       
	{
		//log_error("can't find timeinterval id");
        return 0;
	}
	item = (YK_TimeIntervalItem *)&gRunConfigPara->timeIntervalTable[nTimeIntervalID - 1];
    //�˺����е�ʱ��ֵ��λΪ����
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//Ԥ������һ�����Ĳ�ֵ
	do
	{	//ѭ���ҳ���ǰʱ����ʱ�α��в�ֵ��С��ʱ������Ӧ��actionID��Ϊ��ǰӦ�����е�actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if (gRunConfigPara != NULL && item[i].nSchemeId  == 0) 
			{ //��˵����ʱ�ο���û�б�ʹ�ã�ֱ��continue��
				continue;
			}
			nTempTime = HoursToMinutes(item[i].cStartTimeHour, item[i].cStartTimeMinute);
			if (nCurrentTime == nTempTime) 
			{
				index = i;
				minTimeGap = 0;
			} 
			else if (nCurrentTime > nTempTime) 
			{
				nTimeGap = nCurrentTime - nTempTime;
				if (nTimeGap < minTimeGap) 
				{
					minTimeGap = nTimeGap;
					index = i;
				}
			}
		}
		
		if (nTempTime == -1)
		{
			//log_error("the nTimeIntervalID %d isn't configed", nTimeIntervalID);
			return 0;	//˵����ʱ�α�û������
		}
		if (minTimeGap == MAX_TIME_GAP) 
		{ //˵����ǰʱ�䴦��ʱ�α�����С��λ��
			nCurrentTime += MAX_TIME_GAP;	//�ѵ�ǰʱ������24СʱȻ���ٴ�ѭ��
		}
	} while (index == -1);
	if (ret != NULL)
	{
	    *ret = minTimeGap;
		if (calInfo != NULL)
		{
			calInfo->timeIntervalId = nTimeIntervalID;
			if (item[index].IsCorrdinateCtl == 1)
			{
				calInfo->coordinatePhaseId = 1;	//Ĭ����λ1��ΪЭ����λ
				calInfo->phaseOffset = item[index].phaseOffset;
				calInfo->transitionCycle = gRunConfigPara->wholeConfig.transitionCycle;
			}
		}
	}
	return item[index].nSchemeId;
}
//ʵʱ��Ӧ����Ĭ��ʹ�÷���1����λ˳��
static Boolean SetInductiveControlTime(CalInfo *calInfo)
{
    int i = 0;
	YK_PhaseInfo *phaseInfo = gRunConfigPara->schemeTable[0].phaseInfo;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	
	calInfo->cycleTime = 0;
    for(i = 0; i < gRunConfigPara->schemeTable[0].totalPhaseNum; i++)
    {
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = phaseInfo[i].minGreenTime + phaseInfo[i].yellowTime + phaseInfo[i].redYellowTime + phaseInfo[i].allRedTime;
		passTimeInfo->greenTime = phaseInfo[i].minGreenTime - phaseInfo[i].greenBlinkTime;
		passTimeInfo->greenBlinkTime = phaseInfo[i].greenBlinkTime;
		passTimeInfo->yellowTime = phaseInfo[i].yellowTime;
		passTimeInfo->redYellowTime = phaseInfo[i].redYellowTime;
		passTimeInfo->allRedTime = phaseInfo[i].allRedTime;
		phaseTimes[i].pedestrianClearTime = phaseInfo[i].pedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = 1;
		phaseTimes[i].unitExtendGreen = phaseInfo[i].unitExtendTime;
		phaseTimes[i].maxExtendGreen = phaseInfo[i].maxGreenTime - phaseInfo[i].minGreenTime;
		calInfo->cycleTime += phaseTimes[i].splitTime;
		
		calInfo->stageInfos[i].runTime = phaseTimes[i].splitTime;
		calInfo->stageInfos[i].includeNum = 1;
		calInfo->stageInfos[i].includePhases[0] = i + 1;
		calInfo->includeNums[i] = 1;
		calInfo->phaseIncludeStage[i][0] = i + 1;
    }
	calInfo->maxStageNum = i;
	if (calInfo->maxStageNum < 2)
	{
		log_error("the maxStageNum %d is too small!", calInfo->maxStageNum);
		return FALSE;
	}
	return SetChannelBits(calInfo, &gRunConfigPara->schemeTable[0]);
}

Boolean FillCalInfo(CalInfo *calInfo, UInt8 schemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//��ǰʱ���뵱ǰʱ�ε���ʼʱ��Ĳ�ֵ����λΪ����

	localtime_r(&calTime, &now);
	memset(calInfo, 0, sizeof(CalInfo));
	calInfo->collectCycle = gRunConfigPara->wholeConfig.vehFlowCollectCycleTime;
	calInfo->checkTime = 3;
	if (gSingleAdaptRunCycleNum > 0)
	{
		gSingleAdaptRunCycleNum--;
		calInfo->schemeId = SINGLE_ADAPT_SCHEMEID;
	}
	else
		calInfo->schemeId = (schemeId > 0) ? schemeId : GetSchemeIdAndTimeGap(calInfo, &now, &timeGapMinutes);
    if (calInfo->schemeId == YELLOWBLINK_SCHEMEID || calInfo->schemeId == ALLRED_SCHEMEID || calInfo->schemeId == TURNOFF_SCHEMEID)
	{
		log_debug("special control, scheme: %d", calInfo->schemeId);
        return TRUE;
	}
	else if (calInfo->schemeId == INDUCTIVE_SCHEMEID)	
    {	//����Ǹ�Ӧ����Ӧ��ʹ����С��+�Ƶ�ʱ��+���ʱ��+ȫ��ʱ����Ϊ���ű�ʱ��
        return SetInductiveControlTime(calInfo);
    }
	else if (calInfo->schemeId == SINGLE_ADAPT_SCHEMEID)
		return SetPhaseTime(calInfo, &gSingleAdaptScheme);
	else if(calInfo->schemeId > 0 && calInfo->schemeId <= NUM_SCHEME)
	{
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
		//���ݵ�ǰ�ķ����ţ�����λ���и���λ�����ű�ʱ��θ�ֵ
		return SetPhaseTime(calInfo, &gRunConfigPara->schemeTable[calInfo->schemeId - 1]);
	}
	else
	{
		log_error("can't find scheme id, action: %d", calInfo->actionId);
        return FALSE;
	}
}

void SetRealTimeInfo(const ChannelLockParams *lockparams, const LineQueueData *data)
{
	int i;
	
	pthread_rwlock_wrlock(&gCountDownLock);
	memcpy(gRealTimeInfo.allChannels, data->allChannels, sizeof(data->allChannels));
	gRealTimeInfo.cycleTime = data->cycleTime;
	gRealTimeInfo.schemeId = (lockparams->ucChannelLockStatus == 1) ? CHANNEL_LOCK_SCHEMEID : data->schemeId;
	if (data->schemeId > 0 && data->schemeId <= NUM_SCHEME)
	{
		for (i = 0; i < gRunConfigPara->schemeTable[data->schemeId - 1].totalPhaseNum; i++)
		{
			if (data->phaseInfos[i].phaseStatus == GREEN)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 1;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == GREEN_BLINK)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 2;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == YELLOW)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 3;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == RED_YELLOW)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 4;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
			else if (data->phaseInfos[i].phaseStatus == ALLRED)
			{
				gRealTimeInfo.runPhase = i + 1;
				gRealTimeInfo.stepNum = 5;
				gRealTimeInfo.stepLeftTime = data->phaseInfos[i].phaseLeftTime;
				break;
			}
		}
	}
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsCountDownGet(void *countdown, int size)
{
	if (countdown == NULL || size != sizeof(YK_RealTimeInfo))
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(countdown, &gRealTimeInfo, size);
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsGetCurRunData(LineQueueData *data)
{
	if (data == NULL)
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(data, &gCurRunData, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}
