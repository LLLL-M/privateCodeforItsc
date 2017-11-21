#ifndef __SINGLE_SPOT_H__
#define __SINGLE_SPOT_H__

#include "cycle.h"
#include "detector.h"
#include "singleton.h"

class SingleSpot : public Cycle
{
private:
	static array<UInt16, MAX_PHASE_NUM> phaseExtend;
	//array<UInt16, MAX_PHASE_NUM> SingleSpot::phaseExtend = {0};
	DetectorArray &detectorArray;

	SchemeInfo & Adjust(SchemeInfo &si)
	{
		for (auto &p : si.phaseturn)
		{
			if (p.phaseId == 0 || p.phaseId > MAX_PHASE_NUM || phaseExtend[p.phaseId - 1] == 0)
				continue;
			UInt16 ext = phaseExtend[p.phaseId - 1];
			p.splitTime += ext;
			if (p.motor.size() > 0)
				p.motor[0].stepTime += ext;
			if (p.pedestrian.size() > 0)
				p.pedestrian[0].stepTime += ext;
			si.cycleTime += ext;
		}
		return si;
	}
	
	void ExtendNextCycle(Phase &p)
	{
		auto &ext = phaseExtend[p.phaseId - 1];
		if (p.phaseId == 0 || p.phaseId > MAX_PHASE_NUM || ext >= p.maxExtendTime)
			return;
		if (ext + p.unitExtendTime > p.maxExtendTime)
		{
			INFO("phase %d extend %d sec in next cycle!", p.phaseId, p.maxExtendTime - ext);
			ext = p.maxExtendTime;
		}
		else
		{
			ext += p.unitExtendTime;
			INFO("phase %d extend %d sec in next cycle!", p.phaseId, p.unitExtendTime);
		}
	}
	
	void ReduceNextCycle(Phase &p)
	{
		auto &ext = phaseExtend[p.phaseId - 1];
		if (p.phaseId == 0 || p.phaseId > MAX_PHASE_NUM || ext == 0)
			return;
		if (ext > p.unitExtendTime)
		{
			INFO("phase %d reduce %d sec in next cycle!", p.phaseId, p.unitExtendTime);
			ext -= p.unitExtendTime;
		}
		else
		{
			INFO("phase %d reduce %d sec in next cycle!", p.phaseId, ext);
			ext = 0;
		}
	}
	
public:
	SingleSpot(SchemeInfo &si) : Cycle(Adjust(si)), detectorArray(Singleton<DetectorArray>::GetInstance())
	{
		
	}

	static void Init()
	{
		phaseExtend.fill(0);
	}
	
	virtual void Excute()
	{
		Cycle::Excute();
		Phase &p = phaseTable[curPhase - 1];
		if (p.WindowPeriodEnd())
		{	//窗口期结束时
			if (detectorArray.GetEnter(time(nullptr) - p.unitExtendTime, p.unitExtendTime) & p.vehDetectorBits)
				ExtendNextCycle(p);	//如果相位在窗口期的时间内检测到所对应的的车检器有过车，则下周期此相位延长一个单位延长绿
			else
				ReduceNextCycle(p);
		}
	}
protected:
};

#endif
