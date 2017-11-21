#include "hikmsg.h"
#include "HikConfig.h"
#include "calculate.h"


//��Ѱָ��������λ�ӵ�ǰ�׶ο�ʼ������ʱ���ڵĽ׶κ��Լ�����ʱ�����ĸ��λ
static UInt8 FindFollowPhaseEndStage(UInt8 followPhaseIndex, UInt8 curStage, UInt8 *endPhase)
{
	int i, s;
	UInt8 stageNum, ret;
	int motherPhaseNum = GET_PROTOCOL_PARAM(NUM_PHASE, gRunGbconfig->followPhaseTable[followPhaseIndex].motherPhaseNum);
	UInt8 motherPhase, tmpPhase = 0;
	UInt16 phaseOption;
	
	if (followPhaseIndex >= GET_PROTOCOL_PARAM(NUM_FOLLOW_PHASE, MAX_FOLLOW_PHASE_LIST_NUM)
		|| curStage == 0 || curStage > gCalInfo.maxStageNum
		|| endPhase == NULL)
		return 0;
	*endPhase = 0;
	ret = stageNum = curStage;
	for (s = 0; s < gCalInfo.maxStageNum; s++, stageNum = NEXT_STAGE(stageNum))
	{
		for (i = 0; i < motherPhaseNum; i++)
		{
			motherPhase = GET_PROTOCOL_PARAM(gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1][followPhaseIndex].nArrayMotherPhase[i], gRunGbconfig->followPhaseTable[followPhaseIndex].motherPhase[i]);
			if (motherPhase == 0 || motherPhase > MAX_PHASE_LIST_NUM) 
				break;
			phaseOption = GET_PROTOCOL_PARAM(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][motherPhase - 1].wPhaseOptions, gRunGbconfig->phaseTable[motherPhase - 1].phaseOption);
			if (GET_BIT(phaseOption, 0) == 0) //��λδʹ��
				continue;
			if (IsStageIncludePhase(stageNum, motherPhase))
			{
				tmpPhase = motherPhase;
				break;
			}
		}
		if (tmpPhase == 0)
			break;
		else if (tmpPhase > 0)
		{
			*endPhase = tmpPhase;
			tmpPhase = 0;
			ret = stageNum;
		}
	}
	if (s == gCalInfo.maxStageNum)
		ret = 0;
	return ret;
}
//��ȡָ��������λ����һ�׶ε��´ο�ʼ����ʱ�������ʼĸ��λ
static UInt8 GetFollowPhaseStartMotherPhase(UInt8 followPhaseIndex, UInt8 curStage)
{
	int i, s;
	int motherPhaseNum = GET_PROTOCOL_PARAM(NUM_PHASE, gRunGbconfig->followPhaseTable[followPhaseIndex].motherPhaseNum);
	UInt8 motherPhase;
	UInt16 phaseOption;
	
	for (s = NEXT_STAGE(curStage); s != curStage; s = NEXT_STAGE(s))
	{
		for (i = 0; i < motherPhaseNum; i++)
		{
			motherPhase = GET_PROTOCOL_PARAM(gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1][followPhaseIndex].nArrayMotherPhase[i], gRunGbconfig->followPhaseTable[followPhaseIndex].motherPhase[i]);
			if (motherPhase == 0 || motherPhase > MAX_PHASE_LIST_NUM) 
				break;
			phaseOption = GET_PROTOCOL_PARAM(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][motherPhase - 1].wPhaseOptions, gRunGbconfig->phaseTable[motherPhase - 1].phaseOption);
			if (GET_BIT(phaseOption, 0) == 0) //��λδʹ��
				continue;
			if (IsStageIncludePhase(s, motherPhase))
				return motherPhase;
		}
	}
	return 0;
}

void SetFollowPhaseInfo(PhaseInfo *phaseInfos, UInt8 curStage)
{
	int i, j;
	int maxFollowPhaseNum = GET_PROTOCOL_PARAM(NUM_FOLLOW_PHASE, MAX_FOLLOW_PHASE_LIST_NUM);
	int maxPhaseNum = GET_PROTOCOL_PARAM(NUM_PHASE, MAX_PHASE_LIST_NUM);
	int motherPhaseNum;
	LightStatus status, motherPhaseStatus;
	UInt8 motherPhase, firstMotherPhase;
	UInt16 phaseOption;
	UInt8 endStage = 0, startPhase = 0, endPhase = 0;
	
	for (i = 0; i < maxFollowPhaseNum; i++)
	{
		firstMotherPhase = GET_PROTOCOL_PARAM(gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1][i].nArrayMotherPhase[0], gRunGbconfig->followPhaseTable[i].motherPhase[0]);
		if (firstMotherPhase == 0)
			continue;
		status = RED;
		endStage = FindFollowPhaseEndStage(i, curStage, &endPhase);
		if (endStage == 0)	//˵���������ڸ�����λһֱ����
		{
			phaseInfos[i].followPhaseStatus = GREEN;
			phaseInfos[i].followPhaseLeftTime = gCalInfo.maxStageNum;
			continue;
		}
		motherPhaseNum = GET_PROTOCOL_PARAM(NUM_PHASE, gRunGbconfig->followPhaseTable[i].motherPhaseNum);
		for (j = 0; j < motherPhaseNum; j++)
		{
			motherPhase = GET_PROTOCOL_PARAM(gRunConfigPara->stFollowPhase[gCalInfo.phaseTableId - 1][i].nArrayMotherPhase[j], gRunGbconfig->followPhaseTable[i].motherPhase[j]);
			if (motherPhase == 0 || motherPhase > maxPhaseNum) 
				break;
			phaseOption = GET_PROTOCOL_PARAM(gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][motherPhase - 1].wPhaseOptions, gRunGbconfig->phaseTable[motherPhase - 1].phaseOption);
			if (GET_BIT(phaseOption, 0) == 0) //��λδʹ��
				continue;
			motherPhaseStatus = phaseInfos[motherPhase - 1].phaseStatus;
			if (motherPhaseStatus == GREEN || motherPhaseStatus == GREEN_BLINK 
				|| motherPhaseStatus == YELLOW || motherPhaseStatus == ALLRED)
			{	/*��ĸ��λ���ڷ���ʱ�����������λ����ʱ������ĸ��λ�뵱ǰĸ��λ��ȣ��������λ״̬���ĸ��λһ��*/
				if (endPhase == motherPhase)
				{
					status = motherPhaseStatus;
					break;
				}
				status = GREEN;
			}
		}
		phaseInfos[i].followPhaseStatus = (UInt8)status;
		if (status == GREEN_BLINK || status == YELLOW || status == ALLRED)
		{
			if (endPhase == 0 || endPhase > maxPhaseNum)
				continue;
			phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime;
		}
		else if (status == GREEN)
		{
			if (endPhase == 0 || endPhase > maxPhaseNum)
				continue;
			if (motherPhase == endPhase)
				phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime;
			else
			{
				phaseInfos[i].followPhaseLeftTime = phaseInfos[endPhase - 1].phaseLeftTime 
						+ GET_PROTOCOL_PARAM(gCalInfo.phaseTimes[endPhase - 1].greenTime 
							+ gCalInfo.phaseTimes[endPhase - 1].greenBlinkTime, 
							gCalInfo.stageInfos[endStage - 1].greenTime);
			}
		}
		else if (status == RED)
		{
			startPhase = GetFollowPhaseStartMotherPhase(i, curStage);
			if (startPhase == 0 || startPhase > maxPhaseNum)
				continue;
			phaseInfos[i].followPhaseLeftTime = phaseInfos[startPhase - 1].phaseLeftTime;
		}
	}				
}
