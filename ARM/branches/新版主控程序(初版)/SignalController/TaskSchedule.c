/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : TaskSchedule.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月4日
  最近修改   :
  功能描述   : 本文主要实现了调度相关的接口，先调用Init函数，紧接着可以进行
               增删改查操作，最后需要destroy这个链表。
  函数列表   :
              AddOrAlterTaskScheduleItem
              ClearTaskSchedule
              DeleteTaskScheduleItem
              DestroyTaskSchedule
              GetTimeIntervalIndex
              InitTaskSchedule
  修改历史   :
  1.日    期   : 2014年7月4日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#include "TaskSchedule.h"



/*****************************************************************************
 函 数 名  : InitTaskSchedule
 功能描述  : 初始化调度链表
 输入参数  : PPlanScheduleList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int InitTaskSchedule(PPlanScheduleList *pList)
{
    (*pList) = (PPlanScheduleList)malloc(sizeof(PlanScheduleList));

    if(!(*pList))
    {
        log_debug("InitTaskSchedule  error to alloc \n");
        return false;
    }

    (*pList)->item = null;
    (*pList)->next = null;

    log_debug("%s succeed \n",__func__);

    return true;
}

/*****************************************************************************
 函 数 名  : AddOrAlterTaskScheduleItem
 功能描述  : 增加新的调度或者修改某一个指定调度
 输入参数  : PPlanScheduleList *pList  
             PPlanScheduleItem item    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int AddOrAlterTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item)
{
    if(*pList == null)
    {
        return false;
    }

    PPlanScheduleList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->item != null)
        {
            if(pTempList->item->nScheduleID == item->nScheduleID)//调度计划号已存在且相同，表明是修改某一项，不是添加新项
            {
                pTempList->item->nTimeIntervalID = item->nTimeIntervalID;
                memset(&pTempList->item->planTime,0,sizeof(PlanTime));
                memcpy(&pTempList->item->planTime,&item->planTime,sizeof(item->planTime));

                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->item = (PPlanScheduleItem)malloc(sizeof(PlanScheduleItem));

    if(pTempList->item == null)
    {
        log_error("AddOrAlterTaskScheduleItem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->item->nScheduleID = item->nScheduleID;
    pTempList->item->nTimeIntervalID = item->nTimeIntervalID;
    memset(&pTempList->item->planTime,0,sizeof(PlanTime));
    memcpy(&pTempList->item->planTime,&item->planTime,sizeof(item->planTime));

    //log_debug("........   %d ....\n",pTempList->item->nTimeIntervalID);

    //为当前结点分配下一空结点
    PPlanScheduleList pNewList;
    pNewList = (PPlanScheduleList)malloc(sizeof(PlanScheduleList));

    if(!pNewList)
    {
        log_debug("AddOrAlterTaskScheduleItem  error to alloc \n");
        return false;
    }

    pNewList->item = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}

/*****************************************************************************
 函 数 名  : DeleteTaskScheduleItem
 功能描述  : 根据需要，删除指定的某一项调度
 输入参数  : PPlanScheduleList *pList  
             PPlanScheduleItem item    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月4日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DeleteTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item)
{
    if(*pList == null)
    {
        return false;
    }

    PPlanScheduleList pTempList = *pList;
    PPlanScheduleList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->item != null)
        {
            if(pTempList->item->nScheduleID == item->nScheduleID)//找到需要删除的项
            {
                
                pPriorList->next = pTempList->next;

                free(pTempList->item);//先释放item内容
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
 函 数 名  : GetTimeIntervalIndex
 功能描述  : 根据本地当前时间，得到当前时段表号，通过时段表号再查询当前时段
             的动作
 输入参数  : PPlanScheduleList *pList  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int GetTimeIntervalIndex(PPlanScheduleList *pList)
{
    if(*pList == null)
    {

        return false;
    }

    PPlanScheduleList pTemp = *pList;

    while(pTemp->next != null)
    {
        if(IsItemInIntArray(pTemp->item->planTime.month, sizeof(pTemp->item->planTime.month), GetMonth()) == true)//月份符合
        {
            if(IsItemInIntArray(pTemp->item->planTime.day,sizeof(pTemp->item->planTime.day), GetDay()) == true)//日期符合
            {
                if(IsItemInIntArray(pTemp->item->planTime.week,sizeof(pTemp->item->planTime.week), GetWeek()) == true)//星期符合
                {
                    return pTemp->item->nTimeIntervalID;//直接返回时段表号
                }
            }
        }

        pTemp = pTemp->next;
    }
    

    return false;//没有找到符合的



}

/*****************************************************************************
 函 数 名  : ClearTaskSchedule
 功能描述  : 清空调度表内容，下次可以添加新的调度表项
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int ClearTaskSchedule(PPlanScheduleList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyTaskSchedule(pList);//先释放

    

    return InitTaskSchedule(pList);//再重新初始化

}

/*****************************************************************************
 函 数 名  : DestroyTaskSchedule
 功能描述  : 销毁调度表，销毁后，将无法再次添加新的项，用于系统清理
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int DestroyTaskSchedule(PPlanScheduleList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PPlanScheduleList pTempList = *pList;
    PPlanScheduleList pPriorList = null;

    while(pTempList != null)
    {
        pPriorList = pTempList->next;

        if(pTempList->item != null)//有内容则释放
        {
            free(pTempList->item);
        }

        free(pTempList);
        
        pTempList = pPriorList;

    }


    return true;

}

int LoadDefaultTaskSchedule(PPlanScheduleList pList)
{
    if(!pList)
    {
        return 0;
    }

    PlanScheduleItem item;

    memset(&item,0,sizeof(item));

    item.nScheduleID = 1;
    item.nTimeIntervalID = 1;

    int i = 0;

    for(i = 0 ; i < 12; i++)
    {
        item.planTime.month[i] = i+1;
    }


    for(i = 0 ; i < 7; i++)
    {
        item.planTime.week[i] = i+1;
    }


    for(i = 0 ; i < 31; i++)
    {
        item.planTime.day[i] = i+1;
    }


    return AddOrAlterTaskScheduleItem(&pList, &item);



}


