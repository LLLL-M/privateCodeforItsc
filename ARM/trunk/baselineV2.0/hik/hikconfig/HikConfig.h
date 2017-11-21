/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : HikConfig.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : HikConfig.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �������ĳ�����ͷ�ļ������޸�
******************************************************************************/

#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "hik.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#pragma pack(push, 4)




/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HIKCONFIG_LIB_VERSION	"1.1.0"

#define CFG_NAME "/home/hikconfig.dat"
#define CFG_BAK_NAME "/home/hikconfig.bak"

#define BIT(v, n) (((v) >> (n)) & 0x1)		//ȡv�ĵ� n bitλ

//��λ�Ƿ�ʹ�ܣ��������Ϊ��λѡ���ʹ���ˣ��򷵻�ֵΪ1�����򷵻�ֵΪ0
#define IS_PHASE_INABLE(option)     ((((option)&&0x01) == 1 ) ? 1 : 0)

#define     NUM_SCHEDULE                    255               //���ȱ�����,modified by xiaowh
#define     NUM_TIME_INTERVAL               16               //ʱ�α�����
#define     NUM_TIME_INTERVAL_ID            48               //ÿ��ʱ�α��е�ʱ�κ�����
#define     NUM_ACTION                     255               //����������
#define     NUM_SCHEME                     108               //����������
#define     NUM_CHANNEL                     32               //ͨ��������	changed by Jicky
#define     NUM_GREEN_SIGNAL_RATION         36               //���űȱ�����
#define     NUM_PHASE                       16               //��λ������
#define     NUM_FOLLOW_PHASE                16               //������λ������
#define     NUM_PHASE_TURN                  16               //���������
#define     NUM_RING_COUNT                   4               //�������֧�ֵĻ������ֵ

#define     MAX_VEHICLEDETECTOR_COUNT			72//80	//������������������ trap
#define		MAX_VEHICLEDETECTOR_COUNT_S			48 		//������������������ snmp
#define		MAX_VEHICLEDETECTOR_STATUS_COUNT	8		//���������״̬����������
#define		MAX_PEDESTRIANDETECTOR_COUNT		8		//���˼�������������

#define		MAX_VOLUMEOCCUPANCY_COUNT			MAX_VEHICLEDETECTOR_COUNT		//����ռ���ʱ��������

#define		MAX_OVERLAP_COUNT					16		//�ص����������
#define		MAX_PHASE_COUNT_IN_OVERLAP			16		//�ص��е����include��λ��
#define		MAX_PHASE_COUNT_MO_OVERLAP			16		//�ص��е����Modifier��λ��
#define		MAX_OVERLAP_STATUS_COUNT			2		//�ص�״̬���������
#define		MAX_MODULE_COUNT					255		//ģ����������
#define		MAX_MODULE_STRING_LENGTH			128		//ģ�����ַ�������

#define		MAX_EVENTLOG_COUNT					255		//�¼����������
#define		MAX_EVENTCLASS_COUNT				1		//�¼��������ֵ��ʵ��Ϊ3
#define		MAX_EVENTCONFIG_COUNT				50		//�¼����ñ������

#define		MAX_PREEMPT_COUNT					8		//����һ�������
#define		MAX_PEDESTRIANPHASE_COUNT			8  		//������λ������

#define		MAX_COUNTDOWNBOARD_COUNT			24		//��󵹼�ʱ�Ƹ���
#define		MAX_COUNTDOWNBOARD_INCLUDEDPHASES	16		//ÿ������ʱ�����ܰ����������λ��

#define		MAX_SPECIALFUNCOUTPUT_COUNT			8		//���֧�ֵ����⹦������
#define		MAX_DYNPATTERNSEL_COUNT				8		//���̬����ѡ�����ñ�����
#define		MAX_RS232PORT_COUNT					3		//���֧�ֵĶ˿����ñ�����
#define 	MAX_SIGNALTRANS_LIMIT	            20		//�ź�ת����������ʱ������
#define 	MAX_ALLSTOPPHASE_LIMIT              32      //������λ������ 
#define		MAX_BYTE_VALUE			255
#define		MAX_2BYTE_VALUE			65535

#define		MAX_TRANSINTENSITYCHOICE_COUNT			90		//��ͨǿ������ѡ����������
#define     MAX_PHASE_TYPE_COUNT                3         //��λ����   

#define     MAX_ITSCONSERI_COUNT                6       //���ڱ���������    
#define     MAX_ITSCONIO_COUNT                  24      //IO ���ñ��������   
#define     MAX_ITSCONCHANNEL_COUNT             32      //��ͻͨ�����������  

#define 	MAX_EXTEND_TABLE_COUNT				16						//�����λ�����
#define 	MAX_PHASE_TABLE_COUNT			MAX_EXTEND_TABLE_COUNT		//�����λ�����
#define		MAX_CHANNEL_TABLE_COUNT			MAX_EXTEND_TABLE_COUNT		//���ͨ�������
#define 	MAX_FOLLOW_PHASE_TABLE_COUNT	MAX_EXTEND_TABLE_COUNT		//��������λ�����

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/

