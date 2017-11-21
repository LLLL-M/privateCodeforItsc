#pragma once

#include <set>
#include <algorithm>
#include "colorstep.h"

class Channel
{
private:
	struct ChannelInfo
	{
		TscStatus status;
		UInt16 countdown;
		UInt16 phaseLeft;
		UInt16 phaseGreen;
		ChannelInfo(TscStatus st, UInt16 cd, UInt16 left, UInt16 nonGreen)
		{
			status = st;
			countdown = cd;
			phaseLeft = left;
			phaseGreen = nonGreen;
		}
		/*此比较函数主要为map或set等有序容器排序存储使用*/
		bool operator<(const ChannelInfo & right) const
		{
			if (status == GREEN || status == GREEN_BLINK || status == YELLOW || status == ALLRED)
				return true;
			if (right.status == GREEN || right.status == GREEN_BLINK || right.status == YELLOW || right.status == ALLRED)
				return false;
			if (status == RED && right.status == RED)
				return countdown < right.countdown;
			return false;
		}
	};
	std::set<ChannelInfo> infos;
public:
	/*const */UInt8 id = 0;
	/*const */UInt8 type = 0;
	/*const */UInt8 countdownId = 0;
	/*const */TscStatus specifyStatus = INVALID;
	TscStatus status = INVALID;
	UInt16 countdown = 0;

	void AddInfo(const ColorStep &cs, UInt16 left, UInt16 green)
	{
		infos.emplace(cs.status, cs.countdown, left, green);
		//INFO("****** channel %d ******", id);
		//cs.Print();
	}
	void CalInfo()
	{
		auto Set = [this](TscStatus st, UInt16 cd){
			status = st;
			countdown = cd;
			infos.clear();
		};
		if (specifyStatus != INVALID)
		{
			Set(specifyStatus, 0);
			return;
		}
		if (infos.empty())
		{
			Set(INVALID, 0);
			return;
		}
		auto it = infos.begin();
		auto first = *it;
		if (infos.size() == 1 || first.status == RED)
		{
			Set(first.status, first.countdown);
			return;
		}
		find_if(++it, infos.end(), [&first, this](const ChannelInfo &ci)->bool{
			if (ci.status == GREEN || ci.status == GREEN_BLINK || ci.status == YELLOW || ci.status == ALLRED)
				return true;
			if (ci.countdown > first.phaseLeft)
			{
				if (first.status == ALLRED)
					first.countdown = ci.countdown;
				return true;
			}
			first.countdown = ci.countdown + ci.phaseGreen;
			first.status = GREEN;
			first.phaseLeft = ci.countdown + ci.phaseLeft;
			return false;
		});
		Set(first.status, first.countdown);
	}
	
protected:
};
