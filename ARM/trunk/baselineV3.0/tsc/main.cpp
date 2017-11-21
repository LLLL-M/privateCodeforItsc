#include "singleton.h"
#include "communication.h"
#include "tsc.h"
#include "its.h"
#include "ctrl.h"
#include "log.h"
#include "can.h"
#include "device.h"
#include "memory.h"
#include "collect.h"

#define UNUSEDATTR __attribute__((unused))

int main(void)
{
	hik::memory UNUSEDATTR &memory = Singleton<hik::memory>::GetInstance();
	Log UNUSEDATTR &log = Singleton<Log>::GetInstance();
	hik::device UNUSEDATTR &dev = Singleton<hik::device>::GetInstance();
	Collect &collect = Singleton<Collect>::GetInstance();
	Can &can = Singleton<Can>::GetInstance();
	Tsc UNUSEDATTR &tsc = Singleton<Tsc>::GetInstance();
	Communication &communication = Singleton<Communication>::GetInstance();
	Its &its = Singleton<Its>::GetInstance();
	Ctrl UNUSEDATTR &ctrl = Singleton<Ctrl>::GetInstance();
#if 1
	can.start();
	collect.start();
	its.start();
#endif
	communication.start();
	while (1) 
	{
		sleep(10);
		//communication.SendRealtimeStat(R"({"ctrlType":0,"ctrlMode":0,"ctrlId":0})");
	}
	return 0;
}
