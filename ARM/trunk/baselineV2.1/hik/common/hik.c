/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdlib.h>
#include <pthread.h>
#include "LogSystem.h"
#include "its.h"
#include "calculate.h"
#include "HikConfig.h"
#include "common.h"
#include "configureManagement.h"
#include "platform.h"
#include "countDown.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //��ʱ��ת���ɷ�
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //һ���е����ʱ���ֵ����24Сʱ

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;
extern STRUCT_BINFILE_DESC gStructBinfileDesc;
extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara; 
extern Boolean gAddTimeToFirstPhase;
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void SendRunInfoTOBoard(LineQueueData *data, SignalControllerPara *para);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static SignalControllerPara *gRunConfigPara = NULL;//�źŻ����е����ò���
static pthread_rwlock_t gCountDownLock = PTHREAD_RWLOCK_INITIALIZER;
static LineQueueData gCurRunData;		//��ǰ��1s����������
static PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;   //����ʱ�ӿ���Ϣ
static MsgRealtimePattern gCurrentCycleRealtimePattern;		//��ǰ���ڷ�����Ϣ
static MsgRealtimePattern gNextCycleRealtimePattern;		//��һ���ڷ�����Ϣ
static UInt32 gConflictChannel[2][NUM_CHANNEL] = {{0}, {0}};//[0]�����Ϊ��ǰ���ڵĳ�ͻͨ����[1]������Ǽ�����һ����ʱһ��������ģ���ÿ�����ڵ�1s���[1]��ֵ��[0]
Int32 gNextCycleExtendTime[MAX_PHASE_NUM] = {0}; //�����Ӧ���ӳ�ʱ��
extern UINT32 gCurrentAction;
extern UINT32 gCurrentScheme;
extern UINT8  gCurControlType;

#include "ignorephase.h"	//������λ�йصĺ�����ֻ�ܷ������λ������

//static UInt8 GetPedDetectorBitsByPhase(UInt8 nPhaseId, CalInfo *calInfo);
//static UINT32 GetBusBitsByPhase(UINT8 phaseId, CalInfo* calInfo, UINT32* busDetectorBits);
//static UINT64 GetDetectorBitsByPhase(UInt8 nPhaseId,CalInfo * calInfo);

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
	initFunc(gRunConfigPara->stUnitPara.nBootYellowLightTime, gRunConfigPara->stUnitPara.nBootAllRedTime);
}

static void CalConflictChannel(const PhaseItem *phaseTable, const ChannelItem *channelTable, const FollowPhaseItem *followPhaseTable, UInt32 *conflictChannels)
{
	int ph, ch, n;
	UInt8 phaseId = 0, motherPhaseId = 0, concurrencyPhaseId = 0;
	UInt32 phaseIncludeChannelBits[NUM_PHASE] = {0};	//��λ������ͨ��bitλ
	UInt32 concurrencyChannels[NUM_CHANNEL] = {0};		//������ͨ��bitλ
	UInt32 unuseChannelBit = 0;	//δʹ�õ�ͨ��bitλ
	
	if (phaseTable == NULL || channelTable == NULL 
		|| followPhaseTable == NULL || conflictChannels == NULL)
		return;
	//�����ҵ�����λ��������ͨ��bitλ
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		phaseId = channelTable[ch].nControllerID;
		if (phaseId == 0 || phaseId > NUM_PHASE)
		{	//Ĭ��δ����ͨ����������ͨ������
			concurrencyChannels[ch] = 0xffffffff;
			SET_BIT(unuseChannelBit, ch);
			continue;
		}
		switch (channelTable[ch].nControllerType) 
		{
			case MOTOR:
			case PEDESTRIAN: 
				SET_BIT(phaseIncludeChannelBits[phaseId - 1], ch);
				break;
			case FOLLOW:	//����������
			case OTHER:		//���˸���
				for (n = 0; n < NUM_PHASE; n++)
				{
					motherPhaseId = followPhaseTable[phaseId - 1].nArrayMotherPhase[n];
					if (motherPhaseId == 0 || motherPhaseId > NUM_PHASE)
						continue;
					SET_BIT(phaseIncludeChannelBits[motherPhaseId - 1], ch);
				}
				break;
			default: concurrencyChannels[ch] = 0xffffffff; SET_BIT(unuseChannelBit, ch); break;//Ĭ��δ����ͨ����������ͨ������
		}
	}
	//���ͨ��������λ�ҵ�����λ������ͨ����������ͨ��bit
	for (ph = 0; ph < NUM_PHASE; ph++)
	{
		if (GET_BIT(phaseTable[ph].wPhaseOptions, 0) == FALSE)
		{	//��λδʹ�ܣ�Ĭ�ϴ���λ��������ͨ��������ͨ���ɲ���
			for (ch = 0; ch < NUM_CHANNEL; ch++)
			{
				if (GET_BIT(phaseIncludeChannelBits[ph], ch))
				{
					concurrencyChannels[ch] = 0xffffffff;
					SET_BIT(unuseChannelBit, ch);
				}
			}
		}
		else
		{
			for (ch = 0; ch < NUM_CHANNEL; ch++)
			{
				if (GET_BIT(phaseIncludeChannelBits[ph], ch) == FALSE)
					continue;
				concurrencyChannels[ch] |= phaseIncludeChannelBits[ph];
				for (n = 0; n < NUM_PHASE; n++)
				{
					concurrencyPhaseId = phaseTable[ph].byPhaseConcurrency[n];
					if (concurrencyPhaseId == 0 || concurrencyPhaseId > NUM_PHASE)
						continue;
					concurrencyChannels[ch] |= phaseIncludeChannelBits[concurrencyPhaseId - 1];
				}
			}
		}
	}
	//���Բ���ͨ��ȡ����Ϊ��ͻͨ��
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		concurrencyChannels[ch] |= unuseChannelBit;	//δʹ��ͨ�����������κ�ͨ������
		conflictChannels[ch] = ~concurrencyChannels[ch];	//����ͨ��ȡ����Ϊ��ͻͨ��
		CLR_BIT(conflictChannels[ch], ch);	//�������ĳ�ͻbitλ
	}
}

static void SetChannelBits(CalInfo *calInfo)
{
	UInt8 phaseId = 0, motherPhaseId = 0;
	int ch, n;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	for (ch = 0; ch < NUM_CHANNEL; ch++)
	{
		phaseId = gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerID;
		if (phaseId == 0 || phaseId > NUM_PHASE)
			continue;
		switch (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType) 
		{
			case MOTOR:
				SET_BIT(calInfo->phaseTimes[phaseId - 1].motorChannelBits, ch);
				break;
			case PEDESTRIAN: 
				SET_BIT(calInfo->phaseTimes[phaseId - 1].pedChannelBits, ch);
				break;
			case FOLLOW:
			case OTHER:
				if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType == FOLLOW)
					SET_BIT(calInfo->followPhaseInfos[phaseId - 1].motorChannelBits, ch);
				else if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][ch].nControllerType == OTHER)	
					SET_BIT(calInfo->followPhaseInfos[phaseId - 1].pedChannelBits, ch);//��other��Ϊ���˸���ͨ��
				else
					break;
				for (n = 0; n < NUM_PHASE; n++)
				{
					motherPhaseId = gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1][phaseId - 1].nArrayMotherPhase[n];
					if (motherPhaseId == 0 || motherPhaseId > NUM_PHASE)
						continue;
					SET_BIT(calInfo->followPhaseInfos[phaseId - 1].phaseBits, motherPhaseId - 1);
				}
#if 0
				INFO("followPhaseId: %d, motorChannelBits: %#x, pedChannelBits: %#x, phaseBits: %#x", 
					phaseId, 
					calInfo->followPhaseInfos[phaseId - 1].motorChannelBits,
					calInfo->followPhaseInfos[phaseId - 1].pedChannelBits,
					calInfo->followPhaseInfos[phaseId - 1].phaseBits);
