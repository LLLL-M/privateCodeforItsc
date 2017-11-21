#ifndef __ITS_H__
#define __ITS_H__

#include <atomic>
#include "thread.h"
#include "common.h"

//前向声明
class Cycle;
class Queue;
class Gpio;

class Its: public HikThread::Thread
{
private:
	atomic_bool ruleUpdate;		//规则更新
	atomic<ControlRule> newRule;

	Gpio &gpio;
	
	void KeyCollect();
	void ActiveUploadWorkStatus();
	void SetCurrentInfo(Cycle *cycle);
	void UpdateRule(ControlRule &runRule, Cycle *&cycle, Queue &que);
	
public:
	Its();
	
	atomic<ControlRule>						curRule;		//当前控制规则
	atomic<UInt8>							curPhase;		//当前运行相位
	atomic<UInt16>							curCycleTime;	//当前周期时间
	volatile UInt64							curLampStatus;	//当前灯色状态
	
	atomic<ControlRule>						nextRule;		//下一周期控制规则
	
	void run(void *arg);
	bool SetRule(const ControlRule &rule);
	ControlRule Downgrade();
	void SetCurLampStatus(LightBits &bits);
	virtual bool GetLocalRule(const int aheadOfTime, ControlRule &rule) { return false; }
	virtual bool FillSchemeInfo(const ControlRule &rule, SchemeInfo &schemeInfo) { return false; }
	virtual void StartInit(vector<ColorStep> &vec) {}	//启动黄闪和全红初始化
	virtual void Custom(const Cycle *cycle) {}
protected:
};

#endif
