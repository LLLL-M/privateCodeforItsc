#include "protocol.h"
#include "ipc.h"
#include "calculate.h"
#include "phasecontrol.h"
#include "clock.h"
#include "phasedriver.h"
#include "faultlog.h"
#include "strategycontrol.h"
#include "redsigcheck.h"
#include "its.h"
#include "channelhandle.h"

using namespace HikIts;

using HikManage::Manage;
//静态成员变量初始化
Manage<SemHandle *> HikSem::Sem::manage;
template<class T>
Manage<std::queue<T> *> HikMsg::Msg<T>::manage;
Manage<HikLfq::lfq_t *> HikLfq::Lfq::manage;

Protocol *Its::ptl = NULL;
Ipc *Its::ipc = NULL;
Calculate *Its::calculate = NULL;
Phasecontrol *Its::phasecontrol = NULL;
Phasedriver *Its::phasedriver = NULL;
Clock *Its::clock = NULL;
Faultlog *Its::faultlog = NULL;
Strategycontrol *Its::strategycontrol = NULL;
Redsigcheck *Its::redsigcheck = NULL;
ChannelHandle *Its::chhandle = NULL;

Its::Its(Protocol *p /* = NULL*/)
{
	if (p != NULL && ptl == NULL)
		ptl = p;
}

bool Its::ItsInit(Protocol *p /*= NULL*/)
{	
	if (p != NULL && ptl == NULL)
		ptl = p;
	if (ptl == NULL)
		return false;
	ipc = new Ipc;
	if (ipc == NULL)
		return false;
    /*初始化各个线程模块*/
	faultlog = new Faultlog(*ptl);
	if (faultlog == NULL)
	{
		ItsExit();
		return false;
	}
	clock = new Clock(*ptl, *ipc);
	if (clock == NULL)
	{
		ItsExit();
		return false;
	}
#if defined(__linux__) && defined(__arm__)
    redsigcheck = new Redsigcheck(*ptl, *ipc);
    if (redsigcheck == NULL)
    {
        ItsExit();
        return false;
    }
#endif
	phasedriver = new Phasedriver(*ptl, *ipc);
	if (phasedriver == NULL)
	{
		ItsExit();
		return false;
	}
    chhandle = new ChannelHandle(*ptl, *ipc);
    if (chhandle == NULL)
    {
        ItsExit();
        return false;
    }
	phasecontrol = new Phasecontrol(*ptl, *ipc);
	if (phasecontrol == NULL)
	{
		ItsExit();
		return false;
	}
	strategycontrol = new Strategycontrol(*ipc, *this);
	if (strategycontrol == NULL)
	{
		ItsExit();
		return false;
	}
	calculate = new Calculate(*ptl, *ipc);
	if (calculate == NULL)
	{
		ItsExit();
		return false;
	}
	return true;
}

void Its::ItsExit()
{
	if (calculate != NULL)
	{
		delete calculate;
		calculate = NULL;
	}
	if (strategycontrol != NULL)
	{
		delete strategycontrol;
		strategycontrol = NULL;
	}
	if (phasecontrol != NULL)
	{
		delete phasecontrol;
		phasecontrol = NULL;
	}
    if (chhandle != NULL)
    {
        delete chhandle;
        chhandle = NULL;
    }
	if (phasedriver != NULL)
	{
		delete phasedriver;
		phasedriver = NULL;
	}
	if (redsigcheck != NULL)
	{
		delete redsigcheck;
		redsigcheck = NULL;
	}
	if (clock != NULL)
	{
		delete clock;
		clock = NULL;
	}
	if (faultlog != NULL)
	{
		delete faultlog;
		faultlog = NULL;
	}
	if (ipc != NULL)
	{
		delete ipc;
		ipc = NULL;
	}
}

void Its::ItsThreadCheck()
{
	HikThread::Thread *thread[] = {
		calculate, 
		strategycontrol, 
		phasecontrol, 
        chhandle,
		phasedriver, 
#if defined(__linux__) && defined(__arm__)
		redsigcheck, 
#endif
		clock,
	};
	for(int i = 0; i < sizeof(thread) / sizeof(thread[0]); i++)
	{
		if (thread[i] == NULL)
			continue;
		if (thread[i]->isdead())
			thread[i]->start();
	}
}

void Its::ItsCtl(ControlType type, UInt8 schemeId, Int32 val)
{
	ipc->MsgSend(type, schemeId, val);
}
#if 0
inline void Its::ItsChannelLock(ChannelLockParams *lockparams)
{
	if (phasecontrol != NULL)
		phasecontrol->Lock(lockparams);
}

inline void Its::ItsChannelUnlock()
{
	if (phasecontrol != NULL)
		phasecontrol->Unlock();
}
#endif
std::string & Its::ItsReadFaultLog(int startLine, UInt32 lineNum, std::string & log)
{
	if (faultlog != NULL)
        faultlog->Read(startLine, lineNum, log);
	return log;
}

bool Its::ItsWriteFaultLog(FaultLogType type, int value)
{
	if (faultlog != NULL)
        return faultlog->Write(type, value);
	return false;
}

void Its::ItsClearFaultLog()
{
	if (faultlog != NULL)
		return faultlog->Clear();
}
