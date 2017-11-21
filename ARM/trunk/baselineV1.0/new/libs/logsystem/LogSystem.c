/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : LogSystem.c
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��30��
  ����޸�   :
  ��������   : ����־ϵͳ�÷�:
                1. ����LogSystem.hͷ�ļ�
                2. ����InitLogSystem��������ʵ�����������־·���������С��������־�ļ���С���ò����ʡ�ԣ�ȱʡΪ��ǰ·����
                    ./Log/��Ĭ�ϻ�����100M��Ĭ�ϵ����ļ���С��5M
                3. ֱ�ӵ���log_debug��log_err���ɡ�
  
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
#include "LogSystem.h"

static LogSystemStruct g_LogStruct;         //ȫ����־�ṹ�壬���а�������־�Ļ�����Ϣ
static FILE *fp = NULL;                     //ȫ���ļ�ָ�룬����д����־����
static int flagIsInitOK = 0;                //�Ƿ��ʼ����ȷ

pthread_mutex_t gLockLogFile;               //�ļ�������ֹ���߳�ͬʱд���ļ�

#define false 0
#define true  1
#define DEFAULT_PATH            "./Log/"            //��־�ļ���Ĭ��·��
#define DEFAULT_SIZE_ALL        100                 //��־�ļ����ܴ�С��100MB
#define DEFAULT_SIZE_EACH       5                   //ÿ����־�ļ���Ĭ�ϴ�С��5M


/*****************************************************************************
 �� �� ��  : GetLocalTime
 ��������  : ��õ�ǰʱ�䣬�������̰߳�ȫ
 �������  : char *localTime  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : JudgeTime
 ��������  : �Ƚ�����ʱ��ṹ��time_t�Ĵ�С������ʱ���ش���0��ֵ��С�ڷ���С
             ��0��ֵ�����ʱ����0
 �������  : time_t val_1  
             time_t val_2  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : GetLastLogFileIndex
 ��������  : ��ʼ��ʱ�Ȼ�ȡ��ǰ��־�ļ�����������־�ĺ�׺index���Է���1��ʼ
             �������Ḳ���������ݡ���֤��־��Զ�������µ���־�ļ��
 �������  : char *dirName  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : GetFileSize
 ��������  : ���ĳ���ļ��Ĵ�С�����ڿ����ļ���С����ֹ����
 �������  : char *fileName  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static int GetFileSize(char *fileName)
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
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
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
 �� �� ��  : IncreaseLogFileIndex
 ��������  : ����������־�ļ��ĺ�׺Index��ֹ�ļ����ظ���ͬʱ��֤ѭ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddContent
 ��������  : ʵ�ʵ��õ�д���ļ��ĺ���
 �������  : char *logContent  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : AddLogItem
 ��������  : �����ṩ�ӿڣ�����д��־
 �������  : char *logContent  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : log_debug
 ��������  : ��ʽ������ӿں����������룬�̰߳�ȫ
 �������  : const char* format  
             ...                 
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���
 
  2.��    ��   : 2014��6��27��
    ��    ��   : �ϻ�
    �޸�����   : Ϊ֧�ֱ�����־ϵͳ���޸�buf��ʼ
*****************************************************************************/
void log_debug(const char* format, ...)
{
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//��ǰ���ǵ�ǰ���ں���
	strcat(buff,"  ");//��2���ֽڵĿո�ʹ�����������־���ݷֿ�
	strcat(buff,"<DEBUG>");
	strcat(buff,"  ");	
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

    AddLogItem(buff);
	
}

void log_error(const char* format, ...)
{
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//��ǰ���ǵ�ǰ���ں���
	strcat(buff,"  ");//��2���ֽڵĿո�ʹ�����������־���ݷֿ�
	strcat(buff,"<ERROR>");
	strcat(buff,"  ");
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,"\n");
	fprintf(stderr,"%s",buff);

    AddLogItem(buff);
}



/*****************************************************************************
 �� �� ��  : InitLogSystem
 ��������  : ��־ϵͳ��ʼ������
 �������  : char *path        ��־�ļ���·���Σ�����ĳ���ļ���·����Ĭ���ǵ�·����./Log/ 
             int max_size_all   ��־�ļ�����ռ�ÿռ��С�޶�,Ҫ��each��
             int max_size_each  ������־�ļ���ռ�ÿռ��С�������޷�ʹ�ó���༭����
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int InitLogSystem(char *path,int max_size_all,int max_size_each)
{
    
    path = (((path == NULL) || (strlen(path) >= (MAX_PATH_LENGTH - 2))) ? DEFAULT_PATH : path);
    max_size_all = ((max_size_all <= 0) ? DEFAULT_SIZE_ALL : max_size_all);
    max_size_each = ((max_size_all <= 0) ? DEFAULT_SIZE_EACH: max_size_each);//У������Ϸ��ԣ����Ϸ������Ĭ��ֵ��

    memset(&g_LogStruct,0,sizeof(LogSystemStruct));    
    strcpy(g_LogStruct.pathName,path);
    
    if(path[strlen(path) - 1] != '/')//���·���������/�������/
    {
        strcat(g_LogStruct.pathName,"/");
    }

    int maxIndex = GetLastLogFileIndex(g_LogStruct.pathName);//���»�ú�׺���Է����������󸲸���������
    
    strcpy(g_LogStruct.fileName,g_LogStruct.pathName);
    g_LogStruct.maxSizeAll= max_size_all < max_size_each ? max_size_each : max_size_all;// MB   ����ֵҪ�ȵ�����ֵ��
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


