/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : HikConfig.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : HikConfig.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月3日
    作    者   : 肖文虎
    修改内容   : 按照章文超给的头文件进行修改
******************************************************************************/

#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "gb.h"
#include "parse_ini.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//计划时间


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HIKCONFIG_LIB_VERSION	"1.1.0"

#define CFG_NAME "/home/hikconfig.dat"
#define CFG_BAK_NAME "/home/hikconfig.bak"

#define BIT(v, n) (((v) >> (n)) & 0x1)		//取v的第 n bit位

//相位是否使能，传入参数为相位选项，若使能了，则返回值为1，否则返回值为0
#define IS_PHASE_INABLE(option)     ((((option)&&0x01) == 1 ) ? 1 : 0)

/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/

 
extern Boolean LoadGbDataFromCfg(GbConfig *pSignalControlpara, const char *path);

/*Write*/
extern Boolean WriteGbConfigFile(GbConfig *param, const char *path);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKCONFIG_H__ */
