
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

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nFollowPhaseID == pItem->nFollowPhaseID)//nFollowPhaseID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                memset(pTempList->pItem,0,sizeof(FollowPhaseItem));

                memcpy(pTempList->pItem,pItem,sizeof(FollowPhaseItem));
                
                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PFollowPhaseItem)malloc(sizeof(FollowPhaseItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterFollowPhasetem error to alloc \n");
        return false;
    }



    //����½ڵ�
    memset(pTempList->pItem,0,sizeof(FollowPhaseItem));
    memcpy(pTempList->pItem,pItem,sizeof(FollowPhaseItem));

    //Ϊ��ǰ��������һ�ս��
    PFollowPhaseList pNewList;
    pNewList = (PFollowPhaseList)malloc(sizeof(FollowPhaseList));

    if(!pNewList)
    {
        log_debug("%s  error to alloc \n",__func__);
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
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

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nFollowPhaseID == pItem->nFollowPhaseID)//�ҵ���Ҫɾ������
            {
                
                pPriorList->next = pTempList->next;

                free(pTempList->pItem);//���ͷ�item����
                free(pTempList);//���ͷŽ��

                return true;//�޸���ɺ�ֱ���˳�
            }
        }
        pPriorList = pTempList;//����ǰһ���ɾ��ʱʹ��
        pTempList = pTempList->next;

    }


    return false;//�Ҳ���

}
int ClearFollowPhaseList(PFollowPhaseList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyFollowPhaseList(pList);//���ͷ�

    return InitFollowPhaseList(pList);//�����³�ʼ��

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

        if(pTempList->pItem != null)//���������ͷ�
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
    memset(&item,0,sizeof(item));//Ĭ������ֵ����0�����Ժ���ֻ��Ҫ�Ѳ���0�Ĳ�����ֵ����


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






