#ifndef __COMMON_H__
#define __COMMON_H__

#pragma pack(push, 1)

#ifndef _TYPEDEF_
#define _TYPEDEF_
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long   UINT32;
typedef char 	INT8;
typedef short INT16;
typedef long INT32;
typedef unsigned int   DWORD; // 4, same as long
typedef unsigned short WORD;  // 2
typedef unsigned char  BYTE;  // 1
#endif

//�źŻ��������汾Ӳ���汾
#define SOFTWARE_VERSION_INFO "HIKETSC-V1.0.0.1-2015.03.09"
#define HARDWARE_VERSION_INFO "DS-TSC300"

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
}SPECIAL_PARAMS;

typedef struct STRU_N_IP_ADDRESS
{
	char address[16]; 					//��ַ
	char subnetMask[16]; 				//��������
	char gateway[16];					//����
}STRU_N_IP_ADDRESS;	

typedef struct UDP_INFO
{
	int iHead;                      //��Ϣͷ��Ĭ��Ϊ0x6e6e
	int iType;						//��Ϣ���ͣ�0x94 - ���ؼ����� | 0x88 - ������� | 0x15b - ���ع�����Ϣ | 0x97 - ���غ�Ƶ��� | 0x93 - ���ؼ����� 
	int iValue[16*64];						//
	//unsigned int iReserved;                  //Ԥ��
}UDP_INFO;

typedef struct UDP_CUR_VALUE_INFO
{
	int iHead;                      //��Ϣͷ��Ĭ��Ϊ0x6e6e
	int iType;						//��Ϣ���ͣ�0x94 - ���ؼ����� | 0x88 - ������� | 0x15b - ���ع�����Ϣ | 0x97 - ���غ�Ƶ��� | 0x93 - ���ؼ����� 
	unsigned char redCurrentValue[32];		//32��ͨ������ֵ
	//unsigned int iReserved;                  //Ԥ��
}UDP_CUR_VALUE_INFO;


typedef struct FAILURE_INFO
{
	int nNumber;    		//���к�
	int nID;                //��Ϣ����ID
	long nTime;        		//����ʱ��
	int nValue;         	//ʱ��ֵ(ͨ����)
}FAILURE_INFO;

typedef struct CURRENT_PARAMS
{
	int RedCurrentBase;         //��׼ֵ
	int RedCurrentDiff;         //��ֵ
}CURRENT_PARAMS;

typedef struct CURRENT_PARAMS_UDP
{
	int iHead;                      
	int iType;	
	struct CURRENT_PARAMS struRecCurrent[32];
}CURRENT_PARAMS_UDP;

typedef struct _Count_Down_Veh_
{
	unsigned char veh_phase;                      
	unsigned char veh_color;	
	unsigned char veh_phaseTime;
}CountDownVeh;

typedef struct _Count_Down_Ped_
{
	unsigned char ped_phase;                      
	unsigned char ped_color;	
	unsigned char ped_phaseTime;
}CountDownPed;

typedef struct _Count_Down_Head_
{
	UINT8 head1;
	UINT8 head2;
}CountDownHead;

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
}Count_Down_Params;

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

typedef struct STRU_Extra_Param_Phase_Counting_Down
{
     unsigned int     unExtraParamHead;        			//��־ͷ,0x6e6e
     unsigned int     unExtraParamID;                	//����,0x9E��ʾ����
     unsigned int     stVehPhaseCountingDown[16][2];    //16����������λ�ĵ�ǰ��ɫ������ʱ
     													//��λ��ʾ��ǰ��������λ����ɫ��1:�̵ƣ�2:��ƣ�3:�Ƶ�
     													//��λ��ʾ��ǰ��������λ������λ��
     unsigned int     stPedPhaseCountingDown[16][2];    //16��������λ�ĵ�ǰ��ɫ������ʱ
     													//��λ��ʾ��ǰ��������λ����ɫ��1:�̵ƣ�2:��ƣ�3:�Ƶ�
														//��λ��ʾ��ǰ������λ������λ��
}PHASE_COUNTING_DOWN_PARAMS;    

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
     unsigned char     stPatternNameDesc[16][64]; 
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
#pragma pack(pop)

int read_profile_string( const char *section, const char *key,char *value, 
		 int size, const char *default_value, const char *file);
		 
int write_profile_string(const char *section, const char *key,const char *value, const char *file);

void get_special_params();

void get_failure_number();

void set_failure_number(unsigned int failurenumber);

void set_special_params(struct SPECIAL_PARAMS special_params);

void get_current_params();

void set_current_params();

void get_count_down_params();

void set_count_down_params();

void set_phase_desc_params();

void get_phase_desc_params();

void set_channel_desc_params();

void get_channel_desc_params();

void set_pattern_name_params();

void get_pattern_name_params();

void set_plan_name_params();

void get_plan_name_params();

void set_date_name_params();

void get_date_name_params();

void set_com_params(int comNo);

void get_com_params(int comNo);

void get_all_com_params();
void get_all_params_form_config_file();

//udp sever����غ�������
void udp_send_yellowflash();
int udp_server_init();

//count down����غ�������
int count_down_thread_create();

#endif

