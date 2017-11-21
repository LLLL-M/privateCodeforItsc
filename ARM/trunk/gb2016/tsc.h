#ifndef __TSC_H__
#define __TSC_H__

#include <string>
#include "its.h"
#include "gb2016.h"
#include "lock.h"
#ifdef TAKEOVER
#include "lcb.h"
#endif

using namespace HikLock;
class Loopqueue;
class Log;
class DetectorArray;
class Can;

class Tsc: public Its
{
private:
	//默认值为0
	UInt8			updateFlag;//bit0:通道表,bit1:相位表,bit2:方案表,bit3:调度计划表,bit4:车检器表
	ChannelItem		bakChannelTable[MAX_CHANNEL_NUM];
	PhaseItem		bakPhaseTable[MAX_PHASE_NUM];
	SchemeItem		bakSchemeTable[MAX_SCHEME_NUM];
	ScheduleItem	bakScheduleTable[MAX_SCHEDULE_NUM];
	DetectorItem	bakDetectorTable[MAX_DETECTOR_NUM];

	RWlock			configlock;
	
	Log &log;
	DetectorArray	&detectorArray;
	Can &can;

	int timeGapSec;		//当前计算时间与其实时段的差值,单位:秒
	int leftTransCycle;		//剩余过渡周期
	
	void SetGpsAndWatchdog();
	bool CheckChannel(const UInt32 channelBits);
	bool CheckPhase(const PhaseItem &p, UInt16 &splitTime);
	bool CheckScheme(const SchemeItem &s);
	bool CheckConfig();
	bool UpdateConfig();
	bool SetChannelTable(const UInt8 *data, int size);
	bool SetPhaseTable(const UInt8 *data, int size);
	bool SetSchemeTable(const UInt8 *data, int size);
	bool SetScheduleTable(const UInt8 *data, int size);
	bool SetDetectorTable(const UInt8 *data, int size);
	
	void CoordinateDeal(SchemeItem &scheme, SchemeInfo &schemeInfo);
	bool PedestrianPhase(bitset<MAX_CHANNEL_NUM> &channelBits);
	bool FillFixCycle(const SchemeItem &scheme, SchemeInfo &schemeInfo);
	bool FillInductive(const SchemeItem &scheme, SchemeInfo &schemeInfo);
	
#ifdef TAKEOVER
	UInt8 LCBphaseTurnAndSplitInfoSet(LCBphaseturninfo (&phaseturninfo)[MAX_SUPPORT_PHASETURN_NUM], LCBsplitinfo (&splitinfo)[MAX_SUPPORT_SPLIT_NUM]);
	UInt8 LCBphaseInfoSet(LCBphaseinfo (&phaseinfo)[MAX_SUPPORT_PHASE_NUM]);
	void LCBconfigSet(LCBconfig &config);
	bool CheckLCBconfigValidity(LCBconfig *p);
	void SendLCBconfigToBoard();
	void SendRunInfoTOBoard(const Cycle &cycle);
#else
	void SendLCBconfigToBoard() {}
	void SendRunInfoTOBoard(const Cycle &cycle) {}
#endif

public:
	ChannelItem		channelTable[MAX_CHANNEL_NUM];
	PhaseItem		phaseTable[MAX_PHASE_NUM];
	SchemeItem		schemeTable[MAX_SCHEME_NUM];
	ScheduleItem	scheduleTable[MAX_SCHEDULE_NUM];
	DetectorItem	detectorTable[MAX_DETECTOR_NUM];
	SwtichParam		switchparam;
	Basic			basic;			//基础信息
	
	Tsc();
	bool Write(UInt8 objectId, const string &data);
	void Read(UInt8 objectId, const string &arg, string &buf);
	void StartInit(vector<ColorStep> &vec);
	bool GetLocalRule(const int aheadOfTime, ControlRule &rule);
	bool FillSchemeInfo(const ControlRule &rule, SchemeInfo &schemeInfo);
	void Custom(const Cycle *cycle);

};

#endif
