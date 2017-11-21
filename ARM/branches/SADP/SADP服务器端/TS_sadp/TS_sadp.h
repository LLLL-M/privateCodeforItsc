#ifndef __HIK_SADP_H__
#define __HIK_SADP_H__

#include <sys/types.h>
#include <netinet/ip.h>

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
#define SADP_VERSION	    0x01
#define SADP_SERIES	    0x01
#define SADP_HW_ADDR_LEN    6
#define SADP_PR_ADDR_LEN    4
#define DEVICE_SERIALNO_LEN 48
#define SOFTWARE_VERSION_LEN 48


#define PWD_LEN 	16

typedef int (* NETCFGCALLBACK)(unsigned int ip, unsigned int mask,unsigned int gateway,unsigned int port);

typedef int (* PASSED)(char *arg);

struct ether_header
{
	u_int8_t des_addr[6];
	u_int8_t src_addr[6];
	u_int16_t ether_type;
};

struct sadp_header
{
	u_int8_t sadp_identifier;
	u_int8_t sadp_version;
	u_int8_t sadp_series;
	u_int8_t sadp_len;
	u_int32_t sadp_sequence;
	u_int8_t sadp_hwaddrlen;
	u_int8_t sadp_praddrlen;
	u_int8_t sadp_optype;
	u_int8_t sadp_opcode;
	u_int16_t sadp_crc;
	u_int8_t sadp_src_hwaddr[6];
	u_int32_t sadp_src_praddr;
	u_int8_t sadp_dst_hwaddr[6];
	u_int8_t sadp_dst_praddr[4];
	u_int8_t sadp_subnet_mask[4];
	u_int8_t sadp_data[1];
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
 
/*IP��ַ������IPv4��IPv6��ʽ, 24 bytes*/
typedef struct tagU_IN_ADDR
{            
	struct in_addr    v4;                                      /* IPv4��ַ */
	struct in6_addr6  v6;                                      /* IPv6��ַ */
	unsigned char     byRes[4];
}U_IN_ADDR;
 
 
typedef struct _INTER_IPADDR
{
	U_IN_ADDR   	struIp;     /*IP��ַ*/
	unsigned short  wPort;      /*�˿ں�*/
	unsigned char   byRes[2];
}INTER_IPADDR, *LPINTER_IPADDR;

typedef struct STRU_Extra_Param_Version
{
     unsigned int     unExtraParamHead;                 //��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                   //����,0xA1��ʾ����
     unsigned char    softVersionInfo[32];                //����汾��Ϣ
     unsigned char    hardVersionInfo[32];                //Ӳ���汾��Ϣ
}DEVICE_VERSION_PARAMS;


typedef struct 
{
	unsigned long   	dwIPv4Gafeway;      	//IPv4����
	char       			szIPv6Address[16];     	//IPv6��ַ
	char        		szIPv6Gateway[16];    	//IPv6����
	unsigned char      	byIPv6MaskLen;      	//IPv6����ǰ׺����
	unsigned char      	bySupport;           	/*��λ��ʾ,��0��ʾ֧�֣�0��ʾ��֧��
                                  			 	0x01:�Ƿ�֧��Ipv6
                                   				0x02:�Ƿ�֧���޸�Ipv6����
                                   				0x04:�Ƿ�֧��Dhcp*/
	unsigned char     	byDhcpEnabled;      	//Dhcp״̬, 0 ������ 1 ����
	unsigned char      	byRes1;                	//���� 
	INTER_IPADDR 		struCmsAddr;         	//CMS��ַ�Ͷ˿�
	unsigned short     	wHttpPort;           	//Http�˿�
	unsigned short     	wDigitalChannelNum;  	//����ͨ����Ŀ
	unsigned char 		byOemCode;				//0-�����豸 1-OEM�豸
	unsigned char      	byRes2[31];          	//����
	char       			szDevDesc[24];       	//�豸��������
	char         		szOEMinfo[24];        	//OEM������Ϣ	
}INTER_DEVICE_INFO_V20, *LPINTER_DEVICE_INFO_V20;


/* init sadp lib 
 * unsigned int ip :net order 
 * unsigned int mask:net order 
 * NETCFGCALLBACK pFunc function to modify ip ,mask,gaeway
 */
int init_sadp_lib(unsigned int ip, unsigned int mask, unsigned char* mac,
	       	char*password,int pw_len, dev_info *info, NETCFGCALLBACK netconfig,PASSED resetDfltPasswd);

#if 1   // TS_UPGRADE
int init_sadp_lib2(char *ifName, unsigned int ip, unsigned int mask, unsigned char* mac,
            char*password,int pw_len, dev_info *info, NETCFGCALLBACK netconfig,PASSED resetDfltPasswd);
#endif

int fini_sadp_lib();
int sadp_login();
int start_sadp_cap();
int stop_sadp_cap();

#ifdef __cplusplus
}
#endif

#endif



