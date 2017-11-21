/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : TimeInterval.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : TimeInterval.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

    unsigned short nTimeIntervalID;//ʱ�α�ţ�1��ʱ��������N��ʱ�α�1��ʱ�α������N��ʱ�κ�

    unsigned short nTimeID;//ʱ�κ�

    unsigned char cStartTimeHour;//��ʼʱ��  ʱ
    unsigned char cStartTimeMinute;//��ʼʱ�� ��
    
    unsigned short nActionID;//������

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
