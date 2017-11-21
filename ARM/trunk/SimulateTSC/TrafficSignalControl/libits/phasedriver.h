#ifndef __PHASEDRIVER_H_
#define __PHASEDRIVER_H_

#include "thread.h"
#include "hik.h"

namespace HikIpc
{
	class Ipc;
}
namespace HikIts
{
	class Protocol;
}
namespace HikPhasedriver
{
	using HikThread::Thread;
	using HikIts::Protocol;
	using HikIpc::Ipc;
	
	class Phasedriver: public Thread
	{
	private:
		enum
		{
			LOFF = 0,
			LGREEN = 1,
			LRED = 2,
			LYELLOW = 4,
			LREDYELLOW = 6,
			MAX_BOARD_NUM  = ((MAX_CHANNEL_NUM + 3) / 4),	//点灯时总共的点灯数组个数
		};
		Protocol & ptl;
		Ipc & ipc;
		UInt16 lightValues[MAX_BOARD_NUM];	//存放点灯数据的数组
		
		void Light(UInt8 *allChannel, UInt8 times);
	public:
		Phasedriver(Protocol & p, Ipc & i);
        ~Phasedriver() {}
		void run(void *arg);
	protected:
	};
}

#endif
