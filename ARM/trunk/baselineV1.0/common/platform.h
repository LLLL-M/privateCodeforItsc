#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"

#include "common.h"
#include "HikConfig.h"

#define MSG_RED_SIGNAL_CHECK	111

#define MSGSIZE (sizeof(struct msgbuf) - sizeof(long))

typedef enum
{
	STEP_START = 0,	//������ʼ
	STEP_EXCUTE,	//����ִ��
	STEP_END,		//��������
} StepCmd;

struct msgbuf
{
	long mtype;
	StepCmd cmd;
	UInt8 stageNo;
	UInt16 lightArray[8];
};
/************* ��ͨ��������������**********************/
typedef struct Channel_Lock_Period
{
	unsigned char	 ucChannelStatus[32];	   //32��ͨ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������ ,7:����Ч�����Ը�ͨ�����ƣ�						   
	unsigned char	 ucWorkingTimeFlag; 	   //ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
	unsigned char	 ucBeginTimeHour;		   //������Чʱ�䣺Сʱ
	unsigned char	 ucBeginTimeMin;		   //������Чʱ�䣺����
	unsigned char	 ucBeginTimeSec;		   //������Чʱ�䣺��
	unsigned char	 ucEndTimeHour; 		   //���ƽ���ʱ�䣺Сʱ
	unsigned char	 ucEndTimeMin;		   //���ƽ���ʱ�䣺����
	unsigned char	 ucEndTimeSec;		   //���ƽ���ʱ�䣺��
	unsigned char	 ucReserved;		   //Ԥ��

}CHANNEL_LOCK_PERIOD_PARAMS;

//��ͨ���������
typedef struct STRU_Extra_Param_Channel_Lock_V2
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xD3��ʾ����ͨ������
     CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriod[16];  //ʱ�������
     unsigned char    ucReserved[4];			//Ԥ��
}CHANNEL_LOCK_V2_PARAMS; 

//��ͨ������������
typedef struct STRU_Extra_Param_Channel_Lock_Feedback_V2
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xD3��ʾ����ͨ������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}CHANNEL_LOCK_FEEDBACK_V2_PARAMS;    


//��ͨ���������(��ԭͨ������������ͬһ����Ϣ)
typedef struct STRU_Extra_Param_Channel_Unlock_V2
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xad��ʾ�ر�ͨ������
     unsigned int     unResult;				//1��ʾ��ͨ������������0��ʾԭͨ����������
}CHANNEL_UNLOCK_V2_PARAMS;    

//��ͨ������������(��ԭͨ������������ͬһ����Ϣ)
typedef struct STRU_Extra_Param_Channel_Unlock_Feedback_V2
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xad��ʾ�ر�ͨ������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}CHANNEL_UNLOCK_FEEDBACK_V2_PARAMS;   

//��ͨ������״̬��ѯ���
typedef struct STRU_Extra_Param_Channel_Lock_Status_V2
{
     unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0xD4��ʾ�ر�ͨ������
     unsigned int     unResult;				//����ֵ��1��ʾ�ɹ���0��ʾʧ��
}CHANNEL_LOCK_STATUS_V2_PARAMS;    

//��ͨ������״̬��ѯ������
typedef struct STRU_Extra_Param_Channel_Lock_Status_Feedback_V2
{
    unsigned int     unExtraParamHead;        		//��־ͷ,0x6e6e
    unsigned int     unExtraParamID;                	//����,0xD4��ʾ�ر�ͨ������
    CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriod[16];  //ʱ������÷���
	unsigned char	  unLockStatus; 					//����״̬��1��ʾͨ��������0��ʾͨ��δ������2��ʾ������
	unsigned char	  unReserved[3]; //Ԥ��
}CHANNEL_LOCK_STATUS_FEEDBACK_V2_PARAMS;

typedef struct STRU_Channel_Lock_V2_INFO
{
	UINT8	uChannelLockFlag;							//��ͨ��������־��1��ʾͨ��������0��ʾͨ��δ����
	UINT8	uChannelLockStatus;							//����״̬��1��ʾͨ��������0��ʾͨ��δ������2��ʾ������
	CHANNEL_LOCK_PERIOD_PARAMS stChannelLockPeriods[16];  //ʱ�������	
}STRU_Channel_Lock_V2_INFO;

/********************************************************/

