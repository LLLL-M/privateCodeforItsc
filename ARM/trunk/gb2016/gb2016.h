#ifndef __GB2016_H__
#define __GB2016_H__

#include "hik.h"

#pragma pack(push, 1)

typedef struct
{
	UInt8	version;			//�汾��
	UInt8	sendFlag;			//���ͷ���ʶ
	UInt8	recvFlag;			//���շ���ʶ
	UInt8	dataLinkCode;		//������·��
	UInt8	areaNumber;			//�����
	UInt16	roadNumber;			//·�ں�
	UInt8	operatorType;		//��������
	UInt8	objectId;			//�����ʶ
	UInt8	reserve[5];			//����
} FrameHead;

typedef enum
{
	COMMUNICATION_RULE_LINK = 1,	//ͨ�Ź����·
	BASIC_INFORMATION_LINK = 2,		//������Ϣ��·
	FEATURES_PARAMETER_LINK = 3,	//��������һ�㽻����·
	INTERVENE_COMMAND_LINK = 4,		//��Ԥָ����·
} DataLinkCode;		//������·��

typedef enum
{
	GB_QUERY_REQ = 0x80,		//��ѯ����
	GB_SET_REQ = 0x81,			//��������
	GB_ACTIVE_UPLOAD = 0x82,	//�����ϱ�
	GB_QUERY_REPONSE = 0x83,	//��ѯӦ��
	GB_SET_REPONSE = 0x84,		//����Ӧ��
	GB_ERR_REPONSE = 0x85,		//����Ӧ��
} OperateType;	//��������

enum
{
	ONLINE = 1,					//����
	TRAFFIC_FLOW_INFO = 2,		//��ͨ����Ϣ
	WORK_STATUS = 3,			//����״̬
	LAMP_STATUS = 4,			//��ɫ״̬
	CURRENT_TINE = 5,			//����ʱ��
	SIGNAL_LAMP_GROUP = 6,		//�źŵ���
	PHASE = 7,					//��λ
	SIGNAL_TIMING_SCHEME = 8,	//�ź���ʱ����
	SCHEME_SCHEDULE = 9,		//�������ȼƻ�
	WORK_MODE = 10,				//������ʽ
	SIGNAL_MACHINE_FAULT = 11,	//�źŻ�����
	SIGNAL_MACHINE_VER = 12,	//�źŻ��汾
	FEATURES_PARAMETER = 13,	//��������
	SIGNAL_MACHINE_ID_CODE = 14,//�źŻ�ʶ����
	REMOTE_CONTROL = 15,		//Զ�̿���
	DETECTOR = 16,				//�����
};

typedef struct
{
	UInt8	detectorId;			//�������
	UInt8	vehicleFlow;		//������
	UInt32	beginTime;			//��ʼʱ��
//	UInt8	occupancyRate;		//ռ����
//	UInt8	vehicleSpeed;		//����
//	UInt8	vehicleLength;		//����
//	UInt8	queueLength;		//�Ŷӳ���
} TrafficFlowInfo;	//��ͨ����Ϣ

typedef struct
{
	UInt8	ctrlType;					/*��������
										 0:����ʱ�ο���
										 1:ƽ̨��ͻ��˿���
										 2:��������
										 3:���Ͽ���*/
	UInt8 	ctrlMode;					/*����ģʽ
										 1:�صƿ���,��ʱ�����û��ֶ�ִ��
										 2:��������,��ʱ�����û��ֶ�ִ��
										 3:ȫ�����,��ʱ�����û��ֶ�ִ��
										 4:�����ڿ���,��ʱ�����û��ֶ�ִ��
										 5:Э������,ֻ��ʱ������
										 6:��Ӧ����,��ʱ�����û��ֶ�ִ��
										 7:�����Ż�����,��ʱ�����û��ֶ�ִ��
										 8:��������,ֻ���ֶ�ִ��
										 10:���˸�Ӧ����,��ʱ�����û��ֶ�ִ��
										 11:�������ȿ���,��ʱ�����û��ֶ�ִ��
										 */
    UInt8   ctrlId;                     /*ʹ�õķ�����[0,15],������ȫ�졢�ص�ʱ������Ϊ0
										  ���߲�����(0:��������,����:��ת����)
										  ����ͨ���������[1,15]
										 */
	UInt8	phase;						//��ǰ���е���λ
	UInt16	cycleTime;					//��ǰ������ʱ��
} CurrentWorkStatus;	//�źŻ���ǰ�Ĺ���״̬

typedef enum
{
	LVOFF = 0,
	LVGREEN = 1,
	LVRED = 2,
	LVYELLOW = 3,
} LampValue;	//�źŵ�ֵ

struct LampColorStatus
{
	UInt64	lamps;	//ÿ2bit����һ���ƣ����֧��32���ƣ���ֵ��ö��LampValue
	UInt32	reserve;
};	//�źŵ�ɫ״̬

typedef struct
{
	UInt8	channelId;			//ͨ����,[1,32]
	UInt8	channelType;		//ͨ������,0:δʹ��,1:������,2:����,������ö�ٶ���
	UInt32	conflictChannel;	//��ͻͨ����bit0-bit31�ֱ����ͨ��1-32*/
	UInt8	direction;			//ͨ������
	UInt8	flag;				//������ɱ�־,0:δ���,1:���
	UInt8	reserve[4];			//�����ֽ�
} ChannelItem;	//ͨ��

