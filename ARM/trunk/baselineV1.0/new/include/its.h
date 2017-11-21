#ifndef __EXTERN_H__
#define __EXTERN_H__

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "HikConfig.h"
#include "platform.h"

#ifdef __GNUC__
#define WEAKATTR    __attribute__((weak))
#define UNUSEDATTR  __attribute__((unused))
#else
#define WEAKATTR
#define UNUSEDATTR
#endif

#define MAX_BOARD_NUM	8		//���ʱ�ܹ��ĵ���������
//������Ʒ�����
#define	INDUCTIVE_SCHEMEID		254
#define YELLOWBLINK_SCHEMEID	255
#define	ALLRED_SCHEMEID			252
#define TURNOFF_SCHEMEID		251
#define STEP_SCHEMEID           249

typedef enum
{
	NTCIP = 0,
	GB2007 = 1,
} ProtocolType;

typedef enum
{	//�������ȼ����δӵ͵���
	AUTO_CONTROL = 0,	//�Զ�����
	WEB_CONTROL,		//web��ҳ����
	TOOL_CONTROL,		//���ù��߿���
	//PLATFORM_CONTROL,	//ƽ̨����
	KEY_CONTROL,		//��������
	FAULT_CONTROL,		//���Ͽ���
} ControlType;	//��������

//��ʵ�ֵĹ�����־���Ͷ���
typedef enum
{
	INVALID_TYPE = 0,								//��Ч������	
	//DISCONNECT_WITH_CENTER = 1,					//�����ĶϿ�
	//CONNECT_WITH_CENTER = 2,						//�����ĶϿ���������
	COMMUNICATION_DISCONNECT = 3,					//ͨ�ŶϿ�
	COMMUNICATION_CONNECT = 4,						//ͨ�ŶϿ���������
	//DETECTOR_NO_RESPONSE = 5,						//���������Ӧ
	//DETECTOR_NO_RESPONSE_CLEAR = 6,				//���������Ӧ���
	//DOOR_OPEN = 7,								//����
	//DOOR_CLOSE = 8,								//����
	//POWER_OFF = 9,								//�ϵ�
	//POWER_ON = 0x0A,								//�ϵ�
	RED_GREEN_CONFLICT = 0x0B,						//���̳�ͻ
	RED_GREEN_CONFLICT_CLEAR = 0x0C,				//���̳�ͻ���
	RED_LIGHT_OFF = 0x0D,							//���Ϩ��
	RED_LIGHT_OFF_CLEAR = 0x0E,						//���Ϩ�����
	GREEN_CONFLICT = 0x0F,							//�̳�ͻ
	GREEN_CONFLICT_CLEAR = 0x10,					//�̳�ͻ���
	//VOLTAGE_HIGH = 0x11,							//��ѹ����
	//VOLTAGE_HIGH_CLEAR = 0x12,					//��ѹ�������
	//VOLTAGE_LOW = 0x13,							//��ѹ����
	//VOLTAGE_LOW_CLEAR = 0x14,						//��ѹ�������
	GREEN_LIGHT_OFF = 0x15,							//�̵�Ϩ��
	GREEN_LIGHT_OFF_CLEAR = 0x16,					//�̵�Ϩ�����
	AUTO_TO_MANUAL = 0x17,							//�Զ�ת�ֶ�����
	MANUAL_TO_AUTO = 0x18,							//�ֶ�ת�Զ�����
	//POW_ON_REBOOT = 0x19,							//�ϵ�����
	UNNORMAL_OR_SOFTWARE_REBOOT = 0x1A,				//�쳣���������������
	//COUNTDOWN_COMMUNICATION_FAULT = 0x25,			//����ʱ��ͨѶ����
	//COUNTDOWN_COMMUNICATION_FAULT_RECOVER = 0x26,	//����ʱ��ͨѶ�ָ�
	FAULT_FLASH = 0x29,								//��������
	TIMEINTERVAL_FLASH = 0x2B,						//ʱ������
	MANUAL_PANEL_FLASH = 0x2C,						//�ֶ��������
	TIMEINTERVAL_TURN_OFF = 0x2D,					//ʱ�ιص�
	TIMEINTERVAL_ALL_RED = 0x2E,					//ʱ��ȫ��
	MANUAL_PANEL_ALL_RED = 0x2F,					//�ֶ����ȫ��
	MANUAL_PANEL_STEP = 0x30,						//�ֶ���岽��

	//add by Jicky
	INIT_LOCAL_CONFIG_SUCC = 0x41,					//��ʼ���������óɹ�
	INIT_LOCAL_CONFIG_FAIL = 0x42,					//��ʼ����������ʧ��
	THREAD_EXCEPTION_EXIT = 0x43,					//�߳��쳣�˳�
	LOCAL_CONFIG_UPDATE = 0x44,						//�������ø���
	DOWNLOAD_CONFIG_CHECK_FAIL = 0x45,				//��������У��ʧ��
	TIMEINTERVAL_INDUCTIVE = 0x46,					//���ظ�Ӧ
	CALCULATE_FAIL_CAUSE_FLASH = 0x47,				//����ģ�����ʧ�����������
	WEB_CONTROL_LOG = 0x48,							//web��ҳ������־
	TOOL_CONTROL_LOG = 0x49,						//���߿�����־
} FaultLogType;

