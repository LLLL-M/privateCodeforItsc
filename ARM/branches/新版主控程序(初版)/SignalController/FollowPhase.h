/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : FollowPhase.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月19日
  最近修改   :
  功能描述   : FollowPhase.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月19日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __FOLLOWPHASE_H__
#define __FOLLOWPHASE_H__

#include "Util.h"

#define MAX_FOLLOW_PHASE_MOTHER_NUM 16


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    unsigned short nFollowPhaseID;//跟随号

    unsigned short nFixPhaseID;//修正相位

    unsigned short nGreenTime;//绿灯时间

    unsigned short nYellowTime;//黄灯时间

    unsigned short nRedTime;//红灯时间

    unsigned short nArrayMotherPhase[MAX_FOLLOW_PHASE_MOTHER_NUM];
    
}FollowPhaseItem,*PFollowPhaseItem;

typedef struct _FollowPhaseList{

    PFollowPhaseItem pItem;

    struct _FollowPhaseList *next;


}FollowPhaseList,*PFollowPhaseList;




extern int AddOrAlterFollowPhaseItem(PFollowPhaseList *pList,PFollowPhaseItem pItem);
extern int ClearFollowPhaseList(PFollowPhaseList *pList);
extern int DeleteFollowPhaseItem(PFollowPhaseList *pList,PFollowPhaseItem pItem);
extern int DestroyFollowPhaseList(PFollowPhaseList *pList);
extern int InitFollowPhaseList(PFollowPhaseList *pList);
extern int IsPhaseInFollowPhase(PFollowPhaseList pList,unsigned short nPhaseId,unsigned short nFollowPhaseId);
extern int LoadDefaultFollowPhase(PFollowPhaseList pList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FOLLOWPHASE_H__ */
