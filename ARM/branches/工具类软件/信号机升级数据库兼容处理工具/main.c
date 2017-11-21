#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
//#include "sqlite3ext.h"

/* 整体思路:
 * 需要提供一个升级程序对应的数据库表结构的路径，hikconfig.dbtables，或者 extconfig.dbtables；
 * 如果升级程序的表结构没有变化，则可以不传入dbtables文件，或者传入的dbtables文件不存在；
 * 程序判断传入的dbtables文件存在后，会在/opt/目录下创建对应的db文件，通过dbtables文件中的每
 * 一行表项解析出表名和各个字段，通过表名在/home/目录下的db文件中获取到旧有的各个字段，然后
 * 找到新、旧字段中共有的字段，把这些共有字段的值从旧的/home/的db中读取出来，插入到新的/opt/
 * 的db中；最后把/home/下的db备份为/opt/的bak，删掉/home/下的db和bak，把/opt/下的db移动到/home/下
 */

#define HIKCONFIG_UPDATE_TABLES "hikconfig.dbtables"
#define EXTCONFIG_UPDATE_TABLES "extconfig.dbtables"

#define SCHEMA_NAME "old"

int sqlite3_db_init(const char *db_new,sqlite3 **ppdb,const char *db_old)
{
//    char cmd[32] = {0};
    char cmd[64] = {0};
    char *errmsg = NULL;
    
    if(0 == access(db_new,F_OK))
    {
        printf("%s: %s is exist, remove it.\n",__func__,db_new);
        sprintf(cmd,"rm %s -f",db_new);
        system(cmd);
    }
    if(0 != access(db_old,F_OK))
    {
        printf("%s: %s is not exist.\n",__func__,db_old);
        return -1;
    }
    //打开新数据库    
    if(SQLITE_OK != sqlite3_open(db_new,ppdb))
    {
        printf("%s: sqlite3_open: %s\n",__func__,sqlite3_errmsg(*ppdb));
        sqlite3_close(*ppdb);
        return -1;
    }

    //attach 旧的数据库
    memset(cmd,0,sizeof(cmd));
    sprintf(cmd,"attach '%s' as '%s';",db_old,SCHEMA_NAME);
//    printf("%s: cmd = %s\n",__func__,cmd);
    if(SQLITE_OK != sqlite3_exec(*ppdb,cmd,NULL,NULL,&errmsg))
    {
        printf("%s: sqlite3_exec: %s\n",__func__,errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(*ppdb);
        return -1;
    }

    return 0;
}

static char *read_line(FILE *fp,char *buf,int buf_len)
{
    int ch = 0;

    //过滤掉空白字符；
    while(isspace(ch = fgetc(fp)));
    if(EOF == ch)
        return NULL;
    //过滤掉注释
    while('#' == ch)
    {
        fgets(buf,buf_len - 1,fp);
        while(isspace(ch = fgetc(fp)));
    }
    if(EOF == ch)
        return NULL;
    //读取一行表项
    ungetc(ch,fp);
    fgets(buf,buf_len - 1,fp);

    return buf;
}

static int dbtable_parse(char *table,char *table_name,char *table_columns)
{
    //解析表名，写入table_name中；
    while(*table != ' ' && *table != '(')
    {
        *table_name = *table;
        table_name++;
        table++;
    }
    *table_name = '\0';

    //解析第一个字段，写入table_columns中；
    if(NULL == (table = strchr(table,'(')))
        return -1;
    table++;
    while(*table == ' ') table++;
    while(*table != ' ')
    {
        *table_columns = *table;
        table_columns++;
        table++;
    }
    *table_columns = ',';
    table_columns++;

    //解析其他字段，写入table_columns中；
    while(NULL != (table = strchr(table,',')))
    {
        table++;
        while(*table == ' ') table++;
        while(*table != ' ')
        {
            *table_columns = *table;
            table_columns++;
            table++;
        }
        *table_columns = ',';
        table_columns++;
    }
    table_columns--;
    *table_columns = '\0';
    
    return 0;
}

//把old_columns的各个字段逐个在new_columns中查找，找到的话就写入buf中；
//最后把buf的内容复制到new_columns中；
static int find_common_columns(char *new_columns,char *old_columns)
{
    char buf[512] = {0};
    int offset = 0;
    char *token = NULL;

    token = strtok(old_columns,",");
    if(NULL != strcasestr(new_columns,token))
    {
        strcpy(buf + offset,token);
        offset = strlen(buf);
        buf[offset] = ',';
        offset++;
    }

    while(NULL != (token = strtok(NULL,",")))
    {
        if(NULL != strcasestr(new_columns,token))
        {
            strcpy(buf + offset,token);
            offset = strlen(buf);
            buf[offset] = ',';
            offset++;
        }
    }
    if(offset == 0)
        return -1;

    buf[offset - 1] = '\0';
    strcpy(new_columns,buf);
    return 0;
}

int sqlite3_db_update(FILE *fp,sqlite3 *pdb)
{
    char buf[512] = {0};
    char sql[512] = {0};
    char table_name[64] = {0};
    char table_columns[512] = {0};
    char *errmsg = NULL;
    sqlite3_stmt *stmt = NULL;
    char *value_text = NULL;
    int offset = 0;

    //读取一行表项，在新数据库中创建该表，解析出该表项的表名和各个字段；
    //通过表名在旧数据库中获取该表名的各个字段，如果没有该表名，则读取下一行表项；
    //找到该表名在新、旧数据库中的共同字段；
    //从旧数据库中读取该表所有共同字段的内容插入到新数据库的该表中；
    //循环上述过程，处理每一行表项；
    while(NULL != read_line(fp,buf,sizeof(buf)))
    {
        //在新数据库中创建表；
        memset(sql,0,sizeof(sql));
        sprintf(sql,"create table main.%s",buf);
//        printf("sql = %s\n",sql);
        if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
        {
            printf("%s: sqlite_exec: %s\n",__func__,errmsg);
            sqlite3_free(errmsg);
            sqlite3_close(pdb);
            return -1;
        }

        //解析表名和各个字段
//        printf("%s: read table = %s\n",__func__,buf);
        memset(table_name,0,sizeof(table_name));
        memset(table_columns,0,sizeof(table_columns));
        if(-1 == dbtable_parse(buf,table_name,table_columns))
        {
            printf("%s: wrong table format : %s.",__func__,table_name);
            memset(buf,0,sizeof(buf));
            continue;
        }
//        printf("%s: table_name = %s\n",__func__,table_name);
//        printf("%s: table_columns = %s\n",__func__,table_columns);

        //通过表名读取旧数据库中该表的各个字段，写入buf中；
        memset(sql,0,sizeof(sql));
        sprintf(sql,"pragma %s.table_info(%s);",SCHEMA_NAME,table_name);
        if(SQLITE_OK != sqlite3_prepare_v2(pdb,sql,-1,&stmt,NULL))
        {
            printf("%s: sqlite3_prepare_v2 error: %s\n",__func__,sqlite3_errmsg(pdb));
            sqlite3_free(errmsg);
            sqlite3_close(pdb);
            return -1;
        }
        
        memset(buf,0,sizeof(buf));
        offset = 0;
        while(SQLITE_ROW == sqlite3_step(stmt))
        {
            value_text = (char *)sqlite3_column_text(stmt,1);
//            printf("value_text = %s\n",value_text);
            sprintf(buf + offset,"%s,",value_text);
            offset = strlen(buf);
        }
        //如果在旧数据库中没有该表，则读取下一个表项；
        if(0 == offset)
        {
            printf("%s: old database don't have the %s table\n",__func__,table_name);
            continue;
        }
        buf[offset - 1] = '\0';
//        printf("%s: old %s tabale_columns = %s\n",__func__,table_name,buf);
        
        if(SQLITE_OK != sqlite3_finalize(stmt))
        {
            printf("%s: sqlite3_finalize error: %s\n",__func__,sqlite3_errmsg(pdb));
            sqlite3_close(pdb);
            return -1;
        }

        //找到共同字段，把旧数据库的内容插入到新数据库中；
        if(0 == find_common_columns(table_columns,buf))
        {
            memset(sql,0,sizeof(sql));
            sprintf(sql,"insert into main.%s(%s) select %s from %s.%s;",table_name,table_columns,table_columns,SCHEMA_NAME,table_name);
//            printf("%s: sql = %s\n",__func__,sql);
            if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
            {
                printf("%s: sqlite_exec: %s\n",__func__,errmsg);
                sqlite3_free(errmsg);
                sqlite3_close(pdb);
                return -1;
            }
        }
        
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    FILE *fp = NULL;
    char file_name[32] = {0};
    char new_db_path[32] = {0};
    char old_db_path[32] = {0};
    char cmd[128] = {0};
    char *p = NULL;
    sqlite3 *pdb = NULL;

    if(argc != 2)
    {
        printf("Usage: %s path/filename.dbtables\n",argv[0]);
        printf("Notes: filename should be \"hikconfig\" or \"extconfig\"\n");
        return -1;
    }
    
    p = strrchr(argv[1],'/');
    if(NULL == p)
        strncpy(file_name,argv[1],sizeof(file_name));
    else
        strncpy(file_name,p + 1,sizeof(file_name));

    if(0 != strncmp(file_name,HIKCONFIG_UPDATE_TABLES,strlen(file_name)) && 0 != strncmp(file_name,EXTCONFIG_UPDATE_TABLES,strlen(file_name)))
    {
        printf("%s don't supported , only \"hikconfig.dbtables\" and \"extconfig.dbtables\" supported.\n",file_name);
        return -1;
    }

    p = strchr(file_name,'.');
    *p = '\0';
    //新的数据库文件默认放在 /opt/ 目录下；
    sprintf(new_db_path,"/opt/%s.db",file_name);
    sprintf(old_db_path,"/home/%s.db",file_name);
    if(0 == access(argv[1],R_OK))
    {
        if(NULL == (fp = fopen(argv[1],"r")))
        {
            perror("fopen");
            return -1;
        }
        
        if(-1 == sqlite3_db_init(new_db_path,&pdb,old_db_path))
        {
            printf("%s: sqlite3_db_init %s failed.\n",__func__,new_db_path);
            fclose(fp);
            return -1;
        }
//        p = "attach '/home/hikconfig.db' as 'old';";
//        printf("p = %s\n",p);
//        if(SQLITE_OK != sqlite3_exec(pdb,p,NULL,NULL,NULL))
//        {
//             printf("%s: sqlite3_exec error\n",__func__);
//             sqlite3_close(pdb);
//             return -1;
//        }
        printf("ready to update the sqlite db.\n");
        
        if(-1 == sqlite3_db_update(fp,pdb))
        {
            printf("%s: sqlite3_db_update failed.\n",__func__);
            fclose(fp);
            return -1;
        }

        sqlite3_close(pdb);
        fclose(fp);

        //把旧的数据库文件备份为 /opt/ 目录下的bak文件
        sprintf(cmd,"rm %s.bak -f;mv %s %s.bak",old_db_path,old_db_path,new_db_path);
//        printf("%s\n",cmd);
        system(cmd);
        memset(cmd,0,sizeof(cmd));
        //把更新过的数据库文件放到 /home/ 目录下；
        sprintf(cmd,"cp %s %s;mv %s %s.bak",new_db_path,old_db_path,new_db_path,old_db_path);
//        printf("%s\n",cmd);
        system(cmd);
    }
    //dbtables 文件如果不存在，则不需要更新相应的数据库；
    else
    {
        printf("No need to updates the %s\n",old_db_path);
    }

    return 0;
}


