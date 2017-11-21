/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : strategy.c
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��11��27��
  ����޸�   :
  ��������   : ���Կ���ģ�飬��Ҫ���Ƶ���߼�
  �����б�   :
              GetActionID
              GetPhaseTurnAndSetLightTime
              GetTimeIntervalID
			  GetChannelStatusByTime
              InitChannelLightStatus
              IsPhaseInPhaseTurn
              RunCycle
              SetCurrentChannelStatus
              SetPhaseChannelArrayTotal
              SetPhaseGreenSignalRationItem
              SignalControllerRun
  �޸���ʷ   :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "HikConfig.h"
#include "light.h"
#include "Util.h"
#include "HikMsg.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HoursToMinutes(hours, minutes)	(hours*60 + minutes)    //��ʱ��ת���ɷ�
#define MAX_TIME_GAP	HoursToMinutes(24, 0)   //һ���е����ʱ���ֵ����24Сʱ
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern int msgid;
extern SignalControllerPara *gSignalControlpara;
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
/*�������ò�����ȫ��ָ�룬�����µ�������Ϣ�´���źſ���ģ���Ḵ��һ�����ò������ں���Ŀ����о�ֻ��ʹ����һ�����ݣ�
	���ֻ��һ�����ݵĻ�����ô���ܻ����һ�����¶�ȡ�����ļ���Ϣһ����ͬʱʹ��������Ϣ�����������Ϊ�����ͻ���ⱸ��һ������ */
SignalControllerPara *gConfigPara = NULL;//�źŻ����ò�������
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static unsigned short nCycleTime = 0;//���ڳ�
static lightTime_t gPhaseLightTime[NUM_PHASE];//��ǰ������ÿ����λ�ĵ��ʱ��



/*****************************************************************************
 �� �� ��  : SetPhaseGreenSignalRationItem
 ��������  : �������űȺų�ʼ����λ������ص����ݣ���Ҫ��ʼ����λ�����ű�ʱ
             ����̵�ʱ��
 �������  : unsigned short  nGreenSignalRatioID  ���űȺ�
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetPhaseGreenSignalRationItem(unsigned short  nGreenSignalRatioID)
{  
    int i = 0;
    PGreenSignalRationItem splitItem = NULL;
	PPhaseItem phaseItem = NULL;
	struct STRU_SignalTransEntry *entry = NULL;
	int timeGap = 0;

	memset(gPhaseLightTime,0,sizeof(gPhaseLightTime));
    for(i = 0 ; i < NUM_PHASE; i++)
    {
        splitItem = gConfigPara->stGreenSignalRation[nGreenSignalRatioID - 1] + i;
//      splitItem = &gConfigPara->stGreenSignalRation[nGreenSignalRatioID - 1][i];
		phaseItem = gConfigPara->stPhase + i;
		entry = gConfigPara->AscSignalTransTable + i;
		if (phaseItem->nPhaseID != i + 1)
			continue;

		gPhaseLightTime[i].greenTime = splitItem->nGreenSignalRationTime - entry->nGreenLightTime \
										- phaseItem->nYellowTime - phaseItem->nAllRedTime;
		gPhaseLightTime[i].greenBlinkTime = entry->nGreenLightTime;
		gPhaseLightTime[i].yellowTime = phaseItem->nYellowTime;
		gPhaseLightTime[i].allRedTime = phaseItem->nAllRedTime;
		/*�����λ������"�����Զ�����"��������˷���ʱ����������ʱ��֮�ʹ��ڻ�������λ���̵ƺ�����ʱ��֮�ͣ���ô���˷���ʱ����������ʱ����Ҫ������������λ���̵ƺ�����ʱ�������� */
		if ((BIT(phaseItem->wPhaseOptions, 8) == 1) 
			|| (gPhaseLightTime[i].greenTime + entry->nGreenLightTime < phaseItem->nPedestrianPassTime + phaseItem->nPedestrianClearTime)) 
		{					
			timeGap = gPhaseLightTime[i].greenTime + entry->nGreenLightTime - phaseItem->nPedestrianClearTime;
			if (timeGap > 0) 
			{
				gPhaseLightTime[i].pedestrianPassTime = (unsigned short)timeGap;
				gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
			} 
			else 
			{
				gPhaseLightTime[i].pedestrianPassTime = 0;
				gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime + timeGap;
			}
		} 
		else 
		{	
			gPhaseLightTime[i].pedestrianPassTime = phaseItem->nPedestrianPassTime;
			gPhaseLightTime[i].pedestrianClearTime = phaseItem->nPedestrianClearTime;
		}
        //log_debug("nPhaseId   %d   nTime   %d\n",nPhaseID,splitItem->nGreenSignalRationTime);
		//printf("DEBUG----- passTime: %d, clearTime: %d\n", gPhaseLightTime[i].pedestrianPassTime, gPhaseLightTime[i].pedestrianClearTime);
    }
}

