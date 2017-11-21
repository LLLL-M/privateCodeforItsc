#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>

#include "sadp.h"
#include "MD5.h"
#include "packet.h"

#ifndef FALSE
#define FALSE 	0
#endif

#ifndef TRUE
#define TRUE 	1
#endif

#ifndef BOOL
#define BOOL int
#endif


#define PACKET_SIZE	1024

struct SADP
{
	struct lib_cap descr;

	unsigned int seq;
	u_int32_t* packet;
	u_int32_t packet_s;

	char password[PWD_LEN];
	unsigned char serialno[DEVICE_SERIALNO_LEN];
	unsigned int  mask;                     
	unsigned int  src_ip;
	unsigned char src_mac[MAC_ADDR_LEN];

	unsigned int  dst_ip;
	unsigned char dst_mac[MAC_ADDR_LEN];

	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	int login_result;
	unsigned int		dev_type;
	unsigned int		port;
	unsigned int		enc_cnt;
	unsigned int		hdisk_cnt;
	char				software_version[SOFTWARE_VERSION_LEN];
	char				dsp_software_version[SOFTWARE_VERSION_LEN];
	char				start_time[SOFTWARE_VERSION_LEN];            
	NETCFGCALLBACK fn_net_config;
	PASSED reset_dflt_passwd;
};
static dev_info	g_info;
static struct SADP g_sadpInst;
char* g_filter = "ether proto 0x8033";
volatile int g_stopflag = 0;


/*init sadp
 *ip: net order
 *mask: net order
 */
 void get_dev_info(dev_info * info);
int init_sadp_lib(unsigned int ip, unsigned int mask, unsigned char* mac, 
		char*password,int pw_len, dev_info *info,NETCFGCALLBACK netconfig,PASSED resetDfltPasswd);

// TS_UPGRADE
int init_sadp_lib2(char *ifName, unsigned int ip, unsigned int mask, unsigned char* mac,
        char*password,int pw_len, dev_info *info,NETCFGCALLBACK netconfig,PASSED resetDfltPasswd);

int fini_sadp_lib();
int start_sadp_cap();
int stop_sadp_cap();
int sadp_login();

void* thr_sadp_capture(void*pv);
unsigned int sadp_getseq();
unsigned short sadp_check_sum(unsigned short* data, int len);

void str_to_mac(const char* addr_str, u_char* mac_addr);

BOOL verify_mac_addr(u_char* addr);
BOOL verify_password(const char* pw);
BOOL verify_sadp_packet(char* packet, int len);


int parse_sadp_packet(char* packet, int len);
void proc_op_AQR(struct sadp_header* header, int len);
void proc_op_AIQ(struct sadp_header* header);
void proc_op_UIR(struct sadp_header* header, int len);
void proc_op_passwd(struct sadp_header* header);


void make_sadp_packet(u_int8_t optype,u_int8_t opcode,char * content,int cont_len, unsigned int seq);
void send_sadp_packet(struct SADP* sadp_inst);

int modify_ip_config(struct sadp_header* header, int len);

//-------------------------------------------------------------
//
int init_sadp_lib(unsigned int ip, unsigned int mask, unsigned char* mac, 
		char*password,int pw_len, dev_info *info,  NETCFGCALLBACK netconfig,PASSED resetDfltPasswd)
{
	u_char enet_dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	bzero(&g_sadpInst,sizeof(struct SADP));
	
	if ( init_packet_capture(&g_sadpInst.descr) == -1 )
		return -1;

	g_sadpInst.src_ip = ip;
	g_sadpInst.mask = mask;

	memcpy(g_sadpInst.src_mac,mac,6);
	memcpy(g_sadpInst.dst_mac,enet_dst,6);

	memcpy(g_sadpInst.password,password,pw_len);
	memcpy(g_sadpInst.serialno,info->serial_no,DEVICE_SERIALNO_LEN);
	memcpy(g_sadpInst.dsp_software_version,info->dsp_software_version,SOFTWARE_VERSION_LEN);
	memcpy(g_sadpInst.software_version,info->software_version,SOFTWARE_VERSION_LEN);
	memcpy(g_sadpInst.start_time,info->start_time,SOFTWARE_VERSION_LEN);
	g_sadpInst.dev_type = info->dev_type;
	g_sadpInst.port = info->port;
	g_sadpInst.enc_cnt = info->enc_cnt;
	g_sadpInst.hdisk_cnt = info->hdisk_cnt;	

	g_sadpInst.packet = malloc(PACKET_SIZE);
	g_sadpInst.packet_s = PACKET_SIZE;

	g_sadpInst.fn_net_config = netconfig;

	g_sadpInst.reset_dflt_passwd = resetDfltPasswd;

	pthread_mutex_init(&g_sadpInst.mutex,NULL);

	pthread_cond_init(&g_sadpInst.cond,NULL);

	g_stopflag = FALSE; 

	return 0;
}