typedef enum
{
	ERROR_NULL_POINTER = 1,						    //ȫ��ָ��Ϊ��
	ERROR_SUBSCRIPT_PHASE = 2,				 		//��λ�����±�ֵ+1��ʵ����λֵ����ͬ��nPhaseArray[i] != (i+1)
	ERROR_SUBSCRIPT_PHASE_TURN = 3,				    //�����������±�ֵ+1��ʵ�������Ų���ͬ��
	ERROR_SUSCRIPT_PHASE_TURN_CIRCLE = 4,			//�����Ļ�������±�ֵ+1��ʵ�ʻ��Ų���ͬ��
	ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION = 5,		//���űȱ�������±�ֵ+1��ʵ�����űȱ�Ų���ͬ��
	ERROR_SUBSCRIPT_GREEN_SIGNAL_RATION_PHASE = 6,	//���űȱ����λ�����±�ֵ+1��ʵ����λ�Ų���ͬ��
	ERROR_SUBSCRIPT_CHANNEL = 7,					//ͨ����������±�ֵ+1��ʵ��ͨ���Ų���ͬ��
	ERROR_SUBSCRIPT_SCHEME = 8,					//������������±�ֵ+1��ʵ�ʷ����Ų���ͬ��
	ERROR_SUBSCRIPT_ACTION = 9,					//������������±�ֵ+1��ʵ�ʶ����Ų���ͬ��
	ERROR_SUBSCRIPT_TIMEINTERVAL = 10,				//ʱ�α�������±�ֵ+1��ʵ��ʱ�α�Ų���ͬ��
	ERROR_SUBSCRIPT_TIMEINTERVAL_TIME = 11,			//�������ʱ�������±�ֵ+1��ʵ��ʱ�κŲ���ͬ��
	ERROR_SUBSCRIPT_SCHEDULE = 12,					//���ȱ�������±�ֵ+1����ȱ�Ų���ͬ��
	ERROR_SUBSCRIPT_FOLLOW_PHASE = 13,				//������λ��������±�ֵ+1��ʵ�ʸ�����λ�Ų���ͬ��
	
	ERROR_SAME_CIRCLE_CONCURRENT_PHASE = 14,		//������λ������λ�Ŵ����ڲ�����λ�����С�
	ERROR_FATHER_NOT_EXIST_CONCURRENT_PHASE = 15,	//������λ����λ�Ų��������䲢����λ�Ĳ�����λ�����С�
	ERROR_CHILD_NOT_CONTINUOUS_CONCURRENT_PHASE = 16, //������λ������λ��������в�������
	ERROR_BARRIER_CONCURRENT_PHASE = 17, 			//������λ������һ�����ű�ʱ�䲻��ȡ�
	ERROR_NOT_EQUAL_PHASE_TURN_CIRCLE = 18,			//������л��ź���λ���и���λ�Ļ��Ų���ͬ��
	ERROR_NOT_CORRECT_PHASE_TURN = 19,				//���������λ����źͲ�����λ��ƥ��
	ERROR_SPLIT_LOW_MOTO_GREEN_SIGNAL_RATION = 20,	//���űȱ������ڳ��Ȼ��������������Ƶơ�ȫ��֮��С
	ERROR_SPLIT_LOW_PEDESTRIAN_GREEN_SIGNAL_RATION = 21,//���űȱ������ڳ���������ա����˷���ʱ��֮��С��
	ERROR_ILLEGAL_SCHME = 22,						//�����������������űȱ�ID�Ƿ���
	ERROR_CIRCLE_TIME_SCHEME = 23,					//�����������ڳ������ڵ������ű�֮��
	
	ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE = 24,	//������λ���еĲ�����λID�����ڡ�
	ERROR_NOT_EXIST_PHASE_TURN_PHASE = 25,			//���������λ�Ų����ڡ�
	ERROR_NOT_EXIST_SOURCE_CHANNEL = 26,				//ͨ�����и�ͨ���Ŀ���ԴID�����ڡ�
	ERROR_NOT_EXIST_SOURCE_FOLLOW_CHANNEL = 27,		//ͨ�����и�ͨ���ĸ������ԴID�����ڡ�
	ERROR_NOT_EXIST_SOURCE_OTHER_CHANNEL = 28,		//ͨ�����и�ͨ������������ԴID�����ڡ�
	ERROR_NOT_EXIST_PHASE_TURN_SHCEME = 29,			//����������������ڡ�
	ERROR_NOT_EXIST_GREEN_SIGNAL_RATION_SCHEME = 30,	//�����������űȱ����ڡ�
	ERROR_NOT_EXIST_SCHEME_ACTION = 31,				//�������з��������ڡ�
	ERROR_NOT_EXIST_ACTION_TIMEINTERVAL = 32,		//ʱ�α��ж��������ڡ�
	ERROR_NOT_EXIST_TIMEINTERVAL_SCHEDULE = 33,		//���ȱ���ʱ�α����ڡ�
	ERROR_NOT_EXIST_MOTHER_PHASE_FOLLOW_PHASE = 34,	//�������ĸ��λ�����ڡ�
	ERROR_NOT_EXIST_CONCURRENT_PHASE_PHASE_2 = 35,	//��λ��������λ�����ڲ�����λ��
	ERROR_NOT_EXIST_PHASE_TURN_PHASE_2 = 36,			//��λ��������λ��������������С�
	ERROR_NOT_EXIST_PHASE_GREEN_SIGNAL_RATION = 37,	//��λ��������λ�����������űȱ��С�
	
	ERROR_REPEATE_CONCURRENT_PHASE = 38,			//ĳ��λ�Ĳ�����λ�������ظ�ֵ��
	ERROR_REPEATE_PHASE_TURN = 39,					//ĳ�����������������ظ�ֵ��
	ERROR_REPEATE_FOLLOW_PHASE = 40,				//������λ��ĸ�����λ�������ظ�ֵ��
	ERROR_REPEATE_SCHEDULE = 41,						//���ȱ������ظ����ڵ���
	
	ERROR_NOT_CONFIG_INFORMATION = 42,				//������ϢΪ��
	
	//
	ERROR_ID_LEGAL_SCHEDULE = 43,					//���ȱ�ID���Ϸ�����Χ������[0,108]
	ERROR_ID_LEGAL_INTERVAL = 44,					//ʱ�α�ID���Ϸ�����Χ������[0,16]
	ERROR_ID_LEGAL_INTERVAL_TIME = 45,				//ʱ�α��е�ʱ��ID���Ϸ�����Χ������[0,48]
	ERROR_ID_LEGAL_ACTION = 46,						//������ID���Ϸ�����Χ������[0,255]
	ERROR_ID_LEGAL_SCHEME = 47,						//������ID���Ϸ�����Χ������[0,108]
	ERROR_ID_LEGAL_PHASE = 48,						//��λ��ID���Ϸ�����Χ������[0,16]
	ERROR_ID_LEGAL_SPLIT = 49,						//���ű�ID���Ϸ�����Χ������[0,36]
	ERROR_ID_LEGAL_PHASE_TURN = 50,					//�����ID���Ϸ�����Χ������[0,16]
	ERROR_ID_LEGAL_PHASE_TURN_ID = 51,				//�����Ļ��Ų��Ϸ�����Χ������[0,4]
	ERROR_ID_LEGAL_FOLLOW_PHASE = 52,				//������λID���Ϸ�����Χ������[0,16]
	ERROR_ID_LEGAL_CHANNEL = 53,					//ͨ����ID���Ϸ�����Χ������[0,16]
	//
	ERROR_PHASE_DISABLE = 54						//��λδʹ��
}
TSC_Para_Error_Num;

typedef enum {
	InitSetting_OTHER = 1,
    InitSetting_RED,//���
    InitSetting_PEDESTRIAN_CLEAR,//�������
    InitSetting_GREEN,//�̵�    
    InitSetting_YELLOW,//�Ƶ�    
    InitSetting_ALL_RED,//ȫ��    
    
}InitSetting;//��ʼ������

typedef enum {

    UNDEFINEED = 0,//δ����
    OTHER_TYPE,//��������
    NONE,//��
    MIN_MOTO,//��С������Ӧ    
    MAX_MOTO,//�������Ӧ
    PEDESTRIAN_RES,//������Ӧ
    PEDESTRIAN_QEQ,//�����/��������
    IGNORE_PHASE//������λ
}GreenSignalRationType;//���ű�ģʽ

typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//�ƻ�ʱ��

typedef enum {

    OTHER = 1,//��������
    MOTOR,//����������
    PEDESTRIAN,//��������
    FOLLOW,//��������
}ControllerType;//����Դ������

typedef enum {

    ALTERNATE = 0,//����
    REDLIGHT,//����
    YELLOWLIGHT//����

}FlashLightType;//����ģʽ


