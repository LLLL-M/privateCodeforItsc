/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : Database.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : Database.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __DATABASE_H__
#define __DATABASE_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
#define DATABASE_NAME "/home/hikTSC.db"     //��������ݿ�����

#define TABLE_RECORD_NAME   "T_Record"      //��¼��
#define TABLE_FAULT_NAME    "T_Fault"       //���ϱ�

#define PATH_STARTUP "/home/StartUp.log"
#define PATH_KEYBOARD "/home/Keyboard.log"
#define PATH_FAULT "/home/FaultLog.dat"
#define PATH_LOGIN  "/home/login.ini"
#define MAX_CHECK_FILE_NUM  4
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum 
{
    ITEM_STARTUP = 0,//start up log������¼
    ITEM_KEYBOARD,   //keybord���̲�����¼
    ITEM_LAMP        //�ƹ���
}TABLE_ITEM_TYPE;//operation log table type   ������������

typedef enum
{
    TABLE_RECORD = 0,//��¼��
    TABLE_FAULT   //���ϱ�

}TABLE_TYPE;//�������
/*----------------------------------------------*
 * �ӿں���                                    *
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
