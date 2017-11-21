#include <cstring>
#if defined(__linux__) && !defined(_SVID_SOURCE)
#define _SVID_SOURCE	//stime需要此宏定义，因为glibc版本问题
#endif
#include <ctime>
#include <cstdlib>
#include <functional>
#include "tsc.h"
#include "tscdb.h"
#include "singleton.h"
#include "log.h"
#include "detector.h"
#include "cycle.h"
#include "frame.h"
#include "gpio.h"
#include "can.h"

enum
{
	DOWNLOAD_FINISHED = 1,
	CHANNEL_BIT = 0,
	PHASE_BIT = 1,
	SCHEME_BIT = 2,
	SCHEDULE_BIT = 3,
	DETECTOR_BIT = 4,
	FEATURES_BIT = 5,
};

inline void Tsc::SetGpsAndWatchdog()
{
	Gpio &gpio = Singleton<Gpio>::GetInstance();
	gpio.SetGps(switchparam.gps);
	gpio.SetWatchdog(switchparam.watchdog);
	//INFO("gps: %d, watchdog: %d", switchparam.gps, switchparam.watchdog);
}

Tsc::Tsc() : log(Singleton<Log>::GetInstance()), detectorArray(Singleton<DetectorArray>::GetInstance()), can(Singleton<Can>::GetInstance())
{
	updateFlag = 0;
	Tscdb db;
	if (!db.Open())
		return;
	db.Load(channelTable);
	db.Load(phaseTable);
	db.Load(schemeTable);
	db.Load(scheduleTable);
	db.Load(detectorTable);
	db.Load(switchparam);
	db.Load(basic);
	db.Close();
	SetGpsAndWatchdog();
	memcpy(bakChannelTable, channelTable, sizeof(bakChannelTable));
	memcpy(bakPhaseTable, phaseTable, sizeof(bakPhaseTable));
	memcpy(bakSchemeTable, schemeTable, sizeof(bakSchemeTable));
	memcpy(bakScheduleTable, scheduleTable, sizeof(bakScheduleTable));
	memcpy(bakDetectorTable, detectorTable, sizeof(bakDetectorTable));
	SendLCBconfigToBoard();
	timeGapSec = 0;
	leftTransCycle = 0;
}

bool Tsc::CheckChannel(const UInt32 channelBits)
{
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (GET_BIT(channelBits, i) == 1 && (bakChannelTable[i].conflictChannel & channelBits))
			return false;
	}
	return true;
}

bool Tsc::CheckPhase(const PhaseItem &p, UInt16 &splitTime)
{
	splitTime = p.greenTime + p.greenBlinkTime + p.yellowTime + p.allRedTime;
	return (CheckChannel(p.channelBits) && splitTime > 0 && p.minGreenTime <= p.maxGreenTime);
}

bool Tsc::CheckScheme(const SchemeItem &s)
{
	UInt16 num = 0, sum = 0;
	bool include = s.coordinatePhase > 0 ? false : true;	//相序是否包含协调相位
	for (auto id : s.phaseturn)
	{
		if (id == 0 || id > MAX_PHASE_NUM)
			break;
		num++;
		UInt16 splitTime = 0;
		if (CheckPhase(bakPhaseTable[id - 1], splitTime))
			sum += splitTime;
		else
			return false;
		if (s.coordinatePhase == id)
			include = true;
	}
	return (include && s.cycleTime > 0 && num > 1 && s.cycleTime == sum);
}

bool Tsc::CheckConfig()
{
	int num = 0;
	for (auto &schedule : bakScheduleTable)
	{
		if (schedule.scheduleId == 0 || schedule.scheduleId > MAX_SCHEDULE_NUM)
			continue;
		if ((schedule.week == 0 && schedule.month == 0)
			|| (schedule.week > 7) || (schedule.month > 12) 
			|| (schedule.day == 0 || schedule.day > 31)
			|| (schedule.startHour > 23 || schedule.startMin > 59)
			|| (schedule.endHour > 24 || schedule.endMin > 59)
			|| (schedule.endHour * 60 + schedule.endMin <= schedule.startHour * 60 + schedule.startMin)
			|| (schedule.ctrlType != LOCAL_CONTROL)
			|| (schedule.ctrlMode == STEP_MODE)
			|| (schedule.ctrlMode == CANCEL_STEP)
			|| (schedule.ctrlMode != YELLOWBLINK_MODE && schedule.ctrlMode != TURNOFF_MODE && schedule.ctrlMode != ALLRED_MODE 
				&& (schedule.ctrlId == 0 || schedule.ctrlId > MAX_SCHEME_NUM || !CheckScheme(bakSchemeTable[schedule.ctrlId - 1]))))
			return false;
		num++;
	}
	return (num >= 7);	//调度表记录最少7个，因为最少一天一个调度
}

bool Tsc::UpdateConfig()
{
	if (!CheckConfig())
		return false;
	Tscdb db;
	db.Open();
	configlock.w_lock();
	if (GET_BIT(updateFlag, CHANNEL_BIT))
	{
		memcpy(channelTable, bakChannelTable, sizeof(channelTable));
		db.Store(channelTable);
	}
	if (GET_BIT(updateFlag, PHASE_BIT))
	{
		memcpy(phaseTable, bakPhaseTable, sizeof(phaseTable));
		db.Store(phaseTable);
	}
	if (GET_BIT(updateFlag, SCHEME_BIT))
	{
		memcpy(schemeTable, bakSchemeTable, sizeof(schemeTable));
		db.Store(schemeTable);
	}
	if (GET_BIT(updateFlag, SCHEDULE_BIT))
	{
		memcpy(scheduleTable, bakScheduleTable, sizeof(scheduleTable));
		db.Store(scheduleTable);
	}
	if (GET_BIT(updateFlag, DETECTOR_BIT))
	{
		memcpy(detectorTable, bakDetectorTable, sizeof(detectorTable));
		db.Store(detectorTable);
	}
#if 0
	if (GET_BIT(updateFlag, FEATURES_BIT))
	{
		db.Store(features);
	}
#endif
	configlock.w_unlock();
	db.Close();
	SendLCBconfigToBoard();
	updateFlag = 0;
	return true;
}

