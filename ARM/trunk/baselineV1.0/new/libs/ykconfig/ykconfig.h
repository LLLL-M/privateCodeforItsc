#ifndef __YKCONFIG_H__
#define __YKCONFIG_H__


#ifndef NUM_PHASE
#define NUM_PHASE					16	//�����λ����
#endif
#ifndef NUM_CHANNEL
#define NUM_CHANNEL					32	//���ͨ������
#endif
#ifndef NUM_SCHEDULE
#define NUM_SCHEDULE				40	//�����ȱ����
#endif
#ifndef NUM_TIME_INTERVAL
#define NUM_TIME_INTERVAL           16  //ʱ�α�����
#endif
#ifndef NUM_TIME_INTERVAL_ID
#define NUM_TIME_INTERVAL_ID    	48  //ÿ��ʱ�α��е�ʱ�κ�����
#endif
#ifndef	NUM_SCHEME
#define NUM_SCHEME					16	//��󷽰�����
#endif


#ifndef __HIK_H__
typedef unsigned char 		UInt8;
typedef unsigned short 		UInt16;
typedef unsigned int		UInt32;
typedef unsigned long long	UInt64;

typedef enum
{
	FALSE = 0,
	TRUE,
} Boolean;

#endif

#ifndef __HIKCONFIG_H__
typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//�ƻ�ʱ��

#define BIT(v, n) (((v) >> (n)) & 0x1)		//ȡv�ĵ� n bitλ

#endif

//Э��ͷ���ṹ��ͷ������������涨������ݽṹ��,Ҳ����ֵ�򲿷�
typedef struct
{
	UInt32 iHead;		//����Ϊ0x7e7e������Ϊ0x8e8e
	UInt32 iType;		//���嶨��������ʾ
	UInt32 iValue[0];	//ֵ�򣬾�������Ϊ������ʾ�Ĳ����ṹ��,�˳�Ա��ռ�ô洢�ռ䣬���windows�˲�֧��0���鶨�壬����ע�͵�
} YK_ProtocolHead;

/***********************************************************************************
�������ص��ȱ�ʱ�α��������Լ����������⼸�����ñ���ʱ��Ҫ�ȷ��������ؿ�ʼ�ͽ�����־������������ʾ��
��ʼ��־:	iType = 0x7e00
������־:	iType = 0x7eff
�������贫���κ��������ͣ�ֻ��Ҫ����iHead��iType�Ϳ��ԣ�����ֵ�򲿷�
***********************************************************************************/

/*���е����ط��ض�Ӧ�����ݽṹ���������򷵻�12�ֽڣ�����iHead��iType��һ��4�ֽڷ���ֵ(Ҳ����iValue[0]),
����ֵΪ0��ʾ��������֮���쳣,�����ڷ������ؿ�ʼ�ͽ�����־��Ϣʱ����ֵ������ʱ����ֵһ��*/

/*
iType		����
0x7f01		���ȱ�
0x7f02		ʱ�α�
0x7f03		ͨ����
0x7f04		������
0x7f05		��������
0x7f06		��������Ϣ��
0x7f07		ͨ������
0x7f08		ʵʱ״̬��Ϣ
0x7f09		�ֶ�����
0x7f0a		������־
0x7f0b		��ʱ
0x7f0c		��������
*/



//���ȱ�ṹ��iTypeΪ0x7f01
typedef struct 
{
    UInt16  nScheduleID;    /*���ȼƻ��ţ���Χ[1,40],������ʱ����Ҫָ��һ�����*/
	UInt16  month;	  /* bit1-bit12��ÿλ��ʾһ���¡���1��ʾ�����Ӧ�ƻ��ڸ���ִ��*/
	UInt8   week;	/* bit1-bit7�ֱ��Ӧ�����ա�����1��6��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ��*/
	UInt32  day;	/* bit1-bit31��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ��*/
    UInt8   nTimeIntervalID;   /*ʱ�α�ţ���Χ[1,16]*/
} YK_PlanScheduleItem;

//ʱ�α�ṹ��iTypeΪ0x7f02
typedef struct
{
    UInt8   nTimeIntervalID;                 /*ʱ�α��,��Χ[1,16],������ʱ����Ҫָ��һ�����*/
    UInt8   nTimeID;                        /*ʱ�κţ���Χ[1,48],������ʱ����Ҫָ��һ�����*/
    UInt8   cStartTimeHour;                  /*��ʼִ��ʱ�̵�����������ʱ�䣨24ʱ�ƣ���*/
    UInt8   cStartTimeMinute;                /*��ʼִ�е�������*/
    UInt8   nSchemeId;                       /*��ʱ������*/
	UInt16	phaseOffset;					 /*��λ��*/
	UInt8	IsCorrdinateCtl;				 /*�Ƿ���Э������,0:��Э����1:Э��*/
} YK_TimeIntervalItem;

