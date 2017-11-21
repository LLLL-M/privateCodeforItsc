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
#include "hik.h"
#include "sadp.h"

#define MAX_CARD_NUM	8
#define MAX_BUF_LEN 1024

typedef struct
{
	char name[6];
	UInt8 mac[6];
	UInt32 ipaddr;
	UInt32 mask;
	int index;
} EthInfo;

int sock = -1;	//抓获所有数据包使用

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
	INFO("hikTSC HardVersion:%s\r\nhikTSC SoftVersion:%s\r\n",st_device_version_params.hardVersionInfo,st_device_version_params.softVersionInfo);
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

void send_sadp_packet(char *packet, EthInfo *ethInfo)
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
	make_sadp_packet(etherHeader->h_source, header->sadp_sequence, ethInfo, buf);
	header = (struct sadp_header *)(buf + ETH_HLEN);
	sendto(sock, buf, header->sadp_len + ETH_HLEN, 0, (struct sockaddr *)&sa, sizeof(sa));
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

void SendByRunningEth(char *packet)
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
			send_sadp_packet(packet, &ethInfo);
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
			SendByRunningEth(packet);
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
