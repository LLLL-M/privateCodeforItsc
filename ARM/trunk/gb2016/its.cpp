#include "its.h"
#include "queue.h"
#include "light.h"
#include "frame.h"
#include "camera.h"
#include "gpio.h"
#include "singleton.h"

array<UInt16, MAX_PHASE_NUM> SingleSpot::phaseExtend = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bitset<MAX_PHASE_NUM> Inductive::requests(0xffffffff); 
bitset<MAX_PHASE_NUM> BusAdvance::requests(0xffffffff); 
bitset<MAX_PHASE_NUM> BusAdvance::busPhases(0);

Its::Its() : gpio(Singleton<Gpio>::GetInstance())
{
	ControlRule rule = {LOCAL_CONTROL, SYSTEM_MODE, 0};
	nextRule = rule;
	curRule = rule;
	curPhase = 0;
	curCycleTime = 0;
	curLampStatus = 0;
	ruleUpdate = false;
	newRule = rule;
}

bool Its::SetRule(const ControlRule &rule)
{
	ControlRule cur = curRule;
	
	if ((rule.ctrlMode != STEP_MODE && rule == cur)
		|| cur.ctrlType == FAULT_CONTROL
		|| (cur.SpecialMode() && rule.ctrlMode == STEP_MODE))
		return false;
	if (rule.ctrlMode == SYSTEM_MODE)
	{
		ControlRule tmp = {LOCAL_CONTROL, SYSTEM_MODE, 0};
		nextRule = tmp;
		newRule = tmp;
		ruleUpdate = true;
		return true;
	}
	if ((cur.ctrlType == KEY_CONTROL && (rule.ctrlType == CLIENT_CONTROL || rule.ctrlType == LOCAL_CONTROL
			|| (rule.ctrlType == KEY_CONTROL && rule.ctrlMode != YELLOWBLINK_MODE && rule.ctrlMode != ALLRED_MODE && rule.ctrlMode != STEP_MODE)))
		|| (cur.ctrlType == CLIENT_CONTROL && (rule.ctrlType == LOCAL_CONTROL || (rule.ctrlType == CLIENT_CONTROL && (rule.ctrlMode == COORDINATE_MODE || rule.ctrlMode == INDUCTIVE_COORDINATE_MODE))))
		|| (cur.ctrlType == LOCAL_CONTROL && rule.ctrlType == LOCAL_CONTROL && (rule.ctrlMode == STEP_MODE || rule.ctrlMode == CANCEL_STEP)))
		return false;
	if ((rule.ctrlMode != STEP_MODE && rule.ctrlMode != CANCEL_STEP) || (rule.ctrlType == LOCAL_CONTROL && !rule.SpecialMode()))
		nextRule = rule;
	newRule = rule;
	ruleUpdate = true;
	return true;
}

ControlRule Its::Downgrade()
{
	ControlRule rule = nextRule;
	if (rule.ctrlType == KEY_CONTROL || rule.ctrlType == CLIENT_CONTROL)
	{
		rule.ctrlType = LOCAL_CONTROL;
		rule.ctrlMode = COORDINATE_MODE;
	}
	else if (rule.ctrlType == LOCAL_CONTROL)
	{
		if (rule.ctrlMode == COORDINATE_MODE)
			rule.ctrlMode = INDUCTIVE_MODE;
		else if (rule.ctrlMode == INDUCTIVE_MODE)
			rule.ctrlMode = FIXEDCYCLE_MODE;
		else
		{
			if (rule.ctrlMode == YELLOWBLINK_MODE)
				return rule;
			rule.ctrlMode = YELLOWBLINK_MODE;
			newRule = rule;
			ruleUpdate = true;
		}
	}
	nextRule = rule;
	return rule;
}

void Its::KeyCollect()
{
	CtrlKeyType ctrlKeyType = gpio.GetCtrlKeyStatus();
	ControlRule rule = {KEY_CONTROL, SYSTEM_MODE, 0};
	if (ctrlKeyType == NONE_KEY || ctrlKeyType == MANUAL_KEY)
		return;
	else if (ctrlKeyType == YELLOWBLINK_KEY)
		rule.ctrlMode = YELLOWBLINK_MODE;
	else if (ctrlKeyType == ALLRED_KEY)
		rule.ctrlMode = ALLRED_MODE;
	else if (ctrlKeyType == STEP_KEY)
		rule.ctrlMode = STEP_MODE;
	//INFO("ctrlKeyType = %d", ctrlKeyType);
	SetRule(rule);
}

inline void Its::ActiveUploadWorkStatus()
{
	Frame frame;
	const ControlRule rule = curRule;
	CurrentWorkStatus currentWorkStatus = {rule.ctrlType, rule.ctrlMode, rule.ctrlId, curPhase, curCycleTime};
	frame.Send(currentWorkStatus);
}

