/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : ThreadController.h
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月26日
  最近修改   :
  功能描述   : ThreadController.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#ifndef __THREADCONTROLLER_H__
#define __THREADCONTROLLER_H__

#include "Util.h"
#include "MyLinkList.h"
#include <signal.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int AddThreadControllerItem(int item);
extern int DestroyThreadControllerList();
extern int InitThreadControllerList();
extern int MainThreadControllerFun();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __THREADCONTROLLER_H__ */
