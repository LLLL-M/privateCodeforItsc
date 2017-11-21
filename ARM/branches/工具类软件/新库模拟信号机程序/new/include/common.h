#ifndef __COMMON_H__
#define __COMMON_H__

#pragma pack(push, 1)


//SDK��Ϣ����
#define COM_MSG_HEAD			0x6e6e
#define MSG_CHAN_LOCK			0xb9	//ͨ��������Ϣ
#define MSG_CHAN_UNLOCK			0xba	//ͨ������������Ϣ(�Ͷ�ʱ��ͨ����������ͬһ����Ϣ����)
#define MSG_MP_CHAN_LOCK_SET	0xd3	//��ʱ��ͨ��������Ϣ
#define MSG_MP_CHAN_LOCK_GET	0xd4	//��ʱ��ͨ��������ѯ

typedef struct SPECIAL_PARAMS
{
	int iErrorDetectSwitch;                         //���ϼ�⿪��
	int iCurrentAlarmSwitch;						//������⿪�أ�����������
	int iVoltageAlarmSwitch;						//��ѹ��⿪�أ�����������
	int iCurrentAlarmAndProcessSwitch;				//����������������
	int iVoltageAlarmAndProcessSwitch;				//��ѹ������������
	int iWatchdogSwitch;							//������Ź�����
	int iGpsSwitch;									//GPS����
	int iSignalMachineType;
	int iRedSignalCheckSwitch;						//����źż��������
	int iPhaseTakeOverSwtich;						//��λ�ӹܿ���
}STRU_SPECIAL_PARAMS;

typedef struct STRU_N_IP_ADDRESS
{
	char address[16]; 					//��ַ
	char subnetMask[16]; 				//��������
	char gateway[16];					//����
}STRU_STRU_N_IP_ADDRESS;	

typedef struct UDP_INFO
{
	int iHead;                      //��Ϣͷ��Ĭ��Ϊ0x6e6e
	int iType;						//��Ϣ���ͣ�0x94 - ���ؼ����� | 0x88 - ������� | 0x15b - ���ع�����Ϣ | 0x97 - ���غ�Ƶ��� | 0x93 - ���ؼ����� 
	int iValue[6250];						//
	//unsigned int iReserved;                  //Ԥ��
}STRU_UDP_INFO;

typedef struct UDP_CUR_VALUE_INFO
{
	int iHead;                      //��Ϣͷ��Ĭ��Ϊ0x6e6e
	int iType;						//��Ϣ���ͣ�0x94 - ���ؼ����� | 0x88 - ������� | 0x15b - ���ع�����Ϣ | 0x97 - ���غ�Ƶ��� | 0x93 - ���ؼ����� 
	unsigned char redCurrentValue[32];		//32��ͨ������ֵ
	//unsigned int iReserved;                  //Ԥ��
}STRU_UDP_CUR_VALUE_INFO;


typedef struct FAILURE_INFO
{
	int nNumber;    		//���к�
	int nID;                //��Ϣ����ID
	long nTime;        		//����ʱ��
	int nValue;         	//ʱ��ֵ(ͨ����)
}STRU_FAILURE_INFO;

typedef struct CURRENT_PARAMS
{
	int RedCurrentBase;         //��׼ֵ
	int RedCurrentDiff;         //��ֵ
}STRU_CURRENT_PARAMS;

typedef struct CURRENT_PARAMS_UDP
{
	int iHead;                      
	int iType;	
	struct CURRENT_PARAMS struRecCurrent[32];
}STRU_CURRENT_PARAMS_UDP;

typedef struct _Phase_And_Type_Of_Channel_
{
	int  iphase;//ͨ����Ӧ��λ
	int  iType;//ͨ����Ӧ���ͣ�0:������,1:����(����)
				//ͨ����Ӧ���ͣ���(0),����(1),������(2),���� (3),���� (4)
}PhaseType;

typedef struct Count_Down_Params
{
	int iCountDownMode;				  // ����ʱģʽ
									  //		0:��ѧϰ(ȱʡ)
									  //		1:����ȫ�̵���ʱģʽ
									  //		2:�����̵���ʱģʽ
									  //		485Э��
									  //		3: ���ұ�׼
									  //        4:��˹��׼
									  //        5:���ű�׼
	int iFreeGreenTime;                         //��Ӧ���ʱ�䣬ȱʡֵΪ3
	int iPulseGreenTime;				  //�����̵Ƶ���ʱʱ��
	int iPulseRedTime;				 //�����Ƶ���ʱʱ��
	PhaseType iPhaseOfChannel[32];                //ͨ����Ӧ����λ(Ŀǰ���֧��32��ͨ��)
	int option;						//bit0:����Ƶ�ʱ�Ƿ����
	int redFlashSec;				//��Ƶ���ʱ��˸����
}STRU_Count_Down_Params;

