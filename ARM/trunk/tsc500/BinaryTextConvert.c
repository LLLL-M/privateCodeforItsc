/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : BinaryTextConvert.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��3��20��
  ����޸�   :
  ��������   : ��Դ�ļ���Ҫ��ʵ�ֶ������ļ����ı��ļ�֮���ת���������鿴ϵ
               ͳ����������Ϣ�����ϵͳ������Ϣ
  �����б�   :
              DoConvert
              main
              PrintUsage
              ReadBinaryFile
              WriteBinaryFile
  �޸���ʷ   :
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "HikConfig.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"           //�ı��ļ���Ĭ��·��
#define CFG_BINARY_PAHT     "/home/hikconfig.dat"          //�������ļ���Ĭ��·��
#define IS_FILE_EXIST(fileName)     ((access((fileName),F_OK) == 0) ? 1 : 0)   

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
SignalControllerPara g_SignalPara;
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/




/*****************************************************************************
 �� �� ��  : ReadBinaryFile
 ��������  : ���������ļ����ڴ���
 �������  : char *pFileName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static Boolean ReadBinaryFile(char *pFileName)
{
    FILE *fp = NULL;
    int len = sizeof(SignalControllerPara);

    fp = fopen(pFileName,"rb");
    if(fp == NULL)
    {
       // printf("===>  %d, %d\n",errno,ENOENT);
        return FALSE;
    }

    if(fread(&g_SignalPara,len,1,fp) != 1)
    {
        return FALSE;
    }

    fclose(fp);
    
    return TRUE;
}

/*****************************************************************************
 �� �� ��  : WriteBinaryFile
 ��������  : ���ڴ�����д�뵽�������ļ���
 �������  : char *pFileName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static Boolean WriteBinaryFile(char *pFileName)
{
    FILE *fp = NULL;
    int len = sizeof(SignalControllerPara);

    fp = fopen(pFileName,"w+");
    if(fp == NULL)
    {
        printf("%s failed to open file %s , %s  \n",__func__,pFileName,strerror(errno));
        return FALSE;
    }

    if(fwrite(&g_SignalPara,len,1,fp) != 1)
    {
        printf("%s failed to write file %s , %s  \n",__func__,pFileName,strerror(errno));
        return FALSE;
    }

    fclose(fp);
    
    return TRUE;
}

/*****************************************************************************
 �� �� ��  : DoConvert
 ��������  : ת�����ܺ���
 �������  : char *pFileName        
             unsigned char cSwitch  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static Boolean DoConvert(char *pFileName,unsigned char cSwitch)
{
    char *pNewFileName = NULL;

    if((pFileName == NULL) || ((cSwitch != 0)&&(cSwitch != 1)))
    {
        printf("[DoConvert] should be 0 or 1 .\n");
        return FALSE;
    }

    if(IS_FILE_EXIST(pFileName) == 0)
    {
        printf("[DoConvert] %s is not exist . \n",pFileName);
        return FALSE;
    }

    if(cSwitch == 0)//��ȡ�������ļ�
    {
        if(ReadBinaryFile(pFileName) == FALSE)
        {
            printf("[%s] failed to  read file %s .\n",__func__,pFileName);
            return FALSE;
        }

        pNewFileName = "./hikconfig.ini";

        IsSignalControlparaLegal(&g_SignalPara);//У�飬ֻ��Ϊ���˽����Ĵ���λ�á���������һ�����㡣
        //��ȫ�ֱ������浽�ı��ļ�
        if(WriteConfigFile(&g_SignalPara,pNewFileName) == FALSE)
        {
            printf("[%s] failed to  write file %s .\n",__func__,pNewFileName);
            return FALSE;
        }
    }
    else if(cSwitch == 1)//���ı��ļ�
    {
        if(LoadDataFromCfg(&g_SignalPara, pFileName)== FALSE)
        {
            printf("[%s] failed to  read file %s .\n",__func__,pFileName);
            return FALSE;
        }

        if(IsSignalControlparaLegal(&g_SignalPara) != 0)
        {
            printf("[DoConvert] verify error . \n");
            return FALSE;
        }
        
        pNewFileName = CFG_BINARY_PAHT;

        //��ȫ�ֱ������浽�������ļ�
        if(WriteBinaryFile(pNewFileName) == FALSE)
        {
            printf("[%s] failed to  write file %s .\n",__func__,pNewFileName);
            return FALSE;
        }
    }

    printf("Convert Ok , new file : %s .\n",pNewFileName);
    return TRUE;
}

/*****************************************************************************
 �� �� ��  : PrintUsage
 ��������  : ���û���������ERRORʱ���ø�ʹ�÷�����
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void PrintUsage()
{
    printf("\nBinaryTextConvert Usage : ./BinaryTextConvert [convert] [fileName] \n");
    printf("[convert]  , should be 0 or 1, 0 means convert from binary to ini , 1 means convert from ini to binary , default 0.\n");
    printf("[fileName] , the file to be converted , default is %s or %s , which depends on [convert].\n",CFG_BINARY_PAHT,CFG_TEXT_PATH);
    printf("EXAMPLES: ./BinaryTextConvert 1 ./hikconfig.ini\n\n");
}



/*****************************************************************************
 �� �� ��  : main
 ��������  : hikconfig ������ת��Ϊ�ı��ļ�
 �������  : int argc     
             char **argv  
             ��һ��������0��1,0��ʾ��������ת�����ı��ļ���1��ʾ���ı��ļ�ת���ɶ������ļ���Ĭ���������0.
             �ڶ����������ı��ļ���������ļ���·����Ĭ���������CFG_TEXT_PATH��CFG_BINARY_PAHT
             �����������Ƕ��źŻ����ò����Ƿ����У�飬0��У�飬1У��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��19��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int main(int argc,char **argv)
{
    char *p_fileName = NULL;//��ŵ����ļ�·��,Ĭ���������CFG_TEXT_PATH��CFG_BINARY_PAHT
    unsigned char cSwitch = 0;//0��ʾ��������ת�����ı��ļ���1��ʾ���ı��ļ�ת���ɶ������ļ���Ĭ���������0.

    if(argc == 1)
    {
        
    }
    else if(argc == 2)
    {
        if((strlen(argv[1]) != 1) || ((argv[1][0] != '0') && (argv[1][0] != '1') ))
        {
            PrintUsage();
            return 0;
        }
        cSwitch = atoi(argv[1]);
    }
    else if(argc == 3)
    {
        cSwitch = atoi(argv[1]);
        p_fileName = argv[2];
    }
    else
    {
        PrintUsage();
        return 0;
    }
    
    printf("convert from %s, source fileName %s\n",(cSwitch == 0) ? "binary to ini" : "ini to binary",((p_fileName == NULL) ? (p_fileName = (((cSwitch == 0) ? CFG_BINARY_PAHT : CFG_TEXT_PATH))) : p_fileName));

    memset(&g_SignalPara,0,sizeof(SignalControllerPara));

    if(DoConvert(p_fileName,cSwitch) == FALSE)
    {
        printf("convert failed ,please check it again . \n");
        PrintUsage();
    }

    return 0;
}

