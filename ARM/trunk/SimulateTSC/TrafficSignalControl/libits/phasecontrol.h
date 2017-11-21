#ifndef __PHASECONTROL_H_
#define __PHASECONTROL_H_

#include <ctime>
#include <mutex>
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
namespace HikPhasecontrol
{
	using HikThread::Thread;
	using HikIts::Protocol;
	using HikIpc::Ipc;
	
	class Phasecontrol: public Thread
	{
	private:
		enum 
		{
			STEP_UNUSED_FLAG = 0,	//步进未使用标志
			STEP_EXCUTE_FLAG = 1,	//步进执行标志
		};

		ControlType	curControlType;	//当前的控制类型
		ControlMode curMode;	//当前控制方式
		UInt8 stepflag;	//步进控制时使用
		UInt8 stageNum;	//步进阶段号
		UInt8 mSchemeId;	//控制方案号
		LineQueueData m_data;
		
		Protocol & ptl;
		Ipc & ipc;
		
		//黄闪、全红、关灯控制类
		class SpecialControl
		{
		private:
			Phasecontrol & pc;	//PhaseControl简称pc,相位控制模块的对象引用
			void SetSpecialData(LineQueueData *data, LightStatus status);
		public:
			SpecialControl(Phasecontrol & p) : pc(p) {}
			void Deal(LineQueueData *data);
		protected:
		} specialCtl;
		friend class SpecialControl;
		
		//感应和协调感应控制类
		class InductiveControl: public Thread
		{
		private:
			Phasecontrol & pc;	//PhaseControl简称pc,相位控制模块的对象引用
			UInt16 m_extendTime;	//感应控制时使用
			UInt16 m_cycleLeftTime;	//感应协调控制时剩余的周期时间
			UInt16 totalExtendTime[MAX_PHASE_NUM];	//存放感应控制时相位已经延长的时间和
			LineQueueData m_cycleLeftData;	//感应协调控制时存放周期剩余时间的临时数据
			volatile UInt64 gVehicleData;	//车检数据,0-47bit分别对应1-48号车检器,分别对应1-48号车检器，0:无过车,1:有过车
			
			void ReduceInductiveExtendTime(LineQueueData *data);
			void AdjustInductiveLeftTime(LineQueueData *data, UInt16 extendTime, UInt8 inductivePhaseIndex);
			UInt16 CheckVehicleData(LineQueueData *data);
			UInt16 InductiveCoordinateDeal(LineQueueData *data, LineQueueData *cycleLeftData);
			void ClearData();
		public:
			InductiveControl(Phasecontrol & p) : pc(p) { ClearData(); }
			void Deal(LineQueueData *data);
			void run(void *arg);
			void StartVehCollectThread() { start(); }
		protected:
		} inductiveCtl;
		friend class InductiveControl;
		
		//步进控制类
		class StepControl
		{
		private:
			Phasecontrol & pc;	//PhaseControl简称pc,相位控制模块的对象引用
            bool WithinTransitionalPeriod(PhaseInfo *phaseInfos);
			bool IsStepTransitionComplete(LineQueueData *data); //判断步进过渡是否完成
			void DirectStepToStage(LineQueueData *data, UInt8 stageNum); //直接步进到某个阶段，中间不会有绿闪、黄灯和全红的过渡
		public:
			StepControl(Phasecontrol & p) : pc(p) {} 
			void Deal(LineQueueData *data);
			bool IsStepInvalid(LineQueueData *data, UInt8 stageNum); //判断步进是否无效
		protected:
		} stepCtl;
		friend class StepControl;
#if 0
		//通道锁定控制类
		class ChannelLock
		{
		private:
			enum 
			{
				GREEN_BLINK_TRANSITION_TIME = 3,	//绿闪过渡时间
				YELLOW_TRANSITION_TIME = 3,			//黄灯过渡时间
			};
			std::mutex gChannelLockMutex;	//通道锁定的互斥锁
			ChannelLockParams gChannelLockParams;				//通道锁定参数
			UInt8 gChannelLockReserveStatus[MAX_CHANNEL_NUM];	//通道锁定保留状态
			UInt8 gChannelLockFlag;							//通道锁定标志，1:锁定，2:未锁定
			UInt8 gGreenBlinkTransitionTime;	//绿闪过渡时间
			UInt8 gYellowTransitionTime;		//黄灯过渡时间
			
			void LockTransition(UInt8 *curChannels); //通道锁定过渡
			bool IsUnlockAvailable(UInt8 *curChannels);	//是否可进行通道解锁
		public:
			ChannelLock();
			void Lock(ChannelLockParams *lockparams);
			void Unlock();
			void Deal(LineQueueData *data);
		protected:
		} channelLock;
#endif
		volatile std::time_t gCurTime;		//当前时间
		void GetLocalTime();
#if defined(__linux__) && defined(__arm__)
		int fd;
		UInt8 ledstatus;
		std::time_t GetGPSTime();
		void OpenGpsPort();
#endif 
		
		void ReadLineQueueData(LineQueueData *data, UInt8 schemeId);
        //步进时读取线性队列数据，只为步进使用
        void ReadLineQueueDataForStep(LineQueueData *data);
        //void SendChannelStatus();
		void RecvControlModeMsg();
		void SystemInit();
		void DataDeal();
        //void RestoreChannelStatus();
	public:
		Phasecontrol(Protocol & p, Ipc & i);
        ~Phasecontrol() {}
		void run(void *arg);
#if 0
		void Lock(ChannelLockParams *lockparams) { channelLock.Lock(lockparams); }
		void Unlock() { channelLock.Unlock(); }
#endif
	protected:
	};
}


#endif
