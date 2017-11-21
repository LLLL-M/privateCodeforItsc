#include <algorithm>
#include "common.h"
#include "tsc.h"

struct TscBarrier
{
	UInt8 	id = 0;							//屏障ID号
	
	struct RingInfo
	{
		UInt16	time = 0;						//屏障的时长
		vector<UInt8> phase;
		bitset<MAX_CHANNEL_NUM> conflictTotal = {0};
		bitset<MAX_CHANNEL_NUM> channelTotal = {0} ;
	};
	vector<RingInfo> ring;				//屏障内包含的环

	TscBarrier(const UInt8 &_id) : id(_id)
	{
	}
};

/*检查调度表*/
TscRetval Tsc::ScheduleCheck()
{
	TscRetval ret;

	for (auto &i : schedule.bak)
	{
		//检查调度表中对应的时段表是否存在
		auto timeintervalId = i.second.timeinterval;
		if (!timeinterval.Exist(timeintervalId))
		{
			ret.Err("timeinterval %d isn't exist when check schedule %d", timeintervalId, i.second.id);
			break;
		}

		//检查调度表中的时段表是否符合范围要求
		if (timeintervalId > MAX_TIMELIST_NUM || timeintervalId < 1)
		{
			ret.Err("The timeinterval id[%d] of schedule is out of range[1, %d]!", timeintervalId, MAX_TIMELIST_NUM);
			break;
		}

		auto weekId = i.second.week.to_ulong();
		if (weekId > 254)
		{
			ret.Err("The week[%d] of Schedule is out of range[0,254]!", weekId);
			break;
		}

		for (auto &j : i.second.date)
		{
			if (j.year < 1970 && j.year != 0)
			{
				ret.Err("The year[%d] of schedule is out of range! [1970, ~] or 0", j.year);
				return ret;
			}

			if (j.month > 12 || j.month == 0)
			{
				ret.Err("The month[%d] of schedule is out of range![1, 12]", j.month);
				return ret;
			}

			auto daysId = j.days.to_ulong();
			if (daysId > 0xfffffffe)
			{
				ret.Err("The days[%d] of schedule is out of range![0, 0xfffffffe]", daysId);
				return ret;
			}
		}
	}

	return ret;
}

/*检查时段表*/
TscRetval Tsc::TimeintervalCheck()
{
	TscRetval ret;

	for (auto &i : timeinterval.bak)
	{
		for (auto &j : i.second.section)
		{
			if (!scheme.Exist(j.scheme))
			{
				ret.Err("scheme %d isn't exist when check timeinterval %d", j.scheme, i.second.id);
				return ret;
			}

			if (j.hour > 23)
			{
				ret.Err("The hour[%d] of timeinterval is out of range![0, 23]", j.hour);
				return ret;
			}

			if (j.minute > 59)
			{
				ret.Err("The minute[%d] of timeinterval is out of range![0, 59]", j.minute);
				return ret;
			}

			if (j.mode >= CONTROL_MAX)
			{
				ret.Err("The control mode[%d] of timeinterval is out of range![0, %d)", j.mode, CONTROL_MAX);
				return ret;
			}
			else if (j.mode == CHANNEL_LOCK || j.mode == STEP_MODE || j.mode == CANCEL_STEP)
			{
				ret.Err("The control mode[%d] of timeinterval can't slect![%d,%d,%d]", 
					j.mode, STEP_MODE, CANCEL_STEP, CHANNEL_LOCK);
				return ret;
			}

			if (j.scheme > MAX_SCHEME_NUM || j.scheme == 0)
			{
				ret.Err("The sheme id[%d] of timelist is out of range![1, %d]", j.scheme, MAX_SCHEME_NUM);
				return ret;
			}
		}
	}

	return ret;
}

/*检查方案表*/
TscRetval Tsc::SchemeCheck()
{
	TscRetval ret;

	for (auto &i : scheme.bak)
	{
		for (auto &k : i.second.timing)
		{
			if (!phase.Exist(k.second.phase))
			{
				ret.Err("phase %d isn't exist when check scheme %d", k.second.phase, i.second.id);
				return ret;
			}

			const TscPhase &ph = phase.bak[k.second.phase];

			if (ph.greenFlash + ph.yellow + ph.allred > k.second.time)
			{
				ret.Err("phase %d greenFlash[%d] + yellow[%d] + allred[%d] > time[%d] when check scheme %d",
				        k.second.phase, ph.greenFlash, ph.yellow, ph.allred,
				        k.second.time, i.second.id);
				return ret;
			}

			if (k.second.phase > MAX_PHASE_NUM || k.second.phase < 1)
			{
				ret.Err("The phase id[%d] of scheme is out of range![1, %d]", k.second.phase, MAX_PHASE_NUM);
				return ret;
			}

			if (k.second.status > RED_YELLOW_GREEN || k.second.status < INVALID)
			{
				ret.Err("The phase status[%d] of scheme is out of range![0,12]", k.second.status);
				return ret;
			}
		}

		if (i.second.coordPhase > MAX_PHASE_NUM || i.second.coordPhase < 1)
		{
			ret.Err("The coordphase[%d] of scheme is out of range![1, %d]", i.second.coordPhase, MAX_PHASE_NUM);
			return ret;
		}

		if ( i.second.turn.size() > MAX_RING_NUM)
		{
			ret.Err("The number[%d] of ring is out of range![1, %d]", i.second.turn.size(), MAX_RING_NUM);
			return ret;
		}

		vector<int> tmp;
		for ( auto &k : i.second.turn)
			for ( auto &j : k)
			{
				if (i.second.timing.find(j) == i.second.timing.end())
				{
					ret.Err("phase's configuration %d isn't exist when check scheme timing[%d]", j, j);
					return ret;
				}
				auto it = find(tmp.begin(), tmp.end(), j);
				if (it == tmp.end())
					tmp.emplace_back(j);
				else
				{
					ret.Err("Can't set the same phase[%d] twice in one scheme!", j);
					return ret;
				}
			}

		if (i.second.turn.size() >= 1)
			ret += SetStage(i.second);
	}
	return ret;
}

