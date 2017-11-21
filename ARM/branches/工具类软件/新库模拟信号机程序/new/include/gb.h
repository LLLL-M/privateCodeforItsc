#ifndef __GB_H__
#define __GB_H__

#include "hik.h"

#define GB_EVENT_FILE	"/home/gbevent.dat"

#define	MAX_PHASE_LIST_NUM		16		//�����λ�б����

#define GB_INDUCTIVE_SCHEME		252		//���Э����û��˵�������Ҳ²��
#define	GB_ALLRED_SCHEME		253
#define GB_YELLOWFLASH_SCHEME	254
#define GB_TURNOFF_SCHEME		255

#pragma pack(push, 1)

typedef enum
{
	GB_QUERY_REQ = 0,		//��ѯ����
	GB_SET_REQ = 1,			//��������
	GB_SET_NO_REPONSE = 2,	//�������󣬵�����Ҫȷ��Ӧ��
	GB_AUTO_UPLOAD = 3,		//�����ϱ�
	GB_QUERY_REPONSE = 4,	//��ѯӦ��
	GB_SET_REPONSE = 5,		//����Ӧ��
	GB_ERR_REPONSE = 6,		//����Ӧ��
} GbOperateType;	//��������

typedef struct gbMsgHead
{
	UInt8 operateType:4;	//��������,��ֵ��Ӧ��GbOperateTypeö��
	UInt8 objectNum:3;	//�������
	UInt8 default1:1;		//�̶�Ϊ1
} GbMsgTypeField;	//��Ϣ������

typedef struct gbObjectField
{
	UInt8 objectId;			//�����ʶ
	UInt8 childObject:6;	//�Ӷ���
	UInt8 indexNum:2;		//��������
	UInt8 indexs[0];		//����ֵ
} GbObjectField;

//��������
typedef enum
{
	MSG_LENGTH_TOO_LONG = 1,				//��Ϣ���ȹ���
	MSG_TYPE_ERR,							//��Ϣ���ʹ���
	SETTING_OBJECT_ERR,						//���õĶ���ֵ�����涨�ķ�Χ
	MSG_LENGTH_TOO_SHORT,					//��Ϣ����̫��
	MSG_OTHER_ERR,							//�������������͵���������
	MSG_IGNORE,								//���Ըö���
	MSG_OK,									//��Ϣ����
}MSG_ERR_TYPE;

typedef struct gbModParamList
{
	UInt8 modNo;										//ģ����к�(����)����Χ[1,16]
	UInt8 modDevNodeLen;								//ģ���豸�ڵ㳤��
#define MAX_MOD_DEV_NODE_LEN		255					//���ģ���豸�ڵ㳤��
	UInt8 modDevNode[MAX_MOD_DEV_NODE_LEN];				//ģ���豸���͵Ľڵ�ʶ���
	UInt8 modManufacturerLen;							//ģ�������̳���
#define	MAX_MOD_MANUFACTURER_LEN	255					//���ģ�������̳���
	UInt8 modManufacturer[MAX_MOD_MANUFACTURER_LEN];	//ģ��������
	UInt8 modStyleLen;									//ģ���ͺų���
#define	MAX_MOD_STYLE_LEN			255					//���ģ���ͺų���
	UInt8 modStyle[MAX_MOD_STYLE_LEN];					//ģ���ͺ�
	UInt8 modVersionLen;								//ģ��汾����
#define	MAX_MOD_VERSION_LEN			255					//���ģ��汾����
	UInt8 modVersion[MAX_MOD_VERSION_LEN];				//ģ��汾
	UInt8 modType;				//ģ�����ͣ�1:������2:Ӳ����3:���
} GbModParamList;	//ģ�������б�

typedef struct gbScheduleList
{
	UInt8  scheduleNo;		//���ȼƻ���ţ���Χ[1,40]
	UInt16 month;			//bit1-bit12��ÿλ��ʾһ���¡���1��ʾ�����Ӧ�ƻ��ڸ���ִ��
	UInt8  week;			//bit1-bit7��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ��
	UInt32 day;		        //bit1-bit31��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ��
    UInt8  timeIntervalListNo;	//ʱ�α��ţ�0��ʾ������Ч
} GbScheduleList;	//���ȼƻ��б�

typedef struct gbTimeIntervalList
{
	UInt8 timeIntervalListNo;	//ʱ�α��ţ���Χ[1,16]
	UInt8 timeIntervalNo;		//ʱ�κţ���Χ[1,48]
	UInt8 hour;					//��ʼִ�е���������24Сʱ��
	UInt8 minute;				//��ʼִ�е�������
	UInt8 controlMode;			//���Ʒ�ʽ
	UInt8 schemeId;				//�����ţ�0��ʾû�п�ִ�еķ��������źŻ������������Ʒ�ʽ
	UInt8 assistFunctionOutput;	//�����������
	UInt8 specialFunctionOutput;//���⹦�����
} GbTimeIntervalList;	//ʱ���б�

typedef enum
{
	PROGRAM_START_EVENT = 1,		//���������¼�
	PROGRAM_END_EVENT = 2,			//��������¼�
	NOT_CONFIG_EVENT = 3,			//û������
	AUTO_KEY_PRESS_EVENT = 4,		//�Զ��������¼�
	MANUAL_KEY_PRESS_EVENT = 5,		//�ֶ��������¼�
	YELLOWFLASH_KEY_PRESS_EVENT = 6,//�����������¼�
	ALLRED_KEY_PRESS_EVENT = 7,		//ȫ��������¼�
	STEP_KEY_PRESS_EVENT = 8,		//�����������¼�
	RED_GREEN_CONFLICT_EVENT = 9,	//���̳�ͻ�¼�
	GREEN_CONFLICT_EVENT = 10,		//�̳�ͻ�¼�
	RED_LIGHT_TURNOFF_EVENT = 11,	//���Ϩ���¼�
	//�������ڳ����������¼�
	DETECTOR_UNKNOWN_FAULT_EVENT = 12,		//δ֪�����¼�
	DETECTOR_CONFIG_FAULT_EVENT = 13,		//���ù����¼�
	DETECTOR_COMMUNICATION_FAULT_EVENT = 14,//ͨ�Ź����¼�
	DETECTOR_NOT_STABLE_EVENT = 15,			//���ȶ��¼�
	DETECTOR_EXIST_TIMEOUT_EVENT = 16,		//����ʱ�����
	DETECTOR_NOT_ACTIVITY_EVENT = 17,		//���
	//�������ڸ�Ӧ��Ȧ�����¼�
	INDUCTOR_COIL_BEYOND_EVENT = 18,		//��Ӧ�仯������
	INDUCTOR_COIL_INSUFFICIENT_EVENT = 19,	//��в���
	INDUCTOR_COIL_OPEN_EVENT = 20,			//��Ȧ��·
	INDUCTOR_COIL_WATCHDOG_FAULT_EVENT = 21,//watchdog����
	INDUCTOR_COIL_UNKNOWN_FAULT_EVENT = 22,	//δ֪����
} GbEventType;

typedef struct gbEventTypeList
{
	UInt8	eventTypeNo;							//�¼����ͱ��(����)����Χ[1,255]
	UInt32	eventTypeClearTime;						//�¼��������ʱ��
	UInt8	eventTypeDescLen;						//�¼�������������
#define	MAX_EVENT_TYPE_DESC_LEN		255				//����¼�������������
	UInt8	eventTypeDesc[MAX_EVENT_TYPE_DESC_LEN];	//�¼���������
	UInt8	eventTypeLineNo;						//����¼��������¼���־���е��к�
} GbEventTypeList;	//�¼������б�

typedef struct gbEventLogList
{
	UInt8	eventTypeNo;		//�¼����ͱ��(����)����Χ[1,255]
	UInt8	eventStreamNo;		//�¼���ˮ��ţ���1��ʼ��ѭ����¼(����)
	UInt32	eventCheckedTime;	//�¼�����⵽��ʱ��
	UInt32	eventValue;			//�¼�ֵ
} GbEventLogList;	//�¼���־�б�

typedef struct gbPhaseList
{
	UInt8	phaseNo;				//��λ��ţ���Χ[1,16]
    UInt8  	pedestrianPassTime;		//���˷���ʱ�䣬�������˹����̵Ƶ�����
    UInt8  	pedestrianClearTime;	//�������ʱ�䣨��������ʱ�䣩
    UInt8  	minGreen;				//��С�̵�ʱ�䣬�����С��
    UInt8  	unitExtendGreen;		//��λ�̵��ӳ�ʱ��,��Ƶ�λ�ӳ���
    UInt8  	maxGreen_1;				//����̵�ʱ��1����������1
    UInt8  	maxGreen_2;				//����̵�ʱ��2����������2
	UInt8	fixGreen;				//������λ�̶��̵�ʱ�䣬��ƹ̶���
	UInt8	greenBlinkTime;			//����ʱ��
	UInt8	phaseType;				/* ��λ�������ԣ�0:False/Disabled��1:True/Enabled
									   bit7:�̶���λ
									   bit6:������λ
									   bit5:������λ
									   bit4:�ؼ���λ
									   bit3-0: Reserved	*/
	UInt8	phaseOption;			/* ��λѡ��ܣ�0:False/Disabled��1:True/Enabled
									   bit7-5: Reserved
									   bit	4:����Ϊ1ʱʹһ��û�м�����������λ������һ��ͬ�׶���ͬʱ���е���λһ�����
									   bit	3:�����Ƿ���������һ�����
									   bit	2:�������λ�Ǵ�����λ��ͬ�׶���λ����ʱ������λ�Ƿ����
									   bit	1:�Ƿ�·���е����˹�����λ
									   bit	0:��λ������־	*/
	UInt8	extendField;			//��չ�ֶΣ���Ϊ��չʹ��
} GbPhaseList;	//��λ�б�

typedef struct gbStatusGroup
{
	UInt8	statusGroupNo;			//״̬���ţ���Χ[1,2]
	UInt8	redStatus;				//������״̬��bit0-7�ֱ�1-8����9-16��λ��1�����źţ�0�����ź�
	UInt8	yellowStatus;			//�Ƶ����״̬
	UInt8	greenStatus;			//�̵����״̬
} GbStatusGroup;	//״̬��

typedef struct gbPhaseConflictList
{
	UInt8	phaseConflictNo;			//��λ��ͻ��ţ���Χ[1,16]
	UInt16	conflictPhase;				//��ͻ��λ��bit0-15�ֱ��ʾ��λ1-16��1:�г�ͻ��0:�޳�ͻ
} GbPhaseConflictList;	//��λ��ͻ�б�

typedef struct gbVehDetectorList
{
	UInt8	detectorNo;					//��������ţ���Χ[1,48]
	UInt8	requestPhase;				//�����������λ��0��ʾ�޶�Ӧ��λ����Χ[0,16]
	UInt8	detectorType;				/*������������� bit7:����������bit6:��Ӧ�������bit5:ս�������
									bit4:ս�Լ������bit3:���˰�ť��bit2:�������������bit1:���г��������bit0:����������� */
	UInt8	detectorDirection;			/*���������bit7:������bit6:����bit5:���ϣ�bit4:��
													  bit3:���ϣ�bit2:����bit1:������bit0:�� */
	UInt8	detectorRequestValidTime;	//�����������Чʱ��
	UInt8	detectorOption;				/*�����ѡ�������bit0:�Ƿ����ֳ��ͣ�bit1:�Ƿ����ڹؼ�������bit2:������bit3:ռ����
														  bit4:�ٶȣ�bit5:�Ŷӳ��ȣ�bit6-7: Reserved */
	UInt16	laneFullFlow;				//��Ӧ�ؼ������ı�������
	UInt8	laneFullRate;				//��Ӧ�ؼ������ı���ռ���ʣ���λ0.5
} GbVehDetectorList;	//�������б�

typedef struct gbDetectorStatusList
{
	UInt8	detectorStatusNo;			//�����״̬�б���
	UInt8	detectorStatus;				//�����״̬��1:��⵽��0:δ��⵽��bit0-7�ֱ��Ӧ��1-48��6��������ÿ��8������������
	UInt8	detectorStatusAlarm;		//�����״̬�������κ�һ�����������������Ѷ�Ӧbit��λ		
} GbDetectorStatusList;		//�����״̬�б�

typedef struct gbTrafficDetectDataList
{
	UInt8	detectorNo;		//�������ţ���Χ[1,48]
	UInt8	totalFlow;		//��������255��ʾ���
	UInt8	largeVehFlow;	//���ͳ�����
	UInt8	smallVehFlow;	//С�ͳ�����
	UInt8	rate;			//ռ���ʣ���λ0.5����Χ[0,200]
	UInt8	speed;			//�ٶȣ���λ:ǧ��km
	UInt8	vehBodyLen;		//�����ȣ���λ:����dm
} GbTrafficDetectDataList;	//��ͨ��������б�

typedef struct gbVehDetectorAlarmList
{
	UInt8	detectorNo;					//�������ţ���Χ[1,48]
	UInt8	detectorAlarmStatus;		//������澯״̬����������ʱ��λ��������ʧʱ����
	UInt8	inductiveCoilAlarmStatus;	//��Ӧ��Ȧ�澯״̬
} GbVehDetectorAlarmList;	//����������澯�б�

typedef	struct gbChannelList
{
	UInt8	channelNo;				//ͨ����ţ���Χ[1,16]
	UInt8	channelRelatedPhase;	//ͨ��������λ
	UInt8	channelFlashStatus;		//�������ʱͨ����״̬
	UInt8	channelControlType;		//ͨ���������ͣ���ֵ��Ӧ��ö��ControllerType
} GbChannelList;	//ͨ���б�

typedef	struct gbSchemeList
{
	UInt8	schemeNo;		//������ţ���Χ[1,32]
	UInt8	cycleTime;		//����ʱ��
	UInt8	phaseGap;		//��λ��
	UInt8	coordinatePhase;//Э����λ
	UInt8	stageTimingNo;	//�׶���ʱ����
} GbSchemeList;		//�����б�

typedef struct gbStageTimingList
{
	UInt8	stageTimingNo;	//�׶���ʱ���ţ���Χ[1,16]
	UInt8	stageNo;		//�׶α�ţ���Χ[1,16]
	UInt16	phaseNo;		//���е���λ
	UInt8	greenTime;		//�׶��̵�ʱ�䣬��������ʱ��
	UInt8	yellowTime;		//�׶λƵ�ʱ�䣬������λ�ź��
	UInt8	allRedTime;		//�׶�ȫ��ʱ��
	UInt8	stageOption;	//�׶�ѡ�����
} GbStageTimingList;	//�׶���ʱ�б�

typedef struct gbFollowPhaseList
{
	UInt8	followPhaseNo;		//������λ��ţ���Χ[1,8]
	UInt8	operateType;		//��������
	UInt8	motherPhaseNum;		//ĸ��λ����
	UInt8	motherPhase[MAX_PHASE_LIST_NUM];	//ĸ��λ
	UInt8	correctPhaseNum;	//������λ����
	UInt8	correctPhase[MAX_PHASE_LIST_NUM];	//������λ
	UInt8	tailGreenTime;		//β���̵�ʱ��
	UInt8	tailYellowTime;		//β���Ƶ�ʱ��
	UInt8	tailAllRedTime;		//β��ȫ��ʱ��
} GbFollowPhaseList;	//������λ�б�

