/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Net.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年8月22日
  最近修改   :
  功能描述   : Net.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年8月22日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __NET_H__
#define __NET_H__


#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <arpa/inet.h>
#include<netdb.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <net/if.h>

#include "hik.h"

#define IPPORT	8888

typedef enum {
	WEB_CONFIG,
	TOOL_CONFIG,
	CENTER_CONTROL,
} ConfigType;

typedef struct {
	ConfigType type;
	int size;
} ConfigHeader;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct _IPINFO
{
	char szHostName[64];
    char cIp[64];
	long uIP;
	long uNetMask;
	long NetGateway;
}IPINFO, *PIPINFO;

typedef struct STRU_Extra_Param_Response
{
	UInt32	unExtraParamHead;		//消息头标志
	UInt32	unExtraParamID;			//消息类型ID
	UInt32 	unExtraParamValue;		//块数据类型
	UInt32 	unExtraParamFirst;		//起始行
	UInt32 	unExtraParamTotal;		//总行数
	UInt32  unExtraDataLen;			//数据总长度
	UInt8	data[0];				//上载参数的数据结构体
}NETDATA;	//上载参数回复的结构体


extern int CreateSocket(int* sock,unsigned short ethNo,char* ip,int port,int SendOrRecv,int TcpOrUdp);
extern void GetLocalHost();
extern int SetPortReuse(int *sock, bool bReuse);
extern int SendDataNonBlock(int sock,void *data,int len,char *des,int port);
extern int GetSignalControlParams();
extern int SendSignalControlParams();
extern void *ThreadHeartBeats();



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NET_H__ */
