/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : TimeInterval.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : TimeInterval.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __TIMEINTERVAL_H__
#define __TIMEINTERVAL_H__

#include "Util.h"
#include "Action.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {

    unsigned short nTimeIntervalID;//时段表号，1个时段链表有N个时段表，1个时段表可以有N个时段号

    unsigned short nTimeID;//时段号

    unsigned char cStartTimeHour;//开始时间  时
    unsigned char cStartTimeMinute;//开始时间 分
    
    unsigned short nActionID;//动作号

}TimeIntervalItem,*PTimeIntervalItem;


typedef struct _TimeIntervalList{

    PTimeIntervalItem pItem;

    struct _TimeIntervalList *next;

}TimeIntervalList,*PTimeIntervalList;


extern int AddOrAlterTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem);
extern int ClearTimeIntervalList(PTimeIntervalList *pList);
extern int DeleteTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem);
extern int DestroyTimeIntervalList(PTimeIntervalList *pList);
extern int GetActionID(PTimeIntervalList *pList,unsigned short nTimeIntervalID);
extern int InitTimeIntervalList(PTimeIntervalList *pList);

extern int LoadDefaultTimeInterval(PTimeIntervalList pList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TIMEINTERVAL_H__ */
