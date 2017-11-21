/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : GreenSignalRatio.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月17日
  最近修改   :
  功能描述   : 这里主要实现了绿信比的一些接口函数，比如初始化、增删改等。
  函数列表   :
              AddOrAlterGreenSignalRationItem
              ClearGreenSignalRationList
              DeleteGreenSignalRationItem
              DestroyGreenSignalRationList
              InitGreenSignalRationList
  修改历史   :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 创建文件

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

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if((pTempList->pItem->nGreenSignalRationID == pItem->nGreenSignalRationID) && (pTempList->pItem->nPhaseID == pItem->nPhaseID))//nGreenSignalRationID号已存在且相同，表明是修改某一项，不是添加新项
            {
                pTempList->pItem->nGreenSignalRationTime = pItem->nGreenSignalRationTime;
                pTempList->pItem->nIsCoordinate = pItem->nIsCoordinate;
                pTempList->pItem->nType = pItem->nType;

                //在建立绿信比表的时候，就把绿信比表和相位表关联起来
               // SetPhaseGreenSignalRatioPara(&pPhaseList, pItem->nGreenSignalRationTime, pItem->nType, pItem->nIsCoordinate, pItem->nPhaseID);
                
                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PGreenSignalRationItem)malloc(sizeof(GreenSignalRationItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterGreenSignalRationtem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nGreenSignalRationID = pItem->nGreenSignalRationID;
    pTempList->pItem->nGreenSignalRationTime = pItem->nGreenSignalRationTime;
    pTempList->pItem->nIsCoordinate = pItem->nIsCoordinate;
    pTempList->pItem->nPhaseID = pItem->nPhaseID;
    pTempList->pItem->nType = pItem->nType;

    //在建立绿信比表的时候，就把绿信比表和相位表关联起来
    //SetPhaseGreenSignalRatioPara(&pPhaseList, pItem->nGreenSignalRationTime, pItem->nType, pItem->nIsCoordinate, pItem->nPhaseID);


    //为当前结点分配下一空结点
    PGreenSignalRationList pNewList;
    pNewList = (PGreenSignalRationList)malloc(sizeof(GreenSignalRationList));

    if(!pNewList)
    {
        log_debug("AddOrAlterGreenSignalRationtem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
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

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nGreenSignalRationID == pItem->nGreenSignalRationID)//找到需要删除的项
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
int ClearGreenSignalRationList(PGreenSignalRationList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyGreenSignalRationList(pList);//先释放

    return InitGreenSignalRationList(pList);//再重新初始化

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

        if(pTempList->pItem != null)//有内容则释放
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
 函 数 名  : SetPhaseGreenSignalRationItem
 功能描述  : 根据绿信比表ID和绿信比表中的相位ID，将绿信比表中的绿信比时间等
             参数填入到指定相位的相位参数中
 输入参数  : PGreenSignalRationList *pList        
             unsigned short nGreenSignalRationID  
             unsigned short nPhaseID              
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月24日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int SetPhaseGreenSignalRationItem(PGreenSignalRationList *pList,unsigned short nGreenSignalRationID,unsigned short nPhaseID)
{
    if(*pList == null)
    {
        return false;
    }

    PGreenSignalRationList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if((pTempList->pItem->nGreenSignalRationID == nGreenSignalRationID) && (pTempList->pItem->nPhaseID == nPhaseID))//找到需要删除的项
            {

                //就把绿信比表和相位表关联起来
                SetPhaseGreenSignalRatioPara(&pSignalControllerPara->pPhaseList, pTempList->pItem->nGreenSignalRationTime, pTempList->pItem->nType, pTempList->pItem->nIsCoordinate, pTempList->pItem->nPhaseID);

                return true;
            }
        }
        pTempList = pTempList->next;

    }


    return false;//找不到

}



