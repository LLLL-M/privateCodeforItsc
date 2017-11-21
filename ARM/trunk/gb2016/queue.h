#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <queue>
#include <atomic>
#include "thread.h"
#include "sem.h"
#include "cycle.h"
#include "singlespot.h"
#include "pedestriancross.h"
#include "inductive.h"
#include "busadvance.h"

class Queue: public HikThread::Thread
{
private:
	queue<Cycle *> q;
	HikSem::Sem sem;
	atomic_int aheadOfTime;
	Its &its;
	
public:
	Queue(Its &_its) : its(_its)
	{
		aheadOfTime = 0;
		vector<ColorStep> vec;
		its.StartInit(vec);
		q.emplace(new Cycle(vec));
	}
	
	void CalNextCycle(int _aheadOfTime = 0)
	{
		if (q.empty())
		{
			aheadOfTime = _aheadOfTime;
			sem.post();
		}
	}

	void Clear()
	{
		int n = q.size();
		for (int i = 0; i < n; i++)
			q.pop();
	}
	
	Cycle *Read()
	{
		if (q.empty())
		{
			CalNextCycle();
			return nullptr;
		}
		Cycle *c = q.front();
		q.pop();
		return c;
	}

	void run(void *arg)
	{
		SchemeInfo schemeInfo;
		ControlRule old = {0, 0, 0};
		while (true)
		{
			if (sem.wait())
			{
				INFO("calculate next cycle");
				ControlRule rule = its.nextRule;
				if ((rule.ctrlType == LOCAL_CONTROL && !its.GetLocalRule(aheadOfTime, rule))
					|| !its.FillSchemeInfo(rule, schemeInfo))
				{
					its.Downgrade();
				}
				else
				{
					if (old.ctrlMode != SINGLE_SPOT_OPTIMIZE && rule.ctrlMode == SINGLE_SPOT_OPTIMIZE)
						SingleSpot::Init();
					else if (old.ctrlMode != INDUCTIVE_MODE && rule.ctrlMode == INDUCTIVE_MODE)
						Inductive::Init();
					else if (old.ctrlMode != BUS_ADVANCE_MODE && rule.ctrlMode == BUS_ADVANCE_MODE)
						BusAdvance::Init();
					old = rule;

					if (rule.ctrlMode == FIXEDCYCLE_MODE || rule.ctrlMode == COORDINATE_MODE)
						q.emplace(new Cycle(schemeInfo));
					else if (rule.ctrlMode == SINGLE_SPOT_OPTIMIZE)
						q.emplace(new SingleSpot(schemeInfo));
					else if (rule.ctrlMode == PEDESTRIAN_INDUCTIVE_MODE)
						q.emplace(new PedestrianCross(schemeInfo));
					else if (rule.ctrlMode == INDUCTIVE_MODE)
						q.emplace(new Inductive(schemeInfo));
					else if (rule.ctrlMode == BUS_ADVANCE_MODE)
						q.emplace(new BusAdvance(schemeInfo));
					else if (rule.SpecialMode() && rule.ctrlType == LOCAL_CONTROL)
						its.SetRule(rule);	//本地特殊控制时需要切换当前工作模式
				}
			}
			else
			{
				ERR("Queue::run sem wait fail");
				msleep(200);
			}
		}
	}
	
protected:
};

#endif