bool Tsc::SetChannelTable(const UInt8 *data, int size)
{
	UInt8 num = data[0];
	int i;
	bool downloadComplete = false;

	if (num > MAX_CHANNEL_NUM || num == 0 || num * sizeof(ChannelItem) > size - 1)
	{
		ERR("The number within channel setting is invalid!");
		return false;
	}
	const ChannelItem *item = (ChannelItem *)(data + 1);
	for (i = 0; i < num; i++)
	{
		if (item[i].channelId == 0 || item[i].channelId > MAX_CHANNEL_NUM)
		{
			ERR("The channelId %d is invalid!", item[i].channelId);
			return false;
		}
		bakChannelTable[item[i].channelId - 1] = item[i];
		if (item[i].flag == DOWNLOAD_FINISHED)
		{
			bakChannelTable[item[i].channelId - 1].flag = 0;
			downloadComplete = true;
		}
	}
	if (i == num)
	{
		if (downloadComplete)
		{
			SET_BIT(updateFlag, PHASE_BIT);
			return UpdateConfig();
		}
		else
			return true;
	}
	return false;
}

bool Tsc::SetPhaseTable(const UInt8 *data, int size)
{
	UInt8 num = data[0];
	int i;
	bool downloadComplete = false;

	if (num > MAX_PHASE_NUM || num == 0 || num * sizeof(PhaseItem) > size - 1)
	{
		ERR("The number within phase setting is invalid!");
		return false;
	}
	const PhaseItem *item = (PhaseItem *)(data + 1);
	for (i = 0; i < num; i++)
	{
		if (item[i].phaseId == 0 || item[i].phaseId > MAX_PHASE_NUM)
		{
			ERR("The phaseId %d is invalid!", item[i].phaseId);
			return false;
		}
		bakPhaseTable[item[i].phaseId - 1] = item[i];
		if (item[i].flag == DOWNLOAD_FINISHED)
		{
			bakPhaseTable[item[i].phaseId - 1].flag = 0;
			downloadComplete = true;
		}
	}
	if (i == num)
	{
		if (downloadComplete)
		{
			SET_BIT(updateFlag, PHASE_BIT);
			return UpdateConfig();
		}
		else
			return true;
	}
	return false;
}

bool Tsc::SetSchemeTable(const UInt8 *data, int size)
{
	UInt8 num = data[0];
	int i;
	bool downloadComplete = false;

	if (num > MAX_SCHEME_NUM || num == 0 || num * sizeof(SchemeItem) > size - 1)
	{
		ERR("The number within scheme setting is invalid!");
		return false;
	}
	const SchemeItem *item = (SchemeItem *)(data + 1);
	for (i = 0; i < num; i++)
	{
		if (item[i].schemeId == 0 || item[i].schemeId > MAX_SCHEME_NUM)
		{
			ERR("The schemeId %d is invalid!", item[i].schemeId);
			return false;
		}
		bakSchemeTable[item[i].schemeId - 1] = item[i];
		if (item[i].flag == DOWNLOAD_FINISHED)
		{
			bakSchemeTable[item[i].schemeId - 1].flag = 0;
			downloadComplete = true;
		}
	}
	if (i == num)
	{
		if (downloadComplete)
		{
			SET_BIT(updateFlag, SCHEME_BIT);
			return UpdateConfig();
		}
		else
			return true;
	}
	return false;
}

bool Tsc::SetScheduleTable(const UInt8 *data, int size)
{
	UInt8 num = data[0];
	int i;
	bool downloadComplete = false;

	if (num > MAX_SCHEDULE_NUM || num == 0 || num * sizeof(ScheduleItem) > size - 1)
	{
		ERR("The number within schedule setting is invalid!");
		return false;
	}
	const ScheduleItem *item = (ScheduleItem *)(data + 1);
	for (i = 0; i < num; i++)
	{
		if (item[i].scheduleId == 0 || item[i].scheduleId > MAX_SCHEDULE_NUM)
		{
			ERR("The scheduleId %d is invalid!", item[i].scheduleId);
			return false;
		}
		bakScheduleTable[item[i].scheduleId - 1] = item[i];
		if (item[i].flag == DOWNLOAD_FINISHED)
		{
			bakScheduleTable[item[i].scheduleId - 1].flag = 0;
			downloadComplete = true;
		}
	}
	if (i == num)
	{
		if (downloadComplete)
		{
			SET_BIT(updateFlag, SCHEDULE_BIT);
			return UpdateConfig();
		}
		else
			return true;
	}
	return false;
}

