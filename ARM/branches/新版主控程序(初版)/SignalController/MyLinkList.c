/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : MyLinkList.c
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��26��
  ����޸�   :
  ��������   : ������Ҫ�����˵��������һЩ�ӿ�ʵ�֣���Ӧ���ڶ��߳�ʱ������
               ����ע��������̰߳�ȫ����ʩ��֧����չ���ɸ�����Ҫ�����µ�����
               ����
  �����б�   :
              AddLinkList
              DelLinkListNode
              DestoryLinkList
              GetLinkListLength
              InitLinkList
              PrintLinkListNode
              ReverseLinkList
  �޸���ʷ   :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧��
******************************************************************************/


#include "MyLinkList.h"

//#define DEBUG       //������main��������
//#define DEBUG_INT   //������,main�����г��������������
//#define DEBUG_STR   //���Դ�������



/*****************************************************************************
 �� �� ��  : InitLinkList
 ��������  : ��ʼ��
 �������  : void **list    
             ListType type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��25��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧��
*****************************************************************************/
int InitLinkList(void **list,ListType type)
{

    switch(type)
    {
        case STRING_LIST:
            {

                *list = (PMyStringLinkList)malloc(sizeof(MyStringLinkList));//����ͷ���ռ�

                if(!*list)
                {
                    log_debug("%s  malloc error %s\n",__func__,strerror(errno));
                    return false;
                }

                memset(*list,0,sizeof(MyStringLinkList));
                
                ((PMyStringLinkList)(*list))->next = null;//ͷ���ָ���
                ((PMyStringLinkList)(*list))->val = null;//���������ô���������ڵ��ˣ�ֱ�ӵ�һ������ʵ�ʵ�����
                
                break;
            }
            

       
        case INT_LIST:
            {

                *list = (PMyIntLinkList)malloc(sizeof(MyIntLinkList));//����ͷ���ռ�

                if(!*list)
                {
                    log_debug("%s  malloc error %s\n",__func__,strerror(errno));
                    return false;
                }

                memset(*list,0,sizeof(MyIntLinkList));
                
                ((PMyIntLinkList)(*list))->next = null;//ͷ���ָ���
                ((PMyIntLinkList)(*list))->val = 0;//ͷ����val��ʾ��ͷ�����Ľڵ������Ҳ����ʵ������ĳ���(������ͷ���)
                
                break;
            } 
        
        default:    return false;
    }

    return true;
}

