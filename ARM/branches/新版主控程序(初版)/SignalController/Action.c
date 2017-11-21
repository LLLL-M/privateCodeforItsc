/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Action.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : ������Ҫʵ�����źŻ����ù����ж��������ؽӿڣ�������ʼ����
               ������ӡ�ɾ����������������ͨ��ID����ָ���Ķ������
  �����б�   :
              AddOrAlterActionItem
              ClearActionList
              DeleteActionItem
              DestroyActionList
              GetSchemeID
              InitActionList
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

  2.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : ʹ�ò�����ַ�ķ�ʽ������ʹ��ȫ�ֱ����������������Ժ�Ĵ���
                 ��չ��֧�ֶ��̡߳�
******************************************************************************/

#include "Action.h"



/*****************************************************************************
 �� �� ��  : InitActionList
 ��������  : ��ʼ��������
 �������  : PActionList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int InitActionList(PActionList *pList)
{
    (*pList) = (PActionList)malloc(sizeof(ActionList));

    if(!(*pList))
    {
        log_debug("InitActionList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    log_debug("%s succeed \n",__func__);

    return true;
}

/*****************************************************************************
 �� �� ��  : AddOrAlterActionItem
 ��������  : ����µĶ�����
 �������  : PActionList *pList  
             PActionItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int AddOrAlterActionItem(PActionList *pList,PActionItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PActionList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nActionID == pItem->nActionID)//ActionID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->pItem->nSchemeID = pItem->nSchemeID;
                pTempList->pItem->cGrayController = pItem->cGrayController;

                memset(pTempList->pItem->cAuxiliary,0,sizeof(pTempList->pItem->cAuxiliary));
                memcpy(pTempList->pItem->cAuxiliary,pItem->cAuxiliary,sizeof(pItem->cAuxiliary));
                
                memset(pTempList->pItem->cSpecialFun,0,sizeof(pTempList->pItem->cSpecialFun));
                memcpy(pTempList->pItem->cSpecialFun,pItem->cSpecialFun,sizeof(pItem->cSpecialFun));

                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PActionItem)malloc(sizeof(ActionItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterActionItem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nSchemeID = pItem->nSchemeID;
    pTempList->pItem->nActionID = pItem->nActionID;
    pTempList->pItem->cGrayController = pItem->cGrayController;

    memset(pTempList->pItem->cAuxiliary,0,sizeof(pTempList->pItem->cAuxiliary));
    memcpy(pTempList->pItem->cAuxiliary,pItem->cAuxiliary,sizeof(pItem->cAuxiliary));
                
    memset(pTempList->pItem->cSpecialFun,0,sizeof(pTempList->pItem->cSpecialFun));
    memcpy(pTempList->pItem->cSpecialFun,pItem->cSpecialFun,sizeof(pItem->cSpecialFun));



    //Ϊ��ǰ��������һ�ս��
    PActionList pNewList;
    pNewList = (PActionList)malloc(sizeof(ActionList));

    if(!pNewList)
    {
        log_debug("AddOrAlterActionItem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}


/*****************************************************************************
 �� �� ��  : DeleteActionItem
 ��������  : ɾ��ָ���Ķ�����
 �������  : PActionList *pList  
             PActionItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DeleteActionItem(PActionList *pList,PActionItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PActionList pTempList = *pList;
    PActionList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nActionID == pItem->nActionID)//�ҵ���Ҫɾ������
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
 �� �� ��  : GetSchemeID
 ��������  : ͨ��ʱ�α�Ķ����ţ��ҵ�ָ���Ķ����������ö������еķ�����
 �������  : PActionList *pList        
             unsigned short nActionID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int GetSchemeID(PActionList *pList,unsigned short nActionID)
{
    if(*pList == null)
    {

        return false;
    }

    PActionList pTemp = *pList;

    while(pTemp->next != null)
    {
        if(pTemp->pItem->nActionID == nActionID)
        {
            return pTemp->pItem->nSchemeID;
        }

        pTemp = pTemp->next;
    }
    

    return false;//û���ҵ����ϵ�



}

/*****************************************************************************
 �� �� ��  : ClearActionList
 ��������  : ���ָ���Ķ�����
 �������  : PActionList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int ClearActionList(PActionList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    (void) DestroyActionList(pList);//���ͷ�

    return InitActionList(pList);//�����³�ʼ��

}
/*****************************************************************************
 �� �� ��  : DestroyActionList
 ��������  : ���ٶ�����
 �������  : PActionList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DestroyActionList(PActionList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PActionList pTempList = *pList;
    PActionList pPriorList = null;

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

int LoadDefaultAction(PActionList pList)
{
    if(!pList)
    {
        return 0;
    }


    ActionItem item;

    memset(&item,0,sizeof(item));

    item.nActionID = 1;
    item.nSchemeID = 1;
    AddOrAlterActionItem(&pList,&item);

    item.nActionID = 2;
    item.nSchemeID = 2;    

    return AddOrAlterActionItem(&pList,&item);

}

























