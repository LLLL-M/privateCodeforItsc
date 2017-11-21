/******************************************************************************

                  版权所有 (C), 2003-2014, 老虎工作室

 ******************************************************************************
  文 件 名   : LogSystem.c
  版 本 号   : 初稿
  作    者   : 老虎
  生成日期   : 2014年6月30日
  最近修改   :
  功能描述   : 
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
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "LogSystem.h"
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
pthread_mutex_t gLockLogFile;//文件锁，防止多线程同时写入文件
volatile PLogSystemStruct pLogStruct = null;//全局日志结构体，其中包括该日志的基本信息
int flagIsInitOK = 0;//是否初始化正确
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static FILE *fp = null;//全局文件指针，用来写入日志数据
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : InitLogSys
 功能描述  : 日志系统初始化函数，只需要在main函数后执行即可完成日志系统的初
             始化
 输入参数  : char *path        日志文件的路径Ｎ，不是某个文件的路径，是文件夹的路径如./Log/ 
             int max_size_all   日志文件的总占用空间大小限额,要比each大
             int max_size_each  单个日志文件的占用空间大小，过大将无法使用常规编辑器打开
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int InitLogSys(char *path,int max_size_all,int max_size_each)
{
    printf("\nInitLogSys  path  %s   maxAll %d M maxEach  %dM\n",path,max_size_all,max_size_each);
    if(!path)
    {
        printf("path can't be null \n");
        return false;
    }

    if((max_size_all <= 0) || (max_size_each <= 0))
    {
        printf("max size must  > 0\n");
        return  false;
    }

    pLogStruct = (PLogSystemStruct)malloc(sizeof(LogSystemStruct));
    if(!pLogStruct)
    {
        printf("alloc pLogStruct error \n");
        return false;
    }

    memset(pLogStruct,0,sizeof(LogSystemStruct));    
    memset(pLogStruct->fileName,0,MAX_FILENAME_LENGTH);
    memset(pLogStruct->pathName,0,MAX_PATH_LENGTH);

    strcpy(pLogStruct->pathName,path);
    //if the last para isn't / , add it
    if(path[strlen(path) - 1] != '/')
    {
        strcat(pLogStruct->pathName,"/");
    }

    int maxIndex = GetLastLogFileIndex(pLogStruct->pathName);//重新获得后缀，以防程序重启后覆盖最新数据
    
    strcpy(pLogStruct->fileName,pLogStruct->pathName);
    pLogStruct->maxSizeAll= max_size_all < max_size_each ? max_size_each : max_size_all;// MB   总阈值要比单个阈值大
    pLogStruct->maxSizeEach = max_size_each;//MB
    pLogStruct->index = maxIndex < 0 ? 1 : maxIndex;

    //test the log dir
    if(access(pLogStruct->fileName,F_OK) == -1)
    {
        //dir exist
       // printf("dir doesn't exist\n");

/*        if(mkdir(pLogStruct->pathName,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP) == -1)
        {
            printf("create dir  error %s\n",strerror(errno));
            free(pLogStruct);
            return false;
        }*/
        char cmd[256];
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"mkdir -p %s",pLogStruct->fileName);

        if(system(cmd) == -1)
        {
            printf("create dir  error  %s \n",strerror(errno));
            free(pLogStruct);
            return false;
        }
        
    }

    //open the file for append log 
    snprintf(pLogStruct->fileName+strlen(pLogStruct->fileName),sizeof(pLogStruct->fileName)-strlen(pLogStruct->fileName),"log-%d.log",pLogStruct->index);

    //"./logDir/log-1.log"
    fp = fopen(pLogStruct->fileName,"a+");
    if(!fp)
    {
        printf("InitLogSys error to open the file %s : %s\n",pLogStruct->fileName,strerror(errno));
        free(pLogStruct);
        return false;
    }

    setbuf(fp,NULL);// no buffer

    printf("\nInitLogSys success  \n\n");

    pthread_mutex_init(&gLockLogFile,NULL);
    flagIsInitOK = 1;

    return true;
}