typedef enum
{
	COUNTDOWN_CONFIG_NO_NEW = 0,	//û���µ�������Ϣ
	COUNTDOWN_CONFIG_HAVE_NEW,		//���µ�������Ϣ
	COUNTDOWN_CONFIG_UPDATE,		//������Ϣ����
} CountdownConfigFlag;

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
     unsigned char    stVehPhaseCountingDown[16][2];    //16����������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     							//��1�б�ʾ��ǰ��������λ������λ����ʱʱ��
     unsigned char    stPedPhaseCountingDown[16][2];    //16��������λ�ĵ�ǰ��ɫ������ʱ
     							//��0�б�ʾ��ǰ��������λ��״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
								//��1�б�ʾ��ǰ������λ������λ����ʱʱ��
     unsigned char    ucPlanNo;				//��ǰ���з�����
     unsigned char    ucCurCycleTime;			//��ǰ�������ڳ�
     unsigned char    ucCurRunningTime;		//��ǰ����ʱ��
     unsigned char    ucChannelLockStatus;		//ͨ���Ƿ�����״̬��1��ʾͨ��������0��ʾͨ��δ����
     unsigned char    ucCurPlanDsc[16];			//��ǰ���з�������
     unsigned char    ucOverlap[16][2];                    //������λ״̬��0:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������
     unsigned char    stPhaseRunningInfo[16][2];	//����λ����ʱ������ű�
							//��0�б�ʾ����λ���űȣ���1�б�ʾ����λ����ʱ�䣬�̵�������1�в�����ֵ������Ϊ0
	unsigned char    ucChannelStatus[32]; //32��ͨ��״̬��7:��ƣ� 1:�̵ƣ�2:��ƣ�3:�Ƶƣ�4��������5������ ,0:����Ч�����Ը�ͨ�����ƣ� 
     unsigned char    ucWorkingTimeFlag; //ʱ��ο�����Ч��־��1��ʾ����ʱ��ο�����Ч��0��ʾȫ��ʱ������Ч������ʱ�������Ч
     unsigned char    ucBeginTimeHour; //������Чʱ�䣺Сʱ
     unsigned char    ucBeginTimeMin; //������Чʱ�䣺����
     unsigned char    ucBeginTimeSec; //������Чʱ�䣺��
     unsigned char    ucEndTimeHour; //���ƽ���ʱ�䣺Сʱ
     unsigned char    ucEndTimeMin; //���ƽ���ʱ�䣺����
     unsigned char    ucEndTimeSec; //���ƽ���ʱ�䣺��
     unsigned char    ucReserved[9]; //Ԥ��
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



//������Ϊ���ƽ̨�����ù��߳�ͻ������Ƶ�Э��,added by xiaowh


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
	POWER_ON = 0x0A,								//�ϵ�
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
	POW_ON_REBOOT = 0x19,							//�ϵ�����
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

//����ռ���ʶ���
typedef struct STRU_N_VolumeOccupancy
{
    /*�ṩ��λС���㾫��֧�֣�ʵ��ֵ����100����д洢�����û���ʾʱ����Ҫ����100������ʵ��ֵ*/
	BYTE    byDetectorVolume;			/*2.3.5.4.1һ���������һ�������вɼ���������,��λ�� ��/����*/
	WORD    byDetectorOccupancy;		/*2.3.5.4.2һ���������һ�������е�ռ���� ���������ṩ����ʱ��ռ���ʣ�����100���õ��ٷֱ�ֵ*/
	WORD	byVehicleSpeed;		        /*����,����*100�洢�ģ�����ʵ��ֵӦ�ó���100�󣬵�λ����km/h*/
	WORD	wQueueLengh;		        /*�Ŷӳ���,����*100�洢������100�󣬲��ܵõ�ʵ�ʵĳ���������λ����*/
	WORD    wGreenLost;                 /*���� ��ʧ�̵�ʱ��/�̵���ʱ�� ����ֵ����100����*/
	WORD    wVehicleDensity;            /*�����ܶȣ�����*100�洢������100��λ�� ��/km*/
	WORD    wVehicleHeadDistance;       /*��ͷ��࣬ͬһ������ͬһ����������ʻǰ������������֮����ĳһ˲ʱ�ĳ�ͷ��ࡣ���ţ�hd ��λ��m/����*/
    WORD    wVehicleHeadTimeDistance;   /*��ͷʱ�ࣺͬһ������ͬһ����������ʻǰ�����ڵ��������ĳ�ͷͨ��ĳһ���ʱ���������ţ�ht ��λ��s/�� */
}VolumeOccupancy;	

