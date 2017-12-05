#include <set>
#include <algorithm>
#include "cycle.h"

class ChannelCal	//此类主要用来计算某个通道当前这1s的倒计时以及对应的状态
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
	void AddInfo(const ColorStep &cs, UInt16 left, UInt16 green)
	{
		infos.emplace(cs.status, cs.countdown, left, green);
	}

	void CalInfo(Channel &channel);

protected:
};

void ChannelCal::CalInfo(Channel &channel)
{
	auto Set = [&channel, this](TscStatus st, UInt16 cd){
		channel.status = st;
		channel.countdown = cd;
		infos.clear();
	};
	if (channel.specifyStatus != INVALID)
	{
		Set(channel.specifyStatus, 0);
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

void Cycle::CalCountdown()
{
	if (beginTime == 0)
		beginTime = std::time(nullptr);
	if (ringTable.empty())
		return;
	cycleTime = ringTable[0].Total();
	leftTime = ringTable[0].Left();

	for (auto &ring: ringTable)
		ring.CalPhaseCountdown(cycleTime);

	std::array<ChannelCal, MAX_CHANNEL_NUM> channelCal;
	/*先把通道在所有环中的倒计时存入到对应的infos中，存入是按照放行期在前，红灯时期依次靠后的顺序排列(排序的实现在ChannelInfo类中opeator<()函数)*/
	for (auto &it: phaseTable)
	{
		Phase &phase = it.second;
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			if (phase.channelBits[i])
			{
				if (channelTable[i].type == CHANNEL_TYPE_CAR)
					channelCal[i].AddInfo(phase.MotorColorStep(), phase.Inuse() ? phase.Left() : phase.Total(), phase.Green());
				else if (channelTable[i].type == CHANNEL_TYPE_PED)
					channelCal[i].AddInfo(phase.PedColorStep(), phase.Inuse() ? phase.Left() : phase.Total(), phase.Green());
			}
		}
	}
	/*对每个通道infos中的倒计时进行统一的计算得出最终的倒计时和状态*/
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		channelCal[i].CalInfo(channelTable[i]);
	}
}

void Cycle::SpecialCtrl(UInt8 mode)
{
	TscStatus st;
	switch (mode)
	{
		case TURNOFF_MODE: st = TURN_OFF; break;
		case YELLOWBLINK_MODE: st = YELLOW_BLINK; break;
		case ALLRED_MODE: st = ALLRED; break;
		default: st = INVALID;
	}
	for (auto &it: phaseTable)
		it.second.SetStatus(st);
	for (auto &channel: channelTable)
	{
		if (channel.specifyStatus == INVALID)
			channel.status = st;
		else
			channel.status = channel.specifyStatus;
	}
	specialCtrl = true;
}

void Cycle::Excute()
{
	if (specialCtrl)
		return;

	std::vector<UInt8> curPhases;
	size_t barrierEndCount = 0;
	for (auto &ring: ringTable)
	{
		ring.Excute();
		if (ring.barrierEnd)
			barrierEndCount++;
		curPhases.push_back(ring.CurPhaseId());
	}

	if (barrierEndCount == ringTable.size())	//如果所有环的屏障都结束了则清除屏障结束标志表明可以进入下一个屏障了
		std::for_each(ringTable.begin(), ringTable.end(), [](Ring &ring){ring.barrierEnd = false;});
	//根据当前运行的相位设置当前阶段号
	for (size_t i = 0; i < stages.size(); i++)
	{
		if (operator==(stages[i], curPhases))
		{
			curStage = (UInt8)i + 1;
			break;
		}
	}
	CalCountdown();
#if 0
	INFO("**************************");
	for (auto &it : phaseTable)
		it.second.MotorColorStep().Print();
	cout << "cycleTime: " << cycleTime << ", leftTime: " << leftTime << endl;
	INFO("**************************\n");
#endif
}
