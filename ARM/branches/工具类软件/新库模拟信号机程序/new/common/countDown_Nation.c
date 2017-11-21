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

static unsigned char gNationStandardCountdownNum = 0;       //实际配置使用的倒计时个数，最多32个
static unsigned char g_SendBufNationStandard[68] = {0};     //国标中，最多发送(2+1+32*2+1 = 68)字节数据

extern CountDownCfg        g_CountDownCfg;                         //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识

/*static void PrintDataNationStandard()
{
    int i = 0;

    for(i=0; i < gNationStandardCountdownNum*2+4; i++)
    {
        fprintf(stderr," %x ",g_SendBufNationStandard[i]);

    }

    fprintf(stderr,"        ==\n");
}*/



static void SetCountValueNationStandard(unsigned char *cPosition,unsigned char cDeviceId)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cTemp = 0;
    
    GetRuningPhaseId(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);    
	
	if(gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime)//如果某相位的倒计时时间比感应检测时间大，则将显示时间清零，不予显示。如果需要实时显示倒计时状态，那么可以通过配置工具，将单元参数中的感应检测时间调大些。
	{
		cPhaseColor = TURN_OFF;
		cPhaseCountDownTime = 0;
	}
    //INFO("SetCountValueNationStandard  id  %d, time  %d, color  %d\n",cDeviceId,cPhaseCountDownTime,cPhaseColor);

    switch(cPhaseColor)
    {
        case GREEN:         
        {
            cTemp = 1;
            cTemp |= (cDeviceId<<3);
            break;
        }
        case RED:           
        {
            cTemp = 3;
            cTemp |= (cDeviceId<<3);
            break;
        }
        case YELLOW:      
        {
            cTemp = 2;
            cTemp |= (cDeviceId<<3);
            break;
        }        
        case GREEN_BLINK:    
        {
            cTemp = 1;
            cTemp |= (0x01<<2);
            cTemp |= (cDeviceId<<3);
            break;
        }
        case YELLOW_BLINK:    
        {
            cTemp = 2;
            cTemp |= (0x01<<2);
            cTemp |= (cDeviceId<<3);
            break;
        }
        case TURN_OFF:          
        {
            cTemp |= (cDeviceId<<3);
            break;
        }  
        default:    cTemp |= (cDeviceId<<3); break;
    }


    g_SendBufNationStandard[*cPosition] = cTemp;

    *cPosition += 1;

    g_SendBufNationStandard[*cPosition] = cPhaseCountDownTime;

    *cPosition += 1;
}

static unsigned char GetCheckSum(unsigned char len)
{
    int i = 0;
    unsigned char cCheckSum = 0;

    for(i = 2; i < len; i++)//国标协议中指定，除掉帧头。
    {
        cCheckSum ^= (g_SendBufNationStandard[i]&0xff);

    }

    return cCheckSum;
}


void countDownByNationStandard()
{
    int i = 0;
    unsigned char cPosition = 3;

    gNationStandardCountdownNum = GetCoundownNum();

    memset(g_SendBufNationStandard,0,sizeof(g_SendBufNationStandard));
    
    g_SendBufNationStandard[0] = 0x55;
    g_SendBufNationStandard[1] = 0xAA;//前两个字节，表示一帧的开始。
    
    g_SendBufNationStandard[2] = gNationStandardCountdownNum;//倒计时个数

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfg.cControllerType[i] != 0)//该倒计时已配置
        {
            SetCountValueNationStandard(&cPosition,g_CountDownCfg.cDeviceId[i]);
            
        }

    }

    g_SendBufNationStandard[cPosition] = GetCheckSum(cPosition);

    Send485Data(g_SendBufNationStandard, (gNationStandardCountdownNum*2 + 4));//发送数据

}



