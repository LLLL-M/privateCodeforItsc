#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include <map>
#include <set>
#include <ctime>
#include <atomic>
#include <vector>
#include "lock.h"
#include "common.h"
#include "gb2016.h"

class Log;

class Detector
{
private:
	enum 
	{
		NORMAL = 0,			//检测器正常
		NORESPONSE = 1,		//检测器无响应
		MAXCONTINUOUS = 2,	//检测器持续响应
		MAXVEHCILENUM = 3,		//检测器超过每分钟最大过车数
	};
	volatile int status;
	HikLock::RWlock lock;
	typedef multimap<time_t, time_t>::value_type VT;
	multimap<time_t, time_t> enter;	//key:车辆进入时间, value:车辆通过检测器时间,即车辆离开时间-车辆进入时间
	multimap<time_t, time_t> leave;	//key:车辆离开时间, value:车辆通过检测器时间,即车辆离开时间-车辆进入时间
	enum {MAX_COUNT = 1024};		//enter和leave存储的最大个数
	Log &log;
	const time_t beginTime;				//起始时间
	
	struct VehInfo
	{
		int data = 0;
		int count = 0;
		void Set(int d)
		{
			data += d;
			count++;
		}
		int Get()
		{
			if (count <= 0)
				return 0;
			else
			{
				int avg = data / count;
				data = count = 0;
				return avg;
			}
		}
	};
	VehInfo occupancyRate;
	VehInfo queueLength;
	
	//检测器是否无响应
	bool NoResponse(int noResponseTime);
	//检测器是否超过最大持续响应时间
	bool MaxContinuous(int maxContinuousTime);
	//检测器是否超过每分钟最大通行车辆数
	bool MaxVehcileNum(int maxVehcileNum);
	
public:
	Detector();
	
	void Enter();
	
	void Leave();
	
	bool HasEnter(time_t t, const int sec = 1);
	
	bool Check(const DetectorItem &item);
	
	int Flow(time_t t, const int sec);
	
	void OccupancyRate(time_t t, const int sec);	//占有率 = 车辆通过检测器所用时间之和 / 总时间(绿灯时间)，单位0.5%
	int OccupancyRate() { return occupancyRate.Get(); }
	
	int Speed(time_t t, const int sec, const int length = 5 + 2);	//长度=车长+线圈间距，速度 = 长度 / 车辆通过检测器平均所用时间，单位km/h
	
	void QueueLength(time_t t, const int sec, const int space, const int vehLen, const float vehGap);
	int QueueLength() { return queueLength.Get(); }

protected:
};

class DetectorArray
{
private:
	array<Detector, MAX_DETECTOR_NUM>	detectors;
	array<bool, MAX_DETECTOR_NUM>		checkResults;

public:
	DetectorArray();
	
	void Enter(UInt8 lane)
	{
		if (lane == 0 || lane > MAX_DETECTOR_NUM)
			return;
		detectors[lane - 1].Enter();
	}

	void Leave(UInt8 lane)
	{
		if (lane == 0 || lane > MAX_DETECTOR_NUM)
			return;
		detectors[lane - 1].Leave();
	}

	UInt64 GetEnter(time_t t, int sec = 1);
	
	void Check(const DetectorItem (&table)[MAX_DETECTOR_NUM]);
	
	bool Normal(const UInt64 &vehDetectorBits);
	
	void CalTrafficInfo(const UInt64 &vehDetectorBits, time_t t, const int sec, const int space = 2, const int vehLen = 5, const float vehGap = 1.5);
	
	vector<TrafficFlowInfo> GetTrafficInfo(const DetectorItem (&table)[MAX_DETECTOR_NUM], time_t t, const int sec);
protected:
};

#endif