bool Tsc::SetDetectorTable(const UInt8 *data, int size)
{
	UInt8 num = data[0];
	int i;
	bool downloadComplete = false;

	if (num > MAX_DETECTOR_NUM || num == 0 || num * sizeof(DetectorItem) > size - 1)
	{
		ERR("The number within detector setting is invalid!");
		return false;
	}
	const DetectorItem *item = (DetectorItem *)(data + 1);
	for (i = 0; i < num; i++)
	{
		if (item[i].detectorId == 0 || item[i].detectorId > MAX_DETECTOR_NUM)
		{
			ERR("The detectorId %d is invalid!", item[i].detectorId);
			return false;
		}
		bakDetectorTable[item[i].detectorId - 1] = item[i];
		if (item[i].flag == DOWNLOAD_FINISHED)
		{
			bakDetectorTable[item[i].detectorId - 1].flag = 0;
			downloadComplete = true;
		}
	}
	if (i == num)
	{
		if (downloadComplete)
		{
			SET_BIT(updateFlag, DETECTOR_BIT);
			return UpdateConfig();
		}
		else
			return true;
	}
	return false;
}

bool Tsc::Write(UInt8 objectId, const string &data)
{	
	bool ret = true;
	
	switch (objectId)
	{
		case CURRENT_TINE:		//当前时间
			if (data.size() != sizeof(UInt32))
			{
				ERR("the size[%d] isn't right when set time!", data.size());
				return false;
			}
		#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
			stime((const time_t *)data.data());
		#endif
			break;
		case SIGNAL_LAMP_GROUP:	
			ret = SetChannelTable((const UInt8 *)data.data(), data.size()); 
			if (!ret)
				memcpy(bakChannelTable, channelTable, sizeof(channelTable));
			break;	//信号灯组
		case PHASE:	
			ret = SetPhaseTable((const UInt8 *)data.data(), data.size()); 
			if (!ret)
				memcpy(bakPhaseTable, phaseTable, sizeof(phaseTable));
			break;					//相位
		case SIGNAL_TIMING_SCHEME: 
			ret = SetSchemeTable((const UInt8 *)data.data(), data.size()); 
			if (!ret)
				memcpy(bakSchemeTable, schemeTable, sizeof(schemeTable));
			break;	//信号配时方案
		case SCHEME_SCHEDULE: 
			ret = SetScheduleTable((const UInt8 *)data.data(), data.size()); 
			if (!ret)
				memcpy(bakScheduleTable, scheduleTable, sizeof(scheduleTable));
			break;	//方案调度计划
		case DETECTOR: 
			ret = SetDetectorTable((const UInt8 *)data.data(), data.size()); 
			if (!ret)
				memcpy(bakDetectorTable, detectorTable, sizeof(detectorTable));
			break;			//检测器
		case FEATURES_PARAMETER: 	//特征参数
		{
			if (data.size() != 1)
			{
				ERR("the size[%d] isn't right when set features!", data.size());
				return false;
			}
			//SET_BIT(updateFlag, FEATURES_BIT);
			break;
		}
		case WORK_MODE:				//工作方式
			if (data.size() != sizeof(UInt8))
			{
				ERR("the size[%d] isn't right when set work mode!", data.size());
				return false;
			}
			else
			{
				UInt8 tmp = (UInt8)data[0];
				ControlRule rule;
				rule.ctrlType = (UInt8)CLIENT_CONTROL;
				rule.ctrlMode = (tmp & 0xf);
				rule.ctrlId = (tmp >> 4);
				ret = SetRule(rule);
			}
			break;
		case REMOTE_CONTROL:
			{
			UInt8 param = data[0] & 0x1f;
			memcpy(&switchparam, &param, 1);
			SetGpsAndWatchdog();
			Tscdb db;
			db.Open();
			db.Store(switchparam);
			db.Close();
			SendLCBconfigToBoard();
				UInt8 value = (data[0] >> 6);
				if (value == RESTART)
					system("reboot");
				else if (value == RECOVER_DEFAULT)
					system("rm /home/tsc.db");
				else if (value == FAULT_CLEAR)
					log.Clear();
			}
			break;
		default: ret = false;
	}
	return ret;
}

