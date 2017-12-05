#pragma once

#include <ctime>
#include <bitset>
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
	/*需要填充*/UInt8	phaseId = 0;								//相位号
	/*需要填充*/UInt8 	ring = 0;									//相位所在环号
	/*需要填充*/UInt8 	barrier = 0;								//相位所在屏障号
	/*需要填充*/UInt16	checkTime = 0;								//感应检测时间
	/*需要填充*/bitset<MAX_CHANNEL_NUM> channelBits;				//相位关联的通道
	/*需要填充*/bitset<MAX_VEHDETECTOR_NUM> vehDetectorBits = 0;	//相位关联的车辆检测器
	/*需要填充*/bitset<MAX_PEDDETECTOR_NUM> pedDetectorBits = 0;	//相位关联的行人检测器
	/*需要填充*/UInt16	unitExtendTime = 0;							//单位延长绿
	/*需要填充*/bool	pedPhase = false;							//是否是行人相位
	/*需要填充*/UInt16	maxExtendTime = 0;							//最大可以延长的绿灯时间
	/*需要填充*/TscStatus 	specifyStatus = INVALID;				//相位指定状态
	/*需要填充*/bool	autoReq = true;								//相位自动请求
	/*需要填充*/string desc;										//相位描述

	UInt16	extendTime = 0;										//延长的绿灯时间
	time_t	beginTime = 0;										//相位开始运行时间
	
	/*需要填充*/ColorStepVector motor;
	/*需要填充*/ColorStepVector pedestrian;

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
	/*相位延长绿灯或绿闪，返回未能延长的时间*/
	UInt16 ExtendGreen(UInt16 sec)
	{
		if (sec == 0 || extendTime >= maxExtendTime)
			return sec;
		UInt16 ret = 0;
		if (extendTime + sec > maxExtendTime)
		{
			ret = extendTime + sec - maxExtendTime;
			sec -= ret;
		}
		if (!motor.ExtendGreen(sec))
			return sec + ret;
		if (!pedestrian.ExtendGreen(sec))
			pedestrian.Add(sec, ALLRED, 0);
		extendTime += sec;
		return ret;
	}
	/*相位延长，延长相位第一个色步*/
	void Extend(UInt16 sec)
	{
		motor.Extend(sec);
		pedestrian.Extend(sec);
	}
	/*相位增加色步*/
	void Add(const ColorStep &cs)
	{
		motor.Add(cs);
		pedestrian.Add(cs);
	}
	/*相位是否结束*/
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
	/*相位绿时间包括绿闪*/
	UInt16 Green() const
	{
		return motor.Green();
	}
	/*相位非全红时间，主要包括绿、绿闪以及黄灯*/
	UInt16 NonAllRed() const
	{
		return motor.NonAllRed();
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

	bool GreenLastSec()	const//绿灯最后1秒
	{
		const ColorStep &cur = motor.Current();
		const ColorStep &next = motor.Next();
		return (cur.status == GREEN && cur.stepTime == 1 && next.status != GREEN);
	}

	bool GreenEnd()	const //绿或者绿闪结束
	{
		const ColorStep &cur = motor.Current();
		const ColorStep &next = motor.Next();
		return ((cur.status == GREEN || cur.status == GREEN_BLINK) && cur.stepTime == 1
				&& next.status != GREEN && next.status != GREEN_BLINK);
	}
	
	bool WindowPeriod()	const //是否是检测窗口期
	{
		const ColorStep &cur = motor.Current();
		return ((cur.status == GREEN || cur.status == GREEN_BLINK)
				&& (cur.countdown >= checkTime)
				&& (cur.countdown < checkTime + unitExtendTime));
	}
	
	bool WindowPeriodEnd() const //是否是窗口期结束
	{
		const ColorStep &cur = motor.Current();
		return ((cur.status == GREEN || cur.status == GREEN_BLINK)
				&& (cur.countdown == checkTime));
	}
	
	bool TransitionPeriod()	const //是否是过渡时期
	{
		const ColorStep &cur = motor.Current();
		return cur.TransitionPeriod();
	}
	
	bool ReleasePeriod() const //是否是放行时期
	{
		const ColorStep &cur = motor.Current();
		return (cur.status == GREEN || cur.status == GREEN_BLINK || cur.status == YELLOW || cur.status == ALLRED);
	}
};

