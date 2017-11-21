/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : BinaryTextConvert.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年12月29日
  最近修改   :
  功能描述   : BinaryTextConvert.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年12月29日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BINARYTEXTCONVERT_H__
#define __BINARYTEXTCONVERT_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 宏定义                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"                   //文本文件的默认路径

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 接口函数                                    *
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