void Tsc::Read(UInt8 objectId, const string &arg, string &buf)
{
	int pos = 0, num = 0;
	
	if (buf.size() > 0)
		pos = buf.size() - 1;	//记录存放num个数的位置
	switch (objectId)
	{
		case WORK_STATUS:			//工作状态
		{	//c++对于case定义的变量是可以在其他case中使用的，因此如果case中有定义变量应使用{}避免编译器报错
			ControlRule rule(curRule);
			CurrentWorkStatus workStatus = {rule.ctrlType, rule.ctrlMode, rule.ctrlId, curPhase, curCycleTime};
			buf.append((char *)&workStatus, sizeof(CurrentWorkStatus));
			return;
		}
		case LAMP_STATUS:			//灯色状态
		{
			LampColorStatus lampStatus{curLampStatus, 0};
			buf.append((char *)&lampStatus, sizeof(LampColorStatus));
			return;
		}
		case CURRENT_TINE:			//当前时间
		{
			time_t curTime = time(nullptr);
			buf.append((char *)&curTime, sizeof(UInt32));
			return;
		}
		case WORK_MODE:				//工作方式
		{
			ControlRule rule(curRule);
			buf.append(1, (rule.ctrlMode & 0xf) | (rule.ctrlId << 4));
			return;
		}
		case SIGNAL_MACHINE_VER:	//信号机版本
			buf.append(basic.version, sizeof(basic.version));
			return;
		case FEATURES_PARAMETER: 	//特征参数
			buf.push_back(1);
			return;
		case SIGNAL_MACHINE_ID_CODE://信号机识别码
			buf.append(basic.identifyCode, 14); 
			return;
		case SIGNAL_LAMP_GROUP:		//信号灯组
		{
			configlock.r_lock();
			buf.push_back(0);		//先记录个数为0
			for (int i = 0; i < MAX_CHANNEL_NUM; i++)
			{
				if (channelTable[i].channelId != 0)
				{
					num++;
					buf.append((char *)&channelTable[i], sizeof(ChannelItem));
				}
			}
			buf[pos] = num;
			configlock.r_unlock();
			break;	
		}
		case PHASE:					//相位
		{
			configlock.r_lock();
			buf.push_back(0);		//先记录个数为0
			for (int i = 0; i < MAX_PHASE_NUM; i++)
			{
				if (phaseTable[i].phaseId != 0)
				{
					num++;
					buf.append((char *)&phaseTable[i], sizeof(PhaseItem));
				}
			}
			buf[pos] = num;
			configlock.r_unlock();
			break;	
		}
		case SIGNAL_TIMING_SCHEME: 	//信号配时方案
		{
			configlock.r_lock();
			buf.push_back(0);		//先记录个数为0
			for (int i = 0; i < MAX_SCHEME_NUM; i++)
			{
				if (schemeTable[i].schemeId != 0)
				{
					num++;
					buf.append((char *)&schemeTable[i], sizeof(SchemeItem));
				}
			}
			buf[pos] = num;
			configlock.r_unlock();
			break;	
		}
		case SCHEME_SCHEDULE: 		//方案调度计划
		{
			configlock.r_lock();
			buf.push_back(0);		//先记录个数为0
			for (int i = 0; i < MAX_SCHEDULE_NUM; i++)
			{
				if (scheduleTable[i].scheduleId != 0)
				{
					num++;
					buf.append((char *)&scheduleTable[i], sizeof(ScheduleItem));
				}
			}
			buf[pos] = num;
			configlock.r_unlock();
			break;	
		}
		case DETECTOR: 				//检测器
		{
			configlock.r_lock();
			buf.push_back(0);		//先记录个数为0
			for (int i = 0; i < MAX_DETECTOR_NUM; i++)
			{
				if (detectorTable[i].detectorId != 0)
				{
					num++;
					buf.append((char *)&detectorTable[i], sizeof(DetectorItem));
				}
			}
			buf[pos] = num;
			configlock.r_unlock();
			break;	
		}
		
		case TRAFFIC_FLOW_INFO:		//交通流信息
			/***************************************/
			break;
		case SIGNAL_MACHINE_FAULT:	//信号机故障
		{
			if (arg.size() != 5)
				return;
			UInt32 startTime = 0;
			UInt8 lineNum = 0;
			arg.copy((char *)&startTime, 4);
			arg.copy((char *)&lineNum, 1, 4);
			vector<FaultLog> &&vec = log.Read(startTime, lineNum);	//使用c++11的右值引用
			buf.push_back((char)vec.size());	//首字节填充faultlog个数
			buf.append((char *)vec.data(), vec.size() * sizeof(FaultLog));
			return;
		}
		default: ERR("The objectId[%d] is invalid when read tsc", objectId); return;
	}
}

void Tsc::StartInit(vector<ColorStep> &vec)
{
	vec.emplace_back(basic.bootYellowBlinkTime, YELLOW_BLINK);
	vec.emplace_back(basic.bootAllRedTime, ALLRED);
}

bool Tsc::GetLocalRule(const int aheadOfTime, ControlRule &rule)
{
	struct tm calTime;
	time_t te = time(nullptr) + aheadOfTime;
	bool ret = false;
	auto judgetime = [&calTime, &rule, this](ScheduleItem &s)->bool{ //c++11的lambda函数,用来判断计算时间是否在调度项内
		int start = s.startHour * 60 + s.startMin;
		int end = s.endHour * 60 + s.endMin;
		int now = calTime.tm_hour * 60 + calTime.tm_min;
		if (now >= start && now < end)
		{
			//timeGapSec = (now - start) * 60;
			rule.ctrlType = s.ctrlType;
			rule.ctrlMode = s.ctrlMode;
			rule.ctrlId = s.ctrlId;
			return true;
		}
		else
			return false;
	};
	function<void(ControlRule &)> FindScheme;
	FindScheme = [&FindScheme, &calTime, this](ControlRule &rule){	//在调度表中找寻所配置的对应模式的方案,如果未找到则再次降级直到找到降级的模式方案为止
		if (rule.ctrlMode == YELLOWBLINK_MODE)
			return;
		for (auto &schedule : scheduleTable)
		{
			if (rule.ctrlMode == schedule.ctrlMode)
			{
				rule.ctrlId = schedule.ctrlId;
#if 0
				if (rule.ctrlMode == COORDINATE_MODE)
				{
					int now = calTime.tm_hour * 60 + calTime.tm_min;
					int start = schedule.startHour * 60 + schedule.startMin;
					if (now >= start)
						timeGapSec = (now - start) * 60;
					else
						timeGapSec = (now + 24 * 60 - start) * 60;
				}
#endif
				return;
			}
		}
		rule = Downgrade();
		FindScheme(rule);
	};
	
	memcpy(&calTime, localtime(&te), sizeof(calTime));
	timeGapSec = calTime.tm_hour * 3600 + calTime.tm_min * 60 + calTime.tm_sec;
	//INFO("tm_hour: %d, tm_min: %d, tm_sec: %d, timeGapSec: %d", calTime.tm_hour, calTime.tm_min, calTime.tm_sec, timeGapSec);
	if (calTime.tm_wday == 0)	//tm_wday=0表示星期日 
		calTime.tm_wday = 7;
	calTime.tm_mon++;	//tm_mon范围[0,11]
	configlock.r_lock();
	if (!rule.SpecialMode() && rule.ctrlMode != SYSTEM_MODE)
	{	//依据指定的模式(主要在降级中设置的)来找寻方案
		FindScheme(rule);
		ret = true;
	}
	else
	{	//依据当前时间来找寻方案
		for (auto &schedule : scheduleTable)
		{
			if (calTime.tm_mon == schedule.month && calTime.tm_mday == schedule.day && judgetime(schedule))
			{	//日期调度优先
				ret = true;
				break;
			}
			if (calTime.tm_wday == schedule.week && ret == false && judgetime(schedule))
			{	//星期调度次后
				ret = true;
			}
		}
	}
	configlock.r_unlock();
	if (!ret)
	{
		ERR("search schedule fail, calTime:%d-%d %d:%d:%d", calTime.tm_mon, calTime.tm_wday, calTime.tm_hour, calTime.tm_min, calTime.tm_sec);
		rule.Print();
	}
	return ret;
}

