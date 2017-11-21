#ifndef __CAN_H__
#define __CAN_H__

#include "thread.h"
#include "hik.h"
#include "common.h"
#include "faultcheck.h"

class Log;

class Can : public HikThread::Thread
{
private:
	int canfd;
	void Init();
	array<FaultCheck, MAX_CHANNEL_NUM> faultcheck;
public:
	Can();
	~Can();
	void SetCtrlKeyLedByCan(bitset<8> &ctrlKey);
	void SendCanMsgToBoard(UInt32 can_id, void *data, int size = 8);	//所有通过can发送的LCB配置长度都为8字节
	void Send(LightBits &bits);
	void run(void *arg);
protected:
};

#endif
