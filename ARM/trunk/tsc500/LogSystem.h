/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : LogSystem.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��4��3��
  ����޸�   :
  ��������   : LogSystem.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��4��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

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
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

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

typedef struct {

    char fileName[MAX_FILENAME_LENGTH]; //�ļ�������󳤶�,
    char pathName[MAX_PATH_LENGTH];     //·��������󳤶�
    int maxSizeAll;                     //������־�ļ����ܴ�С    ��λ��MB
    int maxSizeEach;                    //������־�ļ����ܴ�С    ��λ��MB
    int index;                          //��־��׺���
}LogSystemStruct,*PLogSystemStruct;
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void log_debug(const char* format, ...);
extern void log_error(const char* format, ...);
extern int InitLogSystem(char *path,int max_size_all,int max_size_each);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LOGSYSTEM_H__ */
