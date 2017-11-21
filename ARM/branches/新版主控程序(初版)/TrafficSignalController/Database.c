/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : Database.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2014年12月2日
  最近修改   :
  功能描述   : 监控/home/下日志文件的变化，一旦变化，则将新添加的日志信息保
               存到数据库中。前提是需要有数据库文件。
  函数列表   :
              CreateTableByType
              GetLastRecord
              InsertTableItemByType
              JudgeStructTmTime
              ReLoadLoginCfg
              SendFileContentToDB
              ShowTableContentByType
              ThreadCheckCfgChanged
  修改历史   :
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Database.h"
#include "common.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern FAULT_CFG gFaultCfg;

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void get_current_params();
extern void get_special_params();
extern void get_fault_cfg(char *cfgName);
extern void GPSInit();
extern void HardwareWatchdogInit();
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : ReLoadLoginCfg
 功能描述  : 重新加载配置文件到内存，更新全局变量。
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void ReLoadLoginCfg()
{
    get_special_params(PATH_LOGIN);

    get_current_params(PATH_LOGIN);

    get_fault_cfg(PATH_LOGIN);

    HardwareWatchdogInit();

    GPSInit();
}

/*****************************************************************************
 函 数 名  : CreateTableByType
 功能描述  : 根据表格类型，创建表
 输入参数  : TABLE_TYPE type  
             sqlite3 *db      
 返 回 值  : 成功返回0，失败返回1
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int CreateTableByType(TABLE_TYPE type,sqlite3 *db)
{
    char *pErrorMsg = NULL;
    int ret = 0;
    char tableName[25] = "0";
    char sql[1024] = "0";

    if(type == TABLE_RECORD)
    {
        strcpy(tableName,TABLE_RECORD_NAME);   
    }
    else if(type == TABLE_FAULT)
    {
        strcpy(tableName,TABLE_FAULT_NAME);        
    }

    snprintf(sql,sizeof(sql),"CREATE TABLE IF NOT EXISTS %s ( Id INTEGER PRIMARY KEY AUTOINCREMENT , Type INTEGER , Date VARCHAR(30) , Msg VARCHAR(100) );",tableName);

    ret = sqlite3_exec(db,sql,NULL,NULL, &pErrorMsg);
    if(ret != SQLITE_OK)
    {
        log_error("error to create the table , %s\n",sqlite3_errmsg(db));

        return 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : InsertTableItemByType
 功能描述  : 根据表格类型及具体表项的类型，插入数据到数据库
 输入参数  : TABLE_TYPE type_table      
             TABLE_ITEM_TYPE type_item  
             char *dateTime             
             char *Msg                  
             sqlite3 *db                
 返 回 值  : 成功返回0，失败返回1
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int InsertTableItemByType(TABLE_TYPE type_table,TABLE_ITEM_TYPE type_item,char *dateTime,char *Msg,sqlite3 *db)
{
   // printf("%s  %s  %s\n",__func__,dateTime,Msg);
    char *pErrorMsg = NULL;
    int ret = 0;
    char tableName[25] = "0";
    char sql[1024] = "0";

    if(type_table == TABLE_RECORD)
    {
        strcpy(tableName,TABLE_RECORD_NAME);   
    }
    else if(type_table == TABLE_FAULT)
    {
        strcpy(tableName,TABLE_FAULT_NAME);        
    }

    snprintf(sql,sizeof(sql),"INSERT INTO %s VALUES (NULL,%d,'%s','%s') ;",tableName,type_item,dateTime,Msg);

    ret = sqlite3_exec(db,sql,NULL,NULL, &pErrorMsg);
    if(ret != SQLITE_OK)
    {
        log_error("error to INSERT the table , %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    log_debug("%s  succeed !\n",__func__);
    return 0;    
}

/*****************************************************************************
 函 数 名  : ShowTableContentByType
 功能描述  : 测试用，打印出表格的内容
 输入参数  : TABLE_TYPE type  
             sqlite3 *db      
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int ShowTableContentByType(TABLE_TYPE type,sqlite3 *db)
{
    int ret = 0;
    char tableName[25] = "0";
    char sql[1024] = "0";
    sqlite3_stmt *stmt = NULL;

    if(type == TABLE_RECORD)
    {
        strcpy(tableName,TABLE_RECORD_NAME);   
    }
    else if(type == TABLE_FAULT)
    {
        strcpy(tableName,TABLE_FAULT_NAME);        
    }

    snprintf(sql,sizeof(sql),"SELECT * FROM %s ;",tableName);

    ret = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
    if(ret != SQLITE_OK)
    {
        log_error("error to prepare  \n");
        return  1;
    }

    int nCol = 0;
    int nTemp = 0;
    const unsigned  char *pTemp = NULL;

    while(1)
    {
        ret = sqlite3_step(stmt);

        if(ret == SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            break;
        }

        if(SQLITE_ROW == ret)
        {
            nCol = 0;
            nTemp = sqlite3_column_int(stmt,nCol++);
            log_debug("Id :  %d ",nTemp);

            nTemp = sqlite3_column_int(stmt,nCol++);
            log_debug("Type :  %d ",nTemp);

            pTemp = sqlite3_column_text(stmt,nCol++);
            log_debug("  Date  %s  ",pTemp);

            pTemp = sqlite3_column_text(stmt,nCol++);
            log_debug("  Msg  %s  \n",pTemp);

            continue;
        }

        log_error("error !\n");
        sqlite3_finalize(stmt);
        break;
    }

    return 0;    

}
/*****************************************************************************
 函 数 名  : GetLastRecord
 功能描述  : 获取指定表格的最后一项数据，也就是ID最大的那个，用来比较要不要
             把当前数据插入到数据库中
 输入参数  : TABLE_TYPE type            
             TABLE_ITEM_TYPE type_item  
             sqlite3 *db                
             char *timeBuf              
 返 回 值  : 成功返回0，失败返回1
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int GetLastRecord(TABLE_TYPE type,TABLE_ITEM_TYPE type_item,sqlite3 *db,char *timeBuf)
{
    int ret = 0;
    char tableName[25] = "0";
    char sql[1024] = "0";
    sqlite3_stmt *stmt = NULL;

    int flag = 0;

    if(type == TABLE_RECORD)
    {
        strcpy(tableName,TABLE_RECORD_NAME);   
    }
    else if(type == TABLE_FAULT)
    {
        strcpy(tableName,TABLE_FAULT_NAME);        
    }

    snprintf(sql,sizeof(sql),"SELECT Date FROM %s WHERE Type = %d;",tableName,type_item);

    ret = sqlite3_prepare(db,sql,strlen(sql),&stmt,NULL);
    if(ret != SQLITE_OK)
    {
        log_error("error to prepare  \n");
        return  1;
    }

    const unsigned  char *pTemp = NULL;
    char tempBuf[30];

    while(1)
    {
        ret = sqlite3_step(stmt);

        if(ret == SQLITE_DONE)
        {
            if(flag != 0)
            {
              //  printf("tempBuf   %s \n",tempBuf);
                strcpy(timeBuf,tempBuf);

            }
            sqlite3_finalize(stmt);
            break;
        }

        if(SQLITE_ROW == ret)
        {
            flag = 1;
            pTemp = sqlite3_column_text(stmt,0);
            //printf("  Date  %s  \n",pTemp);

            memset(tempBuf,0,sizeof(tempBuf));
            strcpy(tempBuf,(char *)pTemp);
            //memcpy(tempBuf,pTemp,strlen(pTemp));
            continue;
        }

      //  printf("error !\n");
        sqlite3_finalize(stmt);
        break;
    }
    return 0;    

}

/*****************************************************************************
 函 数 名  : JudgeStructTmTime
 功能描述  : 比较两个struct tm型的结构体
 输入参数  : struct tm tm_1  
             struct tm tm_2  
 返 回 值  : 相等返回0，前者大返回1，否则返回-1
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int JudgeStructTmTime(struct tm tm_1,struct tm tm_2)
{
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
 函 数 名  : SendFileContentToDB
 功能描述  : 判断当前项需要写入到数据中后，执行该函数可以将内容写入数据库
 输入参数  : TABLE_TYPE type_table      
             TABLE_ITEM_TYPE type_item  
             char *fileName             
             struct tm  time            
             sqlite3 *db                
 返 回 值  : 成功返回0，失败返回1
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int SendFileContentToDB(TABLE_TYPE type_table,TABLE_ITEM_TYPE type_item,char *fileName,struct tm  time,sqlite3 *db)
{
    FILE *fp = NULL;
	struct tm t_temp; 
    
    fp = fopen(fileName,"r");
    if(!fp)
    {
        return 1;
    }

    char buf[1024];
    char dateTime[60];
    char *pTemp = NULL;

    while(fgets(buf,sizeof(buf),fp) != NULL)
    {
        pTemp = strchr(buf,' ');
        if(pTemp != NULL)
        {
            sscanf(buf,"%d.%d.%d-%d:%d:%d ",&(t_temp.tm_year),&t_temp.tm_mon,&t_temp.tm_mday,&t_temp.tm_hour,&t_temp.tm_min,&t_temp.tm_sec);
        }
        else
        {
            break;
        }
       /* printf("file   date  %d.%d.%d-%d:%d:%d   %s\n",
        		        t_temp.tm_year,t_temp.tm_mon,t_temp.tm_mday,
        		        t_temp.tm_hour,t_temp.tm_min,t_temp.tm_sec,pTemp+1);

        printf("database   date  %d.%d.%d-%d:%d:%d \n",
        		        time.tm_year,time.tm_mon,time.tm_mday,
        		        time.tm_hour,time.tm_min,time.tm_sec);*/

        if(JudgeStructTmTime(t_temp,time) > 0)
        {
            memset(dateTime,0,sizeof(dateTime));
            sprintf(dateTime,"%d.%d.%d-%d:%d:%d",
            		        t_temp.tm_year,t_temp.tm_mon,t_temp.tm_mday,
            		        t_temp.tm_hour,t_temp.tm_min,t_temp.tm_sec);

            //insert data
            InsertTableItemByType(type_table,type_item,dateTime,pTemp,db);
        }

    }

    fclose(fp);
    return 0;

}

