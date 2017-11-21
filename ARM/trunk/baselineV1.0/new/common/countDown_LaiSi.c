/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : countDown_LaiSi.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��4��2��
  ����޸�   :
  ��������   : ��Դ�ļ�ʵ������˹Э�鵹��ʱ�����ṩ�˶�ȡ���޸ġ�������˹��
               ���ļ��Ľӿڣ�ͬʱ�ṩ������˹Э��Ľӿڡ�
  �����б�   :
              ConvertData
              countDownByLaiSiProtocol
              GetNextControllerId
              GetRuningPhaseId
              PrintData
              ReadLaiSiCfgFromIni
              SetCountDownValueLaiSi
              SetCountValue
              UpdateLaiSiCfg
              WriteLaiSiCfgToIni
  �޸���ʷ   :
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"
#include "countDown.h"
#include "common.h"
#include "configureManagement.h"
#include "its.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_NUM_COUNTDOWN_LAISI   16
#define MAX_NUM_PHASE       16
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
//��˹Э���Ӧ��ת��
static unsigned char  gLaiSiConvertArray[16] = {0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70};

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
extern CountDownCfg        g_CountDownCfg;                         //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ

static unsigned char g_SendBufLaiSi[48] = {0};

/*****************************************************************************
 �� �� ��  : PrintData
 ��������  : �����ã���ӡ���ͻ��������ݡ�
 �������  : unsigned char *nArray  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
/*static void PrintData(unsigned char *nArray)
{
    int i = 0;
	
	if(nArray == NULL)
	{
		return;
	}
    for(i = 1; i <= 48; i++)
    {
        fprintf(stderr,"%2x ",nArray[i - 1]);
        if((i%12 == 0) && (i != 0))
        {
            fprintf(stderr,"\n");
        }
    }
    
    fprintf(stderr,"\n\n\n");
}*/


