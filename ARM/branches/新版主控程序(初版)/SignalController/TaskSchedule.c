/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : TaskSchedule.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��4��
  ����޸�   :
  ��������   : ������Ҫʵ���˵�����صĽӿڣ��ȵ���Init�����������ſ��Խ���
               ��ɾ�Ĳ�����������Ҫdestroy�������
  �����б�   :
              AddOrAlterTaskScheduleItem
              ClearTaskSchedule
              DeleteTaskScheduleItem
              DestroyTaskSchedule
              GetTimeIntervalIndex
              InitTaskSchedule
  �޸���ʷ   :
  1.��    ��   : 2014��7��4��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#include "TaskSchedule.h"



/*****************************************************************************
 �� �� ��  : InitTaskSchedule
 ��������  : ��ʼ����������
 �������  : PPlanScheduleList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddOrAlterTaskScheduleItem
 ��������  : �����µĵ��Ȼ����޸�ĳһ��ָ������
 �������  : PPlanScheduleList *pList  
             PPlanScheduleItem item    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int AddOrAlterTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item)
{
    if(*pList == null)
    {
        return false;
    }

    PPlanScheduleList pTempList = *pList;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->item != null)
        {
            if(pTempList->item->nScheduleID == item->nScheduleID)//���ȼƻ����Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->item->nTimeIntervalID = item->nTimeIntervalID;
                memset(&pTempList->item->planTime,0,sizeof(PlanTime));
                memcpy(&pTempList->item->planTime,&item->planTime,sizeof(item->planTime));

                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->item = (PPlanScheduleItem)malloc(sizeof(PlanScheduleItem));

    if(pTempList->item == null)
    {
        log_error("AddOrAlterTaskScheduleItem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->item->nScheduleID = item->nScheduleID;
    pTempList->item->nTimeIntervalID = item->nTimeIntervalID;
    memset(&pTempList->item->planTime,0,sizeof(PlanTime));
    memcpy(&pTempList->item->planTime,&item->planTime,sizeof(item->planTime));

    //log_debug("........   %d ....\n",pTempList->item->nTimeIntervalID);

    //Ϊ��ǰ��������һ�ս��
    PPlanScheduleList pNewList;
    pNewList = (PPlanScheduleList)malloc(sizeof(PlanScheduleList));

    if(!pNewList)
    {
        log_debug("AddOrAlterTaskScheduleItem  error to alloc \n");
        return false;
    }

    pNewList->item = null;
    pNewList->next = null;    

    //ָ���¿ս��
    pTempList->next = pNewList;


    return true;

}

/*****************************************************************************
 �� �� ��  : DeleteTaskScheduleItem
 ��������  : ������Ҫ��ɾ��ָ����ĳһ�����
 �������  : PPlanScheduleList *pList  
             PPlanScheduleItem item    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��4��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int DeleteTaskScheduleItem(PPlanScheduleList *pList,PPlanScheduleItem item)
{
    if(*pList == null)
    {
        return false;
    }

    PPlanScheduleList pTempList = *pList;
    PPlanScheduleList pPriorList = null;

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->item != null)
        {
            if(pTempList->item->nScheduleID == item->nScheduleID)//�ҵ���Ҫɾ������
            {
                
                pPriorList->next = pTempList->next;

                free(pTempList->item);//���ͷ�item����
                free(pTempList);//���ͷŽ��

                return true;//�޸���ɺ�ֱ���˳�
            }
        }
        pPriorList = pTempList;//����ǰһ���ɾ��ʱʹ��
        pTempList = pTempList->next;

    }


    return false;//�Ҳ���

}

/*****************************************************************************
 �� �� ��  : GetTimeIntervalIndex
 ��������  : ���ݱ��ص�ǰʱ�䣬�õ���ǰʱ�α�ţ�ͨ��ʱ�α���ٲ�ѯ��ǰʱ��
             �Ķ���
 �������  : PPlanScheduleList *pList  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
        if(IsItemInIntArray(pTemp->item->planTime.month, sizeof(pTemp->item->planTime.month), GetMonth()) == true)//�·ݷ���
        {
            if(IsItemInIntArray(pTemp->item->planTime.day,sizeof(pTemp->item->planTime.day), GetDay()) == true)//���ڷ���
            {
                if(IsItemInIntArray(pTemp->item->planTime.week,sizeof(pTemp->item->planTime.week), GetWeek()) == true)//���ڷ���
                {
                    return pTemp->item->nTimeIntervalID;//ֱ�ӷ���ʱ�α��
                }
            }
        }

        pTemp = pTemp->next;
    }
    

    return false;//û���ҵ����ϵ�



}

/*****************************************************************************
 �� �� ��  : ClearTaskSchedule
 ��������  : ��յ��ȱ����ݣ��´ο�������µĵ��ȱ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

*****************************************************************************/
int ClearTaskSchedule(PPlanScheduleList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    DestroyTaskSchedule(pList);//���ͷ�

    

    return InitTaskSchedule(pList);//�����³�ʼ��

}

/*****************************************************************************
 �� �� ��  : DestroyTaskSchedule
 ��������  : ���ٵ��ȱ����ٺ󣬽��޷��ٴ�����µ������ϵͳ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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

        if(pTempList->item != null)//���������ͷ�
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


