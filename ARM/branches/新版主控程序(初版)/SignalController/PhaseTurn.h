/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : PhaseTurn.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月24日
  最近修改   :
  功能描述   : PhaseTurn.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月24日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __PHASETURN_H__
#define __PHASETURN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    unsigned short nPhaseTurnID;//相序表号
    unsigned short nCircleID;//环号
    unsigned short nTurnArray[16];//相序组

}PhaseTurnItem,*PPhaseTurnItem;

typedef struct _PhaseTurnList{

    PPhaseTurnItem pItem;

    struct _PhaseTurnList *next;

}PhaseTurnList,*PPhaseTurnList;




extern int AddOrAlterPhaseTurnItem(PPhaseTurnList *pList,PPhaseTurnItem pItem);
extern int ClearPhaseTurnList(PPhaseTurnList *pList);
extern int DeletePhaseTurnItem(PPhaseTurnList *pList,PPhaseTurnItem pItem);
extern int DestroyPhaseTurnList(PPhaseTurnList *pList);
extern int InitPhaseTurnList(PPhaseTurnList *pList);
extern int LoadDefaultPhaseTurnList(PPhaseTurnList pList);
extern int GetPhaseNum(PPhaseTurnList pList,unsigned short nPhaseTurnID);
extern unsigned short *GetPhaseArray(PPhaseTurnList pList,unsigned short nPhaseTurnID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PHASETURN_H__ */
