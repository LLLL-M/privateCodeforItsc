#ifndef _DATATYPE_H_
#define _DATATYPE_H_

#include	"../uemf.h"
#include	"../wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>

#include "../webs.h"


#include    "../inifile.h"
#include    "../util_xml.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hikConfig.h"


#define h_addr h_addr_list[0]


#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void	formDefineUserMgmt(void);
#endif

#define USE_DEMO_MODE 1



/*单位参数*/
typedef struct 
{	int iStartFlashingYellowTime;
	int iStartAllRedTime;
	int iDegradationTime;
	int iSpeedFactor;
	int iMinimumRedLightTime;
	int iCommunicationTimeout;
	int iFlashingFrequency;
	int iTwiceCrossingTimeInterval;
	int iTwiceCrossingReverseTimeInterval;
	int iSmoothTransitionPeriod;
	int iFlowCollectionPeriod;
	int iCollectUnit;
	int iAutoPedestrianEmpty;
	int iOverpressureDetection;
} stUnitParams;

typedef struct 
{	int iPhaseNo;
	int iMinimumGreen;
	int iMaximumGreenOne;
	int iMaximumGreenTwo;
	int iExtensionGreen;
	int iMaximumRestrict;
	int iDynamicStep;
	int iYellowLightTime;
	int iAllRedTime;
	int iRedLightProtect;
	//int iIncreaseInitValue;
	//int iIncreaseInitialValueCalculation;
	//int iMaximumInitialValue;
	//int iTimeBeforeDecrease;
	//int iVehicleBeforeDecrease;
	//int iDecreaseTime;
	//int iUnitDeclineTime;
	//int iMinimumInterval;
	int iPedestrianRelease;
	int iPedestrianCleaned;
	int iKeepPedestrianRelease;
	int iNoLockDetentionRequest;
	int iDoubleEntrancePhase;
	int iGuaranteeFluxDensityExtensionGreen;
	int iConditionalServiceValid;
	int iMeanwhileEmptyLoseEfficacy;
	int iInitialize;
	int iNonInduction;
	int iVehicleAutomaticRequest;
	int iPedestrianAutomaticRequest;
	int iAutomaticFlashInto;
	int iAutomaticFlashExit;
    int nCircleNo;//
    int nGreenLightTime;
    int nIsEnable;
    int nAutoPedestrianPass;
	
}	stPhaseTable;

typedef struct 
{	int iRingForPhase[36];
	int iSamePhase[36]; 
} stRingAndPhase;

/*通道号*/        //add by lxp
typedef struct
{	int iControlSource;                                      //控制源	
	int iControlType;                                        //控制类型	
	int iFlashMode;                                          //闪光模式	
	int iBrightMode;                                         //灰度模式
} stChannelTable;
/*绿信比*/        //add by lxp
typedef struct 
{	
	int iSplitNo;									//绿信比表号
	int iSplitForPhase[36];                                        //绿信比	
	int iModeForPhase[36];                                       //模式	
	int iCoordinatePhase[36];                                 //作为协调相位	
	int nPhaseId[36];                                              //相位号 
} stGreenRatio;
/*故障检测设置*/        //add by lxp
typedef struct 
{	int VoltageDetectionTimes;								            //过欠压检测次数
	int RedLightDetectionTimes;                                         //红灯熄灭检测次数	
	int ConflictDetectionAttempts;                                      //冲突检测次数	
	int ManualPanelKeyNumber;								            //手动面板按键次数
	int RemoteControlKeyNumber;                                         //遥控器按键次数	
	int SenseSwitch;                                                    //检测开关	
	int DynamicStep;								                    //电流故障检测
	int CurrentFaultDetection;                                          //电压故障检测	
	int AlarmAndFaultCurrent;                                           //电流故障报警并处理	
	int AlarmAndFaultVoltage;								            //电压故障报警并处理
	int EnableWatchdog;                                                 //启用看门狗	
	int EnableGPS;
	int CNum[32][3];                                                    //通道号参数
} stFaultDetectionSet;
stFaultDetectionSet gFaultDetectionSet;
/*相序表*/        //add by lxp
typedef struct 
{	int SequenceTableNo;								                //相序表编号
	int SNum[4][16];                                                    //相序号
} stSequenceTable;
/*方案表*/        //add by lxp
typedef struct 
{
    unsigned short nSchemeID;//方案号
    unsigned short nCycleTime;//周期长
    unsigned short nOffset;//相位差
    unsigned short nGreenSignalRatioID;//绿信比号
    unsigned short nPhaseTurnID;//相序表号
} stProgramTable;
/*时基动作表*/        //add by lxp
typedef struct 
{	int ActionTable;													//动作表
	int ProgramNo;														//方案号	
	unsigned char AssistFunction[3];                                              //辅助功能	
	unsigned char SpecialFunction[8];                                             //特殊功能
} stTimeBasedActionTable;
/*时段表*/        //add by lxp
typedef struct 
{	int TimeIntervalNo;													//时段表号
	int Time[4][3];                                                     //时段信息
} stTimeInterval;
stTimeInterval gTimeInterval;
/*调度计划*/        //add by lxp
typedef struct 
{	int SchedulingNo;													//调度计划表
	int TimeIntervalNum;                                                //时段表	
	int Month[12];                                                      //月份	
	int Day[31];                                                        //日期	
	int WeekDay[7];                                                     //星期
} stScheduling;
/*重叠表*/        //add by lxp
typedef struct 
{	int FollowPhase;													//跟随相位
	int GreenLight;														//绿灯	
	int RedLight;                                                       //红灯	
	int YellowLight;                                                    //黄灯	
	int GreenFlash;                                                     //绿闪	
	int ModifiedPhase;                                                  //修正相位	
	int ParentPhase[NUM_PHASE];                                                //母相位
} stOverlapping;
/*协调*/        //add by lxp
typedef struct 
{	int ControlModel;													//控制模式
	int ManualMethod;													//手动方案	
	int CoordinationMode;                                               //协调方式
	int CoordinateMaxMode;                                              //协调最大方式
	int CoordinateForceMode;                                            //协调强制方式
} stCoordinate;
stCoordinate gCoordinate;
/*车辆检测器*/        //add by lxp
typedef struct 
{	int DetectorNo;													    //检测器号
	int RequestPhase;													//请求相位	
	int SwitchPhase;                                                    //开关相位	
	int Delay;                                                          //延迟	
	int FailureTime;                                                    //失败时间	
	int QueueLimit;													    //队列限制
	int NoResponseTime;													//无响应时间	
	int MaxDuration;                                                    //最大持续时间	
	int Extend;                                                         //延长	
	int MaxVehicle;                                                     //最大车辆数	
	int Flow;													        //流量
	int Occupancy;													    //占有率	
	int ProlongGreen;                                                   //延长绿	
	int AccumulateInitial;                                              //积累初始	
	int Queue;                                                          //排队	
	int Request;													    //请求
	int RedInterval;													//红灯区间	
	int YellowInterval;                                                 //黄灯时间
}stVehicleDetector;
stVehicleDetector gVehicleDetector;
/*行人检测器*/        //add by lxp
typedef struct
{	int DetectorNo;													    //检测器编号
	int RequestPhase;													//请求相位
	int NoResponseTime;                                                 //无响应时间
	int MaxDuration;                                                    //最大持续时间	
	int InductionNumber;                                                //感应数
} stPedestrian;
stPedestrian gPedestrian;
/*故障配置*/        //add by lxp
typedef struct 
{	int ControlRecord;													//控制器故障
	int LogRecord;
	int CommunicatRecord;													//通信故障	
	int DetectorRecord;                                               //检测器故障
} stFaultConfig;
stFaultConfig gFaultConfig;
/*树动态参数*/        //add by lxp
typedef struct 
{	
    int addCount;													//相位表总数
	int addChannel;                                                 //通道号总数
	int addProgram;                                                 //方案表总数
	int addSplit;                                                   //绿信比表
	int nPhaseTurnCount;                                            //相序表总数
	int nTimeInterval;                                              //时段表总数
	int nActionCount;                                               //动作表总数 
	int nScheduleCount;                                             //调度计划总数
	int nFollowPhaseCount;                                          //跟随相位总数
    int nVehicleDetectorCount;                                      //车辆检测器总数
    int nPedestrianDetectorCount;                                   //行人检测器总数
} stTreeDynamicPara;
stTreeDynamicPara gTreeDynamicPara;


char * GetEth0Ip(char *dn_or_ip);
void initReqmethod(char *tmpStr,int reqmethod,int flags,int result);


#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif


#endif

