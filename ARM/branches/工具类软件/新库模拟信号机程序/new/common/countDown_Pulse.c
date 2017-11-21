#include "Hikconfig.h"
#include "configureManagement.h"
#include "its.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;

//�ж���λ�Ƿ����ɺ���л���������ɫ������������ɫ�л������
static Boolean IsPhaseChangeBetweenRed(PhaseInfo *nowPhaseInfo,PhaseInfo *lastPhaseInfo,UInt8 nType)
{
    if(nType == MOTOR)
    {
        if(nowPhaseInfo->phaseStatus == RED &&
            lastPhaseInfo->phaseStatus != RED)//������ɫ�л������
        {
            return TRUE;
        }

        if(nowPhaseInfo->phaseStatus != RED && 
            lastPhaseInfo->phaseStatus == RED)//����л���������ɫ
        {
            return TRUE;
        }
        
    }
    else if(nType == FOLLOW)
    {
        if(nowPhaseInfo->followPhaseStatus == RED &&
            lastPhaseInfo->followPhaseStatus != RED)//������ɫ�л������
        {
            return TRUE;
        }

        if(nowPhaseInfo->followPhaseStatus != RED && 
            lastPhaseInfo->followPhaseStatus == RED)//����л���������ɫ
        {
            return TRUE;
        }
    }

    return FALSE;
}


//����ȫ�̵���ʱ��ʵ��,�ýӿڱ���ÿ���ӵ���4������
//��λ���ɺ���л���������ɫ������������ɫ�л������ʱ���ڵ�500msʱ����һ�ιص�ָ��
void countDownByAllPulse(LineQueueData *data)
{
    int i = 0;
    int nPhaseId = 0;//ͨ����Ӧ����λ�������ǻ����������漰���ˣ�����Ŀǰ�ݲ�����,��Ϊ���˴󲿷�����ѧϰ�ģ����ܷ��صƣ��Է���ѧϰʧ��
    static PhaseInfo lastPhaseInfo[NUM_PHASE];//������һ�ε���λ��Ϣ
    static UInt8 nIsChannelSendOff[NUM_CHANNEL] = {0};//ͨ���Ƿ�Ҫ���͹ص���Ϣ

    if(gSignalControlpara == NULL)
        return;

    for(i = 0; i < NUM_CHANNEL; i++)//��ѯ����ͨ����Ӧ����λ
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        if((nPhaseId == 0) || (gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == PEDESTRIAN))//����ԴIDΪ0������û�����ø�ͨ�����������
        {
            continue;
        }
        //���ȷʵ��λ�������䣬��ô�ͼ�¼���������´ε���ʱ�ı�����Ϣ����һ�ε���ʱ��250ms���ڶ��ε���ʱ��500ms�������Ϳ����޸ĵ��ָ����
        if(TRUE == IsPhaseChangeBetweenRed(&data->phaseInfos[nPhaseId - 1],&lastPhaseInfo[nPhaseId - 1],gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType))
        {
            nIsChannelSendOff[i]++;
            continue;
        }

        if(nIsChannelSendOff[i] == 1)
        {
            nIsChannelSendOff[i] = 0;
            //���͹صƵ���ͨ��
            data->allChannels[i] = TURN_OFF;
//            INFO("OFFFF  %d\n",i+1);
        }
    }

    memcpy(lastPhaseInfo,data->phaseInfos,sizeof(lastPhaseInfo));
}



//�����̵���ʱ��ʵ��,�ýӿڱ���ÿ���ӵ���4������
//��λ����ʱ��С�ڵ��������õ���Ӧ��ʱ��ʱ������һ�ιص�ָ��
void countDownByHalfPulse(LineQueueData *data)
{
    int i = 0;
    int nPhaseId = 0;//ͨ����Ӧ����λ�������ǻ����������漰���ˣ�����Ŀǰ�ݲ�����,��Ϊ���˴󲿷�����ѧϰ�ģ����ܷ��صƣ��Է���ѧϰʧ��
    static UInt8 nIsChannelSentOff[NUM_CHANNEL];    //ͨ���Ƿ��Ѿ������˹ص�ָ��

    if(gSignalControlpara == NULL)
        return;
    for(i = 0; i < NUM_CHANNEL; i++)//��ѯ����ͨ����Ӧ����λ
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        if((nPhaseId == 0) || (gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType == PEDESTRIAN))//����ԴIDΪ0������û�����ø�ͨ�����������
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





