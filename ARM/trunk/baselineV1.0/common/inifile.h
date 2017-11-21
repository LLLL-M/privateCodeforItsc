/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : inifile.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年7月31日
  最近修改   :
  功能描述   : inifile.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年7月31日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __INIFILE_H__
#define __INIFILE_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "configureManagement.h"

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
extern void get_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom);
extern void get_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc);
extern void get_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config);
extern void set_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom);
extern void set_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc);
extern void set_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config);
extern void ReadMiscCfgFromIni(char *fileName,STRUCT_BINFILE_MISC *cfg);
extern void WriteMiscCfgToIni(char *fileName,STRUCT_BINFILE_MISC *cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INIFILE_H__ */
