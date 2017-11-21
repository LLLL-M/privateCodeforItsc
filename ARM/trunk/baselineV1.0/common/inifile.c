/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : inifile.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��3��13��
  ����޸�   :
  ��������   : �����ýṹ����Ϣд�뵽�ı��ļ��������ı��ļ��е�������Ϣ��ȡ���ṹ���С����ļ���Ҫ��ת������BinaryTextConvertʹ��
                ������ʵ�ֶ��������ı��ļ����ת����
                ÿ�����������ļ������������������Ҫ�޸ı��ļ�������ת�����߿����޷�����ʹ�á�
  �����б�   :
              get_all_params_from_config_file
              get_custom_params
              get_desc_params
              get_special_params
              set_custom_params
              set_desc_params
              set_failure_number
              set_special_params
  �޸���ʷ   :
  1.��    ��   : 2015��3��13��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "inifile.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : get_special_params
 ��������  : �������ļ��л�ȡ�������
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ͳһ���ýӿڣ����ٶ�δ��ļ�

  3.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �������ļ���ȡ���������ӵ��ýӿ���

  4.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : ���������ӵ��ýӿ���
*****************************************************************************/
void get_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config)
{
	parse_start(fileName);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //һЩ������
    config->sSpecialParams.iErrorDetectSwitch = get_one_value(section, "ErrorDetectSwitch");
	config->sSpecialParams.iCurrentAlarmSwitch = get_one_value(section, "CurrentAlarmSwitch");
	config->sSpecialParams.iVoltageAlarmSwitch = get_one_value(section, "VoltageAlarmSwitch");
	config->sSpecialParams.iCurrentAlarmAndProcessSwitch = get_one_value(section, "CurrentAlarmAndProcessSwitch");
	config->sSpecialParams.iVoltageAlarmAndProcessSwitch = get_one_value(section, "VoltageAlarmAndProcessSwitch");
	config->sSpecialParams.iWatchdogSwitch = get_one_value(section, "WatchdogSwitch");
	config->sSpecialParams.iGpsSwitch = get_one_value(section, "GpsSwitch");
	config->cCarDetectSwitch = get_one_value(section, "CarDetectSwitch");//������Ƿ����
	config->cPrintfLogSwitch = get_one_value(section, "PrintfLogSwitch");//printf��ӡ��Ϣ�Ƿ���
	config->sSpecialParams.iSignalMachineType = get_one_value(section, "SignalMachineType");

	//���߿��Ʋ���
	section="WirelessController";
	config->stWirelessController.iSwitch = get_one_value(section, "wirelessIsOpen");
	config->stWirelessController.iOvertime = get_one_value(section, "wirelessOvertime");
	config->stWirelessController.iCtrlMode = get_one_value(section, "wirelessCtrlMode");
	for(i=0; i<MAX_WIRELESS_KEY-1; i++)
	{
		sprintf(tmpStr,"key%d_Desc",i+1);
		//get_more_value(section, tmpStr,config->stWirelessController.key[i].description, 32);
		get_key_string(section,tmpStr,config->stWirelessController.key[i].description);
		sprintf(tmpStr,"key%d_Chan",i+1);
		get_more_value(section, tmpStr,config->stWirelessController.key[i].ucChan, 32);
	}

    //���Ϻ�
    config->cFailureNumber = get_one_value("failureinfo", "FailureNumber");

    
	DBG("ErrorDetectSwitch=%d  CurrentAlarmSwitch=%d  VoltageAlarmSwitch=%d  CurrentAlarmAndProcessSwitch=%d  \n\t\t\t\t VoltageAlarmAndProcessSwitch=%d  WatchdogSwitch=%d  GpsSwitch=%d  CarDetectSwitch=%d  PrintfLogSwitch=%d \n",
		config->sSpecialParams.iErrorDetectSwitch,config->sSpecialParams.iCurrentAlarmSwitch,config->sSpecialParams.iVoltageAlarmSwitch,
		config->sSpecialParams.iCurrentAlarmAndProcessSwitch,config->sSpecialParams.iVoltageAlarmAndProcessSwitch,
		config->sSpecialParams.iWatchdogSwitch,config->sSpecialParams.iGpsSwitch,config->cCarDetectSwitch,config->cPrintfLogSwitch);
		
    //�������
    section = "currentparams";
	for(i=0;i<32;i++)
	{
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		config->sCurrentParams[i].RedCurrentBase  = get_one_value(section, tmpStr);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
		config->sCurrentParams[i].RedCurrentDiff  = get_one_value(section, tmpStr);

		if( (0 != config->sCurrentParams[i].RedCurrentDiff) && (0 != config->sCurrentParams[i].RedCurrentBase))
		{
    		DBG("ͨ��%02d����ƫ��ֵ:%03d ������׼ֵ:%03d \n",i+1, config->sCurrentParams[i].RedCurrentDiff, config->sCurrentParams[i].RedCurrentBase);
		}
	}
    parse_end();
}

