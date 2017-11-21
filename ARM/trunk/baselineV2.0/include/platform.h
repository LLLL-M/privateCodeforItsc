#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"
#include "HikConfig.h"
	   
#pragma pack(push, 4)
//����ʱ�ӿڻ�ȡ���
typedef struct STRU_Extra_Param_Get_Phase_Counting_Down
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0x9E��ʾ��ȡ����ʱ�ӿ�
}GET_PHASE_COUNTING_DOWN_PARAMS;    


//����ʱ�ӿڻ�ȡ������
typedef struct STRU_Extra_Param_Phase_Counting_Down_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0x9E��ʾ����
     unsigned short    stVehPhaseCountingDown[16][2];    //16����������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     							//��1�б�ʾ��ǰ��������λ������λ����ʱʱ��
     unsigned short    stPedPhaseCountingDown[16][2];    //16��������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
								//��1�б�ʾ��ǰ������λ������λ����ʱʱ��
     unsigned char    ucPlanNo;				//��ǰ���з�����
     unsigned short   ucCurCycleTime;		//��ǰ�������ڳ�	!!!!!!!!!!
     unsigned short   ucCurRunningTime;		//��ǰ����ʱ��		!!!!!!!!!!
     unsigned char    ucChannelLockStatus;		//ͨ���Ƿ�����״̬��1��ʾͨ��������0��ʾͨ��δ����
     unsigned char    ucCurPlanDsc[16];			//��ǰ���з�������
     unsigned short    ucOverlap[16][2];                    //������λ״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     unsigned short    stPhaseRunningInfo[16][2];	//����λ����ʱ������ű�
							//��0�б�ʾ����λ���űȣ���1�б�ʾ����λ����ʱ�䣬�̵�������1�в�����ֵ������Ϊ0
     unsigned char    ucChannelStatus[32]; /*32��ͨ����ǰ״̬
											0:����Ч
											1:�̵�
											2:���
											3:�Ƶ�
											4:����
											5:����
											6:ȫ��
											7:���
											8:����
											9:���
											*/
	 unsigned short	  ucChannelCountdown[32];	//ʵ�ʵ�ͨ������ʱ			++++++
	 unsigned char	  ucIsStep;			  //�Ƿ񲽽���0:������,1:���ڲ���	++++++
	 unsigned char	  ucMaxStageNum;	  //���׶κ�						++++++	
}PHASE_COUNTING_DOWN_FEEDBACK_PARAMS;


//ͨ���������
typedef struct STRU_Extra_Param_Channel_Lock
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xb9��ʾ����ͨ������
     unsigned char    ucChannelStatus[32];		//32��ͨ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������, 6��ȫ�죬 7����ǰͨ������Ч��						
     unsigned char    ucWorkingTimeFlag;		//ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
     unsigned char    ucBeginTimeHour;			//������Чʱ�䣺Сʱ
     unsigned char    ucBeginTimeMin;			//������Чʱ�䣺����
     unsigned char    ucBeginTimeSec;			//������Чʱ�䣺��
     unsigned char    ucEndTimeHour;			//���ƽ���ʱ�䣺Сʱ
     unsigned char    ucEndTimeMin;			//���ƽ���ʱ�䣺����
     unsigned char    ucEndTimeSec;			//���ƽ���ʱ�䣺��
     unsigned char    ucReserved;			//Ԥ��
}CHANNEL_LOCK_PARAMS;    

typedef struct STRU_Extra_Param_Channel_Lock DemotionParams;
//ͨ������������
typedef struct STRU_Extra_Param_Channel_Lock_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xb9��ʾ����ͨ������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}CHANNEL_LOCK_FEEDBACK_PARAMS;    


//ͨ���������
typedef struct STRU_Extra_Param_Channel_Unlock
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xba��ʾ�ر�ͨ������
}CHANNEL_UNLOCK_PARAMS;    

//ͨ������������
typedef struct STRU_Extra_Param_Channel_Unlock_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xba��ʾ�ر�ͨ������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}CHANNEL_UNLOCK_FEEDBACK_PARAMS;    


