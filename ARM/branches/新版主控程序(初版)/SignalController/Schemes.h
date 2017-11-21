/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Schemes.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : Schemes.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __SCHEMES_H__
#define __SCHEMES_H__


#include "Util.h"
#include "Action.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {

    unsigned short nSchemeID;//������
    unsigned short nCycleTime;//���ڳ�
    unsigned short nOffset;//��λ��
    unsigned short nGreenSignalRatioID;//���űȺ�
    unsigned short nPhaseTurnID;//������

}SchemeItem,*PSchemeItem;

typedef struct _SchemeList{

    PSchemeItem pItem;

    struct _SchemeList *next;


}SchemeList,*PSchemeList;


extern int AddOrAlterSchemetem(PSchemeList *pList,PSchemeItem pItem);
extern int ClearSchemeList(PSchemeList *pList);
extern int DeleteSchemeItem(PSchemeList *pList,PSchemeItem pItem);
extern int DestroySchemeList(PSchemeList *pList);
extern int GetGreenSignalRatioID(PSchemeList *pList,unsigned short nSchemeID);
extern int GetPhaseOrderID(PSchemeList *pList,unsigned short nSchemeID);
extern int InitShemeList(PSchemeList *pList);

extern int LoadDefaultSchemes(PSchemeList pList);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCHEMES_H__ */
