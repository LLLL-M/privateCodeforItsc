/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : TimeInterval.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : ������Ҫʵ����ʱ�α����ؽӿڣ���Ҫ������ʼ��ʱ�α���ӡ�
               ɾ������ա�����ʱ�α�����ʱ�α�Ų��Ҷ������
  �����б�   :
              AddOrAlterTimeIntervalItem
              ClearTimeIntervalList
              DeleteTimeIntervalItem
              DestroyTimeIntervalList
              GetActionID
              InitTimeIntervalList
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/


#include "TimeInterval.h"


/*****************************************************************************
 �� �� ��  : InitTimeIntervalList
 ��������  : ��ʼ��ʱ�α�
 �������  : PTimeIntervalList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddOrAlterTimeIntervalItem
 ��������  : ��ӻ��޸�ָ����ʱ�α���
 �������  : PTimeIntervalList *pList  
             PTimeIntervalItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int AddOrAlterTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PTimeIntervalList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nTimeIntervalID == pItem->nTimeIntervalID)//nTimeIntervalID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                if(pTempList->pItem->nTimeID == pItem->nTimeID)//ʱ�κ�Ҳ��ͬ����������޸Ĳ�������
                {
                    pTempList->pItem->nActionID = pItem->nActionID;
                    pTempList->pItem->cStartTimeHour = pItem->cStartTimeHour;
                    pTempList->pItem->cStartTimeMinute = pItem->cStartTimeMinute;

                    return true;//�޸���ɺ�ֱ���˳�
                }

            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PTimeIntervalItem)malloc(sizeof(TimeIntervalItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterActionItem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nActionID = pItem->nActionID;
    pTempList->pItem->nTimeIntervalID = pItem->nTimeIntervalID;
    pTempList->pItem->nTimeID = pItem->nTimeID;
    pTempList->pItem->cStartTimeHour = pItem->cStartTimeHour;
    pTempList->pItem->cStartTimeMinute = pItem->cStartTimeMinute;

    //Ϊ��ǰ��������һ�ս��
    PTimeIntervalList pNewList;
    pNewList = (PTimeIntervalList)malloc(sizeof(TimeIntervalList));

    if(!pNewList)
    {
        log_debug("AddOrAlterActionItem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}
/*****************************************************************************
 �� �� ��  : DeleteTimeIntervalItem
 ��������  : ɾ��ĳ��ʱ�α���
 �������  : PTimeIntervalList *pList  
             PTimeIntervalItem pItem   
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DeleteTimeIntervalItem(PTimeIntervalList *pList,PTimeIntervalItem pItem)
{
    if(*pList == null)
    {
        return false;
    }

    PTimeIntervalList pTempList = *pList;
    PTimeIntervalList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nTimeIntervalID == pItem->nTimeIntervalID)//�ҵ���Ҫɾ������
            {
                if(pTempList->pItem->nTimeID == pItem->nTimeID)
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

static int HoursToMinutes(int hours,int minutes)
{

    return hours*60+minutes;

}



/*****************************************************************************
 �� �� ��  : GetActionID
 ��������  : ����ʱ�α�ż���ǰʱ��ȷ����ǰӦ������Ķ�����
 �������  : PTimeIntervalList *pList        
             unsigned short nTimeIntervalID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
        if(pTemp->pItem->nTimeIntervalID == nTimeIntervalID)//����Ҫ�ҵ����ʱ�α��ΪnTimeIntervalID�ı�Ȼ���ٸ���ʱ��ȷ��������
        {
            nTempTime = HoursToMinutes(pTemp->pItem->cStartTimeHour, pTemp->pItem->cStartTimeMinute);
            //log_debug("==> Hour  %d  Minute  %d\n",pTemp->pItem->cStartTimeHour,pTemp->pItem->cStartTimeMinute);
            if(flag == 0)
            {
                flag = 1;
                nTempMin = nTempTime;
                nTempMax = nTempTime;
            }
            
            if(nTempTime >= nTempMax)//�����ǰʱ���temp��Ҫ�󣬾ͼ�¼��ǰ��ACTION
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

    //�����ǰ��Сֵ�����ֵ��ȣ���˵����ǰֻ��һ���������ô��һ�������Ҫ�ҵ�ActionId
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

    
    //�õ�ǰʱ��͵�ǰ�����е����ֵ����Сֵ�Ƚϣ������ǰ����СֵС���Ǿ�ȡ���ֵʱ��actionId

    nTempTime = HoursToMinutes(GetHour(), GetMin());

    if(nTempTime < nTempMin)
    {
        return nActionId;
    }
    else//�������Сֵ������Ҫ���±��������ҵ���������
    {
        flag = 0;
        int nCurrentTime = 0;
        int nMinusTime = 0;
        nCurrentTime = HoursToMinutes(GetHour(), GetMin());
        while(pTemp->next != null)
        {
            if(pTemp->pItem->nTimeIntervalID == nTimeIntervalID)//����Ҫ�ҵ����ʱ�α��ΪnTimeIntervalID�ı�Ȼ���ٸ���ʱ��ȷ��������
            {
                nTempTime = HoursToMinutes(pTemp->pItem->cStartTimeHour, pTemp->pItem->cStartTimeMinute);
                
                if((flag == 0) && (nCurrentTime > nTempTime))
                {
                    flag = 1;
                    nMinusTime = nCurrentTime - nTempTime;
                    nActionId = pTemp->pItem->nActionID;
                    continue;
                }
                
                if((nCurrentTime > nTempTime)&&(nCurrentTime - nTempTime < nMinusTime))//�����ǰ����0���Ҳ�ֵ��С���ͼ�¼��ǰ��ACTION
                {
                    nMinusTime = nCurrentTime - nTempTime;
                    nActionId = pTemp->pItem->nActionID;
                }

            }

            pTemp = pTemp->next;
        }

    }


    return nActionId;//û���ҵ����ϵ�



}

/*****************************************************************************
 �� �� ��  : ClearTimeIntervalList
 ��������  : ���ʱ�α�����
 �������  : PTimeIntervalList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int ClearTimeIntervalList(PTimeIntervalList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyTimeIntervalList(pList);//���ͷ�

    return InitTimeIntervalList(pList);//�����³�ʼ��

}

/*****************************************************************************
 �� �� ��  : DestroyTimeIntervalList
 ��������  : ����ʱ�α�ʱ�α��޷�ʹ��
 �������  : PTimeIntervalList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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

        if(pTempList->pItem != null)//���������ͷ�
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



