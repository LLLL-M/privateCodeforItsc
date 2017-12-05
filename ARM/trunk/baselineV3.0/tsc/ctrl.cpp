#include "tsc.h"
#include "ctrl.h"
#include "its.h"
#include "singleton.h"
#include "cycle.h"

void Ctrl::UpdateRule(const ControlRule &newRule)
{
	auto now = std::chrono::steady_clock::now();
	ControlRule oldRule = curRule;
	bool update = (newRule != oldRule);

	mtx.lock();
	tp = newRule.duration ? (now + std::chrono::seconds(newRule.duration)) : std::chrono::steady_clock::time_point::max();
	if (update)
	{
		oldRule.duration = (UInt32)(std::chrono::duration_cast<std::chrono::seconds>(now - start).count());
		start = now;
	}
	mtx.unlock();
	if (update)
	{
		curRule = newRule;
		/*此处需要把oldRule存储到文件中方便上位机查看控制时长*/
	}
}

bool Ctrl::TimeIsUp()
{
	hik::mutex_guard guard(mtx);
	auto now = std::chrono::steady_clock::now();
	return (now >= tp);
}

Ctrl::Ctrl() : its(Singleton<Its>::GetInstance()), tsc(Singleton<Tsc>::GetInstance())
{
	ControlRule cur(LOCAL_CONTROL, SYSTEM_MODE);
	Cycle *cycle = tsc.GetInitCycle(cur);
	if (cycle == nullptr)
	{	//说明此时本地没有控制方案，则默认降级到黄闪控制, 定时1s，即每秒都去检查本地是否有配置更新
		cur.Set(LOCAL_CONTROL, YELLOWBLINK_MODE, 0, 1);
	}
	UpdateRule(cur);
	its.AlterCtrl(cur, cycle);
}

bool Ctrl::LocalCtrl()
{
	ControlRule rule(LOCAL_CONTROL, SYSTEM_MODE, 0);
	Cycle *cycle = tsc.GetCycle(rule);
	bool ret = true;
	if (cycle == nullptr)
	{	//说明此时本地没有控制方案，则默认降级到黄闪控制, 定时1s，即每秒都去检查本地是否有配置更新
		rule.Set(LOCAL_CONTROL, YELLOWBLINK_MODE, 0, 1);
		ret = false;
		ERR("!!!!!!!!!!!!!! tsc.GetCycle error");
	}
	else
		INFO("tsc.GetCycle successful! ctrlType: %d, ctrlMode: %d, ctrlId: %d, duration: %d", 
			rule.ctrlType, rule.ctrlMode, rule.ctrlId, rule.duration);
	UpdateRule(rule);
	its.AlterCtrl(rule, cycle);
	return ret;
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

	Cycle *cycle = nullptr;
	if (!rule.SpecialMode() && rule.ctrlMode != STEP_MODE && rule.ctrlId != 0)
	{	//说明是上位机执行某个具体的方案需要重新计算
		cycle = tsc.GetCycle(rule);
		if (cycle == nullptr)
			return false;
	}

	UpdateRule(rule);
	its.AlterCtrl(rule, cycle);
	return true;
}

void Ctrl::ConfigUpdate()
{
	ControlRule cur = curRule;
	if (cur.ctrlType != LOCAL_CONTROL && (cur.SpecialMode() || cur.ctrlMode == STEP_MODE))
		return;
	Cycle *cycle = tsc.GetCycle(cur);
	if (cycle != nullptr)
	{
		UpdateRule(cur);
		its.AlterCtrl(cur, cycle);
	}
}
