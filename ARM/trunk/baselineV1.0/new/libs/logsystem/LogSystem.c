/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : LogSystem.c
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月30日
  最近修改   :
  功能描述   : 本日志系统用法:
                1. 引用LogSystem.h头文件
                2. 调用InitLogSystem函数，按实际情况设置日志路径、缓存大小及单个日志文件大小。该步骤可省略，缺省为当前路径下
                    ./Log/，默认缓存是100M，默认单个文件大小是5M
                3. 直接调用log_debug或log_err即可。
  
  函数列表   :
              AddContent
              AddLogItem
              DestroyLogSys
              GetDirSize
              GetFileSize
              IncreaseLogFileIndex
              InitLogSys
  修改历史   :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 创建文件

******************************************************************************/
#include "LogSystem.h"

static LogSystemStruct g_LogStruct;         //全局日志结构体，其中包括该日志的基本信息
static FILE *fp = NULL;                     //全局文件指针，用来写入日志数据
static int flagIsInitOK = 0;                //是否初始化正确

pthread_mutex_t gLockLogFile;               //文件锁，防止多线程同时写入文件

#define false 0
#define true  1
#define DEFAULT_PATH            "./Log/"            //日志文件的默认路径
#define DEFAULT_SIZE_ALL        100                 //日志文件的总大小是100MB
#define DEFAULT_SIZE_EACH       5                   //每个日志文件的默认大小是5M