typedef struct _Ped_Detect_Params
{
	int  pedDelayTime;   //��ʱʱ��
	int  startPedTimeout; //�Ƿ������˳�ʱ�ж�
}PedDetectParams;

typedef struct STRU_Extra_Param_Phase
{
     unsigned int     unExtraParamHead;           	//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                //����, 0x9A��ʾ���أ�0x9B��ʾ����
     unsigned char    stPhaseDesc[16][64];     			//16����λ����λ����������ÿ����λ���֧��32������    
}PHASE_DESC_PARAMS;    

typedef struct STRU_Extra_Param_Channel
{
     unsigned int     unExtraParamHead;            		//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;              		//����, 0x9C��ʾ���أ�0x9D��ʾ����  
     unsigned char    stChannelDesc[32][64];     		//32��ͨ����������ÿ��ͨ�����֧��32������     
}CHANNEL_DESC_PARAMS;  

typedef struct STRU_Extra_Param_Version
{
     unsigned int     unExtraParamHead;                 //��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                   //����,0xA1��ʾ����
     unsigned char    softVersionInfo[32];                //����汾��Ϣ
     unsigned char    hardVersionInfo[32];                //Ӳ���汾��Ϣ
}DEVICE_VERSION_PARAMS;

typedef struct STRU_Extra_Param_Pattern_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;                //����0xA4,����0xA5
     unsigned char    stPatternNameDesc[16][64]; 
}PATTERN_NAME_PARAMS;    

typedef struct STRU_Extra_Param_Plan_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;               //����0xA6,����0xA7
     unsigned char     stPlanNameDesc[16][64]; 
}PLAN_NAME_PARAMS; 

typedef struct _Date_Desc_
{
	int  dateType;                   //�������ͣ�0����ͨ���ڣ�1����������
	unsigned char dateName[64];      //���ڶ�Ӧ����
}DATE_DESC;

typedef struct STRU_Extra_Param_Date_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;    			//����0xA8,����0xA9
     DATE_DESC        stNameDesc[40];               //
}DATE_NAME_PARAMS;    


typedef struct STRU_Extra_Param_Com
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;      //����0xA3,����0xA2     
     unsigned int     unExtraParamValue; //1��232����   2��485����1   3:485����2    4:422����
     unsigned int     unBaudRate;    //�����ʣ�2400,4800,9600,115200,460800
     unsigned int     unDataBits; 	//����λ7,8
	 unsigned int     unStopBits;   //ֹͣλ��1,2
     unsigned int     unParity;     //��żУ��λ��0��N��У��   1��O����У��  2��Eż��У��
}COM_PARAMS;

#define MAX_NUM_COUNTDOWN             32                        //��֧�ֵĵ���ʱ���������Ϊ32������ͬЭ��ĵ���ʱ�������Բ�ͬ�������ܳ���32����
#define MAX_NUM_PHASE                 16                        //ÿ������ʱ�������ʾ16����λ��
//ʹ��ͨѶЭ��Ļ������֧��32������ʱID,��Щ��������Ҫ�������ļ��ж�ȡ�����ġ�
typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //����ʱID��ͨ����������ʱ����ϵ��������������õ���ʱ��ID���ֱ���0 1 2 3 ...,�����±���ID��
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_NUM_PHASE];      //�õ���ʱ��Ӧ��ʾ�Ŀ���Դ��һ������ʱ������ʾ��ֹһ������Դ(��λ)��
                                                                        //����˵����ͬʱ��ʾһ�������ֱ�к���ת����ʱ��Ϣ,�������Դ�ж�����������ļ��� �Զ��Ÿ�����
                                                                        //������ţ��м�����ʾ����,��λ��1��ʼ��
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //����Դ������, ����ȡControllerType�����ֵ��ע����ʱ����ֻ���ǻ����������ˡ��������ֿ�������
    unsigned char cReservedValue1[512];                                 //ֵ�ֱ�������1��������2������3������4��
}CountDownCfg; 

#pragma pack(pop)

#endif

