#ifndef __BUSADVANCE_H__
#define __BUSADVANCE_H__

#include <algorithm>
#include <array>
#include <set>
#include "cycle.h"
#include "detector.h"
#include "gpio.h"
#include "singleton.h"

class BusAdvance : public Cycle
{
private:
	static bitset<MAX_PHASE_NUM> requests;		//bit0-bit15分别代表相位1-相位16下一周期是否有相位请求
	static bitset<MAX_PHASE_NUM> busPhases;		//bit0-bit15分别代表相位1-相位16是否有公交车请求
	Gpio &gpio;
	DetectorArray &detectorArray;
	enum { TIMEOUT = 100 };
	int timeout;
	
	SchemeInfo & Adjust(SchemeInfo &si)
	{
		for (auto &p : si.phaseturn)
		{
			if (p.phaseId == 0 || p.phaseId > MAX_PHASE_NUM)
				continue;
			if (p.motorRequest)
				requests[p.phaseId - 1] = true;
			else
			{
				p.motorRequest = requests[p.phaseId - 1];
				requests[p.phaseId - 1] = false;
			}
		}
		return si;
	}
	
	void SetOtherPhaseRequest(const UInt64 vehEnterInfo)
	{
		bool nextCycle = true;	//是否把请求设置到下个周期的对应相位
		const UInt8 pedKeyRequest = gpio.GetPedKeyStatus();
		auto PhaseRequest = [&vehEnterInfo, &pedKeyRequest](Phase &p)->bool{
			return ((p.pedDetectorBits & pedKeyRequest) 
					|| (p.vehDetectorBits & vehEnterInfo));
		};
		auto BusRequest = [&pedKeyRequest](Phase &p)->bool{
			//如果是机动车相位且有行人请求,则说明有公交车到来
			return (!p.pedPhase && (p.pedDetectorBits & pedKeyRequest));
		};
		
		if (vehEnterInfo == 0 && pedKeyRequest == 0)
			return;
		for (auto &id : phaseturn)
		{
			if (id == 0 || id > MAX_PHASE_NUM)
				continue;
			auto &p = phaseTable[id - 1];
			if (id == curPhase)
			{	//当前相位
				nextCycle = false;
				if (p.FirstSec())
					busPhases[id - 1] = false;
				if (PhaseRequest(p))
				{	
					const ColorStep &cur = p.motor.Current();
					if (cur.status == GREEN)
					{
						if (BusRequest(p) && cur.countdown < p.advanceExtendTime + 1)
						{
							UInt16 gap = p.advanceExtendTime - cur.countdown + 1;//加1是因为倒计时都是从1开始
							if (gap <= p.maxExtendTime)
								Extend(gap);
							else
							{	//如果当前相位无法再延长公交通行需要的时间,则放到下周期再请求
								Extend(p.maxExtendTime);
								busPhases[id - 1] = true;
								requests[id - 1] = true;
							}
						}
					}
					else
					{
						if (BusRequest(p))
							busPhases[id - 1] = true;
						requests[id - 1] = true;
					}
				}
			}
			else
			{	//其他相位
				if (PhaseRequest(p))
				{
					if (nextCycle)
						requests[id - 1] = true;
					else
						p.motorRequest = true;
					if (BusRequest(p))
						busPhases[id - 1] = true;
				}
			}
			
		}
	}

	UInt8 GetNextPhase()
	{
		bool nextCycle = true;
		for (auto &id : phaseturn)
		{
			if (id == curPhase)
			{
				nextCycle = false;
				continue;
			}
			if ((nextCycle && requests[id - 1]) || (!nextCycle && phaseTable[id - 1].motorRequest))
				return id;	//如果下周期有请求或者后面的相位有请求,则返回相位号表明后面有相位要放行,不能停留在当前
		}
		return 0;
	}
public:
	BusAdvance(SchemeInfo &si) : Cycle(Adjust(si)), gpio(Singleton<Gpio>::GetInstance()), detectorArray(Singleton<DetectorArray>::GetInstance())
	{
		timeout = 0;	
	}

	static void Init()
	{
		requests.set();
		busPhases.reset();
	}
	
	virtual void Excute()
	{
		UInt64 vehEnterInfo = detectorArray.GetEnter(time(nullptr) - 1);
		Phase &p = phaseTable[curPhase - 1];
		SetOtherPhaseRequest(vehEnterInfo);
		if (busPhases.none() && p.WindowPeriod() && (vehEnterInfo & p.vehDetectorBits))
			Extend(min(p.unitExtendTime, p.maxExtendTime));
		if (p.GreenLastSec() && GetNextPhase() == 0)
		{
			if (++timeout <= TIMEOUT)
				return;	//如果在绿灯最后一秒且后续没有机动车请求的相位需要放行时，停留在此步一直检测直到有车请求相位放行或者超时为止
			Init();
		}
		timeout = 0;
		int i, num = leftTime - 1;
		for (i = 0; i < num; i++)
		{
			Cycle::Excute();
			if (phaseTable[curPhase - 1].motorRequest)
				break;
		}
	}
protected:
};

#endif
