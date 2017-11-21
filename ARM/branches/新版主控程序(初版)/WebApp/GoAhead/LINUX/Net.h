/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Net.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��8��22��
  ����޸�   :
  ��������   : Net.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��8��22��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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
	UInt32	unExtraParamHead;		//��Ϣͷ��־
	UInt32	unExtraParamID;			//��Ϣ����ID
	UInt32 	unExtraParamValue;		//����������
	UInt32 	unExtraParamFirst;		//��ʼ��
	UInt32 	unExtraParamTotal;		//������
	UInt32  unExtraDataLen;			//�����ܳ���
	UInt8	data[0];				//���ز��������ݽṹ��
}NETDATA;	//���ز����ظ��Ľṹ��


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
