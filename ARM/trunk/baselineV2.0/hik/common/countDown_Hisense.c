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

extern CountDownCfg        g_CountDownCfg;                         //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ

static void SetCountValueHisenseStandard(unsigned char cDeviceId,char *buf)
{
    unsigned char cPhaseCountDownTime = 0;
    unsigned char cPhaseColor = 0;
    unsigned char cDataLen = 0;
    
    SetCountdownValue(cDeviceId,&cPhaseCountDownTime,&cPhaseColor);    

    if(gStructBinfileCustom.sCountdownParams.iPulseGreenTime != 0)
    {
    	//������̵ƻ������������ҵ���ʱʱ��������õ��̵Ƶ���ʱʱ�䣬
    	//���ߵ���ʱʱ����ڸ�Ӧ���ʱ�䣬�رյ���ʱ
    	if((cPhaseColor == GREEN || cPhaseColor == GREEN_BLINK) && (cPhaseCountDownTime > gStructBinfileCustom.sCountdownParams.iPulseGreenTime \
            || gStructBinfileCustom.sCountdownParams.iFreeGreenTime < cPhaseCountDownTime))
    	{
            return;
    	}
    }
    if(gStructBinfileCustom.sCountdownParams.iPulseRedTime != 0)
    {
        //����Ǻ�ƣ����ҵ���ʱʱ��������õĺ�Ƶ���ʱʱ�䣬�رյ���ʱ
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
        if(g_CountDownCfg.cControllerType[i] != 0)//�õ���ʱ������
        {
            memset(buf,0,sizeof(buf));
            SetCountValueHisenseStandard(g_CountDownCfg.cDeviceId[i],buf);
            
            //INFO("countDownByHisenseStandard  %s\n",buf);

            Send485Data((unsigned char *)buf,strlen(buf));
        }
    }
}