#endif
				break;
			default: break;
		}
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
    int i = 0;

	//INFO("DEBUG: enter %s function!!!!!", __func__);
	for(i = 0 ; i < NUM_SCHEDULE ; i++) { //�����ǰʱ���ĳһ����ȱ�������򷵻ظõ��ȱ��ʱ�κ�
        if (BIT(gRunConfigPara->stPlanSchedule[i].month, now->tm_mon + 1) == 1)
		{	//tm_mon��Χ��[0,11]
			if ((BIT(gRunConfigPara->stPlanSchedule[i].day, now->tm_mday) == 1)
				|| (BIT(gRunConfigPara->stPlanSchedule[i].week, now->tm_wday + 1) == 1))//tm_wday == 0����������
			    return gRunConfigPara->stPlanSchedule[i].nTimeIntervalID;
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
static UInt8 GetSchemeIdAndTimeGap(CalInfo *calInfo, struct tm *now, int *ret)
{
    int i = 0, index = -1;
	int nCurrentTime, nTempTime = -1;
	int nTimeGap, minTimeGap;
	UInt8 nTimeIntervalID = 0, nActionID = 0;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	nTimeIntervalID = GetTimeIntervalID(now);//���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
    if (nTimeIntervalID == 0 || nTimeIntervalID > NUM_TIME_INTERVAL)       
	{
		//log_error("can't find timeinterval id");
        return 0;
	}
    //�˺����е�ʱ��ֵ��λΪ����
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	minTimeGap = MAX_TIME_GAP;	//Ԥ������һ�����Ĳ�ֵ
	do
	{	//ѭ���ҳ���ǰʱ����ʱ�α��в�ֵ��С��ʱ������Ӧ��actionID��Ϊ��ǰӦ�����е�actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if (gRunConfigPara != NULL && gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //��˵����ʱ�ο���û�б�ʹ�ã�ֱ��continue��
				continue;
			}
			
			nTempTime = HoursToMinutes(gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour, gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute);
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
	    *ret = minTimeGap;
	nActionID = gRunConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;//����ʱ�α�ID���õ�������ID    
	if (nActionID == 0 || nActionID > NUM_ACTION)
	{
		//log_error("can't find action id");
		return 0;
	}
	if (calInfo != NULL)
	{
		calInfo->timeIntervalId = nTimeIntervalID;
		calInfo->actionId = nActionID;
	}
	//INFO("DEBUG: leave %s function $$$$$$$$", __func__);
	return gRunConfigPara->stAction[nActionID - 1].nSchemeID;
}

//�õ���λ��Ӧ�ĳ�����bits���Ժ�Ĭ������£������ļ��г�������Ӧ��������λ��ʵ��ͨ����
static UInt64 GetDetectorBitsByPhase(UInt8 nPhaseId, CalInfo *calInfo)
{
	int i;
    UInt8 nChannelId = 0, nControllerType;
	UInt64 vehicleDetectorBits = 0;
	
    if(nPhaseId == 0 || nPhaseId > MAX_PHASE_NUM || calInfo == NULL || gRunConfigPara == NULL)
        return 0;
	for (i = 0; i < MAX_VEHICLEDETECTOR_COUNT; i++)
	{	//��ѯ������������Ϣ������λ��Ӧ�ĳ�������
		nChannelId = gRunConfigPara->AscVehicleDetectorTable[i].byVehicleDetectorCallPhase;
		if (nChannelId == 0 || nChannelId > NUM_CHANNEL)
			continue;
		nControllerType = gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerType;
	    if ((nControllerType == MOTOR || nControllerType == PEDESTRIAN)
			&& gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerID == nPhaseId)
			SET_BIT(vehicleDetectorBits, i);
	}
    return vehicleDetectorBits;
}

static UInt8 GetPedDetectorBitsByPhase(UInt8 nPhaseId, CalInfo *calInfo)
{
	int i;
    UInt8 nChannelId = 0, nControllerType;
	UInt8 PedestrianDetectorBits = 0;
	
    if(nPhaseId == 0 || nPhaseId > MAX_PHASE_NUM || calInfo == NULL || gRunConfigPara == NULL)
        return 0;
	for (i = 0; i < MAX_PEDESTRIANDETECTOR_COUNT; i++)
	{	//��ѯ���˼����������Ϣ������λ��Ӧ�����˼������
		nChannelId = gRunConfigPara->AscPedestrianDetectorTable[i].byPedestrianDetectorCallPhase;
		if (nChannelId == 0 || nChannelId > NUM_CHANNEL)
			continue;
		nControllerType = gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerType;
	    if ((nControllerType == PEDESTRIAN || nControllerType == MOTOR)
			&& gRunConfigPara->stChannel[calInfo->channelTableId - 1][nChannelId - 1].nControllerID == nPhaseId)
			SET_BIT(PedestrianDetectorBits, i);
	}
    return PedestrianDetectorBits;
}

static UINT8 GetBusBitsByPhase(UINT8 phaseId, CalInfo* calInfo, UINT32* busDetectorBits)
{
	int i = 0;
	UINT8 channelId = 0;
	STRU_N_BusDetector* busDetector = gStructBinfileCustom.sBusDetector;
	
	if (phaseId <= 0 || phaseId > MAX_PHASE_NUM || calInfo == NULL || busDetector == NULL)
		return 0;
	for (i = 0; i < MAX_BUSDETECTOR_NUM; i++)
	{
		channelId = busDetector[i].byBusDetectorCallChannel;
		if (channelId <= 0 || channelId > NUM_CHANNEL)
			continue;
		if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][channelId - 1].nControllerID == phaseId)
			SET_BIT(busDetectorBits[i / 32], i % 32);
	}
	return 1;
}
static UINT32 GetBusPrioPassTime(UINT8 phaseId, CalInfo* calInfo)
{
	int i = 0;
	UINT8 channelId = 0;
	UINT32 prioPassTime = 0;
	STRU_N_BusDetector* busDetector = gStructBinfileCustom.sBusDetector;

	if (phaseId <= 0 || phaseId > MAX_PHASE_NUM || calInfo == NULL || busDetector == NULL)
		return 0;
	for (i = 0; i < MAX_BUSDETECTOR_NUM; i++)
	{
		channelId = busDetector[i].byBusDetectorCallChannel;
		if (channelId <= 0 || channelId > NUM_CHANNEL)
			continue;
		if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][channelId - 1].nControllerID == phaseId)
			if (prioPassTime < busDetector[i].passTime)
				prioPassTime = busDetector[i].passTime;
	}
	if (prioPassTime > 0)
		return prioPassTime;
	else
		return 10;//default 10second
}

static UINT8 GetSpecialCarBitsByPhase(UINT8 phaseId, CalInfo* calInfo, UINT32* scarDetectorBits)
{
	int i = 0;
	UINT8 channelId = 0;
	
	STRU_N_SpecialCarDetector* scarDetector = gStructBinfileCustom.sSpecialCarDetector;
	
	if (phaseId <= 0 || phaseId > MAX_PHASE_NUM || calInfo == NULL || scarDetector == NULL)
		return 0;
	//log_debug("GetSpecialCarBitsByPhase start shinezhou");
	for (i = 0; i < MAX_SCARDETECTOR_NUM; i++)
	{
		channelId = scarDetector[i].bySCarDetectorCallChannel;
		if (channelId <= 0 || channelId > NUM_CHANNEL)
			continue;
		if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][channelId - 1].nControllerID == phaseId)
			SET_BIT(scarDetectorBits[i / 32], i % 32);
	}
	//log_debug("GetSpecialCarBitsByPhase end shinezhou");
	return 1;
}

static UINT32 GetSpecialCarPassTime(UINT8 phaseId, CalInfo* calInfo)
{
	int i = 0;
	UINT8 channelId = 0;
	UINT32 scarPassTime = 0;
	STRU_N_SpecialCarDetector* specialCarDetector = gStructBinfileCustom.sSpecialCarDetector;
	
	if (phaseId <= 0 || phaseId > MAX_PHASE_NUM || calInfo == NULL || specialCarDetector == NULL)
		return 0;
	//log_debug("GetSpecialCarPassTime start shinezhou");
	for (i = 0; i < MAX_SCARDETECTOR_NUM; i++)
	{
		channelId = specialCarDetector[i].bySCarDetectorCallChannel;
		if (channelId <= 0 || channelId > NUM_CHANNEL)
			continue;
		if (gRunConfigPara->stChannel[calInfo->channelTableId - 1][channelId - 1].nControllerID == phaseId)
			if (scarPassTime < specialCarDetector[i].passTime)
				scarPassTime = specialCarDetector[i].passTime;
	}
	//log_debug("GetSpecialCarPassTime end shinezhou");
	return((scarPassTime > 0)? scarPassTime : 10);
}