/*****************************************************************************
 �� �� ��  : GetChannelStatusByTime
 ��������  : ��ȡ��ǰʱ����ڱ�ʹ��ͨ����״̬
 �������  : lightTime_t *times  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline lightStatus GetChannelStatusByTime(lightTime_t *times)
{
    if (times->greenTime)
    {
		times->greenTime--;
		return GREEN;
	}
	if (times->greenBlinkTime)
    {
		times->greenBlinkTime--;
		return GREEN_BLINK;
	}
	if (times->yellowTime)
    {
		times->yellowTime--;
		return YELLOW;
	}
	if (times->allRedTime)
    {
		times->allRedTime--;
		return RED;
	}
	return TURN_OFF;
}

/*****************************************************************************
 �� �� ��  : SetCurrentChannelStatus
 ��������  : ����Ҫ�Ľӿں��������ݵ�ǰ������λ����һ���������е���λ������ǰʱ��������ͨ��Ӧ����״̬
 �������  : lightStatus *allChannel	����ͨ����״̬  
             unsigned short nPhaseId     ��ǰ���е���λ
             unsigned short nextPhaseId       ��һ���������е���λ 
             PPhaseChannel nPhaseChannelArray	��λ��ͨ����Ӧ��ϵ������ָ��    
 �� �� ֵ  : �����ǰ��λ�����ű���������ô����false������͸��ݸ���״̬��ʱ��Ƭ������ǰ��λ��Ӧͨ����״̬
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
int SetCurrentChannelStatus(lightStatus *allChannel, unsigned short nPhaseId, unsigned short nextPhaseId, PPhaseChannel nPhaseChannelArray)
{    
    int i = 0, j = 0, ret = true;
    lightStatus status, tmp;
    unsigned short channelId = 0;
	lightTime_t *times = &gPhaseLightTime[nPhaseId - 1];
	PPhaseChannel phaseChannel = &nPhaseChannelArray[nPhaseId - 1];
	PPhaseChannel nextPhaseChannel = &nPhaseChannelArray[nextPhaseId - 1];
	
	//��������ͨ���ķ���ʱ������ʱ��
	unsigned short pedestrianPassTime = times->pedestrianPassTime;
	unsigned short pedestrianClearTime = times->pedestrianClearTime;

	status = GetChannelStatusByTime(times);
    if (status == TURN_OFF) 
    {	//���浱ǰ��λ�����ű�ʱ��������
		status = RED;	//�������λ����Ӧ��ͨ��ȫ����Ϊ��ɫ
		ret = false;
    }
    for(i = 0; i < phaseChannel->num; i++)//
    {
    	channelId = phaseChannel->channel[i];
		tmp = status;
        switch (gConfigPara->stChannel[channelId - 1].nControllerType) 
        {
			case MOTOR:	//����ǻ�����ͨ����ֱ�Ӱ������ű�ʱ���Ӧ��״̬
				break;
			case PEDESTRIAN:
			/*�ӱ����лָ���ǰʣ������˷���ʱ������ʱ�䣬��ֹһ����λ��Ӧ�������ͨ��ʱʱ�䱻��εݼ������� */
				times->pedestrianPassTime = pedestrianPassTime;
				times->pedestrianClearTime = pedestrianClearTime;				
			
				if (times->pedestrianPassTime) 
				{
					times->pedestrianPassTime--;
					tmp = GREEN;
					break;
				}
				if (times->pedestrianClearTime) 
				{
					times->pedestrianClearTime--;
					tmp = GREEN_BLINK;
					break;
				}
				tmp = RED;
				break;
			case FOLLOW:	//����Ǹ���ͨ������Ҫ���ݴ�ͨ���Ƿ�����һ��λ�Ծ����������������̵�ʱ��
				for (j = 0; j < nextPhaseChannel->num; j++) 
				{
					if (channelId == nextPhaseChannel->channel[j]) 
					{
						tmp = GREEN;
						break;
					}
				}
				break;
			default:	break;
        }

		allChannel[channelId - 1] = tmp;
    }

    return ret;
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
static unsigned char GetTimeIntervalID(struct tm *now)
{
    int i = 0;

    for(i = 0 ; i < NUM_SCHEDULE ; i++) 
    { //�����ǰʱ���ĳһ����ȱ�������򷵻ظõ��ȱ��ʱ�κ�
        if (BIT(gConfigPara->stPlanSchedule[i].month, now->tm_mon + 1) == 1) 
        {													//��Ϊtm_mon��Χ��[0,11]
            if (BIT(gConfigPara->stPlanSchedule[i].day, now->tm_mday) == 1) 
            {
			    return gConfigPara->stPlanSchedule[i].nTimeIntervalID;
			}
			if(BIT(gConfigPara->stPlanSchedule[i].week, (now->tm_wday == 0) ? 7 : now->tm_wday) == 1) 
			{												//tm_wday == 0����������
				return gConfigPara->stPlanSchedule[i].nTimeIntervalID;
			}
		}
    }

    return 0;
}



