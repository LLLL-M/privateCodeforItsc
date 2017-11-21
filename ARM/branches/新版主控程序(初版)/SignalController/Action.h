/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Action.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��4��
  ����޸�   :
  ��������   : Action.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��4��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

    unsigned short nActionID;//������
    unsigned short nSchemeID;//������
    
    unsigned char cAuxiliary[3];//��������1/2/3  ��0��ʾ��ѡ�У�1��ʾѡ�У�����˳��ʵ�ʹ���˳��
    unsigned char cGrayController;//�Ҷȿ���
    
    unsigned char cSpecialFun[8];//���⹦��1~8��,0��ʾ��ѡ�У�1��ʾѡ�У�����˳��ʵ�ʹ���˳��

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
