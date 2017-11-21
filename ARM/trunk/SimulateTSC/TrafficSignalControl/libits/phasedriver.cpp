#include <cstring>
#include "protocol.h"
#include "ipc.h"
#include "phasedriver.h"

using namespace HikPhasedriver;

void Phasedriver::Light(UInt8 *allChannel, UInt8 times)
{
	int i;
	UInt16 value;
	UInt16 *nOutLamp = lightValues;
	UInt8 half = ptl.perSecTimes / 2;

	for (i = 0; i < MAX_CHANNEL_NUM; i++) 
	{
		//根据通道所处状态找出这个通道应该所赋的值
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK: value = (times > half) ? LGREEN : LOFF; break;	
			case YELLOW: value = LYELLOW; break;
			case YELLOW_BLINK: value = (times > half) ? LYELLOW : LOFF; break;
			case RED: value = LRED; break;
			case RED_BLINK: value = (times > half) ? LRED : LOFF; break;
			case RED_YELLOW: value = LREDYELLOW; break;
			case ALLRED: value = LRED; break;
			case OFF_GREEN: value = (times == 2) ? LOFF : LGREEN; break;
			case OFF_YELLOW: value = (times == 2) ? LOFF : LYELLOW; break;
			case OFF_RED: value = (times == 2) ? LOFF : LRED;  break;
			//case INVALID: value = LRED; break;	//默认没有配置的通道全部为红灯
			case INVALID: value = LOFF; break;	//默认没有配置的通道全部为灭灯
			default: value = LOFF; break;
		}
		//给这个通道赋值
		ptl.put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0)
			nOutLamp++;
	}
	ptl.ItsLight(MAX_BOARD_NUM, lightValues);
	//主控板运行指示灯
	msleep(50); //延时50ms好让能够把刚才点灯的实际信息反馈过来用以检测
	ptl.ItsFaultCheck(MAX_BOARD_NUM, lightValues);
}

void Phasedriver::run(void *arg)
{
	Ipc::ChannelStatusMsg msg;
	int perSecTimes = ptl.perSecTimes;    //每秒定时次数
	UInt8 times = 0;

	std::memset(&msg, 0, sizeof(msg));
	while (1)
	{
		if (times % perSecTimes == 0)
		{
			ipc.MsgRecv(msg);	//阻塞接收点灯消息
			times = 0;
		}
		times++;
		Light(msg.allChannels, times);
		ipc.SemWaitTimerForPhaseDrv();
	}
}

Phasedriver::Phasedriver(Protocol & p, Ipc & i) : ptl(p), ipc(i)
{
	std::memset(lightValues, 0, sizeof(lightValues));
	start();
}
