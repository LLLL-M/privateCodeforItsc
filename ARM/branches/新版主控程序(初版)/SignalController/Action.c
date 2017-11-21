/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Action.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : 本文主要实现了信号机配置工具中动作表的相关接口，包括初始化动
               作表，添加、删除动作项、情况动作表、通过ID查找指定的动作项等
  函数列表   :
              AddOrAlterActionItem
              ClearActionList
              DeleteActionItem
              DestroyActionList
              GetSchemeID
              InitActionList
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

  2.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 使用参数传址的方式而不是使用全局变量，这样可以在以后的代码
                 扩展中支持多线程。
******************************************************************************/

#include "Action.h"



/*****************************************************************************
 函 数 名  : InitActionList
 功能描述  : 初始化动作表
 输入参数  : PActionList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
 函 数 名  : AddOrAlterActionItem
 功能描述  : 添加新的动作项
 输入参数  : PActionList *pList  
             PActionItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int AddOrAlterActionItem(PActionList *pList,PActionItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PActionList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nActionID == pItem->nActionID)//ActionID号已存在且相同，表明是修改某一项，不是添加新项
            {
                pTempList->pItem->nSchemeID = pItem->nSchemeID;
                pTempList->pItem->cGrayController = pItem->cGrayController;

                memset(pTempList->pItem->cAuxiliary,0,sizeof(pTempList->pItem->cAuxiliary));
                memcpy(pTempList->pItem->cAuxiliary,pItem->cAuxiliary,sizeof(pItem->cAuxiliary));
                
                memset(pTempList->pItem->cSpecialFun,0,sizeof(pTempList->pItem->cSpecialFun));
                memcpy(pTempList->pItem->cSpecialFun,pItem->cSpecialFun,sizeof(pItem->cSpecialFun));

                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PActionItem)malloc(sizeof(ActionItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterActionItem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nSchemeID = pItem->nSchemeID;
    pTempList->pItem->nActionID = pItem->nActionID;
    pTempList->pItem->cGrayController = pItem->cGrayController;

    memset(pTempList->pItem->cAuxiliary,0,sizeof(pTempList->pItem->cAuxiliary));
    memcpy(pTempList->pItem->cAuxiliary,pItem->cAuxiliary,sizeof(pItem->cAuxiliary));
                
    memset(pTempList->pItem->cSpecialFun,0,sizeof(pTempList->pItem->cSpecialFun));
    memcpy(pTempList->pItem->cSpecialFun,pItem->cSpecialFun,sizeof(pItem->cSpecialFun));



    //为当前结点分配下一空结点
    PActionList pNewList;
    pNewList = (PActionList)malloc(sizeof(ActionList));

    if(!pNewList)
    {
        log_debug("AddOrAlterActionItem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}


/*****************************************************************************
 函 数 名  : DeleteActionItem
 功能描述  : 删除指定的动作项
 输入参数  : PActionList *pList  
             PActionItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DeleteActionItem(PActionList *pList,PActionItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PActionList pTempList = *pList;
    PActionList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nActionID == pItem->nActionID)//找到需要删除的项
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
 函 数 名  : GetSchemeID
 功能描述  : 通过时段表的动作号，找到指定的动作项，进而获得动作表中的方案号
 输入参数  : PActionList *pList        
             unsigned short nActionID  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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
    

    return false;//没有找到符合的



}

/*****************************************************************************
 函 数 名  : ClearActionList
 功能描述  : 情况指定的动作表
 输入参数  : PActionList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int ClearActionList(PActionList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    (void) DestroyActionList(pList);//先释放

    return InitActionList(pList);//再重新初始化

}
/*****************************************************************************
 函 数 名  : DestroyActionList
 功能描述  : 销毁动作表
 输入参数  : PActionList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

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

        if(pTempList->pItem != null)//有内容则释放
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

























