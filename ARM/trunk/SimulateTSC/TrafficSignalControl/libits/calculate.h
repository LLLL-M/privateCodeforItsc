#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include "thread.h"
#include "tsc.h"

namespace HikIpc
{
	class Ipc;
}
namespace HikIts
{
	class Protocol;
}
namespace HikCalculate
{
	using HikThread::Thread;
	using HikIts::Protocol;
	using HikIpc::Ipc;
	
	class Calculate: public Thread
	{
	private:
		Protocol & ptl;
		Ipc & ipc;
		UInt8 bootYellowFlashTime;
		UInt8 bootAllRedTime;
		CalInfo calInfo;
		
		void YellowBlinkAllRedInit();
		UInt8 nextStage(UInt8 curStage) { return (curStage % calInfo.maxStageNum + 1); }
		bool IsPhaseIncludeStage(int phaseId, int stageNum);
		bool IsStageIncludePhase(int stageNum, int phaseId);
		int CalculateTransitionTime(UInt16 cycleTime, int timeGap, int phaseOffset);
		void TransitionDeal();
		UInt8 FindFollowPhaseEndStage(FollowPhaseInfo *followPhaseInfo, UInt8 curStage, UInt8 *endPhase);
		UInt8 GetFollowPhaseStartMotherPhase(FollowPhaseInfo *followPhaseInfo, UInt8 curStage);
		void SetFollowPhaseInfo(LineQueueData *data, UInt8 curStage);
		void SendSpecialControlData(UInt8 schemeId, UInt8 leftTime);
		UInt8 GetPhaseEndStageNum(UInt8 phaseId, UInt8 curStageNum);
		void SetPedestrianTime(PhaseTimeInfo *times);
		void SetStagePhaseTime(UInt8 stageNum);
		LightStatus GetPhaseStatusByTime(PassTimeInfo *passTimeInfo);
		UInt16 FindPhaseNextRunPassTime(UInt8 phaseId, UInt8 curStageNum);
		void SetChannelInfo(LineQueueData *data, UInt32 channelBits, UInt8 status, UInt16 leftTime);
		void SetStagePhaseInfo(UInt8 stageNum, UInt16 stageLeftTime, LineQueueData *data);
		void RunCycle();
	public:
		Calculate(Protocol & p, Ipc & i);
        ~Calculate() {}
		void run(void *arg);
	protected:

	};
}


#endif
