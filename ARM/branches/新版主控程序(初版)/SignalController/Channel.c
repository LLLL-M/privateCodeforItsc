/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Channel.c
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月16日
  最近修改   :
  功能描述   : 这里主要实现了通道表的一系列接口函数。
  函数列表   :
              AddOrAlterChanneltem
              ClearChannelList
              DeleteChannelItem
              DestroyChannelList
              InitChannelList
  修改历史   :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#include "Channel.h"

unsigned short gCurrentChannelArray[MAX_CHANNEL_NUM] = {0};//当前配置下，总共使用的通道数数组，可以用来做形参，传递给点灯函数
unsigned short gCurrentChannelNum = 0;//当前配置下，总共使用的通道总数

int InitChannelList(PChannelList *pList)
{
    (*pList) = (PChannelList)malloc(sizeof(ChannelList));

    if(!(*pList))
    {
        log_debug("InitChannelList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;
    
    log_debug("%s succeed \n",__func__);

    return true;
}

int AddOrAlterChanneltem(PChannelList *pList,PChannelItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PChannelList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nChannelID == pItem->nChannelID)//nChannelID号已存在且相同，表明是修改某一项，不是添加新项
            {
                pTempList->pItem->nControllerID = pItem->nControllerID;
                pTempList->pItem->nControllerType = pItem->nControllerType;
                pTempList->pItem->nFlashLightType = pItem->nFlashLightType;

                gCurrentChannelArray[gCurrentChannelNum++] = pItem->nChannelID;
                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PChannelItem)malloc(sizeof(ChannelItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterChanneltem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nChannelID = pItem->nChannelID;
    pTempList->pItem->nControllerID = pItem->nControllerID;
    pTempList->pItem->nControllerType = pItem->nControllerType;
    pTempList->pItem->nFlashLightType = pItem->nFlashLightType;
    
    gCurrentChannelArray[gCurrentChannelNum++] = pItem->nChannelID;

    //为当前结点分配下一空结点
    PChannelList pNewList;
    pNewList = (PChannelList)malloc(sizeof(ChannelList));

    if(!pNewList)
    {
        log_debug("AddOrAlterChanneltem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}



int DeleteChannelItem(PChannelList *pList,PChannelItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PChannelList pTempList = *pList;
    PChannelList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nChannelID == pItem->nChannelID)//找到需要删除的项
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
int ClearChannelList(PChannelList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    (void) DestroyChannelList(pList);//先释放

    return InitChannelList(pList);//再重新初始化

}
int DestroyChannelList(PChannelList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PChannelList pTempList = *pList;
    PChannelList pPriorList = null;

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

/*****************************************************************************
 函 数 名  : GetChannelID
 功能描述  : 在轮询相位时，根据相位的控制类型及相位ID，获得需要执行的通道。
             注意，轮询时应该是轮询相位而不是通道
 输入参数  : PChannelList *pList          
             ControllerType nCtrlType     
             unsigned short ControllerID  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年7月16日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int GetChannelID(PChannelList *pList,ControllerType nCtrlType,unsigned short nControllerID)
{
    if(*pList == null)
    {

        return false;
    }

    PChannelList pTemp = *pList;

    while(pTemp->next != null)
    {
        if((pTemp->pItem->nControllerID == nControllerID) && (pTemp->pItem->nControllerType == nCtrlType))
        {
            return pTemp->pItem->nChannelID;
        }

        pTemp = pTemp->next;
    }
    

    return false;//没有找到符合的



}


int LoadDefaultChannel(PChannelList pList)
{

    if(!pList)
    {
        return 0;
    }

    ChannelItem item;

    memset(&item,0,sizeof(item));

    item.nControllerType = MOTOR;
    item.nFlashLightType = YELLOWLIGHT;

    int i;

    for(i = 1; i <= 6 ; i++)
    {
        item.nChannelID = (unsigned short)(i+12);

        if(i == 1)//第一通道设置跟随
        {
         //   item.nControllerType = FOLLOW;
        }
        else
        {
         //   item.nControllerType = MOTOR;
        }

    
        if((i == 1) || (i == 3) || (i == 5))
        {
            item.nControllerID = 1;
        }
        else if (i == 2)
        {   
            item.nControllerID = 2;
        }
        else
        {
            item.nControllerID = 3;
        }
        
    
        (void) AddOrAlterChanneltem(&pList,&item);


    }


    return 0;





}




