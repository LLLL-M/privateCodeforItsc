/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : LampController.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��18��
  ����޸�   :
  ��������   : LampController.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��18��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __LAMPCONTROLLER_H__
#define __LAMPCONTROLLER_H__

#include "Util.h"
#include "canmsg.h"
#include "CPLD.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum {

    Lamp_Green = 1,//001     �̵�
    Lamp_Yellow = 4,// 100  �Ƶ�
    Lamp_Red = 2,// 010     ���

    Lamp_Red_Light = 6,// red+4       ����
    Lamp_Yellow_Light = 8,// yellow+4    ����
    Lamp_Green_Light = 5,// green+4     ����
    
    Lamp_Off = 7//111          ָ���ĵ���
}LampColor;


extern void ControlLampLight(unsigned short nTotalControllerLampNum,unsigned short *nTotalControllerLampArray,
                        unsigned short nNum,unsigned short *nArray,LampColor type,short nLightTime);

extern void InitLampController();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LAMPCONTROLLER_H__ */
