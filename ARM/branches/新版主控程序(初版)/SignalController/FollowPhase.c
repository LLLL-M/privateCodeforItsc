
#include "FollowPhase.h"



int InitFollowPhaseList(PFollowPhaseList *pList)
{
    (*pList) = (PFollowPhaseList)malloc(sizeof(FollowPhaseList));

    if(!(*pList))
    {
        log_debug("%s  error to alloc \n",__func__);
        return false;
    }

    (*pList)->pItem = null;
    (*pList)->next = null;

    return true;
}

int AddOrAlterFollowPhaseItem(PFollowPhaseList *pList,PFollowPhaseItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PFollowPhaseList pTempList = *pList;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nFollowPhaseID == pItem->nFollowPhaseID)//nFollowPhaseID号已存在且相同，表明是修改某一项，不是添加新项
            {
                memset(pTempList->pItem,0,sizeof(FollowPhaseItem));

                memcpy(pTempList->pItem,pItem,sizeof(FollowPhaseItem));
                
                return true;//修改完成后直接退出
            }
        }

        pTempList = pTempList->next;

    }

    //为结点分配空间
    pTempList->pItem = (PFollowPhaseItem)malloc(sizeof(FollowPhaseItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterFollowPhasetem error to alloc \n");
        return false;
    }



    //填充新节点
    memset(pTempList->pItem,0,sizeof(FollowPhaseItem));
    memcpy(pTempList->pItem,pItem,sizeof(FollowPhaseItem));

    //为当前结点分配下一空结点
    PFollowPhaseList pNewList;
    pNewList = (PFollowPhaseList)malloc(sizeof(FollowPhaseList));

    if(!pNewList)
    {
        log_debug("%s  error to alloc \n",__func__);
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //指向新空结点
    pTempList->next = pNewList;


    return true;

}



int DeleteFollowPhaseItem(PFollowPhaseList *pList,PFollowPhaseItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PFollowPhaseList pTempList = *pList;
    PFollowPhaseList pPriorList = null;

    //找到最后一个结点
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nFollowPhaseID == pItem->nFollowPhaseID)//找到需要删除的项
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
int ClearFollowPhaseList(PFollowPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyFollowPhaseList(pList);//先释放

    return InitFollowPhaseList(pList);//再重新初始化

}
int DestroyFollowPhaseList(PFollowPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    PFollowPhaseList pTempList = *pList;
    PFollowPhaseList pPriorList = null;

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


int LoadDefaultFollowPhase(PFollowPhaseList pList)
{
    if(!pList)
    {
        return 0;
    }

    FollowPhaseItem item;
    memset(&item,0,sizeof(item));//默认所有值都是0，所以后面只需要把不是0的参数赋值即可


    item.nFollowPhaseID = 1;
    item.nArrayMotherPhase[0] = 1;
    item.nArrayMotherPhase[1] = 2;


    return AddOrAlterFollowPhaseItem(&pList,&item);

}

static int IsVarInArray(unsigned short *array,unsigned short len,unsigned short val)
{
    int i = 0;

    if(!array)
    {
        return false;
    }


    for(i = 0 ; i < len; i++)
    {
        if(array[i] == val)
        {
            return true;
        }


    }
    return false;

}

int IsPhaseInFollowPhase(PFollowPhaseList pList,unsigned short nPhaseId,unsigned short nFollowPhaseId)
{
    if(!pList)
    {
        return 0;

    }
    PFollowPhaseList pTempList = pList;

    while(pTempList->next != null)
    {
        if(pTempList->pItem->nFollowPhaseID == nFollowPhaseId)
        {
            if(IsVarInArray(pTempList->pItem->nArrayMotherPhase, MAX_FOLLOW_PHASE_MOTHER_NUM, nPhaseId) == true)
            {
                return true;
            }

        }
        
        pTempList = pTempList->next;

    }

    return false;

}






