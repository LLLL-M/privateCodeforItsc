#pragma once

#include <algorithm>
#include <ctime>
#include <array>
#include <string>
#include "singleton.h"
#include "config.h"
#include "ctrl.h"
#include "ring.h"
#include "channel.h"
#include "memory.h"

class Cycle
{
private:
	void CalCountdown();

public:
	Cycle() = default;
	Cycle(const ControlRule &r) : rule(r) {}
	Cycle(const ControlRule &r, const std::vector<std::vector<UInt8>> &turn) : rule(r)
	{
		for (auto &i: turn)
			ringTable.emplace_back(phaseTable, i);
	}
	Cycle(const Cycle &cycle) : rule(cycle.rule), desc(cycle.desc), phaseTable(cycle.phaseTable), channelTable(cycle.channelTable), stages(cycle.stages) 
	{	/*成员列表初始化顺序最好与成员变量定义的顺序保持一致，不然编译器会报-Wreorder警告*/
		for (auto &ring : cycle.ringTable)
			ringTable.emplace_back(phaseTable, ring.turn);
		cycleTime = cycle.cycleTime;
		leftTime = cycle.leftTime;
	}
	virtual ~Cycle() {}	//因为是基类，所以需要实现析构函数且最好为虚函数
	
	const ControlRule 							rule;				//周期控制规则
	UInt16										cycleTime = 0;		//当前周期总时间
	UInt16										leftTime = 0;		//当前周期还剩下多少时间
	std::time_t									beginTime = 0;		//周期开始运行时间
	UInt8										curStage = 0;		//当前阶段号
	std::vector<Ring> 							ringTable;			//环表

	/*需要填充*/std::string 					desc;				//方案描述
	/*需要填充*/std::map<UInt8, Phase> 			phaseTable;			//相位表
	/*需要填充*/ChannelArray 					channelTable;		//通道表
	/*需要填充*/std::vector<std::vector<UInt8>> stages;				//阶段包含的相位信息

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
		if (stages[stageNum - 1].size() != ringTable.size())
			return;
		for (unsigned int i = 0; i < ringTable.size(); i++)
			ringTable[i].SetJump(stages[stageNum - 1][i]);
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
		return (leftTime <= 1);
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

protected:
	bool specialCtrl = false;
};
