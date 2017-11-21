/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : LampController.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月18日
  最近修改   :
  功能描述   : LampController.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月18日
    作    者   : xiaowh
    修改内容   : 创建文件

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

    Lamp_Green = 1,//001     绿灯
    Lamp_Yellow = 4,// 100  黄灯
    Lamp_Red = 2,// 010     红灯

    Lamp_Red_Light = 6,// red+4       红闪
    Lamp_Yellow_Light = 8,// yellow+4    黄闪
    Lamp_Green_Light = 5,// green+4     绿闪
    
    Lamp_Off = 7//111          指定的灯灭
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
