#if defined(__linux__)
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace Network
{
	int IfUpDown(const char *ifname, int flag);
	bool SetNetwork(const char *name, const char *_ip, const char *_netmask, const char *_gateway);
	in_addr_t GetNetworkCardIp(const char *interface);
	int GetMac(const char* net_dev, char* mac);
	in_addr GetGateway(const char *net_dev, char *gateway);
	in_addr GetMask(const char *net_dev, char *mask);
}


int Network::IfUpDown(const char *ifname, int flag)
{
    int fd, rtn;
    struct ifreq ifr;        

    if (!ifname) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( fd < 0 ) {
        printf("socket called fail, error info: %s\n", strerror(errno));
        return -1;
    }
    
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1);

    if ((rtn = ioctl(fd, SIOCGIFFLAGS, &ifr)) == 0) 
	{
        if ( flag == 0 )
            ifr.ifr_flags &= ~IFF_UP;
        else if ( flag == 1 ) 
            ifr.ifr_flags |= IFF_UP;
        
		if ( (rtn = ioctl(fd, SIOCSIFFLAGS, &ifr) ) != 0)
		{
			printf("SIOCSIFFLAGS failed , error=%s", strerror(errno));
		}
    }
    close(fd);

    return rtn;
}


bool Network::SetNetwork(const char *name, const char *_ip, const char *_netmask, const char * _gateway)
{
	unsigned int ip = inet_addr(_ip);
	unsigned int netmask = inet_addr(_netmask);
	unsigned int gateway = inet_addr(_gateway);
	struct ifreq ifr;
	struct rtentry rt;
	struct sockaddr_in gateway_addr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		printf("socket called fail, error info: %s\n", strerror(errno));
		close(sockfd);
		return false;
	}
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, name);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1)
	{
		printf("get netcard %s IFFLAGS failed: %s", name, strerror(errno));
		close(sockfd);
		return false;
	}
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1)
	{
		printf("set netcard %s IFFLAGS failed: %s", name, strerror(errno));
		close(sockfd);
		return false;
	}

	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip;
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1)
	{
		printf("set %s ip %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return false;
	}
	printf("set %s ip %s successful!", name, inet_ntoa(addr->sin_addr));
	
	addr->sin_addr.s_addr = netmask;
	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1)
	{
		printf("set %s netmask %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return false;
	}
	printf("set %s netmask %s successful!", name, inet_ntoa(addr->sin_addr));
	
	addr->sin_addr.s_addr = gateway;	//ÕâÒ»¾äÖ»ÊÇÓÃÀ´´òÓ¡£¬Ã»Ê²Ã´¾ßÌåÓÃÍ¾
	memset(&rt, 0, sizeof(rt));
	memset(&gateway_addr, 0, sizeof(struct sockaddr_in));
	rt.rt_dev = (char*)name;
	gateway_addr.sin_family = PF_INET;
	
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	ioctl(sockfd, SIOCDELRT, &rt); 	//ÉèÖÃÍø¹ØÖ®Ç°ÏÈÉ¾³ýÖ®Ç°µÄ

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	gateway_addr.sin_addr.s_addr = gateway;
	memcpy(&rt.rt_gateway, &gateway_addr, sizeof(struct sockaddr_in));
	inet_aton("0.0.0.0", &gateway_addr.sin_addr);
	memcpy(&rt.rt_genmask, &gateway_addr, sizeof(struct sockaddr_in));
	memcpy(&rt.rt_dst, &gateway_addr, sizeof(struct sockaddr_in));
	if(ioctl(sockfd, SIOCADDRT, &rt) == -1)		//ÖØÐÂÌí¼ÓÐÂµÄ
	{
		printf("set %s gateway %s fail, error info: %s\n", name, inet_ntoa(addr->sin_addr), strerror(errno));
		close(sockfd);
		return false;
	}
	printf("set %s gateway %s successful!", name, inet_ntoa(addr->sin_addr));
	close(sockfd);
	return true;
}


