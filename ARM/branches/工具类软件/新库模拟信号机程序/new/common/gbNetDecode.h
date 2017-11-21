/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : GBNetDecode.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年6月29日
  最近修改   :
  功能描述   : GBNetDecode.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年6月29日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __GBNETDECODE_H__
#define __GBNETDECODE_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hik.h"
#include "gb.h"




#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern int GBNetDataDecode(int socketFd,struct sockaddr_in toAddr,unsigned char *cNetData);
extern void SendErrorMsg(int socketFd,struct sockaddr_in toAddr,UInt8 MsgTypeField);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GBNETDECODE_H__ */
