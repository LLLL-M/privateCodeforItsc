#ifndef __FAULTLOG_H_
#define __FAULTLOG_H_

#include "tsc.h"
#include "file.h"
#include "lock.h"

namespace HikIts
{
	class Protocol;
}
namespace HikFaultlog
{
	using HikIts::Protocol;
	using HikFile::File;
	using HikLock::RWlock;
	
	class Faultlog
	{
	private:
		Protocol & ptl;
		File file;
		RWlock rwlock;
		struct FaultLogHead
		{
            UInt32 num;
			FaultLogInfo data[0];
		} *head;
	public:
		Faultlog(Protocol & p);
		~Faultlog();
        std::string & Read(int startLine, UInt32 lineNum, std::string & log);
        bool Write(FaultLogType type, int value);
		void Clear();
		
	protected:
	};
}

#endif
