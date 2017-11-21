/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : binfile.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年8月3日
  最近修改   :
  功能描述   : binfile.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年8月3日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BINFILE_H__
#define __BINFILE_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "hik.h"

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
extern UINT32 WriteBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize);
extern UINT32 ReadBackupBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize, UINT32 nOffset);
extern UINT32 ReadBinCfgInfo(char *pFileName, void *pBuf, UINT32 nBufSize, UINT32 nOffset, UINT8 nIsReadBackup);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BINFILE_H__ */
