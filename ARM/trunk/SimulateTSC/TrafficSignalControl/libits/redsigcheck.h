#ifndef __REDSIGCHECK_H_
#define __REDSIGCHECK_H_

#if defined(__linux__) && defined(__arm__)
#include <sys/types.h>
#include <sys/socket.h>
#include "tsc.h"
#include "thread.h"

namespace HikIpc
{
	class Ipc;
}
namespace HikIts
{
	class Protocol;
}
namespace HikRedsigcheck
{
	using HikThread::Thread;
	using HikIts::Protocol;
	using HikIpc::Ipc;
	
	class Redsigcheck: public Thread
	{
	private:
		Protocol & ptl;
		Ipc & ipc;
		
		enum
		{
			RED_SIGNAL_HEARTBEAT_CMD = 0x03,
			RED_SIGNAL_POWERON_CMD = 0x18,
			RED_SIGNAL_JUMP_CMD	= 0x22,	
		};
		typedef struct redSignalData
		{
			UInt8 start;
			UInt8 address1;
			UInt8 address2;
			UInt8 length;
			UInt8 command;
			UInt8 data[4];
			UInt8 checksum;
		} __attribute__((aligned (1))) RedSignalTranData;
		
		void GetRedSignalData(UInt8 *allChannels, UInt8 *data);
		void CalCheckSumAndSend(int fd, struct sockaddr *addr, RedSignalTranData *tranData);
	public:
		Redsigcheck(Protocol & p, Ipc & i) : ptl(p), ipc(i) { start(); }
        ~Redsigcheck() {}
		void run(void *arg);
		
	protected:
	};
}
#else
namespace HikRedsigcheck
{
	class Redsigcheck
	{
	//windows和linux x86平台都不需要红灯信号检测模块，因此定义一个空的类
	};
}
#endif

#endif