in_addr Network::GetGateway(const char *net_dev, char *gateway)
{
    FILE *fp;    
    char buf[1024];  
    char iface[16];    
    unsigned char tmp[100]={'\0'};
    unsigned int dest_addr=0, gate_addr=0;
	struct sockaddr_in Addr = {0};
	
    if(NULL == gateway)
    {
        printf("gateway is NULL \n");
        return Addr.sin_addr; 
    }
    fp = fopen("/proc/net/route", "r");    
    if(fp == NULL){  
        printf("fopen error \n");
        return Addr.sin_addr;   
    }
    
    //fgets(buf, sizeof(buf), fp);    
    while(fgets(buf, sizeof(buf), fp)) 
    {    
        if((sscanf(buf, "%s\t%X\t%X", iface, &dest_addr, &gate_addr) == 3) 
            && (memcmp(net_dev, iface, strlen(net_dev)) == 0)
            && gate_addr != 0) 
        {
                memcpy(tmp, (unsigned char *)&gate_addr, 4);
                sprintf(gateway, "%d.%d.%d.%d", (unsigned char)*tmp, (unsigned char)*(tmp+1), (unsigned char)*(tmp+2), (unsigned char)*(tmp+3));
                break;    
        }
    }    
      
    fclose(fp);
	if (gateway[0] == 0)//don't get gateway,set default 0.0.0.0
		strcpy(gateway, "0.0.0.0");
	inet_aton(gateway, &Addr.sin_addr);
	return Addr.sin_addr;
}

in_addr Network::GetMask(const char *net_dev, char *mask)
{
    struct sockaddr_in Addr = {0};
    struct ifreq ifr;
    int sockfd;

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    memset(&ifr,0,sizeof(ifr));
    strncpy(ifr.ifr_name, net_dev, IFNAMSIZ - 1);
    
    if(ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0){
        close(sockfd);
        return Addr.sin_addr;
    }

	memcpy(&Addr, &(ifr.ifr_addr), sizeof(struct sockaddr_in));
    //pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
    strncpy(mask, (char *)(inet_ntoa(Addr.sin_addr)), 16);
    
    close(sockfd);
	return Addr.sin_addr;
}

int Network::GetMac(const char* net_dev, char* mac)
{
	struct ifreq ifr;
	unsigned char *macaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	if (mac == NULL)
		return 0;
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		printf("socket called fail, error info: %s\n", strerror(errno));
		return false;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, net_dev, IFNAMSIZ - 1);
	//ÏÈ»ñÈ¡macµØÖ·
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
	{
		printf("get mac address fail!");
		close(sockfd);
		return false;
	}
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
		macaddr[0], macaddr[1], macaddr[2],
		macaddr[3], macaddr[4], macaddr[5]);
	close(sockfd);
	return 1;
}

in_addr_t Network::GetNetworkCardIp(const char *interface)
{
	struct ifreq ifreqs[8];
    struct ifconf conf;
    int i, ifc_num;
    struct ifreq *ifr = ifreqs;
    struct in_addr addr = {0};
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	if (sockfd == -1) 
	{
		printf("create socket fail, error info: %s\n", strerror(errno));
		return addr.s_addr;
	}
	memset(ifr, 0, sizeof(ifreqs));
	if (interface != NULL)
	{
		strncpy(ifr->ifr_name, interface, IFNAMSIZ - 1);
		if (ioctl(sockfd, SIOCGIFADDR, ifr) != 0)
			return addr.s_addr;
		addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
		printf("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
		close(sockfd);
		return addr.s_addr;
	}
	//interfaceÎªNULLÊ±£¬»ñÈ¡ÕýÔÚÔËÐÐµÄÍø¿¨µÄipµØÖ·
    conf.ifc_len = sizeof(ifreqs);
    conf.ifc_req = ifreqs;
    if (ioctl(sockfd, SIOCGIFCONF, &conf))
    {   
        printf("get conf fail, error info: %s\n", strerror(errno));
        close(sockfd);
        return addr.s_addr;
    }   

    ifc_num = conf.ifc_len / sizeof(struct ifreq);
    for (i = 0; i < ifc_num; i++, ifr++)
	{
		if ((strncmp(ifr->ifr_name, "lo", 2) == 0)
            || (strncmp(ifr->ifr_name, "can", 3) == 0)) 
            continue;
		ioctl(sockfd, SIOCGIFFLAGS, ifr);
		if (ifr->ifr_flags & IFF_RUNNING)
		{
			addr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr;
			printf("Use network card %s, ip: %s\n", ifr->ifr_name, inet_ntoa(addr));
			break;
		}
	}
	close(sockfd);
	return addr.s_addr;
}
#endif