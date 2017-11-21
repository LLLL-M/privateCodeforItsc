/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ConfigureManagement.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��7��31��
  ����޸�   :
  ��������   : ConfigureManagement.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��7��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __CONFIGUREMANAGEMENT_H__
#define __CONFIGUREMANAGEMENT_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#include <unistd.h>
#include "platform.h"
#include "common.h"
#include "hik.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
#define IS_FILE_EXIST(fileName)     ((access((fileName),F_OK) == 0) ? 1 : 0)   

#define FILE_HIK_CFG_INI            "/home/config.ini"      //ȫ�ֲ��������ļ�������ͨ������λ����źŻ����б��������GPS���ص�������Ϣ���������кŵ���Ϣ
#define FILE_DESC_INI               "/home/desc.ini"        //�������������ļ�������ͨ��������������������Ϣ
#define FILE_CUSTOM_INI             "/home/custom.ini"      //������Ϣ�����ļ� ����������ʱ�����ڡ�����������
#define FILE_COUNTDOWN_INI          "/home/countdown.ini"   //�������е���ʱ�ӿڣ�����ͬһ�������ļ���
#define FILE_MISC_INI               "/home/misc.ini"        //�������л��ӵ�û��һ����������ò���������Ժ������Ҫ��ʱ���һЩ���õĻ������Էŵ����ļ���
#define FILE_TSC_CFG_INI            "/home/hikconfig.ini"

#define FILE_HIK_CFG_DAT        "/home/config.dat"      //ȫ�ֲ��������ļ�������ͨ������λ����źŻ����б��������GPS���ص�������Ϣ���������кŵ���Ϣ
#define FILE_DESC_DAT           "/home/desc.dat"        //�������������ļ�������ͨ��������������������Ϣ
#define FILE_CUSTOM_DAT         "/home/custom.dat"      //������Ϣ�����ļ� ����������ʱ�����ڡ�����������
#define FILE_COUNTDOWN_DAT      "/home/countdown.dat"   //�������е���ʱ�ӿڣ�����ͬһ�������ļ���
#define FILE_MISC_DAT           "/home/misc.dat"        //�������л��ӵ�û��һ����������ò���������Ժ������Ҫ��ʱ���һЩ���õĻ������Էŵ����ļ���
#define FILE_VEHICLE_DAT        "/home/vehicle.dat"     //��¼�˵�λ���ڳ������յ������ݼ���ͳ�ƽ��
#define FILE_TSC_CFG_DAT        "/home/hikconfig.dat"   //�����¿����б���İ����������õ��ļ���ȱ�ٸ��ļ����Ͽ�ĵ���ʱ�ӿڽ����ܵ�Ӱ�졣

#define READ_BIN_CFG_PARAMS(FILENAME,CFG,SIZE)                  ReadBinCfgInfo(FILENAME,CFG,SIZE,0,1)    
#define READ_BIN_CFG_PARAMS_OFFSET(FILENAME,CFG,SIZE,OFFSET)    ReadBinCfgInfo(FILENAME,CFG,SIZE,OFFSET,1)    
#define WRITE_BIN_CFG_PARAMS(FILENAME,CFG,SIZE)                 WriteBinCfgInfo(FILENAME,CFG,SIZE)   
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
#pragma pack(push,1)

//config.dat
typedef struct
{
	STRU_SPECIAL_PARAMS sSpecialParams;											//�����������ṹ��
	UINT8				cReservedValue1[512];									//��������������ṹ��ı����ֽ�
	UINT8				cCarDetectSwitch;										//����忪��
	UINT8 				cPrintfLogSwitch;										//��־��ӡ����
	UINT32				cFailureNumber;											//�������к�
	STRU_CURRENT_PARAMS	sCurrentParams[32];										//����������
	STRU_WIRELESS_CONTROLLER_INFO	stWirelessController;						//���߿�����ģ�����
	UINT8				cReservedValue2[512-(9+(MAX_WIRELESS_KEY-1)*64)];								
}STRUCT_BINFILE_CONFIG;

//custom.dat
typedef struct
{
	STRU_Count_Down_Params	sCountdownParams;									//��Ե���ʱ�Ƶ�����
	UINT8                   cIsCountdownValueLimit;                             //����ʱֵ�Ƿ��ܸ�Ӧ���ʱ������ƣ�Ĭ���ǲ����޵�.
	UINT8 					cReservedValue1[512 - 8 - 1];	//�����ֽ�,��ȥ8����Ϊ���ӻƵơ������˸������ʱ�Ƿ����޵Ĺ���
	COM_PARAMS 				sComParams[4];										//��Դ��ڲ���������
	UINT8 					cReservedValue2[512];
	CHANNEL_LOCK_PARAMS 	sChannelLockedParams;								//���ͨ����������������
	STRU_Channel_Lock_V2_INFO	sNewChannelLockedParams;							//�����ͨ����������������
	UINT8 					cReservedValue3[512];
	UINT8					cChannelLockFlag;									//ͨ��������ʶ//0��ʾδ������1��ʾ����,2��ʾ��������������״̬��ʾ�յ���ͨ����������ǵ�ǰʱ��Ϊ������ʱ��ε�״̬��
	UINT8					cSpecialControlSchemeId;							//������Ʒ�����
}STRUCT_BINFILE_CUSTOM;

//desc.dat
typedef struct
{
	PHASE_DESC_PARAMS 		sPhaseDescParams;									//��λ���� 
	UINT8 					cReservedValue1[512];
	CHANNEL_DESC_PARAMS		sChannelDescParams;   								//ͨ������
	UINT8 					cReservedValue2[512];
	PATTERN_NAME_PARAMS 	sPatternNameParams;   								//��������
	UINT8 					cReservedValue3[512];
	PLAN_NAME_PARAMS 		sPlanNameParams; 							        //�ƻ�����
	UINT8 					cReservedValue4[512];
	DATE_NAME_PARAMS 		sDateNameParams;							        //��������
	UINT8				    cReservedValue6[512];
}STRUCT_BINFILE_DESC;

//misc.dat
typedef struct
{
    UINT8                   cIsCanRestartHiktscAllowed;                         //�Ƿ�����CAN����߳�����hikTSC����
                                                                                //
}STRUCT_BINFILE_MISC;



#pragma pack(pop)
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void CopyChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_PARAMS *pChannelLockedParams);
extern void CopyNewChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_V2_PARAMS *pChannelLockedParams);
extern void ReadBinAllCfgParams(STRUCT_BINFILE_CONFIG *config,STRUCT_BINFILE_CUSTOM *custom,
                                STRUCT_BINFILE_DESC *desc,CountDownCfg *countdown,
                                STRUCT_BINFILE_MISC *misc);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __CONFIGUREMANAGEMENT_H__ */