//�źŻ�������ƣ�
typedef struct STRU_Extra_Param_Special_Ctrl
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbb��ʾ�ֶ������������
     unsigned int     unSpecialCtrlNo;			//������Ʒ�ʽ��0��ϵͳ���ƣ�255��������254����Ӧ��252��ȫ�죬251���صƣ��������ֶ�����
}SPECIAL_CTRL_PARAMS;    

//�źŻ�������Ʒ�����
typedef struct STRU_Extra_Param_Special_Ctrl_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbb��ʾ�ֶ������������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}SPECIAL_CTRL_FEEDBACK_PARAMS;    



//�źŻ��������ƣ�SDK�յ�ƽ̨�����ù��߲�������ʱ���յ������ɹ���������������ⷢ�͹ص�ָ���
typedef struct STRU_Extra_Param_Step_Ctrl
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbc��ʾ������������
     unsigned int     unStepNo;				//�����׶κţ���ת������Ч��ΧֵΪ1-16����ʾ������1-16�׶Σ�0Ϊ��������
}STEP_CTRL_PARAMS;    

//�źŻ��������Ʒ�����
typedef struct STRU_Extra_Param_Step_Ctrl_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbc��ʾ������������
     unsigned int     unStepNo;				//1��ʾ�����ɹ���0��ʾ����ʧ��
}STEP_CTRL_FEEDBACK_PARAMS;

//�źŻ�ȡ��������SDK�յ�ƽ̨�����ù���ȡ����������ʱ���յ�ȡ�������ɹ����������������ϵͳ����ָ�����
typedef struct STRU_Extra_Param_Cancel_Step_Ctrl
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbd��ʾȡ��������������
}CANCEL_STEP_CTRL_PARAMS;    

//�źŻ�ȡ������������
typedef struct STRU_Extra_Param_Cancel_Step_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbd��ʾȡ��������������
     unsigned int     unValue;				//1��ʾȡ�������ɹ���0��ʾȡ������ʧ��
}CANCEL_STEP_FEEDBACK_PARAMS;   


//�������źŻ��ͺ����ã��������ù��ߺ�ƽ̨���öԾ������źŻ����ͺŽ������ã����ھ������źŻ�������ʱ��Ч����
typedef struct STRU_Extra_Param_Set_Device_Type
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbe��ʾ�����豸�ͺ�
     unsigned int     unResult;				//���þ������źŻ�����,1:DS-TSC300-44,2:DS-TSC300-22
}SET_DEVICE_TYPE_PARAMS;  


typedef struct STRU_Extra_Param_Set_Device_Type_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbe��ʾ�����豸�ͺ�
     unsigned int     unResult;				//����ֵ,0:����ʧ��    1�����óɹ�
}SET_DEVICE_TYPE_FEEDBACK_PARAMS;  

//��ȡ�������źŻ��ͺţ��������ù��ߺ�ƽ̨��ȡ�������źŻ����ͺţ����ھ������źŻ��ϻ�ȡʱ����Ч����
typedef struct STRU_Extra_Param_Get_Device_Type
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbf��ʾ��ȡ�豸�ͺ�
}GET_DEVICE_TYPE_PARAMS;  


typedef struct STRU_Extra_Param_Get_Device_Type_Feedback
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xbf��ʾ��ȡ�豸�ͺ�
     unsigned int     unResult;				//���ؾ������źŻ�����,0����ȡʧ�ܣ�1:DS-TSC300-44,2:DS-TSC300-22
}GET_DEVICE_TYPE_FEEDBACK_PARAMS; 

typedef struct STRU_Extra_Param_Response
{
	UInt32	unExtraParamHead;		//��Ϣͷ��־
	UInt32	unExtraParamID;			//��Ϣ����ID
	UInt32 	unExtraParamValue;		//����������
	UInt32 	unExtraParamFirst;		//��ʼ��
	UInt32 	unExtraParamTotal;		//������
	UInt32  unExtraDataLen;			//�����ܳ���
	UInt8	data[0];				//���ز��������ݽṹ��
} MsgFaultInfo;	//���ز����ظ��Ľṹ��

