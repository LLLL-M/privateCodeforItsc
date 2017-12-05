#pragma once 

#include <map>
#include <atomic>
#include <algorithm>
#include "phase.h"

class Ring
{
private:
	UInt8 jumpPhase;
	enum
	{
		MOVEING = 0,
		JUMPING = 1,
		STAYING = 2,
	};
	std::atomic_uint runStatus;
	volatile unsigned int pos;
	std::map<UInt8, Phase> &mp;
	
	UInt16 Sum(unsigned int first, unsigned int last) const
	{
		UInt16 sum = 0;
		for (unsigned int i = first; i < last; i++)
		{
			sum += mp[turn[i]].Total();
		}
		return sum;
	}

public:
	Ring(std::map<UInt8, Phase> &_mp, const std::vector<UInt8> &_turn) : mp(_mp), turn(_turn)
	{
		runStatus = MOVEING;
		pos = 0;
		barrierEnd = false;
	}

	Ring(const Ring &ring) : Ring(ring.mp, ring.turn) {}	//使用了委托构造函数
	//~Ring() {}
	
	const std::vector<UInt8> turn;
	volatile bool barrierEnd;

	/*获取当前放行的相位号*/
	UInt8 CurPhaseId() const
	{
		return turn.empty() ? 0 : turn[pos];
	}
	/*设置环中相位移动*/
	void SetMove()
	{
		runStatus = MOVEING;
	}
	/*设置环中相位跳转*/
	void SetJump(UInt8 phaseId)
	{
		if (find_if(turn.begin(), turn.end(), [&phaseId](const UInt8 id)->bool{return id == phaseId;}) == turn.end())
			return;
		runStatus = JUMPING;
		jumpPhase = phaseId;
	}
	/*设置环中相位驻留*/
	void SetStay()
	{
		runStatus = STAYING;
	}
	/*环的总长度，也就是周期*/
	UInt16 Total() const
	{
		return Sum(0, turn.size());
	}
	/*环中周期已经放行的时间，从0开始*/
	UInt16 Used() const
	{
		if (turn.empty())
			return 0;
		return Sum(0, pos) + mp[turn[pos]].Used();
	}
	/*环中周期未放行的时间，从周期时间开始，最小为1*/
	UInt16 Left() const
	{
		return Total() - Used();
	}
	/*延长环中具体屏障的时间*/
	void Extend(UInt8 barrier, UInt16 sec)
	{
		for (unsigned int i = pos; i < turn.size(); i++)
		{
			if (sec == 0)
				break;
			Phase &phase = mp[turn[i]];
			if (phase.barrier == barrier)
				sec = phase.ExtendGreen(sec);
		}
		if (sec > 0 && !turn.empty())
			mp[turn[pos]].Add({sec, ALLRED, 0});
	}
	//void Reduce();
	/*执行环中的周期放行*/
	void Excute();
	/*计算环中各相位的倒计时*/
	void CalPhaseCountdown(const UInt16 cycleTime);
};
