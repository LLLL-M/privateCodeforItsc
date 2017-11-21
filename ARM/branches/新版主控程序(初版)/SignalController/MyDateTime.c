#include "MyDateTime.h"


static TimeStruct gTimeStruct;
static pthread_rwlock_t gRWLock;
static unsigned short IsDateTimeInitOK = false;


int InitDateTime()
{
    if(pthread_rwlock_init(&gRWLock,NULL) == 0)
    {
        log_debug("InitDateTime OK \n");

        IsDateTimeInitOK = true;
        
        UpdateDateTime();
        
        return true;
    }
    else
    {
        return false;
    }
}

void UpdateDateTime()
{
    if(!IsDateTimeInitOK)
    {
        return ;
    }

    time_t timep;
    time(&timep);
    struct tm now;
    localtime_r(&timep,&now);   


    pthread_rwlock_wrlock(&gRWLock);

    gTimeStruct.year = now.tm_year + 1900;
    gTimeStruct.mon = now.tm_mon + 1;
    gTimeStruct.day = now.tm_mday;
    gTimeStruct.hour = now.tm_hour;
    gTimeStruct.min  = now.tm_min;
    gTimeStruct.sec = now.tm_sec;
    gTimeStruct.week = now.tm_wday + 1;
    
    pthread_rwlock_unlock(&gRWLock);
}

int GetYear()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int year = gTimeStruct.year ;
    pthread_rwlock_unlock(&gRWLock);

    return year;
}

int GetMonth()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int month = gTimeStruct.mon;
    pthread_rwlock_unlock(&gRWLock);

    return month;
}


int GetDay()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int day = gTimeStruct.day;
    pthread_rwlock_unlock(&gRWLock);

    return day;
}

int GetHour()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int hour = gTimeStruct.hour;
    pthread_rwlock_unlock(&gRWLock);

    return hour;
}

int GetMin()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int min = gTimeStruct.min;
    pthread_rwlock_unlock(&gRWLock);

    return min;
}

int GetSec()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int sec = gTimeStruct.sec;
    pthread_rwlock_unlock(&gRWLock);

    return sec;
}

int GetWeek()
{
    if(!IsDateTimeInitOK)
    {
        return 0;
    }
    pthread_rwlock_rdlock(&gRWLock);
    int week = gTimeStruct.week;
    pthread_rwlock_unlock(&gRWLock);

    return week;
}


void ThreadUpdateDateTime()
{
    while(1)
    {

        UpdateDateTime();

        sleep(1);

    }
}


int DestroyDateTime()
{
    return false;
}

#if 0
int main(int argc ,char **argv)
{
    PCurrentTime pTime = InitDateTime();

    log_debug("Thread_%d  %d-%02d-%02d %02d:%02d:%02d\n",*index,pTime->getYear(pTime),pTime->getMonth(pTime)
                                                            ,pTime->getDay(pTime),pTime->getHour(pTime)
                                                            ,pTime->getMin(pTime),pTime->getSec(pTime));
    free(pTime);


}
#endif



