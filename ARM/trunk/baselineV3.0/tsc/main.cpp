#include "singleton.h"
#include "communication.h"
#include "tsc.h"
#include "its.h"
#include "ctrl.h"
#include "log.h"
#include "device.h"
#include "memory.h"
#include "collect.h"
#include "threadpool.h"
#include "timer.h"
#include "detector.h"

#define UNUSEDATTR __attribute__((unused))

class Clock : public hik::timer
{
private:
	int count;
public:
	Clock()
	{
		count = 0;
	}

	~Clock() {}
	
	void Timeout()
	{
		hik::device &dev = Singleton<hik::device>::GetInstance();
		Its &its = Singleton<Its>::GetInstance();
		Ctrl UNUSEDATTR &ctrl = Singleton<Ctrl>::GetInstance();
		hik::threadpool &pool = Singleton<hik::threadpool>::GetInstance();

		if (++count == LIGHT_PER_SEC)
		{
			count = 0;
			pool.addtask(std::bind(&Its::Run, std::ref(its)));
			if (ctrl.TimeIsUp())
				pool.addtask(std::bind(&Ctrl::Timeout, std::ref(ctrl)));
		}
		else
			pool.addtask(std::bind(&hik::device::light, std::ref(dev)));
	}
};

int main(void)
{
	hik::memory UNUSEDATTR &memory = Singleton<hik::memory>::GetInstance();
	Log UNUSEDATTR &log = Singleton<Log>::GetInstance();
	DetectorArray UNUSEDATTR &detectors = Singleton<DetectorArray>::GetInstance();
	Tsc UNUSEDATTR &tsc = Singleton<Tsc>::GetInstance();
	Communication &communication = Singleton<Communication>::GetInstance();
	hik::device UNUSEDATTR &dev = Singleton<hik::device>::GetInstance();
	Collect &collect = Singleton<Collect>::GetInstance();
	Its UNUSEDATTR &its = Singleton<Its>::GetInstance();
	Ctrl UNUSEDATTR &ctrl = Singleton<Ctrl>::GetInstance();
	hik::threadpool &pool = Singleton<hik::threadpool>::GetInstance();
	Clock &clk = Singleton<Clock>::GetInstance();

	pool.start(3);
	communication.start();
	collect.start();
	its.Run();
	clk.boot(1000 / LIGHT_PER_SEC);	//定时250ms
	dev.light();

	while (1) 
	{
		sleep(10);
	}
	return 0;
}
