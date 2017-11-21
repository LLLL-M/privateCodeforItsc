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
#include <time.h>
#include <unistd.h>
#include "HikConfig.h"
#include "hik.h"
#include "configureManagement.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"                   //文本文件的默认路径



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
STRUCT_BINFILE_CONFIG g_ConfigPara;
STRUCT_BINFILE_CUSTOM g_CustomPara;
STRUCT_BINFILE_DESC g_DescPara;
CountDownCfg        g_CountDownCfg;              //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
STRUCT_BINFILE_MISC g_MiscCfg;

//该全局指针数组存放了当前这个转换工具已经支持的配置文件。
static char *gArraySupportCfg[] = {"hikconfig","config","desc","custom","countdown","misc","vehicle","FailureLog"};


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : IsCfgSupport
 功能描述  : 判断配置文件是否已经被当前版本支持，如果支持该配置文件的转换，
             则返回该配置在gArraySupport中的序号，否则返回0
 输入参数  : char *fileName  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static INT8 IsCfgSupport(char *fileName)
{
    int i = 0;
    int len = sizeof(gArraySupportCfg)/sizeof(char *);
    
    for(i = 0; i < len; i++)
    {
        if(strstr(fileName,gArraySupportCfg[i]) != NULL)
        {
            return i+1;
        }
    }
    return 0;
}

static void PrintSupportCfg()
{
    int i = 0;
    int len = sizeof(gArraySupportCfg)/sizeof(char *);

    printf("Current Support Cfgs :\n");
    for(i = 0; i < len; i++)
    {
        printf("%s.dat\t",gArraySupportCfg[i]);
        if((i+1)%5 == 0)
            printf("\n");
    }
    printf("\n");
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
    printf("\nBinaryTextConvert Usage : ./BinaryTextConvert [fileName] \n");
    printf("\nIf [fileName] is end with .ini , then we will convert ini to binary .\n");
    printf("If [fileName] is end with .dat , then we will convert binary to ini .\n");
    printf("EXAMPLES: ./BinaryTextConvert /home/hikconfig.dat\n\n");
    PrintSupportCfg();
}