/*检查相位表*/
TscRetval Tsc::PhaseCheck()
{
	TscRetval ret;

	for (auto &i : phase.bak)
	{
		for (int j = 0; j < MAX_CHANNEL_NUM; j++)
		{
			if (i.second.channelBits[j])
			{
				/*检查相位对应的通道是否存在*/
				if (!channel.Exist(j + 1))
				{
					ret.Err("channel %d isn't exist when check phase %d", j + 1, i.second.id);
					return ret;
				}
				else
				{
					const TscChannel &ch = channel.bak[j + 1];
					auto conflict = i.second.channelBits & ch.conflictBits;

					/*检查对应的通道里的冲突表与此相位内的通道是否冲突*/
					if (conflict.any())
					{
						ret.Err("The channelBits[%lx] of phase %d has conflict[%lx]!",
						        i.second.channelBits.to_ulong(), i.second.id,
						        conflict.to_ulong());
						return ret;
					}
				}						
			}
		}
	}

	return ret;
}


/*检查通道表*/
TscRetval Tsc::ChannelCheck()
{
	TscRetval ret;

	for (auto &i : channel.bak)
	{
		/*检查对应倒计时表是否存在*/
		if (i.second.countdown > 0 && !countdown.Exist(i.second.countdown))
		{
			ret.Err("countdown %d isn't exist when check Channel %d", i.second.countdown, i.second.id);
			return ret;
		}

		if (i.second.type >= CHANNEL_TYPE_MAX || i.second.type < CHANNEL_TYPE_UNUSED)
		{
			ret.Err("The type[%d] of channel is out of range[0, %d)!", i.second.type, CHANNEL_TYPE_MAX);
			return ret;
		}

		if (i.second.status > RED_YELLOW_GREEN || i.second.status < INVALID)
		{
			ret.Err("The status[%d] of channel is out of range![0, 12]", i.second.status);
			return ret;
		}

		if (i.second.countdown > MAX_COUNTDOWN_NUM)
		{
			ret.Err("The countdown id[%d] of channel is out of range![0, %d]", i.second.countdown, MAX_COUNTDOWN_NUM);
			return ret;
		}

		/*检查对应车辆检测器表是否存在*/
		for (int j = 0; j < MAX_VEHDETECTOR_NUM; j++)
		{
			if (i.second.vehBits[j])
			{
				if (!vehDetector.Exist(j + 1))
				{
					ret.Err("vehDetector %d isn't exist when check Channel %d", j + 1, i.second.id);
					return ret;
				}
			}
		}

		/*检查对应行人检测器表是否存在*/
		for (int j = 0; j < MAX_PEDDETECTOR_NUM; j++)
		{
			if (i.second.pedBits[j])
			{
				if (!pedDetector.Exist(j + 1))
				{
					ret.Err("pedDetector %d isn't exist when check Channel %d", j + 1, i.second.id);
					return ret;
				}
			}
		}
	}

	return ret;
}

/*检查优先表*/
TscRetval Tsc::PriorCheck()
{
	TscRetval ret;

	for (auto &i : prior.bak)
	{
		if (!channel.Exist(i.second.channel))
		{
			ret.Err("channel %d isn't exist when check Prior %d", i.second.channel, i.second.id);
			return ret;
		}

		if (i.second.channel > MAX_CHANNEL_NUM || i.second.channel < 1)
		{
			ret.Err("The channel id[%d] of prior is out of range![1, %d]", i.second.channel, MAX_CHANNEL_NUM);
			return ret;
		}
	}
	return ret;
}