#if 1   // TS_UPGRADE
int init_sadp_lib2(char *ifName, unsigned int ip, unsigned int mask, unsigned char* mac,
        char *password,int pw_len, dev_info *info,  NETCFGCALLBACK netconfig,PASSED resetDfltPasswd)
{
    u_char enet_dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    bzero(&g_sadpInst,sizeof(struct SADP));

    if ( init_packet_capture2(&g_sadpInst.descr, ifName) == -1 )
        return -1;

    g_sadpInst.src_ip = ip;
    g_sadpInst.mask = mask;

    memcpy(g_sadpInst.src_mac,mac,6);
    memcpy(g_sadpInst.dst_mac,enet_dst,6);

    memcpy(g_sadpInst.password,password,pw_len);
    memcpy(g_sadpInst.serialno,info->serial_no,DEVICE_SERIALNO_LEN);
    memcpy(g_sadpInst.dsp_software_version,info->dsp_software_version,SOFTWARE_VERSION_LEN);
    memcpy(g_sadpInst.software_version,info->software_version,SOFTWARE_VERSION_LEN);
    memcpy(g_sadpInst.start_time,info->start_time,SOFTWARE_VERSION_LEN);
    g_sadpInst.dev_type = info->dev_type;
    g_sadpInst.port = info->port;
    g_sadpInst.enc_cnt = info->enc_cnt;
    g_sadpInst.hdisk_cnt = info->hdisk_cnt;

    g_sadpInst.packet = malloc(PACKET_SIZE);
    g_sadpInst.packet_s = PACKET_SIZE;

    g_sadpInst.fn_net_config = netconfig;

    g_sadpInst.reset_dflt_passwd = resetDfltPasswd;

    pthread_mutex_init(&g_sadpInst.mutex,NULL);

    pthread_cond_init(&g_sadpInst.cond,NULL);

    g_stopflag = FALSE;

    return 0;
}
#endif

int fini_sadp_lib()
{
	fini_packet_capture(&g_sadpInst.descr);

	pthread_mutex_destroy(&g_sadpInst.mutex);
	pthread_cond_destroy(&g_sadpInst.cond);

	return 0;
}

int start_sadp_cap()
{
	pthread_t cap_thread;
	pthread_create(&cap_thread,NULL,thr_sadp_capture,NULL);

	return 0;
}

int stop_sadp_cap()
{
	g_stopflag = TRUE;
	return 0;
}


void* thr_sadp_capture(void*pv)
{
	//printf("go into thr_sadp_capture\n");
	u_int8_t* pkt_data = NULL;
	struct ether_header* eth_header = NULL;
	int caplen = 0;
	char* sadp_data = NULL;
	char * pbuf	=	NULL;
	pbuf = (char *)memalign(16,MAX_PACKET_LEN);
	if(pbuf==NULL)
		return 0;
	while( !g_stopflag )
	{
		caplen = get_capture_packet(&g_sadpInst.descr,&pkt_data);
		if ( pkt_data == NULL )
			break;

		if ( caplen > MAX_PACKET_LEN || caplen < MIN_PACKET_LEN )
			continue;


		eth_header = (struct ether_header*)pkt_data;
		if ( eth_header->ether_type != ntohs(ETHERTYPE_SADP) )
			continue;

		if ( !verify_mac_addr(eth_header->des_addr) )
			continue;
		
		sadp_data = (char*)(pkt_data + MAC_HEADER_LEN);
		
		// make arm happy. arm request addr alignment.
		memcpy(pbuf, sadp_data,(caplen-MAC_HEADER_LEN));

		// proc sadp packet
		parse_sadp_packet((char*)pbuf,(caplen-MAC_HEADER_LEN));
	}

	return 0;
}


unsigned int sadp_getseq()
{
	return g_sadpInst.seq++;
}

