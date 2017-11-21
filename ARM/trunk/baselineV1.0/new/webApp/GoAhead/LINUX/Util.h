/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : Util.h
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月25日
  最近修改   :
  功能描述   : Util.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年6月25日
    作    者   : 老虎
    修改内容   : 创建文件

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

//#define ARM_PLATFORM    //如果开启这个宏，就表明是工作正在嵌入式板卡上面，否则可以运行在其他linu上面
                            //如果注释掉这个宏，可以使用gcc -f Makefile_server.txt，来生成可以运行在PC机上的程序

#ifndef ARM_PLATFORM
#define LOG4HIK              //默认关闭这个宏，用来支持本地日志打印的，考虑到flash的损耗，在嵌入式板卡上，默认是关闭的
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