//
/*****************************************************************************
 �� �� ��  : AddLinkList
 ��������  : ��β������µ�����ڵ㣬ͷ����val�ֶ�ֻ����Ϊ������ʹ�ã���
             ʵ�ʴ洢����
 �������  : void **list    
             ListType type  
             void *addData  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧��
*****************************************************************************/
int AddLinkList(void **list,ListType type,void *addData)
{
    switch(type)
    {
        case INT_LIST:
        {
            PMyIntLinkList tempLinkList = null;
            

            if(*list == null)
            {
                return false;
            }
            tempLinkList = (PMyIntLinkList)*list;
            
            //get the last node:
            while(tempLinkList->next != null)
            {
                tempLinkList = tempLinkList->next;
            }
            
            //new node
            PMyIntLinkList tempNewList = null;
            tempNewList = (PMyIntLinkList)malloc(sizeof(MyIntLinkList));//���������Ҫ����ӵĽڵ�
            if(!tempNewList)
            {
                PRINT_ERROR;
                return false;
            }
            
            memset(tempNewList,0,sizeof(MyIntLinkList));
            tempNewList->next = null;
            tempNewList->val = *(int *)addData;//Ϊ�½ڵ㸳ֵ����ʼ��

            //log_debug("AddLinkList   %p   ===>   0x%x\n",tempNewList,tempNewList->val);

            tempLinkList->next = tempNewList;//�ϸ��ڵ�ָ��ǰ����ӵĽڵ�
            ((PMyIntLinkList)(*list))->val += 1;//ÿ������ӽڵ㣬ͷ�ڵ��ϵ�val��Ҫ��1����ʾ��������
            
            break;
        }
        case STRING_LIST://�������������������в�ͬ:����ҪΪ����Ĵ�����ռ䣬�ͷ�ʱ��Ҫfree����ռ�
        {
            PMyStringLinkList tempLinkList = null;

            if(*list == null)
            {
                return false;
            }
            tempLinkList = (PMyStringLinkList)*list;
            
            //get the last node:
            while(tempLinkList->next != null)
            {
                tempLinkList = tempLinkList->next;
            }

            //Ϊ����ڵ��val�����ڴ�ռ䣬�������ݴ洢�ڸýڵ�
            tempLinkList->val = (char *)malloc(strlen(addData));
            if(!tempLinkList->val)
            {
                PRINT_ERROR;
                return false;
            }

            memset(tempLinkList->val,0,strlen(addData));
            strcpy(tempLinkList->val,addData);

            //log_debug("AddLinkList  %p  ==>  %s\n",tempLinkList,tempLinkList->val);
            
            //new node
            PMyStringLinkList tempNewList = null;
            tempNewList = (PMyStringLinkList)malloc(sizeof(MyStringLinkList));//���������Ҫ����ӵĽڵ�
            if(!tempNewList)
            {
                PRINT_ERROR;
                return false;
            }
            
            memset(tempNewList,0,sizeof(MyStringLinkList));
            tempNewList->next = null;
            tempNewList->val = null;//Ϊ�½ڵ㸳ֵ����ʼ��

            tempLinkList->next = tempNewList;//�ϸ��ڵ�ָ��ǰ����ӵĽڵ�
            
            break;
        }
        default: return false;
    }

    return true;

}    
/*****************************************************************************
 �� �� ��  : DelLinkListNode
 ��������  : �ýӿڲ�ʵ�ã���ʱ����֮��
 �������  : void **list    
             ListType type  
             void *delNode  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int DelLinkListNode(void **list,ListType type,void *delNode)
{
    return false;
    switch(type)
    {
        case INT_LIST:
        {
            PMyIntLinkList LinkList = (PMyIntLinkList)*list;
            PMyIntLinkList tempLinkList;

            if(LinkList == null)
            {
                return false;
            }
            //find the node
            while(LinkList->next != null)
            {
                if(LinkList == (PMyIntLinkList)delNode)
                {
                    tempLinkList->next = LinkList->next;
                    break;
                }

                tempLinkList = LinkList;
                LinkList = LinkList->next;

            }
            
            break;
        }
        case STRING_LIST:
        {

            break;
        }
        default:    return false;
    }


    return true;
}    

/*****************************************************************************
 �� �� ��  : GetLinkListLength
 ��������  : �ýӿڲ�ʵ�ã���ʱ����֮��
 �������  : void *list     
             ListType type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int GetLinkListLength(void *list,ListType type)
{
    return 0;

}
/*****************************************************************************
 �� �� ��  : ReverseLinkList
 ��������  : ��ת�����һ��ʵ��
 �������  : void **list    
             ListType type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 ����˼·:
����1�ǵ�1����㣬4��β�ڵ�
ͷ -> 1 -> 2 -> 3 -> 4 -> null

ͷ�ڵ�ֻ�洢������ĳ��ȼ�ָ��ĵ�1���ڵ㣬������ʵ�ʵĽ������̣�ֻ���ڽ����������޸���ָ�򼴿ɣ����ԣ�ʵ�ʲ������������:

1 -> 2 -> 3 -> 4 -> null

����������:
�ʼ�����������: (1) -> 2 -> 3 -> 4 -> null  ÿ�ν������԰��ѽ�����ɵ���Ϊ1�����壬��������и�ͷhead��βtail
��һ��: (2 -> 1) -> 3 -> 4 -> null ÿһ���Ȼ��β����һ��ָ�򲢱��档pTemp = pListTail->next;
�ڶ���: (3 -> 2 -> 1) -> 4 -> null Ȼ���ٽ��������һ��ָ���ָ��ֵ��β��ָ��pListTail->next = pTemp->next;
������: 4 -> 3 -> 2 -> 1 -> null Ȼ���ٽ��ýڵ��ָ��ֵΪhead�����޸�heaeΪ��ǰ�ڵ��൱�ڸýڵ���head��pTemp->next = pListHead;pListHead = pTemp;

������ɺ��޸�ͷָ���ָ��:
ͷ -> 4 ��OK�ˡ�

 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧�֡�
*****************************************************************************/
int ReverseLinkList(void **list,ListType type)
{
    switch(type)
    {
        case INT_LIST:
        {
            PMyIntLinkList LinkList = (PMyIntLinkList)*list;

            if((*list) == null)
            {
                return false;
            }

            LinkList = LinkList->next;//ͷ����������洢�����ȼ�ͷ���ģ����Բ����뽻������������һ����ʼ

            PMyIntLinkList pListHead = LinkList;//��Զ��ͷ��㣬�ñ�����Զָ�������ͷ�����������Ϊ��1���ڵ㣬�м��ѽ����Ľڵ㲻����
            PMyIntLinkList pListTail = LinkList;//��ʼ״̬��ͷ��β����ͬ�ģ��ñ�����Զָ���ѽ��������β�����������Ϊ��2���ڵ�
            PMyIntLinkList pTemp = null;//������ʱ�����2���ڵ�
           
            while(pListTail->next != null)
            {
                pTemp = pListTail->next;//��õ�2���ڵ�
                pListTail->next = pTemp->next;//
                pTemp->next = pListHead;
                pListHead = pTemp;
                log_debug("ReverseLinkList  head  %p   tail  %p\n",pListHead,pListTail);
            }
            
            ((PMyIntLinkList)*list)->next = pListHead;
            
            break;
        }
        case STRING_LIST:
        {
            PMyStringLinkList LinkList = (PMyStringLinkList)*list;

            if((*list) == null)
            {
                return false;
            }

            //�����Ǻ�int����ͬ�ĵط���������û��ͷ�ڵ�泤��֮˵���ʽ���Ӧ��ͷ�ڵ㿪ʼ��
            //LinkList = LinkList->next;//ͷ����������洢�����ȼ�ͷ���ģ����Բ����뽻������������һ����ʼ

            PMyStringLinkList pListHead = LinkList;//��Զ��ͷ��㣬�ñ�����Զָ�������ͷ�����������Ϊ��1���ڵ㣬�м��ѽ����Ľڵ㲻����
            PMyStringLinkList pListTail = LinkList;//��ʼ״̬��ͷ��β����ͬ�ģ��ñ�����Զָ���ѽ��������β�����������Ϊ��2���ڵ�
            PMyStringLinkList pTemp = null;//������ʱ�����2���ڵ�
           
            while(pListTail->next != null)
            {
                pTemp = pListTail->next;//��õ�2���ڵ�
                pListTail->next = pTemp->next;//
                pTemp->next = pListHead;
                pListHead = pTemp;
                log_debug("ReverseLinkList  head  %p   tail  %p\n",pListHead,pListTail);
            }
            
            (*list) = pListHead;
            
            break;
        }
        default:    return false;
    }


    

    return true;

}

