#pragma once

#include "cycle.h"

class Special : public Cycle
{
public:
	Special(UInt8 ctrlMode, const std::map<int, TscChannel> &channels) : Cycle({LOCAL_CONTROL, ctrlMode, 0, 0}, channels)
	{
		cycleTime = 3;
		leftTime = 3;
		TscStatus st;
		switch (ctrlMode)
		{
			case TURNOFF_MODE: st = TURN_OFF; break;
			case YELLOWBLINK_MODE: st = YELLOW_BLINK; break;
			case ALLRED_MODE: st = ALLRED; break;
			default: st = INVALID;
		}
		for (auto &channel : channelTable)
		{
			if (channel.type != CHANNEL_TYPE_UNUSED)
				channel.status = (channel.specifyStatus != INVALID) ? channel.specifyStatus : st;
		}
	}

	Special(const Special &) = default;

	void Excute()
	{
		/*此虚函数必须实现，以保证不会调用Cycle::Excute()，并且leftTime!=1也因此周期永不结束*/
	}
	//~Special();
	Cycle *Clone()
	{
		Special *cycle = new Special(*this);
		return dynamic_cast<Cycle *>(cycle);
	}
};