//ͨ����ṹ�壬iTypeΪ0x7f03
typedef struct
{
    UInt8 nChannelID;                        /*ͨ���ţ���Χ[1,32],������ʱ����Ҫָ��һ�����*/
    UInt8 nControllerType;                   /*ͨ���������ͣ�������ͨ��ֵΪ2������ͨ��ֵΪ3������ֵ��Ч*/
    UInt8 nFlashLightType;                   /*�Զ�����״̬��Ĭ��Ϊ2
                                        	Bit 7-4: Reserved
                                			Bit 3: ��������
                                				Bit=0: Off/Disabled & Bit=1: On/Enabled
                                			Bit 2: ����
                                				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
                                			Bit 1: ����
                                				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
                                			Bit 0: Reserved
                                        	Bit 1 �� Bit 2 ͬʱΪ1��Ч����Bit1 = 0��Bit2 = 1*/
	UInt8 nVehDetectorNum;					/*ͨ����Ӧ�ĳ��������*/
	UInt32 conflictChannel;					/*��ͻͨ����bit0-31�ֱ����ͨ��1-32*/
} YK_ChannelItem;

typedef struct
{	//��������ʱ�䵥λͳһ��Ϊ��
	UInt8 greenTime;			//�̵�ʱ�䣬��1��
	UInt8 greenBlinkTime;		//����ʱ�䣬��2��
	UInt8 yellowTime;			//�Ƶ�ʱ�䣬��3��
	UInt8 redYellowTime;		//���ʱ�䣬��4��	!!!!!!!!!!!!!!!!!!!!!!!!!
	UInt8 allRedTime;			//ȫ��ʱ�䣬��5��	!!!!!!!!!!!!!!!!!!!!!!!!!
	//UInt8 pedestrianClearTime;	//�������ʱ�䣬����������ʱ��	++++++++++++++++
	UInt8 minGreenTime;			//��С��
	UInt8 maxGreenTime;			//�����
	UInt8 unitExtendTime;		//��λ�ӳ���
	UInt32 channelBits;			//��λ������ͨ����bit0-bit31�ֱ����ͨ��1-32*/
} YK_PhaseInfo;

//������ṹ��iTypeΪ0x7f04
typedef struct
{
	UInt8 nSchemeId;					//������,��Χ[1,16],������ʱ����Ҫָ��һ�����
	UInt16 cycleTime;					//����ʱ�䣬��λΪ��
	UInt8 totalPhaseNum;				//������������λ����,���16��
	YK_PhaseInfo phaseInfo[NUM_PHASE];	//������������λ��Ϣ����������������ʾ
} YK_SchemeItem;

//�źŻ��������ýṹ��iTypeΪ0x7f05
typedef struct
{
	UInt8 bootYellowBlinkTime;		//��������ʱ��
	UInt8 bootAllRedTime;			//����ȫ��ʱ��
	UInt8 vehFlowCollectCycleTime;	//�������ɼ�����,Ĭ��5s
	UInt8 transitionCycle;			//�̲�����ʱ�Ĺ������ڸ���
	UInt8 adaptCtlEndRunCycleNum;	//����Ӧ��Ӧ���ƽ������е����ڸ���
	UInt8 watchdogEnable:1;			//watchdogʹ�ܿ���
	UInt8 signalMachineType:2;		//�źŻ�����ѡ��,�������źŻ�TSC500��TSC300-44��TSC300-22,��Ӧ��ֵ�ֱ�Ϊ0��1��2
	UInt8 weekPreemptSchedule:1;	//�������ȵ��ȣ�Ĭ���������ȵ��� ++++++++++++++
} YK_WholeConfig;


//����ռ���ʽṹ
typedef struct 
{
    /*�ṩ��λС���㾫��֧�֣�ʵ��ֵ����100����д洢�����û���ʾʱ����Ҫ����100������ʵ��ֵ*/
	UInt8    byDetectorVolume;			/*2.3.5.4.1һ���������һ�������вɼ���������,��λ�� ��/����*/
	UInt16    byDetectorOccupancy;		/*2.3.5.4.2һ���������һ�������е�ռ���� ���������ṩ����ʱ��ռ���ʣ�����100���õ��ٷֱ�ֵ*/
	UInt16	byVehicleSpeed;		        /*����,����*100�洢�ģ�����ʵ��ֵӦ�ó���100�󣬵�λ����km/h*/
	UInt16	wQueueLengh;		        /*�Ŷӳ���,����*100�洢������100�󣬲��ܵõ�ʵ�ʵĳ���������λ����*/
	UInt16    wGreenLost;                 /*���� ��ʧ�̵�ʱ��/�̵���ʱ�� ����ֵ����100����*/
	UInt16    wVehicleDensity;            /*�����ܶȣ�����*100�洢������100��λ�� ��/km*/
	UInt16    wVehicleHeadDistance;       /*��ͷ��࣬ͬһ������ͬһ����������ʻǰ������������֮����ĳһ˲ʱ�ĳ�ͷ��ࡣ���ţ�hd ��λ��m/����*/
    UInt16    wVehicleHeadTimeDistance;   /*��ͷʱ�ࣺͬһ������ͬһ����������ʻǰ�����ڵ��������ĳ�ͷͨ��ĳһ���ʱ���������ţ�ht ��λ��s/�� */
} YK_VolumeOccupancy;	

//��ʷ������ʱ���Ӧ�ṹ��iTypeΪ0x7f06��ֻ�ṩ����
typedef struct 
{
    UInt32 dwTime;                       //��ʷʱ���
    YK_VolumeOccupancy  struVolume[48]; //��Ӧ��ʷ����
} YK_TimeAndHistoryVolume;

//ͨ�������ṹ��iTypeΪ0x7f07,ֻ�ṩ����
typedef struct
{
	unsigned char    ucChannelStatus[NUM_CHANNEL];		//32��ͨ��״̬��������ö��LightStatus��ʾ
	unsigned char    ucBeginTimeHour;			//������Чʱ�䣺Сʱ
	unsigned char    ucBeginTimeMin;			//������Чʱ�䣺����
	unsigned char    ucBeginTimeSec;			//������Чʱ�䣺��
	unsigned char    ucEndTimeHour;			//���ƽ���ʱ�䣺Сʱ
	unsigned char    ucEndTimeMin;			//���ƽ���ʱ�䣺����
	unsigned char    ucEndTimeSec;			//���ƽ���ʱ�䣺��
} YK_ChannnelLockParams;

//ʵʱ״̬�ṹ��iTypeΪ0x7f08��ֻ�ṩ����
typedef struct
{
	UInt8 allChannels[NUM_CHANNEL];	//����ͨ����״̬��������ö��LightStatus��ʾ
	UInt16 cycleTime;				//��ǰ������ʱ��
	UInt8 runPhase;					//��ǰ������λ
	UInt8 schemeId;                 //��ǰ���еķ�����
	UInt8 stepNum;					//��ǰ��λ���еĲ����
	UInt8 stepLeftTime;				//��ǰ���в���ʣ��ʱ��
} YK_RealTimeInfo;

//�ֶ����ƽṹ��iTypeΪ0x7f09��ֻ�ṩ����
typedef UInt8 YK_ManualControl;		//����Ҫִ�еķ�����������궨����ʾ

/*������־��Ϣ�ṹ��ÿ���ṹ����һ����־��һ��������16����
����������ýṹ��nTime==0��˵��û����־�ɹ����ش�ʱ��Ӧ����ֹ����������־��Ϣ��
iTypeΪ0x7f0a,�����ṩ���������־���ܣ�����������أ����Դ�ʱiHeadӦΪ0x8e8e*/


//typedef FaultLogInfo YK_FaultLog[16];


//��ʱ�ṹ��iTypeΪ0x7f0b
struct YK_SAdjustTime
{
	unsigned long			ulGlobalTime;		//UTCʱ��,����Ϊ��λ
	unsigned int			unTimeZone;			//ʱ��,����Ϊ��λ
};

//�������ýṹ��ͳһΪ�����ֽ���iTypeΪ0x7f0c
typedef struct
{
	UInt32 ip;
	UInt32 netmask;
	UInt32 gateway;
} YK_NetworkParam;

typedef struct
{
	YK_PlanScheduleItem scheduleTable[NUM_SCHEDULE];
	YK_TimeIntervalItem timeIntervalTable[NUM_TIME_INTERVAL][NUM_TIME_INTERVAL_ID];
	YK_ChannelItem channelTable[NUM_CHANNEL];
	YK_SchemeItem schemeTable[NUM_SCHEME];
	YK_WholeConfig wholeConfig;
} YK_Config;

#endif
