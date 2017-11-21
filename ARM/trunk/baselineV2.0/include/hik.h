/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : hik.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月27日
  最近修改   :
  功能描述   : 公用的头文件，包含了一些打印的宏定义以及一些通用的类型定义
			   此文件只适用于linux系统的gcc编译器，另外此文件也可用于编写linux驱动使用
  函数列表   :
  修改历史   :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HIK_H__
#define __HIK_H__

#ifndef __linux__	//此宏是gcc编译器定义的，如果当前的编译环境不是gcc，则编译会停止并报错
#error	"You can't use it in other system, only linux system gcc can compile it!"
#endif	//end of __linux__
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdio.h>
#include <stddef.h>
#endif	//end of __KERNEL__

#ifdef __cplusplus
extern "C" {
#endif	//end of __cplusplus

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
/*以下宏定义是printf时的一些颜色打印，可以根据自己的喜好进行选择 */
#define COL_DEF "\033[m"			//系统默认的打印颜色，一般是白色
#define COL_RED "\033[0;32;31m"		//红色打印
#define COL_GRE "\033[0;32;32m"		//绿色打印
#define COL_BLU "\033[0;32;34m"		//蓝色打印
#define COL_YEL "\033[1;33m"		//黄色打印


#define GET_BIT(v, bit) 	(((v) >> (bit)) & 0x1)	//取v的某一bit位
#define SET_BIT(v, bit) 	({(v) |= (1 << (bit));})	//设置v的某一bit位
#define CLR_BIT(v, bit)		({(v) &= (~(1 << (bit)));})	//清零v的某一bit位


/* 若是要打印信息，请使用以下宏定义，不但带有颜色，而且可以很方便的去除打印信息
	ERR：打印错误的信息						eg: ERR("This is a error message!");
	INFO:打印程序运行过程中的一些有用信息	eg: INFO("The udp server process has init!");
	DBG: 打印调试信息，可以通过编译时添加-DDEBUG参数来控制打印的调试信息
以上三个宏定义的打印末尾都默认添加了换行符\n  */
#ifdef __KERNEL__	//for linux driver
	#define ERR(fmt, ...) printk(KERN_ERR COL_RED "driver error function[%s]:"\
				COL_YEL fmt COL_DEF "\n", __func__, ##__VA_ARGS__)
	#define INFO(fmt, ...) printk(KERN_INFO COL_GRE "driver information:"\
				COL_YEL	fmt COL_DEF "\n", ##__VA_ARGS__)

	#ifdef DEBUG
	#define DBG(fmt, ...) printk(KERN_DEBUG COL_BLU "debug function[%s]:"\
				COL_DEF	fmt, __func__, ##__VA_ARGS__)
	#else
	#define DBG(fmt, ...) //({0;})
	#endif	// end of DEBUG
#else		//for linux application
	#define ERR(fmt, ...) fprintf(stderr, COL_RED "error function[%s]:"\
				COL_YEL fmt COL_DEF "\n", __func__, ##__VA_ARGS__)
	#define INFO(fmt, ...) fprintf(stderr, COL_GRE "information:"\
				COL_YEL	fmt COL_DEF "\n", ##__VA_ARGS__)

	#ifdef DEBUG
	#define DBG(fmt, ...) printf(COL_BLU "debug function[%s]:"\
				COL_DEF	fmt "\n", __func__, ##__VA_ARGS__)
	#else
	#define DBG(fmt, ...)
	#endif	// end of DEBUG			
	
	/*这是内核经常用到的一个宏定义，意思是通过结构体的一个成员变量的地址找到这个结构体变量的首地址 
	  其中用到的offsetof是取一个结构体中相关成员的位置偏移，这个宏在stddef.h定义了，所以直接引用了*/
	#define container_of(ptr, type, member) ({          \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );})
		
	#define min(x, y) ({                \
		typeof(x) _min1 = (x);          \
		typeof(y) _min2 = (y);          \
		(void) (&_min1 == &_min2);      \
		_min1 < _min2 ? _min1 : _min2; })

	#define max(x, y) ({                \
		typeof(x) _max1 = (x);          \
		typeof(y) _max2 = (y);          \
		(void) (&_max1 == &_max2);      \
		_max1 > _max2 ? _max1 : _max2; })
	
#endif	//end of __KERNEL__

extern int gOftenPrintFlag;
#define OFTEN(fmt, ...) do {\
	if (gOftenPrintFlag) \
		fprintf(stderr, COL_BLU "[%s]" COL_YEL fmt COL_DEF "\n", __func__, ##__VA_ARGS__);\
} while (0)
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

typedef unsigned int        DWORD; // 4, same as long
typedef unsigned short      WORD;  // 2
typedef unsigned char       BYTE;  // 1

/*说明：
		1.建议对于返回值为正确与否的函数，统一使用这个枚举，即成功返回1，失败返回0.
		2.如果返回的错误编号比较多，建议自定义一个枚举类型，把错误编号一次依次列出，此时应该正确返回0，错误返回正数
*/
#if !defined(TRUE) && !defined(FALSE)
typedef enum
{
	FALSE = 0,
	TRUE,
} Boolean;
#endif

/*----------------------------------------------*
 * 接口函数                                     *
 *----------------------------------------------*/
/*暂时没有想到有什么可以公共使用的函数，如果大家想到了可以提出来加到这一块 */


#ifdef __cplusplus
}
#endif	//end of __cplusplus

#endif	//end of __HIK_H__