/*****************************************************************************
 �� �� ��  : ConvertData
 ��������  : ���ݵ���ʱʱ�䣬��ʮ���Ƶ���ʱʱ�䣬������˹Э��ת���ɷ���Ҫ��
             ��ʮλ������λ����
 �������  : unsigned char iTempPassTime[2]     �ֱ���ת���ĸ�λ��ʮλ
             unsigned char cPhaseCountDownTime  ת��ǰ�ĵ���ʱʱ�䡣
             unsigned char cPhaseColor          ��ǰ��ɫ
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��3��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void ConvertData(unsigned char iTempPassTime[2],unsigned char cPhaseCountDownTime,unsigned char cPhaseColor)
{
    iTempPassTime[0] = cPhaseCountDownTime%10;//��λ
    iTempPassTime[1] = cPhaseCountDownTime/10;//ʮλ

    //ת��
    if((iTempPassTime[0] >= 0) && (iTempPassTime[0] <= 15) && (iTempPassTime[1] >= 0) && (iTempPassTime[1] <= 15)) 
    {
        iTempPassTime[0] = gLaiSiConvertArray[iTempPassTime[0]];
        iTempPassTime[1] = gLaiSiConvertArray[iTempPassTime[1]];
    }

    if((GREEN == cPhaseColor) || (GREEN_BLINK == cPhaseColor))//1 0
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x01; 
    }
    else if(RED == cPhaseColor)//0 1
    {
        iTempPassTime[0] |= 0x01;
        iTempPassTime[1] |= 0x00;
    }
    else if((YELLOW == cPhaseColor) || (YELLOW_BLINK == cPhaseColor))//0 0 
    {
        iTempPassTime[0] |= 0x00; 
        iTempPassTime[1] |= 0x00; 
    }
}

/*****************************************************************************
 �� �� ��  : SetCountValue
 ��������  : ��������豸ID�ĵ���ʱ����ɫ��Ϣ��
 �������  : unsigned char iTempPassTime[2]  
             unsigned char cDeviceId         
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetCountValue(unsigned char iTempPassTime[2],unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;

    GetRuningPhaseId(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);

    ConvertData(iTempPassTime,cPhaseCountDownTime,cPhaseColor);
	
	if(gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime)//���ĳ��λ�ĵ���ʱʱ��ȸ�Ӧ���ʱ�������ʾʱ�����㣬������ʾ�������Ҫʵʱ��ʾ����ʱ״̬����ô����ͨ�����ù��ߣ�����Ԫ�����еĸ�Ӧ���ʱ�����Щ��
	{
		iTempPassTime[0] = 0;
		iTempPassTime[1] = 0;
	}
}


/*****************************************************************************
 �� �� ��  : countDownByLaiSiProtocol
 ��������  : ��˹Э�����ʵ��
 �� �� ֵ  :                      
 �޸���ʷ  
  1.��    ��   : 2015��3��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���
����˼·:
1. ����˵��Ҫ���� a��ĺ��ʱ�䣬a��ʮλ������B,��λ������C
2. ��B��C����Լ����ת��������ת��:��0x02, 0x9e, 0x24, 0x0c,  0x98, 0x48, 0x40, 0x1e, 0x00, 0x08, 0x10, 0x0c0, 0x62, 0x84, 0x60, 0x70���ֱ��Ӧ����0 ~ 15 ���߶��롣
3. ����˵a=24s�Ļ�����ôB����2��C����4�����Ӧ��ת��ֱ���0x24 0x98,��ɫ��01
4. ʮλ�͸�λ�ֱ����ɫ����λ�����������0x24 | 0x00 , 0x98 | 0x01���������0x24 0x99
5. ��ô��������ʱ�Ƶ����ݾ���0x24 0x99
6. 0x50 0x51 0x52 0x53��ʵ�ֱ��Ӧ��IDΪ0 1 2 3�ĵ���ʱ�豸,��Щ�豸��û�ж�������֮�֣���Ҫ����ʵ���������
7. 0x60 0x70 0x80�������16λ����Ҫ��0x50�������������ͬ��ͬ��0x61 ... ֻ������������ʱ����ΪУ����ȷ��
8. ÿ���ӵ���һ�μ��ɡ�
*****************************************************************************/
void countDownByLaiSiProtocol()
{
    int i =0;
    unsigned char iTempPassTime[2] = {0};//[0]��λ��[1]ʮλ
    unsigned char tempBuf[48] ={0x50,0xff,0xff,0x51,0xff,0xff,0x52,0xff,0xff,0x53,0xff,0xff,
                                0x60,0xff,0xff,0x61,0xff,0xff,0x62,0xff,0xff,0x63,0xff,0xff,
                                0x70,0xff,0xff,0x71,0xff,0xff,0x72,0xff,0xff,0x73,0xff,0xff,
                                0x80,0xff,0xff,0x81,0xff,0xff,0x82,0xff,0xff,0x83,0xff,0xff};

    //�������ڳ��ҵĽ��ܣ�0x60 0x70 0x80��0x50��������ͬ����������
    for(i = 0; i < MAX_NUM_COUNTDOWN_LAISI; i++)
    {
        if(g_CountDownCfg.cControllerID[i][0] == 0)//���ĳ������ʱû�б����ã���ֱ�ӷ��ء�
        {
            continue;
        }
    
        SetCountValue(iTempPassTime,g_CountDownCfg.cDeviceId[i]);
        
		if((iTempPassTime[0] == 0) && (iTempPassTime[1] == 0))//��λ����Ϊ0����������λ����ʱ�ȸ�Ӧ���ʱ��󣬲�����ʾ��
		{
			continue;
		}
        tempBuf[3*i+1] = iTempPassTime[1];
        tempBuf[3*i+2] = iTempPassTime[0];//0x50

    }
   // PrintData(tempBuf);
    memcpy(g_SendBufLaiSi,tempBuf,sizeof(g_SendBufLaiSi));//ʹ����˹Э��Ļ�����Ҫͨ��485����48�ֽڵ�����

    Send485Data(g_SendBufLaiSi, sizeof(g_SendBufLaiSi));//��������
}


