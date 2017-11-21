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

extern CountDownCfg        g_CountDownCfg;                         //全局参数，存放的是通讯协议需要的数据，该数据保存在文件中，通过配置文件进行修改、保存
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //存放在/home/custom.dat配置中的所有结构体,包括针对倒计时牌协议的配置/针对串口参数的配置/针对通道锁定参数的配置/通道锁定标识

static void SetCountValueHisenseStandard(unsigned char cDeviceId,char *buf)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cDataLen = 0;
    
    SetCountdownValue(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);    

    if(gStructBinfileCustom.sCountdownParams.iPulseGreenTime != 0)
    {
    	//如果是绿灯或者绿闪，并且倒计时时间大于配置的绿灯倒计时时间，
    	//或者倒计时时间大于感应检测时间，关闭倒计时
    	if((cPhaseColor == GREEN || cPhaseColor == GREEN_BLINK) && (cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseGreenTime \
            || gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime))
    	{
            return;
    	}
    }
    if(gStructBinfileCustom.sCountdownParams.iPulseRedTime != 0)
    {
        //如果是红灯，并且倒计时时间大于配置的红灯倒计时时间，关闭倒计时
        if(cPhaseColor == RED && cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseRedTime)
    	{
            return;
    	}
    }

    
    strcpy(buf,"$SIGN,");//start 
    sprintf(buf+strlen(buf),"%d,",cDeviceId+1);

    if(g_CountDownCfg.cControllerType[cDeviceId] == PEDESTRIAN)//pedestrian type
    {
        strcat(buf,"P,");
    }
    else
    {
        strcat(buf,"V,");
    }
    
    if((GREEN == cPhaseColor) || (GREEN_BLINK == cPhaseColor))//1 0
    {
        strcat(buf,"G,");
    }
    else if(RED == cPhaseColor)//0 1
    {
        strcat(buf,"R,");
    }
    else if((YELLOW == cPhaseColor) || (YELLOW_BLINK == cPhaseColor))//0 0 
    {
        strcat(buf,"Y,");
    }
    else
    {
        strcat(buf,"W,");
    }
    
    sprintf(buf+strlen(buf),"%d:",cPhaseCountDownTime);

    cDataLen = strlen(buf) - strlen("$SIGN");

    sprintf(buf+strlen(buf),"%x\r\n",cDataLen);

}

void countDownByHisenseStandard()
{
    int i = 0;
    char buf[48] = {0};
    
    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfg.cControllerType[i] != 0)//该倒计时已配置
        {
            memset(buf,0,sizeof(buf));
            SetCountValueHisenseStandard(g_CountDownCfg.cDeviceId[i],buf);
            
            //INFO("countDownByHisenseStandard  %s\n",buf);

            Send485Data((unsigned char *)buf,strlen(buf));
        }
    }
}




