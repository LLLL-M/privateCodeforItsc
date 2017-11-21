/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : MyLinkList.c
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月26日
  最近修改   :
  功能描述   : 本文主要定义了单向链表的一些接口实现，在应用在多线程时，还需
               自行注意加锁等线程安全化措施，支持扩展，可根据需要定义新的链表
               类型
  函数列表   :
              AddLinkList
              DelLinkListNode
              DestoryLinkList
              GetLinkListLength
              InitLinkList
              PrintLinkListNode
              ReverseLinkList
  修改历史   :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 创建文件

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持
******************************************************************************/


#include "MyLinkList.h"

//#define DEBUG       //开启后，main函数出现
//#define DEBUG_INT   //开启后,main函数中出现整型链表调试
//#define DEBUG_STR   //调试串型链表



/*****************************************************************************
 函 数 名  : InitLinkList
 功能描述  : 初始化
 输入参数  : void **list    
             ListType type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月25日
    作    者   : 老虎
    修改内容   : 新生成函数

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持
*****************************************************************************/
int InitLinkList(void **list,ListType type)
{

    switch(type)
    {
        case STRING_LIST:
            {

                *list = (PMyStringLinkList)malloc(sizeof(MyStringLinkList));//申请头结点空间

                if(!*list)
                {
                    log_debug("%s  malloc error %s\n",__func__,strerror(errno));
                    return false;
                }

                memset(*list,0,sizeof(MyStringLinkList));
                
                ((PMyStringLinkList)(*list))->next = null;//头结点指向空
                ((PMyStringLinkList)(*list))->val = null;//串链表不设置串长度这个节点了，直接第一个就是实际的数据
                
                break;
            }
            

       
        case INT_LIST:
            {

                *list = (PMyIntLinkList)malloc(sizeof(MyIntLinkList));//申请头结点空间

                if(!*list)
                {
                    log_debug("%s  malloc error %s\n",__func__,strerror(errno));
                    return false;
                }

                memset(*list,0,sizeof(MyIntLinkList));
                
                ((PMyIntLinkList)(*list))->next = null;//头结点指向空
                ((PMyIntLinkList)(*list))->val = 0;//头结点的val表示除头结点外的节点个数，也就是实际链表的长度(不包括头结点)
                
                break;
            } 
        
        default:    return false;
    }

    return true;
}

