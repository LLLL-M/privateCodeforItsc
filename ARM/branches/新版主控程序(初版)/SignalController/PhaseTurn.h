/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : PhaseTurn.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��24��
  ����޸�   :
  ��������   : PhaseTurn.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��24��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __PHASETURN_H__
#define __PHASETURN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    unsigned short nPhaseTurnID;//������
    unsigned short nCircleID;//����
    unsigned short nTurnArray[16];//������

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