/*****************************************************************************
 函 数 名  : GetLocalTime
 功能描述  : 获得当前时间，可重入线程安全
 输入参数  : char *localTime  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月27日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static void GetLocalTime(char *localTime)
{
    if(!localTime)
    {
        return;
    }

    time_t timep;
    time(&timep);

    struct tm now;
    localtime_r(&timep,&now);

	struct timeval tv;  
    struct timezone tz;       
    gettimeofday(&tv, &tz); 
    strftime(localTime,24,"%Y-%m-%d %H:%M:%S",&now);
	sprintf(localTime,"%s.%03ld",localTime,tv.tv_usec/1000);
}


/*****************************************************************************
 函 数 名  : JudgeTime
 功能描述  : 比较两个时间结构体time_t的大小，大于时返回大于0的值，小于返回小
             于0的值，相等时返回0
 输入参数  : time_t val_1  
             time_t val_2  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static int JudgeTime(time_t val_1,time_t val_2)
{
    struct tm tm_1;
    struct tm tm_2;

    localtime_r(&val_1,&tm_1);
    localtime_r(&val_2,&tm_2);

    if(tm_1.tm_year > tm_2.tm_year)//year 
    {
        return 1;
    }
    else if(tm_1.tm_year < tm_2.tm_year)
    {
        return -1;
    }
    else
    {
        if(tm_1.tm_mon > tm_2.tm_mon)//month
        {
            return 1;
        }
        else if(tm_1.tm_mon < tm_2.tm_mon)
        {
            return -1;
        }
        else 
        {
            if(tm_1.tm_mday > tm_2.tm_mday)//day
            {
                return 1;
            }
            else if(tm_1.tm_mday < tm_2.tm_mday)
            {
                return -1;
            }
            else 
            {
                if(tm_1.tm_hour > tm_2.tm_hour)//hour
                {
                    return 1;
                }
                else if(tm_1.tm_hour < tm_2.tm_hour)
                {
                    return -1;
                }
                else 
                {
                    if(tm_1.tm_min > tm_2.tm_min)//minute
                    {
                        return 1;
                    }
                    else if(tm_1.tm_min < tm_2.tm_min)
                    {
                         return -1;
                    }
                    else 
                    {
                        if(tm_1.tm_sec > tm_2.tm_sec)//second
                        {
                            return 1;
                        }
                        else if(tm_1.tm_sec < tm_2.tm_sec)
                        {
                            return -1;
                        }
                        else 
                        {
                            return 0;
                        }
                    }
                }

            }
        }
    }
}


/*****************************************************************************
 函 数 名  : GetLastLogFileIndex
 功能描述  : 初始化时先获取当前日志文件夹内最新日志的后缀index，以防从1开始
             ，这样会覆盖最新数据。保证日志永远存在最新的日志文件里。
 输入参数  : char *dirName  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static int GetLastLogFileIndex(char *dirName)
{
    struct dirent *dirp;
    DIR *dp;
    int count = 0;
    char fileName[MAX_FILENAME_LENGTH];
    struct stat fileStat;
    char fileNameLast[MAX_FILENAME_LENGTH];
    time_t timeLast = 0;
    
    if((dp = opendir(dirName)) == NULL)
    {
        return -1;
    }
    
    int flag = (dirName[strlen(dirName) - 1] == '/');

    memset(fileName,0,sizeof(fileName));
    memset(fileNameLast,0,sizeof(fileNameLast));
    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name,".") == 0 || strcmp(dirp->d_name,"..") == 0)
        {
            continue;
        }
        //get filename
        memset(fileName,0,sizeof(fileName));
        if(flag)
        {
            snprintf(fileName,sizeof(fileName),"%s%s",dirName,dirp->d_name);
        }
        else
        {
            snprintf(fileName,sizeof(fileName),"%s/%s",dirName,dirp->d_name);
        }
        //get the up to date index
        if(stat(fileName,&fileStat) == 0)
        {
            if(0 == count)
            {
                memset(fileNameLast,0,sizeof(fileNameLast));
                strcpy(fileNameLast,fileName);

                memset(&timeLast,0,sizeof(time_t));
                memcpy(&timeLast,&fileStat.st_mtime,sizeof(time_t));//the  last time of modification

                count++;
                continue;
            }

            //if current time is newer than timeLast , update it
            if(JudgeTime(fileStat.st_mtime,timeLast) > 0 )
            {
                memset(fileNameLast,0,sizeof(fileNameLast));
                strcpy(fileNameLast,fileName);

                memset(&timeLast,0,sizeof(time_t));
                memcpy(&timeLast,&fileStat.st_mtime,sizeof(time_t));//the  last time of modification               
            }

        }
        count++;
    }

    //get the fileName index 
    if(strlen(fileNameLast) != 0)
    {
        fileNameLast[strlen(fileNameLast) - strlen(".log") ] = '\0';
    }
    
    char *p = NULL;
    p = strstr(fileNameLast,"log-");

    if(p != NULL)
    {
        p = p + strlen("log-");
       closedir(dp);
       return atoi(p);
    }

    if(closedir(dp) < 0)
    {
        return -2;
    }

    return -3;

}

/*****************************************************************************
 函 数 名  : GetFileSize
 功能描述  : 获得某个文件的大小，用于控制文件大小，防止超限
 输入参数  : char *fileName  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static int GetFileSize(char *fileName)
{
    struct stat fileStat;
    
    if(stat(fileName,&fileStat) == 0)
    {
        
        return fileStat.st_size;//单位是Byte

    }

    return -1;
}

/*****************************************************************************
 函 数 名  : GetDirSize
 功能描述  : 获得某个文件夹内文本文件的大小，用来控制文件夹的大小
 输入参数  : char *dirName  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int GetDirSize(char *dirName)
{
    struct dirent *dirp;
    DIR *dp;
    int size = 0;
    char fileName[MAX_FILENAME_LENGTH];

    if((dp = opendir(dirName)) == NULL)
    {
        return -1;
    }
    
    int flag = dirName[strlen(dirName) - 1] == '/';

    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name,".") == 0 || strcmp(dirp->d_name,"..") == 0)
        {
            continue;
        }
        memset(fileName,0,sizeof(fileName));
        if(flag)
        {
            snprintf(fileName,sizeof(fileName),"%s%s",dirName,dirp->d_name);
        }
        else
        {
            snprintf(fileName,sizeof(fileName),"%s/%s",dirName,dirp->d_name);
        }

        size += GetFileSize(fileName);
    }

    if(closedir(dp) < 0)
    {
        return -2;
    }

    return size;

}

/*****************************************************************************
 函 数 名  : IncreaseLogFileIndex
 功能描述  : 不断增长日志文件的后缀Index防止文件名重复，同时保证循环
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static int IncreaseLogFileIndex()
{
    if(g_LogStruct.index == g_LogStruct.maxSizeAll/g_LogStruct.maxSizeEach)
    {
        g_LogStruct.index = 0;
    }
    
    g_LogStruct.index += 1;

    memset(g_LogStruct.fileName,0,sizeof(g_LogStruct.fileName));

    strcpy(g_LogStruct.fileName,g_LogStruct.pathName);

    //open the file for append log 
    snprintf(g_LogStruct.fileName+strlen(g_LogStruct.fileName),sizeof(g_LogStruct.fileName)-strlen(g_LogStruct.fileName),"log-%d.log",g_LogStruct.index);

    fclose(fp);//free the last file
    fp = NULL;

    if(GetFileSize(g_LogStruct.fileName) >= g_LogStruct.maxSizeEach*1024*1024)
    {
        //delete this file
        remove(g_LogStruct.fileName);
    }

    //"./logDir/log-1.log"
    fp = fopen(g_LogStruct.fileName,"a+");
    if(!fp)
    {
        printf("InitLogSys error to open the file %s : %s\n",g_LogStruct.fileName,strerror(errno));
        return false;
    }

    setbuf(fp,NULL);// no buffer    

    return true;
}
/*****************************************************************************
 函 数 名  : AddContent
 功能描述  : 实际调用的写入文件的函数
 输入参数  : char *logContent  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static int AddContent(char *logContent)
{
    if(!logContent || !fp)
    {
        return false;
    }

    fputs(logContent,fp);

    if(GetFileSize(g_LogStruct.fileName) > g_LogStruct.maxSizeEach*1024*1024)
    {
        IncreaseLogFileIndex();
        
    }

    return true;
}

/*****************************************************************************
 函 数 名  : AddLogItem
 功能描述  : 对外提供接口，用来写日志
 输入参数  : char *logContent  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
static void AddLogItem(char *logContent)
{
    if((flagIsInitOK != 1) && (InitLogSystem(DEFAULT_PATH,DEFAULT_SIZE_ALL,DEFAULT_SIZE_EACH) != 1))
    {
        return ;
    }

    if(!logContent)
    {
        return ;
    }   

    pthread_mutex_lock(&gLockLogFile);
    AddContent(logContent);
    pthread_mutex_unlock(&gLockLogFile);
}


/*****************************************************************************
 函 数 名  : log_debug
 功能描述  : 格式化输出接口函数，可重入，线程安全
 输入参数  : const char* format  
             ...                 
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
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
	strcat(buff,"  ");//用2个字节的空格使日期与具体日志内容分开
	strcat(buff,"<DEBUG>");
	strcat(buff,"  ");	
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

    AddLogItem(buff);
	
}

void log_error(const char* format, ...)
{
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//最前面是当前日期函数
	strcat(buff,"  ");//用2个字节的空格使日期与具体日志内容分开
	strcat(buff,"<ERROR>");
	strcat(buff,"  ");
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//将buff的位置向右移动strlen(buff)字节，最前面这24字节用来存放当前时间，紧接着是2字节的空格用来分割内容和时间
	va_end(argptr);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

    AddLogItem(buff);
}



/*****************************************************************************
 函 数 名  : InitLogSystem
 功能描述  : 日志系统初始化函数
 输入参数  : char *path        日志文件的路径Ｎ，不是某个文件的路径，默认是的路径如./Log/ 
             int max_size_all   日志文件的总占用空间大小限额,要比each大
             int max_size_each  单个日志文件的占用空间大小，过大将无法使用常规编辑器打开
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int InitLogSystem(char *path,int max_size_all,int max_size_each)
{
    
    path = (((path == NULL) || (strlen(path) >= (MAX_PATH_LENGTH - 2))) ? DEFAULT_PATH : path);
    max_size_all = ((max_size_all <= 0) ? DEFAULT_SIZE_ALL : max_size_all);
    max_size_each = ((max_size_all <= 0) ? DEFAULT_SIZE_EACH: max_size_each);//校验参数合法性，不合法则给予默认值。

    memset(&g_LogStruct,0,sizeof(LogSystemStruct));    
    strcpy(g_LogStruct.pathName,path);
    
    if(path[strlen(path) - 1] != '/')//如果路径名最后不是/，则添加/
    {
        strcat(g_LogStruct.pathName,"/");
    }

    int maxIndex = GetLastLogFileIndex(g_LogStruct.pathName);//重新获得后缀，以防程序重启后覆盖最新数据
    
    strcpy(g_LogStruct.fileName,g_LogStruct.pathName);
    g_LogStruct.maxSizeAll= max_size_all < max_size_each ? max_size_each : max_size_all;// MB   总阈值要比单个阈值大
    g_LogStruct.maxSizeEach = max_size_each;//MB
    g_LogStruct.index = maxIndex < 0 ? 1 : maxIndex;

    //test the log dir
    if(access(g_LogStruct.fileName,F_OK) == -1)
    {
        char cmd[256];
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"mkdir -p %s",g_LogStruct.fileName);

        if(system(cmd) == -1)
        {
            printf("create dir  error  %s \n",strerror(errno));
            return false;
        }
        
    }

    //open the file for append log 
    snprintf(g_LogStruct.fileName+strlen(g_LogStruct.fileName),sizeof(g_LogStruct.fileName)-strlen(g_LogStruct.fileName),"log-%d.log",g_LogStruct.index);

    //"./logDir/log-1.log"
    fp = fopen(g_LogStruct.fileName,"a+");
    if(!fp)
    {
        printf("InitLogSys error to open the file %s : %s\n",g_LogStruct.fileName,strerror(errno));
        return false;
    }

    setbuf(fp,NULL);// no buffer

    pthread_mutex_init(&gLockLogFile,NULL);
    flagIsInitOK = 1;

    printf("\nInitLogSys success :  path  %s   maxAll %d MB maxEach  %dMB\n",path,max_size_all,max_size_each);
    return true;
}


