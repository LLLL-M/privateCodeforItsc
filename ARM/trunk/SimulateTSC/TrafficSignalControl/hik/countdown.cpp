#include <stdio.h> 
#include <cstring> 
#include <errno.h> 
#include <stdlib.h> 
#include "common.h"
#include "countDown.h"
#include "platform.h"
#include "hik.h"
#include "configureManagement.h"
#include "its.h"


CountDown::CountDown()
{
    memset(&g_CountDownCfg, 0, sizeof(g_CountDownCfg));
    memset(gChannelStatus, 0, MAX_CHANNEL_NUM);
	memset(gChannelCountdown, 0, 2 * MAX_CHANNEL_NUM);
	fd_485 = -1;
}

int CountDown::GetCountdownNum()
{
	int i = 0;
    int ret = 0;
    
    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(g_CountDownCfg.cControllerType[i] != 0)
        {
            ret++;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : SetCountdownValue
 功能描述  : 计算某个设备当前正要运行的相位号，得到这个相位号，就能算出倒计时应该显示的灯色及数值
协议类型倒计时显示策略:
1. 先找到倒计时值最大的绿灯，如果找到则直接显示；
2. 否则，再找倒计时值最大的黄灯，如果找到则直接显示；
3. 否则，再找倒计时值最小的红灯，如果找到则直接显示；
4. 前面的都不满足条件，就发送关灯，倒计时值清零，同时报错。
 
 输入参数  : unsigned char cDeviceId  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月1日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CountDown::SetCountdownValue(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor)
{
    int i = 0;
    unsigned char nChannelId = g_CountDownCfg.cControllerID[cDeviceId][0];

    unsigned short nMaxGreenValue = 0;//倒计时值最大的绿灯
    unsigned short nMaxYellowValue = 0;//倒计时值最大的黄灯
    unsigned short nMinRedValue = gChannelCountdown[nChannelId - 1];//倒计时值最小的红灯

    *pPhaseCountDownTime = 0;
    *pPhaseColor = TURN_OFF;

    //一次性计算所有需要的值
    for(i = 0; i < NUM_CHANNEL; i++)
    {
        nChannelId = g_CountDownCfg.cControllerID[cDeviceId][i];
        //INFO("cDeviceId  %d ,nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",cDeviceId,nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);
        
        if(nChannelId <= 0 || nChannelId > NUM_CHANNEL)//如果通道号为0，表明已遍历完所有通道
        {
            break;
        }
        //INFO("nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);

        //找到最大绿灯
        if((gChannelStatus[nChannelId - 1] == GREEN || gChannelStatus[nChannelId - 1] == GREEN_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxGreenValue))
        {
            nMaxGreenValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxGreenValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //找到最大黄灯
        if((nMaxGreenValue == 0)
            && (gChannelStatus[nChannelId - 1] == YELLOW || gChannelStatus[nChannelId - 1] == YELLOW_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxYellowValue))
        {
            nMaxYellowValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxYellowValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //找到最小红灯
        if((nMaxGreenValue == 0)
            &&(nMaxYellowValue == 0)
            &&(gChannelStatus[nChannelId - 1] == RED || gChannelStatus[nChannelId - 1] == RED_BLINK || gChannelStatus[nChannelId - 1] == ALLRED) 
            && (gChannelCountdown[nChannelId - 1] <= nMinRedValue))
        {
            nMinRedValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMinRedValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1] == (UInt8)ALLRED ? (UInt8)RED : gChannelStatus[nChannelId - 1];
        }
    }    
}

/*****************************************************************************
 函 数 名  : SetCountDownValue
 功能描述  : 只是简单的保存一下各个通道的倒计时和值，目的是为了让各个协议的实现与库无关。
 
 输入参数  : LineQueueData *data  
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月23日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CountDown::SaveCountDownValue(const LineQueueData *data)
{
    if(data == NULL)
    {
        return;
    }
    
    memcpy(gChannelStatus,data->allChannels,sizeof(data->allChannels));
    memcpy(gChannelCountdown,data->channelCountdown,sizeof(data->channelCountdown));                                    
}


/*****************************************************************************
 函 数 名  : CountDownInterface
 功能描述  : 对外提供的倒计时接口，根据配置工具下发的倒计时类型，选择对应的
             协议。该函数最好每秒钟调用一次。
 输入参数  : 无
 返 回 值  : 
 修改历史  
  1.日    期   : 2015年4月22日
    作    者   : 肖文虎
    修改内容   : 新生成函数

*****************************************************************************/
void CountDown::CountDownInterface(LineQueueData *data)
{
	if (((data->schemeId == 0 || data->schemeId > NUM_SCHEME) 
			&& data->schemeId != INDUCTIVE_SCHEMEID && data->schemeId != INDUCTIVE_COORDINATE_SCHEMEID)
		|| data->phaseTableId == 0 || data->phaseTableId > MAX_PHASE_TABLE_COUNT
		|| data->channelTableId == 0 || data->channelTableId > MAX_CHANNEL_TABLE_COUNT)
		return;
    SaveCountDownValue(data);
  /*  switch(gStructBinfileCustom.sCountdownParams.iCountDownMode)
    {
        case SelfLearning:      break;
        case FullPulse:         countDownByAllPulse(data,g_CountDownCfg.nChannelFlag);break;
        case HalfPulse:         countDownByHalfPulse(data,g_CountDownCfg.nChannelFlag);break;
        case NationStandard:    countDownByNationStandard();break;
        case LaiSiStandard:     countDownByLaiSiProtocol(); break;
        case HisenseStandard:   countDownByHisenseStandard();break;
        case NationStandard2004:    countDownByNationStandard2004();break;
        default:                break;
    }*/
}


