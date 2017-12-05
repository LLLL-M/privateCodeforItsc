#pragma once

#include "channel.h"

#define LIGHT_PER_SEC       4   //每秒点灯次数,此值可改为2或8

namespace hik
{
	class device
	{
	private:
		
	public:
		device() {}
		
		~device() {}

		void setlamps(const ChannelArray &lamps) {}
		void setlamps(UInt8 ctrlMode) {}
		void light() {}
	};
}