void ItsAddSpecialCarChanLock(SpecialCarControl* scarControl)
{
	memcpy(gStructBinfileCustom.sChannelLockedParams.ucChannelStatus, scarControl->SpecialCarChanLockStatus, NUM_CHANNEL);
	gStructBinfileCustom.cChannelLockFlag = 1;
}
void ItsDelSpecialCarChanLock()
{
	memset(gStructBinfileCustom.sChannelLockedParams.ucChannelStatus, INVALID, NUM_CHANNEL);
	gStructBinfileCustom.cChannelLockFlag = 0;
}

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
static void SetPhaseTime(CalInfo *calInfo)
{  
    int i = 0, j = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;

	//INFO("DEBUG: enter %s function!!!!!", __func__);
    for(i = 0 ; i < NUM_PHASE; i++)
    {
		splitItem = &gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1][i];
		phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][i];
		if ((phaseItem->nCircleID == 0) || (splitItem->nGreenSignalRationTime == 0))
			continue;
		if (splitItem->nIsCoordinate)
		{	//˵��������Э����λ����ʱ���û�������������Ӧ��ִ��Э������
			if (calInfo->coordinatePhaseId == 0)	//���ö��Э����λʱ���������ҵ���Ϊ׼
				calInfo->coordinatePhaseId = splitItem->nPhaseID;
			SET_BIT(calInfo->isCoordinatePhase, i);
		}
		if (splitItem->nType == IGNORE_PHASE)
		{
			SET_BIT(calInfo->isIgnorePhase, i);
		}
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = splitItem->nGreenSignalRationTime;
		passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
		passTimeInfo->yellowTime = phaseItem->nYellowTime;
		passTimeInfo->allRedTime = phaseItem->nAllRedTime;
		
		phaseTimes[i].pedestrianPassTime = phaseItem->nPedestrianPassTime;
		phaseTimes[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
		phaseTimes[i].laneLevel = GET_BIT(phaseItem->wPhaseOptions, 9);
		phaseTimes[i].pedestrianDetectorBits = GetPedDetectorBitsByPhase(i + 1, calInfo);
		GetBusBitsByPhase(i + 1, calInfo, phaseTimes[i].busDetectorBits);
		phaseTimes[i].busPrioPassTime = GetBusPrioPassTime(i + 1, calInfo);
		GetSpecialCarBitsByPhase(i + 1, calInfo, phaseTimes[i].scarDetectorBits);
		phaseTimes[i].scarPassTime = GetSpecialCarPassTime(i + 1, calInfo);
    }
}

UINT8 ItsResetNextCycleExtendTime()
{
	memset(gNextCycleExtendTime, 0, MAX_PHASE_NUM);
	return 0;
}

//��ȡ�����ڸ�Ӧͳ�Ƶ���ʱʱ��
//��������Ӧ���ƣ�������λʱ����Ϣ
static void SetPhaseTimeForSingleSpot(CalInfo *calInfo)
{
	int i = 0, j = 0;
	PGreenSignalRationItem splitItem = NULL;
    GreenSignalRationItem splitTable[NUM_PHASE] = {{0}};
	PPhaseItem phaseItem = NULL;
	PPhaseTurnItem phaseTurn = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	UINT8 phaseId = 0, conPhase = 0;
	INT32 timeGap = 0, maxGreen = 0, nextExtendTime = 0, minGreen = 0;

	memcpy(splitTable, gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1], sizeof(GreenSignalRationItem) * NUM_PHASE);
	phaseTurn = &gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][0];//ring 0
	phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][0];
	for (i = 0; i < NUM_PHASE; i++)//phaseTurn array index
	{
		phaseId = phaseTurn->nTurnArray[i];
		if (phaseId == 0)
			break;
		//if ((phaseItem[phaseId - 1].nMaxGreen_1 + phaseItem[phaseId - 1].nAllRedTime + phaseItem[phaseId - 1].nYellowTime) < splitTable[phaseId - 1].nGreenSignalRationTime)
			//splitTable[phaseId - 1].nGreenSignalRationTime = phaseItem[phaseId - 1].nMaxGreen_1 + phaseItem[phaseId - 1].nAllRedTime + phaseItem[phaseId - 1].nYellowTime;
		//else if ((phaseItem[phaseId - 1].nMinGreen + phaseItem[phaseId - 1].nAllRedTime + phaseItem[phaseId - 1].nYellowTime) > splitTable[phaseId - 1].nGreenSignalRationTime)
			//splitTable[phaseId - 1].nGreenSignalRationTime = phaseItem[phaseId - 1].nMinGreen + phaseItem[phaseId - 1].nAllRedTime + phaseItem[phaseId - 1].nYellowTime;
		phaseTimes[phaseId - 1].maxExtendGreen = phaseItem[phaseId - 1].nMaxGreen_1 - splitTable[phaseId - 1].nGreenSignalRationTime + 
			phaseItem[phaseId - 1].nAllRedTime + phaseItem[phaseId - 1].nYellowTime;
		phaseTimes[phaseId - 1].minShortGreen = splitTable[phaseId - 1].nGreenSignalRationTime - phaseItem[phaseId - 1].nAllRedTime - 
			phaseItem[phaseId - 1].nYellowTime - phaseItem[phaseId - 1].nMinGreen;

		maxGreen = phaseTimes[phaseId - 1].maxExtendGreen;
		minGreen = phaseTimes[phaseId - 1].minShortGreen;
		if (maxGreen < 0 && gNextCycleExtendTime[phaseId - 1] > maxGreen)
			gNextCycleExtendTime[phaseId - 1] += maxGreen;
		else if (minGreen < 0 && (gNextCycleExtendTime[phaseId - 1] + minGreen) < 0)
			gNextCycleExtendTime[phaseId - 1] -= minGreen;
		
		nextExtendTime = gNextCycleExtendTime[phaseId - 1];
		for (j = 0; j < NUM_PHASE; j++)//concurrency phase index
		{//find the min time of gNextCycleExtendTime when dual ring concurrency
			conPhase = phaseItem[phaseId - 1].byPhaseConcurrency[j];
			if (conPhase == 0)
				continue;
			nextExtendTime = min(nextExtendTime, gNextCycleExtendTime[conPhase - 1]);
		}
		
		
		/*
		if (maxGreen < 0 && nextExtendTime > maxGreen)
			nextExtendTime += maxGreen;
		if (minGreen < 0 && (nextExtendTime + minGreen) < 0)
			nextExtendTime -= minGreen; 
			*/
		splitTable[phaseId - 1].nGreenSignalRationTime += min(nextExtendTime, maxGreen);
		calInfo->cycleTime += min(nextExtendTime, maxGreen);
		for (j = 0; j < NUM_PHASE; j++)//concurrency phase index
		{
			conPhase = phaseItem[phaseId - 1].byPhaseConcurrency[j];
			if (conPhase == 0)
				continue;
			splitTable[conPhase - 1].nGreenSignalRationTime += min(nextExtendTime, maxGreen);
		}
		//INFO("phase[%d]maxextendtime=%d, minshorttime=%d, nextextendtime=%d, greensplit=%d", 
			//phaseId, phaseTimes[phaseId - 1].maxExtendGreen, phaseTimes[phaseId - 1].minShortGreen, nextExtendTime, splitTable[phaseId - 1].nGreenSignalRationTime);
	}
	//INFO("DEBUG: enter %s function!!!!!", __func__);
    for(i = 0 ; i < NUM_PHASE; i++)
    {
		splitItem = &splitTable[i];
		phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i];
		entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][i];
		if ((phaseItem->nCircleID == 0) || (splitItem->nGreenSignalRationTime == 0))
			continue;
		if (splitItem->nIsCoordinate)
		{	//˵��������Э����λ����ʱ���û�������������Ӧ��ִ��Э������
			if (calInfo->coordinatePhaseId == 0)	//���ö��Э����λʱ���������ҵ���Ϊ׼
				calInfo->coordinatePhaseId = splitItem->nPhaseID;
			SET_BIT(calInfo->isCoordinatePhase, i);
		}
		if (splitItem->nType == IGNORE_PHASE)
		{
			SET_BIT(calInfo->isIgnorePhase, i);
		}
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		phaseTimes[i].splitTime = splitItem->nGreenSignalRationTime;
		passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
		passTimeInfo->yellowTime = phaseItem->nYellowTime;
		passTimeInfo->allRedTime = phaseItem->nAllRedTime;
		
		phaseTimes[i].pedestrianPassTime = phaseItem->nPedestrianPassTime;
		phaseTimes[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
		phaseTimes[i].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
		phaseTimes[i].laneLevel = GET_BIT(phaseItem->wPhaseOptions, 9);
		phaseTimes[i].pedestrianDetectorBits = GetPedDetectorBitsByPhase(i + 1, calInfo);
		phaseTimes[i].vehicleDetectorBits = GetDetectorBitsByPhase(i + 1, calInfo);
		GetBusBitsByPhase(i + 1, calInfo, phaseTimes[i].busDetectorBits);
		phaseTimes[i].busPrioPassTime = GetBusPrioPassTime(i + 1, calInfo);
		GetSpecialCarBitsByPhase(i + 1, calInfo, phaseTimes[i].scarDetectorBits);
		phaseTimes[i].scarPassTime = GetSpecialCarPassTime(i + 1, calInfo);
		phaseTimes[i].unitExtendGreen = phaseItem->nUnitExtendGreen;
    }
}

