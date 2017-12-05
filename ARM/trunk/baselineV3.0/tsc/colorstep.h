#pragma once

#include <vector>
#include "hik.h"
#include "common.h"

struct ColorStep
{
	UInt16 stepTime;
	TscStatus status;
	UInt16 countdown;

	void Set(UInt16 _stepTime = 0, TscStatus _status = INVALID, UInt16 _countdown = 0)
	{
		stepTime = _stepTime;
		status = _status;
		countdown = _countdown;
	}

	ColorStep(UInt16 _stepTime = 0, TscStatus _status = INVALID, UInt16 _countdown = 0)
	{
		Set(_stepTime, _status, _countdown);
	}

	bool PassPeriod() const
	{
		return (status == GREEN || status == GREEN_BLINK || status == YELLOW || status == ALLRED);
	}

	bool TransitionPeriod() const
	{
		return (status == GREEN_BLINK || status == YELLOW || status == ALLRED);
	}

	void Print() const
	{
		string color;
		switch (status)
		{
			case GREEN: color = "绿灯"; break;
			case GREEN_BLINK: color = "绿闪"; break;
			case YELLOW: color = "黄灯"; break;
			case YELLOW_BLINK: color = "黄闪"; break;
			case ALLRED: color = "全红"; break;
			case RED: color = "红灯"; break;
			default: color = "灭灯";
		}
		cout << color << " step: " << stepTime << "s, countdown: " << countdown << endl;
	}
};

class ColorStepVector
{
private:
	int total = 0;
	int used = -1;
	unsigned int pos = 0;
	ColorStep cur = {0, RED, 0};
	std::vector<ColorStep> vec;

public:
	bool Move(int t)
	{
		int sum = 0;
		
		if (vec.empty() || t <= 0)
			return false;
		used += t;
		if (used >= total || used < 0)
		{
			used = total;
			cur.Set(0, RED, 0);
			return false;
		}
		for (unsigned int i = 0; i < vec.size(); i++)
		{
			sum += vec[i].stepTime;
			if (sum > used)
			{
				cur = vec[i];
				pos = i;
				cur.stepTime = (UInt16)(sum - used);
				cur.countdown -= vec[i].stepTime - cur.stepTime;
				break;
			}
		}
		return true;
	}

	const ColorStep & Current() const
	{
		return cur;
	}

	ColorStep Next() const
	{
		if (vec.empty() || pos + 1 >= vec.size())
			return ColorStep(0, RED, 0);
		else
			return vec[pos + 1];
	}

	bool Inuse() const
	{
		return (used > -1 && used < total);
	}

	UInt16 Used() const
	{
		return (used == -1) ? 0 : (UInt16)used;
	}

	UInt16 Left() const
	{
		return (UInt16)((used == -1) ? total : (total - used));
	}

	UInt16 Total() const
	{
		return (UInt16)total;
	}

	UInt16 Green() const
	{
		UInt16 sum = 0;
		for (auto &i: vec)
		{
			if (i.status == GREEN || i.status == GREEN_BLINK)
				sum += i.stepTime;
			else
				break;
		}
		return sum;
	}

	UInt16 NonAllRed() const
	{
		UInt16 sum = 0;
		for (auto &i: vec)
		{
			if (i.status != ALLRED && i.status != RED)
				sum += i.stepTime;
			else
				break;
		}
		return sum;
	}

	bool Over() const
	{
		return (total <= used + 1);
	}

	void SetOver()
	{
		used = total;
		pos = vec.size();
		cur.Set(0, RED, 0);
	}

	void Reset()
	{
		used = -1;
		pos = 0;
		cur.Set(0, RED, 0);
	}

	void SetCurrentStatus(const TscStatus status)
	{
		cur.status = status;
	}

	void SetCurrentCountdown(const UInt16 countdown)
	{
		cur.countdown = countdown;
	}

	bool ExtendGreen(int t)
	{
		if (t == 0)
			return true;
		if (vec.empty() || Over() || (vec[pos].status != GREEN && vec[pos].status != GREEN_BLINK))
			return false;
		//如果相位处于绿灯或者绿闪状态则把时间增加给此相位
		cur.stepTime += t;
		cur.countdown += t;
		vec[pos].stepTime += t;
		vec[pos].countdown += t;
		total += t;
		return true;
	}

	void Extend(UInt16 sec)
	{
		if (vec.empty())
			return;
		vec[0].stepTime += sec;
		vec[0].countdown += sec;
		total += sec;
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

	void Add(const ColorStep &cs)
	{
		if (cs.stepTime == 0)
			return;
		//cs.countdown = 0;
		vec.push_back(cs);
		total += cs.stepTime;
	}

	void Add(std::vector<ColorStep> &vcs)
	{
		for (auto &cs : vcs)
			Add(cs);
	}

	int Jump()
	{
		UInt16 sum = 0;
		for (unsigned int i = 0; i < vec.size(); i++)
		{
			if (vec[i].TransitionPeriod())
			{
				pos = i;
				cur = vec[i];
				break;
			}
			else
				sum += vec[i].stepTime;
		}
		used = sum;
		return sum;
	}

	void PrintCur()
	{
		cur.Print();
	}
	
	void PrintAll()
	{
		if (vec.size() == 0)
			return;
		cout << "*****************************" << endl;
		for (auto &cs : vec)
		{
			cs.Print();
		}
		cout << "total = " << total << endl;
		cout << "*****************************" << endl << endl;
	}

protected:
};
