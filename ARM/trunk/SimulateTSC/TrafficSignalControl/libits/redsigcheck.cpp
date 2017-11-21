#if defined(__linux__) && defined(__arm__)
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "ipc.h"
#include "protocol.h"
#include "redsigcheck.h"

using namespace HikRedsigcheck;

inline void Redsigcheck::GetRedSignalData(UInt8 *allChannels, UInt8 *data)
{
	int i, n = 0;
	
	std::memset(data, 0, 4);
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (n % 8 == 0)
			n = 0;
		data[3 - i / 8] |= ((allChannels[i] == RED || allChannels[i] == ALLRED) ? 0 : 1) << n++;//倒着存入data数组中
	}
}

void Redsigcheck::CalCheckSumAndSend(int fd, struct sockaddr *addr, RedSignalTranData *tranData)
{
	int i, sum = 0;
	UInt8 *bytes = (UInt8 *)tranData;
	char buf[128] = {0};
	if (fd == -1)
		return;
	for (i = 0; i < tranData->length - 1; i++)
		sum += bytes[i];
	tranData->checksum = sum & 0xff;
	//根据协议，在包头增加0x7e，在包尾增加0x7d，0x7f，added by liujie 20160524
	buf[0] = 0x7e;
	std::memcpy(buf + 1, tranData, sizeof(RedSignalTranData));
	buf[1 + sizeof(RedSignalTranData)] = 0x7d;
	buf[2 + sizeof(RedSignalTranData)] = 0x7f;
	sendto(fd, buf, tranData->length + 3, 0, addr, sizeof(*addr));
}

void Redsigcheck::run(void *arg)
{
	int multicastFd = socket(AF_INET, SOCK_DGRAM, 0);
	Ipc::RedStatusMsg msg;
	McastInfo mcastinfo = ptl.mcastinfo;
#if 0
	{
		.enableRedSignal = false,
		.mcastIp = "239.255.255.44",
		.mcastPort = 8168,
		.deviceId = 0x03,
	};
#endif
	RedSignalTranData tranData = {
		.start = 0xf0,
		.address1 = (UInt8)mcastinfo.deviceId,
		.address2 = 0x01,
		.length = 0x0a,
		.command = 0,
		.data = {0},
		.checksum = 0,
	};
	int flag = 0;
	UInt8 bakData[4] = {0};
	struct sockaddr_in mcast_addr = {
	.sin_family = AF_INET,
	.sin_addr.s_addr = inet_addr(mcastinfo.mcastIp),
	.sin_port = htons(mcastinfo.mcastPort),
	.sin_zero = {0},
	};
	
	while (1)
	{
		if (!ipc.MsgRecv(msg) || mcastinfo.enableRedSignal == false)
		{
			msleep(50);
			continue;
		}
		//从点灯数组中获取红灯信号值
		GetRedSignalData(msg.allChannels, tranData.data);
		if (flag == 0)
		{	//系统刚上电，发送上电信号
			flag = 1;
			tranData.command = RED_SIGNAL_POWERON_CMD;
			CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
		}
		else
		{
			if (std::memcmp(bakData, tranData.data, sizeof(bakData)) != 0)
			{	//有红灯跳变，连续发送3次跳变信号，每次间隔100ms
				tranData.command = RED_SIGNAL_JUMP_CMD;
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				msleep(100);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				msleep(100);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				msleep(100);
			}	
			//每1s发送一个心跳信号，因为每一个msg间隔1s
			//根据协议，取消1s发送一次心跳信号，改为1秒1次点灯信号代替，changed by liujie 20160524
			tranData.command = RED_SIGNAL_JUMP_CMD;
			CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
		}
		std::memcpy(bakData, tranData.data, sizeof(bakData));	
	}
}
#endif