typedef struct
{
	UInt8 splitTime;			//��λ���ű�ʱ��
	UInt8 phaseSplitLeftTime;	//��λִ��ʱ�����ű�����ʣ���ʱ��
	UInt8 phaseStatus;			//��λ��ǰ��״̬,ȡֵ��ΧΪö��LightStatus������
	UInt8 phaseLeftTime;		//��λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt8 pedestrianPhaseStatus;//������λ״̬
	UInt8 pedestrianPhaseLeftTime;//������λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt8 followPhaseStatus;	//������λ��״̬
	UInt8 followPhaseLeftTime;	//������λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt8 vehicleDetectorId;	//��λ��Ӧ�ĳ�������
	UInt8 unitExtendGreen;		//��λ�ӳ���
	UInt8 maxExtendGreen;		//�������ӳ����̵�ʱ��
} PhaseInfo;

typedef struct
{
	UInt8 allChannels[NUM_CHANNEL];	//����ͨ����״̬
	UInt8 cycleTime;				//��ǰ������ʱ��
	//UInt8 runTime;				//��ǰ�����Ѿ������˶���ʱ��
	UInt8 leftTime;					//��ǰ���ڻ�ʣ�¶���ʱ��
	UInt8 stageNum;					//��ǰ���еĽ׶κ�
	UInt8 maxStageNum;				//���׶κ�
	UInt8 schemeId;                 //��ǰ���еķ�����
	UInt8 phaseTableId;				//��ǰʹ�õ���λ���
	UInt8 channelTableId;			//��ǰʹ�õ�ͨ�����
	UInt8 checkTime;				//��Ӧ���ʱ��
	PhaseInfo phaseInfos[NUM_PHASE];	//������λ����Ϣ
} LineQueueData;	//���Զ����д�ŵ�����

//������λ����ͨ����ǰ1s��״̬
typedef enum 
{
    INVALID = 0,
	GREEN = 1,
	RED = 2,
	YELLOW = 3,
	GREEN_BLINK = 4,
	YELLOW_BLINK = 5,
	ALLRED = 6,
	TURN_OFF = 7,
	RED_BLINK = 8,
	RED_YELLOW = 9,
} LightStatus;

typedef struct 
{
    UInt16 L0:3;
    UInt16 L1:3;
    UInt16 L2:3;
    UInt16 L3:3;
    UInt16 unused:4;
} lamp_t;

/***************************************
����5��������Ҫ�û��Լ�ȥʵ��
***************************************/
//������
WEAKATTR extern void ItsLight(int boardNum, unsigned short *poutLamp);
//���ϼ��,����Ϊ8���������ָ��
WEAKATTR extern void ItsFaultCheck(UInt16 *lightValues);
//�����������
WEAKATTR extern UInt16 ItsReadVehicleDetectorData(int boardNum);
//�����˰���
//extern UInt16 PedestrianCheckInput(void);
/*����dataΪ��ǰ1s���źŻ�ʹ����ϸ��Ϣ������������ʹ�ã�
 * �˺�����ִ�й��̲��ܳ���700ms������ò��϶��ƿ��Բ���ʵ�� */
