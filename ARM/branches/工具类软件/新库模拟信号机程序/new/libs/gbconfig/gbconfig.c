/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : HikConfig.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : 本文件完成对配置文件进行读取、参数校验的接口
  函数列表   :
              ArrayToInt
              checkPhaseTrunInRing
              DateTimeCmp
              find_next_index
              GetCircleTime
              IsArrayRepeat
              IsBarrierGreenSignalRationEqual
              IsItemInCharArray
              IsItemInShortArray
              IsPhaseContinuousInPhaseTurn
              isPhaseInArray
              IsSignalControllerParaIdLegal
              IsSignalControlparaLegal
              LoadDataFromCfg
              log_error
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

  2.日    期   : 2014年12月3日
    作    者   : 肖文虎
    修改内容   : 按照新的结构体，修改相关接口
******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "gbconfig.h"
#include <stdarg.h>
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define COL_DEF "\033[m"
#define COL_RED "\033[0;32;31m"
#define COL_GRE "\033[0;32;32m"
#define COL_BLU "\033[0;32;34m"
#define COL_YEL "\033[1;33m"

//是否开启中文错误打印
#define LOG_ERR_CHN


#define DEG(fmt,...) fprintf(stdout,"gbConfig library debug : "fmt "\n",##__VA_ARGS__)



/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
char *gErrorContentGb = NULL;//该全局变量存储错误信息，供调用者进行分析


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : log_error
 功能描述  : 打印错误信息接口，其中错误内容会 保存到全局缓冲区，外部接口可直
             接查看其内容。
 输入参数  : const char* format  
             ...                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数
  2.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 修改缓存区大小为1024字节
  3.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 将多个strcat连接字符串改为一个sprintf连接所有字符串	
*****************************************************************************/
static void log_error(const char* format, ...)
{
#ifndef LOG_ERR_CHN    
	char buff[1024] = {0};
    if(NULL == gErrorContentGb)
    {
        gErrorContentGb = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContentGb)
    {
    	memset(gErrorContentGb,0,1024);
    	vsnprintf(gErrorContentGb,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);
#endif
}

/*****************************************************************************
 函 数 名  : log_error_cn
 功能描述  : 打印错误信息接口，其中错误内容会 保存到全局缓冲区，外部接口可直
             接查看其内容。
 输入参数  : const char* format  
             ...                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数
  2.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 修改缓存区大小为1024字节
  3.日    期   : 2014年12月10日
    作    者   : 肖文虎
    修改内容   : 将多个strcat连接字符串改为一个sprintf连接所有字符串	
*****************************************************************************/
static void log_error_cn(const char* format, ...)
{
#ifdef LOG_ERR_CHN
	char buff[1024] = {0};
    if(NULL == gErrorContentGb)
    {
        gErrorContentGb = calloc(1024,1);

    }
	sprintf(buff,"%s HikConfig library error: %s <ERROR>  ",COL_BLU,COL_YEL);
	va_list argptr;
	va_start(argptr, format);

    if(NULL != gErrorContentGb)
    {
    	memset(gErrorContentGb,0,1024);
    	vsnprintf(gErrorContentGb,1024,format,argptr);
    }

	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,COL_DEF);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);
#endif
}


