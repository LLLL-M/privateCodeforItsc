#include <cstring>
#include "ipc.h"
#include "protocol.h"
#include "channelhandle.h"

using namespace HikChannelHandle;


void ChannelHandle::SendChannelStatus(LineQueueData *data)
{
    Ipc::ChannelCheckMsg rmsg = {MAX_CHANNEL_NUM, TURN_OFF};	//接受的通道检测消息
    Ipc::ChannelStatusMsg smsg;	//发送的通道状态消息

    if (ipc.MsgRecv(rmsg, false))	//非阻塞接受通道检测消息
    {	//如果接收到通道检测消息，则使用接收的通道状态点灯
        data->allChannels[(rmsg.channelId - 1) % MAX_CHANNEL_NUM] = rmsg.channelStatus;
    }
    memcpy(smsg.allChannels, data->allChannels, sizeof(data->allChannels));
    ipc.MsgSend(smsg);
}

#if defined(__linux__) && defined(__arm__)
inline void ChannelHandle::SendChanStatus2RedCheck(unsigned char *chan)
{
    //发送通道状态给红灯信号检测器
    if (ptl.mcastinfo.enableRedSignal)
    {
        Ipc::RedStatusMsg msg;
        std::memcpy(msg.allChannels, chan, MAX_CHANNEL_NUM);
        ipc.MsgSend(msg);
    }
}


inline void ChannelHandle::RestoreChannelStatus(LineQueueData *data)
{
    int i = 0;
    UInt8 *allChannelStatus = data->allChannels;

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

void ChannelHandle::run(void *arg)
{
    LineQueueData sysdata;
    unsigned char lockstatus[32]={INVALID};
    unsigned char lockFlag = 0;
    bootTransitonTime = ptl.GetBootTransitionTime();
    if (bootTransitonTime <= 0)
        bootTransitonTime = 6;//default value
    while(1)
    {
        if(bootTransitonTime == 0)
        {
            memset(lockstatus, INVALID, sizeof(lockstatus));
            lockFlag = ptl.ChannelControl(lockstatus);
        }
        if(ipc.SemWaitForChan())//1s
        {
            if(bootTransitonTime>0)
                bootTransitonTime--;
            memset(&sysdata, 0, sizeof(LineQueueData));
            ptl.ItsGetCurRunData(&sysdata);

            ptl.channelLockTransition(lockFlag, sysdata.allChannels, lockstatus);

            ptl.SetRealTimeInfo(lockFlag, &sysdata);
#if defined(__linux__) && defined(__arm__)
            if (sysdata.isStep == FALSE)
                ptl.ItsCountDownOutput(&sysdata);	//输出倒计时牌
            else
                RestoreChannelStatus(&sysdata);//脉冲倒计时器会修改通道状态，在步进时需要把3个特殊的状态恢复到正常。
#endif
            //INFO("ChanControl: Send channel status...0:%d, 1:%d, time: %ld", sysdata.allChannels[0], sysdata.allChannels[1], time(NULL));
            SendChannelStatus(&sysdata);
#if defined(__linux__) && defined(__arm__)
            SendChanStatus2RedCheck(sysdata.allChannels);
#endif
        }
        msleep(50);//50ms
    }
}

ChannelHandle::ChannelHandle(Protocol& p, Ipc& i):ptl(p), ipc(i)
{
    bootTransitonTime = 6; //default value

    start();
}

ChannelHandle::~ChannelHandle()
{

}

