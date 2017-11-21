#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义

//#include <sys/ioctl.h>
//#include <linux/watchdog.h>
#include <unistd.h>
#include "ctrl.h"
#include "device.h"
#include "singleton.h"
#include "collect.h"
#include "log.h"

inline void Collect::GpsLedBlink()
{
	static bool gpsLedOn = true;
	if (gpsLedOn)
		dev.led_on(hik::GPS_LED);
	else
		dev.led_off(hik::GPS_LED);
	gpsLedOn = !gpsLedOn;
}

inline void Collect::RunningLedBlink()
{
	static bool runLedOn = true;
	if (runLedOn)
		dev.led_on(hik::RUNNING_LED);
	else
		dev.led_off(hik::RUNNING_LED);
	runLedOn = !runLedOn;
}

inline void Collect::SetPedKeyStatus()
{
	std::bitset<16> status = dev.get_ped_status();
	std::bitset<16> current = pedKeyStatus;
	if (status.any())
		pedKeyStatus = (status | current);
}

void Collect::ProcessCtrlKey(hik::KeyType old, hik::KeyType now)
{
	if ((old == hik::AUTO_KEY && now != hik::MANUAL_KEY)	//不是由自动到手动
		|| (old != hik::MANUAL_KEY && old != hik::STEP_KEY && now == hik::STEP_KEY)	//不是手动到步进或者步进到步进
		|| (old == now && now != hik::STEP_KEY))	//新旧按键一样且不是步进
	{	//以上都是无效的按键,反馈当前的按键状态
		dev.feedback_keystatus(ctrlKeyStatus);
		return;
	}
	ControlRule rule(KEY_CONTROL, SYSTEM_MODE);
	if (old == hik::AUTO_KEY)
	{	//由自动到手动
		ctrlKeyStatus = hik::MANUAL_KEY;
		//log.Write("auto key pressed, recovery auto control");
	}
	else
	{
		if (now == hik::AUTO_KEY)
		{	//由手动到自动
			ctrlKeyStatus = hik::AUTO_KEY;
			//log.Write("manual key pressed!");
		}
		else if (now == hik::YELLOWBLINK_KEY)
		{	//执行黄闪
			ctrlKeyStatus = hik::YELLOWBLINK_KEY;
			rule.ctrlMode = YELLOWBLINK_MODE;
			//log.Write("yellow flash key pressed!");
		}
		else if (now == hik::ALLRED_KEY)
		{	//执行全红
			ctrlKeyStatus = hik::ALLRED_KEY;
			rule.ctrlMode = ALLRED_MODE;
			//log.Write("allred key pressed!");
		}
		else if (now == hik::STEP_KEY)
		{	//执行步进
			ctrlKeyStatus = hik::STEP_KEY;
			rule.ctrlMode = STEP_MODE;
			//log.Write("step key pressed!");
		}
		else if (now == hik::EAST_KEY)
		{
			ctrlKeyStatus = hik::EAST_KEY;
			//log.Write("east key pressed!");
		}
		else if (now == hik::SOUTH_KEY)
		{
			ctrlKeyStatus = hik::SOUTH_KEY;
			//log.Write("south key pressed!");
		}
		else if (now == hik::WEST_KEY)
		{
			ctrlKeyStatus = hik::WEST_KEY;
			//log.Write("west key pressed!");
		}
		else if (now == hik::NORTH_KEY)
		{
			ctrlKeyStatus = hik::NORTH_KEY;
			//log.Write("north key pressed!");
		}
		else if (now == hik::EWS_KEY)
		{
			ctrlKeyStatus = hik::EWS_KEY;
			//log.Write("east_west_straight key pressed!");
		}
		else if (now == hik::SNS_KEY)
		{
			ctrlKeyStatus = hik::SNS_KEY;
			//log.Write("south_north_straight key pressed!");
		}
		else if (now == hik::EWL_KEY)
		{
			ctrlKeyStatus = hik::EWL_KEY;
			//log.Write("east_west_left key pressed!");
		}
		else if (now == hik::SNL_KEY)
		{
			ctrlKeyStatus = hik::SNL_KEY;
			//log.Write("south_north_left key pressed!");
		}
		else if (now == hik::FUNCTION1_KEY)
		{
			ctrlKeyStatus = hik::FUNCTION1_KEY;
			//log.Write("function1 key pressed!");
		}
		else if (now == hik::FUNCTION2_KEY)
		{
			ctrlKeyStatus = hik::FUNCTION2_KEY;
			//log.Write("function2 key pressed!");
		}
		else if (now == hik::FUNCTION3_KEY)
		{
			ctrlKeyStatus = hik::FUNCTION3_KEY;
			//log.Write("function3 key pressed!");
		}
		else if (now == hik::FUNCTION4_KEY)
		{
			ctrlKeyStatus = hik::FUNCTION4_KEY;
			//log.Write("function4 key pressed!");
		}
	}
	dev.feedback_keystatus(ctrlKeyStatus);
	Singleton<Ctrl>::GetInstance().SetRule(rule);
}

void Collect::SetCtrlKeyStatus()
{
	hik::KeyType current = dev.read_keyboard();
	if (current != hik::NONE_KEY)
	{
		ProcessCtrlKey(ctrlKeyStatus, current);
		return;
	}
	current = dev.read_wireless();
	if (current != hik::NONE_KEY)
		ProcessCtrlKey(ctrlKeyStatus, current);
}

Collect::Collect() : dev(Singleton<hik::device>::GetInstance())
{
	gpsSwitch = true;		//默认开启GPS
	watchdogSwitch = false;	//默认关闭watchdog
	pedKeyStatus = 0;
	ctrlKeyStatus = hik::AUTO_KEY;
	//展会使用默认让GPS、RF、4G、WIFI指示灯全亮
	dev.led_on(hik::RUNNING_LED);
	dev.led_on(hik::GPS_LED);
	dev.led_on(hik::RF_LED);
	if (dev.sim_exist())
		dev.led_on(hik::SIM_LED);
	if (dev.wifi_exist())
		dev.led_on(hik::WIFI_LED);
	dev.feedback_keystatus(hik::AUTO_KEY);
}

inline void Collect::SetGps(bool flag)
{
	if (flag)
		gpsSwitch = true;
	else
	{
		gpsSwitch = false;
		dev.led_off(hik::GPS_LED);
	}
}

inline void Collect::SetWatchdog(bool flag)
{
	bool old = watchdogSwitch;
	if (!old && flag)
	{	//watchdog从关闭到打开
		watchdogSwitch = true;
		watchdog.appctl();
	}
	else if (old && !flag)
	{	//watchdog从打开到关闭
		watchdogSwitch = false;
		watchdog.sysctl();
	}
}

inline const std::bitset<16> Collect::GetPedKeyStatus()
{
	std::bitset<16> value(pedKeyStatus);
	pedKeyStatus = 0;
	return value;
}	

void Collect::Run()
{
	int count = 0;
	
	while (true)
	{
		//SetPedKeyStatus();
		if (++count >= 3)
		{
			//SetCtrlKeyStatus();
			RunningLedBlink();
			count = 0;
#if 0
			if (gpsSwitch && dev.get_gps())
				GpsLedBlink();
#endif
			watchdog.feed();
		}
		usleep(100000);
	}
}

#endif
