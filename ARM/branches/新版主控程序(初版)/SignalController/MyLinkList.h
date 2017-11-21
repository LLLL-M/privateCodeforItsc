/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : MyLinkList.h
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��25��
  ����޸�   :
  ��������   : MyLinkList.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��6��25��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

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


/*����list��type*/
typedef enum{
    STRING_LIST=0,    
    INT_LIST
}ListType;


/*�ַ�����list*/
typedef struct StringLinkList{

    char *val;
    
    struct StringLinkList *next;
    
}MyStringLinkList,*PMyStringLinkList;


/*����list*/
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
