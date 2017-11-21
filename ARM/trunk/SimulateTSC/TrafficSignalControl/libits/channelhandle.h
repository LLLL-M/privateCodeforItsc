#ifndef CHANNELHANDLE_H
#define CHANNELHANDLE_H

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

namespace HikChannelHandle
{


using HikThread::Thread;
using HikIts::Protocol;
using HikIpc::Ipc;

class ChannelHandle: public Thread
{
public:

    Protocol& ptl;
    Ipc& ipc;
    unsigned int bootTransitonTime;

    ChannelHandle(Protocol& p, Ipc& i);
    ~ChannelHandle();

#if defined(__linux__) && defined(__arm__)
    void SendChanStatus2RedCheck(unsigned char *chan);
#endif
    void SendChannelStatus(LineQueueData *data);
    void run(void *arg);

protected:

private:

};
}
#endif // CHANNELHANDLE_H
