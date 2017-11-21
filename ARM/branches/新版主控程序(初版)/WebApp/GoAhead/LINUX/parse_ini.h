/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : parse_ini.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年12月1日
  最近修改   :
  功能描述   : parse_ini.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __PARSE_INI_H__
#define __PARSE_INI_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
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
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef enum 
{
	False = 0,
	True
} Bool;
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
/* 解析起始，必须在解析之前调用 */
extern Bool parse_start(const char *filename);
/* 解析结束，必须在解析完毕之后调用 */
extern void parse_end(void);

/* 获取一个键值，一对一关系 */
extern int get_one_value(const char *section, const char *key);

/* 获取key对应的字符串, buf can't be NULL */
extern char *get_key_string(const char *section, const char *key, char *buf);

/* 获取多个键值对，一对多关系 */
extern Bool get_more_value(const char *section, const char *key, unsigned char *array, int num);

/* 添加一个key=string键值对 */
extern void add_key_string(const char *section, const char *key, const char *string);

/* 添加一个键值对，一对一关系 */
extern void add_one_key(const char *section, const char *key, int value);

/* 添加多个键值对，一对多关系 */
extern void add_more_key(const char *section, const char *key, unsigned char *array, int num);


/* 删除一个键 */
extern void del_key(const char *section, const char *key);

/* 删除一项 */
extern void del_section(const char *section);


/* 以下几个函数可以单独调用，即不需要调用parse_start()和parse_end()就可以直接使用 */

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

