
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

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseTurnID == pItem->nPhaseTurnID)//nPhaseTurnID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                if(pTempList->pItem->nCircleID == pItem->nCircleID)//nCircleIDҲ��ͬ����������޸Ĳ�������
                {
                    memset(pTempList->pItem->nTurnArray,0,sizeof(pTempList->pItem->nTurnArray));
                    memcpy(pTempList->pItem->nTurnArray,pItem->nTurnArray,sizeof(pTempList->pItem->nTurnArray));
                    return true;//�޸���ɺ�ֱ���˳�
                }

            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PPhaseTurnItem)malloc(sizeof(PhaseTurnItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterPhaseTurnItem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nCircleID = pItem->nCircleID;
    pTempList->pItem->nPhaseTurnID = pItem->nPhaseTurnID;
    memset(pTempList->pItem->nTurnArray,0,sizeof(pTempList->pItem->nTurnArray));
    memcpy(pTempList->pItem->nTurnArray,pItem->nTurnArray,sizeof(pTempList->pItem->nTurnArray));

    //Ϊ��ǰ��������һ�ս��
    PPhaseTurnList pNewList;
    pNewList = (PPhaseTurnList)malloc(sizeof(PhaseTurnList));

    if(!pNewList)
    {
        log_debug("AddOrAlterPhaseTurnItem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
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

    //�ҵ����һ�����
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

    return num;//�Ҳ���
}


unsigned short *GetPhaseArray(PPhaseTurnList pList,unsigned short nPhaseTurnID)
{
    if(pList == null)
    {
        return null;
    }

    PPhaseTurnList pTempList = pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem->nPhaseTurnID == nPhaseTurnID)
        {
            return pTempList->pItem->nTurnArray;
        }
        pTempList = pTempList->next;

    }

    return null;//�Ҳ���
}



int DeletePhaseTurnItem(PPhaseTurnList *pList,PPhaseTurnItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PPhaseTurnList pTempList = *pList;
    PPhaseTurnList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nPhaseTurnID == pItem->nPhaseTurnID)//�ҵ���Ҫɾ������
            {
                {
                    pPriorList->next = pTempList->next;

                    free(pTempList->pItem);//���ͷ�item����
                    free(pTempList);//���ͷŽ��
                    
                    return true;//�޸���ɺ�ֱ���˳�
                }
            }
        }
        pPriorList = pTempList;//����ǰһ���ɾ��ʱʹ��
        pTempList = pTempList->next;

    }


    return false;//�Ҳ���

}
int ClearPhaseTurnList(PPhaseTurnList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyPhaseTurnList(pList);//���ͷ�

    return InitPhaseTurnList(pList);//�����³�ʼ��

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

        if(pTempList->pItem != null)//���������ͷ�
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