/***************����״̬��Ϣ����ע���·���״̬��Ϣ��������Ϣ�壩
***************************************/
//��Ϣ���ͣ�	0x71 -- ���ض�ʱ��0x72 -- ���ض�ʱ
//��Ϣ�壺
struct SAdjustTime
{
	unsigned long			ulGlobalTime;		// UTCʱ��
	unsigned int			unTimeZone;			//ʱ��
};


//��Ϣ���ͣ�0x159 -- ��ȡͨ��״̬
//��Ϣ�壺STRU_N_ChannelStatusGroup AscChannelStatusTable[4];
//ͨ��״̬�鶨��
struct STRU_N_ChannelStatusGroup
{
	/*2.9.4.1 channelStatusGroup����кţ����ܴ���
maxChannelStatusGroups��(1..4)*/
	UInt8  byChannelStatusGroupNumber;

	/*2.9.4.2ͨ���ĺ�����״̬(bit=1��ʾ���bit=0��ʾ���)	Bit
7: ͨ���� = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: ͨ���� = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupReds;	
	/*2.9.4.3ͨ���ĻƵ����״̬(bit=1��ʾ���bit=0��ʾ���)	Bit
7: ͨ���� = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: ͨ���� = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupYellows;

	/*2.9.4.4ͨ�����̵����״̬(bit=1��ʾ���bit=0��ʾ���)	Bit
7: ͨ���� = (channelStatusGroupNumber * 8) - 0	
	||Bit 0: ͨ���� = (channelStatusGroupNumber * 8) - 7 (0..255)*/
	UInt8  byChannelStatusGroupGreens;
};

//��Ϣ���ͣ�0x157 -- ��ȡ��λ״̬
//��Ϣ�壺STRU_N_PhaseStatusGroup AscPhaseStatusTable[2]; 
struct STRU_N_PhaseStatusGroup {
	UInt8	byPhaseStatusGroupNumber;/*2.2.4.1��λ״̬��ţ����ܳ������
��λ��ֵ*/
	UInt8	byPhaseStatusGroupReds;	/*2.2.4.2����ÿ������8����λ�ĺ��״
̬����1Ϊ��*/
	UInt8	byPhaseStatusGroupYellows;/*2.2.4.3����ÿ������8����λ�ĻƵ�
״̬����1Ϊ�ƻ*/
	UInt8	byPhaseStatusGroupGreens;/*2.2.4.4����ÿ������8����λ���̵�
״̬����1Ϊ�̻*/
	UInt8	byPhaseStatusGroupDontWalks;/*2.2.4.5��ֹ����״̬����1Ϊ����
������*/
	UInt8	byPhaseStatusGroupPedClears; /*2.2.4.6�������״̬����1Ϊ��
����շ���*/
	UInt8	byPhaseStatusGroupWalks;/*2.2.4.7����״̬����1Ϊ���˷���*/
	UInt8	byPhaseStatusGroupVehCalls;/*2.2.4.8����������״̬����1Ϊ��
��λ�л���������*/
	UInt8	byPhaseStatusGroupPedCalls;/*2.2.4.9��������״̬����1Ϊ����
λ����������*/
	UInt8	byPhaseStatusGroupPhaseOns;/*2.2.4.10��λ�״̬����1Ϊ����
λ���ڻ״̬*/
	UInt8	byPhaseStatusGroupPhaseNexts;/*2.2.4.11��һ����λ��״̬*/
};

//��Ϣ���ͣ�0x2B -- ��ȡ������λ״̬
//��Ϣ�壺STRU_N_OverlapStatusGroup AscOverlapStatusTable[2];
//�ص�״̬�鶨��
struct STRU_N_OverlapStatusGroup
{
	UInt8 byOverlapStatusGroupNumber;	/*2.10.4.1 Status Group�ţ�
���ܳ���maxOverlapStatusGroups��1..2*/
	UInt8 byOverlapStatusGroupReds;		/*2.10.4.2  overlap������
״̬��־��*/
	UInt8 byOverlapStatusGroupYellows;	/*2.10.4.3 overlap�Ƶ����״
̬��־��*/
	UInt8 byOverlapStatusGroupGreens;	/*2.10.4.4 overlap�̵����״
̬��־��*/
};

