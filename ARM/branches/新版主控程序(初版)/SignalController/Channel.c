/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Channel.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : ������Ҫʵ����ͨ�����һϵ�нӿں�����
  �����б�   :
              AddOrAlterChanneltem
              ClearChannelList
              DeleteChannelItem
              DestroyChannelList
              InitChannelList
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/

#include "Channel.h"

unsigned short gCurrentChannelArray[MAX_CHANNEL_NUM] = {0};//��ǰ�����£��ܹ�ʹ�õ�ͨ�������飬�����������βΣ����ݸ���ƺ���
unsigned short gCurrentChannelNum = 0;//��ǰ�����£��ܹ�ʹ�õ�ͨ������

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

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nChannelID == pItem->nChannelID)//nChannelID���Ѵ�������ͬ���������޸�ĳһ������������
            {
                pTempList->pItem->nControllerID = pItem->nControllerID;
                pTempList->pItem->nControllerType = pItem->nControllerType;
                pTempList->pItem->nFlashLightType = pItem->nFlashLightType;

                gCurrentChannelArray[gCurrentChannelNum++] = pItem->nChannelID;
                return true;//�޸���ɺ�ֱ���˳�
            }
        }

        pTempList = pTempList->next;

    }

    //Ϊ������ռ�
    pTempList->pItem = (PChannelItem)malloc(sizeof(ChannelItem));

    if(pTempList->pItem == null)
    {
        log_error("AddOrAlterChanneltem error to alloc \n");
        return false;
    }



    //����½ڵ�
    pTempList->pItem->nChannelID = pItem->nChannelID;
    pTempList->pItem->nControllerID = pItem->nControllerID;
    pTempList->pItem->nControllerType = pItem->nControllerType;
    pTempList->pItem->nFlashLightType = pItem->nFlashLightType;
    
    gCurrentChannelArray[gCurrentChannelNum++] = pItem->nChannelID;

    //Ϊ��ǰ��������һ�ս��
    PChannelList pNewList;
    pNewList = (PChannelList)malloc(sizeof(ChannelList));

    if(!pNewList)
    {
        log_debug("AddOrAlterChanneltem  error to alloc \n");
        return false;
    }

    pNewList->pItem = null;
    pNewList->next = null;    

    //ָ���¿ս��
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

    //�ҵ����һ�����
    while(pTempList->next != null)
    {
        if(pTempList->pItem != null)
        {
            if(pTempList->pItem->nChannelID == pItem->nChannelID)//�ҵ���Ҫɾ������
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
int ClearChannelList(PChannelList *pList)
{
    if(*pList == null)
    {
        return false;
    }

    (void) DestroyChannelList(pList);//���ͷ�

    return InitChannelList(pList);//�����³�ʼ��

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

        if(pTempList->pItem != null)//���������ͷ�
        {
            free(pTempList->pItem);
        }

        free(pTempList);
        
        pTempList = pPriorList;

    }


    return true;

}

/*****************************************************************************
 �� �� ��  : GetChannelID
 ��������  : ����ѯ��λʱ��������λ�Ŀ������ͼ���λID�������Ҫִ�е�ͨ����
             ע�⣬��ѯʱӦ������ѯ��λ������ͨ��
 �������  : PChannelList *pList          
             ControllerType nCtrlType     
             unsigned short ControllerID  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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
    

    return false;//û���ҵ����ϵ�



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

        if(i == 1)//��һͨ�����ø���
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