//check current phaseturn table must set one phase is mainlane phase.
static void CheckMainLanePhase(CalInfo *calInfo)
{
	int i = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	UINT8 mainlane = 0;

	for (ring = 0; ring < NUM_RING_COUNT && mainlane == 0; ring++)
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
				break;
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			if (GET_BIT(phaseItem->wPhaseOptions, 9) == 1)//main lane flag is 1
			{
				mainlane = 1;
				break;
			}
		}
	}
	if (mainlane == 0)//don't find mainlane phase, default set first phase in phaseturn'array to be mainlane phase
	{
		for (ring = 0; ring < NUM_RING_COUNT; ring++)
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[0];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
				break;
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			SET_BIT(phaseItem->wPhaseOptions, 9);
		}
	}

}

UINT64 ItsGetVehDetectorState(UINT64* vehDetectorState)
{
	UINT64 vehDetectorSt = 0;
	vehDetectorSt = GetVehDetectorState();
	*vehDetectorState = vehDetectorSt;
	return vehDetectorSt;
}
static void SetInductiveControlTimeOnVehDetErr(CalInfo *calInfo)
{
    int i = 0, j = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	GreenSignalRationItem *psplit;

    for (ring = 0; ring < 1; ring++)//only one ring
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
				break;
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][nPhaseId - 1];
			psplit = gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			phaseTimes[nPhaseId - 1].vehicleDetectorBits = GetDetectorBitsByPhase(nPhaseId, calInfo);
			passTimeInfo = &phaseTimes[nPhaseId - 1].passTimeInfo;
			if (calInfo->vehDetectorState & phaseTimes[nPhaseId - 1].vehicleDetectorBits)
			{
				phaseTimes[nPhaseId - 1].splitTime = psplit[nPhaseId - 1].nGreenSignalRationTime;
				passTimeInfo->greenTime = phaseTimes[nPhaseId - 1].splitTime - entry->nGreenLightTime - phaseItem->nYellowTime - phaseItem->nAllRedTime;
				passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
				passTimeInfo->yellowTime = phaseItem->nYellowTime;
				passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			}
			else
			{
			    phaseTimes[nPhaseId - 1].splitTime = phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime;
			    passTimeInfo->greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
			    passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
			    passTimeInfo->yellowTime = phaseItem->nYellowTime;
			    passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			}
			
			if (ring == 0)	//�ڵ�һ������ѯʱ��������
			{
				calInfo->cycleTime += phaseTimes[nPhaseId - 1].splitTime;
			}
				
			phaseTimes[nPhaseId - 1].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			phaseTimes[nPhaseId - 1].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			phaseTimes[nPhaseId - 1].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
			phaseTimes[nPhaseId - 1].laneLevel = GET_BIT(phaseItem->wPhaseOptions, 9);
			
			
			phaseTimes[nPhaseId - 1].pedestrianDetectorBits = GetPedDetectorBitsByPhase(nPhaseId, calInfo);
			GetBusBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].busDetectorBits);
			phaseTimes[nPhaseId - 1].busPrioPassTime = GetBusPrioPassTime(nPhaseId, calInfo);
			
			GetSpecialCarBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].scarDetectorBits);
			phaseTimes[nPhaseId - 1].scarPassTime = GetSpecialCarPassTime(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].unitExtendGreen = phaseItem->nUnitExtendGreen;
			if (phaseItem->nMaxGreen_1 > phaseItem->nMinGreen)
			{
				phaseTimes[nPhaseId - 1].maxExtendGreen = phaseItem->nMaxGreen_1 - phaseItem->nMinGreen;
                if(phaseItem->nMaxGreen_2 > phaseItem->nMaxGreen_1)
                {
                    phaseTimes[nPhaseId - 1].maxExtendGreen2 = phaseItem->nMaxGreen_2 - phaseItem->nMinGreen;
                    //INFO("@@@phaseTimes[%d] = %d\n",nPhaseId , phaseTimes[nPhaseId - 1].maxExtendGreen2);
                }
            }
		}
	}
}

static void SetInductiveControlTime(CalInfo *calInfo)
{
    int i = 0, j = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;

    for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE) 
				break;
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][nPhaseId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			passTimeInfo = &phaseTimes[nPhaseId - 1].passTimeInfo;
			phaseTimes[nPhaseId - 1].splitTime = phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime;
			passTimeInfo->greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
			passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
			passTimeInfo->yellowTime = phaseItem->nYellowTime;
			passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			
			if (ring == 0)	//�ڵ�һ������ѯʱ��������
			{
				calInfo->cycleTime += phaseTimes[nPhaseId - 1].splitTime;
			}
				
			phaseTimes[nPhaseId - 1].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			phaseTimes[nPhaseId - 1].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			phaseTimes[nPhaseId - 1].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
			phaseTimes[nPhaseId - 1].laneLevel = GET_BIT(phaseItem->wPhaseOptions, 9);
			
			phaseTimes[nPhaseId - 1].vehicleDetectorBits = GetDetectorBitsByPhase(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].pedestrianDetectorBits = GetPedDetectorBitsByPhase(nPhaseId, calInfo);
			GetBusBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].busDetectorBits);
			phaseTimes[nPhaseId - 1].busPrioPassTime = GetBusPrioPassTime(nPhaseId, calInfo);
			
			GetSpecialCarBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].scarDetectorBits);
			phaseTimes[nPhaseId - 1].scarPassTime = GetSpecialCarPassTime(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].unitExtendGreen = phaseItem->nUnitExtendGreen;
			if (phaseItem->nMaxGreen_1 > phaseItem->nMinGreen)
			{
				phaseTimes[nPhaseId - 1].maxExtendGreen = phaseItem->nMaxGreen_1 - phaseItem->nMinGreen;
                if(phaseItem->nMaxGreen_2 > phaseItem->nMaxGreen_1)
                {
                    phaseTimes[nPhaseId - 1].maxExtendGreen2 = phaseItem->nMaxGreen_2 - phaseItem->nMinGreen;
                    //INFO("@@@phaseTimes[%d] = %d\n",nPhaseId , phaseTimes[nPhaseId - 1].maxExtendGreen2);
                }
            }
		}
	}
}

