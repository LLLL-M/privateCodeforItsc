/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : Util.c
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月25日
  最近修改   :
  功能描述   : 这里描述了一些通用接口如Log_debug等
  函数列表   :
              GetLocalDate
              GetLocalTime
              GetTickCount
              gettid
              IsItemInCharArray
              IsItemInIntArray
              IsItemInShortArray
              log_debug
              log_error
              semPost
              semWait
  修改历史   :
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Util.h"
#include "HikConfig.h"
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

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : GetTickCount
 功能描述  : 计算当前时间，用于精准定时。
 输入参数  : 无
 返 回 值  : 以ms为单位的当前时间
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
unsigned long GetTickCount()
{
	struct timeval tv;
	ulong time_val;
	gettimeofday(&tv, NULL);
	// return ulong(tv.tv_sec*1000L+tv.tv_usec/1000L);
	time_val = tv.tv_sec*1000L + tv.tv_usec/1000L;
	return time_val;
}

/*****************************************************************************
 函 数 名  : GetLocalTime
 功能描述  : 获得当前时间，可重入线程安全
 输入参数  : 
 输出参数  : char *localTime  
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
void GetLocalTime(char *localTime)
{
    if(!localTime)
    {
        return;
    }

    time_t timep;
    time(&timep);

    struct tm now;
    localtime_r(&timep,&now);

    strftime(localTime,24,"%Y-%m-%d %H:%M:%S",&now);
}

/*****************************************************************************
 函 数 名  : log_debug
 功能描述  : 格式化输出接口函数，可重入，线程安全
 输入参数  : const char* format  
             ...                 
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 新生成函数
 
  2.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 为支持本地日志系统，修改buf起始
*****************************************************************************/
void log_debug(const char* format, ...)
{
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//最前面是当前日期函数
	//printf("strlen buff  %d\n",strlen(buff));
	strcat(buff,"  ");//用2个字节的空格使日期与具体日志内容分开
	strcat(buff,"<DEBUG>");
	strcat(buff,"  ");	
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,"\n");
	printf("%s",buff);

#ifdef LOG4HIK
    AddLogItem(buff);
#endif
	
}

/*****************************************************************************
 函 数 名  : log_error
 功能描述  :  格式化输出接口函数，可重入，线程安全
 输入参数  : const char* format  
             ...                 
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void log_error(const char* format, ...)
{
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//最前面是当前日期函数
	//printf("strlen buff  %d\n",strlen(buff));
	strcat(buff,"  ");//用2个字节的空格使日期与具体日志内容分开
	strcat(buff,"<ERROR>");
	strcat(buff,"  ");
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,"\n");
	printf("%s",buff);

#ifdef LOG4HIK
    AddLogItem(buff);
#endif
}


/*****************************************************************************
 函 数 名  : semPost
 功能描述  : 用于线程通过sem同步
 输入参数  : sem_t * pSem   
             int nMaxCount  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int semPost(sem_t * pSem, int nMaxCount)
{
	int nRet, nSemCount;

	sem_getvalue(pSem, &nSemCount);
	if (nSemCount>=nMaxCount)
	{
	    printf("semPost  lost\n");
		return 0;
	}
	else
	{
		nRet=sem_post(pSem);
		return nRet;
	}
}
/*****************************************************************************
 函 数 名  : semWait
 功能描述  : 用于线程通过sem同步
 输入参数  : sem_t * pSem  
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int semWait(sem_t * pSem)
{
	int nRet;
	nRet=sem_wait(pSem);

	while (sem_trywait(pSem)==0) {}
	return nRet;
}
/*****************************************************************************
 函 数 名  : gettid
 功能描述  : 获得线程ID，利于调试top -Hp
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
pid_t gettid()
{
     return syscall(SYS_gettid); 
}

/*****************************************************************************
 函 数 名  : IsItemInIntArray
 功能描述  : 判断整型变量val是否在array数组中
 输入参数  : int *array  
             int length  
             int val     
 输出参数  : 无
 
 修改历史      :
  1.日    期   : 2014年7月3日
    作    者   : xiaowh
    修改内容   : 新生成函数

*****************************************************************************/
int IsItemInIntArray(int *array,int length,int val)
{
    if(!array)
    {
        return false;
    }
    
    int i = 0;
    for(i = 0 ; i < length ; i++)
    {
        if(array[i] == val)
        {
            return true;
        }

    }
    return false;

}

/*****************************************************************************
 函 数 名  : IsItemInShortArray
 功能描述  : 判断short型变量val是否在array数组中
 输入参数  : unsigned short *array  
             int length             
             int val                
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int IsItemInShortArray(unsigned short *array,int length,int val)
{
    if(!array)
    {
        return false;
    }
    
    int i = 0;
    for(i = 0 ; i < length ; i++)
    {
        if(array[i] == val)
        {
            return true;
        }

    }
    return false;
}


/*****************************************************************************
 函 数 名  : IsItemInCharArray
 功能描述  : 判断char变量val是否在array数组中
 输入参数  : unsigned char *array   
             unsigned short length  
             unsigned short val     
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int IsItemInCharArray(unsigned char *array,unsigned short length,unsigned short val)
{
    if(!array)
    {
        return false;
    }
    
    unsigned short i = 0;
    for(i = 0 ; i < length ; i++)
    {
        if(array[i] == val)
        {
            return i+1;
        }

    }
    return 0;

}


