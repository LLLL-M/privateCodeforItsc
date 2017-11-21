#ifndef __LOG_H__
#define __LOG_H__

#include <vector>
#include "sqlite3.h"
#include "gb2016.h"
#include "file.h"
#include "lock.h"
#include "common.h"

//故障日志类型定义
typedef enum
{
	INVALID_TYPE = 0,								//无效的类型	
	COMMUNICATION_DISCONNECT = 3,					//通信断开
	COMMUNICATION_CONNECT = 4,						//通信断开后又联机
	DETECTOR_NO_RESPONSE = 5,						//检测器无响应
	DETECTOR_NO_RESPONSE_CLEAR = 6,					//检测器无响应清除
	DETECTOR_MAX_CONTINUOUS = 7,					//检测器超过最大持续响应时间
	DETECTOR_MAX_CONTINUOUS_CLEAR = 8,				//检测器超过最大持续响应时间清除
	DETECTOR_MAX_VEHCILENUM = 9,					//检测器超过每分钟最大过车数
	DETECTOR_MAX_VEHCILENUM_CLEAR = 0x0A,			//检测器超过每分钟最大过车数清除
	RED_GREEN_CONFLICT = 0x0B,						//红绿冲突
	RED_GREEN_CONFLICT_CLEAR = 0x0C,				//红绿冲突清除
	RED_LIGHT_OFF = 0x0D,							//红灯熄灭
	RED_LIGHT_OFF_CLEAR = 0x0E,						//红灯熄灭清除
	GREEN_CONFLICT = 0x0F,							//绿冲突
	GREEN_CONFLICT_CLEAR = 0x10,					//绿冲突清除
	YELLOW_LIGHT_FAULT = 0x11,						//黄灯故障
	YELLOW_LIGHT_FAULT_CLEAR = 0x12,				//黄灯故障清除
#if 0
	MANUAL_PANEL_FLASH = 0x13,						//手动黄闪
	MANUAL_PANEL_ALL_RED = 0x14,					//手动全红
	MANUAL_PANEL_STEP = 0x15,						//手动步进
#endif
} FaultLogType;

class Log
{
private:
	HikFile::File file;
	sqlite3 *db;
	int row;
	HikLock::RWlock rwlock;
	struct FaultLogHead
	{
		UInt32 num;
		UInt8 faultStatus[MAX_CHANNEL_NUM];
		FaultLog data[0];
	} *head;
	void Insert(const char *content);
public:
	Log();
	~Log();
	string &Read(UInt32 startTime, UInt32 lineNum, string &log);
	vector<FaultLog> Read(UInt32 startTime, UInt32 lineNum);
	void Write(const char *content);
	void Write(FaultLogType type, const char *content = nullptr, const UInt16 value = 0);
	void Clear();
	void UpdateFault(int channelId, int faultStatus);
	const UInt8 LoadFault(int channelId);
	void LoadFault(UInt8 (&faultStatus)[MAX_CHANNEL_NUM]);
	
protected:
};

#endif
