/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : LogSystem.h
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��27��
  ����޸�   :
  ��������   : LogSystem.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __LOGSYSTEM_H__
#define __LOGSYSTEM_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_PATH_LENGTH 128
#define MAX_FILENAME_LENGTH 256
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum {
    LOG_DEBUG = 0,
    LOG_ERROR,
    LOG_WARN
}LOGTYPE;

typedef struct {

    char fileName[MAX_FILENAME_LENGTH];//the max len is MAX_FILENAME_LENGTH ,
    char pathName[MAX_PATH_LENGTH];
    int maxSizeAll;//the size of the full system  beyond maxSize MB������־�ļ����ܴ�С
    int maxSizeEach;//������־�ļ����ܴ�С  MB
    int index;//��־��׺���
}LogSystemStruct,*PLogSystemStruct;

typedef struct {
    char pathName[MAX_PATH_LENGTH];//�����־�ļ���·������֧�־���·�������·������·������ɷ���
    int maxSizeAll;//������־�ļ���С֮�� ��λ��MB
    int maxSizeEach;//ÿ����־�ļ��Ĵ�С    ��λ��MB
}LogSystemThreadData,*PLogSystemThreadData;
/*----------------------------------------------*
 * �ӿں���                                    *
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

