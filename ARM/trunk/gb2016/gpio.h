#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef HARDWARE_MODEL
#include <atomic>
#include <bitset>
#if (HARDWARE_MODEL == 300)	//TSC300
#include <map>
#include <string>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "thread.h"
#include "io_ioctl.h"
#include "CPLD.h"
#include "hik.h"

typedef enum
{
	NONE_KEY = 0,
	AUTO_KEY = 1,
	MANUAL_KEY = 2,
	YELLOWBLINK_KEY = 3,
	ALLRED_KEY = 4,
	STEP_KEY = 5,
} CtrlKeyType;

/**********************
控制按键存放在一个UInt8中，只用了5bit
bit0:自动按键
bit1:手动按键
bit2:黄闪按键
bit3:全红按键
bit4:步进按键
**********************/
class Log;

class Gpio : public HikThread::Thread
{
private:
	static int iofd;
#if (HARDWARE_MODEL == 300)	//TSC300
	int keyfd;
	map<string, int> button;	//存放自动、手动、黄闪、全红、步进几个按键
	map<string, int> other;		//存放其他8个自定义按键
#endif
	Log &log;
	int gpsfd;
	atomic_int watchdogfd;
	atomic_bool	gpsSwitch;
	atomic_bool	watchdogSwitch;
	bool gpsLedOn;
	bool runLedOn;
	bool hardFlashHigh;
	atomic_uchar pedKeyStatus;				//行人按键状态
	atomic_uchar wirelessKeyStatus;			//无线按键状态
	atomic<CtrlKeyType> ctrlKeyStatus;		//控制按键状态
	bitset<8> ctrlKey;					//控制按键值
	
	int Opentty(int port, int baudRate, int dataBits, int stopBits, char parity);//打开串口
	void GetGPSTime(UInt32 timezone = 8 * 3600);	//获取GPS时间
	void GpsLedBlink();		//GPS LED灯闪烁
	void RunningLedBlink();	//程序运行LED灯闪烁
	void HardFlashCtrl();	//硬黄闪控制
	void SetPedKeyStatus();	//设置行人按键状态
	void SetWirelessKeyStatus();	//设置无线按键状态
	void SetCtrlKeyLed();	//设置5个控制按键的LED灯
	void ProcessCtrlKey(bitset<8> &old, bitset<8> &now);	//处理5个控制按键的按键逻辑
	void SetCtrlKeyStatus();	//设置5个控制按键的状态
	void FeedWatchdog();

public:
	Gpio();
	~Gpio()
	{
		if (iofd != -1)
			close(iofd);
		if (gpsfd != -1)
			close(gpsfd);
	}
	
	//设置前面板按键状态
	static void SetCtrlKeyStatus(int status)
	{
#if (HARDWARE_MODEL == 300)	//TSC300
		ioctl(iofd, IO_SET_BUTTON_STATUS, &status);
#endif
	}
	
	void SetGps(bool flag)
	{
		if (flag)
			gpsSwitch = true;
		else
		{
			gpsSwitch = false;
#if (HARDWARE_MODEL == 500)		//TSC500
			ioctl(iofd, LED_OUTPUT2_0);
#elif (HARDWARE_MODEL == 300)	//TSC300
			int arg = ARG_GPS_LED_OFF;
			ioctl(iofd, IO_SET_PIN_STATUS, &arg);
#endif
		}
	}

	void SetWatchdog(bool flag);
	
	const UInt8 GetPedKeyStatus()
	{
#if 0
		UInt8 value = pedKeyStatus;
		pedKeyStatus = 0;
		return value;
#else	//为了调试行人按键方便些，所以暂时改为使用无线按键来触发
		return GetWirelessKeyStatus();
#endif
	}
	
	const UInt8 GetWirelessKeyStatus()
	{
		UInt8 value = wirelessKeyStatus;
		wirelessKeyStatus = 0;
		return value;
	}
	
	const CtrlKeyType GetCtrlKeyStatus()
	{
		CtrlKeyType value = ctrlKeyStatus;
		ctrlKeyStatus = NONE_KEY;
		return value;
	}
	
	void run(void *a);
protected:
};
#endif

#endif
