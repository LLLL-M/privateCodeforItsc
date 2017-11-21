#ifndef __HIK_PACKET_H__
#define __HIK_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

struct lib_cap
{
	char* device;
	int ifindex;
	int fd;
	u_int8_t* buffer;
	u_int32_t buf_len;
};

int init_packet_capture(struct lib_cap* p);

// TS_UPGRADE
int init_packet_capture2(struct lib_cap* p, char *ifName);

int fini_packet_capture(struct lib_cap* p);

int get_capture_packet(struct lib_cap* p,u_int8_t** packet);

int send_ether_packet(struct lib_cap* p, u_int8_t* packet,u_int32_t len);

#ifdef __cplusplus
}
#endif

#endif



