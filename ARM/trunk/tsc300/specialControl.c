/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : specialControl.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年3月30日
  最近修改   :
  功能描述   : 主要是封装了发送黄闪、全红、关灯等特殊控制的信息发送接口
  函数列表   :
              SendUdpMsg
  修改历史   :
  1.日    期   : 2015年3月30日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "hik.h"
#include "specialControl.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
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



// 0：系统控制，255：黄闪，254：感应，252：全红，251：关灯，其它：手动方案
void SendSpecialCtrolUdpMsg(unsigned char cControlType)
{
	//创建UDP服务器
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
	//使用161端口
    localAddr.sin_port = htons (161);
	//向本机161端口发送黄闪信号
	int len = sendto(socketFd,pMsg,cMsgLen,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		ERR("Send flashing signal failed!!!\n");
	}

	ERR("SendUdpMsg  succeed  ,cControlType  :  %d\n",cControlType);
	close(socketFd);
}





