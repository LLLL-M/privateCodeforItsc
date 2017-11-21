/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : GreenSignalRatio.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��17��
  ����޸�   :
  ��������   : ������Ҫʵ�������űȵ�һЩ�ӿں����������ʼ������ɾ�ĵȡ�
  �����б�   :
              AddOrAlterGreenSignalRationItem
              ClearGreenSignalRationList
              DeleteGreenSignalRationItem
              DestroyGreenSignalRationList
              InitGreenSignalRationList
  �޸���ʷ   :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/
#include "Util.h"
#include "Main.h"
#include "GreenSignalRatio.h"
#include "Phase.h"

//extern PPhaseList pPhaseList;
extern PSignalControllerPara pSignalControllerPara;


int InitGreenSignalRationList(PGreenSignalRationList *pList)
{
    (*pList) = (PGreenSignalRationList)malloc(sizeof(GreenSignalRationList));

    if(!(*pList))
    {
        log_debug("InitGreenSignalRationList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    log_debug("%s succeed \n",__func__);

    return true;
}

int AddOrAlterGreenSignalRationItem(PGreenSignalRationList *pList,PGreenSignalRationItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PGreenSignalRationList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if((pTempList->pItem->nGreenSignalRationID == pItem->nGreenSignalRationID) && (pTempList->pItem->nPhaseID == pItem->nPhaseID))//nGreenSignalRationID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->pItem->nGreenSignalRationTime = pItem->nGreenSignalRationTime;
                pTempList->pItem->nIsCoordinate = pItem->nIsCoordinate;
                pTempList->pItem->nType = pItem->nType;

                //�ڽ������űȱ��ʱ�򣬾Ͱ����űȱ����λ���������
               // SetPhaseGreenSignalRatioPara(&pPhaseList, pItem->nGreenSignalRationTime, pItem->nType, pItem->nIsCoordinate, pItem->nPhaseID);
                
                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PGreenSignalRationItem)malloc(sizeof(GreenSignalRationItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterGreenSignalRationtem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nGreenSignalRationID = pItem->nGreenSignalRationID;
    pTempList->pItem->nGreenSignalRationTime = pItem->nGreenSignalRationTime;
    pTempList->pItem->nIsCoordinate = pItem->nIsCoordinate;
    pTempList->pItem->nPhaseID = pItem->nPhaseID;
    pTempList->pItem->nType = pItem->nType;

    //�ڽ������űȱ��ʱ�򣬾Ͱ����űȱ����λ���������
    //SetPhaseGreenSignalRatioPara(&pPhaseList, pItem->nGreenSignalRationTime, pItem->nType, pItem->nIsCoordinate, pItem->nPhaseID);


    //Ϊ��ǰ��������һ�ս��
    PGreenSignalRationList pNewList;
    pNewList = (PGreenSignalRationList)malloc(sizeof(GreenSignalRationList));

    if(!pNewList)
    {
        log_debug("AddOrAlterGreenSignalRationtem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}



int DeleteGreenSignalRationItem(PGreenSignalRationList *pList,PGreenSignalRationItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PGreenSignalRationList pTempList = *pList;
    PGreenSignalRationList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nGreenSignalRationID == pItem->nGreenSignalRationID)//�ҵ���Ҫɾ������
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
int ClearGreenSignalRationList(PGreenSignalRationList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyGreenSignalRationList(pList);//���ͷ�

    return InitGreenSignalRationList(pList);//�����³�ʼ��

}
int DestroyGreenSignalRationList(PGreenSignalRationList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PGreenSignalRationList pTempList = *pList;
    PGreenSignalRationList pPriorList = null;

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

int LoadDefaultGreenSignalRation(PGreenSignalRationList pList)
{
    if(!pList)
    {
        return 0;
    }

    GreenSignalRationItem item;
    memset(&item,0,sizeof(item));

    item.nGreenSignalRationID = 1;
    item.nType = NONE;
    item.nGreenSignalRationTime = 10;

    int i = 0;

    for(i = 0 ; i < 3 ; i++)
    {
        if(i == 0)
        {
            item.nGreenSignalRationTime = 25;
        }

        item.nPhaseID = (unsigned short)(i+1);
    
        AddOrAlterGreenSignalRationItem(&pList,&item);
    }

    return 1;


}


/*****************************************************************************
 �� �� ��  : SetPhaseGreenSignalRationItem
 ��������  : �������űȱ�ID�����űȱ��е���λID�������űȱ��е����ű�ʱ���
             �������뵽ָ����λ����λ������
 �������  : PGreenSignalRationList *pList        
             unsigned short nGreenSignalRationID  
             unsigned short nPhaseID              
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��24��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int SetPhaseGreenSignalRationItem(PGreenSignalRationList *pList,unsigned short nGreenSignalRationID,unsigned short nPhaseID)
{
    if(*pList == null)
    {
        return false;
    }

    PGreenSignalRationList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if((pTempList->pItem->nGreenSignalRationID == nGreenSignalRationID) && (pTempList->pItem->nPhaseID == nPhaseID))//�ҵ���Ҫɾ������
            {

                //�Ͱ����űȱ����λ���������
                SetPhaseGreenSignalRatioPara(&pSignalControllerPara->pPhaseList, pTempList->pItem->nGreenSignalRationTime, pTempList->pItem->nType, pTempList->pItem->nIsCoordinate, pTempList->pItem->nPhaseID);

                return true;
            }
        }
        pTempList = pTempList->next;

    }


    return false;//�Ҳ���

}



