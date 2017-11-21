/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : FollowPhase.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��19��
  ����޸�   :
  ��������   : FollowPhase.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��19��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

    unsigned short nFollowPhaseID;//�����

    unsigned short nFixPhaseID;//������λ

    unsigned short nGreenTime;//�̵�ʱ��

    unsigned short nYellowTime;//�Ƶ�ʱ��

    unsigned short nRedTime;//���ʱ��

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
