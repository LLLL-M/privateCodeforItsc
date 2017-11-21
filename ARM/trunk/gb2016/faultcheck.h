#ifndef __FAULTCHECK_H__
#define __FAULTCHECK_H__

#include <bitset>
#include <atomic>
#include "hik.h"
#include "log.h"
#include "singleton.h"
#include "tsc.h"

class FaultCheck
{
private:
	enum 
	{
		NORMAL_STATUS = 0,				//正常
		GREEN_CONFLICT_STATUS = 1,		//绿冲突
		RED_GREEN_CONFLICT_STATUS = 2,	//红绿冲突
		RED_OFF_STATUS = 3,				//红灯熄灭
		MAX_FAULT_OCCUR_COUNT = 12,
	};
	int			channelId;
	Tsc			*tsc;
	Log			&log;
	UInt8		faultStatus;	//灯故障状态
	int 		err[3];			//错误计数,[0]:绿冲突错误计数,[1]:红绿冲突错误计数,[2]:红灯熄灭错误计数
	int 		normal[3];		//正常计数,[0]:绿冲突正常计数,[1]:红绿冲突正常计数,[2]:红灯熄灭正常计数
	atomic_int	val;			//bit0:绿灯点灯值,bit1:红灯点灯值,bit2:黄灯点灯值
	
	//检测绿冲突,正常返回true,绿冲突返回false
	bool GreenConflictCheck(bitset<3> &setval, bitset<3> &realval);
	//检测红绿冲突,正常返回true,红绿冲突返回false
	bool RedGreenConflictCheck(bitset<3> &setval, bitset<3> &realval);
	//检测红灯熄灭,正常返回true,红灯熄灭返回false
	bool RedLightOffCheck(bitset<3> &setval, UInt8 redCur);
	//电压和电流检测,正常返回true,不正常返回false
	bool VoltAndCurCheck(int voltBits, UInt8 redCur);
	
public:
	FaultCheck() : channelId(0), log(Singleton<Log>::GetInstance()), faultStatus(NORMAL_STATUS), err{0,0,0}, normal{0,0,0}
	{
		val = 0;
		tsc = nullptr;
	}
	//设置通道id
	void SetId(int v)
	{
		if (channelId == 0)
		{
			channelId = v;
			faultStatus = log.LoadFault(channelId);
		}
	}
	//设置点灯值
	void SetVal(int v)
	{
		val = v;
	}
	//检测故障
	void Check(int voltBits, UInt8 redCur)
	{
#if 0
		if (channelId != 1 && channelId != 2 && channelId != 6)
			return;
			//INFO("############## redcur = %d, channelId: %d", redCur, channelId);
#endif
		if (tsc == nullptr)
			tsc = &Singleton<Tsc>::GetInstance();
		else
		{
			if (!VoltAndCurCheck(voltBits, redCur) && tsc->switchparam.faultYellowFlash)
				tsc->SetRule({FAULT_CONTROL, YELLOWBLINK_MODE, 0});
		}
	}

protected:
};

#endif
