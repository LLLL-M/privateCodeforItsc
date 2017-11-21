/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Schemes.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : 这里主要实现了方案表的一些接口，比如初始化方案表、新增、修改
               、删除某项方案；清空、销毁方案表；通过方案表号获得相序表及绿
               信比表
  函数列表   :
              AddOrAlterSchemetem
              ClearSchemeList
              DeleteSchemeItem
              DestroySchemeList
              GetGreenSignalRatioID
              GetPhaseOrderID
              InitShemeList
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/


#include "Schemes.h"

/*****************************************************************************
 函 数 名  : InitShemeList
 功能描述  : 初始化方案表
 输入参数  : PSchemeList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : AddOrAlterSchemetem
 功能描述  : 添加或修改方案表项
 输入参数  : PSchemeList *pList  
             PSchemeItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int AddOrAlterSchemetem(PSchemeList *pList,PSchemeItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PSchemeList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nSchemeID == pItem->nSchemeID)//nTimeIntervalID号已存在且相同，表明是修改某一项，不是添加新项
            {
                pTempList->pItem->nCycleTime = pItem->nCycleTime;
                pTempList->pItem->nGreenSignalRatioID = pItem->nGreenSignalRatioID;
                pTempList->pItem->nOffset = pItem->nOffset;
                pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;

                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PSchemeItem)malloc(sizeof(SchemeItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterSchemetem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nSchemeID = pItem->nSchemeID;
    pTempList->pItem->nCycleTime = pItem->nCycleTime;
    pTempList->pItem->nGreenSignalRatioID = pItem->nGreenSignalRatioID;
    pTempList->pItem->nOffset = pItem->nOffset;
    pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;

    //为当前结点分配下一空结点
    PSchemeList pNewList;
    pNewList = (PSchemeList)malloc(sizeof(SchemeList));

    if(!pNewList)
    {
        log_debug("AddOrAlterSchemetem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}



/*****************************************************************************
 函 数 名  : DeleteSchemeItem
 功能描述  : 删除某方案表项
 输入参数  : PSchemeList *pList  
             PSchemeItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DeleteSchemeItem(PSchemeList *pList,PSchemeItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PSchemeList pTempList = *pList;
    PSchemeList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nSchemeID == pItem->nSchemeID)//找到需要删除的项
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

/*****************************************************************************
 函 数 名  : GetGreenSignalRatioID
 功能描述  : 通过方案表号获得绿信比表ID
 输入参数  : PSchemeList *pList        
             unsigned short nSchemeID  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
    

    return false;//没有找到符合的



}

/*****************************************************************************
 函 数 名  : GetPhaseOrderID
 功能描述  : 通过方案表号获得相序表ID
 输入参数  : PSchemeList *pList        
             unsigned short nSchemeID  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
    

    return false;//没有找到符合的



}




/*****************************************************************************
 函 数 名  : ClearSchemeList
 功能描述  : 清空方案表
 输入参数  : PSchemeList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int ClearSchemeList(PSchemeList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroySchemeList(pList);//先释放

    return InitShemeList(pList);//再重新初始化

}

/*****************************************************************************
 函 数 名  : DestroySchemeList
 功能描述  : 销毁方案表
 输入参数  : PSchemeList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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

        if(pTempList->pItem != null)//有内容则释放
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



















































































