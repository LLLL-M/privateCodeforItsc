#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/filter.h>
#include <net/route.h>   
#include <sys/sysinfo.h>
#include <unistd.h>    
#include <stdio.h>    
#include <pthread.h>
#include "hik.h"
#include "sadp.h"

#define MAX_CARD_NUM	8
#define MAX_BUF_LEN 1024

//#define UUID    "8d2091bc-1dd2-11b2-807b-8ce748cf9334"

char UUID[36*2] = {0};//客户端传输来的UUID值

typedef struct
{
	char name[6];
	UInt8 mac[6];
	UInt32 ipaddr;
	UInt32 mask;
	int index;
} EthInfo;

static int sock = -1;	//抓获所有数据包使用
static int sockUdp = -1;
unsigned short sadp_check_sum(unsigned short* data, int len)
{
	unsigned long sum = 0;
	while ( len > 1 )
	{
		sum += *data++;
		len -= sizeof(unsigned short);
	}

	if ( len )
		sum += *(unsigned char*)data;

	sum = (sum >> 16) + ( sum & 0xffff );
	sum += sum >> 16;

	return (unsigned short)(~sum);
}

typedef struct STRU_Extra_Param_Version
{
	unsigned int     unExtraParamHead;                 //标志头,0x6e6e
	unsigned int     unExtraParamID;                   //类型,0xA1表示上载
	unsigned char    softVersionInfo[32];                //软件版本信息
	unsigned char    hardVersionInfo[32];                //硬件版本信息
}DEVICE_VERSION_PARAMS;

//发送udp组播
static void sendData(int sockfd, char *val,int size)
{
	int ret = 0;
    struct sockaddr_in broadcastServer = {
    .sin_family = PF_INET,  
	.sin_port = htons(37020),
	.sin_addr.s_addr = inet_addr("239.255.255.250"),
    .sin_zero = {0}          
    };	

	ret = sendto(sockfd,(void *)val,size,0,(struct sockaddr *)&broadcastServer,sizeof(struct sockaddr_in));
	
	if(ret <= 0){
		ERR("send error :%s\n",strerror(errno));
	}else{
//		INFO("send succeed , sockfd  %d, size %d, ret %d \n",sockfd,size,ret);
	}
}