int sadp_login()
{
	int i;
	u_int32_t seq;
	u_int8_t code = SADP_ARQ_REQ_UAIP;
	struct timeval  now;
	struct timespec timeout;
	int retcode;

	for ( i = 0; i < 3; i++ ) 
	{
		seq = sadp_getseq();
		get_dev_info(&g_info);
		make_sadp_packet(SADPOPTYPE_ARQ,code,(char *)&g_info,sizeof(dev_info),seq);
		send_sadp_packet(&g_sadpInst);

		pthread_mutex_lock(&g_sadpInst.mutex);
		gettimeofday(&now,NULL);
		timeout.tv_sec = now.tv_sec + 3;
		timeout.tv_nsec = now.tv_usec * 1000;
		retcode = pthread_cond_timedwait(&g_sadpInst.cond,&g_sadpInst.mutex,&timeout);
		pthread_mutex_unlock(&g_sadpInst.mutex);

		if ( retcode == ETIMEDOUT )
		{
			//printf( " wait response timeout.\n");
		}

		//printf("login result=%d i=%d\n", g_sadpInst.login_result,i);

		if ( g_sadpInst.login_result == SADP_AQR_ACK_RRF )
			return -1;

		if ( g_sadpInst.login_result == SADP_AQR_ACK_AIP || g_sadpInst.login_result == SADP_AQR_ACK_UAIP )
		       return 0;	
	}

	return -1;
}


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


void str_to_mac(const char* addr_str, u_char* mac_addr)
{
	int i,index;
	int value,temp;
	u_char c;
    
	index = 0;
	value = 0;
	temp  = 0;

	for(i=0; i<strlen(addr_str);i++)
	{
		c = addr_str[i];
		if ( (c >='0' && c<='9') || (c>='a' && c<='f') || (c >= 'A' && c <='F') )
		{
			if ( c >= '0' && c <= '9' )
				temp = c-'0';
			if ( c >= 'a' && c <= 'f' )
				temp = c-'a' + 0xa;
			if ( c >= 'A' && c <= 'F' )
				temp = c-'A' + 0xa;

			if ( (index %2) == 1 )
			{
				value = value * 0x10 + temp;
				mac_addr[index/2] = value;
			}
			else
			{
				value = temp;
			}
			index++;
		}

		if ( index == 12 )
			break;
	}	
}

BOOL verify_mac_addr(u_char* addr)
{
	const char* broadcast = "ff:ff:ff:ff:ff:ff";
	u_char bc_mac_addr[MAC_ADDR_LEN];

	str_to_mac(broadcast, bc_mac_addr);

	if ( memcmp(addr,bc_mac_addr,MAC_ADDR_LEN) !=0 && memcmp(addr,g_sadpInst.src_mac,MAC_ADDR_LEN) != 0 )
		return FALSE; 

	return TRUE;
}

BOOL verify_password(const char* pw)
{
	unsigned char pw_our[16];

	sadp_MessageDigest( g_sadpInst.password, strlen(g_sadpInst.password), pw_our, 1 );

	if ( strncmp( pw_our, pw, PWD_LEN ) == 0 )
		return TRUE;

	return FALSE;
}

BOOL verify_sadp_packet(char* packet, int len)
{
	//printf("go into verify_sadp_packet\n");
	unsigned short check_sum;
	struct sadp_header* header = (struct sadp_header*)packet;

	//printf("");
	if ( header->sadp_identifier != SADP_IDENTIFIER )
		return FALSE;

	if ( header->sadp_len != len )
		return FALSE;

	check_sum = ntohs(header->sadp_crc);
	header->sadp_crc = 0;

	if ( check_sum != sadp_check_sum( (unsigned short*)packet,len ) )
	{
		//printf("leave verify_sadp_packet_false\n");
		return FALSE;
	
	}
		
	//printf("leave verify_sadp_packet_true\n");
	return 0;
}

int parse_sadp_packet(char* packet, int len)
{
	//printf("go into parse_sadp_packet\n");
	struct sadp_header* header;

	//if ( verify_sadp_packet(packet,len) == -1 )
	//	return -1;
	
	//printf("parse_sadp_packet_1\n");
	header = (struct sadp_header*)packet;

	switch ( header->sadp_optype )
	{
		case SADPOPTYPE_AQR:
			//printf("parse_sadp_packet_proc_op_AQR:%d\n", header->sadp_optype);
			proc_op_AQR(header, len);
			break;

		case SADPOPTYPE_AIQ:
			//printf("parse_sadp_packet_proc_op_AIQ:%d\n", header->sadp_optype);
			proc_op_AIQ(header);
			break;
				
		case SADPOPTYPE_UIR:
			//printf("parse_sadp_packet_proc_op_UIR:%d\n", header->sadp_optype);
			proc_op_UIR(header, len);
			break;

		case SADPOPTYPE_DFLTPASSWD_ARQ:
			//printf("parse_sadp_packet_proc_op_passwd:%d\n", header->sadp_optype);
			proc_op_passwd(header);
			break;
			
		default:
			//printf("parse_sadp_packet_default:%d\n", header->sadp_optype);
			break;
	}


	return 0;
}

