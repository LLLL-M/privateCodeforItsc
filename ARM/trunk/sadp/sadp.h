#ifndef __HIK_SADP_H__
#define __HIK_SADP_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERTYPE_SADP	    0X8033

#define SADPOPTYPE_ARQ	    1
#define SADPOPTYPE_AQR	    2
#define SADPOPTYPE_AIQ	    3
#define SADPOPTYPE_IQR	    4
#define SADPOPTYPE_IAR	    5
#define SADPOPTYPE_UIR	    6
#define SADPOPTYPE_UIA	    7
#define SADPOPTYPE_DFLTPASSWD_ARQ	    0x0a
#define SADPOPTYPE_DFLTPASSWD_ACK	    0x0b




#define SADP_ARQ_REQ_AIP    1
#define SADP_ARQ_REQ_UAIP   2

#define SADP_AQR_ACK_RRF 	1
#define SADP_AQR_ACK_UAIP 	2
#define SADP_AQR_ACK_AIP 	3

#define SADP_UIA_ACK_UIF    1
#define SADP_UIA_ACK_UIS    2

#define SADP_DFLTPASSWD_ACK_OK    1
#define SADP_DFLTPASSWD_ACK_FAILURE    2


#define MAX_PACKET_LEN	    500	    // mac + sadp
#define MIN_PACKET_LEN	    80	    // mac + sadp

#define MIN_PACKET	    66	    // MIN_PACKET_LEN - MAC_ADDR_LEN

#define SADP_HEADER_LEN	    38
#define MAC_HEADER_LEN	    14
#define MAC_ADDR_LEN	    6
#define SADP_IDENTIFIER	    0x21
#define SADP_VERSION	    0x02
#define SADP_SERIES	    0x01
#define SADP_HW_ADDR_LEN    6
#define SADP_PR_ADDR_LEN    4
#define DEVICE_SERIALNO_LEN 48
#define SOFTWARE_VERSION_LEN 48


#define PWD_LEN 	16

typedef int (* NETCFGCALLBACK)(unsigned int ip, unsigned int mask,unsigned int gateway,unsigned int port);

typedef int (* PASSED)(char *arg);

#pragma pack(push, 1)
struct sadp_header
{
	uint8_t sadp_identifier;
	uint8_t sadp_version;
	uint8_t sadp_series;
	uint8_t sadp_len;
	uint32_t sadp_sequence;
	uint8_t sadp_hwaddrlen;
	uint8_t sadp_praddrlen;
	uint8_t sadp_optype;
	uint8_t sadp_opcode;
	uint16_t sadp_crc;
	uint8_t sadp_src_hwaddr[6];
	uint32_t sadp_src_praddr;
	uint8_t sadp_dst_hwaddr[6];
	uint8_t sadp_dst_praddr[4];
	uint8_t sadp_subnet_mask[4];
	uint8_t sadp_data[1];
};
typedef struct {
	char				serial_no[DEVICE_SERIALNO_LEN];
	unsigned int		dev_type;
	unsigned int		port;
	unsigned int		enc_cnt;
	unsigned int		hdisk_cnt;
	char				software_version[SOFTWARE_VERSION_LEN];
	char				dsp_software_version[SOFTWARE_VERSION_LEN];
	char				start_time[SOFTWARE_VERSION_LEN];
}dev_info;
	

struct in6_addr6
{
	unsigned char s6_bytes6[16]; 
};
 
/*IP地址：包括IPv4和IPv6格式, 24 bytes*/
typedef struct tagU_IN_ADDR
{            
	struct in_addr    v4;                                      /* IPv4地址 */
	struct in6_addr6  v6;                                      /* IPv6地址 */
	unsigned char     byRes[4];
}U_IN_ADDR;
 
 
typedef struct _INTER_IPADDR
{
	U_IN_ADDR   	struIp;     /*IP地址*/
	unsigned short  wPort;      /*端口号*/
	unsigned char   byRes[2];
}INTER_IPADDR, *LPINTER_IPADDR;


typedef struct 
{
	unsigned long   	dwIPv4Gafeway;      	//IPv4网关
	char       			szIPv6Address[16];     	//IPv6地址
	char        		szIPv6Gateway[16];    	//IPv6网关
	unsigned char      	byIPv6MaskLen;      	//IPv6子网前缀长度
	unsigned char      	bySupport;           	/*按位表示,非0表示支持，0表示不支持
                                  			 	0x01:是否支持Ipv6
                                   				0x02:是否支持修改Ipv6参数
                                   				0x04:是否支持Dhcp*/
	unsigned char     	byDhcpEnabled;      	//Dhcp状态, 0 不启用 1 启用
	unsigned char      	byRes1;                	//保留 
	INTER_IPADDR 		struCmsAddr;         	//CMS地址和端口
	unsigned short     	wHttpPort;           	//Http端口
	unsigned short     	wDigitalChannelNum;  	//数字通道数目
	unsigned char 		byOemCode;				//0-基线设备 1-OEM设备
	unsigned char      	byRes2[31];          	//保留
	char       			szDevDesc[24];       	//设备类型描述
	char         		szOEMinfo[24];        	//OEM产商信息	
}INTER_DEVICE_INFO_V20, *LPINTER_DEVICE_INFO_V20;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif



