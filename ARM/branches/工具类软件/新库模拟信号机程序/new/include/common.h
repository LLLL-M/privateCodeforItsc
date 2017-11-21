#ifndef __COMMON_H__
#define __COMMON_H__

#pragma pack(push, 1)


//SDK消息定义
#define COM_MSG_HEAD			0x6e6e
#define MSG_CHAN_LOCK			0xb9	//通道锁定消息
#define MSG_CHAN_UNLOCK			0xba	//通道锁定解锁消息(和多时段通道解锁共用同一个消息类型)
#define MSG_MP_CHAN_LOCK_SET	0xd3	//多时段通道锁定消息
#define MSG_MP_CHAN_LOCK_GET	0xd4	//多时段通道锁定查询

typedef struct SPECIAL_PARAMS
{
	int iErrorDetectSwitch;                         //故障检测开关
	int iCurrentAlarmSwitch;						//电流检测开关，报警不处理
	int iVoltageAlarmSwitch;						//电压检测开关，报警不处理
	int iCurrentAlarmAndProcessSwitch;				//电流报警并处理开关
	int iVoltageAlarmAndProcessSwitch;				//电压报警并处理开关
	int iWatchdogSwitch;							//软件看门狗开关
	int iGpsSwitch;									//GPS开关
	int iSignalMachineType;
	int iRedSignalCheckSwitch;						//红灯信号检测器开关
	int iPhaseTakeOverSwtich;						//相位接管开关
}STRU_SPECIAL_PARAMS;

typedef struct STRU_N_IP_ADDRESS
{
	char address[16]; 					//地址
	char subnetMask[16]; 				//子网掩码
	char gateway[16];					//网关
}STRU_STRU_N_IP_ADDRESS;	

typedef struct UDP_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	int iValue[6250];						//
	//unsigned int iReserved;                  //预留
}STRU_UDP_INFO;

typedef struct UDP_CUR_VALUE_INFO
{
	int iHead;                      //消息头，默认为0x6e6e
	int iType;						//消息类型，0x94 - 下载检测参数 | 0x88 - 故障清除 | 0x15b - 上载故障信息 | 0x97 - 上载红灯电流 | 0x93 - 上载检测参数 
	unsigned char redCurrentValue[32];		//32个通道电流值
	//unsigned int iReserved;                  //预留
}STRU_UDP_CUR_VALUE_INFO;


typedef struct FAILURE_INFO
{
	int nNumber;    		//序列号
	int nID;                //消息类型ID
	long nTime;        		//发生时间
	int nValue;         	//时间值(通道号)
}STRU_FAILURE_INFO;

typedef struct CURRENT_PARAMS
{
	int RedCurrentBase;         //基准值
	int RedCurrentDiff;         //差值
}STRU_CURRENT_PARAMS;

typedef struct CURRENT_PARAMS_UDP
{
	int iHead;                      
	int iType;	
	struct CURRENT_PARAMS struRecCurrent[32];
}STRU_CURRENT_PARAMS_UDP;

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
	int option;						//bit0:代表黄灯时是否黄闪
	int redFlashSec;				//红灯倒计时闪烁秒数
}STRU_Count_Down_Params;

typedef struct _Ped_Detect_Params
{
	int  pedDelayTime;   //超时时间
	int  startPedTimeout; //是否开启行人超时判断
}PedDetectParams;

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
     unsigned char    stPatternNameDesc[16][64]; 
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

#define MAX_NUM_COUNTDOWN             32                        //所支持的倒计时牌数，最多为32个，不同协议的倒计时牌数可以不同，但不能超过32个。
#define MAX_NUM_PHASE                 16                        //每个倒计时牌最多显示16个相位。
//使用通讯协议的话，最多支持32个倒计时ID,这些参数是需要从配置文件中读取出来的。
typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //倒计时ID，通过操作倒计时面板上的两个按键来设置倒计时的ID，分别是0 1 2 3 ...,数组下标做ID号
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_NUM_PHASE];      //该倒计时对应显示的控制源，一个倒计时可以显示不止一个控制源(相位)，
                                                                        //比如说可以同时显示一个方向的直行和左转倒计时信息,如果控制源有多个，在配置文件中 以逗号隔开，
                                                                        //连续存放，有几个显示几个,相位从1开始。
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //控制源的类型, 可以取ControllerType里面的值。注意暂时我们只考虑机动车、行人、跟随三种控制类型
    unsigned char cReservedValue1[512];                                 //值分别是其他1、机动车2、行人3、跟随4，
}CountDownCfg; 

#pragma pack(pop)

#endif

