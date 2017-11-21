/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : filebackup.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��6��17��
  ����޸�   :
  ��������   : �����ʵ����һ�������ļ����ݵĹ���
  �����б�   :
              CopyFile
              FileBackup
              JudgeTime
  �޸���ʷ   :
  1.��    ��   : 2015��6��17��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/



/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "filebackup.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

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

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


//�������һ���ļ���Ӧ�ı����ļ��Դ��ڵ�ǰ�ļ���
//���������ļ�����Ϊ����

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


//��src���ļ����ݿ�����desc��ȥ,src�ļ������ڻ��СΪ0ʱ��������
static void CopyFile(char *src,char *desc)
{
    char cmd[512] = {0};
    struct stat fileStat;
	if(src == NULL || desc == NULL)
	{
		return;	
	}
	
	//ֻ��src�ļ��Ĵ�С��Ϊ0ʱ���Ž��и��Ʋ���
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

	if((ret != 0) || (fileStat.st_size == 0))//����Դ�ļ���ʧ���ѱ���գ��Ǿͽ������ļ����Ƶ�Դ�ļ�
	{
		CopyFile(cBackupFileName,fileName);
		return;
	}
    ret = stat(cBackupFileName,&fileStatBackup);
	
	if((ret != 0) || (fileStatBackup.st_size == 0))//���������ļ������ڻ��ѱ���գ���ԭ�ļ����Ƶ������ļ�
	{
		CopyFile(fileName,cBackupFileName);
		return;
	}

    ret = JudgeTime(fileStat.st_mtime, fileStatBackup.st_mtime);

    //Դ�ļ��޸����ڽϱ����ļ���ʱ�����±����ļ�
    if(ret > 0 )
    {
		CopyFile(fileName,cBackupFileName);
		return;
    }
    //�����ļ����ڱȽ���
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