//��Ϣ���ͣ�0x15A -- ��ȡͬ��״̬
//��Ϣ�壺
struct STRU_CoordinateStatus
{
	UInt8	byCoordPatternStatus;		/*2.5.10 Э������״̬
				Not used��0��
				Pattern -��1-253����ǰ���з�����
				Free - (254)��Ӧ
				Flash - (255)����*/
	UInt16	wCoordCycleStatus;		/* 2.5.12 Э������������״̬
��0-510sec���������ڳ�һֱ���ٵ�0*/
	UInt16 	wCoordSyncStatus;			/*2.5.13Э��ͬ��״̬
��0��510����Э����׼�㵽Ŀǰ���ڵ�ʱ�䣬��0��¼���¸����ڻ�׼�㡣���Գ�����
�ڳ�*/

	UInt8	byUnitControlStatus;			/*2.4.5��ǰ���źŻ�
����״̬
				Other(1): δ֪ģʽ��
				systemControl(2): ϵͳЭ�����ơ�
				systemStandby(3): ����Э�����ơ�
				backupMode(4):����ģʽ
				manual(5): �ֶ������ơ�
				Timebase(6): ʱ�α���ƣ�������ʱ�α���
�У���
				Interconnect(7): ����Э����
				interconnectBackup(8):����Э��*/
};

//����ռ���ʶ���
typedef struct STRU_N_VolumeOccupancy
{
    /*�ṩ��λС���㾫��֧�֣�ʵ��ֵ����100����д洢�����û���ʾʱ����Ҫ����100������ʵ��ֵ*/
	BYTE    byDetectorVolume;			/*2.3.5.4.1һ���������һ�������вɼ���������,��λ�� ��/����*/
	WORD    byDetectorOccupancy;		/*2.3.5.4.2һ���������һ�������е�ռ���� ���������ṩ����ʱ��ռ���ʣ�����10000���õ��ٷֱ�ֵ*/
	WORD	byVehicleSpeed;		        /*����,����*100�洢�ģ�����ʵ��ֵӦ�ó���100�󣬵�λ����km/h*/
	WORD	wQueueLengh;		        /*�Ŷӳ���,����*100�洢������100�󣬲��ܵõ�ʵ�ʵĳ���������λ����*/
	WORD    wGreenLost;                 /*���� ��ʧ�̵�ʱ��/�̵���ʱ�� ����ֵ����100����*/
	WORD    wVehicleDensity;            /*�����ܶȣ�����*100�洢������100��λ�� ��/km*/
	WORD    wVehicleHeadDistance;       /*��ͷ��࣬ͬһ������ͬһ����������ʻǰ������������֮����ĳһ˲ʱ�ĳ�ͷ��ࡣ���ţ�hd ��λ��m/����*/
    WORD    wVehicleHeadTimeDistance;   /*��ͷʱ�ࣺͬһ������ͬһ����������ʻǰ�����ڵ��������ĳ�ͷͨ��ĳһ���ʱ���������ţ�ht ��λ��s/�� */
} VolumeOccupancy;	

//��ʷ������ʱ���Ӧ�ṹ
typedef struct STRU_TimeAndHistoryVolume
{
    DWORD dwTime;                       //��ʷʱ���
    VolumeOccupancy  struVolume[48]; //��Ӧ��ʷ����
} TimeAndHistoryVolume;

