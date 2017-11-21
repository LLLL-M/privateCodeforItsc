#ifndef __COMMON_H__
#define __COMMON_H__

#pragma pack(push, 1)

#define FALSE 0
#define TRUE 1

//SDK��Ϣ����
#define COM_MSG_HEAD			0x6e6e
#define MSG_CHAN_LOCK			0xb9	//ͨ��������Ϣ
#define MSG_CHAN_UNLOCK			0xba	//ͨ������������Ϣ(�Ͷ�ʱ��ͨ����������ͬһ����Ϣ����)
#define MSG_MP_CHAN_LOCK_SET	0xd3	//��ʱ��ͨ��������Ϣ
#define MSG_MP_CHAN_LOCK_GET	0xd4	//��ʱ��ͨ��������ѯ
#define MSG_CHAN_LOCK_STATUS_GET	0xe0	//��ȡͨ������״̬

#define EXTEND_MSG_HEAD			0x6e6f	//extend protocol msg head

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
	char isCameraKakou;								//�Ƿ�Ϊ�������
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
									  //        6:����2004
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
#define MAX_CHANNEL_NUM                   32
//ʹ��ͨѶЭ��Ļ������֧��32������ʱID,��Щ��������Ҫ�������ļ��ж�ȡ�����ġ�
typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //����ʱID��ͨ����������ʱ����ϵ��������������õ���ʱ��ID���ֱ���0 1 2 3 ...,�����±���ID��
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_CHANNEL_NUM];      //�õ���ʱ��Ӧ��ʾ�Ŀ���Դ��һ������ʱ������ʾ��ֹһ������Դ(��ǰ����λ��������ͨ��)��
                                                                        //����˵����ͬʱ��ʾһ�������ֱ�к���ת����ʱ��Ϣ,�������Դ�ж�����������ļ��� �Զ��Ÿ�����
                                                                        //������ţ��м�����ʾ����,ͨ���Ŵ�1��ʼ��
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //����Դ������, ����ȡControllerType�����ֵ��ע����ʱ����ֻ���ǻ����������ˡ��������ֿ�������
    unsigned int  nChannelFlag;                                         //���嵹��ʱͨ���رձ�־����0Ϊ���壬��1Ϊ��ѧϰ
    unsigned char cReservedValue1[508];                                 //ֵ�ֱ�������1��������2������3������4��
    
}CountDownCfg; 

#define WIRELESS_CTRL_ON	1
#define WIRELESS_CTRL_OFF	0
#define MAX_WIRELESS_KEY	5

typedef enum
{
	WIRELESS_CTRL_DEFAULT=0,
	WIRELESS_CTRL_SELFDEF=1
}eWirelessCtrlMode;
typedef enum
{
	E_KEY_NO_CHECK = 0,
	E_KEY_CHECK = 1
}eWirelessCtrlKeyChkFlag;
typedef struct WIRELESS_CONTROLLER_KEY_INFO
{
	char description[64];
	unsigned char ucChan[32];
}STRU_WIRELESS_CONTROLLER_KEY_INFO;
typedef struct STRU_WIRELESS_CONTROLLER
{
	char iSwitch; 					// ����ģ���Ƿ�����־, 0 -- �رգ� 1--����
	unsigned int iOvertime;				// ����ģ���ް�����Ӧ��ʱʱ��, Ĭ��ֵ300�� ��λ��
	int iCtrlMode;					//����ģʽ��0 --Ĭ�Ͽ��Ʒ�ʽ��1--�Զ������ģʽ
	struct WIRELESS_CONTROLLER_KEY_INFO key[MAX_WIRELESS_KEY-1];	//�Զ������ʱ��ͨ�����ֵ	
}STRU_WIRELESS_CONTROLLER_INFO;

#define MAX_FRONT_BOARD_KEY	13
typedef enum{
	E_KEY_CTRL_DEFAULT = 0,
	E_KEY_CTRL_SELFDEF = 1
}eKeyCtrlMode;
typedef struct KEY_CHAN_INFO
{
	char description[64];
	unsigned char ucChan[32];
}STRU_KEY_CHAN_INFO;
typedef struct STRU_FRONTBOARD_KEY_INFO
{
	char iSwitch;	//�Ƿ������Զ���
	STRU_KEY_CHAN_INFO key[MAX_FRONT_BOARD_KEY-5];//�Զ������ģʽ��ÿ��������Ӧ��ͨ������
}STRU_FRONTBOARD_KEY_INFO;


typedef struct EXTEND_UDP_MSG
{
	int head;
	short len;
	char checksum;
	char reserved;
	char xml[1024*15];//15k
}STRU_EXTEND_UDP_MSG;

typedef struct STRU_SYS_USER_INFO
{
	char cName[32];						// �û���
	char cPasswd[32];					// ����
}STRU_SYS_USER_INFO;

//wifi��������:
typedef struct STRU_WIFI_INFO
{
	char cSSID[32];						// wifi��
	char cPSK[32];					// wifi��������
}STRU_WIFI_INFO;

typedef struct STRU_CAR_DETECT
{
	unsigned char cCarDetectorType;								// ��������ͣ�1--�޳���壬 2--��Ȧ����壬 3--��Ƶ�����
	STRU_STRU_N_IP_ADDRESS stVedioDetIP;				// ��Ƶ������ip, mask, gateway, ����1��2���ͣ��������ô�ֵ
}STRU_CAR_DETECT_INFO;

//�豸��Ϣ:
typedef struct STRU_DEVICE_INFO
{
	unsigned int uDevID;								// �źŻ�id, 1-255
	char cDevDesc[64]; 							// �豸����
}STRU_DEVICE_INFO;

//�����źż��:
typedef struct STRU_RGSIGNAL_CHECK_INFO
{
	int iRGSignalSwitch;					// �Ƿ��ͺ����źſ���, 1--���ͣ� 0--������
	char cMcastAddr[16]; 					// �鲥��ַ
	unsigned int uMcastPort;						// �鲥��ַ�˿�
}STRU_RGSIGNAL_CHECK_INFO;


int open_port(int comport);
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop) ;

#pragma pack(pop)

#endif

