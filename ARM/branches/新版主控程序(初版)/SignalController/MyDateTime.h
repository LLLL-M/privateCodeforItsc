/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : MyDateTime.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月3日
  最近修改   :
  功能描述   : MyDateTime.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __MYDATETIME_H__
#define __MYDATETIME_H__

#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {

    int sec;//seconds
    int min;//minutes
    int hour;//hour
    int day;//day of month
    int mon;
    int year;    
    int week;
}TimeStruct;



extern int DestroyDateTime();
extern int GetDay();
extern int GetHour();
extern int GetMin();
extern int GetMonth();
extern int GetSec();
extern int GetWeek();
extern int GetYear();
extern int InitDateTime();
extern void UpdateDateTime();

extern void ThreadUpdateDateTime();



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MYDATETIME_H__ */
