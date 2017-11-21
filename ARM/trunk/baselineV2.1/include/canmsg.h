#ifndef _CAN_INTERFACE_H
#define _CAN_INTERFACE_H

#include <sys/socket.h>
#include <linux/can.h>
#include "hik.h"

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LANE_NUM	48	//���֧�ֵĳ�������

//�����������
extern void DealVehPassData(UInt8 lane, Boolean isEnter);
//���泵����������
extern void SaveVehDectorData(struct can_frame *pframe);
//can��ʼ��
extern void CanInit(UInt32 bitrate);
//����can��Ϣ
extern void CanSend(struct can_frame *pframe);
//��ȡcan���׽����ļ�������
extern int GetCanSockfd();
//��ȡ��ѹ
extern UInt8 GetChannelVoltage(int channel);
//��ȡͨ����Ƶ���
extern UInt8 GetChannelRedCurrent(int channel);

#ifdef __cplusplus
}
#endif
 
#endif
