/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : MyDateTime.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��3��
  ����޸�   :
  ��������   : MyDateTime.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __MYDATETIME_H__
#define __MYDATETIME_H__

#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {

    int sec;//seconds
    int min;//minutes
    int hour;//hour
    int day;//day of month
    int mon;
    int year;    
    int week;
}TimeStruct;



extern int DestroyDateTime();
extern int GetDay();
extern int GetHour();
extern int GetMin();
extern int GetMonth();
extern int GetSec();
extern int GetWeek();
extern int GetYear();
extern int InitDateTime();
extern void UpdateDateTime();

extern void ThreadUpdateDateTime();



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MYDATETIME_H__ */