void proc_op_AQR(struct sadp_header* header, int len)
{
	switch(header->sadp_opcode)
	{
		case SADP_AQR_ACK_RRF:
			break;

		case SADP_AQR_ACK_AIP:
			modify_ip_config(header, len);
			break;

		case SADP_AQR_ACK_UAIP:
			break;

		default:
			break;
	}

	g_sadpInst.login_result = header->sadp_opcode;

	pthread_mutex_lock(&g_sadpInst.mutex);
	pthread_cond_broadcast(&g_sadpInst.cond);
	pthread_mutex_unlock(&g_sadpInst.mutex);
}

void proc_op_AIQ(struct sadp_header* header)
{
	unsigned int seq;
	u_int8_t code = 0;

	seq = ntohl(header->sadp_sequence);
	get_dev_info(&g_info);
	make_sadp_packet(SADPOPTYPE_IQR,code,(char *)&g_info,sizeof(dev_info),seq);
	//make_sadp_packet(SADPOPTYPE_IQR,code,g_sadpInst.serialno,DEVICE_SERIALNO_LEN,seq);
	send_sadp_packet(&g_sadpInst);
}

void proc_op_UIR(struct sadp_header* header, int len)
{
	//printf("go into proc_op_UIR\n");
	unsigned int seq;
	int ret = -1;
	u_int8_t code;
	unsigned char password[PWD_LEN];

	seq = ntohl(header->sadp_sequence);

	memcpy(password,header->sadp_data,PWD_LEN);
	password[PWD_LEN-1] = '\0';
	//printf("go into proc_op_UIR_1\n");
	if ( verify_password(password) )
		code = SADP_UIA_ACK_UIS;
	else
		code = SADP_UIA_ACK_UIF;
	
	//printf("**********************************proc_op_UIR_code=%d\n", code);
	//code = SADP_UIA_ACK_UIS;
	if ( code == SADP_UIA_ACK_UIS )
	{
		//printf("go into proc_op_UIR_2\n");
		//ret = modify_ip_config(header, len);  //不修改IP
		//if(ret )
		{
			code = SADP_UIA_ACK_UIF;
		}		
	}
	//printf("go into proc_op_UIR_3\n");
	get_dev_info(&g_info);
	make_sadp_packet(SADPOPTYPE_UIA,code,(char *)&g_info,sizeof(dev_info),seq);

	//make_sadp_packet(SADPOPTYPE_UIA,code,g_sadpInst.serialno,DEVICE_SERIALNO_LEN,seq);

	send_sadp_packet(&g_sadpInst);

}

void proc_op_passwd(struct sadp_header* header)
{
	unsigned int seq;
	u_int8_t code;
	unsigned char password[PWD_LEN];

	seq = ntohl(header->sadp_sequence);
	 
	memcpy(password,header->sadp_data,PWD_LEN);
	if(g_sadpInst.reset_dflt_passwd!=NULL)
		code = g_sadpInst.reset_dflt_passwd(password);
	if(code ==0)
		code =SADP_DFLTPASSWD_ACK_OK;
	else
		code =SADP_DFLTPASSWD_ACK_FAILURE;
		
	//printf("********************[code: %d]\n", code);
	
	
	if ( verify_password(password) )
		code = SADP_UIA_ACK_UIS;
	else
		code = SADP_UIA_ACK_UIF;
	
	#if 0
	if ( code == SADP_UIA_ACK_UIS )
		modify_ip_config(header);
	#endif
	get_dev_info(&g_info);
	make_sadp_packet(SADPOPTYPE_DFLTPASSWD_ACK,code,(char *)&g_info,sizeof(dev_info),seq);

	//make_sadp_packet(SADPOPTYPE_UIA,code,g_sadpInst.serialno,DEVICE_SERIALNO_LEN,seq);

	send_sadp_packet(&g_sadpInst);

}

