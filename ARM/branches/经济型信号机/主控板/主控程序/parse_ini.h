/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : parse_ini.h
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��12��1��
  ����޸�   :
  ��������   : parse_ini.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __PARSE_INI_H__
#define __PARSE_INI_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define PARSEINI_LIB_VERSION "1.0.0"

#define NO_VALUE	0
#define USED		1

#ifdef __GNUC__
#define WEAKATTR    __attribute__((weak))
#define UNUSEDATTR  __attribute__((unused))
#else
#define WEAKATTR
#define UNUSEDATTR
#endif
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum 
{
	False = 0,
	True
} Bool;
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
/* ������ʼ�������ڽ���֮ǰ���� */
extern Bool parse_start(const char *filename);
/* ���������������ڽ������֮����� */
extern void parse_end(void);

/* ��ȡһ����ֵ��һ��һ��ϵ */
extern int get_one_value(const char *section, const char *key);

/* ��ȡkey��Ӧ���ַ���, buf can't be NULL */
extern char *get_key_string(const char *section, const char *key, char *buf);

/* ��ȡ�����ֵ�ԣ�һ�Զ��ϵ */
extern Bool get_more_value(const char *section, const char *key, unsigned char *array, int num);

/* ���һ��key=string��ֵ�� */
extern void add_key_string(const char *section, const char *key, const char *string);

/* ���һ����ֵ�ԣ�һ��һ��ϵ */
extern void add_one_key(const char *section, const char *key, int value);

/* ��Ӷ����ֵ�ԣ�һ�Զ��ϵ */
extern void add_more_key(const char *section, const char *key, unsigned char *array, int num);


/* ɾ��һ���� */
extern void del_key(const char *section, const char *key);

/* ɾ��һ�� */
extern void del_section(const char *section);


/* ���¼����������Ե������ã�������Ҫ����parse_start()��parse_end()�Ϳ���ֱ��ʹ�� */

WEAKATTR extern int read_profile_string(const char *section, const char *key,char *value, 
						int size, const char *default_value, const char *file);
WEAKATTR extern int read_profile_int( const char *section, const char *key,int default_value, 
						const char *file);
WEAKATTR extern int write_profile_string(const char *section, const char *key,const char *value, 
						const char *file);
WEAKATTR extern void delete_profile_string(const char *section, const char *key, const char *file);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __PARSE_INI_H__ */

