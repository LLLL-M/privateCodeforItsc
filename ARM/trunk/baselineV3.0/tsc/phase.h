#pragma once

#include <vector>
#include <map>
#include <atomic>
#include <ctime>
#include <bitset>
#include <algorithm>
#include "colorstep.h"

/*
1.车检器检测到有车来了告诉对应的相位
2.依据当前的控制模式来调整相位，调整方法如下
(1)关灯控制: 相位关灯
(2)黄闪控制: 相位黄闪
(3)全红控制: 相位全红
(4)定周期：相位移动
(5)协调控制: 相位移动
(6)半感应控制: 相位加秒、移动、跳转，驻留					依赖车检器
(7)单点优化: 相位移动，下一周期相位加秒、减秒或者不变		依赖车检器
(8)步进控制: 相位驻留或者跳转之后再驻留				
(9)取消步进: 相位移动
(10)行人感应: 相位驻留、移动、加秒							依赖行人检测器
(11)公交优先: 相位移动、加秒，也可能减秒					依赖公交的检测器
(12)协调感应: 相位移动、加秒 								依赖车检器
(13)全感应控制: 相位移动、加秒、下一周期增加或缩减最大绿1 	依赖车检器
 */

/*相位动作
1. 移动，设置通道状态和倒计时
2. 加秒
3. 驻留
4. 跳转
相位来车时每个ring要根据当前的控制模式做出相应的动作
*/

struct Phase
{
//private:
	/*const*/UInt8	phaseId = 0;								//相位号
	/*const*/UInt8 	ring = 0;									//相位所在环号
	/*const*/UInt8 	barrier = 0;								//相位所在屏障号
	/*const*/UInt16	checkTime = 0;								//感应检测时间
	/*const*/bitset<MAX_CHANNEL_NUM> channelBits;				//相位关联的通道
	/*const*/bitset<MAX_VEHDETECTOR_NUM> vehDetectorBits = 0;	//相位关联的车辆检测器
	/*const*/bitset<MAX_PEDDETECTOR_NUM> pedDetectorBits = 0;	//相位关联的行人检测器
	/*const*/UInt16	unitExtendTime = 0;							//单位延长绿
	/*const*/bool	pedPhase = false;							//是否是行人相位
	/*const*/UInt16	maxExtendTime = 0;							//最大可以延长的绿灯时间
	/*const*/TscStatus 	specifyStatus = INVALID;				//相位指定状态
	/*const*/bool	autoReq = true;								//相位自动请求
	/*const*/string desc;										//相位描述

	UInt16	extendTime = 0;										//延长的绿灯时间
	time_t	beginTime = 0;										//相位开始运行时间
	
	/*const*/ColorStepVector motor;
	/*const*/ColorStepVector pedestrian;

	/*相位移动1s*/
	bool Move()
	{
		if (beginTime == 0)
			beginTime = std::time(nullptr);
		pedestrian.Move(1);
		bool ret = motor.Move(1);
		//motor.PrintCur();
		return ret;
	}
	/*相位延长，返回未能延长的时间*/
	UInt16 Extend(UInt16 sec)
	{
		if (sec == 0 || extendTime >= maxExtendTime)
			return sec;
		UInt16 ret = 0;
		if (extendTime + sec > maxExtendTime)
		{
			ret = extendTime + sec - maxExtendTime;
			sec -= ret;
		}
		if (!motor.Extend(sec))
			return sec + ret;
		if (!pedestrian.Extend(sec))
			pedestrian.Add(sec, ALLRED, 0);
		extendTime += sec;
		return ret;
	}
	/*相位增加色步*/
	void Add(const ColorStep &cs)
	{
		motor.Add(cs);
		pedestrian.Add(cs);
	}
	bool Over() const
	{
		return motor.Over();
	}
	/*设置相位运行结束*/
	void SetOver()
	{
		motor.SetOver();
		pedestrian.SetOver();
	}
	/*重置相位*/
	void Reset()
	{
		motor.Reset();
		pedestrian.Reset();
	}
	/*跳到相位过渡时期*/
	void JumpToTransitionPeriod()
	{
		int used = motor.Jump();
		pedestrian.Reset();
		pedestrian.Move(used + 1);	//加1是因为Reset之后used==-1
	}
	/*相位是否在放行*/
	bool Inuse() const
	{
		return motor.Inuse();
	}
	/*相位已经放行的时间，从0开始*/
	UInt16 Used() const
	{
		return motor.Used();
	}
	/*相位剩余时间，从1开始，放行结束为0*/
	UInt16 Left() const
	{
		return motor.Left();
	}
	/*相位总共的时间，包括后续增加的*/
	UInt16 Total() const
	{
		return motor.Total();
	}
	/*相位非绿时间*/
	UInt16 Green() const
	{
		return motor.Green();
	}
	/*机动车相位当前的状态*/
	TscStatus MotorStatus() const
	{
		return motor.Current().status;
	}
	/*行人相位当前的状态*/
	TscStatus PedStatus() const
	{
		return pedestrian.Current().status;
	}
	/*机动车相位当前的色步*/
	ColorStep MotorColorStep() const
	{
		return motor.Current();
	}
	/*行人相位当前的色步*/
	ColorStep PedColorStep() const
	{
		return pedestrian.Current();
	}
	/*设置机动车相位红灯时的倒计时*/
	void SetMotorCountdown(UInt16 countdown)
	{
		TscStatus st = MotorStatus();
		if (st == ALLRED || st == RED)
			motor.SetCurrentCountdown(countdown);
	}
	/*设置行人相位红灯时的倒计时*/
	void SetPedCountdown(UInt16 countdown)
	{
		TscStatus st = PedStatus();
		if (st == ALLRED || st == RED)
			pedestrian.SetCurrentCountdown(countdown);
	}
	/*设置机动车和行人红灯时的倒计时*/
	void SetCountdown(UInt16 countdown)
	{
		SetMotorCountdown(countdown);
		SetPedCountdown(countdown);
	}
	/*设置机动车和行人相位的状态*/
	void SetStatus(TscStatus st)
	{
		motor.SetCurrentStatus(st);
		pedestrian.SetCurrentStatus(st);
	}
	/*机动车相位是否处于过渡时期*/
	bool TransitionPeriod() const
	{
		return MotorColorStep().TransitionPeriod();
	}
};

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
				sec = phase.Extend(sec);
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

