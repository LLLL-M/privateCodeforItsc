#include "HikConfig.h"
#include "configureManagement.h"
#include "its.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;


//#define GET_BIT(v, n) (((v) >> (n)) & 0x1)		//取v的第 n bit位


//根据当前灯色，设置需要改变的状态
static inline void setChannelBlinkStatus(UINT32 nPhaseStatus,UINT8 *channelStatus)
{
    switch(nPhaseStatus)
    {
        case RED: *channelStatus = OFF_RED;break;
        case GREEN: *channelStatus = OFF_GREEN;break;
        case YELLOW: *channelStatus = OFF_YELLOW;break;
        default: break;
    }
}


//判断相位是否在由红灯切换到其他灯色或者由其他灯色切换到红灯
static inline Boolean IsPhaseChangeBetweenRed(PhaseInfo *nowPhaseInfo,PhaseInfo *lastPhaseInfo,UINT8 *channelStatus,UInt8 nType)
{
    Boolean ret = FALSE;

    if(nType == MOTOR)
    {
        if(nowPhaseInfo->phaseStatus == RED &&
            lastPhaseInfo->phaseStatus != RED)//其他灯色切换到红灯
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->phaseStatus != RED && 
            lastPhaseInfo->phaseStatus == RED)//红灯切换到其他灯色
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->phaseStatus,channelStatus);
    }
    else if(nType == FOLLOW || nType == OTHER)
    {
        if(nowPhaseInfo->followPhaseStatus == RED &&
            lastPhaseInfo->followPhaseStatus != RED)//其他灯色切换到红灯
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->followPhaseStatus != RED && 
            lastPhaseInfo->followPhaseStatus == RED)//红灯切换到其他灯色
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->followPhaseStatus,channelStatus);
    }
    else if(nType == PEDESTRIAN)
    {
        if(nowPhaseInfo->pedestrianPhaseStatus == RED &&
            lastPhaseInfo->pedestrianPhaseStatus != RED)//其他灯色切换到红灯
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->pedestrianPhaseStatus != RED && 
            lastPhaseInfo->pedestrianPhaseStatus == RED)//红灯切换到其他灯色
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->pedestrianPhaseStatus,channelStatus);
    }
    return ret;
}


//脉冲全程倒计时的实现,该接口必须每秒钟调用4次以上
//相位在由红灯切换到其他灯色或者由其他灯色切换到红灯时，在第500ms时发送一次关灯指令
void countDownByAllPulse(LineQueueData *data,UINT32 nChannelFlag)
{
    int i = 0;
    int nPhaseId = 0;//通道对应的相位，可能是机动车、跟随及行人，行人目前暂不考虑,因为行人大部分是自学习的，不能发关灯，以防自学习失败
    static PhaseInfo lastPhaseInfo[NUM_PHASE];//保存上一次的相位信息

    if(gSignalControlpara == NULL)
        return;

    for(i = 0; i < NUM_CHANNEL; i++)//轮询各个通道对应的相位
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        //控制源ID为0，表明没有配置该通道，或者通道i+1没有配置发送脉冲，则不予计算
        if((nPhaseId == 0) || (GET_BIT(nChannelFlag,i) == 0))
        {
            continue;
        }
        //如果确实相位发生跳变，那么就记录下来，待下次调用时改变点灯消息，第一次调用时是250ms，第二次调用时是500ms，这样就可以修改点灯指令了
        if(TRUE == IsPhaseChangeBetweenRed(&data->phaseInfos[nPhaseId - 1],&lastPhaseInfo[nPhaseId - 1],&data->allChannels[i],gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType))
        {
           // INFO("countDownByAllPulse ##############,channel  %d , phase  %d\n",i+1,nPhaseId);
            continue;
        }
    }

    memcpy(lastPhaseInfo,data->phaseInfos,sizeof(lastPhaseInfo));
}



//脉冲半程倒计时的实现,该接口必须每秒钟调用4次以上
//相位倒计时在小于等于已配置的相应的时间时，发送一次关灯指令
void countDownByHalfPulse(LineQueueData *data,UINT32 nChannelFlag)
{
    int i = 0;
    int nPhaseId = 0;//通道对应的相位，可能是机动车、跟随及行人，行人目前暂不考虑,因为行人大部分是自学习的，不能发关灯，以防自学习失败

    if(gSignalControlpara == NULL)
        return;
    for(i = 0; i < NUM_CHANNEL; i++)//轮询各个通道对应的相位
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        //控制源ID为0，表明没有配置该通道，或者通道i+1没有配置发送脉冲，则不予计算
        if((nPhaseId == 0) || (GET_BIT(nChannelFlag,i) == 0) )
        {
            continue;
        }
        if(gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == MOTOR)
        {
            if(( (data->phaseInfos[nPhaseId - 1].phaseStatus == GREEN) 
                    && (data->phaseInfos[nPhaseId - 1].phaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) ) 
                || ( (data->phaseInfos[nPhaseId - 1].phaseStatus == RED) 
                    && (data->phaseInfos[nPhaseId - 1].phaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) )
               )
            {
                setChannelBlinkStatus(data->phaseInfos[nPhaseId - 1].phaseStatus,&data->allChannels[i]);
            }
        }
        else if(gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == FOLLOW \
            || gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == OTHER)
        {
            if(( (data->phaseInfos[nPhaseId - 1].followPhaseStatus== GREEN) 
                    && (data->phaseInfos[nPhaseId - 1].followPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) ) 
                || ( (data->phaseInfos[nPhaseId - 1].followPhaseStatus == RED) 
                    && (data->phaseInfos[nPhaseId - 1].followPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) )
               )
            {
                 setChannelBlinkStatus(data->phaseInfos[nPhaseId - 1].followPhaseStatus,&data->allChannels[i]);
            }
        }
        else if(gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == PEDESTRIAN)
        {
            if(( (data->phaseInfos[nPhaseId - 1].pedestrianPhaseStatus== GREEN) 
                    && (data->phaseInfos[nPhaseId - 1].pedestrianPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) ) 
                || ( (data->phaseInfos[nPhaseId - 1].pedestrianPhaseStatus == RED) 
                    && (data->phaseInfos[nPhaseId - 1].pedestrianPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) )
               )
            {
                 setChannelBlinkStatus(data->phaseInfos[nPhaseId - 1].pedestrianPhaseStatus,&data->allChannels[i]);
            }
        }
    }

}