/*****************************************************************************
 �� �� ��  : PrintLinkListNode
 ��������  : ��ӡ��������
 �������  : void **list    
             ListType type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧�֡�

*****************************************************************************/
int PrintLinkListNode(void **list,ListType type)
{
    switch(type)
    {
        case INT_LIST:
        {
            PMyIntLinkList LinkList = (PMyIntLinkList)*list;

            if((*list) == null)
            {
                return false;
            }
            log_debug("PrintLinkListNode  The Lenght of the List is %p ==> %d\n",LinkList,LinkList->val);
            LinkList = LinkList->next;//ͷ�ڵ㲻������ӵڶ����ڵ㿪ʼ������������Ч����
            
            //find the node
            while(LinkList != null)
            {
                log_debug("PrintLinkListNode   %p  ==>   %d\n",LinkList,LinkList->val);
            
                LinkList = LinkList->next;

            }       
            
            break;
        }
        case STRING_LIST:
        {
            PMyStringLinkList LinkList = (PMyStringLinkList)*list;

            if((*list) == null)
            {
                return false;
            }
            //same reason as above
            //LinkList = LinkList->next;//ͷ�ڵ㲻������ӵڶ����ڵ㿪ʼ������������Ч����

            while(LinkList != null)
            {
                log_debug("PrintLinkListNode   %p  ==>   %s\n",LinkList,LinkList->val);
            
                LinkList = LinkList->next;

            }       

            break;
        }
        default:    return false;
    }


    

    return true;

}
/*****************************************************************************
 �� �� ��  : DestoryLinkList
 ��������  : ���������ʵ�ַ���
 �������  : void **list    
             ListType type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��ӶԴ������֧�֡�
*****************************************************************************/
int DestoryLinkList(void **list,ListType type)
{
    switch(type)
    {
        case INT_LIST:
        {
            PMyIntLinkList LinkList = (PMyIntLinkList)*list;
            PMyIntLinkList tempLinkList;

            if(LinkList == null)
            {
                return false;
            }
            //find the node
            while(LinkList != null)
            {
                tempLinkList = LinkList->next;
            
                log_debug("DestoryLinkList   %p  ==>   %d\n",LinkList,LinkList->val);
                free(LinkList);
            
                LinkList = tempLinkList;

            }            
            break;
        }
        case STRING_LIST:
        {
            PMyStringLinkList LinkList = (PMyStringLinkList)*list;
            PMyStringLinkList tempLinkList;

            if(LinkList == null)
            {
                return false;
            }
            //find the node
            while(LinkList != null)
            {
                tempLinkList = LinkList->next;
            
                log_debug("DestoryLinkList   %p  ==>   %p\n",LinkList,LinkList->val);
                if(LinkList->val)
                {   
                    free(LinkList->val);//����ע�⣬���ڴ�������ѣ�����Ҳ����Ҫ�ͷŵġ�
                }
                
                free(LinkList);//����˵�����̼߳������ڴ��ͷŲ� ���ع�OS:http://www.cnblogs.com/raymondshiquan/archive/2011/06/25/tcmalloc_configuration_analysis.html#2245126
            
                LinkList = tempLinkList;

            }            
            break;
        }
        default:    return false;
    }


    return true;

}