/*****************************************************************************
 �� �� ��  : set_special_params
 ��������  : ���������д�������ļ�
 �������  : struct SPECIAL_PARAMS special_params  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config)
{
	parse_start(fileName);

	int i = 0;
	char *section = "config";
    char tmpStr[32] = "";

    //һЩ������
	add_one_key(section, "ErrorDetectSwitch" ,config->sSpecialParams.iErrorDetectSwitch);
	add_one_key(section, "CurrentAlarmSwitch" ,config->sSpecialParams.iCurrentAlarmSwitch);
	add_one_key(section, "VoltageAlarmSwitch" ,config->sSpecialParams.iVoltageAlarmSwitch);
	add_one_key(section, "CurrentAlarmAndProcessSwitch" ,config->sSpecialParams.iCurrentAlarmAndProcessSwitch);
	add_one_key(section, "VoltageAlarmAndProcessSwitch" ,config->sSpecialParams.iVoltageAlarmAndProcessSwitch);
	add_one_key(section, "WatchdogSwitch" ,config->sSpecialParams.iWatchdogSwitch);
	add_one_key(section, "GpsSwitch" ,config->sSpecialParams.iGpsSwitch);
	add_one_key(section, "CarDetectSwitch" ,config->cCarDetectSwitch);
	add_one_key(section, "PrintfLogSwitch" ,config->cPrintfLogSwitch);
	add_one_key(section, "SignalMachineType" ,config->sSpecialParams.iSignalMachineType);
	add_one_key(section, "RedSignalCheckSwitch" ,config->sSpecialParams.iRedSignalCheckSwitch);

	//����ģ�����
	section = "WirelessController";
	add_one_key(section, "wirelessIsOpen" ,config->stWirelessController.iSwitch);
	add_one_key(section, "wirelessOvertime" ,config->stWirelessController.iOvertime);	
	add_one_key(section, "wirelessCtrlMode" ,config->stWirelessController.iCtrlMode);
	for(i=0; i<MAX_WIRELESS_KEY-1; i++)
	{
		sprintf(tmpStr,"key%d_Desc",i+1);
		//add_more_key(section, tmpStr, config->stWirelessController.key[i].ucChan, 32);
		add_key_string(section,tmpStr,config->stWirelessController.key[i].description);
		sprintf(tmpStr,"key%d_Chan",i+1);
		add_more_key(section, tmpStr, config->stWirelessController.key[i].ucChan, 32);
	}


    //���Ϻ�
    add_one_key("failureinfo", "FailureNumber" ,config->cFailureNumber);

    //�������
    section = "currentparams";
	for(i=0;i<32;i++)
	{
	    if(0 == config->sCurrentParams[i].RedCurrentBase)
	    {
            continue;
	    }
	    
		sprintf(tmpStr,"RedCurrentBase%02d",i+1);
		add_one_key(section, tmpStr ,config->sCurrentParams[i].RedCurrentBase);

		sprintf(tmpStr,"RedCurrentDiff%02d",i+1);
        add_one_key(section, tmpStr ,config->sCurrentParams[i].RedCurrentDiff);
	}

	parse_end();

}


/*****************************************************************************
 �� �� ��  : get_custom_params
 ��������  : ��ȡ�Զ��������ļ��еĵ���ʱ������������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��3��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : ��������ͨ��������Ϣ����������ʱ�ӿ��С�
*****************************************************************************/
void get_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom)
{
	parse_start(fileName);

	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //��ȡ����ʱ����
    pCustom->sCountdownParams.iCountDownMode = get_one_value(section, "CountDownMode");
    pCustom->sCountdownParams.iFreeGreenTime = get_one_value(section, "FreeGreenTime");
    pCustom->sCountdownParams.iPulseGreenTime = get_one_value(section, "PulseGreenTime");
    pCustom->sCountdownParams.iPulseRedTime = get_one_value(section, "PulseRedTime");
    pCustom->cIsCountdownValueLimit = get_one_value(section, "cIsCountdownValueLimit");
    
    DBG("����ʱ����  CountDownMode:%d  PulseGreenTime:%d  PulseRedTime:%d cIsCountdownValueLimit: %d\n", 
                                        pCustom->sCountdownParams.iCountDownMode,
                                        pCustom->sCountdownParams.iPulseGreenTime,
                                        pCustom->sCountdownParams.iPulseRedTime,
                                        pCustom->cIsCountdownValueLimit);

    //����ʱ��Ӧ��ͨ����Ϣ                                     
    for(i=1; i<33; i++)
    {
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase = get_one_value(section, tmpstr);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType = get_one_value(section, tmpstr);

	    if(0 != pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase)
	    {
            DBG("PhaseChannel_%02d  iphase %d  iType  %d\n",i,
                                        pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase,
                                        pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType);
	    }
    }

    //���ڲ���
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    pCustom->sComParams[i].unExtraParamValue = i+1;
	    
		sprintf(tmpstr,"ComBaudRate_%02d",i+1);	
		pCustom->sComParams[i].unBaudRate = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComParity_%02d",i+1);	
		pCustom->sComParams[i].unParity = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComDataBits_%02d",i+1);	
		pCustom->sComParams[i].unDataBits = get_one_value(section, tmpstr);
		
		sprintf(tmpstr,"ComStopBits_%02d",i+1);	
		pCustom->sComParams[i].unStopBits = get_one_value(section, tmpstr);

		if(0 != pCustom->sComParams[i].unBaudRate)
		{
            DBG("comNo=%d unBaudRate=%d unParity=%d unDataBits=%d unStopBits=%d\n",i+1,
                                                    pCustom->sComParams[i].unBaudRate,
                                                    pCustom->sComParams[i].unParity,
                                                    pCustom->sComParams[i].unDataBits,
                                                    pCustom->sComParams[i].unStopBits);
		}
	}	

    //ͨ����������
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    pCustom->sChannelLockedParams.ucWorkingTimeFlag = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeHour");
    pCustom->sChannelLockedParams.ucBeginTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeMin");
    pCustom->sChannelLockedParams.ucBeginTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucBeginTimeSec");
    pCustom->sChannelLockedParams.ucBeginTimeSec = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeHour");
    pCustom->sChannelLockedParams.ucEndTimeHour = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeMin");
    pCustom->sChannelLockedParams.ucEndTimeMin = get_one_value(section,tmpstr);

    strcpy(tmpstr,"ucEndTimeSec");
    pCustom->sChannelLockedParams.ucEndTimeSec = get_one_value(section,tmpstr);

    get_more_value(section,"ucChannelStatus",pCustom->sChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    pCustom->cChannelLockFlag = get_one_value(section,tmpstr);

	//��ͨ����������
	section = "NewChannelLockedParams";
    strcpy(tmpstr,"gNewChannelLockFlag");
    pCustom->sNewChannelLockedParams.uChannelLockFlag = get_one_value(section,tmpstr);

	for(i=0; i<16; i++)
	{
		sprintf(tmpstr,"NewChannelLockedPeriod%02d",i+1);
		get_more_value(tmpstr,"ucChannelStatus",pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucChannelStatus,32);
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucWorkingTimeFlag = get_one_value(tmpstr,"ucWorkingTimeFlag");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeHour = get_one_value(tmpstr,"ucBeginTimeHour");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeMin = get_one_value(tmpstr,"ucBeginTimeMin");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeSec = get_one_value(tmpstr,"ucBeginTimeSec");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeHour = get_one_value(tmpstr,"ucEndTimeHour");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeMin = get_one_value(tmpstr,"ucEndTimeMin");
		pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeSec = get_one_value(tmpstr,"ucEndTimeSec");
	}

    //������ƺ�
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    pCustom->cSpecialControlSchemeId = get_one_value(section,tmpstr);

    parse_end();
}

