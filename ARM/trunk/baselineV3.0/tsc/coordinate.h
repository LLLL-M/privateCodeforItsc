#pragma once

#include "cycle.h"

class Coordinate
{
private:
	int CalTransitionTime(int phaseOffset);

	void AssignTransitionTime(Cycle &cycle, int transitionTime);
	
public:
	Coordinate(UInt16 _schemeCycleTime, UInt16 _offset, 
		UInt8 _transCycle, UInt8 _coordPhase) : schemeCycleTime(_schemeCycleTime),\
												offset(_offset),\
												transCycle(_transCycle),\
												coordPhase(_coordPhase)
	{
		leftTransCycle = 0;
	}
	//~Coordinate();
	Coordinate(const Coordinate &) = default;

protected:
	const int schemeCycleTime;		//配置方案的周期时间
	const int offset;				//配置方案的相位差
	const UInt8  transCycle;     		//协调过渡周期
	const UInt8  coordPhase;			//协调相位
	int leftTransCycle;					//剩余的过渡周期

	virtual bool CalPhaseOffset(int &phaseOffset)
	{
		phaseOffset = offset;
		return true;
	}

	void Transition(Cycle &cycle);
};

class CoordinateGreen : public Cycle, public Coordinate
{
public:
	CoordinateGreen(const ControlRule &r, const std::vector<std::vector<UInt8>> &turn, 
		UInt16 _schemeCycleTime, UInt16 _offset, 
		UInt8 _transCycle, UInt8 _coordPhase) : Cycle(r, turn), Coordinate(_schemeCycleTime, _offset, _transCycle, _coordPhase)
	{
	}
	//~CoordinateGreen();
	CoordinateGreen(const CoordinateGreen &) = default;
	
	virtual Cycle *Clone()
	{
		Cycle *cycle = dynamic_cast<Cycle *>(new CoordinateGreen(*this));
		if (cycle != nullptr)
			Transition(*cycle);
		return cycle;
	}

protected:
	virtual bool CalPhaseOffset(int &phaseOffset);
};

class CoordinateRed : public Cycle, public Coordinate
{
public:
	CoordinateRed(const ControlRule &r, const std::vector<std::vector<UInt8>> &turn, 
		UInt16 _schemeCycleTime, UInt16 _offset, 
		UInt8 _transCycle, UInt8 _coordPhase) : Cycle(r, turn), Coordinate(_schemeCycleTime, _offset, _transCycle, _coordPhase)
	{
	}
	//~CoordinateRed();
	CoordinateRed(const CoordinateRed &) = default;
	
	virtual Cycle *Clone()
	{
		Cycle *cycle = dynamic_cast<Cycle *>(new CoordinateRed(*this));
		if (cycle != nullptr)
			Transition(*cycle);
		return cycle;
	}

protected:
	virtual bool CalPhaseOffset(int &phaseOffset);
};