#ifndef __IPC_H__
#define __IPC_H__

#include <ctime>
#include "tsc.h"
#include "msg.h"
#include "lfq.h"

//命名空间的前向声明，避免引用大量的头文件
namespace HikIts
{
	class Its;
}
namespace HikCalculate
{
	class Calculate;
}
namespace HikPhasecontrol
{
	class Phasecontrol;
}
namespace HikClock
{
	class Clock;
}
namespace HikPhasedriver
{
	class Phasedriver;
}
namespace HikStrategycontrol
{
	class Strategycontrol;
}
namespace HikRedsigcheck
{
	class Redsigcheck;
}
namespace HikChannelHandle
{
    class ChannelHandle;
}

namespace HikIpc
{
	using HikIts::Its;
	using HikCalculate::Calculate;
	using HikPhasecontrol::Phasecontrol;
	using HikClock::Clock;
	using HikPhasedriver::Phasedriver;
	using HikStrategycontrol::Strategycontrol;
	using HikRedsigcheck::Redsigcheck;
    using HikChannelHandle::ChannelHandle;
	
	using HikSem::Sem;
	using HikMsg::Msg;
	using HikLfq::Lfq;

	class Ipc
	{
	private:
		friend class Its;
		friend class Calculate;
		friend class Phasecontrol;
		friend class Clock;
		friend class Phasedriver;
		friend class Strategycontrol;
		friend class Redsigcheck;
        friend class ChannelHandle;

		struct ControlTypeMsg
		{
			ControlType	controlType;	//策略控制模块接收的控制类型
			ControlMode mode;		//策略控制模块发给相位控制模块的控制方式
            Int32 stageNum;		//步进阶段号，单步步进时为0，
			UInt8 mSchemeId;	//表示手动方案号
		};
		
		struct ControlModeMsg
		{
			ControlType	controlType;	//策略控制模块接收的控制类型
			ControlMode mode;		//策略控制模块发给相位控制模块的控制方式
            Int32 stageNum;		//步进阶段号，单步步进时为0，
			UInt8 mSchemeId;	//表示手动方案号
		};

		//相位控制模块发送给计算模块
		struct CalMsg
		{
			UInt8 schemeId;		//开始计算的下一周期所应当使用的方案，0:表示按照本地系统方案
			std::time_t calTime;		//开始计算下一周期的时间
		};

		struct ChannelStatusMsg
		{
			UInt8 allChannels[MAX_CHANNEL_NUM];		//相位控制模块发送给相位驱动模块的通道状态
		};
		
		struct RedStatusMsg
		{
			UInt8 allChannels[MAX_CHANNEL_NUM];		//相位控制模块发送给红灯信号检测模块的通道状态
		};
		
		//通道检测的相关参数
		struct ChannelCheckMsg
		{
			UInt8 channelId;
			LightStatus channelStatus;
		};
	
		Sem gBeginReadData;						//计算模块发送给相位驱动模块开始读取数据
		Sem gStartTimer;						//相位控制模块发送给定时器模块开始定时
		Sem gTimerForPhaseCtl;					//相位控制模块的定时器
		Sem gTimerForPhaseDrv;					//相位驱动模块的定时器
		Sem gSemForVeh;							//相位控制模块与车检器采集模块通信
        Sem gSemForChan;                        //相位控制模块与通道处理模块通信
		
		Msg<CalMsg> gStartCalculateNextCycle;	//相位控制模块发送给计算模块开始计算下一周期
		Msg<ChannelStatusMsg> gChannelStatus;	//相位控制模块发送给相位驱动模块的通道状态
		Msg<RedStatusMsg> gRedSignalCheck;	//相位控制模块发送给红灯信号检测器模块，每秒一次
		Msg<ControlModeMsg> gControlMode;			//策略控制摸发给相位驱动模块的控制方式
		Msg<ControlTypeMsg> gControlType;			//通信、数据采集等等其他模块发送给策略控制模块的控制类型消息
		Msg<ChannelCheckMsg> gChannelCheck;		//通信模块发送给相位控制模块的消息
		
		Lfq gLfq;
		
