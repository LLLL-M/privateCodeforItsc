#include <ctime>
#include <array>
#include "tsc.h"
#include "cycle.h"
#include "special.h"
#include "initcycle.h"
#include "ctrl.h"

Cycle* Tsc::GetInitCycle(ControlRule &rule)
{
	hik::rlock_guard rlock(rwl);
	InitCycle *boot = new InitCycle(channel.use);
	boot->Add(unit.conf.bootFlash, YELLOW_BLINK);
	boot->Add(unit.conf.bootAllRed, ALLRED);
	rule.duration = unit.conf.bootFlash + unit.conf.bootAllRed;
	return dynamic_cast<InitCycle *>(boot);
}

Cycle* Tsc::GetCycle(ControlRule &rule)
{	
	Cycle *cycle = nullptr;

	hik::rlock_guard rlock(rwl);
	if (rule.ctrlType == LOCAL_CONTROL)
	{
		time_t sec = time(nullptr);
		struct tm ptm = {0};

		#if(defined(__WINDOWS__))
		localtime_s(&ptm, &sec);
		#else
		localtime_r(&sec, &ptm);
		#endif
		
		UInt8 timeintervalId = FindTimeinterval(&ptm);

		if (timeintervalId == 0)
			return nullptr;
		FindRule(rule, &ptm, timeintervalId);
	}
	
	if (rule.ctrlMode == FIXEDCYCLE_MODE)
	{	
		cycle = FixedCycleInit(rule);
	}
	else if (rule.ctrlMode == TURNOFF_MODE
		|| rule.ctrlMode == YELLOWBLINK_MODE
		|| rule.ctrlMode == ALLRED_MODE)
	{
		return dynamic_cast<Cycle *>(new Special(rule.ctrlMode, channel.use));
	}

	return cycle;
}

Cycle* Tsc::FixedCycleInit(const ControlRule &rule)
{
	if (scheme.use.find(rule.ctrlId) == scheme.use.end())
		return nullptr;
	Cycle *cyc = new Cycle(rule, channel.use, scheme.use[rule.ctrlId].turn, scheme.use[rule.ctrlId].stage);
	cyc->desc = scheme.use[rule.ctrlId].desc;
	for(auto &i : scheme.use[rule.ctrlId].timing)
	{
		Phase & cyclePhase = cyc->phaseTable[i.second.phase];
		TscPhase & phaseConf = phase.use[i.second.phase];
		cyclePhase.ring = FindRingId(scheme.use[rule.ctrlId], i.second.phase);
		cyclePhase.phaseId = i.second.phase;
		cyclePhase.barrier = i.second.barrier;
		cyclePhase.channelBits = phaseConf.channelBits;
		cyclePhase.pedPhase = cyclePhase.channelBits.any();//空相位默认为机动车相位
		for (int ch = 0; ch < MAX_CHANNEL_NUM; ch++)
		{
			if (cyclePhase.channelBits[ch])
			{
				cyclePhase.vehDetectorBits |= channel.use[ch + 1].vehBits;
				cyclePhase.pedDetectorBits |= channel.use[ch + 1].pedBits;
				if (channel.use[ch + 1].type == CHANNEL_TYPE_CAR)
					cyclePhase.pedPhase = false;
			}
		}

		cyclePhase.unitExtendTime = phaseConf.unitExtend;
		cyclePhase.maxExtendTime = phaseConf.maxGreen - phaseConf.minGreen;
		cyclePhase.specifyStatus = (TscStatus)i.second.status;
		cyclePhase.autoReq = phaseConf.autoReq;
		cyclePhase.checkTime = phaseConf.checkTime;
		cyclePhase.desc = phaseConf.desc;

		cyclePhase.motor.Add(i.second.time - phaseConf.yellow - phaseConf.allred - phaseConf.greenFlash, GREEN, i.second.time - phaseConf.yellow - phaseConf.allred);
		cyclePhase.motor.Add(phaseConf.greenFlash, GREEN_BLINK, phaseConf.greenFlash);
		cyclePhase.motor.Add(phaseConf.yellow, YELLOW, phaseConf.yellow);
		cyclePhase.motor.Add(phaseConf.allred, ALLRED, phaseConf.allred);

		cyclePhase.pedestrian.Add(i.second.time - phaseConf.yellow - phaseConf.allred - phaseConf.pedClear, GREEN, i.second.time - phaseConf.yellow - phaseConf.allred);
		cyclePhase.pedestrian.Add(phaseConf.pedClear, GREEN_BLINK, phaseConf.pedClear);
		cyclePhase.pedestrian.Add(phaseConf.yellow + phaseConf.allred, ALLRED, phaseConf.yellow + phaseConf.allred);				
	}
	return cyc;
}

