#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include "hik.h"
#include "HikConfig.h"
#include "gb.h"
#include "its.h"

#ifndef MAX_STAGE_NUM
#define MAX_STAGE_NUM	16
#endif
#ifndef MAX_PHASE_LIST_NUM
#define MAX_PHASE_LIST_NUM	16
#endif

extern SignalControllerPara *gRunConfigPara;
extern GbConfig *gRunGbconfig;
#define GET_PROTOCOL_PARAM(NTCIP_PARAM, GB2007_PARAM) ({\
	(gRunConfigPara != NULL) ? (NTCIP_PARAM) : \
		((gRunGbconfig != NULL) ? (GB2007_PARAM) : (0));})

typedef struct _PhaseTimeInfo
{
	UInt8 splitTime;
	UInt8 greenTime;
	UInt8 greenBlinkTime;
	UInt8 yellowTime;
	UInt8 allRedTime;
	//����
    UInt8 pedestrianPassTime;//���˷���ʱ��
    UInt8 pedestrianClearTime;//�������ʱ��
	UInt8 maxExtendGreen;	//�������ӳ����̵�ʱ��
} PhaseTimeInfo;

typedef struct _StageInfo
{
	UInt8 runTime;					//�׶�����ʱ��
	UInt8 greenTime;
	Boolean isBarrierStart;			//�׶��Ƿ���������ʼ
	Boolean isBarrierEnd;			//�׶��Ƿ������Ͻ���
	UInt8 includeNum;				//�׶ΰ�����λ�ĸ���
	UInt8 includePhases[MAX_PHASE_LIST_NUM];			//�׶ΰ�����λ
} StageInfo;

typedef struct _CalInfo
{
	UInt8 timeIntervalId;					//ʱ�α��
	UInt8 actionId;							//������
	UInt8 schemeId;							//������
	UInt8 splitId;							//���űȺ�
	UInt8 phaseTurnId;						//�����
	UInt8 phaseTableId;						//��λ���
	UInt8 channelTableId;					//ͨ�����
	UInt8 cycleTime;						//����ʱ��
	
	UInt8 coordinatePhaseId;				//Э����λ
	UInt16 isCoordinatePhase;				//�Ƿ���Э����λ,ÿbit����һ����λ,0:����,1:��
	UInt8 phaseOffset;						//��λ��
	int timeGapSec;							//��ǰʱ����ʱ����ʼʱ����λΪs

	UInt16 isIgnorePhase;					//�Ƿ��Ǻ�����λ,ÿbit����һ����λ,0:����,1:��
	
	PhaseTimeInfo phaseTimes[NUM_PHASE];	//��λʱ����ص���Ϣ
	
	UInt8 maxStageNum;						//���׶κ�
	StageInfo stageInfos[MAX_STAGE_NUM];	//�׶������Ϣ
	
	UInt8 includeNums[NUM_PHASE];						//ÿ����λ�������Ľ׶κŸ���
	UInt8 phaseIncludeStage[NUM_PHASE][MAX_STAGE_NUM];	//��λ�������Ľ׶κ�
} CalInfo;

extern CalInfo gCalInfo;

static inline void SetPedestrianTime(int pedAutoRequestFlag, PhaseTimeInfo *times, 
					UInt8 pedestrianPassTime, UInt8 pedestrianClearTime)
{
	int timeGap = 0;
	
	/*�����λ������"�����Զ�����"��������˷���ʱ����������ʱ��֮�ʹ��ڻ�������λ���̵ƺ�����ʱ��֮�ͣ���ô���˷���ʱ����������ʱ����Ҫ������������λ���̵ƺ�����ʱ�������� */
	if ((pedAutoRequestFlag == 1) 
		|| (times->greenTime + times->greenBlinkTime < pedestrianPassTime + pedestrianClearTime)) 
	{					
		timeGap = times->greenTime + times->greenBlinkTime - pedestrianClearTime;
		if (timeGap > 0) 
		{
			times->pedestrianPassTime = (UInt8)timeGap;
			times->pedestrianClearTime = pedestrianClearTime;
		} 
		else 
		{
			times->pedestrianPassTime = 0;
			times->pedestrianClearTime = pedestrianClearTime + timeGap;
		}
	} 
	else 
	{	
		times->pedestrianPassTime = pedestrianPassTime;
		times->pedestrianClearTime = pedestrianClearTime;
	}
}

/*****************************************************************************
 �� �� ��  : GetPhaseStatusByTime
 ��������  : ��ȡ��ǰʱ����ڱ�ʹ��ͨ����״̬
 �������  : PhaseTimeInfo *times  
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��11��27��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline LightStatus GetPhaseStatusByTime(PhaseTimeInfo *times)
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
		return ALLRED;
	}
	return RED;
}

static inline Boolean IsPhaseIncludeStage(int phaseId, int stageNum)
{
	int i;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_LIST_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	for (i = 0; i < gCalInfo.includeNums[phaseId - 1]; i++)
	{
		if (stageNum == gCalInfo.phaseIncludeStage[phaseId - 1][i])
			return TRUE;
	}
	return FALSE;
}

static inline Boolean IsStageIncludePhase(int stageNum, int phaseId)
{
	int i;
	StageInfo *stageinfo = NULL;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_LIST_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	stageinfo = &gCalInfo.stageInfos[stageNum - 1];
	for (i = 0; i < stageinfo->includeNum; i++)
	{
		if (phaseId == stageinfo->includePhases[i])
			return TRUE;
	}
	return FALSE;
}

#define NEXT_STAGE(_CUR)	((_CUR) % gCalInfo.maxStageNum + 1)

extern void SetFollowPhaseInfo(PhaseInfo *phaseInfos, UInt8 curStage);

#endif
