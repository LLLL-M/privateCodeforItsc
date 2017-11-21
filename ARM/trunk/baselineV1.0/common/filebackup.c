/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : filebackup.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年6月17日
  最近修改   :
  功能描述   : 这里简单实现了一个配置文件备份的功能
  函数列表   :
              CopyFile
              FileBackup
              JudgeTime
  修改历史   :
  1.日    期   : 2015年6月17日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/



/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "filebackup.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

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


//简单起见，一个文件对应的备份文件仍处于当前文件夹
//且这两个文件，互为备份

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


//将src的文件内容拷贝到desc中去,src文件不存在或大小为0时，不拷贝
static void CopyFile(char *src,char *desc)
{
    char cmd[512] = {0};
    struct stat fileStat;
	if(src == NULL || desc == NULL)
	{
		return;	
	}
	
	//只有src文件的大小不为0时，才进行复制操作
	if(0 == stat(src,&fileStat))
	{
		if(fileStat.st_size == 0)
		{
            return;
		}

		snprintf(cmd,sizeof(cmd),"cp -f %s %s",src,desc);
		if(0 == system(cmd))
		{
            printf("copy %s to %s succeed\n",src,desc);
		}
		else
		{   
            printf("copy %s to %s failed\n",src,desc);
		}
	}

}

void FileBackup(char *fileName)
{
    char cBackupFileName[256] = {0};
    
    struct stat fileStat;
    struct stat fileStatBackup;

    int ret = 0;

    if(fileName == NULL)
    {
        return;
    }

    snprintf(cBackupFileName,sizeof(cBackupFileName) - 4,"%s_bak",fileName);

    ret = stat(fileName,&fileStat);

	if((ret != 0) || (fileStat.st_size == 0))//表明源文件丢失或已被清空，那就将备份文件复制到源文件
	{
		CopyFile(cBackupFileName,fileName);
		return;
	}
    ret = stat(cBackupFileName,&fileStatBackup);
	
	if((ret != 0) || (fileStatBackup.st_size == 0))//表明备份文件不存在或已被清空，则将原文件复制到备份文件
	{
		CopyFile(fileName,cBackupFileName);
		return;
	}

    ret = JudgeTime(fileStat.st_mtime, fileStatBackup.st_mtime);

    //源文件修改日期较备份文件新时，更新备份文件
    if(ret > 0 )
    {
		CopyFile(fileName,cBackupFileName);
		return;
    }
    //备份文件日期比较新
    else if(ret < 0)
    {
		CopyFile(cBackupFileName,fileName);	
    	return;
	}

}

#if 0
int main()
{
	char *fileName = "/share/dailyTest/test.txt";	
	FileBackup(fileName);	
}

#endif

