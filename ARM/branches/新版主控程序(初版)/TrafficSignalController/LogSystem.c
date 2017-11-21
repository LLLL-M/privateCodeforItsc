/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : LogSystem.c
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��30��
  ����޸�   :
  ��������   : 
  �����б�   :
              AddContent
              AddLogItem
              DestroyLogSys
              GetDirSize
              GetFileSize
              IncreaseLogFileIndex
              InitLogSys
  �޸���ʷ   :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "LogSystem.h"
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
pthread_mutex_t gLockLogFile;//�ļ�������ֹ���߳�ͬʱд���ļ�
volatile PLogSystemStruct pLogStruct = null;//ȫ����־�ṹ�壬���а�������־�Ļ�����Ϣ
int flagIsInitOK = 0;//�Ƿ��ʼ����ȷ
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static FILE *fp = null;//ȫ���ļ�ָ�룬����д����־����
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : InitLogSys
 ��������  : ��־ϵͳ��ʼ��������ֻ��Ҫ��main������ִ�м��������־ϵͳ�ĳ�
             ʼ��
 �������  : char *path        ��־�ļ���·���Σ�����ĳ���ļ���·�������ļ��е�·����./Log/ 
             int max_size_all   ��־�ļ�����ռ�ÿռ��С�޶�,Ҫ��each��
             int max_size_each  ������־�ļ���ռ�ÿռ��С�������޷�ʹ�ó���༭����
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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

    int maxIndex = GetLastLogFileIndex(pLogStruct->pathName);//���»�ú�׺���Է����������󸲸���������
    
    strcpy(pLogStruct->fileName,pLogStruct->pathName);
    pLogStruct->maxSizeAll= max_size_all < max_size_each ? max_size_each : max_size_all;// MB   ����ֵҪ�ȵ�����ֵ��
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
 �� �� ��  : GetFileSize
 ��������  : ���ĳ���ļ��Ĵ�С�����ڿ����ļ���С����ֹ����
 �������  : char *fileName  
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int GetFileSize(char *fileName)
{
    struct stat fileStat;
    
    if(stat(fileName,&fileStat) == 0)
    {
        
        return fileStat.st_size;//��λ��Byte

    }

    return -1;
}

/*****************************************************************************
 �� �� ��  : GetDirSize
 ��������  : ���ĳ���ļ������ı��ļ��Ĵ�С�����������ļ��еĴ�С
 �������  : char *dirName  
 �� �� ֵ  :  
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : JudgeTime
 ��������  : �Ƚ�����ʱ��ṹ��time_t�Ĵ�С������ʱ���ش���0��ֵ��С�ڷ���С
             ��0��ֵ�����ʱ����0
 �������  : time_t val_1  
             time_t val_2  
 �� �� ֵ  :  
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : GetLastLogFileIndex
 ��������  : ��ʼ��ʱ�Ȼ�ȡ��ǰ��־�ļ�����������־�ĺ�׺index���Է���1��ʼ
             �������Ḳ���������ݡ���֤��־��Զ�������µ���־�ļ��
 �������  : char *dirName  
 �� �� ֵ  :  
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : IncreaseLogFileIndex
 ��������  : ����������־�ļ��ĺ�׺Index��ֹ�ļ����ظ���ͬʱ��֤ѭ��
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddContent
 ��������  : ʵ�ʵ��õ�д���ļ��ĺ���
 �������  : char *logContent  
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddLogItem
 ��������  : �����ṩ�ӿڣ�����д��־
 �������  : char *logContent  
 �� �� ֵ  :  
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : DestroyLogSys
 ��������  : ϵͳ�쳣�˳���Ӧ���ͷŷ�����ڴ�
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int DestroyLogSys()
{
    fclose(fp);

    free(pLogStruct);

    pLogStruct = null;

    printf("\n%s success \n",__func__);
    return true;
}