/*****************************************************************************
 函 数 名  : ThreadCheckCfgChanged
 功能描述  : 这个线程是用来监控配置文件或者日志文件是否改变的，每1秒钟监测一
             次，如果发生 变化，则根据配置文件的类型，来决定是写入数据库还是
             更新全局
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void * ThreadCheckCfgChanged(void *arg)
{
    char fileName[MAX_CHECK_FILE_NUM][256];
    time_t fileTime[MAX_CHECK_FILE_NUM];
    int fileType[MAX_CHECK_FILE_NUM];
    int flagIsFirstRun[MAX_CHECK_FILE_NUM];
    int itemType[MAX_CHECK_FILE_NUM];
    int flagIsSave[MAX_CHECK_FILE_NUM];
    
    char timeBuf[60];
    int rc = 0;
    
    struct stat fileStat;
    int i = 0;
    struct tm tempTime;

    memset(fileName,0,sizeof(fileName));
    memset(fileTime,0,sizeof(fileTime));
    memset(flagIsFirstRun,0,sizeof(flagIsFirstRun));
    memset(flagIsSave,0,sizeof(flagIsSave));

    strcpy(fileName[0],PATH_FAULT);
    strcpy(fileName[1],PATH_KEYBOARD);
    strcpy(fileName[2],PATH_STARTUP);
    strcpy(fileName[3],PATH_LOGIN);

    fileType[0] = TABLE_FAULT;
    fileType[1] = TABLE_RECORD;
    fileType[2] = TABLE_RECORD;
    
    itemType[0] = ITEM_LAMP;
    itemType[1] = ITEM_KEYBOARD;
    itemType[2] = ITEM_STARTUP;

    flagIsSave[0] = gFaultCfg.nIsControlRecord;
    flagIsSave[1] = gFaultCfg.nIsLogRecord;
    flagIsSave[2] = gFaultCfg.nIsLogRecord;

    sqlite3 *db = NULL;
    char *pDatabasePath = DATABASE_NAME;    


    rc = sqlite3_open(pDatabasePath,&db);

    if(rc != SQLITE_OK)
    {
        log_error("error to open the database , %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        
    }

    while(1)
    {

        for(i = 0 ; i < MAX_CHECK_FILE_NUM ; i++)
        {
            if(stat(fileName[i],&fileStat) == 0)
            {
                if(flagIsFirstRun[i] == 0)
                {
                    flagIsFirstRun[i] = 1;
                    memset(&fileTime[i],0,sizeof(time_t));
                    memcpy(&fileTime[i],&fileStat.st_mtime,sizeof(time_t));

                    if(strstr(fileName[i],"login") != NULL)//if the file is login.ini,just record the date 
                    {
                        continue;
                    }

                    if(flagIsSave[i] == 0)
                    {
                        continue;
                    }
                    //get time
                    memset(timeBuf,0,sizeof(timeBuf));
                    if(GetLastRecord(fileType[i],itemType[i],db, timeBuf) == 0)//
                    {
                       // printf("db time ==> %s   strlen  %d\n",timeBuf,strlen(timeBuf));

                        if(sscanf(timeBuf,"%d.%d.%d-%d:%d:%d",
            		        &tempTime.tm_year,&tempTime.tm_mon,&tempTime.tm_mday,
            		        &tempTime.tm_hour,&tempTime.tm_min,&tempTime.tm_sec) < 0)
                        {
                            memset(&tempTime,0,sizeof(tempTime));
                        }

                    }

                    //read file
                    SendFileContentToDB(fileType[i], itemType[i],fileName[i],tempTime,db);
                    
                    continue;
                }

            
                if(fileStat.st_mtime != fileTime[i])//date is changed , then do service
                {
                    log_debug("%s  changed %lu %lu flagIsSave  %d\n",fileName[i],fileStat.st_mtime,fileTime[i],flagIsSave[i]);
                    
                    memset(&fileTime[i],0,sizeof(time_t));
                    memcpy(&fileTime[i],&fileStat.st_mtime,sizeof(time_t));//the  last time of modification    

                    if(strstr(fileName[i],"login") != NULL)//
                    {
                        //do service
                        ReLoadLoginCfg();
                        
                        flagIsSave[0] = gFaultCfg.nIsControlRecord;
                        flagIsSave[1] = gFaultCfg.nIsLogRecord;
                        flagIsSave[2] = gFaultCfg.nIsLogRecord;
                        
                        continue;
                    }

                    if(flagIsSave[i] == 0)
                    {
                        continue;
                    }
                    //get time
                    memset(timeBuf,0,sizeof(timeBuf));
                    if(GetLastRecord(fileType[i],itemType[i],db, timeBuf) == 0)//
                    {
                       //printf("db time  %s \n",timeBuf);
                       if(sscanf(timeBuf,"%d.%d.%d-%d:%d:%d",
            		        &tempTime.tm_year,&tempTime.tm_mon,&tempTime.tm_mday,
            		        &tempTime.tm_hour,&tempTime.tm_min,&tempTime.tm_sec) < 0)
                       {
                            memset(&tempTime,0,sizeof(tempTime));
                       }
                    }
                    SendFileContentToDB(fileType[i], itemType[i],fileName[i],tempTime,db);
                }
            }
        }
        sleep(1);

    }

    sqlite3_close(db);

    return NULL;
}


