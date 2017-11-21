#pragma once

//#include <chrono>
#include <atomic>
#include "hik.h"
#include "common.h"
#include "timer.h"

//控制类型
enum
{
	LOCAL_CONTROL = 0,			//本地控制
	CLIENT_CONTROL = 1,			//客户端或web控制
	PLATFORM_CONTROL = 2,		//平台控制
	PRIOR = 3,					//优先控制
	KEY_CONTROL = 4,			//按键控制
	FAULT_CONTROL = 5,			//故障控制
};

struct ControlRule
{
	UInt8	ctrlType = LOCAL_CONTROL;		//控制类型
	UInt8	ctrlMode = SYSTEM_MODE;			//控制模式
	UInt8	ctrlId = 0;						//控制编号
	UInt32	duration = 0;					//控制持续时间

	ControlRule() = default;
	ControlRule(UInt8 type, UInt8 mode, UInt8 id = 0, UInt32 time = 0)
	{
		Set(type, mode, id, time);
	}

	void Set(UInt8 type, UInt8 mode, UInt8 id = 0, UInt32 time = 0)
	{
		ctrlType = type;
		ctrlMode = mode;
		ctrlId = id;
		duration = time;
	}
	bool operator==(const ControlRule &r) const
	{
		return (r.ctrlType == ctrlType && r.ctrlMode == ctrlMode && r.ctrlId == ctrlId);
	}
	bool operator!=(const ControlRule &r) const
	{
		return (r.ctrlType != ctrlType || r.ctrlMode != ctrlMode || r.ctrlId != ctrlId);
	}
	bool SpecialMode() const
	{
		return (ctrlMode == YELLOWBLINK_MODE || ctrlMode == TURNOFF_MODE || ctrlMode == ALLRED_MODE);
	}
	void Print(const char *before = nullptr) const
	{
		if (before)
			INFO("%s, rule, ctrlType: %d, ctrlMode: %d, ctrlId: %d", before, ctrlType, ctrlMode, ctrlId);
		else
			INFO("rule, ctrlType: %d, ctrlMode: %d, ctrlId: %d", ctrlType, ctrlMode, ctrlId);
	}
}__attribute__((packed)); 	//此处必须增加此语句，取消结构体对齐，将结构体大小压缩至64位以内。
							//因为64位的atomic结构体在ARM中需要配合kernel3.1以上，并且指令集至少要arm v7，并且交叉工具链支持hard float模式
							//旧的开发板指令集只是armv5te，即使开启hard float模式依旧无法支持，并且linux kernel只为2.6.39。
							//所以采取这种折中的办法去回避掉atomic大于等于64位的现象。

class Its;
class Tsc;

class Ctrl : public hik::timer
{
private:
	Its &its;
	Tsc &tsc;
	//std::chrono::steady_clock::time_point last;	//上一次更新控制规则的时间点
	//ControlRule oldRule;							//旧的控制规则也就是上一次的控制规则
	atomic<ControlRule> curRule;					//当前的控制规则
	bool LocalCtrl();
public:
	Ctrl();
	//~Ctrl();
	
	bool SetRule(ControlRule &rule);
	void RecoverRule(const ControlRule &rule);
	//void Downgrade();
	//void Downgrade(const ControlRule &rule);

	void Timeout();

	void ConfigUpdate();
	
};