static void SetInductiveCoordinateControlTime(CalInfo *calInfo)
{
    int i = 0, j = 0, ring = 0;
	UInt8 nPhaseId;
	PPhaseItem phaseItem = NULL;
	PGreenSignalRationItem splitItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	PhaseTimeInfo *phaseTimes = calInfo->phaseTimes;
	PassTimeInfo *passTimeInfo;
	UInt16 cycleTime = 0;

	for (ring = 0; ring < NUM_RING_COUNT; ring++)
	{
		cycleTime = 0;
		for (i = 0; i < NUM_PHASE; i++) 
		{
			nPhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][ring].nTurnArray[i];
			if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
			{
				if (calInfo->cycleTime < cycleTime)
					calInfo->cycleTime = cycleTime;
				break;
			}
			
			splitItem = &gRunConfigPara->stGreenSignalRation[calInfo->splitId - 1][nPhaseId - 1];
			phaseItem = &gRunConfigPara->stPhase[calInfo->phaseTableId - 1][nPhaseId - 1];
			entry = &gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][nPhaseId - 1];
			if (phaseItem->nCircleID == 0)
				continue;
			passTimeInfo = &phaseTimes[nPhaseId - 1].passTimeInfo;
			
			if (i == 0)	//ÿ���еĵ�һ����λ��ΪЭ����λ��Ĭ��ִ�����űȵ���ʱʱ��
				passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime - phaseItem->nYellowTime - phaseItem->nAllRedTime;
			else	//������������λ��Ĭ��ִ����С��
			{
				if (splitItem->nGreenSignalRationTime <= phaseItem->nMinGreen + phaseItem->nYellowTime + phaseItem->nAllRedTime)	//�����űȵ�ʱ��С�ڵ�����С��ʱ��Ĭ��ִ�����ű�ʱ��
					passTimeInfo->greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime - phaseItem->nYellowTime - phaseItem->nAllRedTime;
				else
					passTimeInfo->greenTime = phaseItem->nMinGreen - entry->nGreenLightTime;
			}
				
			passTimeInfo->greenBlinkTime = entry->nGreenLightTime;
			passTimeInfo->yellowTime = phaseItem->nYellowTime;
			passTimeInfo->allRedTime = phaseItem->nAllRedTime;
			phaseTimes[nPhaseId - 1].splitTime = passTimeInfo->greenTime 
												+ passTimeInfo->greenBlinkTime 
												+ passTimeInfo->yellowTime 
												+ passTimeInfo->allRedTime;
			cycleTime += phaseTimes[nPhaseId - 1].splitTime;
			
			phaseTimes[nPhaseId - 1].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			phaseTimes[nPhaseId - 1].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			phaseTimes[nPhaseId - 1].pedAutoRequestFlag = GET_BIT(phaseItem->wPhaseOptions, 13);
			phaseTimes[nPhaseId - 1].laneLevel = GET_BIT(phaseItem->wPhaseOptions, 9);
			
			phaseTimes[nPhaseId - 1].vehicleDetectorBits = GetDetectorBitsByPhase(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].pedestrianDetectorBits = GetPedDetectorBitsByPhase(nPhaseId, calInfo);
			GetBusBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].busDetectorBits);
			phaseTimes[nPhaseId - 1].busPrioPassTime = GetBusPrioPassTime(nPhaseId, calInfo);
			GetSpecialCarBitsByPhase(nPhaseId, calInfo, phaseTimes[nPhaseId - 1].scarDetectorBits);
			phaseTimes[nPhaseId - 1].scarPassTime = GetSpecialCarPassTime(nPhaseId, calInfo);
			phaseTimes[nPhaseId - 1].unitExtendGreen = phaseItem->nUnitExtendGreen;
			phaseTimes[nPhaseId - 1].maxExtendGreen = splitItem->nGreenSignalRationTime - phaseTimes[nPhaseId - 1].splitTime;
		}
	}
}

static void SetNextCycleRealtimePattern(CalInfo *calInfo)
{
	MsgRealtimePattern *p = &gNextCycleRealtimePattern;
	int i;
	
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	memset(p, 0, sizeof(MsgRealtimePattern));
	p->nPatternId = calInfo->schemeId;
	p->nSplitId = calInfo->splitId;
	p->nPhaseTurnId = calInfo->phaseTurnId;
	p->nPhaseTableId = calInfo->phaseTableId;
	p->nChannelTableId = calInfo->channelTableId;
	if (calInfo->schemeId > 0 && calInfo->schemeId <= NUM_SCHEME)
		p->nOffset = gRunConfigPara->stScheme[calInfo->schemeId - 1].nOffset;
	if (calInfo->phaseTurnId > 0 && calInfo->phaseTurnId <= NUM_PHASE_TURN)
		memcpy(p->sPhaseTurn, gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1], sizeof(p->sPhaseTurn));
	p->nCoordinatePhase = calInfo->isCoordinatePhase;
	if (calInfo->phaseTableId > 0 && calInfo->phaseTableId <= MAX_PHASE_TABLE_COUNT)
	{
		for (i = 0; i < NUM_PHASE; i++)
		{
			p->phaseTime[i].greenBlink = gRunConfigPara->AscSignalTransTable[calInfo->phaseTableId - 1][i].nGreenLightTime;
			p->phaseTime[i].yellow = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i].nYellowTime;
			p->phaseTime[i].allred = gRunConfigPara->stPhase[calInfo->phaseTableId - 1][i].nAllRedTime;
		}
		memcpy(p->stFollowPhase, gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1], sizeof(p->stFollowPhase));
	}
	memcpy(p->phaseDesc, gStructBinfileDesc.phaseDescText[calInfo->phaseTableId - 1], sizeof(p->phaseDesc));
}

static int IsConcurrencyPhase(PhaseItem* phaseItems, UINT8 phaseId,UINT8 concurrencyPhase)
{
	int i = 0;
	if (0 >= phaseId || phaseId > NUM_PHASE)
		return 0;
	for (i = 0; i < NUM_PHASE; i++)
		if (phaseItems[phaseId - 1].byPhaseConcurrency[i] == concurrencyPhase)
			return 1;
	return 0;
}

