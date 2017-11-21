/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : CPLD.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月19日
  最近修改   :
  功能描述   : CPLD.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __CPLD_H__
#define __CPLD_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include "Util.h"

#include <sys/ioctl.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void CPLD_IO_Init();
extern int DoorCheck();
extern int GetAllRedKeyStatus();
extern int GetAutoKeyStatus();
extern int GetFlashingKeyStatus();
extern int GetManualKeyStatus();
extern int GetStepByStepKeyStatus();
extern void HardflashDogCtrl(int value);
extern void Hiktsc_Running_Status(void);
extern int PedestrianCheck();
extern int ProcessKeyBoard();
extern void ProcessKeyBoardLight(void);
extern void SetAllRedLight(int value);
extern void SetAutoLight(int value);
extern void SetFlashingLight(int value);
extern void SetManualLight(int value);
extern void SetStepbyStepLight(int value);
extern void Set_LED2_OFF();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CPLD_H__ */
