/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : filebackup.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年6月17日
  最近修改   :
  功能描述   : filebackup.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年6月17日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __FILEBACKUP_H__
#define __FILEBACKUP_H__

#include <stdio.h>
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

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void FileBackup(char *fileName);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __FILEBACKUP_H__ */
