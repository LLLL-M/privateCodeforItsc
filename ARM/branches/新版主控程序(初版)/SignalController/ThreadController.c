/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : ThreadController.c
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月26日
  最近修改   :
  功能描述   : 这里定义了监控线程状态的接口实现
  函数列表   :
              AddControllerItem
              DestroyControllerList
              InitControllerList
              MainControllerFun
  调用说明:
  依次调用InitControllerList,AddControllerItem,MainControllerFun,DestroyControllerList
              
  修改历史   :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 创建文件

******************************************************************************/

#include "ThreadController.h"


static PMyIntLinkList  pControllerList = NULL; 
static pthread_mutex_t g_lock_list;
static int g_sleep_ms = 100;//ms
/*****************************************************************************
 函 数 名  : InitControllerList
 功能描述  : 初始化线程控制器，用来捕捉线程状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int InitThreadControllerList()
{
    if(!InitLinkList((void **)&pControllerList, INT_LIST))
    {
        PRINT_ERROR;
        return false;
    }

	pthread_mutex_init(&g_lock_list,NULL);

    return true;
}

/*****************************************************************************
 函 数 名  : AddControllerItem
 功能描述  : 将需要监控的线程ID添加到线程控制器中。
 输入参数  : int item  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int AddThreadControllerItem(int item)
{
    if(!pControllerList)
    {
        return false;
    }

	pthread_mutex_lock(&g_lock_list);
    if(!AddLinkList((void **)&pControllerList, INT_LIST,&item))
    {
        PRINT_ERROR;
        pthread_mutex_unlock(&g_lock_list);
        return false;
    }

	pthread_mutex_unlock(&g_lock_list);

    log_debug("%s  add new thread ==> 0x%x\n",__func__,item);

    return true;
}
/*****************************************************************************
 函 数 名  : DestroyControllerList
 功能描述  : 销毁线程控制器
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int DestroyThreadControllerList()
{

	pthread_mutex_lock(&g_lock_list);

    if(!DestoryLinkList((void **)&pControllerList,INT_LIST))
    {
        PRINT_ERROR;
        pthread_mutex_unlock(&g_lock_list);
        return false;
    }

	pthread_mutex_unlock(&g_lock_list);

    pthread_mutex_destroy(&g_lock_list);
    
    return true;

}
/*****************************************************************************
 函 数 名  : MainControllerFun
 功能描述  : 适合在main函数中调用的线程控制器入口函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int MainThreadControllerFun()
{
    PMyIntLinkList pTempList = null;
    PMyIntLinkList pTempListPrior = null;//用于删除item时保存的前一项
    int res = 0;

    pTempList = pControllerList;

    pTempList = pTempList->next;
    
    while(1)
    {
        
        if(!pTempList)
        {
            pTempListPrior = pControllerList;
            pTempList = pControllerList->next;
            usleep(100*g_sleep_ms);
            continue;
        }
        
        res = pthread_kill(pTempList->val,0);
        if(res == ESRCH)
        {
            log_debug("MainControllerFun  thread 0x%x has exist . \n",pTempList->val);

            //remove the item  from the list
            pthread_mutex_lock(&g_lock_list);
            pTempListPrior->next = pTempList->next;
            free(pTempList);
            pTempList = pTempListPrior;
            pthread_mutex_unlock(&g_lock_list);
        }

        pTempListPrior = pTempList;
        pTempList = pTempList->next;

        usleep(100*g_sleep_ms);
    }

    return true;
}





