#include "faultcheck.h"

#define MAX_CHECK_COUNT		12		//最大检测次数

enum CheckResult
{
	CHECK_RET0 = 0,	//正常
	CHECK_RET1 = 1,	//有电压无电流，异常
	CHECK_RET2 = 2,	//无电压无电流，异常
	CHECK_RET3 = 3,	//无电压有电流，异常且几乎不存在此故障
	CHECK_RET4 = 4,	//有电压有电流，异常
	CHECK_RET5 = 5,	//有电压无电流，异常
	CHECK_RET6 = 6,	//无电压有电流，异常且几乎不存在此故障
};

struct Led
{
	bool on;
	bool volt;
	bool cur;

	CheckResult check() const
	{
		if (on)	//亮灯时
		{
			if (volt && cur)		//有电压有电流，正常
				return CHECK_RET0;
			else if (volt && !cur)	//有电压无电流，异常
				return CHECK_RET1;
			else if (!volt && !cur)	//无电压无电流，异常
				return CHECK_RET2;
			else					//无电压有电流，异常且几乎不存在此故障
				return CHECK_RET3;
		}
		else	//灭灯时
		{
			if (!volt && !cur)		//无电压无电流，正常
				return CHECK_RET0;
			else if (volt && cur)	//有电压有电流，异常
				return CHECK_RET4;
			else if (volt && !cur)	//有电压无电流，异常
				return CHECK_RET5;
			else					//无电压有电流，异常且几乎不存在此故障
				return CHECK_RET6;
		}
	}
};

bool FaultCheck::greenconflict(const Led &green, const Channel &channel)
{
	CheckResult ret = green.check();
	if (!green.on && ret != CHECK_RET0)
	{	//绿灯不该亮而故障则为绿冲突
		normal[0] = 0;
		if (++error[0] == MAX_CHECK_COUNT)
		{
			error[0] = MAX_CHECK_COUNT;
			faultStatus = GREEN_CONFLICT_STATUS;
			//log.Write("channel %d green conflict, checkresult: %d", channel.id, ret);
			return true;
		}
	}
	else
	{
		error[0] = 0;
		if (++normal[0] == MAX_CHECK_COUNT)
		{
			normal[0] = MAX_CHECK_COUNT;
			if (faultStatus == GREEN_CONFLICT_STATUS)
			{
				faultStatus = NORMAL_STATUS;
				//log.Write("channel %d green conflict clear!", channel.id);
			}
		}
	}
	return false;
}

bool FaultCheck::redgreenconflict(const Led &green, const Led &red, const Channel &channel)
{
	CheckResult retg = green.check();
	CheckResult retr = red.check();

	if ((!red.on && retr != CHECK_RET0) || (!green.on && retg != CHECK_RET0))
	{	//红灯不该亮而故障或者红灯正常亮而绿灯不该亮而故障则为红绿冲突
		normal[1] = 0;
		if (++error[1] == MAX_CHECK_COUNT)
		{
			error[1] = MAX_CHECK_COUNT;
			faultStatus = RED_GREEN_CONFLICT_STATUS;
			//log.Write("channel %d red green conflict, checkresultr: %d, checkresultg: %d", channel.id, retr, retg);
			return true;
		}
	}
	else
	{
		error[1] = 0;
		if (++normal[1] == MAX_CHECK_COUNT)
		{
			normal[1] = MAX_CHECK_COUNT;
			if (faultStatus == RED_GREEN_CONFLICT_STATUS)
			{
				faultStatus = NORMAL_STATUS;
				//log.Write("channel %d red green conflict clear!", channel.id);
			}
		}
	}
	return false;
}

bool FaultCheck::redoff(const Led &red, const Channel &channel)
{
	CheckResult ret = red.check();
	if (red.on && ret != CHECK_RET0)
	{	//红灯该亮而故障则为红灯熄灭
		normal[2] = 0;
		if (++error[2] == MAX_CHECK_COUNT)
		{
			error[2] = MAX_CHECK_COUNT;
			faultStatus = RED_OFF_STATUS;
			//log.Write("channel %d red off, checkresult: %d", channel.id, ret);
			return true;
		}
	}
	else
	{
		error[2] = 0;
		if (++normal[2] == MAX_CHECK_COUNT)
		{
			normal[2] = MAX_CHECK_COUNT;
			if (faultStatus == RED_OFF_STATUS)
			{
				faultStatus = NORMAL_STATUS;
				//log.Write("channel %d red off clear!", channel.id);
			}
		}
	}
	return false;
}

bool FaultCheck::yellowfault(const Led &yellow, const Channel &channel)
{
	CheckResult ret = yellow.check();
	if (ret != CHECK_RET0)
	{	//黄灯检测不正常则为黄灯故障
		normal[3] = 0;
		if (++error[3] == MAX_CHECK_COUNT)
		{
			error[3] = MAX_CHECK_COUNT;
			faultStatus = YELLOW_FAULT;
			//log.Write("channel %d yellow fault, checkresult: %d", channel.id, ret);
			return true;
		}
	}
	else
	{
		error[3] = 0;
		if (++normal[3] == MAX_CHECK_COUNT)
		{
			normal[3] = MAX_CHECK_COUNT;
			if (faultStatus == YELLOW_FAULT)
			{
				faultStatus = NORMAL_STATUS;
				//log.Write("channel %d yellow fault clear!", channel.id);
			}
		}
	}
	return false;
}

bool FaultCheck::hasfault(const std::bitset<9> &info, bool curCheck, const Channel &channel)
{	//如果不检测电流时默认电流状态跟灯的状态一样这样检测时就可以由于电流所带来的影响
	Led green = {info[0], info[1], curCheck ? info[2] : info[0]};
	Led red = {info[3], info[4], curCheck ? info[5] : info[3]};
	Led yellow = {info[6], info[7], curCheck ? info[8] : info[6]};

	//if (!channel.inuse())
	//	return true;
	if (greenconflict(green, channel))
		return true;

	if (redgreenconflict(green, red, channel))
		return true;

	if (redoff(red, channel))
		return true;

	if (!channel.isped())	//非行人灯进行黄灯故障检测
		yellowfault(yellow, channel);	//黄灯故障不属于严重故障，所以此处只做检测，却不反馈检测结果

	return false;
}