//计算协调控制需要过度的时间
void Tsc::CoordinateDeal(SchemeItem &scheme, SchemeInfo &schemeInfo)
{ 
	int totalTransitionTime = 0;	//总共需要过渡的时间
	int transitionTime = 0;			//每个周期需要过渡时间
	int excessTime = 0;	//从当前时段起始时间运行到现在的多余时间
	int phaseOffset = scheme.phaseOffset;
	PhaseInfo *phaseInfo = &schemeInfo.phaseturn[0];

	if (basic.transitionCycle == 0 || scheme.cycleTime == 0)	//如果没有配置过渡周期则不进行过渡
		return;
#if 0
	for (auto &pi : schemeInfo.phaseturn)
	{	//用以处理协调相位不是首相位的情况，此时相位差需要重新计算
		if (pi.phaseId == 0 || pi.phaseId > MAX_PHASE_NUM)
		{	//如果在相序中找不到协调相位，则默认把相序中第一个相位作为协调相位
			scheme.coordinatePhase = scheme.phaseturn[0];
			phaseOffset = scheme.phaseOffset;
			break;
		}
		if (pi.phaseId == scheme.coordinatePhase)
		{
			phaseInfo = &pi;
			break;
		}
		phaseOffset -= pi.splitTime;
	}
#endif
	excessTime = (timeGapSec - phaseOffset) % scheme.cycleTime;
	if (excessTime == 0 || excessTime == 1)
		leftTransCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//如果多余时间在5s之内，可以通过递减协调相位时间来达到过渡
		leftTransCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		//周期时间减去多余时间，再加上相位差时间即是总共需要过渡时间
		totalTransitionTime = (excessTime > 0) ? (scheme.cycleTime - excessTime) : abs(excessTime);
		if (leftTransCycle == 0)
			leftTransCycle = basic.transitionCycle;
		//当总共的过渡时间小于等于过渡周期或是小于等于5s时，便在一个周期内完成
		transitionTime = (totalTransitionTime <= leftTransCycle || totalTransitionTime <= 5) 
						 ? totalTransitionTime 
						 : totalTransitionTime / leftTransCycle;
		leftTransCycle--;
	}
#if 0
#define MAX_CYCLE_TIME	0xff	//最大周期时间
	if (scheme.cycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//如果周期时间加上过渡时间大于最大周期时间0xff,则缩短过渡时间并增加过渡周期
		transitionTime = MAX_CYCLE_TIME - scheme.cycleTime;
		leftTransCycle++;
	}