//��λ����
typedef struct {

    UInt8 nPhaseID;                          /*2.2.2.1 ��λ�ţ����ܳ���maxPhases�������ֵ��(1..32)*/
    UInt8  nPedestrianPassTime;              /*2.2.2.2���˷���ʱ�䣬�������˹����̵Ƶ�������(0..255),second*/
    UInt8  nPedestrianClearTime;             /* 2.2.2.3 �������ʱ�䣨��������ʱ�䣩(0..255)*/
    UInt8  nMinGreen;                        /* 2.2.2.4��С�̵�ʱ�䣬�����С�̡�һ�������ڼ������ͣ���߼�ĳ����Ŷ����ȷ����(0..255)*/
    UInt8  nUnitExtendGreen;                 /*2.2.2.5��λ�̵��ӳ�ʱ��(0-25.5 sec)����Ƶ�λ�ӳ��̡�(0..255)����λΪ1/10��*/
    UInt8  nMaxGreen_1;                      /* 2.2.2.6����̵�ʱ��1����������1��(0..255)*/
    UInt8  nMaxGreen_2;                      /* 2.2.2.7����̵�ʱ��2����������2��(0..255)*/
    UInt8  nYellowTime;                      /*2.2.2.8��λ�Ƶ�ʱ�䣬��λΪ1/10��*/        
    UInt8  nAllRedTime;			            /*2.2.2.9 NTCIP������λ������ʱ��,Ҳ��������ͨ����˵��ȫ��ʱ�䣬��λΪ1/10��*/
    UInt8  nRedProtectedTime;                /*2.2.2.10��֤�Ƶ��Ժ���ֱ�ӷ����̵ƿ��Ƶĺ��ʱ�䱣��*/
	UInt8  byPhaseAddedInitial;		        /*2.2.2.11��λ���ӳ�ʼֵ*/
	UInt8  byPhaseMaximumInitial;	        /*2.2.2.12��λ����ʼֵ*/
	UInt8  byPhaseTimeBeforeReduction;       /*2.2.2.13 gap�ݼ�֮ǰ��ʱ����2.2.2.14�ۺ�ʹ��*/
	UInt8  byPhaseCarsBeforeReduction;       /*2.2.2.14 gap�ݼ�֮ǰͨ���ĳ�����*/
	UInt8  byPhaseTimeToReduce;		        /*2.2.2.15 gap�ݼ���minigap��ʱ��*/
	UInt8  byPhaseReduceBy;			        /*2.2.2.16��λ�ݼ��ʣ���2.2.2.15��16��ѡһ��*/
	UInt8  byPhaseMinimumGap;		        /*2.2.2.17���Եݼ�������Сgap��Ӧ�ú�phaseTimeToReduce�ۺ�ʹ��*/
	UInt8  byPhaseDynamicMaxLimit;	        /*2.2.2.18����MAX���޶�ֵ����С�ڴ�ֵʱ������MAX1����֮����MAX2*/
	UInt8  byPhaseDynamicMaxStep;	        /*2.2.2.19��̬��������*/    
	UInt8  byPhaseStartup;			        /*2.2.2.20��λ��ʼ������
					other(1)��λ��ʹ�ܣ��Ƕ��壩��־λ������phaseOption��bit0=0����phaseRing=0��
					phaseNotON(2)��λ��ʼΪ�죨�ǻ��
					greenWalk(3)��λ��ʼΪ��С�̺�����ʱ��
					greenNoWalk(4)��λ��ʼΪ��С�̵Ŀ�ʼ
					yellowChange(5)��λ��ʼΪ�Ƶƿ�ʼ
					redClear(6)��λ��ʼ��Ϊ��ƿ�ʼ*/

	UInt16  wPhaseOptions;			        /*2.2.2.21��λѡ��
					Bit 0 - Enabled Phase��λʹ��
					Bit 1 �C �������Զ��������ʱ������Ϊ1ʱ����λ�����������������ǰ�Ƚ��к�Ʋ���
					Bit 2 �C ���������ʱ������Ϊ1ʱ�������д���λ��
					Bit 3 �C �Ǹ�Ӧ1
					Bit 4 -  �Ǹ�Ӧ2
					Bit 5 �C ����Ϊ0ʱ��������������λ�Ƶƿ�ʼ��¼������Ϊ1ʱ��������������������detectorOptions����
					Bit 6 - Min Vehicle Recall��Ϊ����Ϊ��С������
					Bit 7 - Max Vehicle Recall��Ϊ����Ϊ���������
					Bit 8 - Actuated Rest In Walk��1�����Ӧ��λ�ڳ�ͻ��û������ʱ�������˷���
					Bit 9 - Soft Vehicle Recall��������λû��ʵ����������еĳ�ͻ��λ��MAX RECALLʱ������Ҳû������פ�����̵�ʱ��������λһ��soft recall��ʹ��λת��������λ��
					Bit 10 - Dual Entry Phase������Ϊ1ʱʹһ��û�м�����������λ������һ������ͬʱ���е���λһ����С�
					Bit 11 - Simultaneous Gap Disable������Ϊ1ʹ������Gap����
					Bit 12 - Guaranteed Passage������Ϊ1ʱ��֤��Ӧ��λ���ȫ�̣�passage��
					Bit 13 - Ped Recall��Ϊ����Ϊ��������
					Bit 14 - Conditional Service Enable����λʱ��û���꣬��ʣ���ʱ�����ͬһ��barrier����λ��
					Bit 15 - Added Initial Calculation������Ϊ1ʱ��ѡ�����������ֵ������Ϊ0ʱ��ѡ�������ļӺ͡�*/
    UInt8 nCircleID;                     /*2.2.2.22�õ�����λ��ring���*/
    UInt8  byPhaseConcurrency[NUM_PHASE];		/*2.2.2.23���Բ�������λ��ÿ���ֽڴ���һ���ɲ�������λ��*/

}PhaseItem,*PPhaseItem;


//�������������
struct STRU_N_VehicleDetector
{
	UInt8  byVehicleDetectorNumber;          /*2.3.2.1������������кš�(1..48)*/
	UInt8  byVehicleDetectorOptions;	        /*2.3.2.2���������ѡ��
			Bit 0 �C ���������
			Bit 1 �C ռ���ʼ����
			Bit 2 -  ������λ�ڻƵ�ʱ���¼������
			Bit 3 -  ������λ�ں��ʱ���¼������
			Bit 4 �C��Ӧ��λ���ӵ�λ�ӳ��̣�passage��
			Bit 5 �CAddedInitial�����
			Bit 6 �C �ŶӼ����
			Bit 7 �C ��������*/
	UInt8  byVehicleDetectorCallPhase;       /*2.3.2.3�ü������Ӧ��������λ*/
	UInt8  byVehicleDetectorSwitchPhase;     /*2.3.2.4�Ǹ���λ��,����λ�ɽ��ոó�������������󣬵�assigned phase Ϊ��ƻ�Ƶ�ʱ����the Switch Phaseʱ�̵�ʱ������λ��ת��*/
	UInt16  byVehicleDetectorDelay;	        /*2.3.2.5��assigned phase�����̵�ʱ������������뽫���ӳ�һ��ʱ�䣨00-999�룩��һ���������ӳ٣�����֮�������ӳٽ����ۼ�*/
	UInt8  byVehicleDetectorExtend;          /*2.3.2.6��assigned phase���̵�ʱ���������ÿ�����룬��λ�������̵Ƶ���ֹ�㱻�ӳ�һ��ʱ�䣨00-999�룩*/
	UInt8  byVehicleDetectorQueueLimit;      /*2.3.2.7������Ŷӳ������ƣ���������һ����ʱ������ĳ���������Ч*/
	UInt8  byVehicleDetectorNoActivity;      /*2.3.2.8 0-255���ӣ������ָ����ʱ���м����û�з�����Ӧ��Ϣ���ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	UInt8  byVehicleDetectorMaxPresence;     /*2.3.2.9 0-255���ӣ������ʱ���ڣ����������������Ӧ��Ϣ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	UInt8  byVehicleDetectorErraticCounts;	/*2.3.2.10ÿ����0-255�Σ��������������ĸ�Ӧ��Ϣ��Ƶ�ʳ������ֵ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	UInt8  byVehicleDetectorFailTime;        /*2.3.2.11 �����ʧ��ʱ�䣬��λ����*/
	UInt8  byVehicleDetectorAlarms;	        /*2.3.2.12 ������澯
					Bit 7: Other Fault �C ��������
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Configuration Fault �C ���õļ����û��ʹ�û��һ�������ڵ���λ��ϵ
					Bit 3: Communications Fault �C�����ͨ�Ŵ���
					Bit 2: Erratic Output Fault �C �������������(��������)
					Bit 1: Max Presence Fault �C �����һֱ�г�
					Bit 0��No Activity Fault - �����һֱû��*/
	UInt8  byVehicleDetectorReportedAlarms; /*2.3.2.12���������
					Bit 7: Reserved.
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Excessive Change Fault - ������������ࡣ
					Bit 3: Shorted Loop Fault - ������ջ���
					Bit 2: Open Loop Fault �C ���������
					Bit 1: Watchdog Fault -  watchdog ����
					Bit 0: Other �C ��������*/
	UInt8  byVehicleDetectorReset;	/*2.3.2.13 ���ö�������Ϊ����ʱ��������cu����������������cu������������֮��ö����Զ�����Ϊ0*/
};		


//���˼��������
struct STRU_N_PedestrianDetector
{
	UInt8  byPedestrianDetectorNumber;		/*2.3.7.1���˼�����к�*/
	UInt8  byPedestrianDetectorCallPhase;	/*2.3.7.2���˼������Ӧ��������λ*/
	UInt8  byPedestrianDetectorNoActivity;	/*2.3.7.3 0-255���ӣ������ָ����ʱ�������˼����û�з�����Ӧ��Ϣ���ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	UInt8  byPedestrianDetectorMaxPresence;	/*2.3.7.4  0-255���ӣ������ʱ���ڣ����˼��������������Ӧ��Ϣ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	UInt8  byPedestrianDetectorErraticCounts;/*2.3.7.5 ÿ����0-255�Σ�������˼���������ĸ�Ӧ��Ϣ��Ƶ�ʳ������ֵ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	//UInt8  byPedestrianDetectorAlarms;		/*2.3.7.6 ���˼����������Ϣ��
	//		Bit 7: Other Fault �C ��������
	//		Bit 6: Reserved.
	//		Bit 5: Reserved.
	//		Bit 4: Configuration Fault �C ���ô���
	//		Bit 3: Communications Fault �C ͨ�Ŵ���
	//		Bit 2: Erratic Output Fault �C ��������
	//		Bit 1: Max Presence Fault �C �����ֳ�
	//		Bit 0: No Activity Fault �C ����û��*/
};

