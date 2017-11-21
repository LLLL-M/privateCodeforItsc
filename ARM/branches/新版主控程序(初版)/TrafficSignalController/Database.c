/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : Database.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : ���/home/����־�ļ��ı仯��һ���仯��������ӵ���־��Ϣ��
               �浽���ݿ��С�ǰ������Ҫ�����ݿ��ļ���
  �����б�   :
              CreateTableByType
              GetLastRecord
              InsertTableItemByType
              JudgeStructTmTime
              ReLoadLoginCfg
              SendFileContentToDB
              ShowTableContentByType
              ThreadCheckCfgChanged
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "Database.h"
#include "common.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern FAULT_CFG gFaultCfg;

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void get_current_params();
extern void get_special_params();
extern void get_fault_cfg(char *cfgName);
extern void GPSInit();
extern void HardwareWatchdogInit();
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : ReLoadLoginCfg
 ��������  : ���¼��������ļ����ڴ棬����ȫ�ֱ�����
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : CreateTableByType
 ��������  : ���ݱ�����ͣ�������
 �������  : TABLE_TYPE type  
             sqlite3 *db      
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���1
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : InsertTableItemByType
 ��������  : ���ݱ�����ͼ������������ͣ��������ݵ����ݿ�
 �������  : TABLE_TYPE type_table      
             TABLE_ITEM_TYPE type_item  
             char *dateTime             
             char *Msg                  
             sqlite3 *db                
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���1
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : ShowTableContentByType
 ��������  : �����ã���ӡ����������
 �������  : TABLE_TYPE type  
             sqlite3 *db      
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : GetLastRecord
 ��������  : ��ȡָ���������һ�����ݣ�Ҳ����ID�����Ǹ��������Ƚ�Ҫ��Ҫ
             �ѵ�ǰ���ݲ��뵽���ݿ���
 �������  : TABLE_TYPE type            
             TABLE_ITEM_TYPE type_item  
             sqlite3 *db                
             char *timeBuf              
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���1
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : JudgeStructTmTime
 ��������  : �Ƚ�����struct tm�͵Ľṹ��
 �������  : struct tm tm_1  
             struct tm tm_2  
 �� �� ֵ  : ��ȷ���0��ǰ�ߴ󷵻�1�����򷵻�-1
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : SendFileContentToDB
 ��������  : �жϵ�ǰ����Ҫд�뵽�����к�ִ�иú������Խ�����д�����ݿ�
 �������  : TABLE_TYPE type_table      
             TABLE_ITEM_TYPE type_item  
             char *fileName             
             struct tm  time            
             sqlite3 *db                
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���1
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : ThreadCheckCfgChanged
 ��������  : ����߳���������������ļ�������־�ļ��Ƿ�ı�ģ�ÿ1���Ӽ��һ
             �Σ�������� �仯������������ļ������ͣ���������д�����ݿ⻹��
             ����ȫ��
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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