/*****************************************************************************
 �� �� ��  : GetActionID
 ��������  : ����ʱ�α�ID��������ʱ�α��µ�ʱ�Σ����ݵ�ǰʱ�䣬�ж�Ӧ������
             ��ʱ�Σ�����ȷ����ǰʱ����Ҫִ�еĶ���
 �������  : ��ǰ���е�ʱ�α��
 �������  : ��
 �� �� ֵ  : ��ǰ���еĶ�����

 �޸���ʷ      :
  1.��    ��   : 2014��11��20��
    ��    ��   : Jicky
    �޸�����   : �޸�������������������޸��˺���ʵ��

*****************************************************************************/
static unsigned char GetActionID(unsigned char nTimeIntervalID, struct tm *now)
{
    int i = 0;
	int nCurrentTime, nTempTime;
	int nTimeGap, oldTimeGap, index = -1;
    
	nCurrentTime = HoursToMinutes(now->tm_hour, now->tm_min);
	oldTimeGap = MAX_TIME_GAP;	//Ԥ������һ�����Ĳ�ֵ
	while (index == -1) 
	{	//ѭ���ҳ���ǰʱ����ʱ�α��в�ֵ��С��ʱ������Ӧ��actionID��Ϊ��ǰӦ�����е�actionID
		for(i= 0; i < NUM_TIME_INTERVAL_ID; i++) 
		{
			if(gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID  == 0) 
			{ //��˵����ʱ�ο���û�б�ʹ�ã�ֱ��continue��
				continue;
			}

			nTempTime = HoursToMinutes(gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeHour,gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].cStartTimeMinute);
			if (nCurrentTime == nTempTime) 
			{
				return gConfigPara->stTimeInterval[nTimeIntervalID - 1][i].nActionID;
			} 
			else if (nCurrentTime > nTempTime) 
			{
				nTimeGap = nCurrentTime - nTempTime;
				if (nTimeGap < oldTimeGap) 
				{
					oldTimeGap = nTimeGap;
					index = i;
				}
			}
		}
		
		if (oldTimeGap == MAX_TIME_GAP) 
		{ //˵����ǰʱ�䴦��ʱ�α�����С��λ��
			nCurrentTime += MAX_TIME_GAP;	//�ѵ�ǰʱ������24СʱȻ���ٴ�ѭ��
		}
	}

	return gConfigPara->stTimeInterval[nTimeIntervalID - 1][index].nActionID;
}


