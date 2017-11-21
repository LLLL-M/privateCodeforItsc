#include <cstring>
#include "ipc.h"
#include "protocol.h"
#include "phasecontrol.h"

using namespace HikPhasecontrol;

#include "gettime.hpp"
#include "special.hpp"
#include "inductive.hpp"
#include "step.hpp"


//读取线性队列中的1s数据信息
inline void Phasecontrol::ReadLineQueueData(LineQueueData *data, UInt8 schemeId)
{
	int left = ipc.gLfq.lfq_element_count();
	
	if (left <= ptl.aheadOfTime)
	{
	    Ipc::CalMsg msg = {schemeId, gCurTime + left};
    	ipc.MsgSend(msg);
	}
	ipc.gLfq.lfq_read(static_cast<void *>(data));	//读取一个元素
}

inline void Phasecontrol::ReadLineQueueDataForStep(LineQueueData *data)
{
    int left = ipc.gLfq.lfq_element_count();

    if (left == 0)
    {//当队列中没有数据时，倒序一个周期读取当前周期的第1s从头开始
        ipc.gLfq.lfq_read_inverted(static_cast<void *>(data), data->cycleTime);
        INFO("data lefttime = %d \n", data->leftTime);
    }
    else
        ipc.gLfq.lfq_read(static_cast<void *>(data));	//读取一个元素
}
#if 0
void Phasecontrol::SendChannelStatus()
{
	Ipc::ChannelCheckMsg rmsg = {MAX_CHANNEL_NUM, TURN_OFF};	//接受的通道检测消息
	Ipc::ChannelStatusMsg smsg;	//发送的通道状态消息

	if (ipc.MsgRecv(rmsg, false))	//非阻塞接受通道检测消息
	{	//如果接收到通道检测消息，则使用接收的通道状态点灯
		m_data.allChannels[(rmsg.channelId - 1) % MAX_CHANNEL_NUM] = rmsg.channelStatus;
	}
	std::memcpy(smsg.allChannels, m_data.allChannels, sizeof(m_data.allChannels));
	ipc.MsgSend(smsg);
#if defined(__linux__) && defined(__arm__)
	//发送通道状态给红灯信号检测器
	if (ptl.mcastinfo.enableRedSignal)
	{
		Ipc::RedStatusMsg msg;
		std::memcpy(msg.allChannels, m_data.allChannels, sizeof(m_data.allChannels));
		ipc.MsgSend(msg);
	}
#endif
}
#endif
void Phasecontrol::RecvControlModeMsg()
{
	Ipc::ControlModeMsg rmsg;

	if (!ipc.MsgRecv(rmsg, false))	//非阻塞接收控制模式消息
		return;
	//接收到新的控制方式
	if (rmsg.mode == STEP_MODE)
	{
		if (stepCtl.IsStepInvalid(&m_data, rmsg.stageNum))
		{	//步进无效时需要把当前的控制方式反馈给策略控制模块
			if (curMode != STEP_MODE)
			{
				Ipc::ControlTypeMsg smsg = {curControlType, curMode, stageNum, mSchemeId};
				ipc.MsgSend(smsg);	//返回之前的控制方式给策略控制模块
			}
		}
		else
		{
			curControlType = rmsg.controlType;
			stageNum = rmsg.stageNum;
			mSchemeId = 0;
			if (curMode == STEP_MODE)
				stepflag = STEP_EXCUTE_FLAG;
			curMode = STEP_MODE;
            //INFO("recv step control, stageNO = %d", stageNum);
		}
	}
	else
	{
		curControlType = rmsg.controlType;
		curMode = rmsg.mode;
		stepflag = STEP_UNUSED_FLAG;
		mSchemeId = rmsg.mSchemeId;	
        //INFO("recv new control mode = %d, mSchemeId = %d\r\n", curMode, mSchemeId);
	}
}

//系统初始化，先等待线性队列中有数据后依次执行黄闪和全红，随后初始化控制模式
void Phasecontrol::SystemInit()
{
	UInt8 initSchemeId = 0;	//默认初始化为系统控制,即系统运行的第一个周期
	
	ipc.SemWaitBeginReadData();	//等待计算模块向线性队列中填充数据
	ipc.SemPostStartTimer();	//启动定时器
	do
	{
		RecvControlModeMsg();
		switch (curMode)
		{
			case MANUAL_MODE: initSchemeId = mSchemeId; break;
			case INDUCTIVE_MODE: initSchemeId = INDUCTIVE_SCHEMEID; break;
			case INDUCTIVE_COORDINATE_MODE: initSchemeId = INDUCTIVE_COORDINATE_SCHEMEID; break;
			default: initSchemeId = 0; break;
		}
		GetLocalTime();
		ReadLineQueueData(&m_data, initSchemeId);
		ipc.SemWaitTimerForPhaseCtl();
        ipc.SemPostForChan();//SendChannelStatus();
	} while (m_data.leftTime != 1); //循环结束表示启动时的黄闪和全红已经运行完毕，下面要按照控制方式开始运行了
    //INFO("yellow and all red init over\n");
}

void Phasecontrol::DataDeal()
{
	switch (curMode)
	{
		case SYSTEM_MODE: //ReadLineQueueData(&m_data, 0); break;
		case MANUAL_MODE: ReadLineQueueData(&m_data, mSchemeId); break;
		
		case YELLOWBLINK_MODE: 
		case TURNOFF_LIGHTS_MODE: 
		case ALLRED_MODE: specialCtl.Deal(&m_data); break;
		
		case STEP_MODE: stepCtl.Deal(&m_data); break;
		
		case INDUCTIVE_MODE:
		case INDUCTIVE_COORDINATE_MODE: inductiveCtl.Deal(&m_data); break;
		default: break;
	}
}
#if 0
inline void Phasecontrol::RestoreChannelStatus()
{
    int i = 0;
	UInt8 *allChannelStatus = m_data.allChannels;

    for(i = 0; i < MAX_CHANNEL_NUM; i++)
    {
        switch(allChannelStatus[i])
        {
            case OFF_GREEN: allChannelStatus[i] = GREEN; break;
            case OFF_RED: allChannelStatus[i] = RED; break;
            case OFF_YELLOW: allChannelStatus[i] = YELLOW; break;
            default: break;
        }
    }
}
#endif
void Phasecontrol::run(void *arg)
{
	inductiveCtl.StartVehCollectThread();	//启动车检信息采集线程
	SystemInit();
	while (1)	//经历一次循环的时间为1s
	{	
		RecvControlModeMsg();
		GetLocalTime();
		DataDeal();	//数据处理
		ptl.ItsCustom(&m_data);	//定制使用
        //channelLock.Deal(&m_data);//通道锁定处理
		ipc.SemWaitTimerForPhaseCtl();
		
		m_data.isStep = (curMode == STEP_MODE) ? true : false;

        ptl.ItsSetCurRunData(&m_data);
        ipc.SemPostForChan();
	}
}

Phasecontrol::Phasecontrol(Protocol & p, Ipc & i) : ptl(p), ipc(i), specialCtl(*this), inductiveCtl(*this), stepCtl(*this)
{
	curControlType = AUTO_CONTROL;
	curMode = SYSTEM_MODE;
	stepflag = STEP_UNUSED_FLAG;
	stageNum = 0;
	mSchemeId = 0;
	gCurTime = 0;
#if defined(__linux__) && defined(__arm__)
	fd = -1;
	ledstatus = 0;
#endif 
	std::memset(&m_data, 0, sizeof(LineQueueData));
	start();
}
