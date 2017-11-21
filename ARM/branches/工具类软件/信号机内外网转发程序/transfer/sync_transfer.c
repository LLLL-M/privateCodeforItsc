#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include "hik.h"
#include "parse_ini.h"

#define BUFF_LEN	2048
#define EXTERN_PORT		54321

typedef struct connect_header
{
	UInt32 type:8;//1:id, 2:161 port, 3:20000 port
	UInt32 id:8;
	UInt32 reqid:16;
} ConnectHeader;

UInt16 gDeviceId = 0;
int srvfd;
int localfd[2] = {-1, -1};	//[0]:161, [1]:20000
pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

void ParseWannetId(struct sockaddr_in *srvaddr)
{
	char ip[40] = {0};
	if (parse_start("/home/wannet_id.ini") == False)
	{
		ERR("parse /home/wannet_id.ini fail");
		exit(1);
	}
	if (get_key_string("wannet", "ip", ip))
		srvaddr->sin_addr.s_addr = inet_addr(ip);
	gDeviceId = get_one_value("device", "id");
	parse_end();
	if (srvaddr->sin_addr.s_addr == INADDR_ANY || gDeviceId == 0)
	{
		ERR("get wannet_id.ini config information fail");
		exit(1);
	}
	INFO("use wan net ip %s", ip);
	INFO("device id is %d", gDeviceId);
}

void ConnectServer(struct sockaddr_in *srvaddr)
{
	ConnectHeader header = {1, gDeviceId};
	srvfd = socket(PF_INET, SOCK_STREAM, 0);
	if (srvfd == -1) 
	{
		ERR("create socket fail, error info: %s\n", strerror(errno));
		return;
	}
	if (connect(srvfd, (struct sockaddr *)srvaddr, sizeof(struct sockaddr)) == -1) 
	{
		ERR("connect socket fail, error info:%s\n", strerror(errno));
		close(srvfd);
		srvfd = -1;
		return;
	}
	INFO("Connect successful!");
	//连接成功发送本机id号
	send(srvfd, &header, sizeof(ConnectHeader), 0);
}

void DealPacket(int fd, struct sockaddr_in *dstaddr, char *buf, ssize_t n)
{
	int t = 0;
	sendto(fd, buf + sizeof(ConnectHeader), n - sizeof(ConnectHeader), 0, (struct sockaddr *)dstaddr, sizeof(struct sockaddr));
	
#if 0
	n = recvfrom(fd, buf + sizeof(ConnectHeader), BUFF_LEN - sizeof(ConnectHeader), 0, NULL, NULL);
	INFO("recv local port %d bytes data", n);
	send(srvfd, buf, n + sizeof(ConnectHeader), 0);
#else
	while (t++ <= 300)
	{
		n = recvfrom(fd, buf + sizeof(ConnectHeader), BUFF_LEN - sizeof(ConnectHeader), MSG_DONTWAIT, NULL, NULL);
		if (n != -1)
		{
			INFO("recv local port %d bytes data", n);
			send(srvfd, buf, n + sizeof(ConnectHeader), 0);
			break;
		}
		usleep(10000);
	}
#endif
}

int main(void)
{
	struct sockaddr_in srvaddr = 
	{
		.sin_family = PF_INET,
		.sin_addr = {INADDR_ANY},
		.sin_port = htons(EXTERN_PORT),
		.sin_zero = {0},
	};
	struct sockaddr_in dstaddr[2] = 
	{
		{
			.sin_family = PF_INET,
			.sin_addr = inet_addr("127.0.0.1"),
			.sin_port = htons(161),
			.sin_zero = {0},
		},
		{
			.sin_family = PF_INET,
			.sin_addr = inet_addr("127.0.0.1"),
			.sin_port = htons(20000),
			.sin_zero = {0},
		}
	};
	socklen_t len = sizeof(struct sockaddr);
	char buf[BUFF_LEN];
	ssize_t n;
	ConnectHeader *header = (ConnectHeader *)buf;

	ParseWannetId(&srvaddr);

	localfd[0] = socket(PF_INET, SOCK_DGRAM, 0);
	localfd[1] = socket(PF_INET, SOCK_DGRAM, 0);
	if (localfd[0] == -1 || localfd[1] == -1) 
	{
		ERR("create socket fail, error info: %s\n", strerror(errno));
		return -1;
	}

	ConnectServer(&srvaddr);
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		n = recv(srvfd, buf, sizeof(buf), 0);
		if (n == -1 || n == 0)
		{
			INFO("the link is invalid, begin to connect again!");
			close(srvfd);
			sleep(3);
			ConnectServer(&srvaddr);
			continue;
		}
		if (header->id != gDeviceId)
			continue;

		INFO("recv server %d bytes data, type = %d", n, header->type);
		if (header->type == 2 && localfd[0] != -1)
		{
			DealPacket(localfd[0], &dstaddr[0], buf, n);
		}
		else if (header->type == 3 && localfd[1] != -1)
		{
			DealPacket(localfd[1], &dstaddr[1], buf, n);
		}
	}

	return 0;
}