//���ȱ�
typedef struct {
    UInt16  nScheduleID;                     /*2.4.3.2.1 ���ȼƻ��ţ���timeBaseScheduleMonth��timeBaseScheduleDate��timeBaseScheduleDate��timeBaseScheduleDayPlan�ĸ�������ͬ�����ƻ��Ƿ����ִ�С�(1..40)*/
	UInt16  month;	                        /*2.4.3.2.2 bit1-bit12��ÿλ��ʾһ���¡���1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..65535)*/
	UInt8   week;		                    /*2.4.3.2.3 bit1-bit7��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..255)*/
	UInt32  day;		                        /* 2.4.3.2.4 bit1-bit31��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..4294967295)*/
    UInt8   nTimeIntervalID;                 /*2.4.3.2.5ʱ�α�ţ�ָ��timeBaseScheduleDayPlan��0��ʾ������Ч��(0..255)*/
}PlanScheduleItem,*PPlanScheduleItem;

//ʱ�ζ���
typedef struct {

    UInt8   nTimeIntervalID;                 /*ʱ�α��(1..16)������*/
    UInt8   nTimeID;                         /*ʱ�Σ��¼����š�(1..48)������,������ͬ���¼�������һ���еĲ�ͬʱ��ִ�С���������¼����ֵ�ʱ����ͬ����ʱ�κ�С����ִ��*/
    UInt8   cStartTimeHour;                  /*��ʼִ��ʱ�̵�����������ʱ�䣨24ʱ�ƣ���*/
    UInt8   cStartTimeMinute;                /*��ʼִ�е�������*/
    UInt8   nActionID;                       /*��ʱ������*/
}TimeIntervalItem,*PTimeIntervalItem;
//��Ԫ��������
typedef struct {

	UInt8 	nBootYellowLightTime;					// 2.4.1 ����ʱ���������ʱ��(��)������ʱ����������ڵ���ָ�����֡�����ָ����������Щ������豸���塣�����ڼ䣬Ӳ���������źŵƼ����ǲ���ģ�����еĻ�����
	UInt8 	cIsPedestrianAutoClear;	                // 2.4.2�����Զ���ղ�����1 = False/Disable 2=True/Enable����	������Ϊ1�����ֶ�������Чʱ���źŻ�����������Զ����ʱ�䣬�Է�ֹ�������ʱ�䱻Ԥ�����õ�ʱ����ֹ��
	UInt16	wUnitBackupTime;						// 2.4.3�źŻ����ߺ󵽽���ǰ��ʱ�䡣
	UInt8	byUnitRedRevert;						// 2.4.4��С���ʱ�䡣�˲���Ϊ���е���λ�ṩ��С���ʱ�䣨�磺�����ֵ����һ����λ�ĺ��ʱ�䣬�������λ�ĺ��ʱ����������������棩���������Ϊ�Ƶ�֮�����һ���̵�֮ǰ�ṩ���ʱ���ṩһ����Сָʾ��
	UInt8	byUnitControl;							// .10 ����Զ�̿���ʵ�弤���źŻ���ĳЩ����( 0 = False / Disabled, 1 = True / Enabled)��
	//Bit 7: �Ҷ�ʹ�ܡ�����Ϊ1ʱ����ʾ����ͨ���ҶȲ�����Ϊ��ʵ��������ܣ�timebaseAscAuxillaryFunction������������Ϊtrue��
	//Bit 6: ����Э�� - ����Ϊ1ʱ����ʾ��Ϊ����Э���ķ������
	//Bit 5��Walk Rest Modifier - ������Ϊ1ʱ�������ͻ��λû�з����������Ӧ��λͣ���ڷ���״̬
	//Bit 4��Call to Non-Actuated 2 - ������Ϊ1ʱ��ʹ��phaseOptions�ֶ�������Non-Actuated 1����λ�����ڷǸ�Ӧ״̬��
	//Bit3: Call to Non-Actuated 1 - ������Ϊ1ʱ��ʹ��phaseOptions�ֶ�������Non-Actuated 2����λ�����ڷǸ�Ӧ״̬��
	//Bit2:External Minimum Recall -������Ϊ1ʱ��ʹ������λ��������С����״̬
	//Bit 1��0: Reserved��
	UInt8  	byCoordOperationalMode;                 // 2.5.1Э���Ĳ�����ʽAutomatic��0�����Զ���ʽ������ΪЭ������Ӧ������ȿ���
	//ManualPattern��1~253���������ֶ��趨�ķ���
	//ManualFree��254������Э���Զ���Ӧ
	//ManualFlash(255)����Э���Զ�����
	UInt8	byCoordCorrectionMode;                  // 2.5.2Э����ʽ
	//other(1)Э��������һ��û���ڱ������ж�����µ���λ��
	//dwell(2)Э��ͨ��פ��Э����λ�����仯�ﵽ��λ��
	//shortway(3)Э��ͨ��ĳ���������ڱ仯�������ٺ�����ʱ��ﵽ��λ���ƽ������
	//addOnly(4)Э��ͨ��ĳ���������ڱ仯��ϰ��������ʱ�����ﵽ��λ��
	UInt8	byCoordMaximumMode;	                    // 2.5.3 Э�������ķ�ʽ
	//other(1)�����ڴ��������δ֪��ʽ
	//maximum1(2)��Max1��Ч��Э��
	//maximum2(3)��Max2��Ч��Э��
	//maxinhibit(4)��������Э��ģʽʱ����ֹ���������
	UInt8	byCoordForceMode;		                // 2.5.4 Patternǿ��ģʽ
	//other(1)���źŻ�ʹ���ڴ�û�ж����ģʽ
	//floating(2)��ÿ����λ�����ǿ�Ƶ��̵�ʱ�䣬�����õ���λʱ��ת����Э����λ
	//fixed(3)��ÿ����λ��ǿ�Ƶ����ڹ̶�λ�ã����õ���λʱ��ӵ�����������λ��

	UInt8	byFlashFrequency;			            //����Ƶ��(UInt8)			
	UInt8	byThroughStreetTimeGap;		            //���ι���ʱ��(UInt8)
	UInt8	byFluxCollectCycle;			            //�����ɼ�����(UInt8)
	UInt8    bySecondTimeDiff;			            //���ι�������Э������
	UInt8	nBootAllRedTime;			            //����ȫ��ʱ��
	UInt8	byCollectCycleUnit;			            //�����ɼ���λ���� / ���� ( 0/1)
	UInt8	byUseStartOrder;			            //������������
	UInt8	byCommOutTime;				            //ͨ�ų�ʱʱ��
	UInt16	wSpeedCoef;					            //�ٶȼ�������
	
	//�Զ��岿��                         
	UInt8  acIpAddr[4];                     //IP��ַ
	UInt32          SubNetMask;                      //��������
	UInt8  acGatwayIp[4];                   //����
	UInt8           byPort;                          //�˿ں�

	UInt8	byTransCycle;				            //ƽ����������
	UInt8	byOption;		                        //ѡ���������λȡֵ
	//BIT 7---------��ѹ������
	//BIT 6---------����
	//BIT 5---------����
	//BIT 4---------����
	//BIT 3---------����
	//BIT 2---------��һ��������
	//BIT 1---------�����������
	//BIT 0---------���ð�����
	UInt8    byUnitTransIntensityCalCo;              //��ͨǿ�ȼ���ϵ��
}UnitPara,*PUnitPara;



