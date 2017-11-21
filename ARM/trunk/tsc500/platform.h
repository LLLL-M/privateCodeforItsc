#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "hik.h"
	   
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

struct STRU_Extra_Param_Block
{
	unsigned int	unExtraParamHead;		//��Ϣͷ��־
	unsigned int	unExtraParamID;			//��Ϣ����ID
	unsigned int 	unExtraParamValue;		//����������
	unsigned int 	unExtraParamFirst;		//��ʼ��
	unsigned int 	unExtraParamTotal;		//������
};

#pragma pack(pop)

typedef struct
{
	int sockfd;
	struct sockaddr addr;
} DestAddressInfo;

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