static int RingConcurrencyOk(CalInfo *calInfo, UINT8 ring)
{
	int i = 0, j = 0;
	UINT16 ringtime1 = 0, ringtime2 = 0;
	UINT16 cycletime = 0;
	PhaseItem* phaseItems = gRunConfigPara->stPhase[calInfo->phaseTableId - 1];
	PhaseTurnItem* phaseTurns = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1];
	PhaseTimeInfo* phaseTimes = calInfo->phaseTimes;

	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseTurns[ring].nTurnArray[i] <= 0 || phaseTurns[ring].nTurnArray[i] > NUM_PHASE)
			break;
		cycletime += phaseTimes[phaseTurns[ring].nTurnArray[i] - 1].splitTime;
	}
	//INFO("ring[%d] cycletime=%d, calinfo cycletime=%d", ring, cycletime, calInfo->cycleTime);
	if (cycletime != calInfo->cycleTime)//ring's cycle time don't be equal
		return 0;
	i = 0;
	j = 0;
	ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
	ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
	//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
	while(1)
	{
		if (i >= NUM_PHASE || j >= NUM_PHASE)
			break;
		if (!IsConcurrencyPhase(phaseItems, phaseTurns[0].nTurnArray[i], phaseTurns[ring].nTurnArray[j]))
			return 0;
		if (ringtime1 == ringtime2)
		{
			if (ringtime1 == cycletime && ringtime2 == cycletime)
				break;
			i++;
			j++;
			ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
			ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
		else if (ringtime1 < ringtime2)
		{
			i++;
			ringtime1 += phaseTimes[phaseTurns[0].nTurnArray[i] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
		else if (ringtime1 > ringtime2)
		{
			j++;
			ringtime2 += phaseTimes[phaseTurns[ring].nTurnArray[j] - 1].splitTime;
			//INFO("phase[i]= %d ringtime1= %d, phase[j]=%d ringtime2 = %d", phaseTurns[0].nTurnArray[i], ringtime1, phaseTurns[ring].nTurnArray[j] , ringtime2);
		}
	}
	return 1;
}

static int JudgeConcurrencyLegal(CalInfo *calInfo)
{
	int ring = 0;
	int i = 0;
	PhaseTurnItem* pphaseturn = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1];

	for (ring = 1; ring < NUM_RING_COUNT; ring++)
	{
		if (pphaseturn[ring].nTurnArray[0] == 0)
			continue;
		if (0 == RingConcurrencyOk(calInfo, ring))
			return 0;
	}
	return 1;
}

void ItsGetCurControlType(UINT8 controltype)
{
	gCurControlType = controltype;
}

Boolean FillCalInfo(CalInfo *calInfo, UInt8 mSchemeId, time_t calTime)
{
    struct tm now;	
    int timeGapMinutes = 0;	//��ǰʱ���뵱ǰʱ�ε���ʼʱ��Ĳ�ֵ����λΪ����
    UInt8 nTempSchemeId = 0;
    static UINT8 is_first_cyle = TRUE;      //�Ƿ��ǳ���������ĵ�һ������
	UINT8 i = 0;
	//INFO("DEBUG: enter %s function!!!!!", __func__);
	localtime_r(&calTime, &now);	
	memset(calInfo, 0, sizeof(CalInfo));
	calInfo->transitionCycle = gRunConfigPara->stUnitPara.byTransCycle;
	calInfo->collectCycle = gRunConfigPara->stUnitPara.byFluxCollectCycle;
	calInfo->checkTime = gStructBinfileCustom.sCountdownParams.iFreeGreenTime;
    calInfo->schemeId = (mSchemeId > 0) ? mSchemeId : GetSchemeIdAndTimeGap(calInfo, &now, &timeGapMinutes);
	//INFO("calInfo->schemeId=%d", calInfo->schemeId);
    //calInfo->schemeId = SINGLE_OPTIMIZE_SCHEMEID;//BUS_PRIORITY_SCHEMEID;            //�Լ�����
    if (calInfo->schemeId == YELLOWBLINK_SCHEMEID || calInfo->schemeId == ALLRED_SCHEMEID || calInfo->schemeId == TURNOFF_SCHEMEID)
	{	//�������
		//log_debug("special control, scheme: %d", calInfo->schemeId);
        return TRUE;
	}
	else if ((calInfo->schemeId == INDUCTIVE_SCHEMEID) 
		|| ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (2 == calInfo->schemeId % 3)))
    {	//��Ӧ����,Ĭ�ϰ��շ���1���ܸ�Ӧ��������ʵ�ʵķ����Ŷ�Ӧ��ӳ�䷽����ִ��
        nTempSchemeId = (calInfo->schemeId == INDUCTIVE_SCHEMEID) ? 1 : (calInfo->schemeId - 1);
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->splitId = gRunConfigPara->stScheme[nTempSchemeId - 1].nGreenSignalRatioID;
		calInfo->channelTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nPhaseTableID;
		calInfo->actionId = INDUCTIVE_ACTIONID;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //����ǳ���������ĵ�һ�����ڣ����gCurRunData.channelTableId��һ����Ч��ͨ����ţ��Խ����һ������Ϊ��Ӧ��Э����Ӧ��
        //����Ӧ����ʱ���ܸ�Ӧ��bug��
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
        if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
        {
            log_error("scheme %d config error when excute inductive control", nTempSchemeId);
            return FALSE;
        }
		CheckMainLanePhase(calInfo);
		ItsGetVehDetectorState(&(calInfo->vehDetectorState));
		if (gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][1].nTurnArray[0] != 0 ||
			gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][2].nTurnArray[0] != 0 ||
			gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][3].nTurnArray[0] != 0)
		{
    	//����Ǹ�Ӧ����Ӧ��ʹ����С��+�Ƶ�ʱ��+ȫ��ʱ����Ϊ���ű�ʱ��
        	SetInductiveControlTime(calInfo);
		}
		else
		{
			SetInductiveControlTimeOnVehDetErr(calInfo);
		}
    }
	else if ((calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID) 
		|| ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (0 == calInfo->schemeId % 3)))
    {	//Э����Ӧ����,Ĭ�ϰ��շ���1����Э����Ӧ��������ʵ�ʵķ����Ŷ�Ӧ��ӳ�䷽����ִ��	
        nTempSchemeId = (calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID) ? 1 : (calInfo->schemeId - 2);
		calInfo->inductiveCoordinateCycleTime = gRunConfigPara->stScheme[nTempSchemeId - 1].nCycleTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[nTempSchemeId - 1].nOffset;
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->splitId = gRunConfigPara->stScheme[nTempSchemeId - 1].nGreenSignalRatioID;
		calInfo->actionId = INDUCTIVE_COORDINATE_ACTIONID;
		calInfo->channelTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[(nTempSchemeId + 2) / 3 - 1].nPhaseTableID;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //����ǳ���������ĵ�һ�����ڣ����gCurRunData.channelTableId��һ����Ч��ͨ����ţ��Խ����һ������Ϊ��Ӧ��Э����Ӧ��
        //����Ӧ����ʱ���ܸ�Ӧ��bug��
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
		if (calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN
			|| calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION
			|| calInfo->inductiveCoordinateCycleTime == 0)
		{
			log_error("scheme %d config error when excute inductive coordinate control", nTempSchemeId);
			return FALSE;
		}
		calInfo->coordinatePhaseId = gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][0].nTurnArray[0];
		//�ֶ�ִ�и�ӦЭ������ʱĬ����0��Ϊ��ʼʱ�ν��й���
		if (mSchemeId > 0)
			timeGapMinutes = now.tm_hour * 60 + now.tm_min;
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
        SetInductiveCoordinateControlTime(calInfo);
    }
	else if ((calInfo->schemeId == SINGLE_ADAPT_SCHEMEID)) 
    {	//����Ӧ����,Ĭ�ϰ��շ���1��Ϊ����������
        nTempSchemeId = 1;
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->channelTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nPhaseTableID;
		calInfo->actionId = SINGLE_ADAPT_ACTIONID;
		//calInfo->schemeId = 1;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
        //����ǳ���������ĵ�һ�����ڣ����gCurRunData.channelTableId��һ����Ч��ͨ����ţ��Խ����һ������Ϊ��Ӧ��Э����Ӧ��
        //����Ӧ����ʱ���ܸ�Ӧ��bug��
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
        if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
        {
            log_error("scheme %d config error when excute single adapt control", nTempSchemeId);
            return FALSE;
        }
    	//����Ӧ���ƺ͸�Ӧ����һ����Ӧ��ʹ����С��+�Ƶ�ʱ��+ȫ��ʱ����Ϊ���ű�ʱ��
        SetInductiveControlTime(calInfo);
    }
	else if ((calInfo->schemeId == BUS_PRIORITY_SCHEMEID))
	{
		nTempSchemeId = 1;//default scheme id  of bus priority is 1.
    	calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;
		calInfo->channelTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nChannelTableID;
		calInfo->phaseTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nPhaseTableID;
		calInfo->actionId = BUS_PRIORITY_ACTIONID;
		//calInfo->schemeId = 1;
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
			calInfo->channelTableId = 1;
		if(is_first_cyle == TRUE)
        {// the bug thst can't inductive cars when exec single adapter inductive when the control mode of the first cycle since tsc start is inductive or coordinate inductive control
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
			calInfo->phaseTableId = 1;
        if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
        {
            log_error("scheme %d config error when excute bus priority control", nTempSchemeId);
            return FALSE;
        }
		//bus priority control base on the inductive,calculation same as inductive control.
		SetInductiveControlTime(calInfo);
	}
	else if (calInfo->schemeId == PEDESTRIAN_REQ_SCHEMEID)
	{
		nTempSchemeId = 1;
		calInfo->splitId = gRunConfigPara->stScheme[nTempSchemeId - 1].nGreenSignalRatioID;//���ݷ�����ID���õ����űȱ�ID
		if(calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION)
		{
			log_error("can't find split id, scheme: %d , splitId: %d", nTempSchemeId,calInfo->splitId);
			return FALSE;
		}

		calInfo->cycleTime = gRunConfigPara->stScheme[nTempSchemeId - 1].nCycleTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[nTempSchemeId - 1].nOffset;
		calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;//���ݷ�����ID���õ������ID
		if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
		{
			log_error("can't find phase turn id, scheme: %d", nTempSchemeId);
			return FALSE;
		}
		
		calInfo->phaseTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nPhaseTableID;
		calInfo->channelTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nChannelTableID;
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
		{	
			calInfo->phaseTableId = 1;
		}
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		{
			calInfo->channelTableId = 1;
		}
		calInfo->actionId = PEDESTRIAN_REQ_ACTIONID;
		//calInfo->schemeId = 1;
		for (i = 0; i < MAX_PHASE_NUM; i++)
		{
			if (gRunConfigPara->stPhaseTurn[calInfo->phaseTurnId - 1][0].nTurnArray[i] == 0)
			{
				if (i == 2)
					calInfo->pedestrianRequest = 1;//config two phase is pedestrian once req.
				else if (i == 4)
					calInfo->pedestrianRequest = 2;//config four phase is pedestrian twice req.
				break;
			}
		}
		
        //����ǳ���������ĵ�һ�����ڣ����gCurRunData.channelTableId��һ����Ч��ͨ����ţ��Խ����һ������Ϊ��Ӧ��Э����Ӧ��
        //����Ӧ����ʱ���ܸ�Ӧ��bug��
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		//���ݵ�ǰ�����űȱ�ID������λ���и���λ�����ű�ʱ��θ�ֵ����ͬʱ�ҳ�Э����λ
		SetPhaseTime(calInfo);
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	}
	else if ((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (1 == calInfo->schemeId % 3))
	{	//��ͨ����
		calInfo->splitId = gRunConfigPara->stScheme[calInfo->schemeId - 1].nGreenSignalRatioID;//���ݷ�����ID���õ����űȱ�ID
		if(calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION)
		{
			log_error("can't find split id, scheme: %d , splitId: %d", calInfo->schemeId,calInfo->splitId);
			return FALSE;
		}

		calInfo->cycleTime = gRunConfigPara->stScheme[calInfo->schemeId - 1].nCycleTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[calInfo->schemeId - 1].nOffset;
		calInfo->phaseTurnId = gRunConfigPara->stScheme[calInfo->schemeId - 1].nPhaseTurnID;//���ݷ�����ID���õ������ID
		if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
		{
			log_error("can't find phase turn id, scheme: %d", calInfo->schemeId);
			return FALSE;
		}
		if (calInfo->actionId == 0)	//����1,4,7��Ӧ����1��2��3
			calInfo->actionId = (calInfo->schemeId + 2) / 3;
		calInfo->phaseTableId = gRunConfigPara->stAction[calInfo->actionId - 1].nPhaseTableID;
		calInfo->channelTableId = gRunConfigPara->stAction[calInfo->actionId - 1].nChannelTableID;
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
		{	
			calInfo->phaseTableId = 1;
			gRunConfigPara->stAction[calInfo->actionId - 1].nPhaseTableID = 1;
		}
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		{
			calInfo->channelTableId = 1;
			gRunConfigPara->stAction[calInfo->actionId - 1].nChannelTableID = 1;
		}
        //����ǳ���������ĵ�һ�����ڣ����gCurRunData.channelTableId��һ����Ч��ͨ����ţ��Խ����һ������Ϊ��Ӧ��Э����Ӧ��
        //����Ӧ����ʱ���ܸ�Ӧ��bug��
        if(is_first_cyle == TRUE)
        {
            gCurRunData.channelTableId = calInfo->channelTableId;
            is_first_cyle = FALSE;
        }
		//���ݵ�ǰ�����űȱ�ID������λ���и���λ�����ű�ʱ��θ�ֵ����ͬʱ�ҳ�Э����λ
		SetPhaseTime(calInfo);
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	}
	else if (calInfo->schemeId == SINGLE_OPTIMIZE_SCHEMEID)
	{	//��������Ӧ
		nTempSchemeId = 1;
		calInfo->splitId = gRunConfigPara->stScheme[nTempSchemeId - 1].nGreenSignalRatioID;//���ݷ�����ID���õ����űȱ�ID
		if(calInfo->splitId == 0 || calInfo->splitId > NUM_GREEN_SIGNAL_RATION)
		{
			log_error("can't find split id, scheme: %d , splitId: %d", calInfo->schemeId,calInfo->splitId);
			return FALSE;
		}
		calInfo->actionId = SINGLE_OPTIMIZE_ACTIONID;
		calInfo->cycleTime = gRunConfigPara->stScheme[nTempSchemeId - 1].nCycleTime;
		calInfo->singlespotCheckTime = gStructBinfileConfigPara.sSpecialParams.singleSpotCheckTime;
		calInfo->phaseOffset = gRunConfigPara->stScheme[nTempSchemeId - 1].nOffset;
		calInfo->phaseTurnId = gRunConfigPara->stScheme[nTempSchemeId - 1].nPhaseTurnID;//���ݷ�����ID���õ������ID
		if(calInfo->phaseTurnId == 0 || calInfo->phaseTurnId > NUM_PHASE_TURN)
		{
			log_error("can't find phase turn id, scheme: %d", calInfo->schemeId);
			return FALSE;
		}
		calInfo->phaseTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nPhaseTableID;
		calInfo->channelTableId = gRunConfigPara->stAction[nTempSchemeId - 1].nChannelTableID;
		if (calInfo->phaseTableId == 0 || calInfo->phaseTableId > MAX_PHASE_TABLE_COUNT)
		{	
			calInfo->phaseTableId = 1;
		}
		if (calInfo->channelTableId == 0 || calInfo->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		{
			calInfo->channelTableId = 1;
		}
        
		//���ݵ�ǰ�����űȱ�ID������λ���и���λ�����ű�ʱ��θ�ֵ����ͬʱ�ҳ�Э����λ
		SetPhaseTimeForSingleSpot(calInfo);
		calInfo->timeGapSec = timeGapMinutes * 60 + now.tm_sec;
	}
	else
	{
		log_error("can't find scheme id, action: %d", calInfo->actionId);
        return FALSE;
	}
	if (calInfo->schemeId == INDUCTIVE_SCHEMEID || 
		calInfo->schemeId == INDUCTIVE_COORDINATE_SCHEMEID ||
		calInfo->schemeId == SINGLE_ADAPT_SCHEMEID ||
		((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (2 == calInfo->schemeId % 3)) ||
		((calInfo->schemeId > 0) && (calInfo->schemeId <= NUM_SCHEME) && (0 == calInfo->schemeId % 3)))
	{
		if (0 == JudgeConcurrencyLegal(calInfo)) //�жϸ�Ӧʱ�����Ƿ����������������
		{
			ItsWriteFaultLog(RING_CONCURRENCY_ILLEGAL, calInfo->phaseTableId);
			log_error("When run inductive scheme, ring concurrency illegal. YellowBlink control.");
			ItsCtl(TOOL_CONTROL, YELLOWBLINK_SCHEMEID, 0);
			return TRUE;
		}
	}
	CalStageInfo(calInfo);		//����׶���ص���Ϣ
    SetNextCycleRealtimePattern(calInfo);
	CalConflictChannel(gRunConfigPara->stPhase[calInfo->phaseTableId - 1], gRunConfigPara->stChannel[calInfo->channelTableId - 1], gRunConfigPara->stFollowPhase[calInfo->phaseTableId - 1], gConflictChannel[1]);
	SetChannelBits(calInfo);	//������λ��Ӧ��ͨ��bitλ
	gCurrentAction = calInfo->actionId;
	gCurrentScheme = calInfo->schemeId;
	return TRUE;
}

void ItsGetRealtimePattern(void *udpdata)
{
	UInt32 *udpHead = (UInt32 *)udpdata;
	MsgRealtimePattern *p = (MsgRealtimePattern *)udpdata;
	if (udpdata == NULL)
		return;
	if (udpHead[2] == 1)
	{	//udp���ݵĵ�����intΪ1������ȡ���ǵ�ǰ���ڷ�����Ϣ
		memcpy(p, &gCurrentCycleRealtimePattern, sizeof(MsgRealtimePattern));
		p->ucIsCurrentCycle = 1;
	}
	else
	{
		memcpy(p, &gNextCycleRealtimePattern, sizeof(MsgRealtimePattern));
		p->ucIsCurrentCycle = 0;
	}
	p->unExtraParamHead = 0x6e6e;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	p->unExtraParamID = 0xd0;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd0
}

//����ʵʱ��Ϣ��1sһ��
void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data)
{
	int i;
	const PhaseInfo *phaseInfos = data->phaseInfos;
	
	pthread_rwlock_wrlock(&gCountDownLock);
	memset(&gCountDownParams, 0, sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS));
	gCountDownParams.unExtraParamHead = 0x6e6e;
	gCountDownParams.unExtraParamID = 0x9e;
	gCountDownParams.ucPlanNo = data->schemeId;
	gCountDownParams.ucCurCycleTime = (data->actionId == INDUCTIVE_COORDINATE_ACTIONID) ? data->inductiveCoordinateCycleTime : data->cycleTime;
	gCountDownParams.ucCurRunningTime = (data->cycleTime) ? (data->cycleTime - data->leftTime + 1) : 0;	//��������ʱ���Ǵ�1��ʼ
	if (gCountDownParams.ucCurRunningTime == 1)	//ÿ�����ڵ�1s���µ�ǰ���ڷ�����Ϣ
	{
		memcpy(&gCurrentCycleRealtimePattern, &gNextCycleRealtimePattern, sizeof(MsgRealtimePattern));
	}
		
	switch (data->schemeId)
	{
		case YELLOWBLINK_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, (ItsControlStatusGet() == 10) ? "���ϻ���" : "����"); break;
		case ALLRED_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "ȫ��"); break;
		case TURNOFF_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "�ص�"); break;
		case INDUCTIVE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "��Ӧ"); break;
		case INDUCTIVE_COORDINATE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "Э����Ӧ"); break;
		case SINGLE_ADAPT_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "����Ӧ"); break;
		case PEDESTRIAN_REQ_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "���˹���"); break;
		case SINGLE_OPTIMIZE_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "�����Ż�"); break;
		case BUS_PRIORITY_SCHEMEID: strcpy((char *)gCountDownParams.ucCurPlanDsc, "��������"); break;
	}
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		gCountDownParams.stVehPhaseCountingDown[i][0] = phaseInfos[i].phaseStatus;
		gCountDownParams.stVehPhaseCountingDown[i][1] = phaseInfos[i].phaseLeftTime;
		gCountDownParams.stPedPhaseCountingDown[i][0] = phaseInfos[i].pedestrianPhaseStatus;
		gCountDownParams.stPedPhaseCountingDown[i][1] = phaseInfos[i].pedestrianPhaseLeftTime;

		gCountDownParams.ucOverlap[i][0] = phaseInfos[i].followPhaseStatus;
		gCountDownParams.ucOverlap[i][1] = phaseInfos[i].followPhaseLeftTime;
		gCountDownParams.stPhaseRunningInfo[i][0] = phaseInfos[i].splitTime;
		gCountDownParams.stPhaseRunningInfo[i][1] = (phaseInfos[i].phaseStatus != RED && phaseInfos[i].splitTime > 0) ? (phaseInfos[i].splitTime - phaseInfos[i].phaseSplitLeftTime + 1) : 0;
	}
	
