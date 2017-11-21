/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : Util.h
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��25��
  ����޸�   :
  ��������   : Util.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��6��25��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#ifndef __UTIL_H__
#define __UTIL_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>


#include "MyLinkList.h"
#include "LogSystem.h"
#include "ThreadController.h"
#include "MyDateTime.h"


#define null    NULL
#define true    1
#define false   0

#define LOG4HIK

#define MAX_CHANNEL_NUM 32

#define ARM_PLATFORM    //�����������꣬�ͱ����ǹ�������Ƕ��ʽ�忨���棬�����������������linu����
                            //���ע�͵�����꣬����ʹ��gcc -f Makefile_server.txt�������ɿ���������PC���ϵĳ���



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */




typedef void * (thrfunc)(void *arg);



extern void GetLocalDate(char *date);

extern void GetLocalTime(char *localTime);

extern unsigned long GetTickCount();

extern void log_debug(const char* format, ...);

extern void log_error(const char* format, ...);


extern int semPost(sem_t * pSem, int nMaxCount);

extern int semWait(sem_t * pSem);

extern pid_t gettid();

extern void * ThreadClearMem();

extern int IsItemInIntArray(int *array,int length,int val);

extern pthread_t  ThreadCreate(thrfunc *routine, void *args);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UTIL_H__ */