//Э����������
struct STRU_N_CoordinationVariable
{
	UInt8	byCoordOperationalMode;		/*2.5.1Э���Ĳ�����ʽ
					Automatic��0�����Զ���ʽ������ΪЭ������Ӧ������ȿ���
					ManualPattern��1~253���������ֶ��趨�ķ���
					ManualFree��254������Э���Զ���Ӧ
					ManualFlash(255)����Э���Զ�����*/
	UInt8	byCoordCorrectionMode;	/*2.5.2Э����ʽ
					other(1)Э��������һ��û���ڱ������ж�����µ���λ��
					dwell(2)Э��ͨ��פ��Э����λ�����仯�ﵽ��λ��
					shortway(3)Э��ͨ��ĳ���������ڱ仯�������ٺ�����ʱ��ﵽ��λ���ƽ������
					addOnly(4)Э��ͨ��ĳ���������ڱ仯��ϰ��������ʱ�����ﵽ��λ��*/
	UInt8	byCoordMaximumMode;		/*2.5.3 Э�������ķ�ʽ
					other(1)�����ڴ��������δ֪��ʽ
					maximum1(2)��Max1��Ч��Э��
					maximum2(3)��Max2��Ч��Э��
					maxinhibit(4)��������Э��ģʽʱ����ֹ���������*/
	UInt8	byCoordForceMode;			/*2.5.4 Patternǿ��ģʽ
					other(1)���źŻ�ʹ���ڴ�û�ж����ģʽ
					floating(2)��ÿ����λ�����ǿ�Ƶ��̵�ʱ�䣬�����õ���λʱ��ת����Э����λ
					fixed(3)��ÿ����λ��ǿ�Ƶ����ڹ̶�λ�ã����õ���λʱ��ӵ�����������λ��*/
	UInt8	byPatternTableType;		/*2.5.6���巽��������Ҫ��������֯�ṹ
					other(1)���˴�û�ж����
					patterns(2)���������ÿһ�д���Ψһ��һ�����������Ҳ�����������
					offset3(3)��ÿ��������3����λ�ռ�������3��
					offset5(4)��ÿ��������5����λ�ռ5��*/
	UInt8	byCoordPatternStatus;		/*2.5.10 Э������״̬
				Not used��0��
				Pattern -��1-253����ǰ���з�����
				Free - (254)��Ӧ
				Flash - (255)����*/
	UInt8	byLocalFreeStatus;		/*2.5.11 Free����״̬
				other: ����״̬
				notFree: û�н���free����
				commandFree: 
				transitionFree: ����free��������Э��
				inputFree: �źŻ����뵼��free������ӦЭ��
				coordFree: the CU programming for the called pattern is to run Free.
				badPlan: ���õķ������Ϸ����Խ���free
				badCycleTime: ���ڲ��Ϸ�����������С�������Խ���free
				splitOverrun: ʱ��Խ��free
				invalidOffset: ��������
				failed: ������ϵ���free*/
	UInt16	wCoordCycleStatus;		/* 2.5.12 Э������������״̬��0-510sec���������ڳ�һֱ���ٵ�0*/
	UInt16 	wCoordSyncStatus;			/*2.5.13Э��ͬ��״̬��0��510����Э����׼�㵽Ŀǰ���ڵ�ʱ�䣬��0��¼���¸����ڻ�׼�㡣���Գ������ڳ�*/
	UInt8 	bySystemPatternControl;	/*2.5.14ϵͳ��������
				Standby(0)ϵͳ��������
				Pattern(1-253)ϵͳ���Ʒ�����
				Free(254)call free 
				Flash(255)�Զ�Flash */
	UInt8	bySystemSyncControl;		/*2.5.14 ����ϵͳͬ����׼��*/
};


//�Ϸ������壬���ڳ�����λ��1���ֽ�̫С
typedef struct {

    UInt8 nSchemeID;                         /*2.5.7.1 �������и��������ĸ�����*/
    UInt8 nCycleTime;                        /*2.5.7.2 �������ڳ�,һ���޸�Ӧ��������λ��С���������miniGreen��Walk��PedClear��yellow��Red һ����Ӧ��������λ��С���������miniGreen+Yellow+Red */
    UInt8 nOffset;                           /*2.5.7.3 ��λ���С*/
    UInt8 nGreenSignalRatioID;               /*2.5.7.4������Ӧ�����űȱ��*/
    UInt8 nPhaseTurnID;                      /*2.5.7.5������Ӧ��������*/
}SchemeItemOld,*PSchemeItemOld;

//�·������壬���ڳ�����λ��Ϊ2���ֽ�
typedef struct {

    UInt8 nSchemeID;                         /*2.5.7.1 �������и��������ĸ�����*/
    UInt16 nCycleTime;                        /*2.5.7.2 �������ڳ�,һ���޸�Ӧ��������λ��С���������miniGreen��Walk��PedClear��yellow��Red һ����Ӧ��������λ��С���������miniGreen+Yellow+Red */
    UInt16 nOffset;                           /*2.5.7.3 ��λ���С*/
    UInt8 nGreenSignalRatioID;               /*2.5.7.4������Ӧ�����űȱ��*/
    UInt8 nPhaseTurnID;                      /*2.5.7.5������Ӧ��������*/
}SchemeItem,*PSchemeItem;

//���űȶ���
typedef struct {

    UInt8 nGreenSignalRationID;              /* 2.5.9.1�������ű���ţ�һ�����ڵĸ��ֶ���ͬ*/
    UInt8 nPhaseID;                          /*2.5.9.2���ж�Ӧ����λ��*/
    UInt8 nGreenSignalRationTime;            /*2.5.9.3��Ӧ���ű���λ��ʱ��*/
    UInt8 nType;                             /*2.5.9.4���ڸ���λӦ��β���
            			other(1)�����ٴ˴������ģʽ
            			none(2)��û�����ű�ģʽ����
            			minimumVehicleRecall(3)��minimumVehRecall����
            			maximumVehicleRecall(4)��maximumVehRecall����
            			pedestrianRecall(5)��pedestrianRecall����
            			maximumVehicleAndPedestrainRecall(6): maximumVeh&PedRecall����
            			phaseOmitted(7)����λ������*/
    
    UInt8 nIsCoordinate;                     /*2.5.9.5�����Ƿ���ΪЭ����λ����*/

}GreenSignalRationItem,*PGreenSignalRationItem;

//ʱ����������
struct STRU_N_TimeBaseVariable
{
	UInt16	wTimebaseAscPatternSync;	    /*2.6.1�ھ�����ҹ�ܶ�ʱ���ڵķ���ͬ���ο�������Ϊ0XFFFFʱ���źſ��Ƶ�Ԫ����ACTION TIME��Ϊ������ͬ���ο���*/
	UInt8	byTimebaseAscActionStatus;      /*2.6.4 ������������ǰ�õ���ʱ����š�*/
	UInt32	dwGlobalTime;				    /*2.4.1 UTC����GMT��ʱ�䣬��1970/1/1 00:00:00���������*/
	UInt8	byGlobalDaylightSaving;	        /*2.4.2����ʱ
                        				other (1)��DST�������û���ڱ���׼�С�
                        				disableDST (2)����ʹ��DST
                        				enableUSDST (3)��DSTʹ������ϰ��
                        				enableEuropeDST (4)��DSTʹ��ŷ��ϰ��
                        				enableAustraliaDST (5)��DSTʹ�ðĴ�����ϰ��
                        				enableTasmaniaDST (6) DSTʹ����˹������ϰ��*/
	UInt8  	byDayPlanStatus;			    /*2��4��4��4��ʾ�ʱ�α�ı�š�0��ʾû�л��ʱ�α�*/
	UInt32 	nGlobalLocalTimeDifferential;	/*2��4��5 ʱ��*/
	UInt32  	nGcontrollerStandardTimeZone;	/*2��4��6 ���ر�׼ʱ����GMT��ʱ��룩����ֵ��ʾ����ʱ���ڶ����򣬸�ֵ��ʾ����ʱ����������*/
	UInt32	dwControllerLocalTime;	        /*2��4��7 ����ʱ�䣬����1970/1/1 00:00:00����������*/
};

//ʱ����������
typedef struct {

    UInt8  nActionID;                        /* 2.6.3.1������*/                        
    UInt8  nSchemeID;                        /* 2.6.3.2��Ӧ�����š�����������ܳ���maxPatterns, flash,����free��ֵ������Ϊ0����û�з�����ѡ��*/
    UInt8  nPhaseTableID;                       /* 2.6.3.3�������Ӧ����λ���*/
    UInt8  nChannelTableID;                     /* 2.6.3.4�������Ӧ��ͨ����*/
}ActionItem,*PActionItem;


//�������
typedef struct {

    UInt8 nPhaseTurnID;                      /*2.8.3.1������*/
    UInt8 nCircleID;                         /* 2.8.3.2����*/
    UInt8 nTurnArray[32];             /*2.8.3.3 ������*/

}PhaseTurnItem,*PPhaseTurnItem;