		void SemPostBeginReadData() { gBeginReadData.post(); }
		bool SemWaitBeginReadData(bool isblock = true) { return gBeginReadData.wait(isblock); }
		void SemPostStartTimer() { gStartTimer.post(); }
		bool SemWaitStartTimer(bool isblock = true) { return gStartTimer.wait(isblock); }
		void SemPostTimerForPhaseCtl() { gTimerForPhaseCtl.post(); }
		bool SemWaitTimerForPhaseCtl(bool isblock = true) { return gTimerForPhaseCtl.wait(isblock); }
		void SemPostTimerForPhaseDrv() { gTimerForPhaseDrv.post(); }
		bool SemWaitTimerForPhaseDrv(bool isblock = true) { return gTimerForPhaseDrv.wait(isblock); }
		void SemPostForVeh() { gSemForVeh.post(); }
        bool SemWaitForVeh(bool isblock = true) { return gSemForVeh.wait(isblock); }
        void SemPostForChan() { gSemForChan.post(); }
        bool SemWaitForChan(bool isblock = true) { return gSemForChan.wait(isblock); }
		
        void MsgSend(ControlType type, UInt8 schemeId, Int32 val)
		{
			ControlTypeMsg msg = {type, SYSTEM_MODE, 0, schemeId};
			switch (schemeId)
			{   
				case YELLOWBLINK_SCHEMEID: msg.mode = YELLOWBLINK_MODE; break;
				case ALLRED_SCHEMEID: msg.mode = ALLRED_MODE; break;
				case TURNOFF_SCHEMEID: msg.mode = TURNOFF_LIGHTS_MODE; break;
				case INDUCTIVE_SCHEMEID: msg.mode = INDUCTIVE_MODE; msg.mSchemeId = val; break;
				case INDUCTIVE_COORDINATE_SCHEMEID: msg.mode = INDUCTIVE_COORDINATE_MODE; msg.mSchemeId = val; break;//感应协调控制
				case STEP_SCHEMEID: msg.mode = STEP_MODE; msg.stageNum = val; break;
				case SYSTEM_RECOVER_SCHEMEID: msg.mode = SYSTEM_MODE; break;
				default: msg.mode = (type == AUTO_CONTROL) ? SYSTEM_MODE : MANUAL_MODE; break;    
			}
			gControlType.send(msg);
		}
		void MsgSend(ControlTypeMsg & msg) { gControlType.send(msg); }
		bool MsgRecv(ControlTypeMsg & msg, bool isblock = true) { return gControlType.recv(msg, isblock); }
		void MsgSend(CalMsg & msg) { gStartCalculateNextCycle.send(msg); }
		bool MsgRecv(CalMsg & msg, bool isblock = true) { return gStartCalculateNextCycle.recv(msg, isblock); }
		void MsgSend(ChannelStatusMsg & msg) { gChannelStatus.send(msg); }
		bool MsgRecv(ChannelStatusMsg & msg, bool isblock = true) { return gChannelStatus.recv(msg, isblock); }
		void MsgSend(RedStatusMsg & msg) { gRedSignalCheck.send(msg); }
		bool MsgRecv(RedStatusMsg & msg, bool isblock = true) { return gRedSignalCheck.recv(msg, isblock); }
		void MsgSend(ControlModeMsg & msg) { gControlMode.send(msg); }
		bool MsgRecv(ControlModeMsg & msg, bool isblock = true) { return gControlMode.recv(msg, isblock); }
		void MsgSend(ChannelCheckMsg & msg) { gChannelCheck.send(msg); }
		bool MsgRecv(ChannelCheckMsg & msg, bool isblock = true) { return gChannelCheck.recv(msg, isblock); }
		
	public:
		Ipc() : gBeginReadData("BeginReadData"), \
				gStartTimer("StartTimer"), \
				gTimerForPhaseCtl("TimerForPhaseCtl"), \
				gTimerForPhaseDrv("TimerForPhaseDrv"), \
				gSemForVeh("VehCheckCollect"), \
                gSemForChan("SemForChannelHandle"), \
				gStartCalculateNextCycle("StartCalculateNextCycle"), \
				gChannelStatus("ChannelStatus"), \
				gRedSignalCheck("RedSignalCheck"), \
				gControlMode("ControlMode"), \
				gControlType("ControlType"), \
				gChannelCheck("ChannelCheck"), \
				gLfq("LFQ", (2 << 20), sizeof(LineQueueData))
		{
			
		}
	protected:
		
	};
}

#endif
