#include <sys/types.h>         
#include <sys/socket.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "gb.h"
#include "GBNetDecode.h"

//�������Э���UDP�˿�Ϊ30000
#define GB_UDP_PORT	30000
//�������Э���TCP�˿�Ϊ40000
#define GB_TCP_PORT 40000
#define GB_PACKAGE_MAX_LEN 484

int gbClientFd;
struct sockaddr_in gbClientAddr;

extern GbConfig *gGbconfig;
extern UInt8 nIsSendErrorMsg;

extern void GbConvertToNtcip();

extern int gGBMsgLen;
void *GbUDPCommunicationModule(void *arg)
{
	//����UDP������
	int socketFd = -1;
	int opt = 1; 
    struct sockaddr_in localAddr;
	struct sockaddr_in fromAddr;
	//int nObjectIdentify = 0;//�ϲ���������Ķ����ʶ
    int nIsVerifyOK = 0;//�������ò����Ƿ�У��ɹ���0��ʾ�ɹ���1��ʾʧ�ܡ�
    
	unsigned char cRecvPackage[GB_PACKAGE_MAX_LEN+100];
	unsigned int nRecvSize = 0;
	memset((char *)&localAddr, 0, (int)sizeof(localAddr));
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);

    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	return NULL;
    }
	
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons (GB_UDP_PORT);

	//���ö˿ڸ��� 
 	setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));  

    int bindResult = bind(socketFd, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if ( -1 == bindResult )
    {
		printf("bind %d port error!!!\n",GB_UDP_PORT);
        close(socketFd);
        return NULL;
    }

	while(1)
	{
		memset(cRecvPackage,0,sizeof(cRecvPackage));
		nRecvSize = recvfrom(socketFd, cRecvPackage, sizeof(cRecvPackage), 0, (struct sockaddr *)&fromAddr, &fromLen);	
		if(nRecvSize == -1)
		{
			continue;	
		}
        if(nRecvSize > GB_PACKAGE_MAX_LEN)
		{
            SetErrorMsg(MSG_LENGTH_TOO_LONG,0);
			nIsSendErrorMsg = 1;
            SendErrorMsg(socketFd,fromAddr,cRecvPackage[0]);
            continue;
		}
        gGBMsgLen = nRecvSize;
		gbClientFd = socketFd;
		gbClientAddr = fromAddr;
		
		nIsVerifyOK = GBNetDataDecode(socketFd,fromAddr,cRecvPackage);
        if( 0 == nIsVerifyOK)
        {
            //���������������⣬Ӧ�÷��ʹ�������Ϣ��
            SendErrorMsg(socketFd,fromAddr,cRecvPackage[0]);
            //���ݻָ�
            memmove(gGbconfig,gGbconfig+1,offsetof(GbConfig, eventTypeTable));
            continue;
        }
        //����ȫ�ֱ���
        GbConvertToNtcip();
	}
}

#if 0
void *GbTCPCommunicationModule(void *arg)
{
	//����TCP������
	int socketFd = -1;
	int opt = 1; 
	int sockConnected = 0;
    struct sockaddr_in localAddr;
	struct sockaddr_in fromAddr;
	//int nObjectIdentify = 0;//�ϲ���������Ķ����ʶ
    int nIsVerifyOK = 0;//�������ò����Ƿ�У��ɹ���0��ʾ�ɹ���1��ʾʧ�ܡ�
    
	unsigned char cRecvPackage[GB_PACKAGE_MAX_LEN+100];
	unsigned int nRecvSize = 0;
	memset((char *)&localAddr, 0, (int)sizeof(localAddr));
    memset((char *)&fromAddr, 0, (int)sizeof(fromAddr));
    socklen_t fromLen = sizeof(fromAddr);

    socketFd = socket (AF_INET, SOCK_STREAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket tcp init error!!!\n");
       	return NULL;
    }
	
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons (GB_TCP_PORT);

	//���ö˿ڸ��� 
 	setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));  

    int bindResult = bind(socketFd, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if ( -1 == bindResult )
    {
		printf("bind %d port error!!!\n",GB_UDP_PORT);
        close(socketFd);
        return NULL;
    }
	//����
	listen(socketFd,1);//ͬʱֻ����1���ͻ�������
	
	while(1)
	{
		sockConnected = accept(socketFd,(struct sockaddr *) &fromAddr,&fromLen);
		
		while(1)
		{
			memset(cRecvPackage,0,sizeof(cRecvPackage));
			nRecvSize = recv(sockConnected,cRecvPackage, sizeof(cRecvPackage), 0);		
			if(nRecvSize <= 0)
			{
				ERR("Client Exit .\n");
				close(sockConnected);
				break;	
			}
			if(nRecvSize > GB_PACKAGE_MAX_LEN)
			{
				SetErrorMsg(MSG_LENGTH_TOO_LONG,0);
				continue;
			}
			gGBMsgLen = nRecvSize;
			//ERR("size %d  ,  %x  %x %x \n",nRecvSize,cRecvPackage[0],cRecvPackage[1],cRecvPackage[2]);
			nIsVerifyOK = GBNetDataDecode(sockConnected,fromAddr,cRecvPackage);
			if(0 == nIsVerifyOK)
			{
				//���������������⣬Ӧ�÷��ʹ�������Ϣ��
				SendErrorMsg(sockConnected,fromAddr,cRecvPackage[0]);
				//���ݻָ�
				memmove(gGbconfig,gGbconfig+1,sizeof(GbConfig));
				continue;
			}
			//����ȫ�ֱ���
			GbConvertToNtcip();		
		}
		
		//ERR("Next ,.,,\n");

	}
}
#endif


















