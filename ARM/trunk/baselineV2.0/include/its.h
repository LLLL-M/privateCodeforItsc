#ifndef __EXTERN_H__
#define __EXTERN_H__

#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"

#ifdef __GNUC__
#define WEAKATTR    __attribute__((weak))
#define UNUSEDATTR  __attribute__((unused))
#else
#define WEAKATTR
#define UNUSEDATTR
#endif

#ifndef MAX_STAGE_NUM
#define MAX_STAGE_NUM	16
#endif
#ifndef MAX_PHASE_NUM
#define MAX_PHASE_NUM	16
#endif
#ifndef MAX_FOLLOWPHASE_NUM
#define MAX_FOLLOWPHASE_NUM MAX_PHASE_NUM
#endif
#ifndef MAX_CHANNEL_NUM
#define MAX_CHANNEL_NUM	32
#endif
#define MAX_BOARD_NUM	8		//���ʱ�ܹ��ĵ���������

//������ƶ�����
#define SINGLE_ADAPT_ACTIONID   112
#define	INDUCTIVE_ACTIONID		118
#define YELLOWBLINK_ACTIONID	119
#define	ALLRED_ACTIONID			116
#define TURNOFF_ACTIONID		115
#define INDUCTIVE_COORDINATE_ACTIONID	114

//������Ʒ�����
#define	INDUCTIVE_SCHEMEID				254			//ʵʱ��Ӧ������
#define YELLOWBLINK_SCHEMEID			255			//����������
#define	ALLRED_SCHEMEID					252			//ȫ�췽����
#define TURNOFF_SCHEMEID				251			//�صƷ�����
#define INDUCTIVE_COORDINATE_SCHEMEID	250			//��ӦЭ������
#define STEP_SCHEMEID           		249			//����������
#define SINGLE_ADAPT_SCHEMEID			248			//����Ӧ��Ӧ������	/*!!!!!!!!!!!!�궨���������޸�!!!!!!!!!!!*/
#define CHANNEL_LOCK_SCHEMEID	247			//ͨ������������,��ִ��ͨ������ʱֻ������ʵʱ״̬�ķ�����ʱ�õ�,Ŀǰ�����ſ�ƽ̨
#define SYSTEM_RECOVER_SCHEMEID	0			//�ָ�ϵͳ���Ʒ���

#include "calculate.h"

//������־���Ͷ���
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
	TIMEINTERVAL_COORDINATE_INDUCTIVE = 0x4a,		//����Э����Ӧ
	SENDMSG_FAIL = 0x4b,							//������Ϣʧ��
	//added by kevin
	MANUAL_PANEL_AUTO=0x4c,							//�ֶ���������Զ�����
	MANUAL_PANEL_MANUAL=0x4d,						//�ֶ���������ֶ�����
	MANUAL_PANEL_EAST=0x4e,							//�ֶ�������嶫���а���
	MANUAL_PANEL_SOUTH=0x4f,						//�ֶ���������Ϸ��а���
	MANUAL_PANEL_WEST=0x50,							//�ֶ�������������а���
	MANUAL_PANEL_NORTH=0x51,						//�ֶ�������山���а���
	MANUAL_PANEL_EASTWEST_DIRECT=0x52,				//�ֶ�������嶫��ֱ�а���
	MANUAL_PANEL_SOUTHNORTH_DIRECT=0x53,			//�ֶ���������ϱ�ֱ�а���
	MANUAL_PANEL_EASTWEST_LEFT=0x54,				//�ֶ�������嶫����ת����
	MANUAL_PANEL_SOUTHNORTH_LEFT=0x55,				//�ֶ���������ϱ���ת����
	WIRELESS_KEY_AUTO=0x56,							//����ң�����Զ�����
	WIRELESS_KEY_MANUAL=0x57,						//����ң�����ֶ�����
	WIRELESS_KEY_YELLOWBLINK=0x58,					//����ң������������
	WIRELESS_KEY_ALLRED=0x59,						//����ң����ȫ�찴��
	WIRELESS_KEY_STEP=0x5a							//����ң������������	
} FaultLogType;

typedef void (*UploadFaultLogFunc)(void *netArg, void *data, int datasize);	//���ع�����־�ĺ���

//������Ϣ�ṹ
typedef struct _fault_log_info
{
	int nNumber;    		//���к�
	int nID;                //��Ϣ����ID,��Ӧ��ö��FaultLogType
	long nTime;        		//����ʱ��,����ʱ�䣬��λΪ��
	int nValue;         	//ʱ��ֵ(ͨ����)
} FaultLogInfo;

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
	
	OFF_GREEN = 100,	//�̵����嵹��ʱ����һ��250ms�ص�
	OFF_YELLOW = 101,	//�̵����嵹��ʱ����һ��250ms�ص�
	OFF_RED = 102,		//������嵹��ʱ����һ��250ms�ص�
} LightStatus;

typedef enum
{	//�������ȼ����δӵ͵���
	AUTO_CONTROL = 0,	//�Զ�����
	WEB_CONTROL,		//web��ҳ����
	TOOL_CONTROL,		//���ù��߿���
	PLATFORM_CONTROL,	//ƽ̨����
	KEY_CONTROL,		//��������
	FAULT_CONTROL,		//���Ͽ���
} ControlType;	//��������

