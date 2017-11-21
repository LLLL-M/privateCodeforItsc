#include <stdlib.h>
#include "its.h"
#include "calculate.h"

#define MAX_CYCLE_TIME	0xff	//最大周期时间


static int CalculateTransitionTime(int timeGap, int phaseOffset)
{ 
	static int transCycle = 0;		//过渡周期
	int totalTransitionTime = 0;	//总共需要过渡的时间
	int transitionTime = 0;			//每个周期需要过渡时间
	int excessTime = timeGap % gCalInfo.cycleTime - phaseOffset % gCalInfo.cycleTime;	//从当前时段起始时间运行到现在的多余时间

	if (excessTime == 0 || excessTime == 1)
		transCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//如果多余时间在5s之内，可以通过递减协调相位时间来达到过渡
		transCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		//周期时间减去多余时间，再加上相位差时间即是总共需要过渡时间
		totalTransitionTime = (excessTime > 0) ? (gCalInfo.cycleTime - excessTime) : abs(excessTime);
		if (transCycle == 0)	//如果没有配置过渡周期，则默认一个周期完成过渡
			transCycle = (gRunConfigPara->stUnitPara.byTransCycle == 0) ? 1 : gRunConfigPara->stUnitPara.byTransCycle;
		//当总共的过渡时间小于等于过渡周期或是小于等于5s时，便在一个周期内完成
		transitionTime = (totalTransitionTime <= transCycle || totalTransitionTime <= 5) 
						 ? totalTransitionTime 
						 : totalTransitionTime / transCycle;
		transCycle--;
	}
	if (gCalInfo.cycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//如果周期时间加上过渡时间大于最大周期时间0xff,则缩短过渡时间并增加过渡周期
		transitionTime = MAX_CYCLE_TIME - gCalInfo.cycleTime;
		transCycle++;
	}
	
	INFO("DEBUG totalTransitionTime = %d, transitionTime = %d", totalTransitionTime, transitionTime);
	return transitionTime;
}

//找出在相序中运行完协调相位所要经历的时间
static inline int FindPassTimeForCoordinatePhase(void)
{
	int i = 0;
	UInt8 nPhaseId, ring;
	int nPassTime = 0;
	
	if (gCalInfo.coordinatePhaseId == 0 || gCalInfo.coordinatePhaseId > NUM_PHASE)
		return 0;	//没有配置协调相位返回0

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
	
	/*当环中没有配置可进行过渡的协调相位时，则找出相序中运行完哪个相位后所经历的时间大于协调相位放行开始所经历的周期时间，则就在此相位上过渡周期时间 */
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
	UInt8 ringTransitionPhase[NUM_RING_COUNT] = {0};	//存放每个环中进行过渡的相位
	PhaseItem *item = &gRunConfigPara->stPhase[gCalInfo.phaseTableId - 1][gCalInfo.coordinatePhaseId - 1];
	
	if (item->nCircleID == 0 || item->nCircleID > NUM_RING_COUNT)
		return;
	ringTransitionPhase[item->nCircleID - 1] = gCalInfo.coordinatePhaseId;
	for (i = 0; i < NUM_PHASE; i++)
	{	//如果找到的第一个协调相位的并发相位也配置为协调相位，则在此并发相位上进行过渡
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
		gCalInfo.phaseTimes[nPhaseId - 1].splitTime += transitionTime;	//增加绿信比时间
		gCalInfo.phaseTimes[nPhaseId - 1].greenTime += transitionTime;	//增加绿灯时间
	}
}

void TransitionDeal(void)
{
	int phaseOffset = gRunConfigPara->stScheme[gCalInfo.schemeId - 1].nOffset;	//协调相位差
    int transitionTime = 0;	//每个周期需要过渡的时间
	int coordinatePhasePassTime = 0;	//运行完协调相位所要经历的时间
	int phaseBeginCycleTime = 0;		//相位开始放行时已经经历的周期时间
	
	//找出相序一个周期中运行完协调相位所要经历的时间
	coordinatePhasePassTime = FindPassTimeForCoordinatePhase();
	//如果配置了协调相位并且此协调相位在相序中存在也不是忽略相位，那便要进行过渡
	if (gCalInfo.coordinatePhaseId > 0 && gCalInfo.coordinatePhaseId <= NUM_PHASE && coordinatePhasePassTime > 0 
		&& GET_BIT(gCalInfo.isIgnorePhase, gCalInfo.coordinatePhaseId - 1) == FALSE) 
	{	
		phaseBeginCycleTime = coordinatePhasePassTime - gCalInfo.phaseTimes[gCalInfo.coordinatePhaseId - 1].splitTime;
		if (phaseBeginCycleTime > 0)	//说明协调相位不是相序中的第一相位
			phaseOffset += gCalInfo.cycleTime - phaseBeginCycleTime;//调整相位差的大小,加上一个周期中从运行协调相位开始剩余周期时间
    	transitionTime = CalculateTransitionTime(gCalInfo.timeGapSec, phaseOffset);
    	if (transitionTime != 0)
    	{
			gCalInfo.cycleTime += transitionTime;
    		AdjustPhaseTurnCycleTime(transitionTime, phaseBeginCycleTime);
    	}
	}
}