//ͨ������
typedef struct {

    UInt8 nChannelID;                        /*2.9.2.1ͨ���ţ����ܴ���maxChannels��(1..32)*/
    UInt8 nControllerID;                     /*ͨ������Դ[��λ(phase)�����ص�(overlap)]����channelControlType��������λ�����ص������ܴ��������λ��������ص�����*/
    UInt8 nControllerType;                   /*2.9.2.2ͨ���������ͣ����ĸ�ȡֵ��other(1),phaseVehicle (2),phasePedestrian (3),overlap (4)���ֱ��ʾ������λ���ơ���������λ���ơ�������λ���ƺ��ص����ơ�*/
    UInt8 nFlashLightType;                   /*2.9.2.3�Զ�����״̬��
                                        	Bit 7: Reserved
                                        	Bit 6: Reserved
                                			Bit 5: Reserved
                                			Bit 4: Reserved
                                			Bit 3: ��������
                                				Bit=0: Off/Disabled & Bit=1: On/Enabled
                                			Bit 2: ����
                                				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
                                			Bit 1: ����
                                				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
                                			Bit 0: Reserved
                                        	Bit 1 �� Bit 2 ͬʱΪ1��Ч����Bit 1 = 0��Bit 2 = 1��Reservedλ����Ϊ0�����򷵻�badValue(3)����*/
	UInt8  	byChannelDim;		            /*2.9.2.4ͨ���Ҷ�״̬
                                			Bit 7: Reserved
                                			Bit 6: Reserved
                                			Bit 5: Reserved
                                			Bit 4: Reserved
                                			Bit 3: Dim Alternate Half Line Cycle
                                				Bit=0: Off/+ half cycle &
                                				Bit=1: On/- half cycle
                                			Bit 2: Dim Red
                                				Bit=0: Off/Red Not Dimmed &
                                				Bit=1: On/Dimmed Red
                                			Bit 1: Dim Yellow
                                				Bit=0: Off / Yellow Not Dimmed &
                                				Bit=1: On / Dimmed Yellow
                                			Bit 0: Dim Green
                                				Bit=0: Off / Green Not Dimmed &
                                				Bit=1: On / Dimmed Green*/

}ChannelItem,*PChannelItem;

typedef struct {

    UInt8 nFollowPhaseID;                    /*2.10.2.1 overlapNumber��overlap�ţ�	������maxOverlaps��1 = Overlap A, 2 = Overlap B etc */
    UInt8 byOverlapType;                     /*2.10.2.2 overlap�������ͣ�ö�����£�
		other(1) ��δ�ڴ������Ĳ������͡�
		normal(2)�����ֲ�������ʱ��overlap�������overlapIncludedPhases�������ơ�����������ʱoverlap����̵ƣ�
		��overlap��������λ���̵�ʱ��
		��overlap��������λ�ǻƵƣ�����ȫ��red clearance����overlap������һ��λ��included phase is next��ʱ��
		���overlap��������λ�ǻƵ���overlap��������һ��λ��included phase is not next����overlap����Ƶơ����overlap���̵ƺͻƵ���Ч���������ơ�
		minusGreenYellow(3)�����ֲ�������ʱ��overlap�������overlapIncludedPhases��overlapModifierPhases�������ơ�����������ʱoverlap����̵ƣ�
		��overlap������λ���̵���overlap��������λ�����̵�ʱ��NOT green��
		��overlap��������λ�ǻƵƣ�����ȫ��red clearance����overlap������һ��λ��included phase is next����overlap��������λ�����̵�ʱ��
		���overlap��������λ�ǻƵ���overlap��������λ���ǻƵ���overlap��������һ��λ��included phase is not next����overlap����Ƶơ����overlap���̵ƺͻƵ���Ч���������ơ�*/
    UInt8 nArrayMotherPhase[NUM_PHASE];/*2.10.2.3 overlap��������λ��ÿ�ֽڱ�ʾһ����λ�š�*/
    UInt8 byArrOverlapModifierPhases[MAX_PHASE_COUNT_MO_OVERLAP];	/*2.10.2.4 overlap��������λ��ÿ�ֽڱ�ʾһ����λ�š�	���Ϊ��ֵ��null����overlapTypeΪnormal�����Ϊ�ǿ�ֵ��non-null����overlapTypeΪminusGreenYellow��*/
    UInt8 nGreenTime;                        /*2.10.2.5  0-255�룬	����˲�������0��overlap���̵�����������overlap�̵ƽ��ӳ��˲����趨��������*/
    UInt8 nYellowTime;                       /*2.10.2.6  3-25.5�롣���overlap���̵Ʊ��ӳ�	��Trailing Green�����㣩���˲���������overlap Yellow Change��ʱ�������ȡ�*/
    UInt8 nRedTime;                          /*2.10.2.7  0-25.5�롣���overlap���̵Ʊ��ӳ�	��Trailing Green�����˲���������overlap Red Clearance��ʱ�������ȡ�*/
    
}FollowPhaseItem,*PFollowPhaseItem;


//�������ö���
struct STRU_N_Preempt
{
	UInt8	byPreemptNumber;
	UInt8	byPreemptControl;
	UInt8	byPreemptLink;
	UInt16	wPreemptDelay;
	UInt16	wPreemptMinimumDuration;
	UInt8	byPreemptMinimumGreen;
	UInt8	byPreemptMinimumWalk;
	UInt8	byPreemptEnterPedClear;
	UInt8 	byPreemptTrackGreen;
	UInt8	byPreemptDwellGreen;
	UInt16	wPreemptMaximumPresence;
	UInt8	abyPreemptTrackPhase[NUM_PHASE];
	UInt8	byPreemptTrackPhaseLen;
	UInt8	abyPreemptDwellPhase[NUM_PHASE];
	UInt8	byPreemptDwellPhaseLen;
	UInt8	abyPreemptDwellPed[MAX_PEDESTRIANPHASE_COUNT];
	UInt8	byPreemptDwellPedLen;
	UInt8	abyPreemptExitPhase[NUM_PHASE];
	UInt8	byPreemptExitPhaseLen;
	UInt8	byPreemptState;
	UInt8	abyPreemptTrackOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptTrackOverlapLen;
	UInt8	abyPreemptDwellOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptDwellOverlapLen;
	UInt8 	abyPreemptCyclingPhase[NUM_PHASE];
	UInt8	byPreemptCyclingPhaseLen;
	UInt8	abyPreemptCyclingPed[MAX_PEDESTRIANPHASE_COUNT];
	UInt8	byPreemptCyclingPedLen;
	UInt8	abyPreemptCyclingOverlap[MAX_OVERLAP_COUNT];
	UInt8	byPreemptCyclingOverlapLen;
	UInt8	byPreemptEnterYellowChange;
	UInt8 	byPreemptEnterRedClear;
	UInt8	byPreemptTrackYellowChange;
	UInt8	byPreemptTrackRedClear;
};

/*==================================================================*/
/*��Ϣ����:STRU_DynPatternSel										*/
/*˵    ��:��̬����ѡ�����ñ�ṹ����								*/
/*==================================================================*/
struct STRU_DynPatternSel
{
	UInt8  	byDynSn;			//������
	UInt8	byDynUpperLimit;	//��ֵ
	UInt8	byDynPatternNo;		//������
};