/*****************************************************************************
 �� �� ��  : GetPhaseTurnAndSetLightTime
 ��������  : ���ݵ�ǰʱ��һ�β�ѯ���ȱ�ʱ�α����������������ջ�ȡ��ǰӦ��ʹ�õ����űȱ������
			���������űȱ��ʱ�����λ���������ݽ��г�ʼ����ͬʱ����ȫ�ֱ�������nCycleTime
 �� �� ֵ  : ���ص�ǰʱ��Ӧʹ�õ������

 �޸���ʷ      :
  1.��    ��   : 2014��11��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
*****************************************************************************/
static unsigned short GetPhaseTurnAndSetLightTime(void)
{
    unsigned short  nTimeIntervalID = 0;
    unsigned short  nActionID = 0;
    unsigned short  nSchemeID = 0;
    unsigned short  nGreenSignalRatioID = 0;
    unsigned short  nPhaseTurnID = 0;
	time_t timep = time(NULL);
    struct tm now;

	localtime_r(&timep, &now);
    //start ...... ����һ�������ڵ���λ
    nTimeIntervalID = GetTimeIntervalID(&now);//���ݵ�ǰʱ�䣬�������ȱ��õ�ʱ�α�ID
    if(nTimeIntervalID == 0)
    {
        return 0;
    }

    nActionID = GetActionID(nTimeIntervalID, &now);//����ʱ�α�ID���õ�������ID
    if(nActionID == 0)
    {
        return 0;
    }

    nSchemeID = gConfigPara->stAction[nActionID - 1].nSchemeID;//���ݶ�����ID���õ�������ID
    if(nSchemeID == 0)
    {
        return 0;
    }
    
    nGreenSignalRatioID = gConfigPara->stScheme[nSchemeID - 1].nGreenSignalRatioID;//���ݷ�����ID���õ����űȱ�ID
    if(nGreenSignalRatioID == 0)
    {
        return 0;
    }

	nCycleTime = gConfigPara->stScheme[nSchemeID - 1].nCycleTime;
    nPhaseTurnID = gConfigPara->stScheme[nSchemeID - 1].nPhaseTurnID;//���ݷ�����ID���õ������ID
    if(nPhaseTurnID == 0)
    {
        return 0;
    }
    
    log_debug("%s  nTimeIntervalID  : %d nActionID  : %d  nSchemeID  :  %d  nGreenSignalRatioID :  %d  nPhaseTurnID  :  %d\n",__func__,nTimeIntervalID,nActionID,nSchemeID,nGreenSignalRatioID,nPhaseTurnID);
    //end ...... ����һ�������ڵ���λ

    SetPhaseGreenSignalRationItem(nGreenSignalRatioID);//���ݵ�ǰ��λID����ǰ�����űȱ�ID������λ���е����űȲ�����ֵ��

#ifndef ARM_PLATFORM
    sleep(2);
#endif
    return nPhaseTurnID;
}

