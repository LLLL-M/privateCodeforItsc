#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "thread.h"
#include "timer.h"
#include "lamp.h"
#include "sem.h"
#include "can.h"
#include "singleton.h"

class Light: public HikThread::Thread, public Timer
{
private:
	typedef array<Lamp, MAX_CHANNEL_NUM> LampArray;
	LampArray lamps;				//所有灯组
	LampArray cache;				//所有灯组缓存
	LightBits bits;
	Can &can;
	Its &its;
	HikSem::Sem semForLight;
	HikSem::Sem semForCtrl;
	int count = 0;	//定时器计数,一秒钟4次
public:
	Light(Its &t) : its(t), can(Singleton<Can>::GetInstance())
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
	void SendChannelStatus(const ChannelStatusArray &channelStatus)
	{
		int i;
		for (i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			cache[i] = channelStatus[i];
		}
		semForCtrl.wait();
	}
	void timeout()
	{
		if (++count == 4)
		{
			count = 0;
			lamps = cache;
			semForCtrl.post();
		}
		semForLight.post();
	}
	void run(void *arg)
	{
		auto light = [this](){
			for (auto &lamp : lamps)
				lamp.SetLightValue(bits);
			can.Send(bits);
			its.SetCurLampStatus(bits);
		};
		can.start();
		lamps = cache;
		timerstart(250);	//定时250ms
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
