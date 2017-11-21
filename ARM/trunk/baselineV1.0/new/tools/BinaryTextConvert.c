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
#include "BinaryTextConvert.h"
#include "gb.h"
#include "ykconfig.h"

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
GbConfig            g_gbCfg;                    //guobiao
YK_Config			g_ykCfg;					//�����ſ�

//��ȫ��ָ���������˵�ǰ���ת�������Ѿ�֧�ֵ������ļ���
char *gArraySupportCfg[] = {"hikconfig","gbconfig","ykconfig",
							"desc",
							"custom","countdown","misc",
							"vehicle","FailureLog","config"};



/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


//Ϊ�˼���һά��ṹ�Ͷ�ά��ṹ���źŻ�����������������ʱ����Ҫ�������ݿ���
void MakeCompatible(PSignalControllerPara para)
{
    //����ϵ���λ��Ϊ�յĻ����Ͱ��µ����򸲸ǵ��ϵ����򣬷����෴����
    if(para->stOldPhase[0].nPhaseID == 0){
        memcpy(para->stOldPhase,para->stPhase[0],sizeof(para->stOldPhase));
        memcpy(para->stOldChannel,para->stChannel[0],sizeof(para->stOldChannel));
        memcpy(para->stOldFollowPhase,para->stFollowPhase[0],sizeof(para->stOldFollowPhase));
        memcpy(para->OldAscSignalTransTable,para->AscSignalTransTable[0],sizeof(para->OldAscSignalTransTable));
    }else{
        memcpy(para->stPhase[0],para->stOldPhase,sizeof(para->stOldPhase));
        memcpy(para->stChannel[0],para->stOldChannel,sizeof(para->stOldChannel));
        memcpy(para->stFollowPhase[0],para->stOldFollowPhase,sizeof(para->stOldFollowPhase));
        memcpy(para->AscSignalTransTable[0],para->OldAscSignalTransTable,sizeof(para->OldAscSignalTransTable));
    }
}


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
INT8 IsCfgSupport(char *fileName)
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

INT8 GetCfgSupportCount(int index)
{
    return sizeof(gArraySupportCfg)/sizeof(char *);
}

INT32 GetCfgSize(int fileIndex)
{
  switch(fileIndex)
  {
        case 1:
        {
            return sizeof(g_SignalPara);
        }
        case 10:
        {
            return sizeof(g_ConfigPara);
        }
		case 3:
		{
			return sizeof(g_ykCfg);
		}
        case 4:
        {
            return sizeof(g_DescPara);
        }
        case 5:
        {
            return sizeof(g_CustomPara);
        }
        case 6:
        {
            return sizeof(g_CountDownCfg);
        }
        case 2:
        {
            return sizeof(g_gbCfg);
        }
        case 8:
        {
            return 0;
        }
        case 9:
        {
            return 0;
        }
        case 7:
        {
            return sizeof(g_MiscCfg);
        }
        default:
        {
            return 0;
        }
    }

}

