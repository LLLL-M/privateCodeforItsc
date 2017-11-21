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


#define null    NULL
#define true    1
#define false   0
#define bool int

#define MAX_CHANNEL_NUM 32

//#define ARM_PLATFORM    //�����������꣬�ͱ����ǹ�������Ƕ��ʽ�忨���棬�����������������linu����
                            //���ע�͵�����꣬����ʹ��gcc -f Makefile_server.txt�������ɿ���������PC���ϵĳ���

#ifndef ARM_PLATFORM
#define LOG4HIK              //Ĭ�Ϲر�����꣬����֧�ֱ�����־��ӡ�ģ����ǵ�flash����ģ���Ƕ��ʽ�忨�ϣ�Ĭ���ǹرյ�
#endif

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

extern pid_t gettid();

extern void * ThreadClearMem();

extern int IsItemInIntArray(int *array,int length,int val);

extern int IsItemInShortArray(unsigned short *array,int length,int val);

extern int IsPhaseInPhaseTurn(unsigned short nPhaseTurn, unsigned short nPhaseId);
extern int IsItemInCharArray(unsigned char *array,unsigned short length,unsigned short val);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UTIL_H__ */