//����ʱ�����ù�ϵ�ṹ������������������ 
struct STRU_N_CountDownBoard
{
	UInt8 byCountdownNo; //����ʱ�ƺ�
	UInt8 abyIncludedPhaseNo[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//��Ӧ��λ��
	UInt8 abyIncludedPhaseChannel[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//��λ��Ӧͨ���� 
	UInt8 abyIncludedPhaseType[MAX_COUNTDOWNBOARD_INCLUDEDPHASES];//ͨ����Ӧ��λ���� 
	UInt8 byIncludedPhaseNum;//����ʱ�ƶ�Ӧ����λ�� 
};

/*==================================================================*/
/*��Ϣ����:STRU_SignalTransEntry									*/
/*˵    ��:�ź�ת�����нṹ����										*/
/*==================================================================*/
struct STRU_SignalTransEntry
{
	UInt8  	byPhaseNumber;		//��λ��
	UInt8	byRedYellow;		//�ź�ת�������еĺ��ʱ��
	UInt8	nGreenLightTime;		//�ź�ת�������е�����ʱ��
	UInt8	bySafeRed;			//�ź�ת�������еİ�ȫ���ʱ��
	UInt8	byGreenTime;		//��һ�����̵�
	UInt8	byAllStopPhase;		//�����ͬ�ϵ���λ��

	UInt8 	byPhaseDirectionProperty;		//��λ�ķ������ԣ���λ��־����
															//���򣺶��������ϡ��������������ϡ����ϡ�
															//�������ϱ���������������������������

	UInt8 	byPhaseLaneProperty;			//��λ�ķ��򳵵����ԣ���λ��ʾ����
															//��������ֱ���ҡ���ֱ����ֱ�ҡ�ֱ�ҡ�
															//��ͷ
	//UInt8 	byPhaseReservedPara;			//��λ�ı������ԣ��Ժ�ʹ��
};

//�¼�����һ����
struct STRU_N_EventConfig	//���������¼���־��������Ϣ���������Ĳ����豸�����м��Ӳ������¼�
{
	UInt16 wEventConfigID;				//�к�
	UInt8 byEventConfigClass;			//�¼���
	UInt8 byEventConfigMode;			//�¼�ģʽ����Ҫ����������������¼������úͲ���
	/*int 	   nEventConfigCompareValue;	//�Ƚ�ֵ
	int 	   nEventConfigCompareValue2;	//���ֵ��������ʱ�����ڣ�Ϊ0ʱ��ʾû�����ڣ��¼�������Ч
	int 	   anEventConfigCompareOID[128];//���бȽϵ�OID��ֵ����ֵ������һ��INTEGER���ͣ�����BADVALUE error
	UInt8  byEventConfigCompareOIDLen;
	int 	   anEventConfigLogOID[128];		//��Ҫ�����ֵ
	UInt8  byEventConfigLogOIDLen;
	*/
	UInt8 byEventConfigAction;			//�¼�������������¼���
};


//����ʱ��Ϣ����
struct SCountDownCfg_DEL		//����ʱ������������ʾ���ء������ʵȲ���,����ʹ��
{
	UInt8	byDriverFlag;			//�źŻ���������ʱ�Ʊ�־
	UInt16	wRs232BaudRate;			//������
	UInt8  	byRs232DataBits;		//����λ											
	UInt8  	byRs232StopBits;		//ֹͣλ	
	UInt8  	byRs232ParityBits;		//��żУ��λ
};

struct STRU_ChannelRedLightCurrent
{
	UInt8  	byChannelNumber;			//ͨ����
	UInt8		byChannelRedCurrentBase;	//ͨ���ĺ�Ƶ�����׼ֵ
	UInt8		byChannelRedCurrentDelta;	//ͨ���ĺ�Ƶ�����ֵ
};

struct STRU_BasicFaultDetect
{
    UInt16 wGreenSumValue;         // �̵Ƶ����ܺ�
    UInt16 wGreenDiffValue;        // �̵Ʋ�ֵ
    UInt16 wOverflowVoltageCount;  // ��Ƿѹ����˲�������Ĭ��50
    UInt8 byRedOffCount;          // ���Ϩ�����˲�������Ĭ��4
    UInt8 byConflictCount;        // �̵Ƴ�ͻ����˲�������Ĭ��2
    UInt8 byManualKeyCount;       // �ֶ���尴������˲�������Ĭ��5
    UInt8 byTeleKeyCount;         // ң������������˲�������Ĭ��50
	UInt8 byRedFaultSwitch;	     // ��⿪��,bit0----���ϼ���ܿ��أ�Ĭ��Ϊ1
                                 //   bit1----������⿪�أ�Ĭ��Ϊ0
                                 //   bit2----��ѹ��⿪�أ�Ĭ��Ϊ0
                                 //   bit3----�������ϱ��������أ�Ĭ��Ϊ1��
                                 //           ���������Ϻ󱨾����������⣬����ֻ����
                                 //   bit4----��ѹ���ϱ��������أ�Ĭ��Ϊ1��
                                 //           ���������Ϻ󱨾����������⣬����ֻ����
                                 //   bit5----watchdog���ÿ��أ�Ĭ��Ϊ1������
};

struct STRU_FaultDetectSet
{
    struct STRU_ChannelRedLightCurrent AscChannelCurrentRedTable[NUM_CHANNEL];
    struct STRU_BasicFaultDetect       AscBasicFaultDetect;
};

struct STRU_Address
{
    UInt16 wSerialAddr;        //���ڵ�ַ
    UInt8  ucIpAddr[4];       //IP��ַ
    UInt32 SubNetMask;        //��������
    UInt8  ucGatwayIp[4];     //����
    UInt8  byPort;            //�˿ں�
    UInt8  ucMacAddr[6];      //MAC��ַ
    UInt8 reserved1;			//����Ϊ�����ֶ�
    UInt8 reserved2;
    UInt8 reserved3;
    UInt8 reserved4;
    UInt8 reserved5;
    UInt8 reserved6;
    UInt8 reserved7;
    UInt8 reserved8;
};
//�����űȱ��������˵�����
struct STRU_SplitPedEx
{
	UInt8 byWalk;		//���˷���ʱ��
	UInt8 byClear;		//�������ʱ��
	UInt8 byClearMode;	//�������ģʽ��1��ʾ������������ʾ����
	UInt8 byCriticalGreen;	//�ٽ��̵�
};
//����λ����Щ��ɫ������
struct STRU_PhaseEx
{
	UInt8 byGreenTime;	//��һ�����̵Ʒ���ʱ��
	UInt8 byReserved1;	//���²�������
	UInt8 byReserved2;
	UInt8 byReserved3;
	UInt8 byReserved4;
	UInt8 byReserved5;
	UInt8 byReserved6;
	UInt8 byReserved7;
};


//��ͨǿ��ѡ���� 
struct STRU_N_TransIntensityChoice
{
	UInt8 byIndexNumber;   /*80.62.1.1 ����*/
	UInt8 byKeyPhase;      /*80.62.1.2 �ؼ���λ��*/
	UInt8 byTransIntensityLower; /*80.62.1.3 ��ͨǿ������*/
	UInt8 byCycleTime;     /*80.62.1.4 ����*/
};
struct STRU_N_OverlapTableEx
{
	UInt8 byOverlapGreenFlash;/*80.61.1.1 */
};
//������Ԫ�Ż���������ʾ�ڵ�Ԫ�����У����Ӽ��������ֶΣ�4���ֽں�1���֣�
struct STRU_UnitOptiPara  
{	

	UInt8	bySysMaxCycle;		//������ڳ���0��255s�����Ż�ʹ�ã�ϵͳ�����㣩
	UInt8	bySysMinCycle;		//��С���ڳ���0��255s�����Ż�ʹ�ã�ϵͳ�����㣩
	UInt8	bySysMaxIntensity;	//���ͨǿ�ȣ�0��100��ʵ��ֵ�Ŵ�100����Ĭ��0.8��
	UInt8	bySysMinIntensity;		//��С��ͨǿ�ȣ�0��100��ʵ��ֵ�Ŵ�100����Ĭ��0.3��
	UInt8	bySysOptiPara_a;		//���ڼ��㹫ʽ����a��0��max��Ĭ��Ϊ1��
	UInt8	bySysOptiPara_b;		//���ڼ��㹫ʽ����b��0��max��Ĭ��Ϊ1��
	UInt8	byDecInternalTime;	//����ռ���ʲɼ����(1~60s)��Ĭ��Ϊ10s 
	UInt8	byDecDyFrequency;	//����ռ���ʲɼ�Ƶ��(1~255s)��Ĭ��Ϊ1s
//	UInt8	byReservedPara_1;		//��������1���˴ζ���ڵ㣬�����κζ��������ʾ
//	UInt8	byReservedPara_2;		//��������2���˴ζ���ڵ㣬�����κζ��������ʾ	
	UInt8	abyDegradePlanSeq[10];	//������������

};


//���������EX���� 
struct STRU_N_VehicleDetectorEx
{
	UInt8 byVehicleDetectorPasstime; /*80.60.1.1 ����ͨ����Ȧ��ͣ���߼��ʱ�� (0-255)*/
	UInt8 byVehicleDetectorStopNum;  /*80.60.1.2 ��Ȧ��ͣ���߼�ͣ�ų����� (0-255)*/
	UInt8 byVehicleDetectorPriority; /*80.60.1.3 ��������ȼ� (0-255)*/
	UInt8 byCorPhaseSatTimeOccupancy; /*80.60.1.4 ��Ӧ��λ����ʱ��ռ���� (0-100)*/ 
	UInt16 byCorPhaseSatVolume;        /*80.60.1.5 ��Ӧ��λ�������� (1500-1800)*/ 
	UInt8 byCongestionOccupancy;//ӵ�´���ռ���ʣ�Ĭ��100
	UInt8 	byVehicleDetectorOptionEx;//�����ѡ����λ��־���������Ϊӵ�¼����������λ������Ĭ��0
};
struct STRU_UnitParamEx
{
	UInt8 byStartAllRedTime;	//����ȫ��ʱ��
	UInt8 byCollectCycleMin;	//�����ɼ���λ���� / ����
	UInt8 byUseStartOrder;	//������������
	UInt8 byCommOutTime;		//ͨ�ų�ʱʱ��
	UInt16 wSpeedCoef;		//�ٶȼ�������
	UInt8 byTransCycle;		//ƽ����������
	UInt8 byOption;		    //ѡ���������λȡֵ
							//BIT 7---------��ѹ������
							//BIT 6---------����
							//BIT 5---------����
							//BIT 4---------����
							//BIT 3---------����
							//BIT 2---------��һ��������
							//BIT 1---------�����������
							//BIT 0---------���ð�����
	UInt8 byTransIntensityCalCo;		//��ͨǿ�ȼ���ϵ��
	UInt8 byReserved2;
	UInt8 byReserved3;
	UInt8 byReserved4;
	UInt8 byReserved5;
	UInt8 byReserved6;
	UInt8 byReserved7;
	UInt8 byReserved8;
	UInt8 byReserved9;
	UInt8 byReserved10;
	UInt8 byReserved11;
	UInt8 byReserved12;
	UInt8 byReserved13;
	UInt8 byReserved14;
};



typedef struct {
    PhaseItem					stOldPhase[NUM_PHASE];                                     //֮ǰδʹ�õ���λ��
    struct STRU_N_VehicleDetector		AscVehicleDetectorTable[MAX_VEHICLEDETECTOR_COUNT];     //�����������
    struct STRU_N_PedestrianDetector	AscPedestrianDetectorTable[MAX_PEDESTRIANDETECTOR_COUNT];//���˼������
    UnitPara 					stUnitPara;                                             //��Ԫ����

    struct STRU_N_CoordinationVariable	AscCoordinationVariable;                                //Э������
    SchemeItemOld					stOldScheme[NUM_SCHEME];                                   //������
    GreenSignalRationItem		stGreenSignalRation[NUM_GREEN_SIGNAL_RATION][NUM_PHASE];//���űȱ�
    struct STRU_N_TimeBaseVariable		AscTimeBaseVariable;                                    //ʱ������
    ActionItem					stAction[NUM_ACTION];                                   //������
    PlanScheduleItem			stPlanSchedule[NUM_SCHEDULE];                           //���ȱ�
    TimeIntervalItem			stTimeInterval[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];//ʱ�α�
    PhaseTurnItem				stPhaseTurn[NUM_PHASE_TURN][NUM_RING_COUNT];            //�����
    ChannelItem					stOldChannel[NUM_CHANNEL];                            //ͨ����
    FollowPhaseItem				stOldFollowPhase[NUM_FOLLOW_PHASE];                   //������λ
    //STRU_N_OverlapTable			AscOverlapTable[MAX_OVERLAP_COUNT];                  //�ص���
    struct STRU_N_Preempt				AscPreemptTable[MAX_PREEMPT_COUNT];                     //���ȱ�
    struct STRU_DynPatternSel          AscDynPatternSel[MAX_DYNPATTERNSEL_COUNT];              //��̬����ѡ���
    struct STRU_N_CountDownBoard       AscCountDownBoardCfg[MAX_COUNTDOWNBOARD_COUNT];         //����ʱ�Ʊ�

    struct STRU_SignalTransEntry	    OldAscSignalTransTable[NUM_PHASE];                   //�ź�ת�����б�,���ж�������λ����ʱ��
    struct STRU_N_EventConfig          AscEventCfgTable[MAX_EVENTCONFIG_COUNT];                //�������ñ�
    struct SCountDownCfg_DEL		    AscCountDownCfg;                                        //����ʱ��
    struct STRU_FaultDetectSet         AscFaultDectectSet;                                     //���ϼ������
    struct STRU_Address                AscAddress;                                             //��ַ���������ڵ�ַ�����ڵ�ַ
    char						AscChannelNotes[NUM_CHANNEL][40];                 //ͨ������ע��
    struct STRU_UnitParamEx			AscUnitParamEx;                                         //���ӵĵ�Ԫ��������
    struct STRU_SplitPedEx				AscSplitPedEx[NUM_GREEN_SIGNAL_RATION][NUM_PHASE];        //���ӵ����ű������˵�ʱ��
    struct STRU_PhaseEx				AscPhaseEx[NUM_PHASE];                            //����λ����Щ���ܣ�
    struct STRU_N_VehicleDetectorEx	AscVehicleDetectorTableEx[MAX_VEHICLEDETECTOR_COUNT];   //�����������
    struct STRU_N_TransIntensityChoice AscTransIntensityChoiceTable[MAX_TRANSINTENSITYCHOICE_COUNT];//��ͨǿ��ѡ���
    struct STRU_N_OverlapTableEx       AscOverlapTableEx[MAX_OVERLAP_COUNT];                   //Overlap�������ñ�
    struct STRU_UnitOptiPara			AscUnitOptiPara;                                        //�źŻ���Ԫ�Ż�����
	PhaseItem					stPhase[MAX_PHASE_TABLE_COUNT][NUM_PHASE];                                     //��λ��,����Ҳ�����˿��Բ�������λID
    struct STRU_SignalTransEntry	    AscSignalTransTable[MAX_PHASE_TABLE_COUNT][NUM_PHASE];                   //�ź�ת�����б�,���ж�������λ����ʱ��
	ChannelItem					stChannel[MAX_CHANNEL_TABLE_COUNT][NUM_CHANNEL];                                 //ͨ����
	FollowPhaseItem				stFollowPhase[MAX_FOLLOW_PHASE_TABLE_COUNT][NUM_FOLLOW_PHASE];                        //������λ
    SchemeItem					stScheme[NUM_SCHEME];                                   //�·�����
}SignalControllerPara,*PSignalControllerPara;//�źŻ����ò������ṹ

#pragma pack(pop)

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/

 
extern int IsSignalControlparaLegal(SignalControllerPara *pSignalControlpara);
extern Boolean LoadDataFromCfg(SignalControllerPara *pSignalControlpara);


/*Read*/
extern unsigned int ArrayToInt(unsigned char *array,int len);
extern void ReadUnitPara(PUnitPara item);
extern void ReadPhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE]);
extern void ReadChannelItem(ChannelItem channel[][NUM_CHANNEL]);
extern void ReadGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE]);
extern void ReadPhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT]);
extern void ReadSchemeItem(SchemeItem *scheme);
extern void ReadActionItem(ActionItem *action);
extern void ReadTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID]);
extern void ReadPlanSchedule(PlanScheduleItem *schedule);
extern void ReadFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE]);
extern void ReadVehicleDetector(struct STRU_N_VehicleDetector *vehicle);
extern void ReadPedestrianDetector(struct STRU_N_PedestrianDetector *item, int num);

/*Write*/
extern Boolean WriteConfigFile(SignalControllerPara *param);

extern void WriteUnitPara(PUnitPara item);
extern void WritePhaseItem(PhaseItem item[][NUM_PHASE], struct STRU_SignalTransEntry entry[][NUM_PHASE]);
extern void WriteChannelItem(ChannelItem channel[][NUM_CHANNEL]);
extern void WriteGreenSignalRationItem(GreenSignalRationItem green_split[][NUM_PHASE]);
extern void WritePhaseTurnItem(PhaseTurnItem phase_turn[][NUM_RING_COUNT]);
extern void WriteSchemeItem(SchemeItem *scheme);
extern void WriteActionItem(ActionItem *action);
extern void WriteTimeIntervalItem(TimeIntervalItem time_interval[][NUM_TIME_INTERVAL_ID]);
extern void WritePlanSchedule(PlanScheduleItem *schedule);
extern void WriteFollowPhaseItem(FollowPhaseItem follow_phase[][NUM_FOLLOW_PHASE]);
extern void WriteVehicleDetector(struct STRU_N_VehicleDetector *vehicle);
extern void WritePedestrianDetector(struct STRU_N_PedestrianDetector *item);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKCONFIG_H__ */
