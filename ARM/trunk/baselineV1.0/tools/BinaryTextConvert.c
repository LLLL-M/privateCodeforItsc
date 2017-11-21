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
#include <time.h>
#include <unistd.h>
#include "HikConfig.h"
#include "hik.h"
#include "configureManagement.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define CFG_TEXT_PATH       "./hikconfig.ini"                   //�ı��ļ���Ĭ��·��



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
STRUCT_BINFILE_CONFIG g_ConfigPara;
STRUCT_BINFILE_CUSTOM g_CustomPara;
STRUCT_BINFILE_DESC g_DescPara;
CountDownCfg        g_CountDownCfg;              //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
STRUCT_BINFILE_MISC g_MiscCfg;

//��ȫ��ָ���������˵�ǰ���ת�������Ѿ�֧�ֵ������ļ���
static char *gArraySupportCfg[] = {"hikconfig","config","desc","custom","countdown","misc","vehicle","FailureLog"};


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : IsCfgSupport
 ��������  : �ж������ļ��Ƿ��Ѿ�����ǰ�汾֧�֣����֧�ָ������ļ���ת����
             �򷵻ظ�������gArraySupport�е���ţ����򷵻�0
 �������  : char *fileName  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
    printf("\nBinaryTextConvert Usage : ./BinaryTextConvert [fileName] \n");
    printf("\nIf [fileName] is end with .ini , then we will convert ini to binary .\n");
    printf("If [fileName] is end with .dat , then we will convert binary to ini .\n");
    printf("EXAMPLES: ./BinaryTextConvert /home/hikconfig.dat\n\n");
    PrintSupportCfg();
}

/*****************************************************************************
 �� �� ��  : InitGlobalVal
 ��������  : ��ʼ��ȫ�ֲ���
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : IsArguLegal
 ��������  : У�������в����Ƿ�Ϸ�
 �������  : INT8 *p_fileName  
             INT argc          
             UINT8 *cSwitch    
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
    nTotalBlock = ftell(fpBin);//�ܹ���С
    fseek(fpBin,0,SEEK_SET);
    
    printf("total blocks : %d  Bytes. \n",nTotalBlock);
    start = time(NULL);
    while(1)
    {
        fread(&info,sizeof(info),1,fpBin);

        pTime = localtime((time_t *)&info.nTime);
        fprintf(fpText,"%d-%d-%d %d:%d:%d ��Ϣ����: 0x%x ,��Ϣֵ: %d , ���к�: %d .\n"
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


//����������ת�����ı���ʽ
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
    nTotalBlock = ftell(fpBin);//�ܹ���С
    fseek(fpBin,0,SEEK_SET);
    
    fread(&nIndex,sizeof(UINT64),1,fpBin);
    printf("total blocks : %d  Bytes, index : %x \n",nTotalBlock,nIndex);
    nCount += 4;
    start = time(NULL);

    fprintf(fpText,"ʱ��\t�����\t����(��)\t�ռ�ռ����\tƽ������(km/h)\t�Ŷӳ���(m)\t�����ܶ�(��/km)\t��ͷ���(m)\t��ͷʱ��(s)\t����(s)\n");
    
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
 �� �� ��  : DoConvertFromBin2Text
 ��������  : �Ѷ�������Ϣת�����ı��ļ�
 �������  : char *pFileName   
             UINT8 cFileIndex  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
            IsSignalControlparaLegal(&g_SignalPara);//У�飬ֻ��Ϊ���˽����Ĵ���λ�á���������һ�����㡣

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
 �� �� ��  : DoConvertFromText2Bin
 ��������  : ���ı���Ϣת���ɶ�����
 �������  : char *pFileName   
             UINT8 cFileIndex  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��8��5��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : DoConvert
 ��������  : ת�����ܺ���
 �������  : char *pFileName        
             unsigned char cSwitch      0��ʾ�Ӷ�����ת�����ı��ļ���1��ʾcognitive�ı��ļ�ת���ɶ�����
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��20��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static Boolean DoConvert(char *pFileName,UINT8 cSwitch,UINT8 cFileIndex)
{
    if((cSwitch == 1) && (IS_FILE_EXIST(pFileName) == 0))
    {
        printf("[DoConvert] %s is not exist . \n",pFileName);
        return FALSE;
    }

    if(cSwitch == 0)//��ȡ�������ļ�
    {
        return DoConvertFromBin2Text(pFileName,cFileIndex);
    }
    else if(cSwitch == 1)//���ı��ļ�
    {
        return DoConvertFromText2Bin(pFileName,cFileIndex);
    }

    return FALSE;
}


/*****************************************************************************
 �� �� ��  : main
 ��������  : hikconfig ������ת��Ϊ�ı��ļ�
 �������  : int argc     
             char **argv  
             �°汾ת�����ߣ�ֻ��Ҫ�ṩһ�������������������:
             ./BinaryTextConvert /home/config.dat
             �����ǰ�/home/config.dat����������ļ�ת����./config.ini����ı��ļ�
             ./BinaryTextConvert ./config.ini
             �����ǰ�./config.ini����ı��ļ�ת���ɶ������ļ�/home/config.dat��
             �°汾ת�����߲����ṩת��������������������ini��β����Ĭ���ǽ��ı��ļ�ת���ɶ������ļ������浽/home�£�����������
             ����dat��β����Ĭ���ǽ��������ļ�ת�����ı��ļ��������浽��ǰ�ļ�����
             �������Ҫ���������ļ�ת�����ı��ļ������������ļ������ڣ��򽫸���ֵΪ0���ı��ļ�ת��������
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��19��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
int main(int argc,char **argv)
{
    char *p_fileName = NULL;//��ŵ����ļ�·��,Ĭ���������CFG_TEXT_PATH��CFG_NAME
    UINT8 cSwitch = 0;//0��ʾ��������ת�����ı��ļ���1��ʾ���ı��ļ�ת���ɶ������ļ���Ĭ���������0.
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

