#include <algorithm>
#include "detector.h"
#include "log.h"
#include "singleton.h"

Detector::Detector() : log(Singleton<Log>::GetInstance()), beginTime(time(nullptr))
{
	status = NORMAL;
}

bool Detector::NoResponse(int noResponseTime)
{	//在无响应时间内如果车辆没有进入则认为该车检器出现无响应故障了
	const int sec = noResponseTime * 60;
	time_t t = time(nullptr) - sec;
	if (noResponseTime <= 0 || (status != NORESPONSE && status != NORMAL) || t <= beginTime)
		return false;
	return !HasEnter(t, sec);
}

bool Detector::MaxContinuous(int maxContinuousTime)
{	//车辆通过车检器时间超过最大持续时间则说明该车检器出现最大持续时间故障了
	const int sec = maxContinuousTime * 60;
	time_t t = time(nullptr) - sec;
	if (maxContinuousTime <= 0 || (status != MAXCONTINUOUS && status != NORMAL) || t <= beginTime)
		return false;
	bool ret = false;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		auto itt = leave.equal_range(t);
		auto it = find_if(itt.first, itt.second, [sec](VT &ele)->bool{return ele.second > sec;});
		if (it != itt.second)		//找到一个车辆离开时通过检测器时间超过最大持续时间的键值对
		{
			ret = true;
			break;
		}
	}
	lock.r_unlock();
	return ret;
}

bool Detector::MaxVehcileNum(int maxVehcileNum)
{	//一分钟内通过检测器的车辆超过最大车辆数则说明出现最大车辆数故障了
	const int sec = 60;
	time_t t = time(nullptr) - sec;
	if (maxVehcileNum <= 0 || (status != MAXVEHCILENUM && status != NORMAL) || t <= beginTime)
		return false;
	return Flow(t, sec) > maxVehcileNum;
}

void Detector::Enter()
{
	time_t key = time(nullptr);
	lock.w_lock();
	if (enter.size() >= MAX_COUNT)
	{	//如果插入元素超过最大个数，则从前面开始删除一半
		auto it = enter.begin();
		for (int i = 0; i < MAX_COUNT / 2; i++)
			enter.erase(it++);
	}
	enter.emplace(key, 0);
	lock.w_unlock();
}

void Detector::Leave()
{
	time_t key = time(nullptr);
	time_t value = 0;
	lock.w_lock();
	for (auto it = enter.begin(); it != enter.end(); it++)
	{
		if (it->second == 0)	//表明车辆进入还未离开
		{
			if (key < it->first)	//可能是时间校正从而导致车辆离开时间小于车辆进入的时间
				enter.erase(it);	//此时应删除之前的车辆进入数据
			else
			{
				value = key - it->first;
				it->second = value;
				break;
			}
		}
	}
	if (leave.size() >= MAX_COUNT)
	{	//如果插入元素超过最大个数，则从前面开始删除一半
		auto it = leave.begin();
		for (int i = 0; i < MAX_COUNT / 2; i++)
			leave.erase(it++);
	}
	leave.emplace(key, value);
	lock.w_unlock();
}

bool Detector::HasEnter(time_t t, const int sec)
{
	bool ret = false;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		if (enter.find(t) != enter.end())
		{
			ret = true;
			break;
		}
	}
	lock.r_unlock();
	return ret;
}

bool Detector::Check(const DetectorItem &item)
{
	char buf[128] = {0};
	if (NoResponse(item.noResponseTime))
	{	//无响应时间检测
		if (status == NORMAL)
		{
			status = NORESPONSE;
			snprintf(buf, sizeof(buf), "detector %d no response!!!", item.detectorId);
			log.Write(DETECTOR_NO_RESPONSE, buf, item.detectorId);
		}
		return false;
	}
	if (MaxContinuous(item.maxContinuousTime))
	{	//最大持续时间检测
		if (status == NORMAL)
		{
			status = MAXCONTINUOUS;
			snprintf(buf, sizeof(buf), "detector %d max continuous response!!!", item.detectorId);
			log.Write(DETECTOR_MAX_CONTINUOUS, buf, item.detectorId);
		}
		return false;
	}
	if (MaxVehcileNum(item.maxVehcileNum))
	{	//最大车辆数检测
		if (status == NORMAL)
		{
			status = MAXVEHCILENUM;
			snprintf(buf, sizeof(buf), "detector %d exceed max vehcile number!!!", item.detectorId);
			log.Write(DETECTOR_MAX_VEHCILENUM, buf, item.detectorId);
		}
		return false;
	}
	//说明检测器正常,如果之前处于故障状态则恢复状态并记录日志
	if (status == NORESPONSE)
	{
		status = NORMAL;
		snprintf(buf, sizeof(buf), "detector %d no response clear.", item.detectorId);
		log.Write(DETECTOR_NO_RESPONSE_CLEAR, buf, item.detectorId);
	}
	else if (status == MAXCONTINUOUS)
	{
		status = NORMAL;
		snprintf(buf, sizeof(buf), "detector %d max continuous response clear.", item.detectorId);
		log.Write(DETECTOR_MAX_CONTINUOUS_CLEAR, buf, item.detectorId);
	}
	else if (status == MAXVEHCILENUM)
	{
		status = NORMAL;
		snprintf(buf, sizeof(buf), "detector %d exceed max vehcile number clear.", item.detectorId);
		log.Write(DETECTOR_MAX_VEHCILENUM_CLEAR, buf, item.detectorId);
	}
	return true;
}

