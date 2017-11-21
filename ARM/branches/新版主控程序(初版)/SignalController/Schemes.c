/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Schemes.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : ������Ҫʵ���˷������һЩ�ӿڣ������ʼ���������������޸�
               ��ɾ��ĳ�������ա����ٷ�����ͨ��������Ż���������
               �űȱ�
  �����б�   :
              AddOrAlterSchemetem
              ClearSchemeList
              DeleteSchemeItem
              DestroySchemeList
              GetGreenSignalRatioID
              GetPhaseOrderID
              InitShemeList
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/


#include "Schemes.h"

/*****************************************************************************
 �� �� ��  : InitShemeList
 ��������  : ��ʼ��������
 �������  : PSchemeList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int InitShemeList(PSchemeList *pList)
{
    (*pList) = (PSchemeList)malloc(sizeof(SchemeList));

    if(!(*pList))
    {
        log_debug("InitShemeList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;
    
    log_debug("%s succeed \n",__func__);

    return true;
}

/*****************************************************************************
 �� �� ��  : AddOrAlterSchemetem
 ��������  : ��ӻ��޸ķ�������
 �������  : PSchemeList *pList  
             PSchemeItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int AddOrAlterSchemetem(PSchemeList *pList,PSchemeItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PSchemeList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nSchemeID == pItem->nSchemeID)//nTimeIntervalID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->pItem->nCycleTime = pItem->nCycleTime;
                pTempList->pItem->nGreenSignalRatioID = pItem->nGreenSignalRatioID;
                pTempList->pItem->nOffset = pItem->nOffset;
                pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;

                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PSchemeItem)malloc(sizeof(SchemeItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterSchemetem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nSchemeID = pItem->nSchemeID;
    pTempList->pItem->nCycleTime = pItem->nCycleTime;
    pTempList->pItem->nGreenSignalRatioID = pItem->nGreenSignalRatioID;
    pTempList->pItem->nOffset = pItem->nOffset;
    pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;

    //Ϊ��ǰ��������һ�ս��
    PSchemeList pNewList;
    pNewList = (PSchemeList)malloc(sizeof(SchemeList));

    if(!pNewList)
    {
        log_debug("AddOrAlterSchemetem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}



/*****************************************************************************
 �� �� ��  : DeleteSchemeItem
 ��������  : ɾ��ĳ��������
 �������  : PSchemeList *pList  
             PSchemeItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DeleteSchemeItem(PSchemeList *pList,PSchemeItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PSchemeList pTempList = *pList;
    PSchemeList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nSchemeID == pItem->nSchemeID)//�ҵ���Ҫɾ������
            {
                
                pPriorList->next = pTempList->next;

                free(pTempList->pItem);//���ͷ�item����
                free(pTempList);//���ͷŽ��

                return true;//�޸���ɺ�ֱ���˳�
            }
        }
        pPriorList = pTempList;//����ǰһ���ɾ��ʱʹ��
        pTempList = pTempList->next;

    }


    return false;//�Ҳ���

}

/*****************************************************************************
 �� �� ��  : GetGreenSignalRatioID
 ��������  : ͨ��������Ż�����űȱ�ID
 �������  : PSchemeList *pList        
             unsigned short nSchemeID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int GetGreenSignalRatioID(PSchemeList *pList,unsigned short nSchemeID)
{
    if(*pList == null)
    {

        return false;
    }

    PSchemeList pTemp = *pList;

    while(pTemp->next != null)
    {
        if(pTemp->pItem->nSchemeID == nSchemeID)
        {
            return pTemp->pItem->nGreenSignalRatioID;
        }

        pTemp = pTemp->next;
    }
    

    return false;//û���ҵ����ϵ�



}

/*****************************************************************************
 �� �� ��  : GetPhaseOrderID
 ��������  : ͨ��������Ż�������ID
 �������  : PSchemeList *pList        
             unsigned short nSchemeID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int GetPhaseOrderID(PSchemeList *pList,unsigned short nSchemeID)
{
    if(*pList == null)
    {

        return false;
    }

    PSchemeList pTemp = *pList;

    while(pTemp->next != null)
    {
        if(pTemp->pItem->nSchemeID == nSchemeID)
        {
            return pTemp->pItem->nPhaseTurnID;
        }

        pTemp = pTemp->next;
    }
    

    return false;//û���ҵ����ϵ�



}




/*****************************************************************************
 �� �� ��  : ClearSchemeList
 ��������  : ��շ�����
 �������  : PSchemeList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int ClearSchemeList(PSchemeList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroySchemeList(pList);//���ͷ�

    return InitShemeList(pList);//�����³�ʼ��

}

/*****************************************************************************
 �� �� ��  : DestroySchemeList
 ��������  : ���ٷ�����
 �������  : PSchemeList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DestroySchemeList(PSchemeList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PSchemeList pTempList = *pList;
    PSchemeList pPriorList = null;

    while(pTempList != null)
    {
        pPriorList = pTempList->next;

        if(pTempList->pItem != null)//���������ͷ�
        {
            free(pTempList->pItem);
        }

        free(pTempList);
        
        pTempList = pPriorList;

    }


    return true;

}


int LoadDefaultSchemes(PSchemeList pList)
{
    if(!pList)
    {
        return 0;
    }

    SchemeItem item;

    memset(&item,0,sizeof(item));


    item.nSchemeID = 1;
    item.nCycleTime = 55;
    item.nOffset = 0;
    item.nPhaseTurnID = 1;
    item.nGreenSignalRatioID = 1;

    return AddOrAlterSchemetem(&pList, &item);


}



















































































