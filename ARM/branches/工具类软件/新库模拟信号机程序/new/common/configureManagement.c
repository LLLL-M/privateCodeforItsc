/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : configureManagement.c
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��8��4��
  ����޸�   :
  ��������   : ���ù���ģ�飬�ṩ�ܽӿڣ�������ȡ������Ϣ���Ժ����Ƕ�������
               �ļ��Ķ�ȡ�����ڿ����������ȡһ�Σ��Ҷ�ȡ������Ψһ�ӿڡ�

               ����������Ϣ�Ĳ���:

               1. ��

               
  �����б�   :
              ReadBinAllCfgParams
  �޸���ʷ   :
  1.��    ��   : 2015��8��4��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#include "configureManagement.h"
#include "binfile.h"
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
STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;  //�����/home/config.dat�����е����нṹ�壬���������������ṹ��/����忪��/��־��ӡ����/�������к�/����������
STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
STRUCT_BINFILE_DESC gStructBinfileDesc;          //�����/home/desc.dat�����е����нṹ�壬������λ����/ͨ������/��������/�ƻ�����/��������pthread_t thread;//���ܵ�����ѹ��Ϣ��CAN���
CountDownCfg        g_CountDownCfg;              //ȫ�ֲ�������ŵ���ͨѶЭ����Ҫ�����ݣ������ݱ������ļ��У�ͨ�������ļ������޸ġ�����
STRUCT_BINFILE_MISC gStructBinfileMisc;         //���Ӳ���

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*****************************************************************************
 �� �� ��  : ReadBinAllCfgParams
 ��������  : �ϵ������󣬰����ж����Ʋ���һ���Զ�������
 �������  : STRUCT_BINFILE_CONFIG *config  
             STRUCT_BINFILE_CUSTOM *custom  
             STRUCT_BINFILE_DESC *desc      
             CountDownCfg *countdown      
             ʵ���߼�:
             1. ���ж�dat�ļ��Ƿ���ڣ�����Ѵ��ڣ���������°汾����ֱ�Ӷ�ȡ�������ļ�;�������dat�ļ������ڻ��СΪ0�����Զ���ȡ�����ļ���
             2. ���dat�ļ������ڣ���������ϰ汾������������û�н��ı��ļ�ת�ɶ����ƣ����ȡ�ı��ļ�����ȡ�����󣬽��ı��ļ�ɾ����
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2015��7��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ɺ���

*****************************************************************************/
void ReadBinAllCfgParams()
{
    if(READ_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT, &gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG)) == 0)
    {
        if(IS_FILE_EXIST(FILE_HIK_CFG_INI) == 1)
        {
            //��ȡconfig.ini���й�GPS���ء��������������Ϻż�����������Ϣ
            get_special_params(FILE_HIK_CFG_INI,&gStructBinfileConfigPara);      
            WRITE_BIN_CFG_PARAMS(FILE_HIK_CFG_DAT,&gStructBinfileConfigPara,sizeof(STRUCT_BINFILE_CONFIG));
            unlink(FILE_HIK_CFG_INI);
        }
    }
#if 0
    INFO("Just For TianJing NingHe , Set GPS = 1, Watchdog = 1\n");
    gStructBinfileConfigPara.sSpecialParams.iGpsSwitch = 1;
    gStructBinfileConfigPara.sSpecialParams.iWatchdogSwitch = 1;
#endif
    if(READ_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC)) == 0)
    {
        //��ȡdesc.ini�й�����λ������ͨ����������Ϣ
        if(IS_FILE_EXIST(FILE_DESC_INI) == 1)
        {
            get_desc_params(FILE_DESC_INI,&gStructBinfileDesc); 
            WRITE_BIN_CFG_PARAMS(FILE_DESC_DAT,&gStructBinfileDesc,sizeof(STRUCT_BINFILE_DESC));
            unlink(FILE_DESC_INI);
        }
    }

    if(READ_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM)) == 0)
    {
         //��ȡ�Զ��������ļ��й��ڵ���ʱ�����ڵȲ�����������Ϣ
        if(IS_FILE_EXIST(FILE_CUSTOM_INI) == 1)
        {
            get_custom_params(FILE_CUSTOM_INI,&gStructBinfileCustom); 
            WRITE_BIN_CFG_PARAMS(FILE_CUSTOM_DAT,&gStructBinfileCustom,sizeof(STRUCT_BINFILE_CUSTOM));
            unlink(FILE_CUSTOM_INI);
        }
    }
    
	if (gStructBinfileCustom.cChannelLockFlag == 1)
		ItsChannelLock(&gStructBinfileCustom.sChannelLockedParams);
    
    if(READ_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,&g_CountDownCfg,sizeof(CountDownCfg)) == 0)
    {
        //����ʱ�������ò���
        if(IS_FILE_EXIST(FILE_COUNTDOWN_INI) == 1)
        {
            ReadCountdowncfgFromIni(FILE_COUNTDOWN_INI,&g_CountDownCfg);
            WRITE_BIN_CFG_PARAMS(FILE_COUNTDOWN_DAT,&g_CountDownCfg,sizeof(CountDownCfg));
            unlink(FILE_COUNTDOWN_INI);
        }
    }
    //ʹ����Դ����ʱ���Ļ�������ʱ�и�bug����������IDΪ0�ĵ���ʱ����ˣ�����ʱ�������û������IDΪ0�ĵ���ʱ������Ĭ�����һ��.
    if(g_CountDownCfg.cControllerType[0] == 0)
    {
        g_CountDownCfg.cControllerType[0] = 2;
        g_CountDownCfg.cDeviceId[0] = 0;
        g_CountDownCfg.cControllerID[0][0] = 1;
    }

    //��ȡ���ӵ����ò����������Ժ��Զ����һЩ�����������Է�������
    if(READ_BIN_CFG_PARAMS(FILE_MISC_DAT,&gStructBinfileMisc,sizeof(STRUCT_BINFILE_MISC)) == 0)
    {
        //����ʱ�������ò���
        if(IS_FILE_EXIST(FILE_MISC_INI) == 1)
        {
            ReadMiscCfgFromIni(FILE_MISC_INI,&gStructBinfileMisc);
            WRITE_BIN_CFG_PARAMS(FILE_MISC_DAT,&gStructBinfileMisc,sizeof(STRUCT_BINFILE_MISC));
            unlink(FILE_MISC_INI);
        }
    }
}