int Detector::Flow(time_t t, const int sec)
{
	int sum = 0;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		sum += leave.count(t);
	}
	lock.r_unlock();
	return sum;
}

void Detector::OccupancyRate(time_t t, const int sec)
{
	if (sec <= 0)
		return;
	int sum = 0;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		auto itt = enter.equal_range(t);
		auto it = find_if(itt.first, itt.second, [](VT &ele)->bool{return ele.second > 0;});
		if (it != itt.second)	//说明这1s有车通过检测器
			sum += it->second;
	}
	lock.r_unlock();
	occupancyRate.Set(sum * 200 / sec);
}

int Detector::Speed(time_t t, const int sec, const int length)
{
	if (length <= 0)
		return 0;
	int sum = 0, count = 0;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		auto itt = enter.equal_range(t);
		count += count_if(itt.first, itt.second, [&sum](VT &ele)->bool{
			sum += ele.second;
			return ele.second > 0;
		});
	}
	lock.r_unlock();
	return (count > 0) ? (count * length * 3600 / (sum  * 1000)) : 0;//换算成km/h
}

void Detector::QueueLength(time_t t, const int sec, const int space, const int vehLen, const float vehGap)
{
	time_t prev = t;
	int veh = 0;	//连续过车数
	int length = 0;
	lock.r_lock();
	for (int i = 0; i < sec; i++, t++)
	{
		if (leave.find(t) != leave.end())
		{
			if (t - prev <= space)
			{	//如果相邻两辆车的离开时间不超过space,则认为是连续过车
				veh++;
				prev = t;
			}
			else
				break;
		}
	}
	lock.r_unlock();
	queueLength.Set((veh > 0) ? int(veh * vehLen + (veh - 1) * vehGap) : 0);
}

DetectorArray::DetectorArray()
{
	for (auto &i : checkResults)
		i = true;
}

UInt64 DetectorArray::GetEnter(time_t t, int sec)
{
	UInt64 value = 0;
	int i = 0;
	for (auto &d : detectors)
	{
		if (checkResults[i] && d.HasEnter(t, sec))
			SET_BIT(value, i);	//检测器无故障并且有过车时对应bit置位
		if (++i >= 64)
			break;
	}
	return value;
}

void DetectorArray::Check(const DetectorItem (&table)[MAX_DETECTOR_NUM])
{
	for (int i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (table[i].detectorId == (i + 1))
			checkResults[i] = detectors[i].Check(table[i]);
	}
}

bool DetectorArray::Normal(const UInt64 &vehDetectorBits)
{
	for (int i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (GET_BIT(vehDetectorBits, i) == 1 && !checkResults[i])
			return false;
	}
	return true;
}

void DetectorArray::CalTrafficInfo(const UInt64 &vehDetectorBits, time_t t, const int sec, const int space, const int vehLen, const float vehGap)
{
	for (int i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (GET_BIT(vehDetectorBits, i) == 1)
		{
			detectors[i].OccupancyRate(t, sec);
			detectors[i].QueueLength(t, sec, space, vehLen, vehGap);
		}
	}
}

vector<TrafficFlowInfo> DetectorArray::GetTrafficInfo(const DetectorItem (&table)[MAX_DETECTOR_NUM], time_t t, const int sec)
{
	vector<TrafficFlowInfo> vec;
	TrafficFlowInfo info;
	for (int i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (table[i].detectorId == (i + 1))
		{
			info.detectorId = table[i].detectorId;
			info.vehicleFlow = detectors[i].Flow(t, sec);
			info.beginTime = t;
#if 0
			info.occupancyRate = detectors[i].OccupancyRate();
			info.vehicleSpeed = detectors[i].Speed(t, sec);
			info.vehicleLength = 5;	//车长默认5米
			info.queueLength = detectors[i].QueueLength();
#endif
			vec.push_back(info);
		}
	}
	return vec;
}
