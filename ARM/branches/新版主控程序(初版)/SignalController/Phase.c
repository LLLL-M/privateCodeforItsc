/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Phase.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��17��
  ����޸�   :
  ��������   : ������Ҫʵ������λ���һЩ�ӿں���.
  �����б�   :
              AddOrAlterPhaseItem
              ClearPhaseList
              DeletePhaseItem
              DestroyPhaseList
              InitPhaseList
  �޸���ʷ   :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#include "Util.h"
#include "GreenSignalRatio.h"
#include "Phase.h"

int InitPhaseList(PPhaseList *pList)
{
    (*pList) = (PPhaseList)malloc(sizeof(PhaseList));

    if(!(*pList))
    {
        log_debug("InitPhaseList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    return true;
}

int AddOrAlterPhaseItem(PPhaseList *pList,PPhaseItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseID == pItem->nPhaseID)//nPhaseID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->pItem->nAllRedTime = pItem->nAllRedTime;
                pTempList->pItem->nCircleID = pItem->nCircleID;
                pTempList->pItem->nGreenLightTime = pItem->nGreenLightTime;
                pTempList->pItem->nMaxGreen_1= pItem->nMaxGreen_1;
                pTempList->pItem->nMaxGreen_2= pItem->nMaxGreen_2;
                pTempList->pItem->nMinGreen = pItem->nMinGreen;
                pTempList->pItem->nPedestrianClearTime = pItem->nPedestrianClearTime;
                pTempList->pItem->nPedestrianPassTime = pItem->nPedestrianPassTime;
                pTempList->pItem->nRedProtectedTime = pItem->nRedProtectedTime;
                pTempList->pItem->nRedYellowTime = pItem->nRedYellowTime;
                pTempList->pItem->nSafeRedTime = pItem->nSafeRedTime;
                pTempList->pItem->nUnitExtendGreen = pItem->nUnitExtendGreen;
                pTempList->pItem->nYellowTime = pItem->nYellowTime;
                pTempList->pItem->cAutoPedestrianPass = pItem->cAutoPedestrianPass;
                pTempList->pItem->cAutoReq = pItem->cAutoReq;
                pTempList->pItem->cDetectorLocked = pItem->cDetectorLocked;
                pTempList->pItem->cIsEnablePhase = pItem->cIsEnablePhase;
                pTempList->pItem->cMaxGreenReq = pItem->cMaxGreenReq;
                pTempList->pItem->cMinGreenReq = pItem->cMinGreenReq;
                pTempList->pItem->cPedestrianReq = pItem->cPedestrianReq;
                pTempList->pItem->cYellowEnter = pItem->cYellowEnter;
                pTempList->pItem->cYellowExit = pItem->cYellowExit;
                pTempList->pItem->initSetting = pItem->initSetting;

                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PPhaseItem)malloc(sizeof(PhaseItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterPhasetem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nPhaseID = pItem->nPhaseID;
    pTempList->pItem->nAllRedTime = pItem->nAllRedTime;
    pTempList->pItem->nCircleID = pItem->nCircleID;
    pTempList->pItem->nGreenLightTime = pItem->nGreenLightTime;
    pTempList->pItem->nMaxGreen_1= pItem->nMaxGreen_1;
    pTempList->pItem->nMaxGreen_2= pItem->nMaxGreen_2;
    pTempList->pItem->nMinGreen = pItem->nMinGreen;
    pTempList->pItem->nPedestrianClearTime = pItem->nPedestrianClearTime;
    pTempList->pItem->nPedestrianPassTime = pItem->nPedestrianPassTime;
    pTempList->pItem->nRedProtectedTime = pItem->nRedProtectedTime;
    pTempList->pItem->nRedYellowTime = pItem->nRedYellowTime;
    pTempList->pItem->nSafeRedTime = pItem->nSafeRedTime;
    pTempList->pItem->nUnitExtendGreen = pItem->nUnitExtendGreen;
    pTempList->pItem->nYellowTime = pItem->nYellowTime;
    pTempList->pItem->cAutoPedestrianPass = pItem->cAutoPedestrianPass;
    pTempList->pItem->cAutoReq = pItem->cAutoReq;
    pTempList->pItem->cDetectorLocked = pItem->cDetectorLocked;
    pTempList->pItem->cIsEnablePhase = pItem->cIsEnablePhase;
    pTempList->pItem->cMaxGreenReq = pItem->cMaxGreenReq;
    pTempList->pItem->cMinGreenReq = pItem->cMinGreenReq;
    pTempList->pItem->cPedestrianReq = pItem->cPedestrianReq;
    pTempList->pItem->cYellowEnter = pItem->cYellowEnter;
    pTempList->pItem->cYellowExit = pItem->cYellowExit;
    pTempList->pItem->initSetting = pItem->initSetting;


    //Ϊ��ǰ��������һ�ս��
    PPhaseList pNewList;
    pNewList = (PPhaseList)malloc(sizeof(PhaseList));

    if(!pNewList)
    {
        log_debug("AddOrAlterPhasetem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}

PPhaseItem GetPhaseItem(PPhaseList pList,unsigned short nPhaseID)
{
    if(pList == null)
    {
        return null;
    }

    PPhaseList pTempList = pList;
    PPhaseItem pTempItem = null;


    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem->nPhaseID == nPhaseID)
        {
            pTempItem = pTempList->pItem;
            break;
        }
        pTempList = pTempList->next;

    }


    return pTempItem;//�Ҳ���

}



int DeletePhaseItem(PPhaseList *pList,PPhaseItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;
    PPhaseList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseID == pItem->nPhaseID)//�ҵ���Ҫɾ������
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
int ClearPhaseList(PPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyPhaseList(pList);//���ͷ�

    return InitPhaseList(pList);//�����³�ʼ��

}
int DestroyPhaseList(PPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;
    PPhaseList pPriorList = null;

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


int LoadDefaultPhase(PPhaseList pList)
{
    if(!pList)
    {
        return 0;
    }

    PhaseItem item;
    memset(&item,0,sizeof(item));//Ĭ������ֵ����0�����Ժ���ֻ��Ҫ�Ѳ���0�Ĳ�����ֵ����


    item.nPhaseID = 1;
    item.nCircleID = 1;
    
    item.nMinGreen = 15;
    item.nMaxGreen_1 = 30;
    item.nMaxGreen_2 = 60;
    item.nUnitExtendGreen = 3;
    item.nYellowTime = 3;
    item.nAllRedTime = 2;
    //item.nRedProtectedTime = 0;
    //item.nRedYellowTime = 0;
    item.nGreenLightTime = 2;
   // item.nSafeRedTime = 0;

    //item.nPedestrianPassTime = 0;
    //item.nPedestrianClearTime = 0;

    item.cIsEnablePhase = 1;
    //item.cYellowEnter = 0;
    
    item.initSetting = YELLOW;

    int i = 0;

    for(i=0; i < 3 ; i++)//Ĭ�ϳ�ʼ��������λ
    {
        item.nPhaseID = i+1;

        AddOrAlterPhaseItem(&pList,&item);

    }


    return 1;

}


/*****************************************************************************
 �� �� ��  : SetPhaseGreenSignalRatioPara
 ��������  : ͨ����������λ�ţ�������λ���й����űȵ�����
 �������  : PPhaseList *pList                      
             unsigned short nGreenSignalRationTime  
             GreenSignalRationType nType            
             unsigned short nIsCoordinate           
             unsigned short nPhaseID                
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int SetPhaseGreenSignalRatioPara(PPhaseList *pList,unsigned short nGreenSignalRationTime,GreenSignalRationType nType,
                                            unsigned short nIsCoordinate,unsigned short nPhaseID)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseID == nPhaseID)
            {
                pTempList->pItem->nGreenSignalRationTime = nGreenSignalRationTime;
                pTempList->pItem->nType = nType;
                pTempList->pItem->nIsCoordinate = nIsCoordinate;

                return true;
            }
        }

        pTempList = pTempList->next;

    }

    return false;

}