/*****************************************************************************
 �� �� ��  : IsPhaseInPhaseTurn
 ��������  : �ж���λ�Ƿ���������
 �������  : unsigned short nPhaseTurn  �����
             unsigned short nPhaseId    ��λ��
 �� �� ֵ  : �����λ���������򷵻�true����������false
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int IsPhaseInPhaseTurn(unsigned short nPhaseTurn, unsigned short nPhaseId)
{
    int i;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		if (gConfigPara->stPhaseTurn[nPhaseTurn - 1][0].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][1].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][2].nTurnArray[i] == nPhaseId \
		||	gConfigPara->stPhaseTurn[nPhaseTurn - 1][3].nTurnArray[i] == nPhaseId)
			return true;
	}

	return false;
}

/*****************************************************************************
 �� �� ��  : SetPhaseChannelArrayTotal
 ��������  : �ҳ���ǰ��������λ��ͨ���Ķ�Ӧ��ϵ
 �������  : unsigned short nPhaseTurn         ��ǰ�����
             PPhaseChannel nPhaseChannelArray  ��λ��ͨ���Ķ�Ӧ��ϵ�ṹ��ָ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetPhaseChannelArrayTotal(unsigned short nPhaseTurn, PPhaseChannel nPhaseChannelArray)
{     
    int i = 0;
    int j = 0;
    unsigned short nTempId = 0, motherPhase = 0;
	PPhaseChannel phaseChannel = NULL;

	memset(nPhaseChannelArray, 0, sizeof(PhaseChannel) * NUM_PHASE);
    for(i = 0 ; i < NUM_CHANNEL; i++)
    {
        if(gConfigPara->stChannel[i].nChannelID != 0)
        {
            nTempId = gConfigPara->stChannel[i].nControllerID;
			phaseChannel = &nPhaseChannelArray[nTempId - 1];
            if((gConfigPara->stChannel[i].nControllerType == MOTOR) || (gConfigPara->stChannel[i].nControllerType == PEDESTRIAN))
            {
				if (BIT(gConfigPara->stPhase[nTempId - 1].wPhaseOptions, 0) == 0) 
				{//��λδʹ��
					continue;
				}
				if (IsPhaseInPhaseTurn(nPhaseTurn, nTempId) == true) 
				{
					phaseChannel->channel[phaseChannel->num++] = i + 1;
				}          
            } 
            else if(gConfigPara->stChannel[i].nControllerType == FOLLOW) 
            {
                for(j = 0 ; j < NUM_PHASE; j++)
                {
                	motherPhase = gConfigPara->stFollowPhase[nTempId - 1].nArrayMotherPhase[j];
                    if(motherPhase == 0) 
                    {
                        break;
                    }
					if (BIT(gConfigPara->stPhase[motherPhase - 1].wPhaseOptions, 0) == 0) 
					{//��λδʹ��
						continue;
					}
                    if(IsPhaseInPhaseTurn(nPhaseTurn, motherPhase) == true)
                    {
                    	phaseChannel = &nPhaseChannelArray[motherPhase - 1];
                        phaseChannel->channel[phaseChannel->num++] = i + 1;
                    }
                }
            }

        }

    }
}

/*****************************************************************************
 �� �� ��  : InitChannelLightStatus
 ��������  : ������λ��ͨ���Ķ�Ӧ��ϵ����ʼ��ÿ��ͨ����״̬
 �������  : lightStatus *allChannel           ����ͨ��״̬�Ľṹ��ָ��
             lightStatus status                ���õ�״̬��һ�㶼��ΪRED
             PPhaseChannel nPhaseChannelArray  ��λ��ͨ���Ķ�Ӧ��ϵ�ṹ��ָ��
 �� �� ֵ  :  
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void InitChannelLightStatus(lightStatus *allChannel, lightStatus status, PPhaseChannel nPhaseChannelArray)
{
	int i, j;
	PPhaseChannel phaseChannel = NULL;

	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		allChannel[i] = (gConfigPara->stChannel[i].nChannelID != 0) ? status : TURN_OFF;
	}
	
	if (nPhaseChannelArray == NULL)
		return;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		phaseChannel = &nPhaseChannelArray[i];
		for (j = 0; j < phaseChannel->num; j++) 
		{
			allChannel[phaseChannel->channel[j] - 1] = status;
		}
	}
}

/*****************************************************************************
 �� �� ��  : LightPerSecond
 ��������  : ����ͨ����״̬ѭ�����õ�Ƶ����飬���ն�ʱ��ɵ���Ϣ��Ȼ���͸�
             ���ģ�飬ÿ����Ӧ�ñ�����һ��
 �������  : lightStatus *allChannel  ��������ͨ��״̬������
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void LightPerSecond(lightStatus *allChannel)
{
	int i;
	unsigned short nOutLampArray[NUM_BOARD] = {0};
	struct msgbuf msg = {0, {0}};
	for (i = 0; i < LOOP_TIMES_PER_SECOND; i++)
	{
		ControlLampLight(allChannel, nOutLampArray);
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_TIMER_COMPLETE, 0) == -1) 
		{	//��鶨ʱ�Ƿ����
			log_error("error when yellowBlink, error info: %s\n", strerror(errno));
			i--;
			continue;
		}
		msg.mtype = MSG_LIGHT;
		memcpy(msg.mtext, nOutLampArray, sizeof(nOutLampArray));
		msgsnd(msgid, &msg, MSGSIZE, 0);
	}
}

/*****************************************************************************
 �� �� ��  : RunCycle
 ��������  : ͨ���������������ͨ����״̬�Լ���λ��ͨ���Ķ�Ӧ��ϵ������һ
             ������
 �������  : unsigned short nPhaseTurn           ��ǰҪ���е�����
             lightStatus *nCurrentChannelStatus  ��ŵ�ǰ1s������ͨ��״̬�Ľṹ��ָ��
             PPhaseChannel nPhaseChannelArray    ��λ��ͨ���Ķ�Ӧ��ϵ�ṹ��ָ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void RunCycle(unsigned short nPhaseTurn, 
					 lightStatus *nCurrentChannelStatus, 
					 PPhaseChannel nPhaseChannelArray)
{
	int i = 0, ring = 0;
    unsigned short  nPhaseId = 0, nextPhaseId = 0;
	
	while (nCycleTime--) 
	{	//һ�εݼ�1s
		for (ring = 0; ring < 4; ring++) 
		{	//ÿ1s��������ĸ����зֱ�ȡ����1s��Ӧ��ִ�е���λ����Ӧ��ͨ����״̬
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[i];
				nextPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[((i + 1) == NUM_PHASE) ? 0 : (i + 1)];
				if (nPhaseId == 0) 
				{
					break;
				}
				if (nextPhaseId == 0) 
				{
					nextPhaseId = gConfigPara->stPhaseTurn[nPhaseTurn - 1][ring].nTurnArray[0];
				}
				//������λ����ͨ��״̬����������ʧ��˵������λ�����ű�ʱ����꣬Ӧ�ü�����Ѱ��һ����λ���������ã�ֱ�����óɹ�Ϊֹ��
				if (SetCurrentChannelStatus(nCurrentChannelStatus, nPhaseId, nextPhaseId, nPhaseChannelArray) == true) 
				{
					break;
				}
			}
		}
		//����ǰ�����õ�ͨ��״̬��ͨ�����е��
		LightPerSecond(nCurrentChannelStatus);	//ÿ1s����һ��
	}
}


/*****************************************************************************
 �� �� ��  : SignalControllerRun
 ��������  : �źŻ����Ⱥ�������Ҫ���ݵ�ǰʱ���ȡӦʹ�õ������Ѿ���ʹ����λ�����ű�ʱ�䣬Ȼ�����ÿ����λ���ű�ʱ����ö�Ӧͨ��Ӧ����״̬��ʹ��ȫ�ֱ�������nCycleTimeÿ1sѭ��һ�Σ���ÿ1s��ȡһ�θ�ͨ����״̬Ȼ����е�ƴ���
 �� �� ֵ  : �������ȡʧ�ܱ㷵��false�������һֱ���е��ȿ����źŵ�
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *SignalControllerRun(void *arg)
{
	unsigned short nPhaseTurn = 0;
	lightStatus nCurrentChannelStatus[NUM_CHANNEL] = {TURN_OFF};//��ǰʱ����ÿ��ͨ����״̬
	PhaseChannel nPhaseChannelArray[NUM_PHASE];
    int i = 0;
	struct msgbuf msg = {MSG_CONFIG_UPDATE, {0}};

	//��������һ���ڴ棬��������������Ϣ
	if (gConfigPara == NULL) 
	{
		gConfigPara = (SignalControllerPara *)calloc(sizeof(SignalControllerPara), 1);
		if (gConfigPara == NULL) 
		{
			log_error("memory is not enough to store the backup config information\n");
			pthread_exit(NULL);
		}
	}
	while (msgrcv(msgid, &msg, MSGSIZE, MSG_CONFIG_UPDATE, 0) == -1) 
	{
		usleep(100000);	//��ʱ100msȻ���ٴζ�ȡ��Ϣ��ֱ����ȡ��Ϊֹ
	}
	//����������Ϣ
	memcpy(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
	//���Ͷ�ʱ����������Ϣ
	msg.mtype = MSG_TIMER_START;
	msgsnd(msgid, &msg, MSGSIZE, 0);

	log_debug("start to excute yellow blink status");
	//��������
	InitChannelLightStatus(nCurrentChannelStatus, YELLOW_BLINK, NULL);
	for (i = 0; i < gConfigPara->stUnitPara.nBootYellowLightTime; i++) 
	{
		LightPerSecond(nCurrentChannelStatus);
	}
	log_debug("start to excute all red status");
    //����ȫ��
	InitChannelLightStatus(nCurrentChannelStatus, RED, NULL);
	for (i = 0; i < gConfigPara->stUnitPara.nBootAllRedTime; i++) 
	{
		LightPerSecond(nCurrentChannelStatus);
	}

	log_debug("start to run cycle");
    while(1)
    {
		//������յ��µ�������Ϣ���ȱ�����ʹ��
		if (msgrcv(msgid, &msg, MSGSIZE, MSG_CONFIG_UPDATE, IPC_NOWAIT) != -1) 
		{
			memcpy(gConfigPara, gSignalControlpara, sizeof(SignalControllerPara));
		}
		//���ݵ�ǰʱ���ȡ���򣬲���ʼ������λ�����ű�
		nPhaseTurn = GetPhaseTurnAndSetLightTime();
		if (nPhaseTurn == 0) 
		{
			log_error("get phaseTurn error");
			continue;
		}
		//�ҳ�������ÿ����λ��ʹ�õ�ͨ����ȷ�����ǵĶ�Ӧ��ϵ
		SetPhaseChannelArrayTotal(nPhaseTurn, nPhaseChannelArray);
		//��ʼ����ʹ��ͨ����״̬Ϊȫ��
		InitChannelLightStatus(nCurrentChannelStatus, RED, nPhaseChannelArray);
		//ÿ����ѯһ������������
        RunCycle(nPhaseTurn, nCurrentChannelStatus, nPhaseChannelArray);
    }

    pthread_exit(NULL);
}

