/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : calculate.c
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��11��27��
  ����޸�   :
  ��������   : ���Կ���ģ�飬��Ҫ���Ƶ���߼�
  �����б�   :
              GetActionID
              GetPhaseTurnAndSetPhaseTimeInfo
              GetTimeIntervalID
			  GetPhaseStatusByTime
              InitChannelLightStatus
              IsPhaseInPhaseTurn
              RunCycle
              SetCurrentChannelStatus
              SetPhaseTime
              SignalControllerRun
  �޸���ʷ   :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "hikmsg.h"
#include "lfq.h"
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //��ʱ��ת���ɷ�
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //һ���е����ʱ���ֵ����24Сʱ

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern int msgid;
extern void *gHandle;

extern pthread_rwlock_t gConfigLock;
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void CalStageInfo(void);
extern void IgnorePhaseDeal(void);
extern void TransitionDeal(void);
extern Boolean CalculateByGbconfig(struct msgbuf *msg);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

CalInfo gCalInfo;
CustomInfo gCustomInfo = {
	.checkTime = 3,		//Ĭ�ϸ�Ӧ���ʱ��Ϊ3s
	.ignoreOption = 1,	//Ĭ�Ϻ�����λ��ǰ����
};

/*****************************************************************************
 �� �� ��  : SetPhaseTime
 ��������  : �������űȺų�ʼ��ÿ����λ�ĸ���ʱ��Σ�������Э����λ�źͺ�����λ����
 �������  : UInt8  nGreenSignalRatioID  ���űȺ�
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetPhaseTime(void)
{  
    int i = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;

    for(i = 0 ; i < NUM_PHASE; i++)
    {
		splitItem = &gRunConfigPara->stGreenSignalRation[gCalInfo.splitId - 1][i];
		phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i];
		if ((phaseItem->nCircleID == 0) || (splitItem->nGreenSignalRationTime == 0))
			continue;
		if (splitItem->nIsCoordinate)
		{	//˵��������Э����λ����ʱ���û�������������Ӧ��ִ��Э������
			if (gCalInfo.coordinatePhaseId == 0)	//���ö��Э����λʱ���������ҵ���Ϊ׼
				gCalInfo.coordinatePhaseId = splitItem->nPhaseID;
			SET_BIT(gCalInfo.isCoordinatePhase, i);
		}
		if (splitItem->nType == IGNORE_PHASE)
		{
			SET_BIT(gCalInfo.isIgnorePhase, i);
		}

		phaseTimes[i].splitTime = splitItem->nGreenSignalRationTime;
		phaseTimes[i].greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		phaseTimes[i].greenBlinkTime = entry->nGreenLightTime;
		phaseTimes[i].yellowTime = phaseItem->nYellowTime;
		phaseTimes[i].allRedTime = phaseItem->nAllRedTime;
    }
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
    int i = 0, schduleNum = GET_PROTOCOL_PARAM(NUM_SCHEDULE, MAX_SCHEDULE_LIST_NUM);
	UInt16 month;
	UInt8 week;
	UInt32 day;

	for(i = 0 ; i < schduleNum ; i++) { //�����ǰʱ���ĳһ����ȱ�������򷵻ظõ��ȱ��ʱ�κ�
		month = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].month, gRunGbconfig->scheduleTable[i].month);
		day = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].day, gRunGbconfig->scheduleTable[i].day);
		week = GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].week, gRunGbconfig->scheduleTable[i].week);
        if (BIT(month, now->tm_mon + 1) == 1)
		{	//tm_mon��Χ��[0,11]
			if ((BIT(day, now->tm_mday) == 1)
				|| (BIT(week, now->tm_wday + 1) == 1))//tm_wday == 0����������
			    return GET_PROTOCOL_PARAM(gRunConfigPara->stPlanSchedule[i].nTimeIntervalID, gRunGbconfig->scheduleTable[i].timeIntervalListNo);
		}
    }

    return 0;
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
UInt8 GetSchemeIdAndTimeGap(struct tm *now, int *ret)
{
	UInt8 hour, minute;
    int i = 0, index = -1;
	int maxTimeintervalNum = GET_PROTOCOL_PARAM(NUM_TIME_INTERVAL, MAX_TIMEINTERVAL_LIST_NUM);
	int maxTimeintervalIdNum = GET_PROTOCOL_PARAM(NUM_TIME_INTERVAL_ID, MAX_TIMEINTERVAL_NUM);
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nSchemeID = 0;
	
	nTimeIntervalID = GetTimeIntervalID(now);//���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > maxTimeintervalNum)       
	{
		log_error("can't find timeinterval id");
        return 0;
	}
	gCalInfo.timeIntervalId = nTimeIntervalID;
    //�˺����е�ʱ��ֵ��λΪ����
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//Ԥ������һ�����Ĳ�ֵ
	do
	{	//ѭ���ҳ���ǰʱ����ʱ�α��в�ֵ��С��ʱ������Ӧ��actionID��Ϊ��ǰӦ�����е�actionID
		for(i= 0; i < maxTimeintervalIdNum; i++) 
		{
			if (gRunConfigPara != NULL && gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //��˵����ʱ�ο���û�б�ʹ�ã�ֱ��continue��
				continue;
			}
			
			hour = GET_PROTOCOL_PARAM(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour, gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][i].hour);
			minute = GET_PROTOCOL_PARAM(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute, gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][i].minute);
			nTempTime = HoursToMinutes(hour, minute);
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
			log_error("the nTimeIntervalID %d isn't configed", nTimeIntervalID);
			return 0;	//˵����ʱ�α�û������
		}
		if (minTimeGap == MAX_TIME_GAP) 
		{ //˵����ǰʱ�䴦��ʱ�α�����С��λ��
			nCurrentTime += MAX_TIME_GAP;	//�ѵ�ǰʱ������24СʱȻ���ٴ�ѭ��
		}
	} while (index == -1);
	if (ret != NULL)
	    *ret = minTimeGap;
	if (gRunConfigPara != NULL)
	{
		gCalInfo.actionId = gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;//����ʱ�α�ID���õ�������ID    
		if (gCalInfo.actionId == 0 || gCalInfo.actionId > NUM_ACTION)
		{
			log_error("can't find action id");
			return 0;
		}
		nSchemeID = gRunConfigPara->stAction[gCalInfo.actionId - 1].nSchemeID;
	}
	else if (gRunGbconfig != NULL)
	{
		switch (gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][index].controlMode)
		{
			case SYSTEM_MODE: 
				nSchemeID = gRunGbconfig->timeIntervalTable[nTimeIntervalID - 1][index].schemeId;
				break;
			case TURNOFF_LIGHTS_MODE: nSchemeID = TURNOFF_SCHEMEID; break;
			case YELLOWBLINK_MODE: nSchemeID = YELLOWBLINK_SCHEMEID; break;
			case ALLRED_MODE: nSchemeID = ALLRED_SCHEMEID; break;
			case INDUCTIVE_MODE: nSchemeID = INDUCTIVE_SCHEMEID; break;
			default: nSchemeID = 1;	//������������Ʒ�ʽδʵ�֣�Ĭ��ʹ�÷���1
		}
	}
	return nSchemeID;
}

static void SetInductiveControlTime(void)
{
    int i = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;

    for(i = 0; i < NUM_PHASE; i++)
    {
		phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i];
		if (phaseItem->nCircleID == 0)
			continue;
		phaseTimes[i].splitTime = phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime;
		phaseTimes[i].greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
		phaseTimes[i].greenBlinkTime = entry->nGreenLightTime;
		phaseTimes[i].yellowTime = phaseItem->nYellowTime;
		phaseTimes[i].allRedTime = phaseItem->nAllRedTime;
    }
	//��������
	gCalInfo.cycleTime = 0;
	for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{	//��Ӧ����Ĭ��ʹ�������1
			nPhaseId = gRunConfigPara->stPhaseTurn[0][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
				break;
			gCalInfo.cycleTime += phaseTimes[nPhaseId - 1].splitTime;
		}
		if (gCalInfo.cycleTime > 0)
			break;
	}
	//INFO("DEBUG inductive control: gCalInfo.cycleTime = %d", gCalInfo.cycleTime);
}

static void SendSpecialControlData(UInt8 schemeId, UInt8 leftTime)
{
    LineQueueData data;
    int i, channelNum = GET_PROTOCOL_PARAM(NUM_CHANNEL, MAX_CHANNEL_LIST_NUM);
    UInt8 phaseId = 0;
    LightStatus status = INVALID;
	UInt8 controllerType = 0;

	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: status = YELLOW_BLINK; break;	//��������
		case ALLRED_SCHEMEID: status = ALLRED; break;		//ȫ�����
		case TURNOFF_SCHEMEID: status = TURN_OFF; break;	//�صƿ���
		default: break;
	}
    memset(&data, 0, sizeof(data));
    for (i = 0; i < channelNum; i++)
    {
		phaseId = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[0][i].nControllerID, gRunGbconfig->channelTable[i].channelRelatedPhase);
        if (phaseId == 0 || phaseId > NUM_PHASE)
            continue;
        
    	data.allChannels[i] = status;
		controllerType = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[0][i].nControllerType, gRunGbconfig->channelTable[i].channelControlType);
    	switch (controllerType)
    	{
    	    case MOTOR: data.phaseInfos[phaseId - 1].phaseStatus = status; break;
    	    case PEDESTRIAN: data.phaseInfos[phaseId - 1].pedestrianPhaseStatus = status; break;
    	    case FOLLOW: data.phaseInfos[phaseId - 1].followPhaseStatus = status; break;
    	}
    }
    data.schemeId = schemeId;
	data.leftTime = leftTime;
    while (0 != lfq_write(gHandle, &data));
}

#define UNKNOWN_CONTROL		0
#define SPERICAL_CONTROL 	1
#define GENERAL_CONTROL		2
static UInt8 GetPhaseTurnAndSetPhaseTimeInfo(UInt8 schemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//��ǰʱ���뵱ǰʱ�ε���ʼʱ��Ĳ�ֵ����λΪ����

	localtime_r(&calTime, &now);
	memset(&gCalInfo, 0, sizeof(CalInfo));
    gCalInfo.schemeId = (schemeId > 0) ? schemeId : GetSchemeIdAndTimeGap(&now, &timeGapMinutes);
    if (gCalInfo.schemeId == YELLOWBLINK_SCHEMEID || gCalInfo.schemeId == ALLRED_SCHEMEID || gCalInfo.schemeId == TURNOFF_SCHEMEID)
	{
		//SendSpecialControlData(YELLOWBLINK_SCHEMEID, 0);
		log_debug("special control, scheme: %d", gCalInfo.schemeId);
        return SPERICAL_CONTROL;	//������Ʒ�����������+1
	}
	else if (gCalInfo.schemeId == INDUCTIVE_SCHEMEID)	
    {	
    	gCalInfo.phaseTurnId = 1;	//��Ӧ����Ĭ��ʹ�������1
		gCalInfo.actionId = INDUCTIVE_ACTIONID;
		gCalInfo.channelTableId=1;
		gCalInfo.phaseTableId =1;
		
    	//����Ǹ�Ӧ����Ӧ��ʹ����С��+�Ƶ�ʱ��+ȫ��ʱ����Ϊ���ű�ʱ��
        SetInductiveControlTime();

		return GENERAL_CONTROL;
    }

    if(gCalInfo.schemeId == 0 || gCalInfo.schemeId > NUM_SCHEME)
	{
		log_error("can't find scheme id, action: %d", gCalInfo.actionId);
        return UNKNOWN_CONTROL;
	}
    gCalInfo.splitId = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nGreenSignalRatioID;//���ݷ�����ID���õ����űȱ�ID
    if(gCalInfo.splitId == 0 || gCalInfo.splitId > NUM_GREEN_SIGNAL_RATION)
	{
		log_error("can't find split id, scheme: %d", gCalInfo.schemeId);
        return UNKNOWN_CONTROL;
	}

	gCalInfo.cycleTime = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nCycleTime;
    gCalInfo.phaseTurnId = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nPhaseTurnID;//���ݷ�����ID���õ������ID
    if(gCalInfo.phaseTurnId == 0 || gCalInfo.phaseTurnId > NUM_PHASE_TURN)
	{
		log_error("can't find phase turn id, scheme: %d", gCalInfo.schemeId);
        return UNKNOWN_CONTROL;
	}
	if (gCalInfo.actionId == 0)	//����1,4,7��Ӧ����1��2��3
		gCalInfo.actionId = (gCalInfo.schemeId + 2) / 3;
	gCalInfo.phaseTableId = gRunConfigPara->stAction[gCalInfo.actionId - 1].nPhaseTableID;
	gCalInfo.channelTableId = gRunConfigPara->stAction[gCalInfo.actionId - 1].nChannelTableID;
	if (gCalInfo.phaseTableId == 0 || gCalInfo.phaseTableId > MAX_PHASE_TABLE_COUNT)
	{	//�����λ��Ų��������֮ǰ�ɵ�
		memcpy(gRunConfigPara->stPhase[0], gRunConfigPara->stOldPhase, sizeof(gRunConfigPara->stOldPhase));
		memcpy(gRunConfigPara->AscSignalTransTable[0], gRunConfigPara->OldAscSignalTransTable, sizeof(gRunConfigPara->OldAscSignalTransTable));
		memcpy(gRunConfigPara->stFollowPhase[0], gRunConfigPara->stOldFollowPhase, sizeof(gRunConfigPara->stOldFollowPhase));
		gCalInfo.phaseTableId = 1;
	}
	if (gCalInfo.channelTableId == 0 || gCalInfo.channelTableId > MAX_CHANNEL_TABLE_COUNT)
	{
		memcpy(gRunConfigPara->stChannel[0], gRunConfigPara->stOldChannel, sizeof(gRunConfigPara->stOldChannel));
		gCalInfo.channelTableId = 1;
	}
	//���ݵ�ǰ�����űȱ�ID������λ���и���λ�����ű�ʱ��θ�ֵ����ͬʱ�ҳ�Э����λ
    SetPhaseTime();
	gCalInfo.timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	return GENERAL_CONTROL;
}

void SetCurrentChannelStatus(PhaseInfo *phaseInfos, UInt8 *allChannel)
{    
    int i = 0, channelNum = GET_PROTOCOL_PARAM(NUM_CHANNEL, MAX_CHANNEL_LIST_NUM);
	LightStatus status;
	UInt8 phaseId, leftTime;
	UInt8 pedestrianPhase[NUM_PHASE][2];	//������λ��״̬�͵���ʱ��[0]:״̬��[1]:����ʱ
	UInt8 redFlashSec = gCustomInfo.redFlashSec;	//�����˸����
	UInt8 isYellowFlash = gCustomInfo.isYellowFlash;	//�Ƶ�ʱ�Ƿ���˸

	memset(pedestrianPhase, 0, sizeof(pedestrianPhase));
    for(i = 0; i < channelNum; i++)
    {
		phaseId = GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[gCalInfo.channelTableId - 1][i].nControllerID, gRunGbconfig->channelTable[i].channelRelatedPhase);
		if (phaseId == 0)
		{
			allChannel[i] = INVALID;
			continue;
		}	
		
		switch (GET_PROTOCOL_PARAM(gRunConfigPara->stChannel[gCalInfo.channelTableId - 1][i].nControllerType, gRunGbconfig->channelTable[i].channelControlType)) 
		{
			case MOTOR: 
				status = phaseInfos[phaseId - 1].phaseStatus; 
				leftTime = phaseInfos[phaseId - 1].phaseLeftTime; 
				break;
			case PEDESTRIAN: 
				status = phaseInfos[phaseId - 1].pedestrianPhaseStatus; 
				pedestrianPhase[phaseId - 1][0] = phaseInfos[phaseId - 1].pedestrianPhaseStatus;
				pedestrianPhase[phaseId - 1][1] = phaseInfos[phaseId - 1].pedestrianPhaseLeftTime;
				leftTime = phaseInfos[phaseId - 1].pedestrianPhaseLeftTime;
				break;
			case FOLLOW: 
				status = phaseInfos[phaseId - 1].followPhaseStatus; 
				leftTime = phaseInfos[phaseId - 1].followPhaseLeftTime;  
				break;
			default: status = INVALID; break;
		}
		if (status == RED && redFlashSec >= leftTime)
			status = RED_BLINK;
		else if (status == YELLOW && isYellowFlash == 1)
			status = YELLOW_BLINK;
		allChannel[i] = (UInt8)status;
    }
	//����������λ״̬�͵���ʱ����Ϊ֮ǰ�ڼ����������λʱ�Ѷ�Ӧ��������λҲ�����ˣ��п��ܸ���û���õ�
	for (i = 0; i < NUM_PHASE; i++)
	{
		phaseInfos[i].pedestrianPhaseStatus = pedestrianPhase[i][0];
		phaseInfos[i].pedestrianPhaseLeftTime = pedestrianPhase[i][1];
	}
}

static inline void SetpedestrianPhaseStatusAndTime(PhaseTimeInfo *times, UInt8 runTime, UInt8 startTime, PhaseInfo *info)
{
	LightStatus status = RED;
	if (times->pedestrianPassTime) 
	{
		times->pedestrianPassTime--;
		status = GREEN;
	}
	else
	{
		if (times->pedestrianClearTime) 
		{
			times->pedestrianClearTime--;
			status = GREEN_BLINK;
		}
	}
	info->pedestrianPhaseStatus = status;
	if (status == GREEN || status == GREEN_BLINK)	//�����̵Ƶ���ʱ
		info->pedestrianPhaseLeftTime = times->pedestrianPassTime + times->pedestrianClearTime + 1;
	else	//���˺�Ƶ���ʱ
		info->pedestrianPhaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
}

static void SetPhaseInfo(UInt8 phaseId, UInt8 runTime, UInt8 startTime, PhaseInfo *phaseInfos)
{
	int i = 0;
	LightStatus status = RED;	//����Ĭ�Ϸ���RED״̬
	PhaseTimeInfo *times = &gCalInfo.phaseTimes[phaseId - 1];
	PhaseItem *phaseItem = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][phaseId - 1];
	PhaseInfo *info = &phaseInfos[phaseId - 1];
	int endTime = startTime + times->splitTime;
	
	if (startTime == endTime)
	{	//˵������λ�ѱ����Ե�
		info->phaseStatus = TURN_OFF;
		info->pedestrianPhaseStatus = TURN_OFF;
		return;
	}
	info->pedestrianPhaseStatus = RED;	//������λĬ��Ϊ��ɫ
	if (runTime >= startTime && runTime < endTime)
	{	//˵����ʱ�������д���λ�����ű�
		status = GetPhaseStatusByTime(times);
		SetpedestrianPhaseStatusAndTime(times, runTime, startTime, info);
		//������1s����Ϊ֮ǰ��GetPhaseStatusByTime�������Ѿ��ݼ���1s
		if (status == GREEN || status == GREEN_BLINK)	//�̵Ƶ���ʱ
		{
			info->phaseLeftTime = times->greenTime + times->greenBlinkTime + 1;
		}
		else if (status == YELLOW)	//�ƵƵ���ʱ
		{
			info->phaseLeftTime = times->yellowTime + 1;
		}	
		else	//��Ƶ���ʱ
		{
			info->phaseLeftTime = gCalInfo.cycleTime - times->splitTime + times->allRedTime + 1;
		}
		info->phaseSplitLeftTime = times->greenTime + times->greenBlinkTime + times->yellowTime + times->allRedTime + 1;
	}
	else if (runTime < startTime)
	{
		info->phaseLeftTime = startTime - runTime;
		info->pedestrianPhaseLeftTime = startTime - runTime;
	}
	else  //(runTime >= endTime)
	{
		info->phaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
		info->pedestrianPhaseLeftTime = gCalInfo.cycleTime - runTime + startTime;
	}
	
	if (GET_BIT(gCalInfo.isIgnorePhase, phaseId - 1) == TRUE)
	{	//�Ǻ�����λ
		info->phaseStatus = TURN_OFF;
		info->pedestrianPhaseStatus = TURN_OFF;
	}
	else
		info->phaseStatus = status;
	info->splitTime = times->splitTime;
	if (gCalInfo.schemeId != INDUCTIVE_SCHEMEID)
		return;	//������Ǹ�Ӧ���Ʋ���Ҫ����Ĳ���
	for (i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{	//��ѯ������������Ϣ������λ��Ӧ�ĳ�������
		if (phaseId == gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase)
		{
			info->vehicleDetectorId = gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorNumber;
			break;
		}
	}
	info->unitExtendGreen = phaseItem->nUnitExtendGreen;
	info->maxExtendGreen = phaseItem->nMaxGreen_1 - (times->splitTime - phaseItem->nYellowTime - phaseItem->nAllRedTime);
}

/*****************************************************************************
 �� �� ��  : RunCycle
 ��������  : ͨ���������������ͨ����״̬�Լ���λ��ͨ���Ķ�Ӧ��ϵ������һ
             ������
 �������  : unsigned short nPhaseTurn           ��ǰҪ���е�����
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void RunCycle(UInt8 nPhaseTurn)
{
	int t = 0, i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	LineQueueData data;
	UInt8 startTime = 0;	//��λִ��ʱ�Ѿ����е�����ʱ��
	UInt8 stageNum = 1;	//�׶κ�
	UInt8 stepRunTime = 0;	//�׶�����ʱ��
	UInt8 checkTime = gCustomInfo.checkTime;	//��Ӧ���ʱ��

	for (t = 0; t < gCalInfo.cycleTime; t++) 
	{	//һ��ѭ����ȡ1���Ӹ�ͨ����״̬�Լ�������Ϣ���͸���λ����ģ��
		memset(&data, 0, sizeof(data));
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	//ÿѭ��һ�δ�������ĸ�����������1s������ͨ����״̬���Լ�������������λ����Ϣ
			startTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gRunConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[i];
				if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
					break;
				//������λ�������Ϣ
				SetPhaseInfo(nPhaseId, (UInt8)t, startTime, data.phaseInfos);
				startTime += gCalInfo.phaseTimes[nPhaseId - 1].splitTime;
				if (t == startTime && startTime > stepRunTime)
				{
					stageNum++;
					stepRunTime = startTime;
				}
			}
		}
		SetFollowPhaseInfo(data.phaseInfos, stageNum);
		SetCurrentChannelStatus(data.phaseInfos, data.allChannels);
		data.cycleTime = gCalInfo.cycleTime;
		//data.runTime = t + 1;
		data.leftTime = data.cycleTime - t;
		data.schemeId = gCalInfo.schemeId;
		data.stageNum = stageNum;
		data.maxStageNum = gCalInfo.maxStageNum;
		data.checkTime = checkTime;
		data.phaseTableId = gCalInfo.phaseTableId;
		data.channelTableId = gCalInfo.channelTableId;
		while (0 != lfq_write(gHandle, &data));	//��֤һ���ܰ�����д�뵽������
	}
	INFO("max stage num is %d", stageNum);
}

static inline void YellowBlinkAllRedInit()
{
	UInt8 bootYellowFlashTime = GET_PROTOCOL_PARAM(gRunConfigPara->stUnitPara.nBootYellowLightTime, gRunGbconfig->bootBlinkTime);
	UInt8 bootAllRedTime = GET_PROTOCOL_PARAM(gRunConfigPara->stUnitPara.nBootAllRedTime, gRunGbconfig->bootAllRedTime);
	UInt8 firstCycle = bootYellowFlashTime + bootAllRedTime;
	struct msgbuf msg;
	int i;
	
	//�ѻ�����ȫ����Ϊ��һ�����ڣ������ڵ�ʣ��ʱ��д����Ϣ�б�����λ����ģ��SystemInit����ʹ��
	if (firstCycle == 0)
	{	//���û�����û�����ȫ�������ʱ�������ȷ���1s�Ĺص����ݱ�����λ����ģ��SystemInit����ʹ��
		SendSpecialControlData(TURNOFF_SCHEMEID, 1);
	}
	else
	{
		//��ʼ��Ϊ��������
		for (i = 0; i < bootYellowFlashTime; i++) 	
			SendSpecialControlData(YELLOWBLINK_SCHEMEID, firstCycle--);
		//��ʼ��Ϊȫ�����
		for (i = 0; i < bootAllRedTime; i++) 
			SendSpecialControlData(ALLRED_SCHEMEID, firstCycle--);
	}
	//��֪��λ����ģ�鿪ʼ��ȡ����
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_BEGIN_READ_DATA;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

static Boolean CalculateByHikconfig(struct msgbuf *msg)
{
	UInt8 ret = 0;
	int i;
	
	//���ݵ�ǰʱ���ȡ���򣬲���ʼ������λ�����ű�ʱ���
	ret = GetPhaseTurnAndSetPhaseTimeInfo(msg->msgSchemeId, msg->msgCalTime);
	if (ret == UNKNOWN_CONTROL)
		return FALSE;
	if (ret == GENERAL_CONTROL) 
	{
		CalStageInfo();	//����׶���ص���Ϣ
		if (gCalInfo.maxStageNum < 2)
		{	//����Ҫ2���׶�
			log_error("the maxStageNum[%d] is too small, schemeId = %d, splitId = %d, phaseTurnId = %d", gCalInfo.schemeId, gCalInfo.maxStageNum, gCalInfo.splitId, gCalInfo.phaseTurnId);
			return FALSE;
		}
		else
		{
			if (gCalInfo.schemeId != INDUCTIVE_SCHEMEID)
			{
				if (gCalInfo.isIgnorePhase > 0)	//��ʾ�����ú�����λ
					IgnorePhaseDeal();	//�Ժ�����λ���д���
				if (msg->msgSchemeId == 0 && gCalInfo.coordinatePhaseId > 0)
					TransitionDeal();	//��Э����λ���й���
			}
			for (i = 0; i < NUM_PHASE; i++)
			{
				SetPedestrianTime(GET_BIT(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].wPhaseOptions, 13), 
									&gCalInfo.phaseTimes[i], 
									gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nPedestrianPassTime, 
									gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nPedestrianClearTime);	//����������ص�ʱ��
			}
			RunCycle(gCalInfo.phaseTurnId);	//ÿ����ѯһ������������	
		}
	}
	return TRUE; 
}

void *CalculateModule(void *arg)
{	
	struct msgbuf msg;
	
	YellowBlinkAllRedInit();
    while (1)
    {
		memset(&msg, 0, sizeof(msg));
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_START_CALCULATE_NEXT_CYCLE, 0) == -1)
		{
			usleep(10000);
			continue;
		}
		//Ϊ��ֹ��λ����ģ���η��ͼ�����һ������Ϣ�������һ�ж�
		if (lfq_element_count(gHandle) > AHEAD_OF_TIME)  
		    continue;
		pthread_rwlock_rdlock(&gConfigLock);
		if ((gRunConfigPara != NULL && CalculateByHikconfig(&msg))
			|| (gRunGbconfig != NULL && CalculateByGbconfig(&msg)))
			log_debug("msgschemeid = %d, gSchemeId = %d, cycleTime = %d", msg.msgSchemeId, gCalInfo.schemeId, gCalInfo.cycleTime);
		else
		{
			ItsCtl(TOOL_CONTROL, YELLOWBLINK_SCHEMEID, 0);
			ItsWriteFaultLog(CALCULATE_FAIL_CAUSE_FLASH, 0);
			if (gRunConfigPara == NULL && gRunGbconfig == NULL)
				log_error("There isn't a piece of config exist!");
		}
		pthread_rwlock_unlock(&gConfigLock);
    }
}

void ItsSetRedFlashSec(UInt8 sec)
{
	gCustomInfo.redFlashSec = sec;
}

void ItsYellowLampFlash(Boolean val)
{
	gCustomInfo.isYellowFlash = val;
}

void ItsGetRealtimePattern(MsgRealtimePattern *p)
{
	int i;
	pthread_rwlock_rdlock(&gConfigLock);
	p->nPatternId = gCalInfo.schemeId;
	p->nSplitId = gCalInfo.splitId;
	p->nPhaseTurnId = gCalInfo.phaseTurnId;
	if (gCalInfo.schemeId > 0 && gCalInfo.schemeId <= NUM_SCHEME)
		p->nOffset = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nOffset;
	if (gCalInfo.phaseTurnId > 0 && gCalInfo.phaseTurnId <= NUM_PHASE_TURN)
		memcpy(p->sPhaseTurn, gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1], sizeof(p->sPhaseTurn));
	p->nCoordinatePhase = gCalInfo.isCoordinatePhase;
	if (gCalInfo.phaseTableId > 0 && gCalInfo.phaseTableId <= MAX_PHASE_TABLE_COUNT)
	{
		for (i = 0; i < NUM_PHASE; i++)
		{
			p->phaseTime[i].greenBlink = gRunConfigPara->AscSignalTransTable[gCalInfo.phaseTableId - 1][i].nGreenLightTime;
			p->phaseTime[i].yellow = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nYellowTime;
			p->phaseTime[i].allred = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][i].nAllRedTime;
		}
		memcpy(p->stFollowPhase, gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1], sizeof(p->stFollowPhase));
	}
	pthread_rwlock_unlock(&gConfigLock);
}
