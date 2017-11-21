#ifndef __CYCLE_H__
#define __CYCLE_H__

#include <vector>
#include "common.h"

class ColorStepVector
{
private:
	int total = 0;
	int used = 0;
	int pos = 0;
	ColorStep cur;
	vector<ColorStep> vec;

	const ColorStep Move(int t);
	
public:
	const ColorStep operator>>(int t)
	{
		Move(t);
	}
	const ColorStep operator<<(int t)
	{
		return Move(-t);
	}
	const ColorStep Current() const
	{
		return (vec.size() > 0 && used == 0) ? vec[0] : cur;
	}
	const ColorStep Next() const
	{
		return (vec.size() == 0) ? ColorStep() : vec[(pos + 1 == vec.size()) ? 0 : (pos + 1)];
	}
	const UInt16 Left() const
	{
		return (UInt16)(total - used);
	}
	void SetCurrentStatus(const Status status)
	{
		if (vec.size() == 0)
			return;
		cur.status = status;
	}
	void Extend(int t)
	{
		if (t == 0)
			return;
		for (int i = pos; i < vec.size(); i++)
		{
			if (vec[i].status != YELLOW)
			{
				vec[i].stepTime += t;
				vec[i].countdown += t;
				total += t;
				break;
			}
		}
	}
	template<typename...Args>
	void Add(Args&&... args)	//c++11可变参数模板
	{	
		vec.emplace_back(forward<Args>(args)...);
		ColorStep &cs = vec.back();
		if (cs.stepTime == 0)
			vec.pop_back();
		else
			total += cs.stepTime;
	}
	void Add(ColorStep &cs)
	{
		if (cs.stepTime == 0)
			return;
		cs.countdown = 0;
		vec.push_back(cs);
		total += cs.stepTime;
	}
	void Add(vector<ColorStep> &vcs)
	{
		for (auto &cs : vcs)
		{
			Add(cs);
		}
	}
	void Print(ColorStep &cs)
	{
		string color;
		switch (cs.status)
		{
			case GREEN: color = "绿灯"; break;
			case GREEN_BLINK: color = "绿闪"; break;
			case YELLOW: color = "黄灯"; break;
			case YELLOW_BLINK: color = "黄闪"; break;
			case ALLRED: color = "全红"; break;
			case RED: color = "红灯"; break;
			default: color = "灭灯";
		}
		cout << color << " step: " << cs.stepTime << "s, countdown: " << cs.countdown << endl;
	}
	void PrintCur()
	{
		if (vec.size() > 0)
		{
			Print(cur);
			//cout << "used = " << used << endl;
		}
	}
	void PrintAll()
	{
		if (vec.size() == 0)
			return;
		cout << "*****************************" << endl;
		for (auto &cs : vec)
		{
			Print(cs);
		}
		cout << "total = " << total << endl;
		cout << "*****************************" << endl << endl;
	}
	void CalCountdown();
protected:
};

class Channel
{
private:

public:
	UInt8 id = 0;
	UInt8 type = 0;
	UInt8 countdownId = 0;
	UInt8 countdownType = 0;
	ColorStepVector colorSteps;
	
	void Fill(const ChannelInfo &ci)
	{
		id = ci.channelId;
		type = ci.channelType;
		countdownId = ci.countdownId;
		countdownType = ci.countdownType;
	}
protected:
};

class Phase
{
private:
	
public:
	UInt8	phaseId = 0;							//当前运行的相位号
	UInt8	stageNum = 0;							//相位所处阶段号
	UInt16	splitTime = 0;							//相位绿信比时间
	UInt16	splitLeftTime = 0;						//相位执行时在绿信比内所剩余的时间
	UInt16	checkTime = 0;							//感应检测时间
	UInt64	vehDetectorBits = 0;					//相位关联的车检器
	UInt16	unitExtendTime = 0;						//单位延长绿
	UInt16	maxExtendTime = 0;						//最大可以延长的绿灯时间
	UInt8 	pedDetectorBits = 0;					//相位关联行人或公交检测器，bit0-bit7分别代表车检器1-8
	UInt16	advanceExtendTime = 0;					//优先延长时间
	UInt16	pedResponseTime = 0;					//行人请求响应时间
	bool	pedPhase = false;						//是否是行人相位
	bool	pedRequest = false;						//行人请求
	bool	motorRequest = false;					//机动车请求
	ColorStepVector motor;
	ColorStepVector pedestrian;
	time_t	beginTime = 0;							//相位开始运行时间
	
	void Fill(PhaseInfo &pi, const UInt8 stage);
	
	void operator>>(int t)
	{
		motor >> t;
		pedestrian >> t;
		if (!ReleasePeriod())
		{
			splitLeftTime = 0;
			return;
		}
		if (splitLeftTime == 0)
		{
			splitLeftTime = splitTime;
			beginTime = time(nullptr);
		}
		else
			splitLeftTime -= t;
	}
	
	void Extend(UInt16 t)
	{
		if (t == 0)
			return;
		if (ReleasePeriod())
		{
			splitTime += t;
			splitLeftTime += t;
			maxExtendTime -= t;
			INFO("phaseId %d extend %d second", phaseId, t);
		}
		motor.Extend(t);
		pedestrian.Extend(t);
	}
	