typedef struct
{
	UInt8	phaseId;			//��λ��,[1,16]
	UInt8	greenTime;			//�̵�ʱ��
	UInt8	greenBlinkTime;		//����ʱ��
	UInt8	yellowTime;			//�Ƶ�ʱ��
	UInt8	allRedTime;			//ȫ��ʱ��
	UInt8	autoRequest;		//�Զ�����
	UInt8	pedClearTime;		//�������ʱ�䣬����������ʱ��
	UInt8	pedResponseTime;	//���˹�����Ӧʱ��
	UInt8	minGreenTime;		//��С��
	UInt8	maxGreenTime;		//�����
	UInt8	maxGreenTime2;		//�����2
	UInt8	unitExtendTime;		//��λ�ӳ���
	UInt8	checkTime;			//��Ӧ���ʱ��
	UInt32	channelBits;		//��λ������ͨ����bit0-bit31�ֱ����ͨ��1-32
	UInt32 	vehDetectorBits;	//��λ�����ĳ�������bit0-bit32�ֱ��������1-32
	UInt8 	pedDetectorBits;	//��λ�������˻򹫽��������bit0-bit7�ֱ��������1-8
	UInt8	advanceExtendTime;	//�����ӳ�ʱ��
	UInt8	flag;				//������ɱ�־,0:δ���,1:���
} PhaseItem;	//��λ

typedef struct
{
	UInt8	schemeId;			//������,[1,16]
	UInt16	cycleTime;			//����
	UInt8	phaseOffset;		//��λ��
	UInt8	coordinatePhase;	//Э����λ
	UInt8	phaseturn[16];		//����
	UInt8	flag;				//������ɱ�־,0:δ���,1:���
	UInt8	reserve[2];			//�����ֽ�
} SchemeItem;	//����

typedef struct
{
	UInt8	scheduleId;			//���Ⱥ�,[1,255]
	UInt8	week;				//����,[0,7],0:��ʹ������,7��ʾ������,1~6��ʾ����һ��������
	UInt8	month;				//�·�,[0,12],0:��ʹ���·�,1~12��ʾһ�µ�ʮ����
	UInt8	day;				//����,[1,31]
	UInt8	startHour;			//��ʼִ��Сʱ,[0,23]
	UInt8	startMin;			//��ʼִ�з���,[0,59]
	UInt8	endHour;			//����ִ��Сʱ,[0,24]
	UInt8	endMin;				//����ִ�з���,[0,59]
	UInt8	ctrlType;			/*�������ͣ�0:����ʱ�ο���*/
	UInt8 	ctrlMode;			/*����ģʽ
								 1:�صƿ���,��ʱ�����û��ֶ�ִ��
								 2:��������,��ʱ�����û��ֶ�ִ��
								 3:ȫ�����,��ʱ�����û��ֶ�ִ��
								 4:�����ڿ���,��ʱ�����û��ֶ�ִ��
								 5:Э������,ֻ��ʱ������
								 6:��Ӧ����,��ʱ�����û��ֶ�ִ��
								 7:�����Ż�����,��ʱ�����û��ֶ�ִ��
								 10:���˸�Ӧ����,��ʱ�����û��ֶ�ִ��
								 11:�������ȿ���,��ʱ�����û��ֶ�ִ��
								 */
    UInt8   ctrlId;             /*ʹ�õķ�����[0,15],������ȫ�졢�ص�ʱ������Ϊ0*/
	UInt8	flag;				//������ɱ�־,0:δ���,1:���
} ScheduleItem;		//���ȼƻ�

typedef struct
{
	UInt32	seq;    		//���к�
	UInt16	type;           //��־����
	UInt16	value;         	//ʱ��ֵ(ͨ����)
	UInt32	time;        	//����ʱ��
} FaultLog;			//������־

typedef struct
{
	UInt8	gps:1;					//GPS����
	UInt8	watchdog:1;				//watchdog����
	UInt8	voltCheck:1;			//��ѹ��⿪��
	UInt8	curCheck:1;				//������⿪��
	UInt8	faultYellowFlash:1;		//���ϻ�������
	UInt8	takeover:1;				//�ƿؽӹܿ���
	UInt8	reserve:2;				//����λ
} SwtichParam;		//���ز���

typedef enum
{
	RESTART = 1,			//����
	RECOVER_DEFAULT = 2,	//�ָ�Ĭ��
	FAULT_CLEAR = 3,		//�������
} RemoteControlCmd;		//Զ�̿���ָ��

typedef struct
{
	UInt8	detectorId;			//�������,[1,32]
	UInt8	noResponseTime;		//����Ӧʱ��,��λΪ����
	UInt8	maxContinuousTime;	//������ʱ��,��λΪ����
	UInt8	maxVehcileNum;		//�������,��λΪ��/����
	UInt8	detectorType;		//���������,1���������������2�����˼������3���������ȼ����
	UInt8	flag;				//������ɱ�־,0:δ���,1:���
	UInt8	reserve[10];		//�����ֽ�
} DetectorItem;		//�����

typedef struct
{
	UInt8	areaNumber;				//������,Ĭ��1
	UInt16	roadNumber;				//������·�ڱ��,Ĭ��1
	char	version[20];			//�źŻ��汾,Ĭ��V2.0.1.1
	char	identifyCode[20];		//�źŻ�ʶ����,Ĭ��HK344H20170001
	UInt8	bootYellowBlinkTime;	//��������ʱ��,Ĭ��6s
	UInt8	bootAllRedTime;			//����ȫ��ʱ��,Ĭ��6s
	UInt16	vehFlowUploadCycleTime;	//��ͨ���ϱ�����,��λΪs��Ĭ��5min
	UInt8	transitionCycle;		//�̲�����ʱ�Ĺ������ڸ���,Ĭ��2������
	char	ip[20];					//��λ��ip
	UInt16	port;					//��λ���˿�
} Basic;

#pragma pack(pop)

#endif
