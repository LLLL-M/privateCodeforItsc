#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include "hik.h"
#include "platform.h"
#include "parse_ini.h"

extern int msgid;

#define RED_SIGNAL_POWERON_CMD		0x18
#define RED_SIGNAL_JUMP_CMD			0x22
#define RED_SIGNAL_HEARTBEAT_CMD	0x03	


typedef struct redSignalData
{
	UInt8 start;
	UInt8 address1;
	UInt8 address2;
	UInt8 length;
	UInt8 command;
	UInt8 data[4];
	UInt8 checksum;
} __attribute__((aligned (1))) RedSignalTranData;

#define MCAST_ADDR	"239.255.255.44"
#define MCAST_PORT	8168

static inline void GetRedSignalData(UInt16 *lightArray, UInt8 *data)
{
	int i, n = 0;
	lamp_t *lamp;
	
	memset(data, 0, 4);
	for (i = 0; i < 8; i++)
	{
		if (n % 8 == 0)
			n = 0;
		lamp = (lamp_t *)(&lightArray[i]);
		//取反红灯值倒着存入data数组中
		data[3 - i / 2] |= (((~lamp->L0 >> 1) & 0x1) << n++);	
		data[3 - i / 2] |= (((~lamp->L1 >> 1) & 0x1) << n++);
		data[3 - i / 2] |= (((~lamp->L2 >> 1) & 0x1) << n++);
		data[3 - i / 2] |= (((~lamp->L3 >> 1) & 0x1) << n++);
	}
}

static inline void CalCheckSumAndSend(int fd, struct sockaddr *addr, RedSignalTranData *tranData)
{
	int i, sum = 0;
	UInt8 *bytes = (UInt8 *)tranData;
	char buf[128] = {0};
	
	for (i = 0; i < tranData->length - 1; i++)
		sum += bytes[i];
	tranData->checksum = sum & 0xff;
	buf[0] = 0x7e;
	memcpy(buf + 1, tranData, sizeof(RedSignalTranData));
	buf[1 + sizeof(RedSignalTranData)] = 0x7d;
	buf[2 + sizeof(RedSignalTranData)] = 0x7f;
	sendto(fd, buf, tranData->length + 3, 0, addr, sizeof(struct sockaddr));
}

static void *RedSignalCheckProcess(void *arg)
{
	int multicastFd = (intptr_t)arg;
	struct msgbuf msg;
	RedSignalTranData tranData = {
		.start = 0xf0,
		.address1 = (UInt8)read_profile_int("device", "id", 1, "/home/wannet_id"),
		.address2 = 0x01,
		.length = 0x0a,
		.command = 0,
		.data = {0},
		.checksum = 0,
	};
	UInt64 n = 0;
	UInt8 bakData[4] = {0};
	struct sockaddr_in mcast_addr = {
	.sin_family = AF_INET,
	.sin_addr.s_addr = inet_addr(MCAST_ADDR),
	.sin_port = htons(MCAST_PORT),
	.sin_zero = {0},
	};
	
	while (1)
	{
		if (-1 == msgrcv(msgid, &msg, MSGSIZE, MSG_RED_SIGNAL_CHECK, 0))
		{
			usleep(50000);
			continue;
		}
		//从点灯数组中获取红灯信号值
		GetRedSignalData(msg.lightArray, tranData.data);
		if (++n == 1)
		{	//系统刚上电，发送上电信号
			tranData.command = RED_SIGNAL_POWERON_CMD;
			CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
		}
		else
		{
			if (memcmp(bakData, tranData.data, sizeof(bakData)) != 0)
			{	//有红灯跳变，连续发送3次跳变信号，每次间隔100ms
				tranData.command = RED_SIGNAL_JUMP_CMD;
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				usleep(100000);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				usleep(100000);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
			}
			
			if (n % 4 == 0)
			{	//每收到4个msg发送一个心跳信号，即每1s发送一次，因为每一个msg间隔250ms，
				tranData.command = RED_SIGNAL_HEARTBEAT_CMD;
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
			}
		}
		memcpy(bakData, tranData.data, sizeof(bakData));	
	}
	pthread_exit(NULL);
}

int RedSignalCheckInit(void)
{
	pthread_t threadId;
	int multicastFd = -1;
	//初始化多播套接字
	multicastFd = socket(AF_INET, SOCK_DGRAM, 0); /*建立套接字*/
	if (multicastFd == -1)
	{
		ERR("multicast init fail!");
		return 0;
	}
	//初始化红灯信号检测线程
	if (pthread_create(&threadId, NULL, RedSignalCheckProcess, (void *)multicastFd) != 0)
	{
		ERR("red signal check thread init fail, error info: %s\n", strerror(errno));
		close(multicastFd);
		return 0;
	}
	pthread_detach(threadId);
	INFO("red signal check init successful!");
	return 1;
}