typedef struct
{
	UInt16 splitTime;			//��λ���ű�ʱ��
	UInt16 phaseSplitLeftTime;	//��λִ��ʱ�����ű�����ʣ���ʱ��
	UInt8 phaseStatus;			//��λ��ǰ��״̬,ȡֵ��ΧΪö��LightStatus������
	UInt16 phaseLeftTime;		//��λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt8 pedestrianPhaseStatus;//������λ״̬
	UInt16 pedestrianPhaseLeftTime;//������λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt8 followPhaseStatus;	//������λ��״̬
	UInt16 followPhaseLeftTime;	//������λʣ����̵�ʱ����Ǻ��ʱ��
	
	UInt64 vehicleDetectorBits;	//��λ��Ӧ�ĳ�������
	UInt8 unitExtendGreen;		//��λ�ӳ���
	UInt8 maxExtendGreen;		//�������ӳ����̵�ʱ��
	UInt8 maxExtendGreen2;      //����Ӧ���Ƶ��������ӳ����̵�ʱ��
	UInt32 motorChannelBits;	//��λ��Ӧ�Ļ�����ͨ����bit0-bit31�ֱ����ͨ��1-32
	UInt32 pedChannelBits;		//��λ��Ӧ������ͨ����bit0-bit31�ֱ����ͨ��1-32
} PhaseInfo;

typedef struct
{
	UInt8 allChannels[MAX_CHANNEL_NUM];	//����ͨ����״̬
	UInt16 channelCountdown[MAX_CHANNEL_NUM];	//ͨ���ĵ���ʱ
	UInt16 cycleTime;				//��ǰ������ʱ��
	UInt16 inductiveCoordinateCycleTime;	//��ӦЭ����������ʱ��
	//UInt8 runTime;				//��ǰ�����Ѿ������˶���ʱ��
	UInt16 leftTime;					//��ǰ���ڻ�ʣ�¶���ʱ��
	UInt8 stageNum;					//��ǰ���еĽ׶κ�
	UInt8 maxStageNum;				//���׶κ�
	Boolean isStep;					//�Ƿ����ڲ���
	UInt8 schemeId;                 //��ǰ���еķ�����
	UInt8 phaseTableId;				//��ǰʹ�õ���λ���
	UInt8 channelTableId;			//��ǰʹ�õ�ͨ�����
	UInt8 actionId;					//������
	UInt8 checkTime;				//��Ӧ���ʱ��
	UInt8 collectCycle;				//�����ɼ����ڣ���λs

    UInt32 motorChanType;           //���õĻ���������������ͨ��
    UInt32 pedChanType;             //���õ����ˣ����˸�������ͨ��
    
	PhaseInfo phaseInfos[MAX_PHASE_NUM];	//������λ����Ϣ
} LineQueueData;	//���Զ����д�ŵ�����

typedef struct 
{
    UInt16 L0:3;
    UInt16 L1:3;
    UInt16 L2:3;
    UInt16 L3:3;
    UInt16 unused:4;
} lamp_t;

typedef void *(*threadProcessFunc)(void *arg);
struct threadInfo
{
	pthread_t id;				//�̺߳ţ�������ItsExitʱ�ͷ��߳���Դʹ��
	threadProcessFunc func;		//�̵߳Ĵ�����
	char *moduleName;			//�̵߳�����
	UInt32 countvalue;			//����ֵ����Ҫ�����ж���λ����ģ�������ģ���Ƿ�һֱ����˯��״̬
};

//ͨ�������ṹ��
typedef struct channel_lock_params
{
#define CHANNEL_UNLOCK			0	//ͨ��δ����
#define CHANNEL_LOCK			1	//ͨ������
#define CHANNEL_WAIT_FOR_LOCK	2	//ͨ��������
#define CHANGEABLE_CHANNEL_LOCK	3	//�ɱ�ͨ������
	unsigned char    ucChannelLockStatus;		//ͨ������״̬���������궨������
	unsigned char    ucChannelStatus[MAX_CHANNEL_NUM];		//32��ͨ��������״̬������ֵ��ö��LightStatus����
	unsigned char    ucWorkingTimeFlag;		//ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
	unsigned char    ucBeginTimeHour;			//������Чʱ�䣺Сʱ
	unsigned char    ucBeginTimeMin;			//������Чʱ�䣺����
	unsigned char    ucBeginTimeSec;			//������Чʱ�䣺��
	unsigned char    ucEndTimeHour;			//���ƽ���ʱ�䣺Сʱ
	unsigned char    ucEndTimeMin;			//���ƽ���ʱ�䣺����
	unsigned char    ucEndTimeSec;			//���ƽ���ʱ�䣺��
} ChannelLockParams;

