/******************************************************************************

                  \xB0\xE6\x{0228}\xCB\xF9\xD3\xD0 (C), 2001-2014, HikVision

 ******************************************************************************
  \xCE\xC4 \xBC\xFE \xC3\xFB   : CPLD.h
  \xB0\xE6 \xB1\xBE \xBA\xC5   : \xB3\xF5\xB8\xE5
  \xD7\xF7    \xD5\xDF   : xiaowh
  \xC9\xFA\xB3\xC9\xC8\xD5\xC6\xDA   : 2014\xC4\xEA7\xD4\xC219\xC8\xD5
  \xD7\xEE\xBD\xFC\xD0\x{07b8}\xC4   :
  \xB9\xA6\xC4\xDC\xC3\xE8\xCA\xF6   : CPLD.c \xB5\xC4\x{0377}\xCE\x{013c}\xFE
  \xBA\xAF\xCA\xFD\xC1\x{0431}\xED   :
  \xD0\x{07b8}\xC4\xC0\xFA\x{02b7}   :
  1.\xC8\xD5    \xC6\xDA   : 2014\xC4\xEA7\xD4\xC219\xC8\xD5
    \xD7\xF7    \xD5\xDF   : xiaowh
    \xD0\x{07b8}\xC4\xC4\xDA\xC8\xDD   : \xB4\xB4\xBD\xA8\xCE\x{013c}\xFE

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