inline UInt8 Tsc::FindRingId(const TscScheme &item, int phaseid)
{
	for (UInt8 i = 0; i < item.turn.size(); i++)
	{
		for (auto &j : item.turn[i])
		{
			if (j == phaseid) 
			{
				return i;
			}
		}
	}
	return 0;
}

void Tsc::FindRule(ControlRule &rule, tm *calTime, int id)
{
	int nowmin = calTime->tm_hour * 60 + calTime->tm_min;
	int startgap = 0;
	int startgapmin = 1440;  /*min number in one day*/
	int endgap = 0;
	int endgapmin = 1440;  /*min number in one day*/
	int startselect = -1;
	int endselect = -1;
	int maxstartmin = 0;
	int maxstartid = -1;
	int endmin = 0;

	if (timeinterval.use[id].section.size() == 1)
	{
		rule.ctrlId = timeinterval.use[id].section[0].scheme;
		rule.ctrlMode = timeinterval.use[id].section[0].mode;
		rule.duration = (24 * 60 - nowmin) * 60 - calTime->tm_sec;
	}

	for (unsigned int i = 0; i < timeinterval.use[id].section.size(); i++)
	{
		int startmin = timeinterval.use[id].section[i].hour * 60 + timeinterval.use[id].section[i].minute;
		
		if (nowmin >= startmin)
		{
			startgap = nowmin -startmin;
			if (startgap <= startgapmin)
			{
				startgapmin = startgap;
				startselect = i;
			}
		}

		if ( nowmin < startmin)
		{
			endgap = startmin - nowmin;
			if (endgap <= endgapmin )
			{
				endgapmin = endgap;
				endselect = i;
			}
		}

		if (startmin >= maxstartmin)
		{
			maxstartmin = startmin;
			maxstartid = i;
		}
	}

	if (startselect == -1)
	{
		rule.ctrlId = timeinterval.use[id].section[maxstartid].scheme;
		rule.ctrlMode = timeinterval.use[id].section[maxstartid].mode;
		endmin = timeinterval.use[id].section[endselect].hour * 60	+ timeinterval.use[id].section[endselect].minute;
		rule.duration = (endmin - nowmin) * 60 - calTime->tm_sec;
	}
	else if (endselect == -1)
	{
		rule.ctrlId = timeinterval.use[id].section[maxstartid].scheme;
		rule.ctrlMode = timeinterval.use[id].section[maxstartid].mode;
		rule.duration = (24 * 60 - nowmin) * 60 - calTime->tm_sec;
	}
	else
	{
		rule.ctrlId = timeinterval.use[id].section[startselect].scheme;
		rule.ctrlMode = timeinterval.use[id].section[startselect].mode;
		endmin = timeinterval.use[id].section[endselect].hour * 60	+ timeinterval.use[id].section[endselect].minute;
		rule.duration = (endmin - nowmin) * 60 - calTime->tm_sec;
	}
}

UInt8 Tsc::FindTimeinterval(tm *calTime)
{
	/*先查找日期*/
	for (auto &i : schedule.use)
	{
		for (auto &j : i.second.date)
		{
			if ((j.year == calTime->tm_year + 1900 || j.year == 0)
				&& j.month == calTime->tm_mon + 1 && j.days[calTime->tm_mday])
				return i.second.timeinterval;
		}
	}
	/*日期查不到再查找星期*/
	for (auto &i : schedule.use)
	{
		if (i.second.week[calTime->tm_wday + 1])
			return i.second.timeinterval;
	}
	return 0;
}
