#pragma once

#include <atomic>
#include "ctrl.h"
#include "rwlock.h"

//前向声明
class Cycle;

class Its
{
private:
	std::atomic_bool ruleUpdate;
	atomic<ControlRule> newRule;	//新的控制规则
	atomic<ControlRule> curRule;	//当前使用的控制规则
	Cycle *cycle;					//当前使用的周期对象指针
	Cycle *nextCycle;				//下一周期要使用的周期对象指针
	hik::rwlock rwlock;

	void UpdateRule();
	void RealtimeStat();
	void FillTakeOverInfo();
	void GetNextCycle();
	
public:
	Its()
	{
		curRule = ControlRule(LOCAL_CONTROL, SYSTEM_MODE);
		ruleUpdate = false;
		cycle = nullptr;
		nextCycle = nullptr;
	}

	~Its();
	
	/*改变控制*/
	void AlterCtrl(const ControlRule &rule, Cycle *next = nullptr)
	{
		newRule = rule;
		ruleUpdate = true;
		UpdateCycle(next);
	}
	
	/*周期执行函数，1s执行一次*/
	void Run();

	/*更新周期数据*/
	void UpdateCycle(Cycle *next);

	/*获取当前使用的控制规则*/
	ControlRule GetRule() const
	{
		return curRule;
	}

protected:
};