#endif
	if (transitionTime > 0)
	{
		INFO("cycleTime=%d, timeGapSec=%d, phaseOffset=%d, totalTransitionTime=%d, transitionTime=%d", scheme.cycleTime, timeGapSec, phaseOffset, totalTransitionTime, transitionTime);
		phaseInfo->splitTime += transitionTime;
		phaseInfo->motor[0].stepTime += transitionTime;
		phaseInfo->pedestrian[0].stepTime += transitionTime;
		schemeInfo.cycleTime += transitionTime;
	}
}
//判断是否是行人相位
bool Tsc::PedestrianPhase(bitset<MAX_CHANNEL_NUM> &channelBits)
{
	if (channelBits.none())
		return false;
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (channelBits[i] == 1 && channelTable[i].channelType == MOTOR)
			return false;
	}
	return true;
}
//填充定周期方案信息
bool Tsc::FillFixCycle(const SchemeItem &scheme, SchemeInfo &schemeInfo)
{
	schemeInfo.cycleTime = 0;
	for (auto phaseId : scheme.phaseturn)
	{
		if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
			break;
		auto &phaseItem = phaseTable[phaseId - 1];
		PhaseInfo phaseInfo;
		phaseInfo.phaseId = phaseItem.phaseId;
		phaseInfo.checkTime = phaseItem.checkTime;
		phaseInfo.vehDetectorBits = phaseItem.vehDetectorBits;
		phaseInfo.unitExtendTime = phaseItem.unitExtendTime;
		phaseInfo.maxExtendTime = phaseItem.maxGreenTime - phaseItem.minGreenTime;
		phaseInfo.pedDetectorBits = phaseItem.pedDetectorBits;
		phaseInfo.advanceExtendTime = phaseItem.advanceExtendTime;
		phaseInfo.pedResponseTime = phaseItem.pedResponseTime;
		phaseInfo.pedRequest = true;
		phaseInfo.motorRequest = true;
		phaseInfo.channelBits = phaseItem.channelBits;
		phaseInfo.pedPhase = PedestrianPhase(phaseInfo.channelBits);
		short pedPassTime = phaseItem.greenTime + phaseItem.greenBlinkTime - phaseItem.pedClearTime;
		short pedClearTime = phaseItem.pedClearTime;
		if (pedPassTime < 0)
		{
			pedClearTime = phaseItem.greenTime + phaseItem.greenBlinkTime;
			pedPassTime = 0;
		}
		//INFO("phaseId: %d, pedPhase: %d, channelBits: %#x", phaseInfo.phaseId, phaseInfo.pedPhase, phaseItem.channelBits);
		if (phaseInfo.pedPhase)
		{	//行人相位时，把行人放行和行人清空时间作为绿信比时间
			phaseInfo.vehDetectorBits = 0;
			phaseInfo.splitTime = pedPassTime + pedClearTime;
			phaseInfo.motor.emplace_back(pedPassTime, GREEN);
			phaseInfo.motor.emplace_back(pedClearTime, GREEN_BLINK);
			phaseInfo.pedestrian.emplace_back(pedPassTime, GREEN);
			phaseInfo.pedestrian.emplace_back(pedClearTime, GREEN_BLINK);
		}
		else
		{
			phaseInfo.splitTime = phaseItem.greenTime + phaseItem.greenBlinkTime + phaseItem.yellowTime + phaseItem.allRedTime;
			phaseInfo.motor.emplace_back(phaseItem.greenTime, GREEN);
			phaseInfo.motor.emplace_back(phaseItem.greenBlinkTime, GREEN_BLINK);
			phaseInfo.motor.emplace_back(phaseItem.yellowTime, YELLOW);
			phaseInfo.motor.emplace_back(phaseItem.allRedTime, ALLRED);
			phaseInfo.pedestrian.emplace_back(pedPassTime, GREEN);
			phaseInfo.pedestrian.emplace_back(pedClearTime, GREEN_BLINK);
			phaseInfo.pedestrian.emplace_back(phaseInfo.splitTime - pedPassTime - pedClearTime, RED);
		}
		schemeInfo.cycleTime += phaseInfo.splitTime;
		schemeInfo.phaseturn.push_back(phaseInfo);
	}
	return (schemeInfo.cycleTime > 0 && schemeInfo.phaseturn.size() > 1);
}
//填充感应控制方案信息
bool Tsc::FillInductive(const SchemeItem &scheme, SchemeInfo &schemeInfo)
{
	schemeInfo.cycleTime = 0;
	for (auto phaseId : scheme.phaseturn)
	{
		if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
			break;
		auto &phaseItem = phaseTable[phaseId - 1];
		PhaseInfo phaseInfo;
		phaseInfo.phaseId = phaseItem.phaseId;
		phaseInfo.pedRequest = phaseItem.autoRequest;
		phaseInfo.motorRequest = phaseItem.autoRequest;
		phaseInfo.channelBits = phaseItem.channelBits;
		phaseInfo.checkTime = phaseItem.checkTime;
		phaseInfo.unitExtendTime = phaseItem.unitExtendTime;
		phaseInfo.maxExtendTime = phaseItem.maxGreenTime - phaseItem.minGreenTime;
		phaseInfo.vehDetectorBits = phaseItem.vehDetectorBits;
		phaseInfo.pedDetectorBits = phaseItem.pedDetectorBits;
		phaseInfo.advanceExtendTime = phaseItem.advanceExtendTime;
		phaseInfo.pedResponseTime = phaseItem.pedResponseTime;
		phaseInfo.pedPhase = PedestrianPhase(phaseInfo.channelBits);
		short pedPassTime = phaseItem.minGreenTime - phaseItem.pedClearTime;
		short pedClearTime = phaseItem.pedClearTime;
		if (pedPassTime < 0)
		{
			pedClearTime = phaseItem.minGreenTime;
			pedPassTime = 0;
		}
		if (phaseInfo.pedPhase)
		{	//行人相位时，把行人放行和行人清空时间作为绿信比时间
			phaseInfo.vehDetectorBits = 0;
			phaseInfo.splitTime = pedPassTime + pedClearTime;
			phaseInfo.motor.emplace_back(pedPassTime, GREEN);
			phaseInfo.motor.emplace_back(pedClearTime, GREEN_BLINK);
			phaseInfo.pedestrian.emplace_back(pedPassTime, GREEN);
			phaseInfo.pedestrian.emplace_back(pedClearTime, GREEN_BLINK);
		}
		else
		{
			if (detectorArray.Normal(phaseInfo.vehDetectorBits))
			{	//正常感应控制,以最小绿+黄灯+全红作为绿信比
				phaseInfo.splitTime = phaseItem.minGreenTime + phaseItem.yellowTime + phaseItem.allRedTime;
				phaseInfo.motor.emplace_back(phaseItem.minGreenTime - phaseItem.greenBlinkTime, GREEN);
			}
			else
			{	//感应控制时如果检测器有故障则默认放行定周期
				phaseInfo.pedRequest = true;
				phaseInfo.motorRequest = true;
				phaseInfo.splitTime = phaseItem.greenTime + phaseItem.greenBlinkTime + phaseItem.yellowTime + phaseItem.allRedTime;
				phaseInfo.motor.emplace_back(phaseItem.greenTime, GREEN);
				pedPassTime = phaseItem.greenTime + phaseItem.greenBlinkTime - phaseItem.pedClearTime;
				if (pedPassTime < 0)
				{
					pedClearTime = phaseItem.greenTime + phaseItem.greenBlinkTime;
					pedPassTime = 0;
				}
			}
			phaseInfo.motor.emplace_back(phaseItem.greenBlinkTime, GREEN_BLINK);
			phaseInfo.motor.emplace_back(phaseItem.yellowTime, YELLOW);
			phaseInfo.motor.emplace_back(phaseItem.allRedTime, ALLRED);
			phaseInfo.pedestrian.emplace_back(pedPassTime, GREEN);
			phaseInfo.pedestrian.emplace_back(pedClearTime, GREEN_BLINK);
			phaseInfo.pedestrian.emplace_back(phaseInfo.splitTime - pedPassTime - pedClearTime, RED);
		}
		//INFO("phaseId: %d, vehDetectorBits: %#llx, spilitTime: %d", phaseInfo.phaseId, phaseInfo.vehDetectorBits, phaseInfo.splitTime);
		schemeInfo.cycleTime += phaseInfo.splitTime;
		schemeInfo.phaseturn.push_back(phaseInfo);
	}
	return (schemeInfo.cycleTime > 0 && schemeInfo.phaseturn.size() > 1);
}