//������λ���������
typedef enum    //���ں�����λʱ��Ĵ���
{
    NONE_IGNORE = 0,        //������
    FORWARD_IGNORE = 1,     //��ǰ���ԣ����Ѻ�����λʱ���ǰ�����λ
    BACKWARD_IGNORE = 2,    //�����ԣ����Ѻ�����λʱ����������λ
    ALL_IGNORE = 3,         //ǰ�󶼺��ԣ����Ѻ�����λ��ʱ��ƽ�ָ�ǰ��ͺ������λ
} IgnoreAttr;

typedef struct
{
	Boolean isContinueRun;			//�������������֮���Ƿ����֮ǰ�����ڼ�������
	Boolean addTimeToFirstPhase;	//��ӦЭ�����Ͽ���ʱ�Ƿ��ʣ��ʱ���ۼӵ���һ����λ
} CustomParams;

typedef struct
{
	Boolean enableRedSignal;		//����ź�ʹ��
	char mcastIp[16];				//�鲥ip
	UInt16 mcastPort;				//�鲥�˿�
	UInt16 deviceId;				//�豸id
} McastInfo;

/***************************************
���º�����Ҫ�û��Լ�ȥʵ��
***************************************/
//���������õ��ڴ�
WEAKATTR extern void *ItsAllocConfigMem(void *config, int configSize);
//��ʼ����Ԫ����
WEAKATTR extern void ItsUnitInit(void (*initFunc)(UInt8, UInt8));
//��������Ϣ
struct _CalInfo;
WEAKATTR extern Boolean FillCalInfo(struct _CalInfo *calInfo, UInt8 schemeId, time_t calTime);
//���ں�����λ�Ĵ���
WEAKATTR extern void IgnorePhaseDeal(struct _CalInfo *calInfo);
//����ʵʱ��Ϣ
WEAKATTR extern void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data);
//������
WEAKATTR extern void ItsLight(int boardNum, unsigned short *poutLamp);
//���ϼ��,����Ϊ8���������ָ��
WEAKATTR extern void ItsFaultCheck(UInt16 *lightValues);
//�����������
WEAKATTR extern UInt64 ItsReadVehicleDetectorData();
//�����˰���
//extern UInt16 PedestrianCheckInput(void);
/*����dataΪ��ǰ1s���źŻ�ʹ����ϸ��Ϣ������������ʹ�ã�
 * �˺�����ִ�й��̲��ܳ���700ms������ò��϶��ƿ��Բ���ʵ�� */
WEAKATTR extern void ItsCustom(LineQueueData *data, CustomParams *customParams);
//Ԥ���ĵ���ʱ��������ӿڣ�����Ϊ��1��λ��ʼ��num����λ�����������Ϣ
WEAKATTR extern void ItsCountDownOutput(LineQueueData *data);
WEAKATTR void ItsSetCurRunData(LineQueueData *data);

WEAKATTR extern unsigned char ChannelControl(unsigned char *chan);//ͨ������
WEAKATTR extern void channelLockTransition(unsigned char lockFlag, unsigned char *curStatus, unsigned char *lockstatus);//ͨ����������ʵ�ֽӿ�
WEAKATTR extern unsigned char GetBootTransitionTime(void);//��ȡϵͳ����ʱ�Ļ���ȫ��ʱ��

/*********************************************
������libits���ṩ��һЩ�ɹ��ⲿ������õĽӿ�
*********************************************/
//libits��ĳ�ʼ�����˳����߳�״̬���
/*	threads:��Ҫ��libitsһ���ʼ�����߳������Ϣ��ָ��
	num:�̵߳ĸ���	*/
extern Boolean ItsInit(struct threadInfo *threads, int num, void *config, int configSize);
extern void ItsExit(void);
extern void ItsThreadCheck(void);
//libits������úͻ�ȡ������Ϣ
extern void ItsSetConfig(void *config, int configSize);
extern void ItsGetConfig(void *config, int configSize);
//libits��Ŀ���
/*	type:���Ʒ�ʽ������������ö��ControlType
	mschemeid:�ֶ�����ʱ�ķ�����
	stageNum:�����Ľ׶κ�	*/
extern void ItsCtl(ControlType type, UInt8 schemeId, int val);
//���������Ϳ�����Ϣ
extern void ItsCtlNonblock(ControlType type, UInt8 schemeId, int val);
//ͨ���ļ����������Լ�ͨ�����

extern void ItsChannelCheck(UInt8 channelId, LightStatus status);
//��ȡƽ̨����ʱ��Ϣ
extern void ItsCountDownGet(void *countdown, int size);
extern void ItsGetCurRunData(LineQueueData *data);
//��ȡ��ǰ�Ŀ���״̬���������������platform.h�е�STRU_CoordinateStatus�ṹ�����byUnitControlStatus
extern UInt8 ItsControlStatusGet(void);
//������־�Ķ�ȡ��д������
extern void ItsReadFaultLog(int startLine, int lineNum, void *netArg, int netArgSize, UploadFaultLogFunc func);
extern void ItsWriteFaultLog(FaultLogType type, int value);
extern void ItsClearFaultLog(void);
//������ȫ�졢�صƵ�������ƺ��Ƿ��������֮ǰ����������
extern void ItsIsContinueRun(Boolean val);
//�����鲥��ص���Ϣ
extern void ItsSetMcastInfo(McastInfo *);

#endif