typedef struct
{
	UINT8 greenBlink;	//����ʱ��
	UINT8 yellow;		//�Ƶ�ʱ��
	UINT8 allred;		//ȫ��ʱ��
} PhaseTime;
//���ڷ�������,��������ÿ�����ڵĿ�ʼ���и���,�����źŻ���ǰ���з�����������Ϣ��
typedef struct
{
	UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd0
	
	UINT8 nPatternId;						//��ǰ���з����ţ�����ID���Ӵ�1��ʼ��������4,7������ת����
	UINT8 nSplitId;							//��ǰ�������űȱ��
	UINT8 nPhaseTurnId;						//��ǰ����������
	UINT16 nOffset;							//��ǰ������λ��	!!!!!!!!!!!!!!!!!!!!!
	PhaseTurnItem sPhaseTurn[4];			//��ǰ���е��ĸ���������,�±�0����1����������
	UINT32 nCoordinatePhase;				//��ǰ����ʹ�õ�Э����λ,bit0������λ1����ֵΪ0��ʾ����Э����λ��Ϊ1��ʾ����λ��ΪЭ����λ��
	PhaseTime phaseTime[16];
	FollowPhaseItem stFollowPhase[16];      //������λ										
	UINT8 ucIsCurrentCycle;		//�Ƿ��ǵ�ǰ����,0:����,1:��	++++++++++++++
	char phaseDesc[16][64];	//��ǰʹ����λ��ĸ���λ����
    UINT8 nPhaseTableId;
    UINT8 nChannelTableId;	
} MsgRealtimePattern;//��������

typedef struct
{
	UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd2
	UINT32 unExtraDataLen;					//ʣ������ݳ���
	UINT8 nPhaseArray[16];					//�źŻ����õ�������λ����λ��
	UINT8 nPatternArray[108];				//�źŻ����õ����з����ţ�����ת��	
} MsgPhaseSchemeId;//��ƽ̨1049Э��ʹ��

//ʵʱ������Ϣͨ������Ľṹ���ȡ����ʷ������Ϣͨ��sftp��ȡָ�����ļ�����,���ļ��Ķ����Ƹ�ʽ��TimeAndHistoryVolume��
typedef struct
{
	UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd1
	
	TimeAndHistoryVolume volumeOccupancy;		//����ռ���ʵȽṹ��	
} MsgRealtimeVolume;

//�����������͵�ͨ������ʱ��Ϣ
typedef struct {
    UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd3
    
    UINT8 nChannelStatus[32][2];            //32��ͨ���ĵ���ʱֵ����ɫ״̬    
                //��0�б�ʾ��ǰͨ����״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
                //��1�б�ʾ��ǰͨ����������ʱʱ��
    UINT8 nReserved[256];				//Ԥ���ֽ�
}MsgChannelCountDown;

#define MAX_CHAN_LOCK_PERIODS	16
typedef struct 
{
	unsigned char	 ucChannelStatus[32];	   //32��ͨ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������, 6��ȫ�죬 7����ǰͨ������Ч�� 					   
	unsigned char	 ucWorkingTimeFlag; 	   //ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
	unsigned char	 ucBeginTimeHour;		   //������Чʱ�䣺Сʱ
	unsigned char	 ucBeginTimeMin;		   //������Чʱ�䣺����
	unsigned char	 ucBeginTimeSec;		   //������Чʱ�䣺��
	unsigned char	 ucEndTimeHour; 		   //���ƽ���ʱ�䣺Сʱ
	unsigned char	 ucEndTimeMin;		   //���ƽ���ʱ�䣺����
	unsigned char	 ucEndTimeSec;		   //���ƽ���ʱ�䣺��
	unsigned char	 ucReserved;		   //Ԥ��
}STRU_CHAN_LOCK_PARAMS;

typedef struct 
{
	UINT8					cLockFlag;	
	STRU_CHAN_LOCK_PARAMS	chans[MAX_CHAN_LOCK_PERIODS];
}STU_MUL_PERIODS_CHAN_PARAMS;

/*��Э��ר�������ṩ��SDK��ȡ�źŻ������ͺ���ʹ�õ�ͨ�Ŷ˿ڣ�
 * ��Э��ֻ��ͨ��udp 20000�˿ڻ�ȡ�������Ժ���źŻ����س���Ҫ����20000�˿�ר������ʵ�ִ�Э��*/
struct STRU_N_PatternNew	//����Ϊ223,��0xdf
{
	UINT16	wProtocol;		//�źŻ�Э�����ͣ�1-NTCIP��2-HIK��3-GB
	UINT16	wDscType;		//�źŻ��ͺţ�0:500�ͣ�1:300-44�ͣ�2:300-22��
	UINT32	unPort;			//ͨѶ�˿�
};


#pragma pack(pop)

#endif

