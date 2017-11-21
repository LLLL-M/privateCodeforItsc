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

#include "common.h"
     

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

//�ƿذ�
void i_can_its_init();
void i_can_its_send_led_request(int boardNum, unsigned short *poutLamp);
void i_can_its_get_Volt(int boardNum, unsigned short *pboardInfo);
unsigned short i_can_its_get_cur(int boardNum, int pahseNum, int redGreen);


//�����
unsigned short recv_date_from_vechile(int boardNum);

//�ڲ��ӿ�
void canits_set_bitrate(const char *name, unsigned long bitrate);
void canits_start(const char *name);
void canits_create_recv_thread();
void canits_create_socket_can_send();
void canits_init();
void canits_send(struct can_frame *pcanfram);
void *canits_recv_thread(void *p);  //����CAN��Ϣ�߳�


//��Ƶ�������Խ�

void video_detece_main_thread_create();
int  video_detect_interface(void);
void recv_from_camera(int *client_fd);

#pragma pack(push, 1)

typedef unsigned int   DWORD; // 4, same as long
typedef unsigned short WORD;  // 2
typedef unsigned char  BYTE;  // 1

#define NAME_LEN				32				/* �û������� */
#define MAX_TPS_RULE                  8				/*������������Ŀ*/
#define SERIALNO_LEN        		48				/* ���кų��� */
#define LOW_4bits                         0                       /*����λ*/
#define TALL_4bits                         4                       /*����λ*/
#define BuffLen                              2048                  /*����buff����*/
#define IsNeedSend                         1                      /*��Ҫ��������*/      
#define NoNeedSend                        0                      /*����Ҫ����*/
typedef struct
{		/* 24 bytes */
	struct in_addr	v4;							//< IPv4��ַ
	struct in6_addr	v6;							//< IPv6��ַ
	unsigned char	res[4];
}U_IN_ADDR;

typedef struct 
{
	DWORD dwLength;   						/*�����ܳ���*/
	BYTE    byRes1[2]; 						/* ���� */
	BYTE    byCommand;						 /* �������� 0x3a*/
	BYTE    byIPType;		 				/*IP���� 0-IPV4 1-IPV6 */
	DWORD dwVersion; 						/* �豸�汾��Ϣ */
	BYTE   sDVRName[NAME_LEN];     			/* �豸���� */
	BYTE   sSerialNumber[SERIALNO_LEN]; 		/* �豸���к� */
	U_IN_ADDR  struIPAddr;              			/*�豸IP��ַ*/ 
	WORD wPort; 							/* �豸�˿ں� */
	BYTE byMacAddr[6];	 					/* �豸MAC��ַ*/
	BYTE byRes3[8]; 							/* ���� */
}INTER_DVR_REQUEST_HEAD_V30;

typedef struct _TPS_NET_VCA_POINT_
{	
 	UINT16 wX;								/*(0.000-1)*1000 X������ */
 	UINT16 wY;								/*(0.000-1)*1000 Y������ */
} TPS_NET_VCA_POINT;

typedef struct _NET_LANE_QUEUE_
{
    	TPS_NET_VCA_POINT   struHead;			/*����ͷ*/
    	TPS_NET_VCA_POINT   struTail;				/*����β*/
    	UINT32          dwlength;					/*ʵ�ʶ��г��� ��λΪ�� ������*1000*/
}NET_LANE_QUEUE; 

typedef enum tagTRAFFIC_DATA_VARY_TYPE
{    
     ENUM_TRAFFIC_VARY_NO               	= 0x00,   //�ޱ仯
     ENUM_TRAFFIC_VARY_VEHICLE_ENTER  = 0x01,   //��������������Ȧ
     ENUM_TRAFFIC_VARY_VEHICLE_LEAVE  = 0x02,   //�����뿪������Ȧ
     ENUM_TRAFFIC_VARY_QUEUE           	= 0x04,   //���б仯
     ENUM_TRAFFIC_VARY_STATISTIC        	= 0x08   //ͳ�����ݱ仯��ÿ���ӱ仯һ�ΰ���ƽ���ٶȣ������ռ�/ʱ��ռ���ʣ���ͨ״̬��        
} TRAFFIC_DATA_VARY_TYPE;  

