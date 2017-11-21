/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : LogSystem.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年4月3日
  最近修改   :
  功能描述   : LogSystem.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年4月3日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __LOGSYSTEM_H__
#define __LOGSYSTEM_H__

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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_PATH_LENGTH 128
#define MAX_FILENAME_LENGTH 256


/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/

typedef struct {

    char fileName[MAX_FILENAME_LENGTH]; //文件名的最大长度,
    char pathName[MAX_PATH_LENGTH];     //路径名的最大长度
    int maxSizeAll;                     //所有日志文件的总大小    单位是MB
    int maxSizeEach;                    //单个日志文件的总大小    单位是MB
    int index;                          //日志后缀序号
}LogSystemStruct,*PLogSystemStruct;
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void log_debug(const char* format, ...);
extern void log_error(const char* format, ...);
extern int InitLogSystem(char *path,int max_size_all,int max_size_each);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LOGSYSTEM_H__ */
