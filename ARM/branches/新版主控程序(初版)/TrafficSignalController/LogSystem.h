/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : LogSystem.h
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月27日
  最近修改   :
  功能描述   : LogSystem.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __LOGSYSTEM_H__
#define __LOGSYSTEM_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "Util.h"

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
typedef enum {
    LOG_DEBUG = 0,
    LOG_ERROR,
    LOG_WARN
}LOGTYPE;

typedef struct {

    char fileName[MAX_FILENAME_LENGTH];//the max len is MAX_FILENAME_LENGTH ,
    char pathName[MAX_PATH_LENGTH];
    int maxSizeAll;//the size of the full system  beyond maxSize MB所有日志文件的总大小
    int maxSizeEach;//单个日志文件的总大小  MB
    int index;//日志后缀序号
}LogSystemStruct,*PLogSystemStruct;

typedef struct {
    char pathName[MAX_PATH_LENGTH];//存放日志文件的路径名，支持绝对路径和相对路径，该路径必须可访问
    int maxSizeAll;//所有日志文件大小之和 单位是MB
    int maxSizeEach;//每个日志文件的大小    单位是MB
}LogSystemThreadData,*PLogSystemThreadData;
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern int InitLogSys(char *path,int max_size_all,int max_size_each);

extern int GetFileSize(char *fileName);

extern int GetDirSize(char *dirName);

extern int AddContent(char *logContent);

extern int DestroyLogSys();

extern int GetLastLogFileIndex(char *dirName);

extern int JudgeTime(time_t val_1,time_t val_2);

extern void AddLogItem(char *logContent);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LOGSYSTEM_H__ */

