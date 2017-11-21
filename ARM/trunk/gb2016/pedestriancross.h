#ifndef __PEDESTRIANCROSS_H__
#define __PEDESTRIANCROSS_H__

#include <cassert>
#include "cycle.h"
#include "gpio.h"
#include "singleton.h"

class PedestrianCross : public Cycle
{
private:
	Gpio &gpio;
	UInt16	pedResponseTime;
	UInt8	pedDetectorBits;
	bool	pedRequest;
	
	void SetPedRequest()
	{
		if (gpio.GetPedKeyStatus() & pedDetectorBits)
			pedRequest = true;
	}
public:
	PedestrianCross(SchemeInfo &si) : Cycle(si), gpio(Singleton<Gpio>::GetInstance())
	{	//行人过街控制时断言相序中只有两个相位，并且第二个相位必须是行人相位
		assert(si.phaseturn.size() == 2 && si.phaseturn[1].pedPhase);
		pedResponseTime = si.phaseturn[1].pedResponseTime;
		pedDetectorBits = si.phaseturn[1].pedDetectorBits;
		pedRequest = false;
	}
	
	virtual void Excute()
	{
		Phase &p = phaseTable[curPhase - 1];
		if (!p.pedPhase)
		{	//机动车相位
			SetPedRequest();
			if (!pedRequest && (p.splitLeftTime <= pedResponseTime || p.GreenLastSec()))
			{	//只有没有行人请求,且机动车相位在绿信比剩余时间小于等于行人响应时间或绿灯最后1s时才会驻留
				return;
			}
		}
		else
		{	//行人相位
			if ((gpio.GetPedKeyStatus() & pedDetectorBits) && p.WindowPeriod())
				Extend(min(p.unitExtendTime, p.maxExtendTime));
		}
		Cycle::Excute();
	}

protected:
};

#endif
