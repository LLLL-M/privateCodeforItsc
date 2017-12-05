#pragma once

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
#include <atomic>
#include <bitset>
#include <map>
#include <string>

#include "thread.h"
#include "linux.h"

namespace hik
{
	class device;
}

class Collect : public hik::thread
{
private:
	hik::device &dev;
	hik::watchdog watchdog;
	std::atomic_bool	gpsSwitch;
	std::atomic_bool	watchdogSwitch;
	std::atomic<std::bitset<16>> pedKeyStatus;				//行人按键状态
	std::atomic<hik::KeyType> ctrlKeyStatus;		//控制按键状态

	void SetPedKeyStatus();	//设置行人按键状态
	void SetCtrlKeyStatus();//设置控制按键的状态
	void ProcessCtrlKey(hik::KeyType old, hik::KeyType now);	//处理5个控制按键的按键逻辑

public:
	Collect();
	
	void SetGps(bool flag);

	void SetWatchdog(bool flag);
	
	const std::bitset<16> GetPedKeyStatus()
	{
		std::bitset<16> value(pedKeyStatus);
		pedKeyStatus = 0;
		return value;
	}

	void Run();
protected:
};

#else

class Collect
{
public:
	void SetGps(bool flag) {}

	void SetWatchdog(bool flag) {}
	
	const std::bitset<16> GetPedKeyStatus() {return 0;}
	
	void start() {}	//启动线程时用
};

#endif

