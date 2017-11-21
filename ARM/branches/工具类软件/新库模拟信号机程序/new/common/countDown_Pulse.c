#include "Hikconfig.h"
#include "configureManagement.h"
#include "its.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;

//判断相位是否在由红灯切换到其他灯色或者由其他灯色切换到红灯
static Boolean IsPhaseChangeBetweenRed(PhaseInfo *nowPhaseInfo,PhaseInfo *lastPhaseInfo,UInt8 nType)
{
    if(nType == MOTOR)
    {
        if(nowPhaseInfo->phaseStatus == RED &&
            lastPhaseInfo->phaseStatus != RED)//其他灯色切换到红灯
        {
            return TRUE;
        }

        if(nowPhaseInfo->phaseStatus != RED && 
            lastPhaseInfo->phaseStatus == RED)//红灯切换到其他灯色
        {
            return TRUE;
        }
        
    }
    else if(nType == FOLLOW)
    {
        if(nowPhaseInfo->followPhaseStatus == RED &&
            lastPhaseInfo->followPhaseStatus != RED)//其他灯色切换到红灯
        {
            return TRUE;
        }

        if(nowPhaseInfo->followPhaseStatus != RED && 
            lastPhaseInfo->followPhaseStatus == RED)//红灯切换到其他灯色
        {
            return TRUE;
        }
    }

    return FALSE;
}


//脉冲全程倒计时的实现,该接口必须每秒钟调用4次以上
//相位在由红灯切换到其他灯色或者由其他灯色切换到红灯时，在第500ms时发送一次关灯指令
void countDownByAllPulse(LineQueueData *data)
{
    int i = 0;
    int nPhaseId = 0;//通道对应的相位，可能是机动车、跟随及行人，行人目前暂不考虑,因为行人大部分是自学习的，不能发关灯，以防自学习失败
    static PhaseInfo lastPhaseInfo[NUM_PHASE];//保存上一次的相位信息
    static UInt8 nIsChannelSendOff[NUM_CHANNEL] = {0};//通道是否要发送关灯消息

    if(gSignalControlpara == NULL)
        return;

    for(i = 0; i < NUM_CHANNEL; i++)//轮询各个通道对应的相位
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        if((nPhaseId == 0) || (gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == PEDESTRIAN))//控制源ID为0，表明没有配置该通道，则不予计算
        {
            continue;
        }
        //如果确实相位发生跳变，那么就记录下来，待下次调用时改变点灯消息，第一次调用时是250ms，第二次调用时是500ms，这样就可以修改点灯指令了
        if(TRUE == IsPhaseChangeBetweenRed(&data->phaseInfos[nPhaseId - 1],&lastPhaseInfo[nPhaseId - 1],gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType))
        {
            nIsChannelSendOff[i]++;
            continue;
        }

        if(nIsChannelSendOff[i] == 1)
        {
            nIsChannelSendOff[i] = 0;
            //发送关灯到该通道
            data->allChannels[i] = TURN_OFF;
//            INFO("OFFFF  %d\n",i+1);
        }
    }

    memcpy(lastPhaseInfo,data->phaseInfos,sizeof(lastPhaseInfo));
}



//脉冲半程倒计时的实现,该接口必须每秒钟调用4次以上
//相位倒计时在小于等于已配置的相应的时间时，发送一次关灯指令
void countDownByHalfPulse(LineQueueData *data)
{
    int i = 0;
    int nPhaseId = 0;//通道对应的相位，可能是机动车、跟随及行人，行人目前暂不考虑,因为行人大部分是自学习的，不能发关灯，以防自学习失败
    static UInt8 nIsChannelSentOff[NUM_CHANNEL];    //通道是否已经发送了关灯指令

    if(gSignalControlpara == NULL)
        return;
    for(i = 0; i < NUM_CHANNEL; i++)//轮询各个通道对应的相位
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        if((nPhaseId == 0) || (gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == PEDESTRIAN))//控制源ID为0，表明没有配置该通道，则不予计算
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
                if(nIsChannelSentOff[i] == 0)
                {
                    nIsChannelSentOff[i] = 1;
                    data->allChannels[i] = TURN_OFF;
                }
            }
            else
            {
                nIsChannelSentOff[i] = 0;
            }
        }
        else if(gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == FOLLOW)
        {
            if(( (data->phaseInfos[nPhaseId - 1].followPhaseStatus== GREEN) 
                    && (data->phaseInfos[nPhaseId - 1].followPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseGreenTime) ) 
                || ( (data->phaseInfos[nPhaseId - 1].followPhaseStatus == RED) 
                    && (data->phaseInfos[nPhaseId - 1].followPhaseLeftTime == gStructBinfileCustom.sCountdownParams.iPulseRedTime) )
               )
            {
                if(nIsChannelSentOff[i] == 0)
                {
                    nIsChannelSentOff[i] = 1;
                    data->allChannels[i] = TURN_OFF;
                }
            }
            else
            {
                nIsChannelSentOff[i] = 0;
            }
        }
        
    }

}