void make_sadp_packet(u_int8_t optype,u_int8_t opcode,char * content,int cont_len, unsigned int seq)
{
	struct sadp_header* header;
	u_int8_t* packet = (u_int8_t*)g_sadpInst.packet;
	u_int8_t total_len;
	unsigned int temp;

	total_len = SADP_HEADER_LEN + cont_len;

	if ( total_len < MIN_PACKET )
		total_len = MIN_PACKET;

	bzero( packet,total_len );

	header = (struct sadp_header*)packet;

	header->sadp_identifier = SADP_IDENTIFIER;
	header->sadp_version = SADP_VERSION;
	header->sadp_series  = SADP_SERIES;
	header->sadp_len = total_len;
	header->sadp_sequence = htonl(seq);
	header->sadp_hwaddrlen = SADP_HW_ADDR_LEN;
	header->sadp_praddrlen = SADP_PR_ADDR_LEN;
	header->sadp_optype = optype;
	header->sadp_opcode = opcode;
	header->sadp_crc = 0;
	header->sadp_src_praddr = htonl(g_sadpInst.src_ip);
	memcpy(header->sadp_src_hwaddr,g_sadpInst.src_mac, 6);
	memcpy(header->sadp_dst_hwaddr,g_sadpInst.dst_mac, 6);
	temp = htonl(g_sadpInst.dst_ip);
	memcpy(header->sadp_dst_praddr,&temp, 4);
	temp = htonl(g_sadpInst.mask);
	memcpy(header->sadp_subnet_mask,&temp,4);

	memcpy(header->sadp_data,content, cont_len);

	header->sadp_crc = htons( sadp_check_sum((unsigned short*)packet,total_len) );

	g_sadpInst.packet_s = total_len;
}


void send_sadp_packet(struct SADP* sadp_inst)
{
	u_int8_t buf[PACKET_SIZE];
	u_int8_t* temp = buf;
	u_int32_t len =  sadp_inst->packet_s + MAC_HEADER_LEN;
	u_int16_t type = htons(ETHERTYPE_SADP);

	memcpy(temp,sadp_inst->dst_mac,MAC_ADDR_LEN);
	temp += MAC_ADDR_LEN;

	memcpy(temp,sadp_inst->src_mac,MAC_ADDR_LEN);
	temp += MAC_ADDR_LEN;

	memcpy(temp,&type,sizeof(unsigned short));
	temp += sizeof(unsigned short);

	memcpy(temp, sadp_inst->packet, sadp_inst->packet_s);

	send_ether_packet(&sadp_inst->descr, buf,len );
}


int modify_ip_config(struct sadp_header* header, int len)
{
	unsigned int  mask = 0;
	unsigned int  ip = 0;
	unsigned int port = 0;
	unsigned int  gw = 0;
	
	int ret = -1;
	char temp[5] = "";	
	memcpy(&ip, header->sadp_dst_praddr,4);
	memcpy(&mask, header->sadp_subnet_mask,4);		
	memcpy(&port, header->sadp_data+PWD_LEN,4);
	//根据长度判断是否有V2.0数据
	if (len > header->sadp_len && len < 500)
	{
		memcpy(&gw, header->sadp_data+32, 4); //网关
	}
	else
	{
		//printf("V1.0_len=%d, header->sadp_len=%d\n", len, header->sadp_len);
	}
	ip = ntohl(ip);
	mask = ntohl(mask);
	port = ntohl(port);
	gw = ntohl(gw);
	
	if(g_sadpInst.fn_net_config)
	{
		ret = g_sadpInst.fn_net_config(ip ,mask ,gw, port);
		if(ret == 0)
		{
			g_sadpInst.src_ip = ip;
			g_sadpInst.mask = mask;
			g_sadpInst.port = port;
		}
	}
	return ret;
}

void get_dev_info(dev_info * info){
	
	memcpy(info->serial_no,g_sadpInst.serialno,DEVICE_SERIALNO_LEN);
	memcpy(info->dsp_software_version,g_sadpInst.dsp_software_version,SOFTWARE_VERSION_LEN);
	memcpy(info->software_version,g_sadpInst.software_version,SOFTWARE_VERSION_LEN);
	memcpy(info->start_time,g_sadpInst.start_time,SOFTWARE_VERSION_LEN);

	info->dev_type = htonl(g_sadpInst.dev_type);
	info->port = htonl(g_sadpInst.port);
	info->enc_cnt = htonl(g_sadpInst.enc_cnt);
	info->hdisk_cnt = htonl(g_sadpInst.hdisk_cnt); 
	return;

}

