/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : TaskSchedule.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��3��
  ����޸�   :
  ��������   : TaskSchedule.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

}PlanTime;//�ƻ�ʱ��


typedef struct {

    unsigned short nScheduleID;//���ȼƻ���
    unsigned short nTimeIntervalID;//ʱ�α��
    PlanTime planTime;

}PlanScheduleItem,*PPlanScheduleItem;//����������



typedef struct _PlanSchedule{

    PlanScheduleItem *item;

    struct _PlanSchedule *next;

}PlanScheduleList,*PPlanScheduleList;//��������



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
