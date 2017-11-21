#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <cstring>
#include <ctime>
#include "hik.h"
#include "tsc.h"

namespace HikIts
{
	class Protocol
	{
	private:
		struct lamp_t
		{
			UInt16 L0:3;
			UInt16 L1:3;
			UInt16 L2:3;
			UInt16 L3:3;
			UInt16 unused:4;
		};
	public:
		enum PerSecTimes
		{
			TWO = 2,
			FOUR = 4,
			EIGHT = 8,
			TEN = 10,
		};
		const PerSecTimes perSecTimes; //每秒定时次数,默认4次
		UInt8 aheadOfTime;
		McastInfo mcastinfo;
		bool isContinueRun;			//黄闪等特殊控制之后是否接着之前的周期继续运行
		bool enableGPS;				//是否使能GPS校时
		bool addTimeToFirstPhase;	//感应协调联合控制时是否把剩余时间累加到第一个相位
		UInt16 redFlashSec;	//红灯倒计时闪烁秒数
		bool isYellowFlash;
		Protocol(McastInfo * info = NULL, PerSecTimes times = FOUR) : perSecTimes(times)
		{
			aheadOfTime = 6;	//每次提前计算下个周期的秒数
			if (info == NULL)
			{
				mcastinfo.enableRedSignal = false;
				std::strcpy(mcastinfo.mcastIp, "239.255.255.44");
				mcastinfo.mcastPort = 8168;
				mcastinfo.deviceId = 0x03;
			}
			isContinueRun = false;
			enableGPS = false;
			addTimeToFirstPhase = false;
			redFlashSec = 6;
			isYellowFlash = false;
		}
		
		UInt16 get_lamp_value(UInt16 *lights, int n)
		{
			lamp_t *p = (lamp_t *)(lights);
			UInt16 value = 0;
			switch (n) 
			{   
				case 0: value = p->L0; break;
				case 1: value = p->L1; break;
				case 2: value = p->L2; break;
				case 3: value = p->L3; break;
				default: break;
			}   
			
			return value;
		}
		void put_lamp_value(UInt16 *lights, int n, UInt16 value)
		{
			lamp_t *p = (lamp_t *)(lights);
			switch (n) 
			{
				case 0:	p->L0 = value; break;
				case 1:	p->L1 = value; break;
				case 2:	p->L2 = value; break;
				case 3:	p->L3 = value; break;
				default: break;
			}
		}
        virtual void ItsCustom(LineQueueData *data) {}
        virtual void ItsCountDownOutput(LineQueueData *data) {}
		virtual int ItsControlStatusGet(){return 0;}
		virtual void SetStartTime(UInt8 & yellowFlashTime, UInt8 & allRedTime)
		{
			yellowFlashTime = 6;
			allRedTime = 6;
		}
		virtual bool FillCalInfo(CalInfo *calInfo, UInt8 schemeId, std::time_t calTime) = 0;
        virtual void IgnorePhaseDeal(CalInfo *calInfo) {}
		virtual UInt64 ItsReadVehicleDetectorData() { return 0; } //读车检板数据
        virtual void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data) {}
        virtual void ItsGetCurRunData(LineQueueData *data) {}
        virtual void ItsSetCurRunData(const LineQueueData *data) {}
        virtual void ItsGetConflictChannel(UInt32 *conflictChannels) {}
        virtual void channelLockTransition(unsigned char lockFlag, unsigned char *curStatus, unsigned char *lockstatus) {}
        virtual unsigned int GetBootTransitionTime(){ return 0; }
        virtual unsigned char ChannelControl(unsigned char *chan) { return 0; }
        virtual void ItsFaultCheck(int boardNum, UInt16 *lightValues) {}
        virtual void ItsLight(int boardNum, UInt16 *lightValues) {}
		virtual void GPS_led_on() {}
		virtual void GPS_led_off() {}
	protected:
	};
}

#endif
