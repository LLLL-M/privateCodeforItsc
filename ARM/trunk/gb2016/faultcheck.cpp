#include "faultcheck.h"

bool FaultCheck::GreenConflictCheck(bitset<3> &setval, bitset<3> &realval)
{	//检测绿冲突
	char buf[128] = {0};
	if (setval[0] == 0 && realval[0] == 1)
	{	//绿灯不该亮而亮则为绿冲突
		err[0]++;
		normal[0] = 0;
		if (faultStatus == NORMAL_STATUS && err[0] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = GREEN_CONFLICT_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d green conflict!!!", channelId);
			log.Write(GREEN_CONFLICT, buf, channelId);
			return false;
		}
	}
	else
	{
		normal[0]++;
		err[0] = 0;
		if (faultStatus == GREEN_CONFLICT_STATUS && normal[0] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = NORMAL_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d green conflict clear!!!", channelId);
			log.Write(GREEN_CONFLICT_CLEAR, buf, channelId);
		}
	}
	return true;
}

bool FaultCheck::RedGreenConflictCheck(bitset<3> &setval, bitset<3> &realval)
{	//检测红绿冲突
	char buf[128] = {0};
	if ((setval[1] == 0 && realval[1] == 1) || (realval[0] == 1 && realval[1] == 1))
	{	//红灯不该亮而亮或者红绿同时点亮则为红绿冲突
		err[1]++;
		normal[1] = 0;
		if (faultStatus == NORMAL_STATUS && err[1] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = RED_GREEN_CONFLICT_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d red green conflict!!!", channelId);
			log.Write(RED_GREEN_CONFLICT, buf, channelId);
			return false;
		}
	}
	else
	{
		normal[1]++;
		err[1] = 0;
		if (faultStatus == RED_GREEN_CONFLICT_STATUS && normal[1] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = NORMAL_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d red green conflict clear!!!", channelId);
			log.Write(RED_GREEN_CONFLICT_CLEAR, buf, channelId);
		}
	}
	return true;
}

bool FaultCheck::RedLightOffCheck(bitset<3> &setval, UInt8 redCur)
{	//检测红灯熄灭
	char buf[128] = {0};
#if 0
	if (setval[1] == 1 && channelId == 4)
		INFO("!!!!!!!!!! channel %d, redCur = %d ", channelId, redCur);
#endif
	if (setval[1] == 1 && redCur < 20)
	{	//红灯该亮但是电流值低于20则为红灯熄灭
		err[2]++;
		normal[2] = 0;
		if (faultStatus == NORMAL_STATUS && err[2] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = RED_OFF_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d red light off!!!", channelId);
			log.Write(RED_LIGHT_OFF, buf, channelId);
			return false;
		}
	}
	else
	{
		normal[2]++;
		err[2] = 0;
		if (faultStatus == RED_OFF_STATUS && normal[2] == MAX_FAULT_OCCUR_COUNT)
		{
			faultStatus = NORMAL_STATUS;
			log.UpdateFault(channelId, faultStatus);
			sprintf(buf, "channel %d red light off clear!!!", channelId);
			log.Write(RED_LIGHT_OFF_CLEAR, buf, channelId);
		}
	}
	return true;
}

bool FaultCheck::VoltAndCurCheck(int voltBits, UInt8 redCur)
{
	if (tsc == nullptr)
		return true;
	bitset<3> setval = val.load();
	bitset<3> realval = voltBits;		//bit0:绿灯电压,bit1:红灯电压,bit2:黄灯电压
	
	if (tsc->switchparam.voltCheck)
	{	//电压相关检测
		if (faultStatus == GREEN_CONFLICT_STATUS)	//当有绿冲突时只进行绿冲突检测
			return GreenConflictCheck(setval, realval);	
		else if (faultStatus == RED_GREEN_CONFLICT_STATUS)	//当有红绿冲突时只进行红绿冲突检测
			return RedGreenConflictCheck(setval, realval);
		else if (faultStatus == NORMAL_STATUS)		//状态正常时先进行绿冲突检测再进行红绿冲突检测
		{
			if (!GreenConflictCheck(setval, realval))
				return false;
			if (!RedGreenConflictCheck(setval, realval))
				return false;
		}
	}
	if (tsc->switchparam.curCheck)	//电流相关检测
		return RedLightOffCheck(setval, redCur);
	return true;
}