//��ʷ������ʱ���Ӧ�ṹ
typedef struct STRU_TimeAndHistoryVolume
{
    DWORD dwTime;                       //��ʷʱ���
    VolumeOccupancy  struVolume[48];    //��Ӧ��ʷ����
}TimeAndHistoryVolume;

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
	UINT8 nOffset;							//��ǰ������λ��
	PhaseTurnItem sPhaseTurn[4];			//��ǰ���е��ĸ���������,�±�0����1����������
	UINT32 nCoordinatePhase;				//��ǰ����ʹ�õ�Э����λ,bit0������λ1����ֵΪ0��ʾ����Э����λ��Ϊ1��ʾ����λ��ΪЭ����λ��
	PhaseTime phaseTime[16];
	FollowPhaseItem stFollowPhase[16];      //������λ										
	
	UINT8 nReserved[256];					//Ԥ���ֽ�
}MsgRealtimePattern;//��������

typedef struct
{
	UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd2
	UINT32 unExtraDataLen;					//ʣ������ݳ���
	UINT8 nPhaseArray[16];					//�źŻ����õ�������λ����λ��
	UINT8 nPatternArray[108];				//�źŻ����õ����з����ţ�����ת��	
}MsgPhaseSchemeId;//��ƽ̨1049Э��ʹ��

//������Ϣ,���а����źŻ���¼�İ����źŵƹ��ϡ��������������ڵ����й�����Ϣ���ýṹ������¿⡣
typedef struct STRU_Extra_Param_Response
{
	UInt32	unExtraParamHead;				//��Ϣͷ��־��Ĭ��Ϊ0x6e6e
	UInt32	unExtraParamID;					//��Ϣ����ID����ֵΪ0xc0
	UInt32 	unExtraParamValue;				//���������ͣ���ֵΪ0x15b
	UInt32 	unExtraParamFirst;				//��ʼ�У���ʼ���Ϻţ���1��ʼ
	UInt32 	unExtraParamTotal;				//���������ܹ���ȡ�Ĺ�����
	UInt32  unExtraDataLen;					//�����ܳ��ȣ��������ݶ��ܳ���
	STRU_FAILURE_INFO data[0];				//ʵ�ʵĹ��Ͼ�������
}MsgFaultInfo;

//ʵʱ������Ϣͨ������Ľṹ���ȡ����ʷ������Ϣͨ��sftp��ȡָ�����ļ�����,���ļ��Ķ����Ƹ�ʽ��TimeAndHistoryVolume��
typedef struct
{
	UINT32 unExtraParamHead;				//��Ϣͷ��Ĭ��Ϊ0x6e6e
	UINT32 unExtraParamID;					//��Ϣ���ͣ�����Ϣ�������ݵĵ�ֵΪ0xd1
	
	TimeAndHistoryVolume volumeOccupancy;		//����ռ���ʵȽṹ��	
}MsgRealtimeVolume;


#pragma pack(pop)


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
	RED_YELLOW = 9
} PhaseChannelStatus;

typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

/*****************************************************************************
 �� �� ��  : put_lamp_value
 ��������  : ��Ҫ��������һ�����ĳ���Ƶ�״ֵ̬
 �������  : volatile unsigned short *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
             unsigned short value             Ҫ���õĵƵ�״ֵ̬
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}
/*****************************************************************************
 �� �� ��  : get_lamp_value
 ��������  : ��Ҫ������ȡһ����о���ĳ���Ƶ�״ֵ̬
 �������  : UInt16 *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
 �� �� ֵ  : ����ĳ���Ƶ�״ֵ̬
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline UInt16 get_lamp_value(UInt16 *lights, int n)
{
	lamp_t *p = (lamp_t *)(lights);
	UInt16 value = 0;
	switch (n) 
	{
		case 0:	value = p->L0; break;
		case 1:	value = p->L1; break;
		case 2:	value = p->L2; break;
		case 3:	value = p->L3; break;
		default: break;
	}
	
	return value;
}

#endif