//创建套接字
static int CreateSocket(int type,char *ip,int port)
{
    int tmpSocket = 0;
    struct sockaddr_in tmpAddr;
    int ret  = 0;
    int opt = 1;
	struct ip_mreq mreq;
    
    tmpSocket = socket(AF_INET,type == 0 ? SOCK_DGRAM : SOCK_STREAM,0);
    if(tmpSocket == -1){
        ERR("create socket failed : %s\n",strerror(errno));
        return -1;
    }
    
    memset(&tmpAddr,0,sizeof(tmpAddr));
    tmpAddr.sin_family = PF_INET;
    tmpAddr.sin_port = htons(port);
    tmpAddr.sin_addr.s_addr = inet_addr(ip);
	
	mreq.imr_multiaddr.s_addr = inet_addr(ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
    ret = bind(tmpSocket, (struct sockaddr *)&tmpAddr,sizeof(struct sockaddr));
    if(ret == -1){
        ERR("bind socket failed : %s\n",strerror(errno));
        close(tmpSocket);
        return -1;
    }
    
    setsockopt(tmpSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(int));
    setsockopt(tmpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(struct ip_mreq));
	
    return tmpSocket;
}
//设置网关
int set_gateway(unsigned long gw)    
{    
  int skfd;    
  struct rtentry rt;    
  int err;    
  
  skfd = socket(PF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
  
  /* Delete existing defalt gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP;    
  
  err = ioctl(skfd, SIOCDELRT, &rt);    
  
  if ((err == 0 || errno == ESRCH) && gw) {    
  /* Set default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP | RTF_GATEWAY;    
  
  err = ioctl(skfd, SIOCADDRT, &rt);    
 }    
  
  close(skfd);    
  
  return err;    
}    
//获取网关  
int get_gateway(UInt32 *p)    
{    
  FILE *fp;    
  char buf[256]; // 128 is enough for linux    
  char iface[16];    
  UInt32 dest_addr, gate_addr;    
  *p = INADDR_NONE;    
  fp = fopen("/proc/net/route", "r");    
  if (fp == NULL)    
    return -1;    
  /* Skip title line */    
  if(fgets(buf, sizeof(buf), fp) == NULL)
       return 0;    
  while (fgets(buf, sizeof(buf), fp)) {    
    if (sscanf(buf, "%s\t%X\t%X", iface,&dest_addr, &gate_addr) != 3 || dest_addr != 0)    
             continue;    
      *p = gate_addr; 
      break;    
  }  
  fclose(fp);    
  return 0;    
}  
//获取系统启动时间
time_t get_boot_time()
{
    struct sysinfo info;
    time_t cur_time = 0;
    time_t boot_time = 0;
    if (sysinfo(&info)) {
        fprintf(stderr, "Failed to get sysinfo,reason:%s\n",strerror(errno));
        return 0;
    }
    time(&cur_time);
    if (cur_time > info.uptime) {
        boot_time = cur_time - info.uptime;
    }
    else {
        boot_time = info.uptime - cur_time;
    }   
    
    return boot_time;
}

void getSoftwareVersion(char *softVerBuf,char *hardVerBuf)
{
    DEVICE_VERSION_PARAMS st_device_version_params;
	struct sockaddr_in toAddr = 
	{
		.sin_family = AF_INET,
		.sin_addr = {inet_addr("127.0.0.1")},
		.sin_port = htons(20000),
	};
	int localSock = socket(AF_INET,SOCK_DGRAM,0);	//与本地通信获取软硬件版本使用
	if(localSock < 0)
	{
		ERR("创建本地套接字失败!\r\n");
		return;
	}

	memset(&st_device_version_params,0,sizeof(DEVICE_VERSION_PARAMS));
	st_device_version_params.unExtraParamID = 0xa1;
	st_device_version_params.unExtraParamHead = 0x6e6e;
	if(sendto(localSock, &st_device_version_params, 9, MSG_DONTWAIT, (struct sockaddr*)&toAddr, sizeof(toAddr)) == -1)
	{
	 	ERR("sendto() 失败");
		close(localSock);
	 	return;
	}
	sleep(1);
	if(recv(localSock, &st_device_version_params, sizeof(DEVICE_VERSION_PARAMS), MSG_DONTWAIT) <= 0)
	{
	 	ERR("recvfrom()失败\r\n");
		close(localSock);
	 	return;
	}
	//INFO("hikTSC HardVersion:%s\r\nhikTSC SoftVersion:%s\r\n",st_device_version_params.hardVersionInfo,st_device_version_params.softVersionInfo);
	strcpy(softVerBuf, (char *)st_device_version_params.softVersionInfo);
	strcpy(hardVerBuf, (char *)st_device_version_params.hardVersionInfo);
	close(localSock);
}

void make_sadp_packet(UInt8 *smac, UInt32 seq, EthInfo *ethInfo, char *buf)
{
	dev_info devInfo = {
		.serial_no = "UNKNOWN",
		.dev_type = htonl(2),
		.port = htonl(0),
		.enc_cnt = htonl(0),
		.hdisk_cnt = htonl(0),
		.software_version = "UNKNOWN",
		.dsp_software_version = {0},
		.start_time = {0},
	};
	struct ethhdr *etherHeader = (struct ethhdr *)buf;
	struct sadp_header *header = (struct sadp_header *)(buf + ETH_HLEN);

	getSoftwareVersion(devInfo.software_version, devInfo.serial_no);
	if (strstr(devInfo.software_version, "V1") != NULL)
		devInfo.port = htonl(161);
	else if (strstr(devInfo.software_version, "V2") != NULL)
		devInfo.port = htonl(20000);
	memcpy(etherHeader->h_dest, smac, ETH_ALEN);
	etherHeader->h_proto = htons(ETHERTYPE_SADP);
#if 0
	memcpy(header->sadp_dst_hwaddr, smac, ETH_ALEN);
	temp = htonl(g_sadpInst.dst_ip);
	memcpy(header->sadp_dst_praddr,&temp, 4);
#endif
	header->sadp_sequence = seq;
	header->sadp_identifier = SADP_IDENTIFIER;
	header->sadp_version = SADP_VERSION;
	header->sadp_series  = SADP_SERIES;
	header->sadp_len = SADP_HEADER_LEN + sizeof(dev_info);
	header->sadp_hwaddrlen = SADP_HW_ADDR_LEN;
	header->sadp_praddrlen = SADP_PR_ADDR_LEN;
	header->sadp_optype = SADPOPTYPE_IQR;
	header->sadp_opcode = 0;
	header->sadp_crc = 0;
	memcpy(header->sadp_data, &devInfo, sizeof(dev_info));
	
	memcpy(etherHeader->h_source, ethInfo->mac, ETH_ALEN);
	header->sadp_src_praddr = ethInfo->ipaddr;
	memcpy(header->sadp_src_hwaddr, ethInfo->mac, ETH_ALEN);
	memcpy(header->sadp_subnet_mask, &ethInfo->mask, 4);
	header->sadp_crc = htons(sadp_check_sum((unsigned short *)header, header->sadp_len));
}

#define GET_START_ADDR(pack)	(pack)+strlen((pack))
//type: 0 主动上线，发送的是hello，1 搜索请求应答
void make_sadp_packet_xml(EthInfo *ethInfo, char *pack,int bufSize,int type)
{
	dev_info devInfo = {
		.serial_no = "UNKNOWN",
		.dev_type = 2,
		.port = htonl(0),
		.enc_cnt = htonl(0),
		.hdisk_cnt = htonl(0),
		.software_version = "UNKNOWN",
		.dsp_software_version = {0},
		.start_time = {0},
	};

    UInt32 gateWay = 0;
    struct tm *ptm = NULL;
    time_t boot_time = 0;
    
    get_gateway(&gateWay);
    boot_time = get_boot_time();
    ptm = gmtime(&boot_time);
    
	getSoftwareVersion(devInfo.software_version, devInfo.serial_no);
	if (strstr(devInfo.software_version, "V1") != NULL)
		devInfo.port = 161;
	else if (strstr(devInfo.software_version, "V2") != NULL)
		devInfo.port = 20000;
	
    memset(pack,0,sizeof(bufSize));
    
    sprintf(pack,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
    if(type == 0){
        sprintf(GET_START_ADDR(pack),"<Hello>\r\n");
        sprintf(GET_START_ADDR(pack),"<Types>hello</Types>\r\n");
    }else{
        sprintf(GET_START_ADDR(pack),"<ProbeMatch>\r\n");
        sprintf(GET_START_ADDR(pack),"<Uuid>%s</Uuid>\r\n",UUID);
        sprintf(GET_START_ADDR(pack),"<Types>inquiry</Types>\r\n");
    }
	
	sprintf(GET_START_ADDR(pack),"<DeviceType>%d</DeviceType>\r\n",devInfo.dev_type);
	sprintf(GET_START_ADDR(pack),"<DeviceDescription>%s</DeviceDescription>\r\n","HIK TSC");
	sprintf(GET_START_ADDR(pack),"<DeviceSN>%s</DeviceSN>\r\n",devInfo.serial_no);
	sprintf(GET_START_ADDR(pack),"<CommandPort>%d</CommandPort>\r\n",devInfo.port);
	sprintf(GET_START_ADDR(pack),"<HttpPort>80</HttpPort>\r\n");
	
	sprintf(GET_START_ADDR(pack),"<MAC>%02x:%02x:%02x:%02x:%02x:%02x</MAC>\r\n",
            ethInfo->mac[0], ethInfo->mac[1], ethInfo->mac[2],
			ethInfo->mac[3], ethInfo->mac[4], ethInfo->mac[5]);
	sprintf(GET_START_ADDR(pack),"<IPv4Address>%s</IPv4Address>\r\n",inet_ntoa(*(struct in_addr *)&ethInfo->ipaddr));
	sprintf(GET_START_ADDR(pack),"<IPv4SubnetMask>%s</IPv4SubnetMask>\r\n",inet_ntoa(*(struct in_addr *)&ethInfo->mask));
	sprintf(GET_START_ADDR(pack),"<IPv4Gateway>%s</IPv4Gateway>\r\n",inet_ntoa(*(struct in_addr *)&gateWay));
	
	sprintf(GET_START_ADDR(pack),"<IPv6Address></IPv6Address>\r\n");
	sprintf(GET_START_ADDR(pack),"<IPv6Gateway></IPv6Gateway>\r\n");
	sprintf(GET_START_ADDR(pack),"<IPv6MaskLen></IPv6MaskLen>\r\n");
	sprintf(GET_START_ADDR(pack),"<DHCP>false</DHCP>\r\n");
	sprintf(GET_START_ADDR(pack),"<AnalogChannelNum>0</AnalogChannelNum>\r\n");
	sprintf(GET_START_ADDR(pack),"<DigitalChannelNum>0</DigitalChannelNum>\r\n");
	
	sprintf(GET_START_ADDR(pack),"<SoftwareVersion>%s</SoftwareVersion>\r\n",devInfo.software_version);//
	sprintf(GET_START_ADDR(pack),"<DSPVersion></DSPVersion>\r\n");
	sprintf(GET_START_ADDR(pack),"<BootTime>%d-%-d-%d %d:%d:%d</BootTime>\r\n", ptm->tm_year + 1900,
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	sprintf(GET_START_ADDR(pack),"<OEMCode>false</OEMCode>\r\n");
	sprintf(GET_START_ADDR(pack),"<DeviceType>2</DeviceType>\r\n");
//	sprintf(GET_START_ADDR(pack),"<SoftwareVersionEncrypt>2</SoftwareVersionEncrypt>\r\n");
//	sprintf(GET_START_ADDR(pack),"<DSPVersionEncrypt>2</DSPVersionEncrypt>\r\n");
	
//	sprintf(GET_START_ADDR(pack),"<Encrypt>false</Encrypt>\r\n");
//	sprintf(GET_START_ADDR(pack),"<SafeCode>123456</SafeCode>\r\n");
//	sprintf(GET_START_ADDR(pack),"<ResetAbility>false</ResetAbility>\r\n");
//	sprintf(GET_START_ADDR(pack),"<DiskNumber>0</DiskNumber>\r\n");
	sprintf(GET_START_ADDR(pack),"<Activated>true</Activated>\r\n");
//	sprintf(GET_START_ADDR(pack),"<PasswordResetAbility>false</PasswordResetAbility>\r\n");
//	sprintf(GET_START_ADDR(pack),"<SyncIPCPassword>true</SyncIPCPassword>\r\n");
//	sprintf(GET_START_ADDR(pack),"<PasswordResetModeSecond>false</PasswordResetModeSecond>\r\n");
//	sprintf(GET_START_ADDR(pack),"<DetailOEMCode>12</DetailOEMCode>\r\n");
	sprintf(GET_START_ADDR(pack),"<EZVIZCode>false</EZVIZCode>\r\n");
	
    sprintf(GET_START_ADDR(pack),"</%s>\r\n",type == 0 ? "Hello" : "ProbeMatch");
    
    //log_debug("--->  %s\n",pack);
}

//version: 0 表示第一版的SADP，1表示新版的SADP，走的是xml
void send_sadp_packet(int version,int msgType,char *packet, EthInfo *ethInfo)
{
	char buf[MAX_BUF_LEN] = {0};
	struct ethhdr *etherHeader = (struct ethhdr *)packet;
	struct sadp_header *header = (struct sadp_header *)(packet + ETH_HLEN);
	struct sockaddr_ll sa;
	
	memset(&sa, 0, sizeof(sa));
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = ethInfo->index; 
	sa.sll_protocol = htons(ETH_P_ALL);
#if 0
	struct in_addr srcip = {header->sadp_src_praddr};
	char cmd[40] = {0};
	sprintf(cmd, "ping %s -w 4 > /dev/null &", inet_ntoa(srcip));
	INFO("source ip is %s\n", inet_ntoa(srcip));
	system(cmd);
#endif
	
	if(version == 0)
	{
        make_sadp_packet(etherHeader->h_source, header->sadp_sequence, ethInfo, buf);
        header = (struct sadp_header *)(buf + ETH_HLEN);
		sendto(sock, buf, header->sadp_len + ETH_HLEN, 0, (struct sockaddr *)&sa, sizeof(sa));
	}else{
        make_sadp_packet_xml(ethInfo,packet,MAX_BUF_LEN*2,msgType);
		sendData(sockUdp,packet,strlen(packet));
	}
}

Boolean GetEthInfo(int sockfd, struct ifreq *ifr, EthInfo *ethInfo)
{
	ioctl(sockfd, SIOCGIFFLAGS, ifr);
	if ((ifr->ifr_flags & IFF_RUNNING) == 0)
		return FALSE;
#if 0	//设置网口为混杂模式
	ifr->ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, ifr);
#endif
	ethInfo->ipaddr = ((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr.s_addr;
	memcpy(ethInfo->name, ifr->ifr_name, strlen(ifr->ifr_name));
	ioctl(sockfd, SIOCGIFHWADDR, ifr);
	memcpy(ethInfo->mac, ifr->ifr_hwaddr.sa_data, 6);
//    ethInfo->mac[6] = '\0';
	ioctl(sockfd, SIOCGIFNETMASK, ifr);
	ethInfo->mask = ((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr;
	ioctl(sockfd, SIOCGIFINDEX, ifr);
	ethInfo->index = ifr->ifr_ifindex;
#if 0
	printf("name = [%s]\n" , ethInfo->name);
	printf("mac = %02x:%02x:%02x:%02x:%02x:%02x\n", 
			ethInfo->mac[0], ethInfo->mac[1], ethInfo->mac[2],
			ethInfo->mac[3], ethInfo->mac[4], ethInfo->mac[5]);
	printf("ipaddr = [%s]\n", inet_ntoa(*(struct in_addr *)&ethInfo->ipaddr));
	printf("mask = [%s]\n", inet_ntoa(*(struct in_addr *)&ethInfo->mask));
#endif
	return TRUE;
}

void SendByRunningEth(int version,int msgType,char *packet)
{
	struct ifreq ifreqs[MAX_CARD_NUM];
	struct ifconf conf;
	int i, ifc_num;
	struct ifreq *ifreq = ifreqs;
	int sockfd = -1;
	EthInfo ethInfo;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		ERR("socket called fail, error info: %s\n", strerror(errno));
		return;
	}
	conf.ifc_len = sizeof(ifreqs);
	conf.ifc_req = ifreqs;
	if (ioctl(sockfd, SIOCGIFCONF, &conf))
	{
		ERR("get conf fail, error info: %s\n", strerror(errno));
		close(sockfd);
		return;
	}

	ifc_num = conf.ifc_len / sizeof(struct ifreq);
	for (i = 0; i < ifc_num; i++, ifreq++)
	{
		if ((strncmp(ifreq->ifr_name, "lo", 2) == 0)
			|| (strncmp(ifreq->ifr_name, "can", 3) == 0))
			continue;
		memset(&ethInfo, 0, sizeof(ethInfo));
		if (GetEthInfo(sockfd, ifreq, &ethInfo))
		{
			send_sadp_packet(version,msgType,packet, &ethInfo);
		}
	}
	close(sockfd);
}

void DealPacket(char *packet, int len)
{
	//struct ethhdr *etherHeader = (struct ethhdr *)packet;
	struct sadp_header* header = (struct sadp_header *)(packet + ETH_HLEN);
	switch (header->sadp_optype)
	{
		case SADPOPTYPE_AQR:
			//printf("parse_sadp_packet_proc_op_AQR:%d\n", header->sadp_optype);
			//proc_op_AQR(header, len);
			break;

		case SADPOPTYPE_AIQ:
			//printf("parse_sadp_packet_proc_op_AIQ:%d\n", header->sadp_optype);
			//proc_op_AIQ(etherHeader->h_source, header->sadp_sequence);
			SendByRunningEth(0,0,packet);
			break;

		case SADPOPTYPE_UIR:
			//printf("parse_sadp_packet_proc_op_UIR:%d\n", header->sadp_optype);
			//proc_op_UIR(header, len);
			break;

		case SADPOPTYPE_DFLTPASSWD_ARQ:
			//printf("parse_sadp_packet_proc_op_passwd:%d\n", header->sadp_optype);
			//proc_op_passwd(header);
			break;

		default:
			//printf("parse_sadp_packet_default:%d\n", header->sadp_optype);
			break;
	}
}

Boolean is_inquiry_valid(char *msg)
{
    char *p = NULL;
    char *q = NULL;
    int ret = 0;
    
    if(msg == NULL || strstr(msg,"<Probe>") == NULL){
        return FALSE;
    }
    
    p = strstr(msg,"<Uuid>");
	q = strstr(msg,"</Uuid>");
 
    if(q != NULL && p != NULL)
	{
        memset(UUID,0,sizeof(UUID));
		ret = q - p - 6;
		strncpy(UUID,p+6,ret);
//		printf("~~~~~~   %s \n",UUID);
    }else {
        return FALSE;
    }    
    
//    INFO("%s \n",msg);
    return TRUE;
}


int IsIfconfigReady()
{
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	int sockfd;

	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		return -1;
		
	}

	ioctl(sockfd,SIOCGIFCONF,&ifconf);

	ifreq = (struct ifreq*)buf;

	int i = 0;
	i = ifconf.ifc_len / sizeof(struct ifreq);
	int index = 0;	

	for(; i > 0 ; i--)
	{
		if(ifreq->ifr_name[0] != 'l')
		{
			if(strlen(inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr)) != 0)
			{	
				//log_debug("%s   \n",inet_ntoa(((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
				index++;
			}

		}

		ifreq++;
	}
	
	return index;
}

void WaitForNetWorkReady()
{
	//if the ethnet ready
	while(1)
	{
		if(IsIfconfigReady() < 1)
		{
			//log_debug("net work is not ok , waiting ...\n");
			sleep(1);

		}
		else
		{
			//log_debug("net work is  ok ,start service\n");
			break;
		}	
	}

}

void *threadSendDeviceInfo(void *arg)
{
	char packet[MAX_BUF_LEN*2];
	int ret = -1;
	struct sockaddr_in addrFrom;
	int nLen = sizeof(addrFrom);
	
	WaitForNetWorkReady();
	sockUdp = CreateSocket(0,"239.255.255.250",37020);
	
    SendByRunningEth(1,0,packet);//上电启动发送到是hello消息
	while(1)
	{
		memset(packet,0,sizeof(packet));
                
		ret = recvfrom(sockUdp, packet, sizeof(packet), 0, (struct sockaddr*)&addrFrom, (socklen_t*)&nLen);
		
		if((ret > 0) && (is_inquiry_valid(packet) == TRUE)){//接收到请求后，反馈的是请求消息
			
			SendByRunningEth(1,1,packet);
		}
	
	}
	return NULL;
}


int main(void)
{
	// tcpdump -dd ether proto 0x8033
	struct sock_filter BPF_code[] = {
		{0x28, 0, 0, 0x0000000c},
		{0x15, 0, 1, 0x00008033},
		{0x6, 0, 0, 0x00000060},
		{0x6, 0, 0, 0x00000000}
	};
	//init filter settings
	struct sock_fprog Filter = {
		.len = sizeof(BPF_code) / sizeof(struct sock_filter),
		.filter = BPF_code,	
	};
	char packet[MAX_BUF_LEN];
	int len = 0;
	pthread_t id = 0;

	INFO("compile time: %s, %s", __DATE__, __TIME__);
	sock = socket(PF_PACKET, SOCK_RAW, htons(ETHERTYPE_SADP));
	if (sock < 0) 
	{
		ERR("create BPF filter fail\n");
		return -1;
	}
	/* Attach the filter to the socket */
	if(setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter)) < 0)
	{
		close(sock);
		ERR("attack filter to socket fail, error: %s\n", strerror(errno));
		return -1;
	}
	//InitLogSystem("/root/",4,1);
	pthread_create(&id,NULL,threadSendDeviceInfo,NULL);
	
	while (1)
	{
		memset(packet, 0, sizeof(packet));
		len = read(sock, packet, MAX_BUF_LEN);
		if (len > MAX_PACKET_LEN || len < MIN_PACKET_LEN)
			continue;
		DealPacket(packet, len);
	}

	return 0;
}
