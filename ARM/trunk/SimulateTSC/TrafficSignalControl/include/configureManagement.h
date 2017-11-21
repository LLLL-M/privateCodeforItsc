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
#define FILE_MISC_DAT               "/home/misc.dat"        //�������л��ӵ�û��һ����������ò���������Ժ������Ҫ��ʱ���һЩ���õĻ������Էŵ����ļ���

#define FILE_VEHICLE_DAT        "/home/vehicle.dat"     //��¼�˵�λ���ڳ������յ������ݼ���ͳ�ƽ��
#define FILE_TSC_CFG_DAT        "/home/hikconfig.dat"   //�����¿����б���İ����������õ��ļ���ȱ�ٸ��ļ����Ͽ�ĵ���ʱ�ӿڽ����ܵ�Ӱ�졣
#define FILE_TSC_GB_CFG_DAT		"/home/gbconfig.dat"	//����������ļ�
//WIN32
#define DATABASE_HIKCONFIG "hikconfig.db"
#define DATABASE_EXTCONFIG "extconfig.db"


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
	STRU_WIFI_INFO				stWifi;											//Wifi������Ϣ
	STRU_DEVICE_INFO			stDevice;										//�豸��Ϣ
	STRU_RGSIGNAL_CHECK_INFO	stRGSignalCheck;								//�����źż��
	UINT8				cReservedValue1[512 - 4 -64 -68 -24-1];					//��������������ṹ��ı����ֽ�,��ȥ4��������һ����λ�ӹܿ���
																				// -wifi(64)	-device(68) - rgsCheck(24)-kakou(1)
	UINT8				cCarDetectSwitch;										//����忪��, ����ʹ������ĳ���������
	UINT8 				cPrintfLogSwitch;										//��־��ӡ����
	UINT32				cFailureNumber;											//�������к�
	STRU_CURRENT_PARAMS	sCurrentParams[32];										//����������
	STRU_WIRELESS_CONTROLLER_INFO	stWirelessController;						//���߿�����ģ�����
	STRU_CAR_DETECT_INFO	stCarDetector;										//������
	STRU_SYS_USER_INFO		stUser;												//��¼�û���Ϣ
	STRU_FRONTBOARD_KEY_INFO	sFrontBoardKeys;									//ǰ��尴������
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
	DemotionParams 			demotionParams;		//��������ز���
	UINT8 					cReservedValue3[512 - sizeof(DemotionParams)];
	UINT8					cChannelLockFlag;									//ͨ��������ʶ//0��ʾδ������1��ʾ����,2��ʾ��������������״̬��ʾ�յ���ͨ����������ǵ�ǰʱ��Ϊ������ʱ��ε�״̬��
	UINT8					cSpecialControlSchemeId;							//������Ʒ�����
	STU_MUL_PERIODS_CHAN_PARAMS	sMulPeriodsChanLockParams;						//��ʱ��ͨ����������
}STRUCT_BINFILE_CUSTOM;

typedef UINT8 PhaseDescText[64];
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
	//added by Jicky
    PhaseDescText   		phaseDescText[16][16];     //16����λ����ÿ����λ��������Ϣ��ÿ����λ���֧��32������
}STRUCT_BINFILE_DESC;

//misc.dat
typedef struct
{
    UINT8                   cIsCanRestartHiktscAllowed;                         //�Ƿ�����CAN����߳�����hikTSC����
    unsigned int            time_zone_gap;                                      //ƽ̨�����ù���Уʱʱ����ʱ�������Ϊ��λ 
}STRUCT_BINFILE_MISC;


#pragma pack(pop)
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void CopyChannelLockInfoToCountDownParams(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParams,CHANNEL_LOCK_PARAMS *pChannelLockedParams);
extern void ReadBinAllCfgParams();

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __CONFIGUREMANAGEMENT_H__ */
