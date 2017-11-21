#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
//#include "sqlite3ext.h"

/* ����˼·:
 * ��Ҫ�ṩһ�����������Ӧ�����ݿ��ṹ��·����hikconfig.dbtables������ extconfig.dbtables��
 * �����������ı�ṹû�б仯������Բ�����dbtables�ļ������ߴ����dbtables�ļ������ڣ�
 * �����жϴ����dbtables�ļ����ں󣬻���/opt/Ŀ¼�´�����Ӧ��db�ļ���ͨ��dbtables�ļ��е�ÿ
 * һ�б�������������͸����ֶΣ�ͨ��������/home/Ŀ¼�µ�db�ļ��л�ȡ�����еĸ����ֶΣ�Ȼ��
 * �ҵ��¡����ֶ��й��е��ֶΣ�����Щ�����ֶε�ֵ�Ӿɵ�/home/��db�ж�ȡ���������뵽�µ�/opt/
 * ��db�У�����/home/�µ�db����Ϊ/opt/��bak��ɾ��/home/�µ�db��bak����/opt/�µ�db�ƶ���/home/��
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
    //�������ݿ�    
    if(SQLITE_OK != sqlite3_open(db_new,ppdb))
    {
        printf("%s: sqlite3_open: %s\n",__func__,sqlite3_errmsg(*ppdb));
        sqlite3_close(*ppdb);
        return -1;
    }

    //attach �ɵ����ݿ�
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

    //���˵��հ��ַ���
    while(isspace(ch = fgetc(fp)));
    if(EOF == ch)
        return NULL;
    //���˵�ע��
    while('#' == ch)
    {
        fgets(buf,buf_len - 1,fp);
        while(isspace(ch = fgetc(fp)));
    }
    if(EOF == ch)
        return NULL;
    //��ȡһ�б���
    ungetc(ch,fp);
    fgets(buf,buf_len - 1,fp);

    return buf;
}

static int dbtable_parse(char *table,char *table_name,char *table_columns)
{
    //����������д��table_name�У�
    while(*table != ' ' && *table != '(')
    {
        *table_name = *table;
        table_name++;
        table++;
    }
    *table_name = '\0';

    //������һ���ֶΣ�д��table_columns�У�
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

    //���������ֶΣ�д��table_columns�У�
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

//��old_columns�ĸ����ֶ������new_columns�в��ң��ҵ��Ļ���д��buf�У�
//����buf�����ݸ��Ƶ�new_columns�У�
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

    //��ȡһ�б���������ݿ��д����ñ��������ñ���ı����͸����ֶΣ�
    //ͨ�������ھ����ݿ��л�ȡ�ñ����ĸ����ֶΣ����û�иñ��������ȡ��һ�б��
    //�ҵ��ñ������¡������ݿ��еĹ�ͬ�ֶΣ�
    //�Ӿ����ݿ��ж�ȡ�ñ����й�ͬ�ֶε����ݲ��뵽�����ݿ�ĸñ��У�
    //ѭ���������̣�����ÿһ�б��
    while(NULL != read_line(fp,buf,sizeof(buf)))
    {
        //�������ݿ��д�����
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

        //���������͸����ֶ�
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

        //ͨ��������ȡ�����ݿ��иñ�ĸ����ֶΣ�д��buf�У�
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
        //����ھ����ݿ���û�иñ����ȡ��һ�����
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

        //�ҵ���ͬ�ֶΣ��Ѿ����ݿ�����ݲ��뵽�����ݿ��У�
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
    //�µ����ݿ��ļ�Ĭ�Ϸ��� /opt/ Ŀ¼�£�
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

        //�Ѿɵ����ݿ��ļ�����Ϊ /opt/ Ŀ¼�µ�bak�ļ�
        sprintf(cmd,"rm %s.bak -f;mv %s %s.bak",old_db_path,old_db_path,new_db_path);
//        printf("%s\n",cmd);
        system(cmd);
        memset(cmd,0,sizeof(cmd));
        //�Ѹ��¹������ݿ��ļ��ŵ� /home/ Ŀ¼�£�
        sprintf(cmd,"cp %s %s;mv %s %s.bak",new_db_path,old_db_path,new_db_path,old_db_path);
//        printf("%s\n",cmd);
        system(cmd);
    }
    //dbtables �ļ���������ڣ�����Ҫ������Ӧ�����ݿ⣻
    else
    {
        printf("No need to updates the %s\n",old_db_path);
    }

    return 0;
}


