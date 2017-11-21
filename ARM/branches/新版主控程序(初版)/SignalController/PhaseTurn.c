
#include "Util.h"
#include "PhaseTurn.h"

int InitPhaseTurnList(PPhaseTurnList *pList)
{
    (*pList) = (PPhaseTurnList)malloc(sizeof(PhaseTurnList));

    if(!(*pList))
    {
        log_debug("InitPhaseTurnList  error to alloc \n");
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    log_debug("%s succeed \n",__func__);

    return true;
}



int AddOrAlterPhaseTurnItem(PPhaseTurnList *pList,PPhaseTurnItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseTurnList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseTurnID == pItem->nPhaseTurnID)//nPhaseTurnID号已存在且相同，表明是修改某一项，不是添加新项
            {
                if(pTempList->pItem->nCircleID == pItem->nCircleID)//nCircleID也相同，则表明是修改不是新增
                {
                    memset(pTempList->pItem->nTurnArray,0,sizeof(pTempList->pItem->nTurnArray));
                    memcpy(pTempList->pItem->nTurnArray,pItem->nTurnArray,sizeof(pTempList->pItem->nTurnArray));
                    return true;//修改完成后直接退出
                }

            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PPhaseTurnItem)malloc(sizeof(PhaseTurnItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterPhaseTurnItem error to alloc \n");
        return false;
    }



    //填充新节点
    pTempList->pItem->nCircleID = pItem->nCircleID;
    pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;
    memset(pTempList->pItem->nTurnArray,0,sizeof(pTempList->pItem->nTurnArray));
    memcpy(pTempList->pItem->nTurnArray,pItem->nTurnArray,sizeof(pTempList->pItem->nTurnArray));

    //为当前结点分配下一空结点
    PPhaseTurnList pNewList;
    pNewList = (PPhaseTurnList)malloc(sizeof(PhaseTurnList));

    if(!pNewList)
    {
        log_debug("AddOrAlterPhaseTurnItem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}

int GetPhaseNum(PPhaseTurnList pList,unsigned short nPhaseTurnID)
{
    if(pList == null)
    {
        return false;
    }

    PPhaseTurnList pTempList = pList;
    int i = 0;
    int num = 0;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem->nPhaseTurnID == nPhaseTurnID)
        {
            for(i = 0 ; i < 16 ;i++)
            {
                if(pTempList->pItem->nTurnArray[i] != 0)
                {
                    num++;
                }

            }

            break;
            
        }
        pTempList = pTempList->next;

    }

    return num;//找不到
}


unsigned short *GetPhaseArray(PPhaseTurnList pList,unsigned short nPhaseTurnID)
{
    if(pList == null)
    {
        return null;
    }

    PPhaseTurnList pTempList = pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem->nPhaseTurnID == nPhaseTurnID)
        {
            return pTempList->pItem->nTurnArray;
        }
        pTempList = pTempList->next;

    }

    return null;//找不到
}



int DeletePhaseTurnItem(PPhaseTurnList *pList,PPhaseTurnItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseTurnList pTempList = *pList;
    PPhaseTurnList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseTurnID == pItem->nPhaseTurnID)//找到需要删除的项
            {
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
int ClearPhaseTurnList(PPhaseTurnList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyPhaseTurnList(pList);//先释放

    return InitPhaseTurnList(pList);//再重新初始化

}

int DestroyPhaseTurnList(PPhaseTurnList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseTurnList pTempList = *pList;
    PPhaseTurnList pPriorList = null;

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


int LoadDefaultPhaseTurnList(PPhaseTurnList pList)
{
    if(!pList)
    {
        return 0;

    }

    PhaseTurnItem item;
    memset(&item,0,sizeof(item));

    item.nPhaseTurnID = 1;
    item.nCircleID = 1;
    item.nTurnArray[0] = 3;
    item.nTurnArray[1] = 1;
    item.nTurnArray[2] = 2;

    return AddOrAlterPhaseTurnItem(&pList,&item);
}






