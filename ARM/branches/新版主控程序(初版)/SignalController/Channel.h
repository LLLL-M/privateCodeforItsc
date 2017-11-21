/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Channels.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : Channels.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __CHANNEL_H__
#define __CHANNEL_H__


#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum {

    OTHER = 0,//其他类型
    MOTOR,//机动车类型
    PEDESTRIAN,//行人类型
    FOLLOW//跟随类型
    
}ControllerType;//控制源的类型


typedef enum {

    ALTERNATE = 0,//交替
    REDLIGHT,//红闪
    YELLOWLIGHT//黄闪

}FlashLightType;//闪光模式



typedef struct {

    unsigned short nChannelID;//通道号
    unsigned short nControllerID;//控制源号
    ControllerType nControllerType;//控制源类型
    FlashLightType nFlashLightType;//闪光模式

}ChannelItem,*PChannelItem;

typedef struct _ChannelList{

    PChannelItem pItem;

    struct _ChannelList *next;


}ChannelList,*PChannelList;



extern int AddOrAlterChanneltem(PChannelList *pList,PChannelItem pItem);
extern int ClearChannelList(PChannelList *pList);
extern int DeleteChannelItem(PChannelList *pList,PChannelItem pItem);
extern int DestroyChannelList(PChannelList *pList);
extern int InitChannelList(PChannelList *pList);
extern int LoadDefaultChannel(PChannelList pList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */
