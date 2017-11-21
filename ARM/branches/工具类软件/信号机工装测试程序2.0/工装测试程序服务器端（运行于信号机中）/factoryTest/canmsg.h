#ifndef _CAN_INTERFACE_H
#define _CAN_INTERFACE_H

#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "libsocketcan.h"
//��Ƶ�������
#include<sys/un.h>
#include<unistd.h>
#include<stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
     

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

#define GET_BIT(v, bit) ((v >> bit) & 0x01)		//��
#define SET_BIT(v, bit) 	({(v) |= (1 << (bit));})	//��v���bit�

//�ƿذ�
void i_can_its_init_300();
void i_can_its_send_led_request_300(int boardNum, unsigned short *poutLamp);
void i_can_its_get_Volt_300(int boardNum, unsigned short *pboardInfo);
unsigned short i_can_its_get_cur_300(int boardNum, int pahseNum, int redGreen);
unsigned short getPedONStateFromCanLib();

//�����
unsigned short recv_date_from_vechile(int boardNum);

//�ڲ��ӿ�
void canits_set_bitrate_300(const char *name, unsigned long bitrate);
void canits_start_300(const char *name);
void canits_create_recv_thread_300();
void canits_create_socket_can_send_300();
//void canits_init();
void canits_send_300(struct can_frame *pcanfram);
void *canits_recv_thread_300(void *p);  //����CAN��Ϣ�߳�


//��Ƶ�������Խ�

//void video_detece_main_thread_create();
//int  video_detect_interface(void);
//void recv_from_camera(int *client_fd);

void turn_on_all_led();
void turn_off_all_led();
void wireless_init();
void KeyBoardInit(void);
void set_wireless_check_flag(char v);
void set_keyboard_check_flag(int v);
#pragma pack(push, 1)

#ifndef _TYPEDEF_
#define _TYPEDEF_
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long   UINT32;
typedef char 	INT8;
typedef short INT16;
typedef long INT32;
typedef unsigned int   DWORD; // 4, same as long
typedef unsigned short WORD;  // 2
typedef unsigned char  BYTE;  // 1
#endif

#define NAME_LEN				32				/* �û������� */
#define MAX_TPS_RULE                  8				/*������������Ŀ*/
#define SERIALNO_LEN        		48				/* ���кų��� */
#define LOW_4bits                         0                       /*����λ*/
#define TALL_4bits                         4                       /*����λ*/
#define BuffLen                              2048                  /*����buff����*/
#define IsNeedSend                         1                      /*��Ҫ��������*/      
#define NoNeedSend                        0                      /*����Ҫ����*/

typedef enum {
	MANUAL_PANEL_FLASH = 0x2C,						//??????
	MANUAL_PANEL_ALL_RED = 0x2F,					//??????
	MANUAL_PANEL_STEP = 0x30,						//??????
	MANUAL_PANEL_AUTO=0x4c,							//??????????
	MANUAL_PANEL_MANUAL=0x4d,						//??????????
	MANUAL_PANEL_EAST=0x4e,							//???????????
	MANUAL_PANEL_SOUTH=0x4f,						//???????????
	MANUAL_PANEL_WEST=0x50,							//???????????
	MANUAL_PANEL_NORTH=0x51,						//???????????
	MANUAL_PANEL_EASTWEST_DIRECT=0x52,				//????????????
	MANUAL_PANEL_SOUTHNORTH_DIRECT=0x53,			//????????????
	MANUAL_PANEL_EASTWEST_LEFT=0x54,				//????????????
	MANUAL_PANEL_SOUTHNORTH_LEFT=0x55,				//????????????
}Mannual_Key_Val;

typedef enum
{
	V_KEY_INVALID = 0,
	V_KEY_AUTO = 1,
	V_KEY_MANUAL = 2,
	V_KEY_YELLOWBLINK = 3,
	V_KEY_ALLRED = 4,
	V_KEY_STEP = 5,
	V_KEY_EAST = 6,
	V_KEY_SOUTH = 7,
	V_KEY_WEST = 8,
	V_KEY_NORTH = 9,
	V_D_EAST_WEST = 10,
	V_D_SOUTH_NORTH = 11,
	V_L_EAST_WEST = 12,
	V_L_SOUTH_NORTH = 13,
	V_KEY_MAX = 13
}eKeyStatus;

#ifdef __cplusplus
}
#endif
 
#endif
