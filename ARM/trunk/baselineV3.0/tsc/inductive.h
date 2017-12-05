#pragma once

#include "cycle.h"
#include "singleton.h"
#include "detector.h"

class Inductive : public Cycle
{
public:
	Inductive(const ControlRule &r, const std::vector<std::vector<UInt8>> &turn) : Cycle(r, turn), detectorArray(Singleton<DetectorArray>::GetInstance())
	{

	}
	//~Inductive();
	Inductive(const Inductive &) = default;
	
	virtual Cycle *Clone()
	{
		return dynamic_cast<Cycle *>(new Inductive(*this));
	}

	virtual void Excute()
	{
		CheckExtend();
		Cycle::Excute();
	}

protected:
	DetectorArray &detectorArray;
	
	void CheckExtend()
	{
		std::bitset<MAX_VEHDETECTOR_NUM> vehEnter = detectorArray.GetEnter(time(nullptr) - 1);

		for (auto &ring : ringTable)
		{
			Phase &phase = phaseTable[ring.CurPhaseId()];
			if (phase.WindowPeriod() && (vehEnter & phase.vehDetectorBits).any())
				phase.ExtendGreen(phase.unitExtendTime);
		}
	}
};