bool Tsc::FillSchemeInfo(const ControlRule &rule, SchemeInfo &schemeInfo)
{
	if (rule.SpecialMode())
	{	//关灯、黄闪、全红特殊控制
		//无需填充数据
		return true;
	}
	if (rule.ctrlId == 0 || rule.ctrlId > MAX_SCHEME_NUM || schemeTable[rule.ctrlId - 1].cycleTime == 0)
	{
		ERR("ctrlId[%d] is invalid", rule.ctrlId);
		return false;
	}
	bool ret = false;
	schemeInfo.Clear();
	configlock.r_lock();
	SchemeItem scheme = schemeTable[rule.ctrlId - 1];
	for (auto &ch : channelTable)
	{
		if (ch.channelId == 0 || ch.channelId > MAX_CHANNEL_NUM || ch.channelType == NOUSE)
			continue;
		schemeInfo.channelInfo[ch.channelId - 1].channelId = ch.channelId;
		schemeInfo.channelInfo[ch.channelId - 1].channelType = ch.channelType;
	}
	schemeInfo.rule = rule;
	
	if (rule.ctrlMode == FIXEDCYCLE_MODE || rule.ctrlMode == COORDINATE_MODE)
	{	//基于定周期的控制
		ret = FillFixCycle(scheme, schemeInfo);
		if (rule.ctrlMode == COORDINATE_MODE && scheme.coordinatePhase > 0 && scheme.coordinatePhase <= MAX_PHASE_NUM)
			CoordinateDeal(scheme, schemeInfo);
	}
	else if (rule.ctrlMode == INDUCTIVE_MODE || rule.ctrlMode == BUS_ADVANCE_MODE 
		|| rule.ctrlMode == SINGLE_SPOT_OPTIMIZE || rule.ctrlMode == PEDESTRIAN_INDUCTIVE_MODE 
		|| rule.ctrlMode == SINGLE_ADAPT_MODE)
	{	//基于感应的控制
		ret = FillInductive(scheme, schemeInfo);
	}
#if 0
	else if (rule.ctrlMode == INDUCTIVE_COORDINATE_MODE)
	{
		
	}
#endif
	else
	{
		ERR("the ctrlMode[%d] isn't supported!", rule.ctrlMode);
	}
	configlock.r_unlock();
	return ret;
}

void Tsc::Custom(const Cycle *cycle)
{
	static int count = 0;
	detectorArray.Check(detectorTable);
	if (cycle != nullptr)
	{
#if 0
		const Phase *phase = cycle->GetCurPhase();
		if (phase != nullptr && phase->GreenEnd() && phase->beginTime > 0)
			detectorArray.CalTrafficInfo(phase->vehDetectorBits, phase->beginTime, time(nullptr) - phase->beginTime);
#endif
		if (switchparam.takeover)
			SendRunInfoTOBoard(*cycle);
	}
	if (++count == basic.vehFlowUploadCycleTime)
	{
		count = 0;
		Frame frame;
		vector<TrafficFlowInfo> &&vec = detectorArray.GetTrafficInfo(detectorTable, time(nullptr) - basic.vehFlowUploadCycleTime, basic.vehFlowUploadCycleTime);
		frame.Send(vec);
	}
}

#ifdef TAKEOVER
UInt8 Tsc::LCBphaseTurnAndSplitInfoSet(LCBphaseturninfo (&phaseturninfo)[MAX_SUPPORT_PHASETURN_NUM], LCBsplitinfo (&splitinfo)[MAX_SUPPORT_SPLIT_NUM])
{
	int i, j;
	
	//由于历史遗留问题把感应控制作为第0方案，所以后续的索引值+1
	for (i = 0; i < MAX_SUPPORT_SCHEME_NUM; i++)
	{	
		auto &scheme = schemeTable[i];
		if (scheme.schemeId == 0 || scheme.schemeId > MAX_SUPPORT_SCHEME_NUM || scheme.cycleTime == 0)
			break;
		for (j = 0; j < MAX_SUPPORT_PHASE_NUM; j++)
		{
			auto phaseId = scheme.phaseturn[j];
			if (phaseId == 0 || phaseId > MAX_SUPPORT_PHASE_NUM)
				break;
			phaseturninfo[i + 1].phases[j] = phaseId;
			auto &phase = phaseTable[phaseId - 1];
			splitinfo[i + 1].times[j] = phase.greenTime + phase.greenBlinkTime + phase.yellowTime + phase.allRedTime;
		}
	}
	phaseturninfo[0] = phaseturninfo[1];
	splitinfo[0] = splitinfo[1];
	return i;
}

