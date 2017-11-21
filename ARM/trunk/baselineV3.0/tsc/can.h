#pragma once

#include "thread.h"
#include "hik.h"
#include "lamp.h"
//#include "faultcheck.h"

namespace hik
{
	class device;
}

class Can : public hik::thread
{
private:
	hik::device &dev;
	int canfd;
	//array<FaultCheck, MAX_CHANNEL_NUM> faultcheck;
	void Init();
	
public:
	Can();
	~Can();
	void SetCtrlKeyLedByCan(bitset<8> &ctrlKey);
	void SendCanMsgToBoard(UInt32 can_id, void *data, int size = 8);	//所有通过can发送的LCB配置长度都为8字节
	void Send(LightBits &bits);
	void Run();
protected:
};