/*****************************************************************************
 函 数 名  : InitGlobalVal
 功能描述  : 初始化全局参数
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static void InitGlobalVal()
{
    memset(&g_SignalPara,0,sizeof(g_SignalPara));
    memset(&g_ConfigPara,0,sizeof(g_ConfigPara));
    memset(&g_CustomPara,0,sizeof(g_CustomPara));
    memset(&g_CountDownCfg,0,sizeof(g_CountDownCfg));
    memset(&g_DescPara,0,sizeof(g_DescPara));
}

/*****************************************************************************
 函 数 名  : IsArguLegal
 功能描述  : 校验命令行参数是否合法
 输入参数  : INT8 *p_fileName  
             INT argc          
             UINT8 *cSwitch    
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static UINT8 IsArguLegal(char *p_fileName,INT8 argc,UINT8 *cSwitch)
{
    if(argc == 2)
    {
        if(strstr(p_fileName,".ini") != NULL)
        {
            *cSwitch = 1;
        }
        else if(strstr(p_fileName,".dat") != NULL)
        {   
            *cSwitch = 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return IsCfgSupport(p_fileName);
}

UINT8 ConvertFailureLogToText()
{
	FILE *fpBin = NULL;
	FILE *fpText = NULL;
	int nTotalBlock = 0;
	int nCount = 0;
	time_t start;
	int nCostSec;
	struct tm *pTime;
    struct FAILURE_INFO info; 
	
	fpBin = fopen("/home/FailureLog.dat","r");//
	if(NULL == fpBin)
	{
		printf("open /home/FailureLog.dat failed : %s .\n",strerror(errno));
		return 0;
	}

	fpText = fopen("./FailureLog.ini","w");//
	if(NULL == fpText)
	{
		printf("open ./FailureLog.ini failed : %s .\n",strerror(errno));
		return 0;
	}

    fseek(fpBin,0,SEEK_END);
    nTotalBlock = ftell(fpBin);//总共大小
    fseek(fpBin,0,SEEK_SET);
    
    printf("total blocks : %d  Bytes. \n",nTotalBlock);
    start = time(NULL);
    while(1)
    {
        fread(&info,sizeof(info),1,fpBin);

        pTime = localtime((time_t *)&info.nTime);
        fprintf(fpText,"%d-%d-%d %d:%d:%d 消息类型: 0x%x ,消息值: %d , 序列号: %d .\n"
                                            ,1900+pTime->tm_year,1+pTime->tm_mon,pTime->tm_mday,pTime->tm_hour,pTime->tm_min,pTime->tm_sec
                                            ,info.nID
                                            ,info.nValue
                                            ,info.nNumber);
                                            
        nCount = ftell(fpBin);;
        nCostSec = (nTotalBlock - nCount)*(time(NULL)- start)/nCount;
        if(nTotalBlock > 0)
        {
            printf("Converting  [%0.2f%%], %d blocks have been converted, %2d:%2d:%2d  to complete .\r"
                                            ,100.0*(nCount)/nTotalBlock
                                            ,nCount
                                            ,nCostSec/3600,(nCostSec%3600)/60,((nCostSec%3600)%60));///

            fflush(stdout);
        }

        if(nCount == nTotalBlock)
        {
            break;
        }
    }

    fflush(fpBin);
	fclose(fpBin);

    fflush(fpText);
	fclose(fpText);
	
	return 1; 
}


//把流量数据转换成文本格式
UINT8 ConvertVehicleDataToText()
{
    int i = 0;
	FILE *fpBin = NULL;
	FILE *fpText = NULL;
	int nTotalBlock = 0;
	int nCount = 0;
	int nIndex = 0;
	time_t start;
	int nCostSec;
	struct tm *pTime;
    TimeAndHistoryVolume vol;
	
	fpBin = fopen("/home/vehicle.dat","r");//
	if(NULL == fpBin)
	{
		printf("open /home/vehicle.dat failed : %s .\n",strerror(errno));
		return 0;
	}

	fpText = fopen("./vehicle.ini","w");//
	if(NULL == fpText)
	{
		printf("open ./vehicle.ini failed : %s .\n",strerror(errno));
		return 0;
	}

    fseek(fpBin,0,SEEK_END);
    nTotalBlock = ftell(fpBin);//总共大小
    fseek(fpBin,0,SEEK_SET);
    
    fread(&nIndex,sizeof(UINT64),1,fpBin);
    printf("total blocks : %d  Bytes, index : %x \n",nTotalBlock,nIndex);
    nCount += 4;
    start = time(NULL);

    fprintf(fpText,"时间\t检测器\t总数(辆)\t空间占有率\t平均车速(km/h)\t排队长度(m)\t车流密度(辆/km)\t车头间距(m)\t车头时距(s)\t绿损(s)\n");
    
    while(1)
    {
        fread(&vol,sizeof(vol),1,fpBin);

        pTime = localtime((time_t *)&vol.dwTime);
        for(i = 0; i < 48; i++)
        {
#if 1        
            fprintf(fpText,"%d-%d-%d %d:%d:%d\t%d\t%d\t%0.2f%%\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%d\n"
                                                ,1900+pTime->tm_year,1+pTime->tm_mon,pTime->tm_mday,pTime->tm_hour,pTime->tm_min,pTime->tm_sec
                                                ,i+1
                                                ,vol.struVolume[i].byDetectorVolume
                                                ,vol.struVolume[i].byDetectorOccupancy/100.0
                                                ,vol.struVolume[i].byVehicleSpeed/100.0
                                                ,vol.struVolume[i].wQueueLengh/100.0
                                                ,vol.struVolume[i].wVehicleDensity/100.0
                                                ,vol.struVolume[i].wVehicleHeadDistance/100.0
                                                ,vol.struVolume[i].wVehicleHeadTimeDistance/100.0
                                                ,vol.struVolume[i].wGreenLost/100);
#else         
            fprintf(fpText,"%s TotalVolume: %d , Occupancy: %0.2f%%, Speed: %0.2f km/h, QueueLengh: %0.2f m, Density: %0.2f veh/km, HeadDistance: %0.2f m, HeadTimeDistance: %0.2f s, GreenLost: %d s\n"
                                                ,ctime((time_t *)&vol.dwTime)
                                                ,vol.struVolume[i].byDetectorVolume
                                                ,vol.struVolume[i].byDetectorOccupancy/100.0
                                                ,vol.struVolume[i].byVehicleSpeed/100.0
                                                ,vol.struVolume[i].wQueueLengh/100.0
                                                ,vol.struVolume[i].wVehicleDensity/100.0
                                                ,vol.struVolume[i].wVehicleHeadDistance/100.0
                                                ,vol.struVolume[i].wVehicleHeadTimeDistance/100.0
                                                ,vol.struVolume[i].wGreenLost);
#endif

        }
        nCount = ftell(fpBin);;
        nCostSec = (nTotalBlock - nCount)*(time(NULL)- start)/nCount;
        if(nTotalBlock > 0)
        {
            printf("Converting  [%0.2f%%], %d blocks have been converted, %2d:%2d:%2d  to complete .\r"
                                            ,100.0*(nCount)/nTotalBlock
                                            ,nCount
                                            ,nCostSec/3600,(nCostSec%3600)/60,((nCostSec%3600)%60));///

            fflush(stdout);
        }

        if(nCount >= nTotalBlock)
        {
            break;
        }
    }

    fflush(fpBin);
	fclose(fpBin);

    fflush(fpText);
	fclose(fpText);
	
	return 1; 
}



/*****************************************************************************
 函 数 名  : DoConvertFromBin2Text
 功能描述  : 把二进制信息转换成文本文件
 输入参数  : char *pFileName   
             UINT8 cFileIndex  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static UINT8 DoConvertFromBin2Text(char *pFileName,UINT8 cFileIndex)
{
    char *pNewFileName = NULL;
    
    switch(cFileIndex)
    {
        case 1:
        {
            if(0 != ReadBinCfgInfo(pFileName,&g_SignalPara, sizeof(g_SignalPara),0,0))
            {
                remove(CFG_BAK_NAME);
            }
            IsSignalControlparaLegal(&g_SignalPara);//校验，只是为了了解错误的大致位置。并不做进一步计算。

            pNewFileName = "./hikconfig.ini";
            unlink(pNewFileName);
            WriteConfigFile(&g_SignalPara,pNewFileName);
            break;
        }
        case 2:
        {
            ReadBinCfgInfo(pFileName,&g_ConfigPara, sizeof(g_ConfigPara),0,0);
            pNewFileName = "./config.ini";
            unlink(pNewFileName);
            set_special_params(pNewFileName,&g_ConfigPara);       
            break;
        }
        case 3:
        {
            ReadBinCfgInfo(pFileName,&g_DescPara , sizeof(g_DescPara),0,0);
            pNewFileName = "./desc.ini";
            unlink(pNewFileName);
            set_desc_params(pNewFileName,&g_DescPara);   
            break;
        }
        case 4:
        {
            ReadBinCfgInfo(pFileName,&g_CustomPara, sizeof(g_CustomPara),0,0);
            pNewFileName = "./custom.ini";
            unlink(pNewFileName);
            set_custom_params(pNewFileName,&g_CustomPara);   
            break;
        }
        case 5:
        {
            ReadBinCfgInfo(pFileName,&g_CountDownCfg, sizeof(g_CountDownCfg),0,0);
            pNewFileName = "./countdown.ini";
            unlink(pNewFileName);
            WriteCountdownCfgToIni(pNewFileName,&g_CountDownCfg); 
            break;
        }
        case 6:
        {
            ReadBinCfgInfo(pFileName,&g_MiscCfg, sizeof(g_MiscCfg),0,0);
            pNewFileName = "./misc.ini";
            unlink(pNewFileName);
            WriteMiscCfgToIni(pNewFileName,&g_MiscCfg); 
            break;
        }
        case 7:
        {
            pNewFileName = "./vehicle.ini";
            ConvertVehicleDataToText();
            break;
        }
        case 8:
        {
            pNewFileName = "./FailureLog.ini";
            ConvertFailureLogToText();
            break;
        }
        default:
        {
            pNewFileName = "ERROR";
            break;
        }
    }
    printf("\nConvert Ok , new file : %s .\n",pNewFileName);
    return 1;
}

/*****************************************************************************
 函 数 名  : DoConvertFromText2Bin
 功能描述  : 把文本信息转换成二进制
 输入参数  : char *pFileName   
             UINT8 cFileIndex  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年8月5日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static UINT8 DoConvertFromText2Bin(char *pFileName,UINT8 cFileIndex)
{
    char *pNewFileName = NULL;
    int ret = 0;
    
    switch(cFileIndex)
    {
        case 1:
        {
            if(LoadDataFromCfg(&g_SignalPara, pFileName)== FALSE)
            {
                printf("[%s] failed to  read file %s .\n",__func__,pFileName);
                return FALSE;
            }
            IsSignalControlparaLegal(&g_SignalPara);
            pNewFileName = "/home/hikconfig.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_SignalPara,sizeof(g_SignalPara));            
            break;
        }
        case 2:
        {
            get_special_params(pFileName,&g_ConfigPara);
            pNewFileName = "/home/config.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_ConfigPara, sizeof(g_ConfigPara));            
            break;
        }
        case 3:
        {
            get_desc_params(pFileName,&g_DescPara); 
            pNewFileName = "/home/desc.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_DescPara, sizeof(g_DescPara));               
            break;
        }
        case 4:
        {
            get_custom_params(pFileName,&g_CustomPara);
            pNewFileName = "/home/custom.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_CustomPara, sizeof(g_CustomPara));                      
            break;
        }
        case 5:
        {
            ReadCountdowncfgFromIni(pFileName,&g_CountDownCfg);
            pNewFileName = "/home/countdown.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_CountDownCfg, sizeof(g_CountDownCfg));               
            break;
        }
        case 6:
        {
            ReadMiscCfgFromIni(pFileName,&g_MiscCfg);
            pNewFileName = "/home/misc.dat";
            ret = WriteBinCfgInfo(pNewFileName,&g_MiscCfg, sizeof(g_MiscCfg));               
            break;
        }
        case 7:
        {
            ret = 0;
            break;
        }
        case 8:
        {
            ret = 0;
            break;
        }
        default:
        {
            pNewFileName = "ERROR";
            break;
        }
    }
    printf("[WriteBinaryFile] write file %s %s .\n",pNewFileName,(ret == 0 ? "failed" : "success"));
    return ret;
}


/*****************************************************************************
 函 数 名  : DoConvert
 功能描述  : 转换功能函数
 输入参数  : char *pFileName        
             unsigned char cSwitch      0表示从二进制转换成文本文件，1表示cognitive文本文件转换成二进制
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月20日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
static Boolean DoConvert(char *pFileName,UINT8 cSwitch,UINT8 cFileIndex)
{
    if((cSwitch == 1) && (IS_FILE_EXIST(pFileName) == 0))
    {
        printf("[DoConvert] %s is not exist . \n",pFileName);
        return FALSE;
    }

    if(cSwitch == 0)//读取二进制文件
    {
        return DoConvertFromBin2Text(pFileName,cFileIndex);
    }
    else if(cSwitch == 1)//读文本文件
    {
        return DoConvertFromText2Bin(pFileName,cFileIndex);
    }

    return FALSE;
}


/*****************************************************************************
 函 数 名  : main
 功能描述  : hikconfig 二进制转换为文本文件
 输入参数  : int argc     
             char **argv  
             新版本转换工具，只需要提供一个参数，比如下面这个:
             ./BinaryTextConvert /home/config.dat
             表明是把/home/config.dat这个二进制文件转换成./config.ini这个文本文件
             ./BinaryTextConvert ./config.ini
             表明是把./config.ini这个文本文件转换成二进制文件/home/config.dat。
             新版本转换工具不再提供转换方向，如果输入参数是以ini结尾，就默认是将文本文件转换成二进制文件并保存到/home下，如果输入参数
             是以dat结尾，就默认是将二进制文件转换成文本文件，并保存到当前文件夹下
             如果是需要将二进制文件转换成文本文件，但二进制文件不存在，则将各项值为0的文本文件转换出来。
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年3月19日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
int main(int argc,char **argv)
{
    char *p_fileName = NULL;//存放的是文件路径,默认情况下是CFG_TEXT_PATH或CFG_NAME
    UINT8 cSwitch = 0;//0表示将二进制转换成文本文件，1表示将文本文件转换成二进制文件，默认情况下是0.
    UINT8 cFileIndex = 0;

    cFileIndex = IsArguLegal(argv[1],argc,&cSwitch);
    if(cFileIndex == 0)
    {
        PrintUsage();
        return 0;
    }
    p_fileName = argv[1];
    
    printf("convert from %s, source fileName %s\n",(cSwitch == 0) ? "binary to ini" : "ini to binary",((p_fileName == NULL) ? (p_fileName = (((cSwitch == 0) ? CFG_NAME : CFG_TEXT_PATH))) : p_fileName));

    InitGlobalVal();

    if(DoConvert(p_fileName,cSwitch,cFileIndex) == FALSE)
    {
        printf("convert failed ,please check it again . \n");
        PrintUsage();
    }

    return 0;
}

