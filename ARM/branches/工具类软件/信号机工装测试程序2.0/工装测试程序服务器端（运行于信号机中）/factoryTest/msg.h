#ifndef __MSG_H_
#define __MSG_H_


#define SA(x) ((struct sockaddr *)&x)
#define UDP_SOCK_PORT	34000

#define FTEST_UDP_HEAD	0x4c4c
#define FTEST_PRO_VER2  2

typedef enum {
   FTEST_MSG_NONE = 0x00,
   FTEST_MSG_CONNECT = 0xa1,
   FTEST_MSG_REQ_RECONNECT,
   FTEST_MSG_APP_EXIT,
   FTEST_MSG_USB = 0xb1,
   FTEST_MSG_RS232,
   FTEST_MSG_RS422,
   FTEST_MSG_RS485,
   FTEST_MSG_CURVOLT,
   FTEST_MSG_WIFI,
   FTEST_MSG_AUTO,
   FTEST_MSG_LAMP,
   FTEST_MSG_FRONTBOARD,
   FTEST_MSG_WIRELESS,
   FTEST_MSG_PEDKEY,
   FTEST_MSG_WIRELESS_DATA,
   FTEST_MSG_IO_OUTPUT_H,
   FTEST_MSG_IO_OUTPUT_L,
   FTEST_MSG_IO_INPUT,
   FTEST_MSG_IO_INPUT_DATA,
   FTEST_MSG_CAR_DETECTOR,
   FTEST_MSG_GPS,
   FTEST_MSG_KEYBOARD,
   FTEST_MSG_KEYBOARD_DATA,
   FTEST_MSG_TEST = 0xff
}UdpMsgtype;


#define MSG_EX_DEF		0
#define MSG_EX_SUCC		1
#define MSG_EX_FAIL		2


typedef struct udp_msg {
	unsigned int uVer;
	unsigned int uTscid;
	unsigned short uHead;
	unsigned short uType;
	int data[512 * 5];	//512*5*4=10k
}ST_UDP_MSG;




#endif
