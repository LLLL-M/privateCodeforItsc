/******************************************************************************

                  版权所有 (C), 2001-2020, hikvision

 ******************************************************************************
  文 件 名   : countdown_nation_2004.c
  版 本 号   : 初稿
  作    者   : jgp
  生成日期   : 2016年7月14日
  最近修改   :
  功能描述   : 倒计时器国标2004协议
  函数列表   :
              base_dump_data
              countDownByNationStandard2004
              SetCountValueNationStandard2004
  修改历史   :
  1.日    期   : 2016年7月14日
    作    者   : jgp
    修改内容   : 创建文件

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

extern CountDownCfg        g_CountDownCfg;                         //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识

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
    	//如果是绿灯或者绿闪，并且倒计时时间大于配置的绿灯倒计时时间，
    	//或者倒计时时间大于感应检测时间，关闭倒计时
    	if ((cPhaseColor == GREEN || cPhaseColor == GREEN_BLINK) && (cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseGreenTime \
            || gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime))
    	{
    		cPhaseColor = TURN_OFF;
    		cPhaseCountDownTime = 0;
    	}
    }
    if (gStructBinfileCustom.sCountdownParams.iPulseRedTime != 0)
    {
        //如果是红灯，并且倒计时时间大于配置的红灯倒计时时间，关闭倒计时
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
            cTemp = 1;           //国标2004 中 00<->0 表示黑屏 ；01<->1 表示绿色；10<->2 表示黄色；11<->3 表示红色
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
        if (g_CountDownCfg.cControllerType[i] != 0)//该倒计时已配置
        {
            frame_buf[j].frame_head = FRAME_HEAD_2004;
            SetCountValueNationStandard2004(frame_buf + j,g_CountDownCfg.cDeviceId[i]);
            frame_buf[j].check_sum = 0x7F & ((frame_buf[j].color_addr) ^ (frame_buf[j].data_high) ^ (frame_buf[j].data_low));
            j++;
        }
    }
    
    Send485Data((unsigned char *)frame_buf,sizeof(CountDownFrame2004) * j);//发送数据

#ifdef DUMP_DATA
    BaseDumpData((unsigned char *)frame_buf,sizeof(CountDownFrame2004) * j,5);
#endif
}