//	gCountDownParams.ucChannelLockStatus = (lockparams->ucChannelLockStatus == CHANGEABLE_CHANNEL_LOCK) ? 0 : lockparams->ucChannelLockStatus;	//����ǿɱ�ͨ�������򵹼�ʱ�в���ʾΪ����״̬
	gCountDownParams.ucChannelLockStatus = lockflag;	//����ǿɱ�ͨ�������򵹼�ʱ�в���ʾΪ����״̬
	memcpy(gCountDownParams.ucChannelStatus, data->allChannels, sizeof(gCountDownParams.ucChannelStatus));	//��֮ǰ������״̬ȫ����Ϊʵʱ��ͨ��״̬
	memcpy(gCountDownParams.ucChannelCountdown, data->channelCountdown, sizeof(gCountDownParams.ucChannelCountdown));
	
	gCountDownParams.ucIsStep = data->isStep;
	gCountDownParams.ucMaxStageNum = data->maxStageNum;
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsCountDownGet(void *countdown, int size)
{
	if (countdown == NULL || size != sizeof(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS))
		return;
	pthread_rwlock_rdlock(&gCountDownLock);
	memcpy(countdown, &gCountDownParams, size);
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
void ItsSetCurRunData(LineQueueData *data)
{
	if (data == NULL)
		return;
	//INFO("Set running data...0:%d, 1:%d", data->allChannels[0], data->allChannels[1]);
	pthread_rwlock_wrlock(&gCountDownLock);
	memcpy(&gCurRunData, data, sizeof(LineQueueData));
	pthread_rwlock_unlock(&gCountDownLock);
}

void ItsGetConflictChannel(UInt32 *conflictChannels)
{
	if (conflictChannels == NULL)
		return;
	memcpy(conflictChannels, gConflictChannel[0], sizeof(gConflictChannel[0]));
}

//��������źŵ�����ʱ��
void ItsCountDownOutput(LineQueueData *data)
{
    CountDownInterface(data);
}

UNUSEDATTR static void PrintVehCountDown(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintVehCountDown>  %s:%d--%s:%d--%s:%d--%s:%d   %s:%d--%s:%d--%s:%d--%s:%d\n\n",\
		GET_COLOR(phaseInfos[0].phaseStatus), phaseInfos[0].phaseLeftTime,
		GET_COLOR(phaseInfos[1].phaseStatus), phaseInfos[1].phaseLeftTime,
		GET_COLOR(phaseInfos[2].phaseStatus), phaseInfos[2].phaseLeftTime,
		GET_COLOR(phaseInfos[3].phaseStatus), phaseInfos[3].phaseLeftTime,
		GET_COLOR(phaseInfos[4].phaseStatus), phaseInfos[4].phaseLeftTime,
		GET_COLOR(phaseInfos[5].phaseStatus), phaseInfos[5].phaseLeftTime,
		GET_COLOR(phaseInfos[6].phaseStatus), phaseInfos[6].phaseLeftTime,
		GET_COLOR(phaseInfos[7].phaseStatus), phaseInfos[7].phaseLeftTime);
}

