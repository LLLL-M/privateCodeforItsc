/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : TimeInterval.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : 这里主要实现了时段表的相关接口，主要包括初始化时段表，添加、
               删除、清空、销毁时段表及根据时段表号查找动作表号
  函数列表   :
              AddOrAlterTimeIntervalItem
              ClearTimeIntervalList
              DeleteTimeIntervalItem
              DestroyTimeIntervalList
              GetActionID
              InitTimeIntervalList
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/


#include "TimeInterval.h"


/*****************************************************************************
 函 数 名  : InitTimeIntervalList
 功能描述  : 初始化时段表
 输入参数  : PTimeIntervalList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int InitTimeIntervalList(PTimeIntervalList *pList)
{
    (*pList) = (PTimeIntervalList)malloc(sizeof(TimeIntervalList));

    if(!(*pList))
    {
        log_debug("InitTimeIntervalList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    log_debug("%s succeed \n",__func__);

    return true;
}
/*****************************************************************************
 函 数 名  : AddOrAlterTimeIntervalItem
 功能描述  : 添加或修改指定的时段表项
 输入参数  : PTimeIntervalList *pList  
             PTimeIntervalItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int AddOrAlterTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PTimeIntervalList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nTimeIntervalID == pItem->nTimeIntervalID)//nTimeIntervalID号已存在且相同，表明是修改某一项，不是添加新项
            {
                if(pTempList->pItem->nTimeID == pItem->nTimeID)//时段号也相同，则表明是修改不是新增
                {
                    pTempList->pItem->nActionID = pItem->nActionID;
                    pTempList->pItem->cStartTimeHour = pItem->cStartTimeHour;
                    pTempList->pItem->cStartTimeMinute = pItem->cStartTimeMinute;

                    return true;//修改完成后直接退出
                }

            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PTimeIntervalItem)malloc(sizeof(TimeIntervalItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterActionItem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nActionID = pItem->nActionID;
    pTempList->pItem->nTimeIntervalID = pItem->nTimeIntervalID;
    pTempList->pItem->nTimeID = pItem->nTimeID;
    pTempList->pItem->cStartTimeHour = pItem->cStartTimeHour;
    pTempList->pItem->cStartTimeMinute = pItem->cStartTimeMinute;

    //为当前结点分配下一空结点
    PTimeIntervalList pNewList;
    pNewList = (PTimeIntervalList)malloc(sizeof(TimeIntervalList));

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
 函 数 名  : DeleteTimeIntervalItem
 功能描述  : 删除某项时段表项
 输入参数  : PTimeIntervalList *pList  
             PTimeIntervalItem pItem   
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DeleteTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PTimeIntervalList pTempList = *pList;
    PTimeIntervalList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nTimeIntervalID == pItem->nTimeIntervalID)//找到需要删除的项
            {
                if(pTempList->pItem->nTimeID == pItem->nTimeID)
                {
                    pPriorList->next = pTempList->next;

                    free(pTempList->pItem);//先释放item内容
                    free(pTempList);//再释放结点
                    
                    return true;//修改完成后直接退出
                }
            }
        }
        pPriorList = pTempList;//保存前一项，留删除时使用
        pTempList = pTempList->next;

    }


    return false;//找不到

}

static int HoursToMinutes(int hours,int minutes)
{

    return hours*60+minutes;

}



/*****************************************************************************
 函 数 名  : GetActionID
 功能描述  : 根据时段表号及当前时间确定当前应该允许的动作号
 输入参数  : PTimeIntervalList *pList        
             unsigned short nTimeIntervalID  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int GetActionID(PTimeIntervalList *pList,unsigned short nTimeIntervalID)
{
    if(*pList == null)
    {

        return false;
    }

    unsigned int nTempMin = 0;
    unsigned int nTempMax = 0;
    unsigned short nActionId = 0;
    int nTempTime = 0;
    int flag = 0;

    PTimeIntervalList pTemp = *pList;

    while(pTemp->next != null)
    {
        if(pTemp->pItem->nTimeIntervalID == nTimeIntervalID)//首先要找到这个时段表号为nTimeIntervalID的表，然后再根据时段确定动作号
        {
            nTempTime = HoursToMinutes(pTemp->pItem->cStartTimeHour, pTemp->pItem->cStartTimeMinute);
            //log_debug("==> Hour  %d  Minute  %d\n",pTemp->pItem->cStartTimeHour,pTemp->pItem->cStartTimeMinute);
            if(flag == 0)
            {
                flag = 1;
                nTempMin = nTempTime;
                nTempMax = nTempTime;
            }
            
            if(nTempTime >= nTempMax)//如果当前时间比temp还要大，就记录当前的ACTION
            {
                nTempMax = nTempTime;
                nActionId = pTemp->pItem->nActionID;
            }

            if(nTempTime < nTempMin)
            {
                nTempMin = nTempTime;
            }
        
        }

        pTemp = pTemp->next;
    }

    //如果当前最小值和最大值相等，那说明当前只有一个数据项，那么第一项就是需要找的ActionId
    pTemp = *pList;
    if(nTempMin == nTempMax)
    {
        if(pTemp->next != null)
        {
            return pTemp->pItem->nActionID;
        }
        else
        {
            return false;
        }
    }

    
    //用当前时间和当前链表中的最大值与最小值比较，如果当前比最小值小，那就取最大值时的actionId

    nTempTime = HoursToMinutes(GetHour(), GetMin());

    if(nTempTime < nTempMin)
    {
        return nActionId;
    }
    else//如果比最小值大，则需要重新遍历链表，找到数据项中
    {
        flag = 0;
        int nCurrentTime = 0;
        int nMinusTime = 0;
        nCurrentTime = HoursToMinutes(GetHour(), GetMin());
        while(pTemp->next != null)
        {
            if(pTemp->pItem->nTimeIntervalID == nTimeIntervalID)//首先要找到这个时段表号为nTimeIntervalID的表，然后再根据时段确定动作号
            {
                nTempTime = HoursToMinutes(pTemp->pItem->cStartTimeHour, pTemp->pItem->cStartTimeMinute);
                
                if((flag == 0) && (nCurrentTime > nTempTime))
                {
                    flag = 1;
                    nMinusTime = nCurrentTime - nTempTime;
                    nActionId = pTemp->pItem->nActionID;
                    continue;
                }
                
                if((nCurrentTime > nTempTime)&&(nCurrentTime - nTempTime < nMinusTime))//如果当前大于0，且差值更小，就记录当前的ACTION
                {
                    nMinusTime = nCurrentTime - nTempTime;
                    nActionId = pTemp->pItem->nActionID;
                }

            }

            pTemp = pTemp->next;
        }

    }


    return nActionId;//没有找到符合的



}

/*****************************************************************************
 函 数 名  : ClearTimeIntervalList
 功能描述  : 清空时段表数据
 输入参数  : PTimeIntervalList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int ClearTimeIntervalList(PTimeIntervalList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyTimeIntervalList(pList);//先释放

    return InitTimeIntervalList(pList);//再重新初始化

}

/*****************************************************************************
 函 数 名  : DestroyTimeIntervalList
 功能描述  : 销毁时段表，时段表将无法使用
 输入参数  : PTimeIntervalList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DestroyTimeIntervalList(PTimeIntervalList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PTimeIntervalList pTempList = *pList;
    PTimeIntervalList pPriorList = null;

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


int LoadDefaultTimeInterval(PTimeIntervalList pList)
{
    if(!pList)
    {
        return 0;

    }

    TimeIntervalItem item;
    memset(&item,0,sizeof(item));

    item.nTimeIntervalID = 1;
    item.nTimeID = 1;
    item.nActionID = 1;

    AddOrAlterTimeIntervalItem(&pList,&item);

    item.nTimeIntervalID = 1;
    item.nActionID = 1;
    item.nTimeID = 2;
    item.cStartTimeHour = 4;
    item.cStartTimeMinute = 37;

    return AddOrAlterTimeIntervalItem(&pList,&item);
}