	bool FirstSec() const	//相位运行第1s
	{
		return (splitTime > 0 && splitLeftTime == splitTime);
	}

	bool GreenLastSec()	const//绿灯最后1秒
	{
		const ColorStep &cur = motor.Current();
		const ColorStep &next = motor.Next();
		return (cur.status == GREEN && next.status != GREEN && cur.stepTime == 1);
	}

	bool GreenEnd()	const //绿或者绿闪结束
	{
		const ColorStep &cur = motor.Current();
		const ColorStep &next = motor.Next();
		return ((cur.status == GREEN || cur.status == GREEN_BLINK)
				&& next.status != GREEN && next.status != GREEN_BLINK && cur.stepTime == 1);
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
		return (cur.status == GREEN_BLINK || cur.status == YELLOW || cur.status == ALLRED);
	}
	
	bool ReleasePeriod() const //是否是放行时期
	{
		const ColorStep &cur = motor.Current();
		return (cur.status == GREEN || cur.status == GREEN_BLINK || cur.status == YELLOW || cur.status == ALLRED);
	}
protected:
};

class Cycle
{
private:
	void SetCurrentStatus(Status status)
	{
		for (auto &p : phaseTable)
		{
			p.motor.SetCurrentStatus(status);
			p.pedestrian.SetCurrentStatus(status);
		}
		for (auto &c : channelTable)
			c.colorSteps.SetCurrentStatus(status);
	}
	
public:
	UInt16	cycleTime;							//当前周期总时间
	UInt16	leftTime;							//当前周期还剩下多少时间
	time_t	beginTime;							//周期开始运行时间
	const ControlRule rule;						//周期控制规则
	UInt8	curStage;							//当前阶段号
	UInt8	curPhase;							//当前放行相位
	const UInt8	maxStageNum;					//最大阶段号
	array<Channel, MAX_CHANNEL_NUM> channelTable;	//通道表
	array<Phase, MAX_PHASE_NUM>		phaseTable;		//相位表
	vector<UInt8>					phaseturn;		//相序
	
	Cycle(vector<ColorStep> &vec);
	Cycle(SchemeInfo &si);
	
	const UInt8 NextStage() const
	{
		if (maxStageNum < 2)
			return 0;
		else
			return (curStage + 1 > maxStageNum) ? 1 : (curStage + 1);
	}

	const bool StepInvalid(const UInt8 stageNum) //判断步进号是否无效
	{
		return ((stageNum == 0 && phaseTable[curPhase - 1].TransitionPeriod())	//单独步进在相位处于过渡期是无效的
				|| stageNum > maxStageNum);
	}

	void Step(const UInt8 stageNum)
	{
		if (StepInvalid(stageNum) || stageNum == 0 || stageNum == curStage)
			return;
		UInt8 phaseId = curPhase;
		for (UInt16 t = 0; t < cycleTime; t++)
		{
			MoveOneSec();
			if (phaseTable[phaseId - 1].TransitionPeriod() || stageNum == curStage)
				break;
		}
	}
	
	void SpecialCtrl(UInt8 mode)
	{
		switch (mode)
		{
			case TURNOFF_MODE: SetCurrentStatus(TURN_OFF); break;
			case YELLOWBLINK_MODE: SetCurrentStatus(YELLOW_BLINK); break;
			case ALLRED_MODE: SetCurrentStatus(ALLRED); break;
		}
	}
	
	void Extend(UInt16 t)
	{
		if (t == 0)
			return;
		cycleTime += t;
		leftTime += t;
		for (auto &p : phaseTable)
			p.Extend(t);
		for (auto &c : channelTable)
			c.colorSteps.Extend(t);
	}
	
	bool Begin() const
	{
		return cycleTime == leftTime;
	}

	bool Over() const
	{
		return leftTime == 1;
	}
	
	bool PhaseBegin() const
	{
		if (curPhase > 0 && curPhase <= MAX_PHASE_NUM)
			return phaseTable[curPhase - 1].FirstSec();
		else
			return false;
	}

	const Phase *GetCurPhase() const
	{
		if (curPhase > 0 && curPhase <= MAX_PHASE_NUM)
			return &phaseTable[curPhase - 1];
		else
			return nullptr;
	}

	void SetChannelStatus(ChannelStatusArray &channelStatus)
	{
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			channelStatus[i] = channelTable[i].colorSteps.Current().status;
		}
	}

	void MoveOneSec()	//每次向前移动1秒
	{
		for (auto &p : phaseTable)
		{
			p >> 1;
			if (p.ReleasePeriod())
			{
				curPhase = p.phaseId;
				curStage = p.stageNum;
				p.motor.PrintCur();
				//p.pedestrian.PrintCur();
			}
			//p.motor.PrintCur();
			//p.pedestrian.PrintCur();
		}
		for (auto &c : channelTable)
			c.colorSteps >> 1;
		if (leftTime > 0)
			leftTime--;
		else
			leftTime = cycleTime;
	}
	
	virtual void Excute()
	{
		if (beginTime == 0)
		{
			beginTime = time(nullptr);
			if (curPhase > 0 && curPhase <= MAX_PHASE_NUM)
				phaseTable[curPhase - 1].beginTime = beginTime;
		}
		else
			MoveOneSec();
	}
protected:
};

#endif
