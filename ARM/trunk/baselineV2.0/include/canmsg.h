#ifndef _CAN_INTERFACE_H
#define _CAN_INTERFACE_H

#include <sys/socket.h>
#include <linux/can.h>
#include "hik.h"

//#define AF_CAN		29	/* Controller Area Network      *
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LANE_NUM	48	//最大支持的车道个数

//处理过车数据
extern void DealVehPassData(UInt8 lane, Boolean isEnter);
//保存车检板过车数据
extern void SaveVehDectorData(struct can_frame *pframe);
//can初始化
extern void CanInit(UInt32 bitrate);
//发送can消息
extern void CanSend(struct can_frame *pframe);
//获取can的套接字文件描述符
extern int GetCanSockfd();
//获取电压
extern UInt8 GetChannelVoltage(int channel);
//获取通道红灯电流
extern UInt8 GetChannelRedCurrent(int channel);

#ifdef __cplusplus
}
#endif
 
#endif
