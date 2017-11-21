/******************************************************************************

                  ��Ȩ���� (C), 2003-2014, �ϻ�������

 ******************************************************************************
  �� �� ��   : Util.c
  �� �� ��   : ����
  ��    ��   : �ϻ�
  ��������   : 2014��6��25��
  ����޸�   :
  ��������   : ����������һЩͨ�ýӿ���Log_debug��
  �����б�   :
              GetLocalTime
              GetTickCount
              log_debug
  �޸���ʷ   :
  1.��    ��   : 2014��6��25��
    ��    ��   : �ϻ�
    �޸�����   : �����ļ�

******************************************************************************/

#include "Util.h"

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

void GetLocalDate(char *date)
{
    if(!date)
    {
        return;
    }

    time_t timep;
    time(&timep);

    struct tm now;
    localtime_r(&timep,&now);

    strftime(date,24,"%Y-%m-%d-",&now);
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
	//printf("==========\n");
	//char localTime[24];
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//��ǰ���ǵ�ǰ���ں���
	//printf("strlen buff  %d\n",strlen(buff));
	strcat(buff,"  ");//��2���ֽڵĿո�ʹ�����������־���ݷֿ�
	strcat(buff,"<DEBUG>");
	strcat(buff,"  ");	
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,"\n");
	printf("%s",buff);

#ifdef LOG4HIK
    AddLogItem(buff);
#endif
	
	//memset(localTime,0,sizeof(localTime));
	//GetLocalTime(localTime);
	//printf("%s  %s\n",localTime,buff);
}

void log_error(const char* format, ...)
{
	//printf("==========\n");
	//char localTime[24];
	char buff[2048] = {0};
	memset(buff,0,sizeof(buff));
	GetLocalTime(buff);//��ǰ���ǵ�ǰ���ں���
	//printf("strlen buff  %d\n",strlen(buff));
	strcat(buff,"  ");//��2���ֽڵĿո�ʹ�����������־���ݷֿ�
	strcat(buff,"<ERROR>");
	strcat(buff,"  ");
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buff+strlen(buff),sizeof(buff)-strlen(buff)-1, format, argptr);//��buff��λ�������ƶ�strlen(buff)�ֽڣ���ǰ����24�ֽ�������ŵ�ǰʱ�䣬��������2�ֽڵĿո������ָ����ݺ�ʱ��
	va_end(argptr);
	strcat(buff,"\n");
	printf("%s",buff);

#ifdef LOG4HIK
    AddLogItem(buff);
#endif
	
	//memset(localTime,0,sizeof(localTime));
	//GetLocalTime(localTime);
	//printf("%s  %s\n",localTime,buff);
}


/*****************************************************************************
 �� �� ��  : semPost
 ��������  : �����߳�ͨ��semͬ��
 �������  : sem_t * pSem   
             int nMaxCount  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : semWait
 ��������  : �����߳�ͨ��semͬ��
 �������  : sem_t * pSem  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int semWait(sem_t * pSem)
{
	int nRet;
	nRet=sem_wait(pSem);

	while (sem_trywait(pSem)==0) {}
	return nRet;
}
/*****************************************************************************
 �� �� ��  : gettid
 ��������  : ����߳�ID�����ڵ���top -Hp
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
pid_t gettid()
{
     return syscall(SYS_gettid); 
}
/*****************************************************************************
 �� �� ��  : ThreadClearMem
 ��������  : �ļ���дʱϵͳ���ǻỺ��ܴ�ռ䣬����߳̿��Զ�ʱ��ջ�����
 �������  : ��
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��6��30��
    ��    ��   : �ϻ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void * ThreadClearMem()
{

    while(1)
    {
        sleep(1);
        system("sync");
        system("echo 1 > /proc/sys/vm/drop_caches");
        system("echo \"0\" >> /proc/sys/vm/overcommit_memory");

    }

    return null;
}


/*****************************************************************************
 �� �� ��  : IsItemInIntArray
 ��������  : �ж����ͱ���val�Ƿ���array������
 �������  : int *array  
             int length  
             int val     
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2014��7��3��
    ��    ��   : xiaowh
    �޸�����   : �����ɺ���

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

pthread_t   ThreadCreate(thrfunc *routine, void *args)
{
    pthread_t id = 0;
    pthread_attr_t attributes;
    int ret;

    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE);

    ret =  pthread_create(&id, &attributes, routine, args);
    if (ret != 0)
    {
        log_debug("%s, pthread_create failed with %d\n", __FUNCTION__, ret);

    }

    pthread_attr_destroy(&attributes);

    AddThreadControllerItem(id);
    
    return id;
    
}


#if 0

int main(int argc,char **argv)
{
    log_debug("Hello World ");

    return 0;
}


#endif


