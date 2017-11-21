/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : TaskSchedule.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月3日
  最近修改   :
  功能描述   : TaskSchedule.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __TASKSCHEDULE_H__
#define __TASKSCHEDULE_H__

#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    int month[12];
    int day[31];
    int week[7];

}PlanTime;//计划时间


typedef struct {

    unsigned short nScheduleID;//调度计划号
    unsigned short nTimeIntervalID;//时段表号
    PlanTime planTime;

}PlanScheduleItem,*PPlanScheduleItem;//单个调度项



typedef struct _PlanSchedule{

    PlanScheduleItem *item;

    struct _PlanSchedule *next;

}PlanScheduleList,*PPlanScheduleList;//调度链表



extern int AddOrAlterTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item);
extern int ClearTaskSchedule(PPlanScheduleList *pList);
extern int DeleteTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item);
extern int DestroyTaskSchedule(PPlanScheduleList *pList);
extern int GetTimeIntervalIndex(PPlanScheduleList *pList);
extern int InitTaskSchedule(PPlanScheduleList *pList);
extern int LoadDefaultTaskSchedule(PPlanScheduleList pList);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TASKSCHEDULE_H__ */
