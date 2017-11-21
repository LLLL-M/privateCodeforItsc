#ifndef __ITS_H__
#define __ITS_H__

#include <string>
#include "tsc.h"

//命名空间的前向声明，避免引用大量的头文件
namespace HikIpc
{
	class Ipc;
}
namespace HikCalculate
{
	class Calculate;
}
namespace HikPhasecontrol
{
	class Phasecontrol;
}
namespace HikClock
{
	class Clock;
}
namespace HikPhasedriver
{
	class Phasedriver;
}
namespace HikFaultlog
{
	class Faultlog;
}
namespace HikStrategycontrol
{
	class Strategycontrol;
}
namespace HikRedsigcheck
{
	class Redsigcheck;
}
namespace HikChannelHandle
{
    class ChannelHandle;
}

namespace HikIts
{
	class Protocol;
	using HikIpc::Ipc;
	using HikCalculate::Calculate;
	using HikPhasecontrol::Phasecontrol;
	using HikClock::Clock;
	using HikPhasedriver::Phasedriver;
	using HikFaultlog::Faultlog;
	using HikStrategycontrol::Strategycontrol;
	using HikRedsigcheck::Redsigcheck;
    using HikChannelHandle::ChannelHandle;

    class Its
	{
	private:
		static Protocol *ptl;
		static Ipc *ipc;
		static Calculate *calculate;
		static Phasecontrol *phasecontrol;
		static Clock *clock;
		static Phasedriver *phasedriver;
		static Faultlog *faultlog;
		static Strategycontrol *strategycontrol;
		static Redsigcheck *redsigcheck;
        static ChannelHandle *chhandle;
	public:
		Its(Protocol *p = NULL);
		bool ItsInit(Protocol *p = NULL);
		void ItsExit();
		void ItsThreadCheck();
        void ItsCtl(ControlType type, UInt8 schemeId, Int32 val);
        //void ItsChannelLock(ChannelLockParams *lockparams);
        //void ItsChannelUnlock();
        std::string & ItsReadFaultLog(int startLine, UInt32 lineNum, std::string & log);
		bool ItsWriteFaultLog(FaultLogType type, int value);
		void ItsClearFaultLog();
	protected:
		
	};
}

#endif
