/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : ThreadController.c
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��26��
  ����޸�   :
  ��������   : ���ﶨ���˼���߳�״̬�Ľӿ�ʵ��
  �����б�   :
              AddControllerItem
              DestroyControllerList
              InitControllerList
              MainControllerFun
  ����˵��:
  ���ε���InitControllerList,AddControllerItem,MainControllerFun,DestroyControllerList
              
  �޸���ʷ   :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

******************************************************************************/

#include "ThreadController.h"


static PMyIntLinkList  pControllerList = NULL; 
static pthread_mutex_t g_lock_list;
static int g_sleep_ms = 100;//ms
/*****************************************************************************
 �� �� ��  : InitControllerList
 ��������  : ��ʼ���߳̿�������������׽�߳�״̬
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddControllerItem
 ��������  : ����Ҫ��ص��߳�ID��ӵ��߳̿������С�
 �������  : int item  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : DestroyControllerList
 ��������  : �����߳̿�����
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : MainControllerFun
 ��������  : �ʺ���main�����е��õ��߳̿�������ں���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int MainThreadControllerFun()
{
    PMyIntLinkList pTempList = null;
    PMyIntLinkList pTempListPrior = null;//����ɾ��itemʱ�����ǰһ��
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