WEAKATTR extern void ItsCustom(LineQueueData *data);
//Ԥ���ĵ���ʱ��������ӿڣ�����Ϊ��1��λ��ʼ��num����λ�����������Ϣ
WEAKATTR extern void ItsCountDownOutput(LineQueueData *data);

typedef void *(*threadProcessFunc)(void *arg);
struct threadInfo
{
	pthread_t id;				//�̺߳ţ�������ItsExitʱ�ͷ��߳���Դʹ��
	threadProcessFunc func;		//�̵߳Ĵ�����
	char *moduleName;			//�̵߳�����
	UInt32 countvalue;			//����ֵ����Ҫ�����ж���λ����ģ�������ģ���Ƿ�һֱ����˯��״̬
};

/*********************************************
������libits���ṩ��һЩ�ɹ��ⲿ������õĽӿ�
*********************************************/
//libits��ĳ�ʼ�����˳����߳�״̬���
/*	threads:��Ҫ��libitsһ���ʼ�����߳������Ϣ��ָ��
	num:�̵߳ĸ���	*/
extern Boolean ItsInit(struct threadInfo *threads, int num, ProtocolType type, void *config);
extern void ItsExit(void);
extern void ItsThreadCheck(void);
//libits������úͻ�ȡ������Ϣ
extern void ItsSetConfig(ProtocolType type, void *config);
extern void ItsGetConfig(ProtocolType type, void *config);
//libits��Ŀ���
/*	type:���Ʒ�ʽ������������ö��ControlType
	mschemeid:�ֶ�����ʱ�ķ�����
	stageNum:�����Ľ׶κ�	*/
extern void ItsCtl(ControlType type, UInt8 schemeId, UInt8 stageNum);
//ͨ���ļ����������Լ�ͨ�����
extern void ItsChannelLock(CHANNEL_LOCK_PARAMS *lockparams);
extern void ItsChannelUnlock(void);
extern void ItsChannelCheck(UInt8 channelId, LightStatus status);
//��ȡƽ̨����ʱ��Ϣ
extern void ItsCountDownGet(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *countdown);
//��ȡ��ǰ�Ŀ���״̬���������������platform.h�е�STRU_CoordinateStatus�ṹ�����byUnitControlStatus
extern UInt8 ItsControlStatusGet(void);
//������־�Ķ�ȡ��д������
extern void ItsReadFaultLog(int startLine, int lineNum, int socketfd, struct sockaddr *fromAddr);
extern void ItsWriteFaultLog(FaultLogType type, int value);
extern void ItsClearFaultLog(void);

//������λ���������
typedef enum	//���ں�����λʱ��Ĵ���
{
	NONE_IGNORE = 0,		//������
	FORWARD_IGNORE = 1,		//��ǰ���ԣ����Ѻ�����λʱ���ǰ�����λ
	BACKWARD_IGNORE = 2,	//�����ԣ����Ѻ�����λʱ����������λ
	ALL_IGNORE = 3,			//ǰ�󶼺��ԣ����Ѻ�����λ��ʱ��ƽ�ָ�ǰ��ͺ������λ
} IgnoreAttr;
extern void ItsSetIgnoreAttr(IgnoreAttr attr);

//���ú�Ƶ���ʱ��˸����
extern void ItsSetRedFlashSec(UInt8 sec);
//���ûƵ��Ƿ���˸,FALSE:����˸��TRUE:��˸
extern void ItsYellowLampFlash(Boolean val);
//�Ƿ�����������л���ϵͳ���ƺ����֮ǰ�ķ�����������
extern void ItsIsContinueRun(Boolean val);
//��ȡʵʱ�ķ��������Ϣ
extern void ItsGetRealtimePattern(MsgRealtimePattern *p);

#endif

