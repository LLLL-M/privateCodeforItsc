#ifndef __STRATEGYCONTROL_H_
#define __STRATEGYCONTROL_H_

#include "tsc.h"
#include "thread.h"

namespace HikIpc
{
	class Ipc;
}
namespace HikIts
{
	class Its;
}
namespace HikStrategycontrol
{
	using HikThread::Thread;
	using HikIts::Its;
	using HikIpc::Ipc;
	
	class Strategycontrol: public Thread
	{
	private:
		Ipc & ipc;
		Its & its;
		ControlType	curControlType;
		ControlMode curMode;
        UInt8 curSchemeID;
		UInt8 gCurControlStatus;
		
		bool IsChange(ControlType newControlType, ControlType nowControlType, ControlMode newMode, ControlMode nowMode);
		void WriteControlModeChangeLog(ControlType controlType, ControlMode mode);
		
	public:
		Strategycontrol(Ipc & c, Its & t);
        ~Strategycontrol() {}
		void run(void *arg);
        UInt8 GetControlStatus() const { return gCurControlStatus; }
	protected:
	};
}

#endif
