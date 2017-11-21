/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, hikvision

 ******************************************************************************
  �� �� ��   : countdown_nation_2004.c
  �� �� ��   : ����
  ��    ��   : jgp
  ��������   : 2016��7��14��
  ����޸�   :
  ��������   : ����ʱ������2004Э��
  �����б�   :
              base_dump_data
              countDownByNationStandard2004
              SetCountValueNationStandard2004
  �޸���ʷ   :
  1.��    ��   : 2016��7��14��
    ��    ��   : jgp
    �޸�����   : �����ļ�

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HikConfig.h"
#include "platform.h"
//#include "parse_ini.h"
#include "countDown.h"
#include "common.h"
#include "configureManagement.h"
#include "its.h"
#include "countdown_nation_2004.h"

extern CountDownCfg        g_CountDownCfg;                         //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ

//#define DUMP_DATA

#ifdef DUMP_DATA
static void BaseDumpData(unsigned char *buf, int len, int base)
{
    int i;

    for (i = 0; i < len; i++)
    {
        fprintf(stderr,"%02X ",buf[i]);
        if ((i % base) == (base - 1))
            fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
}
#endif

static void SetCountValueNationStandard2004(CountDownFrame2004 *pframe, unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cTemp = 0;
    
    SetCountdownValue(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);    

    if (gStructBinfileCustom.sCountdownParams.iPulseGreenTime != 0)
    {
    	//������̵ƻ������������ҵ���ʱʱ��������õ��̵Ƶ���ʱʱ�䣬
    	//���ߵ���ʱʱ����ڸ�Ӧ���ʱ�䣬�رյ���ʱ
    	if ((cPhaseColor == GREEN || cPhaseColor == GREEN_BLINK) && (cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseGreenTime \
            || gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime))
    	{
    		cPhaseColor = TURN_OFF;
    		cPhaseCountDownTime = 0;
    	}
    }
    if (gStructBinfileCustom.sCountdownParams.iPulseRedTime != 0)
    {
        //����Ǻ�ƣ����ҵ���ʱʱ��������õĺ�Ƶ���ʱʱ�䣬�رյ���ʱ
        if (cPhaseColor == RED && cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseRedTime)
    	{
    		cPhaseColor = TURN_OFF;
    		cPhaseCountDownTime = 0;
    	}
    }

    switch (cPhaseColor)
    {
        case GREEN:
        case GREEN_BLINK:    
        {
            cTemp = 1;           //����2004 �� 00<->0 ��ʾ���� ��01<->1 ��ʾ��ɫ��10<->2 ��ʾ��ɫ��11<->3 ��ʾ��ɫ
//            cTemp |= (cDeviceId<<2);
            break;
        }
        case RED:           
        {
            cTemp = 3;
//            cTemp |= (cDeviceId<<2);
            break;
        }
        case YELLOW:    
        case YELLOW_BLINK:    
        {
            cTemp = 2;
//            cTemp |= (cDeviceId<<2);
            break;
        }        
        case TURN_OFF:          
        {
//            cTemp |= (cDeviceId<<2);
            break;
        }  
        default:    
        { 
//            cTemp |= (cDeviceId<<2); 
            break;
        }
    }

    cTemp |= (cDeviceId<<2);

    pframe->color_addr = cTemp;
    pframe->data_high = HEX2BCD(cPhaseCountDownTime / 100);
    pframe->data_low = HEX2BCD(cPhaseCountDownTime % 100);

}

void countDownByNationStandard2004()
{
    int i = 0,j = 0;
    CountDownFrame2004 frame_buf[MAX_DIRECTION];

    memset(frame_buf,0,sizeof(frame_buf));

    for (i = 0; i < MAX_DIRECTION; i++)
    {
        if (g_CountDownCfg.cControllerType[i] != 0)//�õ���ʱ������
        {
            frame_buf[j].frame_head = FRAME_HEAD_2004;
            SetCountValueNationStandard2004(frame_buf + j,g_CountDownCfg.cDeviceId[i]);
            frame_buf[j].check_sum = 0x7F & ((frame_buf[j].color_addr) ^ (frame_buf[j].data_high) ^ (frame_buf[j].data_low));
            j++;
        }
    }
    
    Send485Data((unsigned char *)frame_buf,sizeof(CountDownFrame2004) * j);//��������

#ifdef DUMP_DATA
    BaseDumpData((unsigned char *)frame_buf,sizeof(CountDownFrame2004) * j,5);
#endif
}

