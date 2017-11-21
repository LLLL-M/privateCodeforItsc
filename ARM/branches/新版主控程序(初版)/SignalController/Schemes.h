/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Schemes.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : Schemes.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __SCHEMES_H__
#define __SCHEMES_H__


#include "Util.h"
#include "Action.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {

    unsigned short nSchemeID;//方案号
    unsigned short nCycleTime;//周期长
    unsigned short nOffset;//相位差
    unsigned short nGreenSignalRatioID;//绿信比号
    unsigned short nPhaseTurnID;//相序表号

}SchemeItem,*PSchemeItem;

typedef struct _SchemeList{

    PSchemeItem pItem;

    struct _SchemeList *next;


}SchemeList,*PSchemeList;


extern int AddOrAlterSchemetem(PSchemeList *pList,PSchemeItem pItem);
extern int ClearSchemeList(PSchemeList *pList);
extern int DeleteSchemeItem(PSchemeList *pList,PSchemeItem pItem);
extern int DestroySchemeList(PSchemeList *pList);
extern int GetGreenSignalRatioID(PSchemeList *pList,unsigned short nSchemeID);
extern int GetPhaseOrderID(PSchemeList *pList,unsigned short nSchemeID);
extern int InitShemeList(PSchemeList *pList);

extern int LoadDefaultSchemes(PSchemeList pList);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */
