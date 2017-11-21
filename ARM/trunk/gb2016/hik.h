/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : hik.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2017年03月07日
  最近修改   :
  功能描述   : C++公用的头文件，包含了一些打印的宏定义以及一些通用的类型定义
			   此文件只适用于linux系统的gcc编译器以及windows的VS
  函数列表   :
  修改历史   :
  1.日    期   : 2017年03月07日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HIK_H__
#define __HIK_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <iostream>
#include <cstdio>
#include <cstddef>
using namespace std;
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#define GET_BIT(v, bit) 	(((v) >> (bit)) & 0x1)	//取v的某一bit位
#define SET_BIT(v, bit) 	({(v) |= (1 << (bit));})	//设置v的某一bit位
#define CLR_BIT(v, bit)		({(v) &= (~(1 << (bit)));})	//清零v的某一bit位

#ifdef __linux__
	/*以下宏定义是printf时的一些颜色打印，可以根据自己的喜好进行选择 */
	#define COL_DEF "\033[m"			//系统默认的打印颜色，一般是白色
	#define COL_RED "\033[0;32;31m"		//红色打印
	#define COL_GRE "\033[0;32;32m"		//绿色打印
	#define COL_BLU "\033[0;32;34m"		//蓝色打印
	#define COL_YEL "\033[1;33m"		//黄色打印
	#include <unistd.h>
	#define msleep(X) usleep((X) * 1000)
#else	//__WIN32__ C++
	#define COL_DEF 
	#define COL_RED 
	#define COL_GRE 
	#define COL_BLU 
	#define COL_YEL
	#include <windows.h>
	#define sleep(X) Sleep((X) * 1000)
	#define msleep(X) Sleep(X)
#endif

//#define ERR cerr << COL_RED"file[" << __FILE__ << "]"COL_YEL"line[" << __LINE__ << "]:"COL_DEF
//#define INFO cout << COL_GRE"file[" << __FILE__ << "]"COL_YEL"line[" << __LINE__ << "]:"COL_DEF
#define ERR(fmt, ...) fprintf(stderr, COL_RED "file[%s]:" COL_YEL "line[%d]:" COL_DEF fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define INFO(fmt, ...) printf(COL_GRE "file[%s]:" COL_YEL "line[%d]:" COL_DEF fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
/*这是内核经常用到的一个宏定义，意思是通过结构体的一个成员变量的地址找到这个结构体变量的首地址 
  其中用到的offsetof是取一个结构体中相关成员的位置偏移，这个宏在stddef.h定义了，所以直接引用了*/
#define container_of(ptr, type, member) ({          \
	const decltype( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
/*说明：
		1.尽量使用以下重定义的数据类型，而且最好不要使用long类型，因为long类型与体系密切相关，32位：4字节，64位：8字节
		2.对于float和double由于不经常使用，因此这里没有具体定义，使用时可自己定义即可。
*/
typedef signed char 		Int8;	//Int8 	cValue;	为了保持统一，建议不使用ch修饰而改为c修饰
typedef signed short 		Int16;	//Int16	sValue;
typedef signed int			Int32;	//Int32 iValue;
typedef signed long long	Int64;	//Int64	lValue;

typedef unsigned char 		UInt8;	//UInt8  ucValue;
typedef unsigned short 		UInt16;	//UInt16 usValue;
typedef unsigned int		UInt32;	//UInt32 uiValue;
typedef unsigned long long	UInt64;	//UInt64 ulValue;


typedef signed char 		INT8;	//Int8 	cValue;	为了保持统一，建议不使用ch修饰而改为c修饰
typedef signed short 		INT16;	//Int16	sValue;
typedef signed int			INT32;	//Int32 iValue;
typedef signed long long	INT64;	//Int64	lValue;

typedef unsigned char 		UINT8;//UInt8  ucValue;
typedef unsigned short 		UINT16;//UInt16 usValue;
typedef unsigned int		UINT32;	//UInt32 uiValue;
typedef unsigned long long	UINT64;	//UInt64 ulValue;

#ifdef __linux__
typedef unsigned int        DWORD; // 4, same as long
typedef unsigned short      WORD;  // 2
typedef unsigned char       BYTE;  // 1
#endif


#endif	//end of __HIK_H__