/*****************************************************************************
 �� �� ��  : set_custom_params
 ��������  : �����Ʋ������浽�����ļ�
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom)
{
	parse_start(fileName);
	int i = 0;
	char *section = "countdown";
    char tmpstr[64] = "";

    //����ʱ����
    add_one_key(section, "CountDownMode", pCustom->sCountdownParams.iCountDownMode);
    add_one_key(section, "FreeGreenTime", pCustom->sCountdownParams.iFreeGreenTime);
    add_one_key(section, "PulseGreenTime", pCustom->sCountdownParams.iPulseGreenTime);
    add_one_key(section, "PulseRedTime", pCustom->sCountdownParams.iPulseRedTime);
    add_one_key(section, "cIsCountdownValueLimit", pCustom->cIsCountdownValueLimit);

    //����ʱ��Ӧ��ͨ����Ϣ                                     
    for(i=1; i<33; i++)
    {
        if(0 == pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase)
        {
            continue;
        }
        
		sprintf(tmpstr,"PhaseChannel_%02d",i);
		add_one_key(section, tmpstr, pCustom->sCountdownParams.iPhaseOfChannel[i-1].iphase);

		sprintf(tmpstr,"ChannelProperty_%02d",i);
		add_one_key(section, tmpstr, pCustom->sCountdownParams.iPhaseOfChannel[i-1].iType);
    }

    //���ڲ���
    section = "comparams";
	for(i = 0; i < 4; i ++)
	{
	    if(0 == pCustom->sComParams[i].unBaudRate)
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"ComBaudRate_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unBaudRate);
		
		sprintf(tmpstr,"ComParity_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unParity);
		
		sprintf(tmpstr,"ComDataBits_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unDataBits);
		
		sprintf(tmpstr,"ComStopBits_%02d",pCustom->sComParams[i].unExtraParamValue);	
		add_one_key(section, tmpstr, pCustom->sComParams[i].unStopBits);

	}	

    //ͨ����������
    section = "ChannelLockedParams";

    strcpy(tmpstr,"ucWorkingTimeFlag");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucWorkingTimeFlag);

    strcpy(tmpstr,"ucBeginTimeHour");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeHour);

    strcpy(tmpstr,"ucBeginTimeMin");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeMin);

    strcpy(tmpstr,"ucBeginTimeSec");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucBeginTimeSec);

    strcpy(tmpstr,"ucEndTimeHour");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeHour);

    strcpy(tmpstr,"ucEndTimeMin");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeMin);

    strcpy(tmpstr,"ucEndTimeSec");
    add_one_key(section, tmpstr, pCustom->sChannelLockedParams.ucEndTimeSec);

    add_more_key(section, "ucChannelStatus",pCustom->sChannelLockedParams.ucChannelStatus,32);

    strcpy(tmpstr,"gChannelLockFlag");
    add_one_key(section, tmpstr, ((pCustom->cChannelLockFlag >= 1) ? 1 : pCustom->cChannelLockFlag));

    //��ͨ����������
	section = "NewChannelLockedParams";
    strcpy(tmpstr,"gNewChannelLockFlag");
	add_one_key(section, tmpstr, ((pCustom->sNewChannelLockedParams.uChannelLockFlag >= 1) ? 1 : pCustom->sNewChannelLockedParams.uChannelLockFlag));

	for(i=0; i<16; i++){
		sprintf(tmpstr, "NewChannelLockedPeriod%02d",i+1);
		add_more_key(tmpstr, "ucChannelStatus",pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucChannelStatus,32);
	    add_one_key(tmpstr, "ucWorkingTimeFlag", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucWorkingTimeFlag);
	    add_one_key(tmpstr, "ucBeginTimeHour", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeHour);
	    add_one_key(tmpstr, "ucBeginTimeMin", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeMin);
	    add_one_key(tmpstr, "ucBeginTimeSec", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucBeginTimeSec);
	    add_one_key(tmpstr, "ucEndTimeHour", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeHour);
	    add_one_key(tmpstr, "ucEndTimeMin", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeMin);
	    add_one_key(tmpstr, "ucEndTimeSec", pCustom->sNewChannelLockedParams.stChannelLockPeriods[i].ucEndTimeSec);
	}

    //������ƺ�
    section = "SpecialControlSchemeId";
    strcpy(tmpstr,"gSpecialControlSchemeId");
    add_one_key(section, tmpstr, pCustom->cSpecialControlSchemeId);

    DBG("set_custom_params  succeed\n");
    parse_end();

}

/*****************************************************************************
 �� �� ��  : get_desc_params
 ��������  : ��ȡdesc.ini�й�����λ������ͨ����������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��2��28��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void get_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc)
{
	parse_start(fileName);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //��ȡ��λ������Ϣ
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPhaseDescParams.stPhaseDesc[i]);

		if(0 != strlen((char *)pDesc->sPhaseDescParams.stPhaseDesc[i]))
		{
    		DBG("%s=%s\n",tmpstr,pDesc->sPhaseDescParams.stPhaseDesc[i]);
		}
	}	

    //��ȡͨ����Ϣ����
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sChannelDescParams.stChannelDesc[i]);

		if(0 != strlen((char *)pDesc->sChannelDescParams.stChannelDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sChannelDescParams.stChannelDesc[i]);
		}
	}	

    //��ȡ������Ϣ����
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPatternNameParams.stPatternNameDesc[i]);

		if(0 != strlen((char *)pDesc->sPatternNameParams.stPatternNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sPatternNameParams.stPatternNameDesc[i]);
		}
	}	
	
    //��ȡ�ƻ���Ϣ����
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sPlanNameParams.stPlanNameDesc[i]);

		if(0 != strlen((char *)pDesc->sPlanNameParams.stPlanNameDesc[i]))
		{
            DBG("%s=%s\n",tmpstr,pDesc->sPlanNameParams.stPlanNameDesc[i]);
		}
	}	

	//��ȡ������Ϣ����
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		get_key_string(section,tmpstr,(char *)pDesc->sDateNameParams.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		pDesc->sDateNameParams.stNameDesc[i].dateType = get_one_value(section, tmpstr);

        if(0 != strlen((char *)pDesc->sDateNameParams.stNameDesc[i].dateName))
        {
            DBG("DateDesc_%02d=%s  dateType= %d\n",i+1,pDesc->sDateNameParams.stNameDesc[i].dateName,pDesc->sDateNameParams.stNameDesc[i].dateType);
        }
	}	

    parse_end();
}

void set_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc)
{
	parse_start(fileName);

	int i = 0;
	char *section = "phasedesc";
    char tmpstr[64] = "";

    //��λ������Ϣ
	for(i = 0; i < 16; i ++)
	{
	    if(0 == strlen((char *)pDesc->sPhaseDescParams.stPhaseDesc[i]))
	    {
            continue;
	    }
	    
		sprintf(tmpstr,"PhaseDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPhaseDescParams.stPhaseDesc[i]);

	}	

    //ͨ����Ϣ����
    section = "channeldesc";
	for(i = 0; i < 32; i ++)
	{
		if(0 == strlen((char *)pDesc->sChannelDescParams.stChannelDesc[i]))
		{
            continue;
		}	
		
		sprintf(tmpstr,"ChannelDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sChannelDescParams.stChannelDesc[i]);


	}	

    //������Ϣ����
    section = "patterndesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pDesc->sPatternNameParams.stPatternNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PatternDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPatternNameParams.stPatternNameDesc[i]);


	}	
	
    //�ƻ���Ϣ����
    section = "plandesc";
	for(i = 0; i < 16; i ++)
	{
		if(0 == strlen((char *)pDesc->sPlanNameParams.stPlanNameDesc[i]))
		{
            continue;
		}
	
		sprintf(tmpstr,"PlanDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sPlanNameParams.stPlanNameDesc[i]);
	}	

	//������Ϣ����
    section = "datedesc";
	for(i = 0; i < 40; i ++)
	{
        if(0 == strlen((char *)pDesc->sDateNameParams.stNameDesc[i].dateName))
        {
            continue;
        }
	
		sprintf(tmpstr,"DateDesc_%02d",i+1);	
		add_key_string(section,tmpstr,(char *)pDesc->sDateNameParams.stNameDesc[i].dateName);
		
		sprintf(tmpstr,"DateFlag_%02d",i+1);
		add_one_key(section,tmpstr, pDesc->sDateNameParams.stNameDesc[i].dateType);
	}	

    parse_end();

}

/*****************************************************************************
 �� �� ��  : ReadCountdowncfgFromIni
 ��������  : ��������ʱ�Ƶ�������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��23��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadCountdowncfgFromIni(char *fileName,CountDownCfg *cfg)
{
	parse_start(fileName);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        sprintf(tmpstr,"Device_%d",i);

        cfg->cDeviceId[i] = i;
        get_more_value(tmpstr,"cControllerID",cfg->cControllerID[i],MAX_NUM_PHASE);
        cfg->cControllerType[i] = get_one_value(tmpstr,"cControllerType");

        //INFO("===>  id %d, cControllerId  %d, type %d \n",g_CountDownCfg.cDeviceId[i],g_CountDownCfg.cControllerID[i][0],g_CountDownCfg.cControllerType[i]);
    }

    parse_end();
}

/*****************************************************************************
 �� �� ��  : WriteCountdownCfgToIni
 ��������  : д����ʱ�Ƶ�������Ϣ
 �������  : ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��4��23��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void WriteCountdownCfgToIni(char *fileName,CountDownCfg *cfg)
{
	parse_start(fileName);

	int i = 0;
    char tmpstr[64] = "";

    for(i = 0; i < MAX_NUM_COUNTDOWN; i++)
    {
        if(cfg->cControllerType[i] == 0)//���û�����õ���ʱ�����Ͳ�����ʾ
        {
            continue;
        }
        
        sprintf(tmpstr,"Device_%d",i);

        add_one_key(tmpstr,"cControllerType",cfg->cControllerType[i]);
        add_more_key(tmpstr,"cControllerID",cfg->cControllerID[i],MAX_NUM_PHASE);
    }

    parse_end();
}

void ReadMiscCfgFromIni(char *fileName,STRUCT_BINFILE_MISC *cfg)
{
	parse_start(fileName);

    char tmpstr[64] = "MISC";
    cfg->cIsCanRestartHiktscAllowed = get_one_value(tmpstr,"cControllerType");
    parse_end();
}

void WriteMiscCfgToIni(char *fileName,STRUCT_BINFILE_MISC *cfg)
{
	parse_start(fileName);

    char tmpstr[64] = "MISC";
    add_one_key(tmpstr,"cControllerType",cfg->cIsCanRestartHiktscAllowed);
    
    parse_end();
}



