#pragma pack(push, 1)

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long   UINT32;
typedef char 	INT8;
typedef short INT16;
typedef long INT32;

//信号机软件程序版本硬件版本,长度控制在32个字符以内，否则sadp获取会异常
#define SOFTWARE_VERSION_INFO "HIKTSC-V1.0.0.1-2015.04.09"
#define HARDWARE_VERSION_INFO "DS-TSC500"

typedef struct SPECIAL_PARAMS
{
	int iErrorDetectSwitch;                         //故障检测开关
	int iCurrentAlarmSwitch;						//电流检测开关，报警不处理
	int iVoltageAlarmSwitch;						//电压检测开关，报警不处理
	int iCurrentAlarmAndProcessSwitch;				//电流报警并处理开关
	int iVoltageAlarmAndProcessSwitch;				//电压报警并处理开关
	int iWatchdogSwitch;							//软件看门狗开关
	int iGpsSwitch;									//GPS开关
}SPECIAL_PARAMS;

typedef struct STRU_N_IP_ADDRESS
{
	char address[16]; 					//地址
	char subnetMask[16]; 				//子网掩码
	char gateway[16];					//网关
}STRU_N_IP_ADDRESS;	

typedef struct UDP_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	int iValue[16*64];						//
	//unsigned int iReserved;                  //预留
}UDP_INFO;

typedef struct UDP_CUR_VALUE_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	unsigned char redCurrentValue[32];		//32个通道电流值
	//unsigned int iReserved;                  //预留
}UDP_CUR_VALUE_INFO;


typedef struct FAILURE_INFO
{
	int nNumber;    		//序列号
	int nID;                //消息类型ID
	long nTime;        		//发生时间
	int nValue;         	//时间值(通道号)
}FAILURE_INFO;

typedef struct CURRENT_PARAMS
{
	int RedCurrentBase;         //基准值
	int RedCurrentDiff;         //差值
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
	int  iphase;//通道对应相位
	int  iType;//通道对应类型，0:机动车,1:行人(弃用)
				//通道对应类型，无(0),其他(1),机动车(2),行人 (3),跟随 (4)
}PhaseType;

typedef struct Count_Down_Params
{
	int iCountDownMode;				  // 倒计时模式
									  //		0:自学习(缺省)
									  //		1:脉冲全程倒计时模式
									  //		2:脉冲半程倒计时模式
									  //		485协议
									  //		3: 国家标准
									  //        4:莱斯标准
									  //        5:海信标准
	int iFreeGreenTime;                         //感应检测时间，缺省值为3
	int iPulseGreenTime;				  //脉冲绿灯倒计时时间
	int iPulseRedTime;				 //脉冲红灯倒计时时间
	PhaseType iPhaseOfChannel[32];                //通道对应的相位(目前最大支持32个通道)
}Count_Down_Params;

typedef struct STRU_Extra_Param_Phase
{
     unsigned int     unExtraParamHead;           	//标志头,0x6e6e
     unsigned int     unExtraParamID;                //类型, 0x9A表示下载，0x9B表示上载
     unsigned char    stPhaseDesc[16][64];     			//16个相位的相位方向描述，每个相位最多支持32个中文    
}PHASE_DESC_PARAMS;    

typedef struct STRU_Extra_Param_Channel
{
     unsigned int     unExtraParamHead;            		//标志头,0x6e6e
     unsigned int     unExtraParamID;              		//类型, 0x9C表示下载，0x9D表示上载  
     unsigned char    stChannelDesc[32][64];     		//32个通道的描述，每个通道最多支持32个中文     
}CHANNEL_DESC_PARAMS;

typedef struct STRU_Extra_Param_Phase_Counting_Down
{
     unsigned int     unExtraParamHead;        			//标志头,0x6e6e
     unsigned int     unExtraParamID;                	//类型,0x9E表示上载
     unsigned int     stVehPhaseCountingDown[16][2];    //16个机动车相位的当前灯色及倒计时
     													//低位表示当前机动车相位的颜色，1:绿灯，2:红灯，3:黄灯
     													//高位表示当前机动车相位所属相位号
     unsigned int     stPedPhaseCountingDown[16][2];    //16个行人相位的当前灯色及倒计时
     													//低位表示当前机动车相位的颜色，1:绿灯，2:红灯，3:黄灯
														//高位表示当前行人相位所属相位号
}PHASE_COUNTING_DOWN_PARAMS;    

typedef struct STRU_Extra_Param_Version
{
     unsigned int     unExtraParamHead;                 //标志头,0x6e6e
     unsigned int     unExtraParamID;                   //类型,0xA1表示上载
     unsigned char    softVersionInfo[32];                //软件版本信息
     unsigned char    hardVersionInfo[32];                //硬件版本信息
}DEVICE_VERSION_PARAMS;

typedef struct STRU_Extra_Param_Pattern_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;                //下载0xA4,上载0xA5
     unsigned char     stPatternNameDesc[16][64]; 
}PATTERN_NAME_PARAMS;    

typedef struct STRU_Extra_Param_Plan_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;               //下载0xA6,上载0xA7
     unsigned char     stPlanNameDesc[16][64]; 
}PLAN_NAME_PARAMS; 

typedef struct _Date_Desc_
{
	int  dateType;                   //日期类型：0：普通日期，1：特殊日期
	unsigned char dateName[64];      //日期对应名称
}DATE_DESC;

typedef struct STRU_Extra_Param_Date_Name
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;    			//下载0xA8,上载0xA9
     DATE_DESC        stNameDesc[40];               //
}DATE_NAME_PARAMS;    


typedef struct STRU_Extra_Param_Com
{
     unsigned int     unExtraParamHead;        
     unsigned int     unExtraParamID;      //下载0xA3,上载0xA2     
     unsigned int     unExtraParamValue; //1：232串口   2：485串口1   3:485串口2    4:422串口
     unsigned int     unBaudRate;    //波特率：2400,4800,9600,115200,460800
     unsigned int     unDataBits; 	//数据位7,8
	 unsigned int     unStopBits;   //停止位：1,2
     unsigned int     unParity;     //奇偶校验位：0：N无校验   1：O奇数校验  2：E偶数校验
}COM_PARAMS;

#pragma pack(pop)