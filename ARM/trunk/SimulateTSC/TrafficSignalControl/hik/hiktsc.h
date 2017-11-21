#ifndef __HIKTSC_H__
#define __HIKTSC_H__

#include "HikConfig.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "common.h"
#include "sqlite3.h"
#include "sqlite_conf.h"
#include "countdown.h"
#include "chanlockcontrol.h"
#include "protocol.h"
#include "lock.h"

using HikIts::Protocol;
using HikLock::RWlock;

enum PerSecTimes
{
    TWO = 2,
    FOUR = 4,
    EIGHT = 8,
    TEN = 10,
};

class Hiktsc: public Protocol
{
private:
	void CalConflictChannel(const PhaseItem *phaseTable, const ChannelItem *channelTable, const FollowPhaseItem *followPhaseTable, UInt32 *conflictChannels);
	void SetChannelBits(CalInfo *calInfo);
	void SetPhaseTime(CalInfo *calInfo);
	UInt8 GetTimeIntervalID(struct tm *now);
	UInt8 GetSchemeIdAndTimeGap(CalInfo *calInfo, struct tm *now, int *ret);
	void SetInductiveControlTime(CalInfo *calInfo);
	void SetInductiveCoordinateControlTime(CalInfo *calInfo);
	void SetNextCycleRealtimePattern(CalInfo *calInfo);

	void SetBarrier(CalInfo *calInfo, UInt8 stageNO);
	void CalStageInfo(CalInfo *calInfo);
	Boolean IsStageIgnore(CalInfo *calInfo, StageInfo *s);
	void AdjustPhaseIncludeStage(CalInfo *calInfo, UInt8 ignoreStageNO);
	UINT8 FindPrevPhase(CalInfo *calInfo, UInt8 ignorePhaseId);
	UINT8 FindNextPhase(CalInfo *calInfo, UInt8 ignorePhaseId);
	void AdjustIgnorePhaseTime(CalInfo *calInfo, UInt8 prevPhase, UInt8 nextPhase, UInt8 ignorePhase);
	void IgnorePhaseDeal(CalInfo *calInfo);
	UINT8 ItsGetDetectorMapPhaseId(UINT8 nDetectorId);
	

public:
	RWlock gCountDownLock;
	RWlock gConfigLock;
    RWlock gChlock;
	UINT32 gConfigSize;
	SignalControllerPara *gRunConfigPara;
	LineQueueData gCurRunData;
	PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParams;
    CountDown* gCountdown;
    ChanLockControl* gChanLockCtrl;
	MsgRealtimePattern gCurrentCycleRealtimePattern;
	MsgRealtimePattern gNextCycleRealtimePattern;
	UInt32 gConflictChannel[2][NUM_CHANNEL];
	UInt8 gIgnoreOption;
	
    Hiktsc(CountDown* pcountdown, ChanLockControl* pchlockcontrol, McastInfo *info, int times);
    Hiktsc(McastInfo *info, PerSecTimes times);
    ~Hiktsc();

	Boolean FillCalInfo(CalInfo *calInfo, UInt8 mSchemeId, std::time_t calTime);
	void ItsGetRealtimePattern(void *udpdata);
    void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data);
	void ItsCountDownGet(void *countdown, int size);
	void ItsGetConflictChannel(UInt32 *conflictChannels);
	void ItsCustom(LineQueueData *data);
	void ItsSetConfig(void *config, int configSize);
	void ItsGetConfig(void *config, int configSize);
    //int ItsControlStatusGet();
    unsigned int GetBootTransitionTime();
    void ItsCountDownOutput(LineQueueData *data);
    void ItsGetCurRunData(LineQueueData *data);
    void ItsSetCurRunData(const LineQueueData *data);
    unsigned char ChannelControl(unsigned char *chan);
    void channelLockTransition(unsigned char lockFlag, unsigned char *curStatus, unsigned char *lockstatus);


protected:
};

#endif
