#include "coordinate.h"

int Coordinate::CalTransitionTime(int phaseOffset)
{
	time_t cur = time(nullptr);
	struct tm now = {0};
	#if(defined(__WINDOWS__))
	localtime_s(&now, &cur);
	#else
	localtime_r(&cur, &now);
	#endif

	int totalTransitionTime = 0;	//总共需要过渡的时间
	int transitionTime = 0;			//每个周期需要过渡时间
	int timeGapSec = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
	int excessTime = (timeGapSec - phaseOffset) % schemeCycleTime;//从当前时段起始时间运行到现在的多余时间
	if (excessTime == 0 || excessTime == 1)
		leftTransCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//如果多余时间在5s之内，可以通过递减协调相位时间来达到过渡
		leftTransCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{	//周期时间减去多余时间，再加上相位差时间即是总共需要过渡时间
		totalTransitionTime = (excessTime > 0) ? (schemeCycleTime - excessTime) : (-excessTime);
		if (leftTransCycle == 0)
			leftTransCycle = transCycle;
		if (totalTransitionTime <= leftTransCycle || totalTransitionTime <= 5) 
		{	//当总共的过渡时间小于等于过渡周期或是小于等于5s时，便在一个周期内完成
			transitionTime = totalTransitionTime;
			leftTransCycle = 0;
		}
		else
		{
			transitionTime = totalTransitionTime / leftTransCycle;
			leftTransCycle--;
		}
	}
#if 0
#define MAX_CYCLE_TIME	0xff	//最大周期时间
	if (schemeCycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//如果周期时间加上过渡时间大于最大周期时间0xff,则缩短过渡时间并增加过渡周期
		transitionTime = MAX_CYCLE_TIME - schemeCycleTime;
		leftTransCycle++;
	}
#endif
	return transitionTime;
}

void Coordinate::AssignTransitionTime(Cycle &cycle, int transitionTime)
{
	if (transitionTime <= 0 || cycle.stages.empty())
		return;

	UInt16 avg = transitionTime / cycle.stages.size();
	UInt16 left = transitionTime % cycle.stages.size();
	auto coordStage = cycle.stages.begin();
	for (auto i = coordStage; i != cycle.stages.end(); i++)
	{	/*把过渡时间平均分给每个阶段包含的相位*/
		for (auto &phaseId : *i)
		{
			cycle.phaseTable[phaseId].Extend(avg);
			if (phaseId == coordPhase)
				coordStage = i;
		}
	}
	if (left > 0)
	{	//把剩余的时间全部增加到协调相位对应的那个阶段
		for (auto &phaseId : *coordStage)
			cycle.phaseTable[phaseId].Extend(left);
	}
}

void Coordinate::Transition(Cycle &cycle)
{
	int phaseOffset = 0;
	if (!CalPhaseOffset(phaseOffset))
		return;
	int transitionTime = CalTransitionTime(phaseOffset);
	AssignTransitionTime(cycle, transitionTime);
}


bool CoordinateGreen::CalPhaseOffset(int &phaseOffset)
{
	auto it = phaseTable.find(coordPhase);
	if (transCycle == 0 || schemeCycleTime == 0 || it == phaseTable.end() 
		|| it->second.ring == 0 || it->second.ring > ringTable.size())
		return false;

	Ring &ring = ringTable[it->second.ring - 1];
	phaseOffset = offset;
	for (auto &phaseId : ring.turn)
	{	//用以处理协调相位不是首相位的情况，此时相位差需要重新计算
		if (phaseId == coordPhase)
			break;
		phaseOffset -= phaseTable[phaseId].Total();
	}
	return true;
}

bool CoordinateRed::CalPhaseOffset(int &phaseOffset)
{
	auto it = phaseTable.find(coordPhase);
	if (transCycle == 0 || schemeCycleTime == 0 || it == phaseTable.end() 
		|| it->second.ring == 0 || it->second.ring > ringTable.size())
		return false;

	Ring &ring = ringTable[it->second.ring - 1];
	phaseOffset = offset - it->second.NonAllRed();
	for (auto &phaseId : ring.turn)
	{	//用以处理协调相位不是首相位的情况，此时相位差需要重新计算
		if (phaseId == coordPhase)
			break;
		phaseOffset -= phaseTable[phaseId].Total();
	}
	return true;
}
