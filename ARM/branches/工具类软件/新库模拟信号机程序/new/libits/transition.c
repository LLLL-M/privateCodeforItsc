#include <stdlib.h>
#include "its.h"
#include "calculate.h"

#define MAX_CYCLE_TIME	0xff	//�������ʱ��


static int CalculateTransitionTime(int timeGap, int phaseOffset)
{ 
	static int transCycle = 0;		//��������
	int totalTransitionTime = 0;	//�ܹ���Ҫ���ɵ�ʱ��
	int transitionTime = 0;			//ÿ��������Ҫ����ʱ��
	int excessTime = timeGap % gCalInfo.cycleTime - phaseOffset % gCalInfo.cycleTime;	//�ӵ�ǰʱ����ʼʱ�����е����ڵĶ���ʱ��

	if (excessTime == 0 || excessTime == 1)
		transCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//�������ʱ����5s֮�ڣ�����ͨ���ݼ�Э����λʱ�����ﵽ����
		transCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		//����ʱ���ȥ����ʱ�䣬�ټ�����λ��ʱ�伴���ܹ���Ҫ����ʱ��
		totalTransitionTime = (excessTime > 0) ? (gCalInfo.cycleTime - excessTime) : abs(excessTime);
		if (transCycle == 0)	//���û�����ù������ڣ���Ĭ��һ��������ɹ���
			transCycle = (gRunConfigPara->stUnitPara.byTransCycle == 0) ? 1 : gRunConfigPara->stUnitPara.byTransCycle;
		//���ܹ��Ĺ���ʱ��С�ڵ��ڹ������ڻ���С�ڵ���5sʱ������һ�����������
		transitionTime = (totalTransitionTime <= transCycle || totalTransitionTime <= 5) 
						 ? totalTransitionTime 
						 : totalTransitionTime / transCycle;
		transCycle--;
	}
	if (gCalInfo.cycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//�������ʱ����Ϲ���ʱ������������ʱ��0xff,�����̹���ʱ�䲢���ӹ�������
		transitionTime = MAX_CYCLE_TIME - gCalInfo.cycleTime;
		transCycle++;
	}
	
	INFO("DEBUG totalTransitionTime = %d, transitionTime = %d", totalTransitionTime, transitionTime);
	return transitionTime;
}

//�ҳ���������������Э����λ��Ҫ������ʱ��
static inline int FindPassTimeForCoordinatePhase(void)
{
	int i = 0;
	UInt8 nPhaseId, ring;
	int nPassTime = 0;
	
	if (gCalInfo.coordinatePhaseId == 0 || gCalInfo.coordinatePhaseId > NUM_PHASE)
		return 0;	//û������Э����λ����0

	ring = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][gCalInfo.coordinatePhaseId - 1].nCircleID - 1;
	for (i = 0; i < NUM_PHASE; i++) 
	{
		nPhaseId = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring].nTurnArray[i];
		if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
			break;
		nPassTime += gCalInfo.phaseTimes[nPhaseId - 1].splitTime;
		if (nPhaseId == gCalInfo.coordinatePhaseId)
			break;
	}
	return nPassTime;
}

static inline UInt8 FindRingTransitionPhase(UInt8 ring, int phaseBeginCycleTime)
{
	int i, passTime = 0;
	PhaseTimeInfo *phaseTimes = gCalInfo.phaseTimes;
	UInt8 nPhaseId = 0;
	
	/*������û�����ÿɽ��й��ɵ�Э����λʱ�����ҳ��������������ĸ���λ����������ʱ�����Э����λ���п�ʼ������������ʱ�䣬����ڴ���λ�Ϲ�������ʱ�� */
	for (i = 0; i < NUM_PHASE; i++) 
	{
		nPhaseId = gRunConfigPara->stPhaseTurn[gCalInfo.phaseTurnId - 1][ring - 1].nTurnArray[i];
		if (nPhaseId == 0)
			break;
		passTime += phaseTimes[nPhaseId - 1].splitTime;
		if (passTime > phaseBeginCycleTime)
			break;
	}
	return nPhaseId;
}

static void AdjustPhaseTurnCycleTime(int transitionTime, int phaseBeginCycleTime)
{
	int i = 0, ring = 0;
	UInt8 nPhaseId = 0;
	UInt8 ringTransitionPhase[NUM_RING_COUNT] = {0};	//���ÿ�����н��й��ɵ���λ
	PhaseItem *item = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][gCalInfo.coordinatePhaseId - 1];
	
	if (item->nCircleID == 0 || item->nCircleID > NUM_RING_COUNT)
		return;
	ringTransitionPhase[item->nCircleID - 1] = gCalInfo.coordinatePhaseId;
	for (i = 0; i < NUM_PHASE; i++)
	{	//����ҵ��ĵ�һ��Э����λ�Ĳ�����λҲ����ΪЭ����λ�����ڴ˲�����λ�Ͻ��й���
		nPhaseId = item->byPhaseConcurrency[i];
		if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
			break;
		ring = gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][nPhaseId - 1].nCircleID;
		if (GET_BIT(gCalInfo.isCoordinatePhase, nPhaseId - 1) == TRUE
			&& ring > 0 && ring <= NUM_RING_COUNT)
			ringTransitionPhase[ring - 1] = nPhaseId;
	}
	for (i = 0; i < NUM_RING_COUNT; i++) 
	{	
		nPhaseId = (ringTransitionPhase[i] == 0) 
					? FindRingTransitionPhase(i + 1, phaseBeginCycleTime) 
					: ringTransitionPhase[i];
		if (nPhaseId == 0 || nPhaseId > NUM_PHASE)
			continue;
		gCalInfo.phaseTimes[nPhaseId - 1].splitTime += transitionTime;	//�������ű�ʱ��
		gCalInfo.phaseTimes[nPhaseId - 1].greenTime += transitionTime;	//�����̵�ʱ��
	}
}

void TransitionDeal(void)
{
	int phaseOffset = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nOffset;	//Э����λ��
    int transitionTime = 0;	//ÿ��������Ҫ���ɵ�ʱ��
	int coordinatePhasePassTime = 0;	//������Э����λ��Ҫ������ʱ��
	int phaseBeginCycleTime = 0;		//��λ��ʼ����ʱ�Ѿ�����������ʱ��
	
	//�ҳ�����һ��������������Э����λ��Ҫ������ʱ��
	coordinatePhasePassTime = FindPassTimeForCoordinatePhase();
	//���������Э����λ���Ҵ�Э����λ�������д���Ҳ���Ǻ�����λ���Ǳ�Ҫ���й���
	if (gCalInfo.coordinatePhaseId > 0 && gCalInfo.coordinatePhaseId <= NUM_PHASE && coordinatePhasePassTime > 0 
		&& GET_BIT(gCalInfo.isIgnorePhase, gCalInfo.coordinatePhaseId - 1) == FALSE) 
	{	
		phaseBeginCycleTime = coordinatePhasePassTime - gCalInfo.phaseTimes[gCalInfo.coordinatePhaseId - 1].splitTime;
		if (phaseBeginCycleTime > 0)	//˵��Э����λ���������еĵ�һ��λ
			phaseOffset += gCalInfo.cycleTime - phaseBeginCycleTime;//������λ��Ĵ�С,����һ�������д�����Э����λ��ʼʣ������ʱ��
    	transitionTime = CalculateTransitionTime(gCalInfo.timeGapSec, phaseOffset);
    	if (transitionTime != 0)
    	{
			gCalInfo.cycleTime += transitionTime;
    		AdjustPhaseTurnCycleTime(transitionTime, phaseBeginCycleTime);
    	}
	}
}
