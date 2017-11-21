#pragma once

#include <algorithm>
#include <ctime>
#include <array>
#include <string>
#include "singleton.h"
#include "config.h"
#include "ctrl.h"
#include "phase.h"
#include "channel.h"
#include "memory.h"

class Cycle
{
private:
	bool specialCtrl = false;
	const std::vector<TscStage> stages;

	void CalCountdown();

	void InitChannelTable(const std::map<int, TscChannel> &channels)
	{
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			channelTable[i].id = i + 1;
			auto it = channels.find(i + 1);
			if (it != channels.end() && it->second.type != CHANNEL_TYPE_UNUSED)
			{
				channelTable[i].type = it->second.type;
				channelTable[i].specifyStatus = (TscStatus)it->second.status;
				channelTable[i].countdownId = it->second.countdown;
				if (channelTable[i].specifyStatus != INVALID)
					channelTable[i].status = channelTable[i].specifyStatus;
			}
		}
	}

public:
	Cycle() = default;
	Cycle(const ControlRule &r, const std::map<int, TscChannel> &channels) : rule(r)
	{
		InitChannelTable(channels);
	}
	Cycle(const ControlRule &r, const std::map<int, TscChannel> &channels, const std::vector<std::vector<UInt8>> &turn, const std::vector<TscStage> &_stages) : stages(_stages), rule(r)
	{
		InitChannelTable(channels);
		InitRing(turn);
	}
	Cycle(const Cycle &cycle) : stages(cycle.stages), rule(cycle.rule), desc(cycle.desc), phaseTable(cycle.phaseTable), channelTable(cycle.channelTable)
	{
		for (auto &ring : cycle.ringTable)
			ringTable.emplace_back(phaseTable, ring.turn);
		cycleTime = cycle.cycleTime;
		leftTime = cycle.leftTime;
	}
	virtual ~Cycle() {}
	
	UInt16	cycleTime = 0;							//当前周期总时间
	UInt16	leftTime = 0;							//当前周期还剩下多少时间
	std::time_t	beginTime = 0;						//周期开始运行时间
	const ControlRule rule;							//周期控制规则
	/*const */std::string desc;							//方案描述
	UInt8	curStage = 0;							//当前阶段号

	/*需要填充*/std::map<UInt8, Phase> phaseTable;					//相位表
	/*需要填充*/std::array<Channel, MAX_CHANNEL_NUM> channelTable;	//通道表
	std::vector<Ring> ringTable;							//环表

	void InitRing(const std::vector<std::vector<UInt8>> &turn)
	{
		for (auto &i: turn)
			ringTable.emplace_back(phaseTable, i);
	}

	bool StepInvalid(const UInt8 stageNum) const//判断步进号是否无效
	{
		if (stageNum == 0)
		{	//单独步进在相位处于过渡期或者没有阶段表时是无效的
			if (stages.empty())
				return true;
			for (auto &it : phaseTable)
			{
				if (it.second.TransitionPeriod())
					return true;
			}
			return false;
		}
		else
			return (stageNum > (UInt8)stages.size());	//步进号大于最大阶段个数也是无效的
	}

	const UInt8 NextStage() const
	{
		return (curStage + 1 > (UInt8)stages.size()) ? 1 : (curStage + 1);
	}

	void Step(const UInt8 stageNum)
	{
		if (stageNum == 0 || stageNum == curStage)
		{
			for (auto &ring: ringTable)
				ring.SetStay();
			return;
		}
		if (stages[stageNum - 1].phase.size() != ringTable.size())
			return;
		for (unsigned int i = 0; i < ringTable.size(); i++)
			ringTable[i].SetJump(stages[stageNum - 1].phase[i]);
	}

	void CancelStep()
	{
		for (auto &ring: ringTable)
			ring.SetMove();
	}

	void SpecialCtrl(UInt8 mode);

	virtual void Excute();

	bool Over() const
	{
		return (leftTime == 1);
	}

	virtual Cycle *Clone()
	{
		return new Cycle(*this);
	}

	void * operator new(size_t size)
	{
		hik::memory &memory = Singleton<hik::memory>::GetInstance();
		return memory.alloc(size);
	}

	void operator delete(void *p)
	{
		hik::memory &memory = Singleton<hik::memory>::GetInstance();
		return memory.free(p);
	}
};