void PrintSupportCfg()
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
void InitGlobalVal()
{
    memset(&g_SignalPara,0,sizeof(g_SignalPara));
    memset(&g_ConfigPara,0,sizeof(g_ConfigPara));
    memset(&g_CustomPara,0,sizeof(g_CustomPara));
    memset(&g_CountDownCfg,0,sizeof(g_CountDownCfg));
    memset(&g_DescPara,0,sizeof(g_DescPara));
	memset(&g_ykCfg,0,sizeof(g_ykCfg));
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
UINT8 IsArguLegal(char *p_fileName,INT8 argc,UINT8 *cSwitch)
{
	if(p_fileName == NULL || cSwitch == NULL)
	{
		return 0;
	}

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
        nCount = ftell(fpBin);
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
char * DoConvertFromBin2Text(char *pFileName,UINT8 cFileIndex,void *dataContent)
{
    char *pNewFileName = NULL;
    
    switch(cFileIndex)
    {
        case 1:
        {
            if(dataContent == NULL)
            {
                if(0 != ReadBinCfgInfo(pFileName,&g_SignalPara, sizeof(g_SignalPara),0,0))
                {
                    remove(CFG_BAK_NAME);
                }
            }else{
                memcpy(&g_SignalPara,dataContent,sizeof(g_SignalPara));
            }
            MakeCompatible(&g_SignalPara);
            IsSignalControlparaLegal(&g_SignalPara);//У�飬ֻ��Ϊ���˽����Ĵ���λ�á���������һ�����㡣

            pNewFileName = "hikconfig.ini";
            unlink(pNewFileName);
            WriteConfigFile(&g_SignalPara,pNewFileName);
            break;
        }
        case 3:
        {
            if(dataContent == NULL)
            {
                if(0 != ReadBinCfgInfo(pFileName,&g_ykCfg, sizeof(g_ykCfg),0,0))
                {
                   // remove(CFG_BAK_NAME);
                }
            }else{
                memcpy(&g_ykCfg,dataContent,sizeof(g_SignalPara));
            }

            pNewFileName = "ykconfig.ini";
            unlink(pNewFileName);
            WriteykConfigFile(&g_ykCfg,pNewFileName);
            break;
        }		
        case 10:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_ConfigPara, sizeof(g_ConfigPara),0,0);
            }else{
                memcpy(&g_ConfigPara,dataContent,sizeof(g_ConfigPara));
            }
            
            pNewFileName = "config.ini";
            unlink(pNewFileName);
            set_special_params(pNewFileName,&g_ConfigPara);       
            break;
        }
        case 4:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_DescPara , sizeof(g_DescPara),0,0);
            }else{
                memcpy(&g_DescPara,dataContent,sizeof(g_DescPara));
            }
            
            pNewFileName = "desc.ini";
            unlink(pNewFileName);
            set_desc_params(pNewFileName,&g_DescPara);   
            break;
        }
        case 5:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_CustomPara, sizeof(g_CustomPara),0,0);
            }else{
                memcpy(&g_CustomPara,dataContent,sizeof(g_CustomPara));
            }
            
            pNewFileName = "custom.ini";
            unlink(pNewFileName);
            set_custom_params(pNewFileName,&g_CustomPara);   
            break;
        }
        case 6:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_CountDownCfg, sizeof(g_CountDownCfg),0,0);
            }else{
                memcpy(&g_CountDownCfg,dataContent,sizeof(g_CountDownCfg));
            }
            
            pNewFileName = "countdown.ini";
            unlink(pNewFileName);
            WriteCountdownCfgToIni(pNewFileName,&g_CountDownCfg); 
            break;
        }
        case 2:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_gbCfg, sizeof(g_gbCfg),0,0);
            }else {
                memcpy(&g_gbCfg,dataContent,sizeof(g_gbCfg));
            }
            
            pNewFileName = "gbconfig.ini";
            unlink(pNewFileName);
            WriteGbConfigFile(&g_gbCfg,pNewFileName); 
            break;
        }
        case 8:
        {
            pNewFileName = "vehicle.ini";
            ConvertVehicleDataToText();
            break;
        }
        case 9:
        {
            pNewFileName = "FailureLog.ini";
            ConvertFailureLogToText();
            break;
        }
        case 7:
        {
            if(dataContent == NULL)
            {
                ReadBinCfgInfo(pFileName,&g_MiscCfg, sizeof(g_MiscCfg),0,0);
            }else {
                memcpy(&g_MiscCfg,dataContent,sizeof(g_MiscCfg));
            }
            
            pNewFileName = "misc.ini";
            unlink(pNewFileName);
            WriteMiscCfgToIni(pNewFileName,&g_MiscCfg); 
            break;
        }
        default:
        {
            pNewFileName = NULL;
            break;
        }
    }
    printf("\nConvert Ok , new file : %s .\n",pNewFileName);
    return pNewFileName;
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
void *DoConvertFromText2Bin(char *pFileName,UINT8 cFileIndex,void *pNewName)
{
    char *pNewFileName = NULL;
    int ret = 0;
    unsigned char *retPointer = NULL;
    
    switch(cFileIndex)
    {
        case 1:
        {
            if(LoadDataFromCfg(&g_SignalPara, pFileName)== FALSE)
            {
                printf("[%s] failed to  read file %s .\n",__func__,pFileName);
                //return FALSE;
            }
            MakeCompatible(&g_SignalPara);
            IsSignalControlparaLegal(&g_SignalPara);
            pNewFileName = (pNewName == NULL ? "/home/hikconfig.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_SignalPara,sizeof(g_SignalPara));     
            retPointer = (void *)&g_SignalPara;
            break ;
        }
        case 3:
        {
            if(LoadykDataFromCfg(&g_ykCfg, pFileName)== FALSE)
            {
                printf("[%s] failed to  read file %s .\n",__func__,pFileName);
                //return FALSE;
            }
            pNewFileName = (pNewName == NULL ? "/home/ykconfig.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_ykCfg,sizeof(g_ykCfg));     
            retPointer = (void *)&g_ykCfg;
            break ;
        }		
        case 10:
        {
            get_special_params(pFileName,&g_ConfigPara);
            pNewFileName = (pNewName == NULL ? "/home/config.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_ConfigPara, sizeof(g_ConfigPara));  
            retPointer = (void *)&g_ConfigPara;
            break;
        }
        case 4:
        {
            get_desc_params(pFileName,&g_DescPara); 
            pNewFileName = (pNewName == NULL ? "/home/desc.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_DescPara, sizeof(g_DescPara));  
            retPointer = (void *)&g_DescPara;
            break;
        }
        case 5:
        {
            get_custom_params(pFileName,&g_CustomPara);
            pNewFileName = (pNewName == NULL ? "/home/custom.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_CustomPara, sizeof(g_CustomPara));  
            retPointer = (void *)&g_CustomPara;
            break;
        }
        case 6:
        {
            ReadCountdowncfgFromIni(pFileName,&g_CountDownCfg);
            pNewFileName = (pNewName == NULL ? "/home/countdown.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_CountDownCfg, sizeof(g_CountDownCfg));     
            retPointer = (void *)&g_CountDownCfg;
            break;
        }
        case 7:
        {
            ReadMiscCfgFromIni(pFileName,&g_MiscCfg);
            pNewFileName = (pNewName == NULL ? "/home/misc.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_MiscCfg, sizeof(g_MiscCfg));   
            retPointer = (void *)&g_MiscCfg;
            break;
        }
        case 8:
        {
            ret = 0;
            break;
        }
        case 9:
        {
            ret = 0;
            break;
        }
        case 2:
        {
            LoadGbDataFromCfg(&g_gbCfg,pFileName);
            pNewFileName = (pNewName == NULL ? "/home/gbconfig.dat" : pNewName);
            ret = WriteBinCfgInfo(pNewFileName,&g_gbCfg, sizeof(g_gbCfg));   
            retPointer = (void *)&g_gbCfg;
            break;
        }
        default:
        {
            pNewFileName = "ERROR";
            break;
        }
    }
    printf("[WriteBinaryFile] write file %s %s .\n",pNewFileName,(ret == 0 ? "failed" : "success"));
    return (void *)retPointer;
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
Boolean DoConvert(char *pFileName,UINT8 cSwitch,UINT8 cFileIndex)
{
    if((cSwitch == 1) && (IS_FILE_EXIST(pFileName) == 0))
    {
        printf("[DoConvert] %s is not exist . \n",pFileName);
        return FALSE;
    }

    if(cSwitch == 0)//��ȡ�������ļ�
    {
        return DoConvertFromBin2Text(pFileName,cFileIndex,NULL) == NULL ? FALSE : TRUE;
    }
    else if(cSwitch == 1)//���ı��ļ�
    {
        return DoConvertFromText2Bin(pFileName,cFileIndex,NULL) == NULL ? FALSE : TRUE;
    }

    return FALSE;
}

#ifndef __MINGW32__

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
    
    printf("convert from %s, source fileName %s fileIndex %d\n",
            (cSwitch == 0) ? "binary to ini" : "ini to binary",
            ((p_fileName == NULL) ? (p_fileName = (((cSwitch == 0) ? CFG_NAME : CFG_TEXT_PATH))) : p_fileName),
            cFileIndex);

    InitGlobalVal();

    if(DoConvert(p_fileName,cSwitch,cFileIndex) == FALSE)
    {
        printf("convert failed ,please check it again . \n");
        PrintUsage();
    }

    return 0;
}

#endif
