#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hikmsg.h"

#define RED_SIGNAL_POWERON_CMD		0x18
#define RED_SIGNAL_JUMP_CMD			0x22
#define RED_SIGNAL_HEARTBEAT_CMD	0x03	

#define MCAST_ADDR	"239.255.255.44"
#define MCAST_PORT	8168

extern int msgid;

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

static inline void GetRedSignalData(UInt8 *allChannels, UInt8 *data)
{
	int i, n = 0;
	
	memset(data, 0, 4);
	for (i = 0; i < NUM_CHANNEL; i++)
	{
		if (n % 8 == 0)
			n = 0;
		data[3 - i / 8] |= ((allChannels[i] == RED || allChannels[i] == ALLRED) ? 0 : 1) << n++;//���Ŵ���data������
	}
}

static inline void CalCheckSumAndSend(int fd, struct sockaddr *addr, RedSignalTranData *tranData)
{
	int i, sum = 0;
	UInt8 *bytes = (UInt8 *)tranData;
	
	if (fd == -1)
		return;
	for (i = 0; i < tranData->length - 1; i++)
		sum += bytes[i];
	tranData->checksum = sum & 0xff;
	sendto(fd, (void *)tranData, tranData->length, 0, addr, sizeof(*addr));
}

void *RedSignalCheckModule(void *arg)
{
	int multicastFd = socket(AF_INET, SOCK_DGRAM, 0);
	struct msgbuf msg;
	RedSignalTranData tranData = {
		.start = 0xf0,
		.address1 = 0x03,
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
		//�ӵ�������л�ȡ����ź�ֵ
		GetRedSignalData(msg.msgAllChannels, tranData.data);
		if (flag == 0)
		{	//ϵͳ���ϵ磬�����ϵ��ź�
			flag = 1;
			tranData.command = RED_SIGNAL_POWERON_CMD;
			CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
		}
		else
		{
			if (memcmp(bakData, tranData.data, sizeof(bakData)) != 0)
			{	//�к�����䣬��������3�������źţ�ÿ�μ��100ms
				tranData.command = RED_SIGNAL_JUMP_CMD;
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				usleep(100000);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				usleep(100000);
				CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
				usleep(100000);
			}			
			//ÿ1s����һ�������źţ���Ϊÿһ��msg���1s
			tranData.command = RED_SIGNAL_HEARTBEAT_CMD;
			CalCheckSumAndSend(multicastFd, (struct sockaddr *)&mcast_addr, &tranData);
		}
		memcpy(bakData, tranData.data, sizeof(bakData));	
	}
	pthread_exit(NULL);
}
