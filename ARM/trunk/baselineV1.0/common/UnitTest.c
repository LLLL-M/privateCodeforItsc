#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"

/*****************************************************************************
 �� �� ��  : ThreadTestUdpSeverMsgCoundown
 ��������  : ��Ҫ����ģ��ƽ̨��ÿ������20000�˿����󵹼�ʱ��Ϣ���жϵ���ʱ��
             Ϣ�Ƿ�����
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��15��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void  *TestUdpSeverMsgCoundown()
{
    int cSocket = 0;
    STRU_UDP_INFO sMsg;
    struct sockaddr_in sToAddr;

    cSocket = socket(AF_INET,SOCK_DGRAM,0);

    memset(&sToAddr, 0, sizeof(sToAddr));
    sToAddr.sin_family = AF_INET;
    sToAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sToAddr.sin_port = htons (20000);

    sMsg.iHead = 0x6e6e;
    sMsg.iType = 0x9e;

    printf("TestUdpSeverMsgCoundown start ....\n");

    while(1)
    {
        if(-1 == sendto(cSocket,(void *)&sMsg,sizeof(sMsg),0,(struct sockaddr *)&sToAddr,sizeof(sToAddr)))
        {
            printf("ThreadTestUdpSeverMsgCoundown  error to send msg \n");
        }

        sleep(1);
    }

    return NULL;
}

int ThreadTestCountdown()
{
	int result = 0;
	pthread_t thread_id;
	result = pthread_create(&thread_id,NULL,(void *) TestUdpSeverMsgCoundown,NULL);	
	if(result != 0 )
	{
		printf("ThreadTestCountdown  error!\n");
		return 0;
	}
	return 1;

}










