#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <array>
#include "thread.h"
#include "timer.h"
#include "lamp.h"
#include "sem.h"
#include "can.h"
#include "singleton.h"
#include "channel.h"

class Light: public hik::thread, public hik::timer
{
private:
	typedef std::array<Lamp, MAX_CHANNEL_NUM> LampArray;
	LampArray lamps;				//所有灯组
	LampArray cache;				//所有灯组缓存
	LightBits bits;
	Can &can;
	hik::sem semForLight;
	hik::sem semForCtrl;
	//hik::timer timer;
	volatile int count = 0;	//定时器计数,一秒钟4次

public:
	Light() : can(Singleton<Can>::GetInstance())//, timer(std::bind(&Light::Timeout, std::ref(*this)))
	{
		UInt8 id = 1;
		for (auto &lamp: lamps)
		{
			lamp.id = id++;
		}
		id = 1;
		for (auto &lamp: cache)
		{
			lamp.id = id++;
		}
	}

	void Timeout()
	{
		if (++count == 4)
		{
			count = 0;
			lamps = cache;
			semForCtrl.post();
		}
		semForLight.post();
	}

	void LightChannel(UInt8 ctrlMode)
	{
		TscStatus status;
		switch (ctrlMode)
		{
			case TURNOFF_MODE: status = TURN_OFF; break;
			case YELLOWBLINK_MODE: status = YELLOW_BLINK; break;
			case ALLRED_MODE: status = ALLRED; break;
			default: status = INVALID;
		}
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			cache[i] = status;
		}
		semForCtrl.wait();
	}
	
	void LightChannel(const std::array<Channel, MAX_CHANNEL_NUM> &channelTable)
	{
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			cache[i] = channelTable[i].status;
		}
		semForCtrl.wait();
	}
	
	void Run()
	{
		auto light = [this](){
			for (auto &lamp : lamps)
				lamp.SetLightValue(bits);
			can.Send(bits);
		};
		lamps = cache;
		//timer.boot(250);	//定时250ms
		boot(250);	//定时250ms
		light();
		while (true)
		{
			if (semForLight.wait())
				light();
			else
			{
				ERR("Light::run sem wait fail");
				msleep(200);
			}
		}
	}
protected:
};

#endif
