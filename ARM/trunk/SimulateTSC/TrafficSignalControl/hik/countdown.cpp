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
 �� �� ��  : SetCountdownValue
 ��������  : ����ĳ���豸��ǰ��Ҫ���е���λ�ţ��õ������λ�ţ������������ʱӦ����ʾ�ĵ�ɫ����ֵ
Э�����͵���ʱ��ʾ����:
1. ���ҵ�����ʱֵ�����̵ƣ�����ҵ���ֱ����ʾ��
2. �������ҵ���ʱֵ���ĻƵƣ�����ҵ���ֱ����ʾ��
3. �������ҵ���ʱֵ��С�ĺ�ƣ�����ҵ���ֱ����ʾ��
4. ǰ��Ķ��������������ͷ��͹صƣ�����ʱֵ���㣬ͬʱ����
 
 �������  : unsigned char cDeviceId  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��1��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void CountDown::SetCountdownValue(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor)
{
    int i = 0;
    unsigned char nChannelId = g_CountDownCfg.cControllerID[cDeviceId][0];

    unsigned short nMaxGreenValue = 0;//����ʱֵ�����̵�
    unsigned short nMaxYellowValue = 0;//����ʱֵ���ĻƵ�
    unsigned short nMinRedValue = gChannelCountdown[nChannelId - 1];//����ʱֵ��С�ĺ��

    *pPhaseCountDownTime = 0;
    *pPhaseColor = TURN_OFF;

    //һ���Լ���������Ҫ��ֵ
    for(i = 0; i < NUM_CHANNEL; i++)
    {
        nChannelId = g_CountDownCfg.cControllerID[cDeviceId][i];
        //INFO("cDeviceId  %d ,nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",cDeviceId,nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);
        
        if(nChannelId <= 0 || nChannelId > NUM_CHANNEL)//���ͨ����Ϊ0�������ѱ���������ͨ��
        {
            break;
        }
        //INFO("nChannelId %d, gChannelStatus  %s  gChannelCountdown  %d\n",nChannelId,GET_COLOR(gChannelStatus[nChannelId - 1]),gChannelCountdown[nChannelId - 1]);

        //�ҵ�����̵�
        if((gChannelStatus[nChannelId - 1] == GREEN || gChannelStatus[nChannelId - 1] == GREEN_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxGreenValue))
        {
            nMaxGreenValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxGreenValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //�ҵ����Ƶ�
        if((nMaxGreenValue == 0)
            && (gChannelStatus[nChannelId - 1] == YELLOW || gChannelStatus[nChannelId - 1] == YELLOW_BLINK) 
            && (gChannelCountdown[nChannelId - 1] > nMaxYellowValue))
        {
            nMaxYellowValue = gChannelCountdown[nChannelId - 1];
            *pPhaseCountDownTime = nMaxYellowValue;
            *pPhaseColor = gChannelStatus[nChannelId - 1];
            
            continue;
        }

        //�ҵ���С���
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
 �� �� ��  : SetCountDownValue
 ��������  : ֻ�Ǽ򵥵ı���һ�¸���ͨ���ĵ���ʱ��ֵ��Ŀ����Ϊ���ø���Э���ʵ������޹ء�
 
 �������  : LineQueueData *data  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��23��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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
 �� �� ��  : CountDownInterface
 ��������  : �����ṩ�ĵ���ʱ�ӿڣ��������ù����·��ĵ���ʱ���ͣ�ѡ���Ӧ��
             Э�顣�ú������ÿ���ӵ���һ�Ρ�
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��22��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

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


