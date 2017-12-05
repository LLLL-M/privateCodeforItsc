#pragma once

#include "cycle.h"

class Special : public Cycle
{
public:
	Special(const ControlRule &r) : Cycle(r)
	{
		cycleTime = 3;
		leftTime = 3;
	}
	//~Special();

	Special(const Special &) = default;

	virtual void Excute()
	{
		TscStatus st;
		switch (rule.ctrlMode)
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

	virtual Cycle *Clone()
	{
		Special *cycle = new Special(*this);
		return dynamic_cast<Cycle *>(cycle);
	}
};