/*检查多通道锁定表*/
TscRetval Tsc::MultilockCheck()
{
	TscRetval ret;

	for (auto &i : multilock.bak)
	{
		if (i.second.hour > 23)
		{
			ret.Err("The hour[%d] of multiLock is out of range![0, 23]", i.second.hour);
			return ret;
		}
		
		if (i.second.minute > 59)
		{
			ret.Err("The minute[%d] of multiLock is out of range![0, 59]", i.second.minute);
			return ret;
		}
		
		if (i.second.sec > 59)
		{
			ret.Err("The second[%d] of multiLock is out of range![0, 59]", i.second.sec);
			return ret;
		}
		
		if (i.second.status.size() == MAX_CHANNEL_NUM || i.second.status.empty())
		{
			if ( !i.second.status.empty() )
			{
				for (unsigned int j = 0; j < MAX_CHANNEL_NUM; j++)
				{
					if (i.second.status[j] > RED_YELLOW || i.second.status[j] < INVALID)
					{
						ret.Err("The status[%d] of multiLock is out of range![0, 9]", i.second.status[j]);
						return ret;
					}
				}
			}
		}
		else
		{
			ret.Err("The MultiLock status size[%d] isn't match %d", i.second.status.size(), MAX_CHANNEL_NUM);
			return ret;
		}
	}
	return ret;
}


TscRetval Tsc::WholeCheck()
{
	TscRetval ret;
	ret = ScheduleCheck();
	ret += TimeintervalCheck();
	ret += SchemeCheck();
	ret += PhaseCheck();
	ret += ChannelCheck();
	ret += PriorCheck();
	ret += MultilockCheck();

	return ret;
}


TscRetval Tsc::SetStage(TscScheme &scheme)
{
	TscRetval ret;
	vector<TscBarrier> barr;
	
	for (unsigned int i = 0; i < scheme.turn.size(); i++)
	{
		for (unsigned int j = 0; j < scheme.turn[i].size(); j++)
		{
			auto it = scheme.timing.find(scheme.turn[i][j]);
			if (it->second.barrier == barr.size() + 1)
			{
				barr.emplace_back(it->second.barrier);
				barr.back().ring.resize(scheme.turn.size());
			}
			
			if (it->second.barrier <= barr.size())
			{
				barr[it->second.barrier - 1].ring[i].time += it->second.time;
				barr[it->second.barrier - 1].ring[i].phase.emplace_back(it->second.phase);
				barr[it->second.barrier - 1].ring[i].channelTotal |= phase.bak[it->second.phase].channelBits;

				for (unsigned int k = 0; k < MAX_CHANNEL_NUM; k++)
				{
					/*阶段表中的相位表对应的通道表里的冲突通道*/
					if (phase.bak[it->second.phase].channelBits[k])
						barr[it->second.barrier - 1].ring[i].conflictTotal |= channel.bak[k + 1].conflictBits;
				}
			}
			else
			{
				ret.Err("The barrier number isn't start at 1 or isn't continuous");
				return ret;
			}
		}

		if (i != 0 )
		{	/*检查每个屏障的时长是否相等*/
			for (auto &m : barr)
				if (m.ring[i].time != m.ring[i - 1].time)
				{
					ret.Err("The barrier time isn't match which check ring[%d]", i + 1);
					return ret;
				} 
		}
	}

	StageEstablish(barr, scheme);
	ret += BarrierConflictCheck(barr, scheme);

	return ret;
}

void Tsc::StageEstablish(const vector<TscBarrier> &barr, TscScheme &scheme)
{
	UInt8 stageNo = 1;
	for (unsigned int i = 0; i < barr.size(); i++) /*barrier*/
	{
		size_t index = 0, max = 1;
		do
		{
			scheme.stage.emplace_back(stageNo++);
			TscStage &stage = scheme.stage.back();
			for (unsigned int j = 0; j < barr[i].ring.size(); j++)
			{
				const vector<UInt8> & includePhases = barr[i].ring[j].phase;
				size_t size = includePhases.size();
				if (size == 0)
					continue;
				if (size > max)
					max = size;
				if (index >= size)
					stage.phase.push_back(includePhases.back());
				else
					stage.phase.push_back(includePhases[index]);
			}
			index++;
		} while (index < max);
	}
}

TscRetval Tsc::BarrierConflictCheck(const vector<TscBarrier> &barr, const TscScheme &scheme)
{
	TscRetval ret;
	/*检查每个屏障内部所有相位关联通道是否有冲突*/
	bitset<MAX_CHANNEL_NUM> channelCheck = {0};
	for(unsigned int i = 0; i < barr.size(); i++) /*barrier*/
	{
		for(unsigned int j = 0; j < barr[i].ring.size(); j++)
		{
			for(unsigned int k = 0; k < barr[i].ring.size(); k++)
			{
				if (k != j)
					channelCheck |= barr[i].ring[k].conflictTotal;				
			}
			auto cal = channelCheck & barr[i].ring[j].channelTotal;
			if (cal.any())
			{
				ret.Err("The channelBits[%lx] of barrier[%d] has conflict[%lx]!",
				        barr[i].ring[j].channelTotal.to_ulong(), i, cal.to_ulong());
				return ret;
			}
		}
	}
	return ret;
}