/*****************************************************************************
 函 数 名  : GetFileSize
 功能描述  : 获得某个文件的大小，用于控制文件大小，防止超限
 输入参数  : char *fileName  
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int GetFileSize(char *fileName)
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
 返 回 值  :  
 
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

    if((dp = opendir(dirName)) == null)
    {
        return -1;
    }
    
    int flag = dirName[strlen(dirName) - 1] == '/';

    while((dirp = readdir(dp)) != null)
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
        //printf("%s\n",fileName);

    }

    if(closedir(dp) < 0)
    {
        return -2;
    }

    return size;

}
/*****************************************************************************
 函 数 名  : JudgeTime
 功能描述  : 比较两个时间结构体time_t的大小，大于时返回大于0的值，小于返回小
             于0的值，相等时返回0
 输入参数  : time_t val_1  
             time_t val_2  
 返 回 值  :  
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int JudgeTime(time_t val_1,time_t val_2)
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
 返 回 值  :  
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int GetLastLogFileIndex(char *dirName)
{
    struct dirent *dirp;
    DIR *dp;
    int count = 0;
    char fileName[MAX_FILENAME_LENGTH];
    struct stat fileStat;
    char fileNameLast[MAX_FILENAME_LENGTH];
    time_t timeLast = 0;
    
    if((dp = opendir(dirName)) == null)
    {
        return -1;
    }
    
    int flag = dirName[strlen(dirName) - 1] == '/';

    memset(fileName,0,sizeof(fileName));
    memset(fileNameLast,0,sizeof(fileNameLast));
    while((dirp = readdir(dp)) != null)
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
        //printf("%s\n",fileName);

        count++;
    }

    //get the fileName index 
    if(strlen(fileNameLast) != 0)
    {
        fileNameLast[strlen(fileNameLast) - strlen(".log") ] = '\0';
    }
    
    char *p = null;
    p = strstr(fileNameLast,"log-");

    if(p != null)
    {
        p = p + strlen("log-");
       // printf("==>%s\n",p);
       closedir(dp);
       //printf("GetLastLogFileIndex    %d\n",atoi(p));
       return atoi(p);
    }

    if(closedir(dp) < 0)
    {
        return -2;
    }

    return -3;

}


/*****************************************************************************
 函 数 名  : IncreaseLogFileIndex
 功能描述  : 不断增长日志文件的后缀Index防止文件名重复，同时保证循环
 输入参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int IncreaseLogFileIndex()
{
    if(pLogStruct->index == pLogStruct->maxSizeAll/pLogStruct->maxSizeEach)
    {
        pLogStruct->index = 0;
    }
    
    pLogStruct->index += 1;

    memset(pLogStruct->fileName,0,sizeof(pLogStruct->fileName));

    strcpy(pLogStruct->fileName,pLogStruct->pathName);

    //open the file for append log 
    snprintf(pLogStruct->fileName+strlen(pLogStruct->fileName),sizeof(pLogStruct->fileName)-strlen(pLogStruct->fileName),"log-%d.log",pLogStruct->index);

    fclose(fp);//free the last file
    fp = null;

    if(GetFileSize(pLogStruct->fileName) >= pLogStruct->maxSizeEach*1024*1024)
    {
        //delete this file
        remove(pLogStruct->fileName);
    }

    //"./logDir/log-1.log"
    fp = fopen(pLogStruct->fileName,"a+");
    if(!fp)
    {
        printf("InitLogSys error to open the file %s : %s\n",pLogStruct->fileName,strerror(errno));
        free(pLogStruct);
        return false;
    }

    setbuf(fp,NULL);// no buffer    

    return true;
}
/*****************************************************************************
 函 数 名  : AddContent
 功能描述  : 实际调用的写入文件的函数
 输入参数  : char *logContent  
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int AddContent(char *logContent)
{
    if(!logContent || !fp)
    {
        printf("AddContent   \n");
        return false;
    }

    fputs(logContent,fp);

    if(GetFileSize(pLogStruct->fileName) > pLogStruct->maxSizeEach*1024*1024)
    {
        printf("new log file \n");
        IncreaseLogFileIndex();
        
    }

    //printf("now the size of file is %d Byte\n",GetFileSize(pLogStruct->fileName));
    //printf("now the size of dir is %d Byte\n",GetDirSize(pLogStruct->pathName));

    return true;
}
/*****************************************************************************
 函 数 名  : AddLogItem
 功能描述  : 对外提供接口，用来写日志
 输入参数  : char *logContent  
 返 回 值  :  
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
void AddLogItem(char *logContent)
{
    if(flagIsInitOK != 1)
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
    
    //semPost(&sem_add,100);

    //printf("Add  %llu\n",GetTickCount());
}
/*****************************************************************************
 函 数 名  : DestroyLogSys
 功能描述  : 系统异常退出，应该释放分配的内存
 输入参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年6月30日
    作    者   : 老虎
    修改内容   : 新生成函数

*****************************************************************************/
int DestroyLogSys()
{
    fclose(fp);

    free(pLogStruct);

    pLogStruct = null;

    printf("\n%s success \n",__func__);
    return true;
}



