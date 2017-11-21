/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : binfile.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��8��3��
  ����޸�   :
  ��������   : binfile.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��8��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BINFILE_H__
#define __BINFILE_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "hik.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern UInt32 WriteBinCfgInfo(char *pFileName, void *pBuf, UInt32 nBufSize);
extern UInt32 ReadBackupBinCfgInfo(char *pFileName, void *pBuf, UInt32 nBufSize, UInt32 nOffset);
extern UInt32 ReadBinCfgInfo(char *pFileName, void *pBuf, UInt32 nBufSize, UInt32 nOffset, UInt8 nIsReadBackup);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BINFILE_H__ */