#include "tsc.h"
#include "ctrl.h"
#include "its.h"
#include "singleton.h"
#include "cycle.h"

Ctrl::Ctrl() : its(Singleton<Its>::GetInstance()), tsc(Singleton<Tsc>::GetInstance())
{
	//last = std::chrono::steady_clock::now();
	ControlRule cur(LOCAL_CONTROL, SYSTEM_MODE);
	Cycle *cycle = tsc.GetInitCycle(cur);
	if (cycle == nullptr)
	{	//说明此时本地没有控制方案，则默认降级到黄闪控制
		cur.Set(LOCAL_CONTROL, YELLOWBLINK_MODE, 0);
		curRule = cur;
		its.AlterCtrl(cur);
		reset(1000);	//定时1s，即每秒都去检查本地是否有配置更新
	}
	else
	{
		curRule = cur;
		its.AlterCtrl(cur, cycle);
		reset(cur.duration * 1000);
	}
}

bool Ctrl::LocalCtrl()
{
	ControlRule rule(LOCAL_CONTROL, SYSTEM_MODE, 0);
	Cycle *cycle = tsc.GetCycle(rule);
	if (cycle == nullptr)
	{	//说明此时本地没有控制方案，则默认降级到黄闪控制
		rule.Set(LOCAL_CONTROL, YELLOWBLINK_MODE, 0);
		curRule = rule;
		its.AlterCtrl(rule);
		reset(1000);	//定时1s，即每秒都去检查本地是否有配置更新
		ERR("!!!!!!!!!!!!!! tsc.GetCycle error");
		return false;
	}
	else
	{
		curRule = rule;
		its.AlterCtrl(rule, cycle);
		reset(rule.duration * 1000);
		INFO("tsc.GetCycle successful! ctrlType: %d, ctrlMode: %d, ctrlId: %d, duration: %d", 
			rule.ctrlType, rule.ctrlMode, rule.ctrlId, rule.duration);
		return true;
	}
}

bool Ctrl::SetRule(ControlRule &rule)
{
	rule.Print(__func__);
	ControlRule cur = curRule;
	if ((rule.ctrlMode != STEP_MODE && rule == cur)	//非步进模式新旧规则一样
		|| cur.ctrlType == FAULT_CONTROL	//当前是故障控制模式
		|| rule.ctrlMode == CHANNEL_LOCK	//下发的是通道锁定模式
		|| (cur.SpecialMode() && rule.ctrlMode == STEP_MODE)	//当前处于特殊控制下去执行步进控制
		|| (cur.ctrlMode != STEP_MODE && rule.ctrlMode == CANCEL_STEP)	//非步进控制下却要取消步进
		|| (cur.ctrlType == LOCAL_CONTROL && rule.ctrlMode == SYSTEM_MODE))	//本地控制时执行系统控制模式
		return false;
	if (rule.ctrlMode == SYSTEM_MODE || rule.ctrlMode == CANCEL_STEP)
		return LocalCtrl();
	if ((cur.ctrlType == KEY_CONTROL && (rule.ctrlType < KEY_CONTROL))
		|| (cur.ctrlType == CLIENT_CONTROL && rule.ctrlType == LOCAL_CONTROL)
		|| (cur.ctrlType == LOCAL_CONTROL && rule.ctrlType == LOCAL_CONTROL && (rule.ctrlMode == STEP_MODE || rule.ctrlMode == CANCEL_STEP)))
		return false;

	//auto tp = std::chrono::steady_clock::now();
	//UInt32 gap = (UInt32)((tp - last).count());
	Cycle *cycle = nullptr;
	if (!rule.SpecialMode() && rule.ctrlMode != STEP_MODE && rule.ctrlId != 0)
	{	//说明是上位机执行某个具体的方案需要重新计算
		cycle = tsc.GetCycle(rule);
		if (cycle == nullptr)
			return false;
	}

	if (rule.duration == 0)	//说明要一直持续控制
		halt();	//停止定时器
	else	//说明只控制一段时间
		reset(rule.duration * 1000);	//重新启动一个定时器
	//oldRule = curRule;
	curRule = rule;
	its.AlterCtrl(rule, cycle);
	return true;
}

void Ctrl::RecoverRule(const ControlRule &rule)
{
	curRule = rule;
	reset(rule.duration * 1000);
}

void Ctrl::Timeout()
{
	LocalCtrl();
}

void Ctrl::ConfigUpdate()
{
	ControlRule cur = curRule;
	if (cur.ctrlType != LOCAL_CONTROL && (cur.SpecialMode() || cur.ctrlMode == STEP_MODE))
		return;
	Cycle *cycle = tsc.GetCycle(cur);
	if (cycle != nullptr)
	{
		curRule = cur;
		its.AlterCtrl(cur, cycle);
		if (cur.ctrlType == LOCAL_CONTROL)
			reset(cur.duration * 1000);
	}
}
