/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Action.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月4日
  最近修改   :
  功能描述   : Action.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月4日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __ACTION_H__
#define __ACTION_H__

#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    unsigned short nActionID;//动作号
    unsigned short nSchemeID;//方案号
    
    unsigned char cAuxiliary[3];//辅助功能1/2/3  ，0表示不选中，1表示选中，数组顺序即实际功能顺序
    unsigned char cGrayController;//灰度控制
    
    unsigned char cSpecialFun[8];//特殊功能1~8，,0表示不选中，1表示选中，数组顺序即实际功能顺序

}ActionItem,*PActionItem;


typedef struct _ActionList{

    PActionItem pItem;

    struct _ActionList *next;

}ActionList,*PActionList;






extern int AddOrAlterActionItem(PActionList *pList,PActionItem pItem);
extern int ClearActionList(PActionList *pList);
extern int DeleteActionItem(PActionList *pList,PActionItem pItem);
extern int DestroyActionList(PActionList *pList);
extern int GetSchemeID(PActionList *pList,unsigned short nActionID);
extern int InitActionList(PActionList *pList);

extern int LoadDefaultAction(PActionList pList);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ACTION_H__ */
