#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "hik.h"
#include "parse_ini.h"
#include "lfq.h"

#define SA(x) ((struct sockaddr *)&x)

#define EXTERN_PORT		54321

#define STACK_SIZE	1024

#define log_debug INFO

#define log_err ERR

typedef struct connect_header
{
	int type:8;			//1:id, 2:161 port, 3:20000 port
	int nMachineId:8;	//用来区分信号机的ID
	int nSdkId:16;		//主要用来区分是哪个SDK发送的请求信息
	int nMessageId;		//主要用来区分是哪个消息，也就是消息的ID
	int nMessageLen;	//标识接下来需要接收的字节数
	int nLinuxGetMsgTime;   //linux下获得当前消息的时间，以下时间均换算成毫秒
	int nLinuxSendMsgTime;  //linux下发送当前消息的时间，以下时间均换算成毫秒
	int nWinGetMsgTime;     //windows下获得当前消息的时间，以下时间均换算成毫秒
	int nWinSendMsgTime;    //windows下获得当前消息的时间，以下时间均换算成毫秒
} ConnectHeader;

UInt16 gDeviceId = 0;
int srvfd;
pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct connect_params
{
	int localfd;
	void *handle;
	struct sockaddr_in dstaddr;
	UInt8 type;
} ConnectParams;
ConnectParams gParams[2];	//[0]:161, [1]:20000

//得到系统的毫秒值
static unsigned int GetLocalTime()
{
    struct timeval t;

    gettimeofday(&t,NULL);

    return (unsigned int)(t.tv_sec*1000 + t.tv_usec/1000);
}

//给socket设置超时
void setSocketTimeOut(int *sockfd)
{
    struct timeval timeout={20,0};//20s内，如果收发数据不成功，则立即返回.

    //发送超时
    setsockopt(*sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));

    //接收超时
    setsockopt(*sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
}


void *DealLocalMsg(void *arg)
{
	ConnectParams *params = (ConnectParams *)arg;
	ssize_t size, n;
	char buf[8192], *p = NULL;
	ConnectHeader *header = (ConnectHeader *)buf;

	p = buf + sizeof(ConnectHeader);
	size = sizeof(buf) - sizeof(ConnectHeader);
//	unsigned int nStart = 0;
	while (1)
	{
		memset(p, 0, size);
		n = recv(params->localfd, p, size, 0);
		if (n == -1 || n == 0)
			continue;
		log_debug("recv local %u", GetLocalTime());
		lfq_read(params->handle, header);
		header->nMessageLen = n;
		
		pthread_mutex_lock(&gMutex);
		header->nLinuxSendMsgTime = GetLocalTime();
		if(-1 == send(srvfd, buf, n + sizeof(ConnectHeader), /*MSG_DONTWAIT*/0))
			log_debug("!!!!!!!!!!!!!!!!!! send error , %s !!!!!!\n",strerror(errno));
		pthread_mutex_unlock(&gMutex);
#if 0
		if (params->type == 2)
		{
			time_t tim = time(NULL);
			struct tm ti;
			localtime_r(&tim, &ti);
			log_debug("Local send: index: %d, time: %d:%d:%d", header->index, ti.tm_hour, ti.tm_min, ti.tm_sec);
		}
#endif
	}
	
	pthread_exit(NULL);
}

void ParseWannetId(struct sockaddr_in *srvaddr)
{
	char ip[40] = {0};
	if (parse_start("/home/wannet_id.ini") == False)
	{
		log_debug("parse /home/wannet_id.ini fail");
		exit(1);
	}
	if (get_key_string("wannet", "ip", ip))
		srvaddr->sin_addr.s_addr = inet_addr(ip);
	gDeviceId = get_one_value("device", "id");
	parse_end();
	if (srvaddr->sin_addr.s_addr == INADDR_ANY || gDeviceId == 0)
	{
		log_debug("get wannet_id.ini config log_debugrmation fail");
		exit(1);
	}
	log_debug("use wan net ip %s", ip);
	log_debug("device id is %d", gDeviceId);
}

int ConnectServer(struct sockaddr_in *srvaddr)
{
	ConnectHeader header = {1, gDeviceId, 0, 0, 0};
	int flags, ret = 0;
	fd_set fdw;
	struct timeval timeout = {10, 0};
	socklen_t len;

	srvfd = socket(PF_INET, SOCK_STREAM, 0);
	if (srvfd == -1) 
	{
		log_debug("create socket fail,   log_debug: %s\n", strerror(errno));
		return -1;
	}
	flags = fcntl(srvfd, F_GETFL, 0);
	fcntl(srvfd, F_SETFL, flags | O_NONBLOCK);
	if (connect(srvfd, (struct sockaddr *)srvaddr, sizeof(struct sockaddr)) == -1) 
	{
		if(errno != EINPROGRESS) 
		{ 
			log_debug("connect socket fail,   log_debug:%s\n", strerror(errno));
			close(srvfd);
			srvfd = -1;
			return -2;
		}
	}
	
	FD_ZERO(&fdw);
	FD_SET(srvfd, &fdw);
	if (select(srvfd + 1, NULL, &fdw, NULL, &timeout) <= 0)
	{
		log_debug("select  ,   log_debug:%s", strerror(errno));
		close(srvfd);
		srvfd = -1;
		return -3;
	}
#if 0
	getsockopt(srvfd, SOL_SOCKET, SO_ERROR, &ret, &len);
	if(ret != 0)
	{
		log_err("connect error %d, error info:%s", ret, strerror(errno));
		close(srvfd);
		srvfd = -1;
		return -3;
	}
#endif
	fcntl(srvfd, F_SETFL, flags);
	setSocketTimeOut(&srvfd);
	//连接成功发送本机id号
	ret = send(srvfd, &header, sizeof(ConnectHeader), 0);
	if (ret > 0)
		log_debug("Connect successful!");
	return ret;
}