//
/*****************************************************************************
 函 数 名  : AddLinkList
 功能描述  : 从尾部添加新的链表节点，头结点的val字段只是作为链表长度使用，不
             实际存储数据
 输入参数  : void **list    
             ListType type  
             void *addData  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持
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
            tempNewList = (PMyIntLinkList)malloc(sizeof(MyIntLinkList));//这个就是需要新添加的节点
            if(!tempNewList)
            {
                PRINT_ERROR;
                return false;
            }
            
            memset(tempNewList,0,sizeof(MyIntLinkList));
            tempNewList->next = null;
            tempNewList->val = *(int *)addData;//为新节点赋值及初始化

            //log_debug("AddLinkList   %p   ===>   0x%x\n",tempNewList,tempNewList->val);

            tempLinkList->next = tempNewList;//上个节点指向当前新添加的节点
            ((PMyIntLinkList)(*list))->val += 1;//每次新添加节点，头节点上的val就要加1，表示长度增加
            
            break;
        }
        case STRING_LIST://串链表与整型链表略有不同:串需要为保存的串分配空间，释放时需要free这个空间
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

            //为这个节点的val分配内存空间，并将数据存储在该节点
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
            tempNewList = (PMyStringLinkList)malloc(sizeof(MyStringLinkList));//这个就是需要新添加的节点
            if(!tempNewList)
            {
                PRINT_ERROR;
                return false;
            }
            
            memset(tempNewList,0,sizeof(MyStringLinkList));
            tempNewList->next = null;
            tempNewList->val = null;//为新节点赋值及初始化

            tempLinkList->next = tempNewList;//上个节点指向当前新添加的节点
            
            break;
        }
        default: return false;
    }

    return true;

}    
/*****************************************************************************
 函 数 名  : DelLinkListNode
 功能描述  : 该接口不实用，暂时废弃之。
 输入参数  : void **list    
             ListType type  
             void *delNode  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 新生成函数

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
 函 数 名  : GetLinkListLength
 功能描述  : 该接口不实用，暂时废弃之。
 输入参数  : void *list     
             ListType type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int GetLinkListLength(void *list,ListType type)
{
    return 0;

}
/*****************************************************************************
 函 数 名  : ReverseLinkList
 功能描述  : 翻转链表的一个实现
 输入参数  : void **list    
             ListType type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 大致思路:
其中1是第1个结点，4是尾节点
头 -> 1 -> 2 -> 3 -> 4 -> null

头节点只存储了链表的长度及指向的第1个节点，不参与实际的交换过程，只需在交换结束后，修改其指向即可，所以，实际参与的链表如下:

1 -> 2 -> 3 -> 4 -> null

可以这样做:
最开始可以这样理解: (1) -> 2 -> 3 -> 4 -> null  每次交换可以把已交换完成的作为1个整体，这个整体有个头head和尾tail
第一次: (2 -> 1) -> 3 -> 4 -> null 每一次先获得尾的下一个指向并保存。pTemp = pListTail->next;
第二次: (3 -> 2 -> 1) -> 4 -> null 然后再将保存的下一个指向的指向赋值到尾的指向。pListTail->next = pTemp->next;
第三次: 4 -> 3 -> 2 -> 1 -> null 然后再将该节点的指向赋值为head，并修改heae为当前节点相当于该节点是head。pTemp->next = pListHead;pListHead = pTemp;

交换完成后，修改头指针的指向:
头 -> 4 就OK了。

 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持。
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

            LinkList = LinkList->next;//头结点是用来存储链表长度及头结点的，所以不参与交换，交换从下一个开始

            PMyIntLinkList pListHead = LinkList;//永远是头结点，该变量永远指向链表的头部，可以理解为第1个节点，中间已交换的节点不计入
            PMyIntLinkList pListTail = LinkList;//初始状态，头和尾是相同的，该变量永远指向已交换链表的尾部，可以理解为第2个节点
            PMyIntLinkList pTemp = null;//用来临时保存第2个节点
           
            while(pListTail->next != null)
            {
                pTemp = pListTail->next;//获得第2个节点
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

            //这里是和int链表不同的地方，串链表没有头节点存长度之说，故交换应从头节点开始。
            //LinkList = LinkList->next;//头结点是用来存储链表长度及头结点的，所以不参与交换，交换从下一个开始

            PMyStringLinkList pListHead = LinkList;//永远是头结点，该变量永远指向链表的头部，可以理解为第1个节点，中间已交换的节点不计入
            PMyStringLinkList pListTail = LinkList;//初始状态，头和尾是相同的，该变量永远指向已交换链表的尾部，可以理解为第2个节点
            PMyStringLinkList pTemp = null;//用来临时保存第2个节点
           
            while(pListTail->next != null)
            {
                pTemp = pListTail->next;//获得第2个节点
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
 函 数 名  : PrintLinkListNode
 功能描述  : 打印链表内容
 输入参数  : void **list    
             ListType type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持。

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
            LinkList = LinkList->next;//头节点不输出，从第二个节点开始才是真正的有效数据
            
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
            //LinkList = LinkList->next;//头节点不输出，从第二个节点开始才是真正的有效数据

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
 函 数 名  : DestoryLinkList
 功能描述  : 销毁链表的实现方法
 输入参数  : void **list    
             ListType type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加对串链表的支持。
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
                    free(LinkList->val);//尤其注意，对于串链表而已，这里也是需要释放的。
                }
                
                free(LinkList);//有人说，多线程间会出现内存释放并 不回归OS:http://www.cnblogs.com/raymondshiquan/archive/2011/06/25/tcmalloc_configuration_analysis.html#2245126
            
                LinkList = tempLinkList;

            }            
            break;
        }
        default:    return false;
    }


    return true;

}


/*****************************************************************************
 函 数 名  : main
 功能描述  : 对本文描述接口的测试，也是一般使用链表的步骤
 输入参数  : int argc     
             char **argv  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月26日
    作    者   : 老虎
    修改内容   : 新生成函数

  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 添加测试串链表
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