typedef struct gbConfig
{
//���������������Ϣ
	UInt16	pubDevIdentifyParam;		//�����豸ʶ������������ʶ0x81
	UInt8	pubModMaxLineNum;			//����ģ�����������������ʶ0x82
	UInt16	pubSyncTime;				//����ͬ��ʱ�䣬�����ʶ0x83
	UInt16	pubSyncFlag;				//����ͬ����־�������ʶ0x84
#define	MAX_MOD_PARAM_LIST_NUM	16		//���ģ������б����
	GbModParamList modParamTable[MAX_MOD_PARAM_LIST_NUM];	//ģ������������ʶ0x85
	UInt32	pubTime;					//����ʱ�䣬UTC��GMTʱ�䣬��1970/1/1 00:00:00������������������ʶ0x86
	int		standardTimeZone;			//��׼ʱ�������ر�׼ʱ����GMT��ʱ��(��)������ʱ��ʱ��̶�Ϊ8*3600�������ʶ0x87
	UInt32	localTime;					//����ʱ�䣬����1970/1/1 00:00:00������������������ʶ0x88
	UInt8	maxScheduleListNum;			//ʱ�������б���������Ĭ��40�������ʶ0x89
	UInt8	maxTimeIntervalListNum;		//ʱ���б���������Ĭ��16�������ʶ0x8A
	UInt8	maxTimeIntervalNum;			//ÿ��ʱ�α���������ʱ������Ĭ��48�������ʶ0x8B
	UInt8	activeTimeIntervalListNo;	//�ʱ�α��ţ�0��ʾû�л��ʱ�α������ʶ0x8C
	UInt8	maxEventTypeListNum;		//�¼������б��������������ʶ0x8F
	UInt8	maxEventLogListNum;			//�¼���־�б��������������ʶ0x90
	UInt8	maxPhaseListNum;			//�����λ�б������Ĭ��16�������ʶ0x93
	UInt8	maxPhaseStatusGroupNum;		//�����λ״̬�������ÿ��8����λ��Ĭ��2�������ʶ0x94
	UInt8	phaseStatusGroupNum;		//��λ״̬������������ʶ0x96
	UInt8	maxVehDetectorNum;		//����������(�������˰�ť)������Ĭ��48�������ʶ0x98
	UInt8	maxDetectorStatusGroupNum;	//�����״̬���������8�������һ�飬Ĭ��6�������ʶ0x99
	UInt8	detectDateStreamNo;			//���������ˮ�ţ�ÿ���ɼ�����˳���1��ѭ�������������ʶ0x9A
	UInt8	dataCollectCycle;			//���ݲɼ�����(��λ��)�������ʶ0x9B
	UInt8	activeDetectorNum;			//����������,��Χ[0,48]�������ʶ0x9C
	UInt8	pulseDataStreamNo;			//����������ˮ�ţ�ÿ���ɼ�����˳���1��ѭ�������������ʶ0x9D
	UInt8	pulseDataCollectCycle;		//�������ݲɼ�����(��λ��)�������ʶ0x9E
	UInt8	controlStatus;				/*��ǰ�źŻ��Ŀ���״̬��1:δ֪ģʽ,2:ϵͳЭ������,3:����Э������
										4:�ֶ�������,5:ʱ�α����,6:����Э��,�����ʶ0xA5 */
	UInt8	flashControlMode;			//�������ģʽ����Χ[1,7]�������ʶ0xA6
	UInt8	deviceAlarm2;				//�źŻ��豸����2�������ʶ0xA7
	UInt8	deviceAlarm1;				//�źŻ��豸����1�������ʶ0xA8
	UInt8	deviceAlarmSummary;			//�źŻ��豸����ժҪ�������ʶ0xA9
	UInt8	allowRemoteActivate;		//����Զ�̿���ʵ�弤���źŻ���ĳЩ���ܣ������ʶ0xAA
	UInt32	brightnessControlStartTime;	//�Զȿ��ƿ���ʱ�䣬�����ʶ0xAC
	UInt32	brightnessControlEndTime;	//�Զȿ��ƽ���ʱ�䣬�����ʶ0xAD
	UInt8	maxSupportChannelNum;		//�źŻ�֧�ֵ����ͨ��������Ĭ��16�������ʶ0xAE
	UInt8	maxChannelStatusGroupNum;	//ͨ�����״̬����������Ĭ��2�������ʶ0xAF
	UInt8	maxSchemeNum;				//�����õ���󷽰�������Ĭ��32�������ʶ0xB2
	UInt8	maxStageTimingNum;			//�����õ����׶���ʱ������Ĭ��16�������ʶ0xB3
	UInt8	maxStageNum;				//�����õ����׶θ�����Ĭ��16�������ʶ0xB4
	UInt8	manualControlScheme;		//�ֶ����Ʒ����������ʶ0xB5
	UInt8	systemControlScheme;		//ϵͳ���Ʒ����������ʶ0xB6
	UInt8	controlMode;				//���Ʒ�ʽ�������ʶ0xB7
	UInt8	pubCycleTime;				//��������ʱ���������ʶ0xB8
	UInt8	coordinatePhaseGap;			//Э����λ������ʶ0xB9
	UInt8	stageStatus;				//�׶�״̬�������ʶ0xBA
	UInt8	stepCommand;				//����ָ������ʶ0xBB
	UInt8	demotionMode;				//����ģʽ�������ʶ0xBC
	UInt8	demotionStandardSchemeTable[14];	//������׼�����������ʶ0xBD
	UInt8	stageTime[16];				//��ǰ�������׶�ʱ���������ʶ0xBE
	UInt8	keyPhaseGreenTime[MAX_PHASE_LIST_NUM];		//��ǰ�������ؼ���λ�̵�ʱ���������ʶ0xBF
	UInt8	downloadFlag;				//���ر�־��1:���ؿ�ʼ��2:���ؽ����������ʶ0xC2
	UInt8	controlHostOption;			//��������ѡ������������ʶ0xC3
	UInt16	deviceBaseAddr;				//�źŻ��豸����ַ����Χ[0,8192]�������ʶ0xC4
	UInt8	crossingNum;				//·�ڸ�������Χ[1,8]�������ʶ0xC5
	UInt8	maxFollowPhaseListNum;		//������λ�����������Ĭ��8�������ʶ0xC6
	UInt8	maxFollowPhaseStatusNum;	//������λ״̬������Ĭ��1�������ʶ0xC7

	
//��������ص�������Ϣ
#define	MAX_SCHEDULE_LIST_NUM	40		//�������б����
	GbScheduleList scheduleTable[MAX_SCHEDULE_LIST_NUM];	//���ȱ������ʶ0x8D	
#define	MAX_TIMEINTERVAL_LIST_NUM	16	//���ʱ���б����
#define	MAX_TIMEINTERVAL_NUM		48	//���ʱ����
	GbTimeIntervalList timeIntervalTable[MAX_TIMEINTERVAL_LIST_NUM][MAX_TIMEINTERVAL_NUM];	//ʱ�α������ʶ0x8D
	GbPhaseList	phaseTable[MAX_PHASE_LIST_NUM];	//��λ�������ʶ0x95
	GbPhaseConflictList	phaseConflictTable[MAX_PHASE_LIST_NUM];	//��λ��ͻ�������ʶ0x97
#define	MAX_VEH_DETECTOR_NUM	48	//��������������
	GbVehDetectorList vehDetectorTable[MAX_VEH_DETECTOR_NUM];	//����������������ʶ0x9F
	UInt8	bootBlinkTime;				//��������ʱ�䣬�����ʶ0xA3
	UInt8	bootAllRedTime;			//����ȫ��ʱ�䣬�����ʶ0xA4
	UInt8	flashFrequency;				//����Ƶ�ʣ������ʶ0xAB
#define	MAX_CHANNEL_LIST_NUM	16		//���ͨ���б����
	GbChannelList channelTable[MAX_CHANNEL_LIST_NUM];		//ͨ���������ʶ0xB0
#define	MAX_SCHEME_LIST_NUM		32		//��󷽰��б����
	GbSchemeList schemeTable[MAX_SCHEME_LIST_NUM];	//�����������ʶ0xC0
#define	MAX_STAGE_TIMING_LIST_NUM	16	//���׶���ʱ�б����
#define	MAX_STAGE_NUM				16	//���׶���
	GbStageTimingList stageTimingTable[MAX_STAGE_TIMING_LIST_NUM][MAX_STAGE_NUM];	//�׶���ʱ�������ʶ0xC1
#define	MAX_FOLLOW_PHASE_LIST_NUM	8	//��������λ�����
	GbFollowPhaseList followPhaseTable[MAX_FOLLOW_PHASE_LIST_NUM];	//������λ�������ʶ0xC8	


//���й����м�¼�Ĺ��ϻ�����־��Ϣ
#define	MAX_EVENT_TYPE_LIST_NUM		255	//����¼������б����
	GbEventTypeList eventTypeTable[MAX_EVENT_TYPE_LIST_NUM];//�¼����ͱ������ʶ0x91
#define	MAX_EVENT_LOG_LIST_NUM		255	//����¼���־�б����
	GbEventLogList eventLogTable[MAX_EVENT_TYPE_LIST_NUM][MAX_EVENT_LOG_LIST_NUM];//�¼���־�������ʶ0x92
	

//һЩֻ��Ҫ��ȡ��״̬��Ϣ
#define MAX_PHASE_STATUS_GROUP_NUM	2	//�����λ״̬�����
	GbStatusGroup phaseStatusTable[MAX_PHASE_STATUS_GROUP_NUM];	//��λ״̬��
#define	MAX_CHANNEL_STATUS_GROUP_NUM	2	//���ͨ�����״̬�����
	GbStatusGroup channelStatusTable[MAX_CHANNEL_STATUS_GROUP_NUM];	//ͨ�����״̬�������ʶ0xB1
#define MAX_FOLLOW_PHASE_STATUS_GROUP_NUM	1	//��������λ״̬�����
	GbStatusGroup followPhaseStatusTable[MAX_FOLLOW_PHASE_STATUS_GROUP_NUM];	//������λ״̬�������ʶ0xC9
#define MAX_DETECTOR_STATUS_LIST_NUM 6	//�������״̬�б����
	GbDetectorStatusList detectorStatusTable[MAX_DETECTOR_STATUS_LIST_NUM];	//�����״̬�������ʶ0xA0	
	GbTrafficDetectDataList trafficDetectDataTable[MAX_VEH_DETECTOR_NUM];	//��ͨ������ݱ������ʶ0xA1
	GbVehDetectorAlarmList vehDetectorAlarmTable[MAX_VEH_DETECTOR_NUM];	//����������澯�������ʶ0xA2
} GbConfig;

typedef struct gbObject
{
	UInt32	baseOffset;					//����ƫ����
	UInt16	size;						//��ռ��С
	Boolean	isCheck;					//�Ƿ���
	UInt32	minValue;					//��Сֵ
	UInt32	maxValue;					//���ֵ
} GbObject;

typedef void (*GbObjectDealFunc)(GbOperateType type);

typedef struct gbObjectIdentify
{
	GbObject 	object;					//������
	UInt8		objectId;				//�����ʶ
	UInt16		maxIndex;				//�����������
	UInt8		childObjectNum;			//�Ӷ������
#define	MAX_CHILD_OBJECT_NUM	16		//����Ӷ������
	GbObject	childObject[MAX_CHILD_OBJECT_NUM];	//�Ӷ���
	UInt8		ntcipUpdateBit;					//��Ӧ��NTPIP�ṹ��ĸ���bitλ
	GbObjectDealFunc func;
} GbObjectIdentify;

#pragma pack(pop)
#endif