void InitConnectParams(ConnectParams *params, UInt16 port)
{
	pthread_t threadid;
	
	memset(params, 0, sizeof(ConnectParams));
	params->localfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (params->localfd == -1) 
	{
		log_debug("create socket fail,   log_debug: %s\n", strerror(errno));
		exit(1);
	}
	params->handle = calloc(1, STACK_SIZE);
	if (params->handle == NULL)
	{
		log_debug("Can't alloc %d stack memory!", STACK_SIZE);
		exit(1);
	}
	lfq_init(params->handle, STACK_SIZE, sizeof(ConnectHeader));
	
	params->dstaddr.sin_family = PF_INET;
	params->dstaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	params->dstaddr.sin_port = htons(port);
	params->type = (port == 161) ? 2 : 3;
	
	if (pthread_create(&threadid, NULL, DealLocalMsg, (void *)params) == -1) 
	{
		log_debug("create recv local msg thread(port = %d) fail,   log_debug:%s\n", port, strerror(errno));
		exit(1);
	}
	pthread_detach(threadid);
}

__attribute__((destructor)) void ProgramExit(void)
{
	if (gParams[0].localfd != 0 && gParams[0].localfd != -1)
		close(gParams[0].localfd);
	if (gParams[1].localfd != 0 && gParams[1].localfd != -1)
		close(gParams[1].localfd);
	if (gParams[0].handle != NULL)
		free(gParams[0].handle);
	if (gParams[1].handle != NULL)
		free(gParams[1].handle);
	log_debug("The program exit!");
}

static void SigHandle(int sigNum)
{
	switch (sigNum)
	{
		case SIGTERM:	log_debug("This program has been kill"); break;
		case SIGINT:	log_debug("This program has been intlog_debugupted by ctrl+C"); break;
	}
	exit(0);
}


void PrintfConnectHeader(ConnectHeader *h)
{
	printf("type: %d\n", h->type);			
	printf("nMachineId: %d\n", h->nMachineId);	
	printf("nSdkId: %d\n", h->nSdkId);	
	printf("nMessageId: %d\n", h->nMessageId);
	printf("nMessageLen: %d\n", h->nMessageLen);
	printf("nLinuxGetMsgTime: %d\n", h->nLinuxGetMsgTime);
	printf("nLinuxSendMsgTime: %d\n", h->nLinuxSendMsgTime);
	printf("nWinGetMsgTime: %d\n", h->nWinGetMsgTime);
	printf("nWinSendMsgTime: %d\n", h->nWinSendMsgTime);
		
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
	socklen_t len = sizeof(struct sockaddr);
	char buf[8192];
	ssize_t n;
	ConnectHeader *header = (ConnectHeader *)buf;
	ConnectParams *params;
	//InitLogSystem(NULL,100,1);
	//解析配置的外网ip和设备id
	ParseWannetId(&srvaddr);

	//注册信号用于回收资源
	signal(SIGTERM, SigHandle);
	signal(SIGINT, SigHandle);
	
	//初始化本地端口连接以及相关参数
	InitConnectParams(&gParams[0], 161);
	InitConnectParams(&gParams[1], 20000);

    unsigned int nStart = 0;
    while(1)
    {
		//连接外网服务器
		if (ConnectServer(&srvaddr) <= 0) //如果失败一直重连
		{
		    close(srvfd);
			sleep(1);
			log_debug("the link is invalid, begin to connect again!!! err info:%s", strerror(errno));
			continue;
		}

        while (1)
    	{
    		memset(buf, 0, sizeof(buf));
    		n = recv(srvfd, buf, sizeof(ConnectHeader), 0);
    		if (n <= 0)
    		{
    		    close(srvfd);
    			sleep(1);
    			log_debug("recv error, begin to connect again!!!");
    			break;
    		}
    		if (header->nMachineId != gDeviceId || (header->type != 2 && header->type != 3))
    			continue;

			//PrintfConnectHeader(header);
    		n = recv(srvfd, buf + sizeof(ConnectHeader), header->nMessageLen, 0);
    		if (n <= 0)
    		{
    		    close(srvfd);
    			sleep(1);
    			log_debug("the link is invalid, begin to connect again!!!");
    			break;
    		}
#if 0
    		if ((header->type == 2) || (header->type == 3))
    		{
    			time_t tim = time(NULL);
    			struct tm ti;
    			localtime_r(&tim, &ti);
    			log_debug("Server recv:port:%d, index: %d, len: %d, rcvsize: %ld, time: %d:%d:%d", 
    				(header->type == 2) ? 161 : 20000, header->index, header->len,
    				n, ti.tm_hour, ti.tm_min, ti.tm_sec);
    		}
#else
    		//log_debug("recv server %d bytes data, type = %d", n, header->type);
    		params = (header->type == 2) ? (&gParams[0]) : (&gParams[1]);
    		header->nLinuxGetMsgTime = GetLocalTime();
    		//log_debug(" win  %lu , linux %lu\n",header->nWinGetMsgTime,header->nLinuxGetMsgTime);
           nStart = GetLocalTime();
            lfq_write(params->handle, header);
    		sendto(params->localfd, buf + sizeof(ConnectHeader), n, 0, SA(params->dstaddr), len);
    		log_debug("1111 send cost %u\n",GetLocalTime() - nStart);
#endif
    	}
    }
	

	return 0;
}