UNUSEDATTR static void PrintRunInfo(LineQueueData *data)
{
	PhaseInfo *phaseInfos = data->phaseInfos;
    OFTEN("PrintRunInfo>  %d/%d--%d/%d--%d/%d--%d/%d   %d/%d--%d/%d--%d/%d--%d/%d  %d/%d",\
		phaseInfos[0].splitTime - phaseInfos[0].phaseSplitLeftTime + 1, phaseInfos[0].splitTime,
		phaseInfos[1].splitTime - phaseInfos[1].phaseSplitLeftTime + 1, phaseInfos[1].splitTime,
		phaseInfos[2].splitTime - phaseInfos[2].phaseSplitLeftTime + 1, phaseInfos[2].splitTime,
		phaseInfos[3].splitTime - phaseInfos[3].phaseSplitLeftTime + 1, phaseInfos[3].splitTime,
		phaseInfos[4].splitTime - phaseInfos[4].phaseSplitLeftTime + 1, phaseInfos[4].splitTime,
		phaseInfos[5].splitTime - phaseInfos[5].phaseSplitLeftTime + 1, phaseInfos[5].splitTime,
		phaseInfos[6].splitTime - phaseInfos[6].phaseSplitLeftTime + 1, phaseInfos[6].splitTime,
		phaseInfos[7].splitTime - phaseInfos[7].phaseSplitLeftTime + 1, phaseInfos[7].splitTime,
		data->cycleTime - data->leftTime + 1, data->cycleTime);
}
//�˴�ֻ�Ǿ�һ��ItsCustom����ʵ�ֵ�����,����ģ��ȥ��һЩ����������
void ItsCustom(LineQueueData *data, CustomParams *customParams)
{
	UInt16 redFlashSec = gStructBinfileCustom.sCountdownParams.redFlashSec;
	UInt8 isYellowFlash = GET_BIT(gStructBinfileCustom.sCountdownParams.option, 0);
	int i;

	customParams->addTimeToFirstPhase = gAddTimeToFirstPhase;
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (data->allChannels[i] == YELLOW && isYellowFlash)
			data->allChannels[i] = YELLOW_BLINK;
		if (data->allChannels[i] == RED && redFlashSec >= data->channelCountdown[i])
			data->allChannels[i] = RED_BLINK;
	}
	if (data->cycleTime == data->leftTime)	//ÿ�����ڸտ�ʼʱ����ͨ����ͻ������
		memcpy(gConflictChannel[0], gConflictChannel[1], sizeof(gConflictChannel[1]));
#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
	if (gStructBinfileConfigPara.sSpecialParams.iPhaseTakeOverSwtich)
		SendRunInfoTOBoard(data, gRunConfigPara);
#endif
	PrintRunInfo(data);
	//PrintVehCountDown(data);
}