typedef struct _NET_LANE_PARAM
{
    	UINT8  byRuleName[NAME_LEN];  		/*������������ */
    	UINT8  byRuleID;                 			/*������ţ�Ϊ�������ýṹ�±꣬0-7 */
	UINT8  byLaneType;			      		/*�������л�����*/
	UINT8  byTrafficState;					/*������ͨ״̬*/
	UINT8  byRes1;				        	/* ����*/
	UINT32 dwVaryType;					/* ������ͨ�����仯���� ����   TRAFFIC_DATA_VARY_TYPE */
	UINT32 dwTpsType;					/* ���ݱ仯���ͱ�־����ʾ��ǰ�ϴ���ͳ�Ʋ����У���Щ������Ч����ֵΪITS_TPS_TYPE���������*/
    	UINT32 dwLaneVolume;	                   /* �������� ��ͳ���ж��ٳ���ͨ��*/
    	UINT32 dwLaneVelocity;         			/*�����ٶȣ��������	������*1000*/
   	UINT32 dwTimeHeadway ;       			/*��ͷʱ�࣬�������	������*1000*/
    	UINT32 dwSpaceHeadway;       		/*��ͷ��࣬���������㸡����*1000*/
    	UINT32 dwSpaceOccupyRation;   		/*����ռ���ʣ��ٷֱȼ��㣨�ռ���) ������*1000*/
	UINT32 dwTimeOccupyRation;          	/*ʱ��ռ����*/
	UINT32 dwLightVehicle;       			/* С�ͳ�����*/
	UINT32 dwMidVehicle;        			/* ���ͳ�����*/
	UINT32 dwHeavyVehicle;      			/* ���ͳ�����*/
    	NET_LANE_QUEUE struLaneQueue;       	/*�������г���*/
    	TPS_NET_VCA_POINT  struRuleLocation;	/*����λ��*/
    	UINT8    byRes2[64];
}NET_LANE_PARAM;

//��ͨͳ�Ʋ�����Ϣ�ṹ��
typedef struct _NET_TPS_INFO_
{
    	UINT32   dwLanNum;   				/* ��ͨ�����ĳ�����Ŀ*/
    	NET_LANE_PARAM struLaneParam[MAX_TPS_RULE];
	UINT8     byRes[32];    				/*����*/
}NET_TPS_INFO;

typedef struct _NET_VCA_DEV_INFO_
{
	U_IN_ADDR  struDevIP;				/*ǰ���豸��ַ*/ 
	UINT16 wPort; 						/*ǰ���豸�˿ں�*/ 
	UINT8 byChannel;  					/*ǰ���豸ͨ��*/
	UINT8 byIvmsChannel;				/*��ΪIVMSͨ����*/ 
} NET_VCA_DEV_INFO;

typedef struct _NET_TPS_ALARM_
{
    UINT32 dwSize;						/* �ṹ���С*/
    UINT32 dwRelativeTime;				/* ���ʱ��*/
    UINT32 dwAbsTime;					/* ����ʱ��*/
    NET_VCA_DEV_INFO struDevInfo;			/* ǰ���豸��Ϣ*/
    NET_TPS_INFO  struTPSInfo;				/* ��ͨ�¼���Ϣ*/
    //UINT8  byRes1[128];					/* �����ֽ�*/
    UINT32 dwDeviceId; /*�豸ID*/ 
    UINT8 byRes1[124]; /* �����ֽ�*/ 
}NET_TPS_ALARM;

//IS_NEED_DATA�ṹ����Ϊ�źŻ���Ҫ����Ϣ
typedef struct _IS_NEED_DATA_
{
    UINT32 varyType;						/* ��������*/
    UINT32 laneSpeed;						/*�����ٶ�*/
    UINT8  ruleID;							/* �������*/
    UINT32 cameraID;   					 /*�豸ID*/ 

}IS_NEED_DATA;

#pragma pack(pop)

void  deal_data_from_video(NET_TPS_ALARM  alarmData);
#ifdef __cplusplus
}
#endif
 
#endif
