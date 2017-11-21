#pragma once

#include "cycle.h"

class InitCycle : public Cycle
{
private:
	ColorStepVector csv;
	bool once;
public:
	InitCycle(const std::map<int, TscChannel> &channels) : Cycle({LOCAL_CONTROL, SYSTEM_MODE}, channels)
	{
		once = true;
	}
	InitCycle(const InitCycle &) = default;
	
	void Add(UInt16 time, TscStatus st)
	{
		csv.Add(time, st);
		cycleTime += time;
		leftTime += time;
	}

	void Excute()
	{
		if (leftTime > 0)
			leftTime--;
		csv.Move(1);
		ColorStep cs = csv.Current();
		for (auto &channel : channelTable)
		{
			if (channel.type != CHANNEL_TYPE_UNUSED)
				channel.status = (channel.specifyStatus != INVALID) ? channel.specifyStatus : cs.status;
		}
	}

	Cycle *Clone()
	{
		if (!once)
			return nullptr;
		once = false;
		InitCycle *cycle = new InitCycle(*this);
		return dynamic_cast<Cycle *>(cycle);
	}
	
};
