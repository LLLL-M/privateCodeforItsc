/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : specialControl.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��3��30��
  ����޸�   :
  ��������   : ��Ҫ�Ƿ�װ�˷��ͻ�����ȫ�졢�صƵ�������Ƶ���Ϣ���ͽӿ�
  �����б�   :
              SendUdpMsg
  �޸���ʷ   :
  1.��    ��   : 2015��3��30��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "hik.h"
#include "specialControl.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

static unsigned char cArrayMsgYellowBlink[48] = {0x30, 0x2E, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
                                    0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x21, 0x02, 
                                    0x02, 0x00, 0x82, 0x02, 0x01, 0x00, 0x02, 0x01, 
                                    0x00, 0x30, 0x15, 0x30, 0x13, 0x06, 0x0D, 0x2B, 
                                    0x06, 0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 
                                    0x01, 0x04, 0x0E, 0x00, 0x02, 0x02, 0x00, 0xFF};


static unsigned char cArrayMsgTurnOff[47] = {0x30, 0x2D, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
                                    0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x20, 0x02, 
                                    0x01, 0x6B, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,     
                                    0x30, 0x15, 0x30, 0x13, 0x06, 0x0D, 0x2B, 0x06, 
                                    0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 0x01, 
                                    0x04, 0x0E, 0x00, 0x02, 0x02, 0x00, 0xFB};

static unsigned char cArrayMsgAllRed[48] = {0x30, 0x2E, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
                                    0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x21, 0x02, 
                                    0x02, 0x00, 0xAB, 0x02, 0x01, 0x00, 0x02, 0x01, 
                                    0x00, 0x30, 0x15, 0x30, 0x13, 0x06, 0x0D, 0x2B, 
                                    0x06, 0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 
                                    0x01, 0x04, 0x0E, 0x00, 0x02, 0x02, 0x00, 0xFC};

static unsigned char cArrayMsgInduction[48] = {0x30, 0x2E, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
                                    0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x21, 0x02, 
                                    0x02, 0x00, 0x8A, 0x02, 0x01, 0x00, 0x02, 0x01, 
                                    0x00, 0x30, 0x15, 0x30, 0x13, 0x06, 0x0D, 0x2B, 
                                    0x06, 0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 
                                    0x01, 0x04, 0x0E, 0x00, 0x02, 0x02, 0x00, 0xFE};

static unsigned char cArrayMsgSystem[47] = {0x30, 0x2C, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
        							  0x75, 0x62, 0x6C, 0x69, 0x63, 0xA3, 0x1F, 0x02,
        							  0x01, 0x6E, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
        							  0x30, 0x14, 0x30, 0x12, 0x06, 0x0D, 0x2B, 0x06,
        							  0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02, 0x01,
        							  0x04, 0x0E, 0x00, 0x02, 0x02, 0x01, 0x00};

static unsigned char cArrayMsgManual[47] = {0x30, 0x2D, 0x02, 0x01, 0x00, 0x04, 0x06, 0x70,
        							  0x75, 0x62, 0x6C, 0x69, 0x63, 0xA2, 0x20, 0x02,
        							  0x02, 0x00, 0xCB, 0x02, 0x01, 0x00, 0x02, 0x01,
        							  0x00, 0x30, 0x14, 0x30, 0x12, 0x06, 0x0D, 0x2B,
        							  0x06, 0x01, 0x04, 0x01, 0x89, 0x36, 0x04, 0x02,
        							  0x01, 0x04, 0x0E, 0x00, 0x02, 0x01, 0x01};



// 0��ϵͳ���ƣ�255��������254����Ӧ��252��ȫ�죬251���صƣ��������ֶ�����
void SendSpecialCtrolUdpMsg(unsigned char cControlType)
{
	//����UDP������
	int socketFd = -1;
	unsigned char *pMsg = NULL;
    struct sockaddr_in localAddr;
    unsigned char cMsgLen = 0;

    switch(cControlType)
    {
        case SPECIAL_CONTROL_SYSTEM:         pMsg = cArrayMsgSystem;         cMsgLen = sizeof(cArrayMsgSystem);          break;
        case SPECIAL_CONTROL_YELLOW_BLINK:   pMsg = cArrayMsgYellowBlink;    cMsgLen = sizeof(cArrayMsgYellowBlink);     break;
        case SPECIAL_CONTROL_INDUCTION:      pMsg = cArrayMsgInduction;      cMsgLen = sizeof(cArrayMsgInduction);       break;
        case SPECIAL_CONTROL_TURN_OFF:       pMsg = cArrayMsgTurnOff;        cMsgLen = sizeof(cArrayMsgTurnOff);         break;
        case SPECIAL_CONTROL_ALL_RED:        pMsg = cArrayMsgAllRed;         cMsgLen = sizeof(cArrayMsgAllRed);          break;
        default:                             cArrayMsgManual[46]=cControlType;  pMsg = cArrayMsgManual;                  cMsgLen = sizeof(cArrayMsgManual);          break;
    }

	memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = sizeof(localAddr);

    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		ERR("socket udp init error!!!\n");
       	return;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//ʹ��161�˿�
    localAddr.sin_port = htons (161);
	//�򱾻�161�˿ڷ��ͻ����ź�
	int len = sendto(socketFd,pMsg,cMsgLen,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		ERR("Send flashing signal failed!!!\n");
	}

	ERR("SendUdpMsg  succeed  ,cControlType  :  %d\n",cControlType);
	close(socketFd);
}





