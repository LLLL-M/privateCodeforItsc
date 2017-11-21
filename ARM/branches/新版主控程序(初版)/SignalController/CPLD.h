/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : CPLD.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��19��
  ����޸�   :
  ��������   : CPLD.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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