void Its::SetCurLampStatus(LightBits &bits)
{
	auto Set = [&bits](int n)->UInt8{
		if (3 * n >= MAX_LIGHT_BITS)
			return LVOFF;
		if (bits[3 * n])
			return LVGREEN;
		if (bits[3 * n + 1])
			return LVRED;
		if (bits[3 * n + 2])
			return LVYELLOW;
		return LVOFF;
	};
	bitset<MAX_CHANNEL_NUM * 2> lampValues = 0;
	for (int i = 0, n = 0; i < MAX_CHANNEL_NUM; i++)
	{
		UInt8 v = Set(i);
		lampValues[n++] = v & 0x1;
		lampValues[n++] = v & 0x2;
	}
	if (curLampStatus == lampValues.to_ullong())
		return;
	curLampStatus = lampValues.to_ullong();
	Frame frame;
	LampColorStatus lampStatus{curLampStatus, 0};
	frame.Send(lampStatus);
}

void Its::SetCurrentInfo(Cycle *cycle)
{
	if (cycle != nullptr)
	{
#if 1
		cout << "curPhase: " << (int)cycle->curPhase << endl;
		cout << "cycleTime: " << cycle->cycleTime << endl;
		cout << "leftTime: " << cycle->leftTime << endl;
		cout << "mode: " << (int)cycle->rule.ctrlMode << endl << endl;
#endif
		if (cycle->PhaseBegin())
		{
			curPhase = cycle->curPhase;
			if (cycle->Begin())
			{
				curCycleTime = cycle->cycleTime;
				curRule = cycle->rule;
			}
			ActiveUploadWorkStatus();		//当前运行相位改变主动上报
		}
	}
}

void Its::UpdateRule(ControlRule &runRule, Cycle *&cycle, Queue &que)
{
	if (!ruleUpdate)
		return;
	ruleUpdate = false;
	ControlRule rule = newRule;
	
	rule.Print();
	if (rule.SpecialMode())
	{	//黄闪、全红或关灯则立即改变控制规则
		rule.ctrlId = 0;
		curRule = rule;
		curPhase = 0;
		curCycleTime = 0;
		if (cycle != nullptr)
		{
			delete cycle;
			cycle = nullptr;
			que.Clear();
		}
		runRule = rule;
		ActiveUploadWorkStatus();
	}
	else if (rule.ctrlMode == STEP_MODE)
	{
		if (cycle == nullptr || cycle->StepInvalid(rule.ctrlId))
			return;
		curRule = rule;
		ActiveUploadWorkStatus();
		if (runRule.ctrlMode == STEP_MODE && rule.ctrlId == 0)
			rule.ctrlId = cycle->NextStage();	//如果之前就处于步进模式，再次单步步进时把步进号设定为下个阶段号
		runRule = rule;
	}
	else
	{
		if (cycle == nullptr)
		{	//说明之前执行的是黄闪、全红或关灯控制
			runRule = rule;
			return;
		}
		if (rule.ctrlMode == CANCEL_STEP)
		{
			curRule = cycle->rule;
			ActiveUploadWorkStatus();
			runRule = cycle->rule;
		}
	}
}

void Its::run(void *arg)
{
	Camera	&camera = Singleton<Camera>::GetInstance();
	Queue que(*this);		//队列
	Light light(*this);			//点灯模块
	Cycle *cycle = que.Read();
	ChannelStatusArray channelStatus;
	ControlRule rule = curRule;
	auto ChangeChannelStatus = [&channelStatus](UInt8 mode){
		Status status;
		switch (mode)
		{
			case TURNOFF_MODE: status = TURN_OFF; break;
			case YELLOWBLINK_MODE: status = YELLOW_BLINK; break;
			case ALLRED_MODE: status = ALLRED; break;
			default: return;
		}
		for (auto &st : channelStatus)
		{
			if (st != INVALID)
				st = status;
		}
	};
	
	channelStatus.fill(INVALID);
	camera.start();
	gpio.start();
	que.start();		//开启队列计算线程
	light.start();
	while (true)
	{
		KeyCollect();
		UpdateRule(rule, cycle, que);
		
		if (cycle != nullptr)
		{
			if (rule.ctrlMode == STEP_MODE)
				cycle->Step(rule.ctrlId);
			else
				cycle->Excute();
			cycle->SetChannelStatus(channelStatus);
		}
		else
			ChangeChannelStatus(rule.ctrlMode);
		
		light.SendChannelStatus(channelStatus);
#if 0
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
			cout << "channel " << i + 1 << ": " << channelStatus[i] << endl;
		cout << endl;
#endif
		SetCurrentInfo(cycle);
		Custom(cycle);
		if (cycle != nullptr)
		{
			if (cycle->leftTime == 2)
				que.CalNextCycle(2);
			if (cycle->Over())
			{
				delete cycle;
				cycle = que.Read();
			}
		}
		else
			cycle = que.Read();
	}
}
