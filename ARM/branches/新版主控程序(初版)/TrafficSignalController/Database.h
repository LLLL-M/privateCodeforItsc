/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : Database.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : Database.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __DATABASE_H__
#define __DATABASE_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Util.h"
#include "LogSystem.h"
#include "HikConfig.h"
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DATABASE_NAME "/home/hikTSC.db"     //保存的数据库名称

#define TABLE_RECORD_NAME   "T_Record"      //记录表
#define TABLE_FAULT_NAME    "T_Fault"       //故障表

#define PATH_STARTUP "/home/StartUp.log"
#define PATH_KEYBOARD "/home/Keyboard.log"
#define PATH_FAULT "/home/FaultLog.dat"
#define PATH_LOGIN  "/home/login.ini"
#define MAX_CHECK_FILE_NUM  4
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef enum 
{
    ITEM_STARTUP = 0,//start up log开机记录
    ITEM_KEYBOARD,   //keybord键盘操作记录
    ITEM_LAMP        //灯故障
}TABLE_ITEM_TYPE;//operation log table type   具体表项的类型

typedef enum
{
    TABLE_RECORD = 0,//记录表
    TABLE_FAULT   //故障表

}TABLE_TYPE;//表格类型
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern int CreateTableByType(TABLE_TYPE type,sqlite3 *db);
extern int GetLastRecord(TABLE_TYPE type,TABLE_ITEM_TYPE type_item,sqlite3 *db,char *timeBuf);
extern int InsertTableItemByType(TABLE_TYPE type_table,TABLE_ITEM_TYPE type_item,char *dateTime,char *Msg,sqlite3 *db);
extern int JudgeStructTmTime(struct tm tm_1,struct tm tm_2);
extern void ReLoadLoginCfg();
extern int SendFileContentToDB(TABLE_TYPE type_table,TABLE_ITEM_TYPE type_item,char *fileName,struct tm  time,sqlite3 *db);
extern int ShowTableContentByType(TABLE_TYPE type,sqlite3 *db);
extern void * ThreadCheckCfgChanged(void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __DATABASE_H__ */
