/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : BinaryTextConvert.c
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年3月20日
  最近修改   :
  功能描述   : 该源文件主要是实现二进制文件与文本文件之间的转换，用来查看系
               统运行配置信息或更改系统配置信息
  函数列表   :
              DoConvert
              main
              PrintUsage
              ReadBinaryFile
              WriteBinaryFile
  修改历史   :
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "HikConfig.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"           //文本文件的默认路径
#define CFG_BINARY_PAHT     "/home/hikconfig.dat"          //二进制文件的默认路径
#define IS_FILE_EXIST(fileName)     ((access((fileName),F_OK) == 0) ? 1 : 0)   

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
SignalControllerPara g_SignalPara;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/




/*****************************************************************************
 函 数 名  : ReadBinaryFile
 功能描述  : 读二进制文件到内存中
 输入参数  : char *pFileName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : WriteBinaryFile
 功能描述  : 将内存数据写入到二进制文件中
 输入参数  : char *pFileName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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
 函 数 名  : DoConvert
 功能描述  : 转换功能函数
 输入参数  : char *pFileName        
             unsigned char cSwitch  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数

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

    if(cSwitch == 0)//读取二进制文件
    {
        if(ReadBinaryFile(pFileName) == FALSE)
        {
            printf("[%s] failed to  read file %s .\n",__func__,pFileName);
            return FALSE;
        }

        pNewFileName = "./hikconfig.ini";

        IsSignalControlparaLegal(&g_SignalPara);//校验，只是为了了解错误的大致位置。并不做进一步计算。
        //将全局变量保存到文本文件
        if(WriteConfigFile(&g_SignalPara,pNewFileName) == FALSE)
        {
            printf("[%s] failed to  write file %s .\n",__func__,pNewFileName);
            return FALSE;
        }
    }
    else if(cSwitch == 1)//读文本文件
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

        //将全局变量保存到二进制文件
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
 函 数 名  : PrintUsage
 功能描述  : 在用户操作出现ERROR时调用该使用方法。
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void PrintUsage()
{
    printf("\nBinaryTextConvert Usage : ./BinaryTextConvert [convert] [fileName] \n");
    printf("[convert]  , should be 0 or 1, 0 means convert from binary to ini , 1 means convert from ini to binary , default 0.\n");
    printf("[fileName] , the file to be converted , default is %s or %s , which depends on [convert].\n",CFG_BINARY_PAHT,CFG_TEXT_PATH);
    printf("EXAMPLES: ./BinaryTextConvert 1 ./hikconfig.ini\n\n");
}



/*****************************************************************************
 函 数 名  : main
 功能描述  : hikconfig 二进制转换为文本文件
 输入参数  : int argc     
             char **argv  
             第一个参数是0或1,0表示将二进制转换成文本文件，1表示将文本文件转换成二进制文件，默认情况下是0.
             第二个参数是文本文件或二进制文件的路径，默认情况下是CFG_TEXT_PATH或CFG_BINARY_PAHT
             第三个参数是对信号机配置参数是否进行校验，0不校验，1校验
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月19日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int main(int argc,char **argv)
{
    char *p_fileName = NULL;//存放的是文件路径,默认情况下是CFG_TEXT_PATH或CFG_BINARY_PAHT
    unsigned char cSwitch = 0;//0表示将二进制转换成文本文件，1表示将文本文件转换成二进制文件，默认情况下是0.

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

