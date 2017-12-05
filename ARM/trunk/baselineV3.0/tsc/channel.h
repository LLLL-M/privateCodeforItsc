#pragma once

#include <array>
#include "hik.h"
#include "common.h"

class Channel
{
private:
	
public:
	/*const */UInt8 id = 0;
	/*const */UInt8 type = 0;
	/*const */UInt8 countdownId = 0;
	/*const */TscStatus specifyStatus = INVALID;
	TscStatus status = INVALID;
	UInt16 countdown = 0;

	//Channel() = default;

	bool inuse() const
	{
		return (type != CHANNEL_TYPE_UNUSED);
	}

	bool isped() const
	{
		return (type == CHANNEL_TYPE_PED);
	}
	
protected:
};

typedef std::array<Channel, MAX_CHANNEL_NUM> ChannelArray;
