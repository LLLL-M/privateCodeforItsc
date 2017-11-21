/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : MyLinkList.h
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月25日
  最近修改   :
  功能描述   : MyLinkList.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年6月25日
    作    者   : 老虎
    修改内容   : 创建文件

******************************************************************************/


#ifndef __MYLINKLIST_H__
#define __MYLINKLIST_H__

#include "Util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "TaskSchedule.h"



#define PRINT_ERROR log_debug("%s  malloc error %s\n",__func__,strerror(errno))

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*定义list的type*/
typedef enum{
    STRING_LIST=0,    
    INT_LIST
}ListType;


/*字符串型list*/
typedef struct StringLinkList{

    char *val;
    
    struct StringLinkList *next;
    
}MyStringLinkList,*PMyStringLinkList;


/*整型list*/
typedef struct IntLinkList{

    int val;
    
    struct IntLinkList *next;
    
}MyIntLinkList,*PMyIntLinkList;


extern int InitLinkList(void **list,ListType type);

extern int AddLinkList(void **list,ListType type,void *addData);

extern int DestoryLinkList(void **list,ListType type);

extern int PrintLinkListNode(void **list,ListType type);






#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MYLINKLIST_H__ */
