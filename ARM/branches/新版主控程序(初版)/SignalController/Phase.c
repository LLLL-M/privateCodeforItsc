/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Phase.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月17日
  最近修改   :
  功能描述   : 这里主要实现了相位表的一些接口函数.
  函数列表   :
              AddOrAlterPhaseItem
              ClearPhaseList
              DeletePhaseItem
              DestroyPhaseList
              InitPhaseList
  修改历史   :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 创建文件

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

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseID == pItem->nPhaseID)//nPhaseID号已存在且相同，表明是修改某一项，不是添加新项
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

                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PPhaseItem)malloc(sizeof(PhaseItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterPhasetem error to alloc \n");
        return false;
    }



    //填充新节点
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


    //为当前结点分配下一空结点
    PPhaseList pNewList;
    pNewList = (PPhaseList)malloc(sizeof(PhaseList));

    if(!pNewList)
    {
        log_debug("AddOrAlterPhasetem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
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


    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem->nPhaseID == nPhaseID)
        {
            pTempItem = pTempList->pItem;
            break;
        }
        pTempList = pTempList->next;

    }


    return pTempItem;//找不到

}



int DeletePhaseItem(PPhaseList *pList,PPhaseItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;
    PPhaseList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseID == pItem->nPhaseID)//找到需要删除的项
            {
                
                pPriorList->next = pTempList->next;

                free(pTempList->pItem);//先释放item内容
                free(pTempList);//再释放结点

                return true;//修改完成后直接退出
            }
        }
        pPriorList = pTempList;//保存前一项，留删除时使用
        pTempList = pTempList->next;

    }


    return false;//找不到

}
int ClearPhaseList(PPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyPhaseList(pList);//先释放

    return InitPhaseList(pList);//再重新初始化

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

        if(pTempList->pItem != null)//有内容则释放
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
    memset(&item,0,sizeof(item));//默认所有值都是0，所以后面只需要把不是0的参数赋值即可


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

    for(i=0; i < 3 ; i++)//默认初始化三个相位
    {
        item.nPhaseID = i+1;

        AddOrAlterPhaseItem(&pList,&item);

    }


    return 1;

}


/*****************************************************************************
 函 数 名  : SetPhaseGreenSignalRatioPara
 功能描述  : 通过给定的相位号，设置相位中有关绿信比的属性
 输入参数  : PPhaseList *pList                      
             unsigned short nGreenSignalRationTime  
             GreenSignalRationType nType            
             unsigned short nIsCoordinate           
             unsigned short nPhaseID                
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int SetPhaseGreenSignalRatioPara(PPhaseList *pList,unsigned short nGreenSignalRationTime,GreenSignalRationType nType,
                                            unsigned short nIsCoordinate,unsigned short nPhaseID)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseList pTempList = *pList;

    //找到最后一个结点
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