UInt8 Tsc::LCBphaseInfoSet(LCBphaseinfo (&phaseinfo)[MAX_SUPPORT_PHASE_NUM])
{
	int i = 0;
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		auto &phase = phaseTable[i];
		if (phase.phaseId == 0 || phase.phaseId > MAX_SUPPORT_PHASE_NUM)
			break;
		phaseinfo[i].greenFlashTime = phase.greenBlinkTime;
		phaseinfo[i].yellowTime = phase.yellowTime;
		phaseinfo[i].allredTime = phase.allRedTime;
		phaseinfo[i].pedFlashTime = phase.pedClearTime;
		phaseinfo[i].channelbits = phase.channelBits;
	}
	return i;
}

void Tsc::LCBconfigSet(LCBconfig &config)
{
	LCBbaseinfo *baseinfo = &config.baseinfo;
	memset(&config, 0, sizeof(LCBconfig));
	baseinfo->isTakeOver = switchparam.takeover;
	baseinfo->isVoltCheckEnable = switchparam.voltCheck;
	baseinfo->isCurCheckEnable = switchparam.curCheck;
	baseinfo->phaseNum = LCBphaseInfoSet(config.phaseinfo);//设置相位的相关信息
	baseinfo->minRedCurVal = 30;
	//设置相序和绿信比的相关信息
	baseinfo->schemeNum = LCBphaseTurnAndSplitInfoSet(config.phaseturninfo, config.splitinfo);
	baseinfo->canTotalNum = baseinfo->schemeNum * 2 + 1 //加1是因为要多发送一个感应方案的绿信比信息
							+ baseinfo->phaseNum;
}

bool Tsc::CheckLCBconfigValidity(LCBconfig *p)
{
	int i, j;
	int totalNum = p->baseinfo.schemeNum * 2 + 1 
					+ p->baseinfo.phaseNum;
	
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		if (p->phaseinfo[i].channelbits == 0)
			break;
	}
	for (j = 0; j < MAX_SUPPORT_PHASETURN_NUM; j++)
	{
		if (p->phaseturninfo[j].turn.first == 0
		|| p->splitinfo[j].split.phase1 == 0)
			break;
	}
	return (i > 0 && j > 0
			&& totalNum == p->baseinfo.canTotalNum
			&& i == p->baseinfo.phaseNum 
			&& j == p->baseinfo.schemeNum + 1);
}

void Tsc::SendLCBconfigToBoard()
{
	LCBconfig config;
	int i, no = 1;

	LCBconfigSet(config);
	if (!CheckLCBconfigValidity(&config))
	{	//设置的LCB配置检查无效
		ERR("LCB config set is invalid, so don't send it to Light Control Board!");
		return;
	}
	INFO("LCB config check valid!!!");
	//检查设置有效，先发送基本信息
	can.SendCanMsgToBoard(LCB_BASEINFO_CAN, &config.baseinfo);
	//发送相位信息,一次发送两个相位的信息
	for (i = 0; i < config.baseinfo.phaseNum; i++, no++)
		can.SendCanMsgToBoard(LCB_PHASEINFO_CAN(no, i), &config.phaseinfo[i]);
	//发送相序信息
	for (i = 1; i <= config.baseinfo.schemeNum; i++, no++)
		can.SendCanMsgToBoard(LCB_PHASETURNINFO_CAN(no, i), &config.phaseturninfo[i]);
	//发送绿信比信息
	for (i = 1; i <= config.baseinfo.schemeNum; i++, no++)
		can.SendCanMsgToBoard(LCB_SPLITINFO_CAN(no, i), &config.splitinfo[i]);
	//最后一次发送感应控制的绿信比信息
	can.SendCanMsgToBoard(LCB_CAN_EXTID(no, 0, SPLITINFO), &config.splitinfo[0]);
}

void Tsc::SendRunInfoTOBoard(const Cycle &cycle)
{
	if (cycle.curPhase == 0 || cycle.curPhase > MAX_SUPPORT_PHASE_NUM)
		return;
	const ColorStep &motor = cycle.phaseTable[cycle.curPhase - 1].motor.Current();
	const ColorStep &ped = cycle.phaseTable[cycle.curPhase - 1].pedestrian.Current();
	UInt8 status = motor.status;
	LightValue lightvalue;
	if (status == GREEN)
		lightvalue = (ped.status == GREEN_BLINK) ? LGREEN_FLASH_PED : LGREEN;
	else if (status == GREEN_BLINK)
		lightvalue = LGREEN_FLASH;
	else if (status == YELLOW)
		lightvalue = LYELLOW;
	else if (status == ALLRED)
		lightvalue = LRED;
	else
		lightvalue = LOFF;
	
	LCBruninfo runinfo;
	runinfo.schemeid = cycle.rule.ctrlId;
	runinfo.runtime = cycle.cycleTime - cycle.leftTime;
	runinfo.phaseR1 = cycle.curPhase;
	runinfo.phaseR2 = 0;
	runinfo.lightvalueR1 = lightvalue;
	runinfo.lightvalueR2 = LOFF;
	can.SendCanMsgToBoard(LCB_CAN_EXTID(0, 0, RUNINFO), &runinfo);
}
#endif

