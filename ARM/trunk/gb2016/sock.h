#ifndef __SOCK_H_
#define __SOCK_H_

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <unistd.h>
#include <errno.h>
#else	// __WIN32__
#include <WinSock2.h>
#endif

#include "hik.h"
#include <cstring>

class Sock
{
private:
	struct sockaddr_in upMachineAddr;	//上位机地址信息

	bool CreateSocket(int type, UInt16 port)
	{
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr);
		int opt = 1;
		
		if (sockfd != -1)
		{
			INFO("You have already create socket for this object, sockfd = %d", sockfd);
			return true;
		}
		addr.sin_family = PF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);
		memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
		if ((sockfd = socket(PF_INET, type, 0)) == -1)
		{
			ERR("create socket fail!");
			return false;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(int));
		if (bind(sockfd, (struct sockaddr *)(&addr), len) == -1)
		{
			ERR("bind socket fail!");
			CloseSocket();
			return false;
		}
		return true;
	}

public:
	int sockfd;

    Sock()
	{
		sockfd = -1;
		memset(&upMachineAddr, 0, sizeof(upMachineAddr));
#ifdef __WIN32__
		WSAData ws;
		if (WSAStartup(MAKEWORD(2,2), &ws))
		{
			ERR("WSAStartup failed!");
		}
#endif
	}
	
    ~Sock()
	{
		CloseSocket();
#ifdef __WIN32__
		WSACleanup();
#endif
	}
	
	void SetUpMachineAddr(const char *ip, UInt16 port)
	{
		upMachineAddr.sin_family = PF_INET;
		upMachineAddr.sin_addr.s_addr = inet_addr(ip);
		upMachineAddr.sin_port = htons(port);
		memset(upMachineAddr.sin_zero, 0, sizeof(upMachineAddr.sin_zero));
	}

	bool CreateUdpSocket(UInt16 port)
	{
		return CreateSocket(SOCK_DGRAM, port);
	}

	bool CreateTcpSocket(UInt16 port)
	{
		return CreateSocket(SOCK_STREAM, port);
	}
	
	void CloseSocket()
	{
		if (sockfd == -1)
			return;
#ifdef __linux__
		close(sockfd);
#else	//__WIN32__
		closesocket(sockfd);
#endif
		sockfd = -1;
	}
	
    int Listen(int num)
	{
		return listen(sockfd, num);
	}
	
    int Accept(struct sockaddr *addr, socklen_t *addrlen)
	{
		return accept(sockfd, addr, addrlen);
	}
	
	int SetNonblock()
	{
		return fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
	}

    int SetSockOpt(int level, int optname, const char* optval, int optlen)
	{
	   return setsockopt(sockfd, level, optname, optval, optlen);
	}

	int GetSockopt(int level, int optname, char* optval, int *optlen)
	{
		return getsockopt(sockfd, level, optname, optval, (socklen_t *)optlen);
	}
	
	void SetRecvTimeOut(int sec)
	{
#ifdef __linux__
		struct timeval timeout = {sec, 0};
#else	//__WIN32__
		int timeout = sec * 1000;
#endif
		//设置接收超时
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}
	
	void SetSendTimeOut(int sec)
	{
#ifdef __linux__
		struct timeval timeout = {sec, 0};
#else	//__WIN32__
		int timeout = sec * 1000;
#endif
		//设置发送超时
		setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
	}
	
    ssize_t Recvfrom(void *buf, size_t len, int flags)
	{
		struct sockaddr_in src_addr;
		socklen_t addrlen = 0;
		memset(&src_addr, 0, sizeof(src_addr));
		ssize_t ret = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&src_addr, &addrlen);
#if 0
		if (src_addr.sin_addr.s_addr == upMachineAddr.sin_addr.s_addr && src_addr.sin_port == upMachineAddr.sin_port)
			return ret;
		else
			return -1;
#else
		//INFO("recv from ip[%s] port %d, %d bytes", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port), ret);
		return ret;
#endif
	}
	
    ssize_t Recvfrom(void *buf, size_t len, int flags, struct sockaddr_in *src_addr, socklen_t *addrlen)
	{
		return recvfrom(sockfd, buf, len, flags, (struct sockaddr *)src_addr, addrlen);
	}
	
	ssize_t Recv(void *buf, size_t len, int flags)
	{
		return recv(sockfd, buf, len, flags);
	}
	
	ssize_t Sendto(const void *buf, size_t len, int flags)
	{
		return sendto(sockfd, buf, len, flags, (struct sockaddr *)&upMachineAddr, sizeof(struct sockaddr));
	}
	
    ssize_t Sendto(const void *buf, size_t len, int flags, struct sockaddr_in *dst_addr, socklen_t addrlen = sizeof(struct sockaddr))
	{
		return sendto(sockfd, buf, len, flags, (struct sockaddr *)&dst_addr, addrlen);
	}
	
	ssize_t Send(const void *buf, size_t len, int flags)
	{
		return send(sockfd, buf, len, flags);
	}
protected:
};

#endif
