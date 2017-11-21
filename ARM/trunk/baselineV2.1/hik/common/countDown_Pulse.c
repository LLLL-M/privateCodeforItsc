#include "HikConfig.h"
#include "configureManagement.h"
#include "its.h"

extern SignalControllerPara *gSignalControlpara;
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;


//#define GET_BIT(v, n) (((v) >> (n)) & 0x1)		//ȡv�ĵ� n bitλ


//���ݵ�ǰ��ɫ��������Ҫ�ı��״̬
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


//�ж���λ�Ƿ����ɺ���л���������ɫ������������ɫ�л������
static inline Boolean IsPhaseChangeBetweenRed(PhaseInfo *nowPhaseInfo,PhaseInfo *lastPhaseInfo,UINT8 *channelStatus,UInt8 nType)
{
    Boolean ret = FALSE;

    if(nType == MOTOR)
    {
        if(nowPhaseInfo->phaseStatus == RED &&
            lastPhaseInfo->phaseStatus != RED)//������ɫ�л������
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->phaseStatus != RED && 
            lastPhaseInfo->phaseStatus == RED)//����л���������ɫ
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->phaseStatus,channelStatus);
    }
    else if(nType == FOLLOW || nType == OTHER)
    {
        if(nowPhaseInfo->followPhaseStatus == RED &&
            lastPhaseInfo->followPhaseStatus != RED)//������ɫ�л������
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->followPhaseStatus != RED && 
            lastPhaseInfo->followPhaseStatus == RED)//����л���������ɫ
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->followPhaseStatus,channelStatus);
    }
    else if(nType == PEDESTRIAN)
    {
        if(nowPhaseInfo->pedestrianPhaseStatus == RED &&
            lastPhaseInfo->pedestrianPhaseStatus != RED)//������ɫ�л������
        {
            ret = TRUE;
        }

        if(nowPhaseInfo->pedestrianPhaseStatus != RED && 
            lastPhaseInfo->pedestrianPhaseStatus == RED)//����л���������ɫ
        {
            ret = TRUE;
        }
        if(TRUE == ret)
            setChannelBlinkStatus(nowPhaseInfo->pedestrianPhaseStatus,channelStatus);
    }
    return ret;
}


//����ȫ�̵���ʱ��ʵ��,�ýӿڱ���ÿ���ӵ���4������
//��λ���ɺ���л���������ɫ������������ɫ�л������ʱ���ڵ�500msʱ����һ�ιص�ָ��
void countDownByAllPulse(LineQueueData *data,UINT32 nChannelFlag)
{
    int i = 0;
    int nPhaseId = 0;//ͨ����Ӧ����λ�������ǻ����������漰���ˣ�����Ŀǰ�ݲ�����,��Ϊ���˴󲿷�����ѧϰ�ģ����ܷ��صƣ��Է���ѧϰʧ��
    static PhaseInfo lastPhaseInfo[NUM_PHASE];//������һ�ε���λ��Ϣ

    if(gSignalControlpara == NULL)
        return;

    for(i = 0; i < NUM_CHANNEL; i++)//��ѯ����ͨ����Ӧ����λ
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        //����ԴIDΪ0������û�����ø�ͨ��������ͨ��i+1û�����÷������壬�������
        if((nPhaseId == 0) || (GET_BIT(nChannelFlag,i) == 0))
        {
            continue;
        }
        //���ȷʵ��λ�������䣬��ô�ͼ�¼���������´ε���ʱ�ı�����Ϣ����һ�ε���ʱ��250ms���ڶ��ε���ʱ��500ms�������Ϳ����޸ĵ��ָ����
        if(TRUE == IsPhaseChangeBetweenRed(&data->phaseInfos[nPhaseId - 1],&lastPhaseInfo[nPhaseId - 1],&data->allChannels[i],gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerType))
        {
           // INFO("countDownByAllPulse ##############,channel  %d , phase  %d\n",i+1,nPhaseId);
            continue;
        }
    }

    memcpy(lastPhaseInfo,data->phaseInfos,sizeof(lastPhaseInfo));
}



//�����̵���ʱ��ʵ��,�ýӿڱ���ÿ���ӵ���4������
//��λ����ʱ��С�ڵ��������õ���Ӧ��ʱ��ʱ������һ�ιص�ָ��
void countDownByHalfPulse(LineQueueData *data,UINT32 nChannelFlag)
{
    int i = 0;
    int nPhaseId = 0;//ͨ����Ӧ����λ�������ǻ����������漰���ˣ�����Ŀǰ�ݲ�����,��Ϊ���˴󲿷�����ѧϰ�ģ����ܷ��صƣ��Է���ѧϰʧ��

    if(gSignalControlpara == NULL)
        return;
    for(i = 0; i < NUM_CHANNEL; i++)//��ѯ����ͨ����Ӧ����λ
    {
        nPhaseId = gSignalControlpara->stChannel[data->channelTableId - 1][i].nControllerID;
        //����ԴIDΪ0������û�����ø�ͨ��������ͨ��i+1û�����÷������壬�������
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





