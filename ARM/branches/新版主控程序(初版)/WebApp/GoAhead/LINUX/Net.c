#include "Util.h"
#include "Net.h"
#include "hikConfig.h"

 IPINFO IpInfo_eth0; //
 IPINFO IpInfo_eth1;//
extern SignalControllerPara *gSignalControlpara;//全局信号机参数  added by xwh 2014-8-13


/*把所有本地可以使用的IP都读出来，并复制到相应的端口去*/
void GetLocalHost()
{
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	int sockfd;

	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		return;
	}

	ioctl(sockfd,SIOCGIFCONF,&ifconf);

	ifreq = (struct ifreq*)buf;

	int i = 0;
	i = ifconf.ifc_len / sizeof(struct ifreq);
	for(;i>0;i--)
	{
		if(ifreq->ifr_name[0] != 'l')
		{
			if((strcmp(ifreq->ifr_name,"eth0") == 0) && (strlen(IpInfo_eth0.szHostName) == 0))//eth0//main input
			{
				//log_debug("%s:%s\n",ifreq->ifr_name,inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
				strcpy(IpInfo_eth0.szHostName,ifreq->ifr_name);
				strcpy(IpInfo_eth0.cIp,inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
				memcpy(&IpInfo_eth0.uIP,&(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr),sizeof(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
			}
			if((strcmp(ifreq->ifr_name,"eth1") == 0) && (strlen(IpInfo_eth1.szHostName) == 0))//eth1 //output
			{
				//log_debug("%s:%s\n",ifreq->ifr_name,inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
				strcpy(IpInfo_eth1.szHostName,ifreq->ifr_name);
				strcpy(IpInfo_eth1.cIp,inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
				memcpy(&IpInfo_eth1.uIP,&(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr),sizeof(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));

			}
		}
		ifreq++;
	}
}



int SetPortReuse(int *sock, bool bReuse)	// 在bind之前调用

{
	int value = bReuse ? 1 : 0;
	if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//给socket设置超时
void setSocketTimeOut(int *sockfd)
{
    struct timeval timeout={5,0};//5s内，如果收发数据不成功，则立即返回.

    //发送超时
    setsockopt(*sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));

    //接收超时
    setsockopt(*sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
}



//SendOrRecv : 0 send , just bind local address 
//			1 recv , bind local address and join the group
//TcpOrUdp: 	1  tcp , this socket supports tcp
//			0 udp , this socket supports udp	
int CreateSocket(int* sock,unsigned short ethNo,char* ip,int port,int SendOrRecv,int TcpOrUdp)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	if(SendOrRecv == 0)
	{
		addr.sin_port = htons(0);
	}
	else
	{
		addr.sin_port = htons(port);
	}

	if(TcpOrUdp == 0)//udp
	{
		*sock = socket(AF_INET, SOCK_DGRAM, 0);//SOCK_DGRAM   ======UDP
	}
	else
	{
		*sock = socket(AF_INET, SOCK_STREAM, 0);//SOCK_STREAM  ======TCP
		unsigned long ul = 1;
		ioctl( *sock, FIONBIO, &ul);
	}
											
	if (*sock == -1)
	{
		log_debug("Error to create listen socket %s\n",strerror(errno));
		return 0;
	}

	SetPortReuse(sock, 1);

	//addr.sin_addr.s_addr= htonl(INADDR_ANY);
	struct in_addr in;

	switch(ethNo)
	{
		case 0: memcpy(&addr.sin_addr.s_addr,&IpInfo_eth0.uIP,sizeof(in));
				//addr.sin_addr.s_addr= htonl(IpInfo_eth0.uIP);
				break;
		case 1: memcpy(&addr.sin_addr.s_addr,&IpInfo_eth1.uIP,sizeof(in));
				break;
				break;				
		default: memcpy(&addr.sin_addr.s_addr,&IpInfo_eth0.uIP,sizeof(in));
				break;
	}

	//join the group  recv
	if(SendOrRecv == 1)//接收socket，需要绑定接收地址
	{
		long lIP = inet_addr(ip);
		if((lIP & htonl(0xF0000000)) == htonl(0xE0000000))//for board
		{
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(ip);
			addr.sin_addr.s_addr= inet_addr(ip);
			
			if(bind(*sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
				log_debug("error in bind the listen socket %s\n",strerror(errno));
				return 0;
			}	

			//mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			switch(ethNo)
			{
				case 0: memcpy(&mreq.imr_interface.s_addr,&IpInfo_eth0.uIP,sizeof(in));
						break;
				case 1: memcpy(&mreq.imr_interface.s_addr,&IpInfo_eth1.uIP,sizeof(in));
						break;
				default: memcpy(&mreq.imr_interface.s_addr,&IpInfo_eth0.uIP,sizeof(in));
			}		
			
	
			if(setsockopt(*sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(struct ip_mreq)) < 0)
			{
				if(strcmp("Address already in use",strerror(errno)) == 0)
				{
					log_debug("%s joined --\n",ip);
				}
				else		
				{
					log_debug("Failed to join group - %s\n",strerror(errno)); 
				}	
			}			
		}
		else
		{
			if(bind(*sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
				log_debug("error in bind the listen socket %s\n",strerror(errno));
			}
		}
		
	}
	else if(SendOrRecv == 0)//只是用来发送
	{
		if(bind(*sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		{
			log_debug("error in bind the listen socket %s\n",strerror(errno));
			return 0;
		}

		if(TcpOrUdp == 0)//udp
		{
			int i_opt = 1;
		    setsockopt(*sock, SOL_SOCKET, SO_BROADCAST, (void*) &i_opt,sizeof( i_opt));
		}

	}
	int nSendBuf = 50 * 1024 * 1024;
	setsockopt(*sock, SOL_SOCKET, SO_RCVBUF,(const char*)&nSendBuf,sizeof(int));

    return 1;
}


int SendDataNonBlock(int sock,void *data,int len,char *des,int port)
{
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);

	int ret = -1;
	ret = select(sock+1, NULL,&readfds, NULL, &tv);	

	struct sockaddr_in addrTo;
	addrTo.sin_family = AF_INET;
	addrTo.sin_port = htons(port);
	addrTo.sin_addr.s_addr =  inet_addr(des);

	if(ret == 1)
	{
		int size = 0;
		while((ret = sendto(sock,data+size,len-size,0,(struct sockaddr*) &addrTo, sizeof(addrTo))) != (len-size))
		{
			size += ret;
			//log_debug("@@@@@@@@@@@&&&&&&&&&&&&&&&&&&&&&\n");
			continue;
		}
	}
	//log_debug("%s ============   %s\n",__func__,strerror(errno));
	return ret;
}


//系统启动及页面刷新时，均需要通过socket来向hikTSC请求数据信息
int GetSignalControlParams()
{
    int socketfd = 0;
    NETDATA head;
    struct sockaddr_in fromAddr;
    struct sockaddr_in toAddr;
    socklen_t fromLen = sizeof(fromAddr);
    int ret = 0;

    char cNetBuf[25000];
    NETDATA *p;
    p = (NETDATA *)cNetBuf;
    
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(socketfd == -1)
    {
        return 0;
    }
    SetPortReuse(&socketfd,1);
    setSocketTimeOut(&socketfd);
    
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(20000);
	toAddr.sin_addr.s_addr =  htonl(INADDR_ANY);
    
    //上载开始，发送请求数据头
    head.unExtraParamHead = 0x6e6e;
    head.unExtraParamID = 0xc0;
    head.unExtraParamValue = 206;
    sendto(socketfd,&head,sizeof(head),0,(struct sockaddr *)&toAddr,sizeof(toAddr));

    //接收上载开始返回值
    memset(&head,0,sizeof(head));
    head.unExtraParamValue = 206;
    recvfrom(socketfd,&head,sizeof(head),0,(struct sockaddr *)&fromAddr, &fromLen);

    if(head.unExtraParamValue == 0)
    {
        head.unExtraParamHead = 0x6e6e;
        head.unExtraParamID = 0xc0;
        head.unExtraParamValue = 0xeeeeeeee;//start
        sendto(socketfd,&head,sizeof(head),0,(struct sockaddr *)&toAddr,sizeof(toAddr));

        //接收全局参数
        ret = recvfrom(socketfd,cNetBuf,sizeof(cNetBuf),0,(struct sockaddr *)&fromAddr, &fromLen);
        memcpy(gSignalControlpara,p->data,sizeof(SignalControllerPara));
//        printf("==>  %d\n",ret);

        //接收上载完成标识
        head.unExtraParamHead = 0x6e6e;
        head.unExtraParamID = 0xc0;
        head.unExtraParamValue = 207;
        sendto(socketfd,&head,sizeof(head),0,(struct sockaddr *)&toAddr,sizeof(toAddr));        
        
        memset(&head,0,sizeof(head));
        recvfrom(socketfd,&head,sizeof(head),0,(struct sockaddr *)&fromAddr, &fromLen);

        close(socketfd);
        return 1;
    }

    close(socketfd);
    return 0;
}


int SendSignalControlParams()
{
    int socketfd = 0;
    NETDATA head;
    struct sockaddr_in fromAddr;
    struct sockaddr_in toAddr;
    socklen_t fromLen = sizeof(fromAddr);    

    char cNetBuf[25000];
    NETDATA *p;
    p = (NETDATA *)cNetBuf;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd == -1)
    {
        return 0;
    }
    SetPortReuse(&socketfd,1);
    setSocketTimeOut(&socketfd);
    
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(20000);
	toAddr.sin_addr.s_addr =  htonl(INADDR_ANY);
        
    //下载开始，发送请求数据头
    head.unExtraParamHead = 0x6e6e;
    head.unExtraParamID = 0xb7;
    head.unExtraParamValue = 0xb7;//unuse
    sendto(socketfd,&head,sizeof(head),0,(struct sockaddr *)&toAddr,sizeof(toAddr));

    //接收下载开始返回值
    recvfrom(socketfd,&head,sizeof(head),0,(struct sockaddr *)&fromAddr, &fromLen);

    if(head.unExtraParamValue == 0)
    {
        //发送全局参数
        p->unExtraParamHead = 0x6e6e;
        p->unExtraParamID = 0xeeeeeeee;
        memcpy(&p->unExtraParamValue,gSignalControlpara,sizeof(SignalControllerPara));
        sendto(socketfd,p,sizeof(cNetBuf),0,(struct sockaddr *)&toAddr,sizeof(toAddr));
        
        recvfrom(socketfd,&head,sizeof(head),0,(struct sockaddr *)&fromAddr, &fromLen);
    
        //发送下载完成标识
        head.unExtraParamHead = 0x6e6e;
        head.unExtraParamID = 0xb8;
        sendto(socketfd,&head,sizeof(head),0,(struct sockaddr *)&toAddr,sizeof(toAddr));        

        //接收下载完成
        memset(&head,0,sizeof(head));
        recvfrom(socketfd,&head,sizeof(head),0,(struct sockaddr *)&fromAddr, &fromLen);

        close(socketfd);
        return 1;
    }
    
    close(socketfd);
    return 0;
}

void *ThreadHeartBeats()
{
    int socketfd = 0;
    char buf[10] = "0";
    int ret = 0;
    struct sockaddr_in addr;
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);        

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd == -1)
    {
        return 0;
    }

	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr= htonl(INADDR_ANY);

    bind(socketfd, (struct sockaddr *)&addr, sizeof(addr));

    SetPortReuse(&socketfd,1);
    setSocketTimeOut(&socketfd);
    while(1)
    {
        memset(buf,0,sizeof(buf));
        ret = recvfrom(socketfd,buf,sizeof(buf),0,(struct sockaddr *)&fromAddr, &fromLen);

        if(ret > 0)
        {
            sendto(socketfd,buf,strlen(buf),0,(struct sockaddr *)&fromAddr, fromLen);
        }
    }

    close(socketfd);
    return 0;
}



