/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : BinaryTextConvert.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��12��29��
  ����޸�   :
  ��������   : BinaryTextConvert.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��12��29��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BINARYTEXTCONVERT_H__
#define __BINARYTEXTCONVERT_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "HikConfig.h"
#include "hik.h"
#include "configureManagement.h"
#include "gbconfig.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"                   //�ı��ļ���Ĭ��·��

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern UINT8 ConvertFailureLogToText();
extern UINT8 ConvertVehicleDataToText();
extern Boolean DoConvert(char *pFileName,UINT8 cSwitch,UINT8 cFileIndex);
extern char * DoConvertFromBin2Text(char *pFileName,UINT8 cFileIndex,void *dataContent);
extern void * DoConvertFromText2Bin(char *pFileName,UINT8 cFileIndex,void *pNewName);
extern void InitGlobalVal();
extern UINT8 IsArguLegal(char *p_fileName,INT8 argc,UINT8 *cSwitch);
extern INT8 IsCfgSupport(char *fileName);
extern int main(int argc,char **argv);
extern void PrintSupportCfg();
extern void PrintUsage();
extern INT8 GetCfgSupportCount();
extern INT32 GetCfgSize(int fileIndex);
extern void MakeCompatible(PSignalControllerPara para);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BINARYTEXTCONVERT_H__ */