/*****************************************************************************
 �� �� ��  : main
 ��������  : �Ա��������ӿڵĲ��ԣ�Ҳ��һ��ʹ������Ĳ���
 �������  : int argc     
             char **argv  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��26��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : ��Ӳ��Դ�����
*****************************************************************************/
#ifdef DEBUG
int main(int argc,char **argv)
{
    PMyIntLinkList list = null;

#ifdef DEBUG_INT

    InitLinkList((void **)&list, INT_LIST);

    int i = 0;

    for(i ; i < 5; i++)
    {
        AddLinkList((void **)&list,INT_LIST,&i);

    }

    PrintLinkListNode((void **)&list,INT_LIST);

    ReverseLinkList((void **)&list,INT_LIST);

    PrintLinkListNode((void **)&list,INT_LIST);

    DestoryLinkList((void **)&list,INT_LIST);
    
#endif

#ifdef DEBUG_STR
    InitLinkList((void **)&list,STRING_LIST);

    int i = 0;

    char *StringArray[] = {
        "aaaaa",
        "bbbbb",
        "ccccc",
        "ddddd",
        "eeeee",
        "fffff",
    };

    for(i ; i < 6 ; i++)
    {
        AddLinkList((void **)&list,STRING_LIST,StringArray[i]);

    }

    PrintLinkListNode((void **)&list,STRING_LIST);

    ReverseLinkList((void **)&list,STRING_LIST);

    PrintLinkListNode((void **)&list,STRING_LIST);

    DestoryLinkList((void **)&list,STRING_LIST);    

#endif
    return 0;

}
#endif

