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



/*��λ����*/
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

/*ͨ����*/        //add by lxp
typedef struct
{	int iControlSource;                                      //����Դ	
	int iControlType;                                        //��������	
	int iFlashMode;                                          //����ģʽ	
	int iBrightMode;                                         //�Ҷ�ģʽ
} stChannelTable;
/*���ű�*/        //add by lxp
typedef struct 
{	
	int iSplitNo;									//���űȱ��
	int iSplitForPhase[36];                                        //���ű�	
	int iModeForPhase[36];                                       //ģʽ	
	int iCoordinatePhase[36];                                 //��ΪЭ����λ	
	int nPhaseId[36];                                              //��λ�� 
} stGreenRatio;
/*���ϼ������*/        //add by lxp
typedef struct 
{	int VoltageDetectionTimes;								            //��Ƿѹ������
	int RedLightDetectionTimes;                                         //���Ϩ�������	
	int ConflictDetectionAttempts;                                      //��ͻ������	
	int ManualPanelKeyNumber;								            //�ֶ���尴������
	int RemoteControlKeyNumber;                                         //ң������������	
	int SenseSwitch;                                                    //��⿪��	
	int DynamicStep;								                    //�������ϼ��
	int CurrentFaultDetection;                                          //��ѹ���ϼ��	
	int AlarmAndFaultCurrent;                                           //�������ϱ���������	
	int AlarmAndFaultVoltage;								            //��ѹ���ϱ���������
	int EnableWatchdog;                                                 //���ÿ��Ź�	
	int EnableGPS;
	int CNum[32][3];                                                    //ͨ���Ų���
} stFaultDetectionSet;
stFaultDetectionSet gFaultDetectionSet;
/*�����*/        //add by lxp
typedef struct 
{	int SequenceTableNo;								                //�������
	int SNum[4][16];                                                    //�����
} stSequenceTable;
/*������*/        //add by lxp
typedef struct 
{
    unsigned short nSchemeID;//������
    unsigned short nCycleTime;//���ڳ�
    unsigned short nOffset;//��λ��
    unsigned short nGreenSignalRatioID;//���űȺ�
    unsigned short nPhaseTurnID;//������
} stProgramTable;
/*ʱ��������*/        //add by lxp
typedef struct 
{	int ActionTable;													//������
	int ProgramNo;														//������	
	unsigned char AssistFunction[3];                                              //��������	
	unsigned char SpecialFunction[8];                                             //���⹦��
} stTimeBasedActionTable;
/*ʱ�α�*/        //add by lxp
typedef struct 
{	int TimeIntervalNo;													//ʱ�α��
	int Time[4][3];                                                     //ʱ����Ϣ
} stTimeInterval;
stTimeInterval gTimeInterval;
/*���ȼƻ�*/        //add by lxp
typedef struct 
{	int SchedulingNo;													//���ȼƻ���
	int TimeIntervalNum;                                                //ʱ�α�	
	int Month[12];                                                      //�·�	
	int Day[31];                                                        //����	
	int WeekDay[7];                                                     //����
} stScheduling;
/*�ص���*/        //add by lxp
typedef struct 
{	int FollowPhase;													//������λ
	int GreenLight;														//�̵�	
	int RedLight;                                                       //���	
	int YellowLight;                                                    //�Ƶ�	
	int GreenFlash;                                                     //����	
	int ModifiedPhase;                                                  //������λ	
	int ParentPhase[NUM_PHASE];                                                //ĸ��λ
} stOverlapping;
/*Э��*/        //add by lxp
typedef struct 
{	int ControlModel;													//����ģʽ
	int ManualMethod;													//�ֶ�����	
	int CoordinationMode;                                               //Э����ʽ
	int CoordinateMaxMode;                                              //Э�����ʽ
	int CoordinateForceMode;                                            //Э��ǿ�Ʒ�ʽ
} stCoordinate;
stCoordinate gCoordinate;
/*���������*/        //add by lxp
typedef struct 
{	int DetectorNo;													    //�������
	int RequestPhase;													//������λ	
	int SwitchPhase;                                                    //������λ	
	int Delay;                                                          //�ӳ�	
	int FailureTime;                                                    //ʧ��ʱ��	
	int QueueLimit;													    //��������
	int NoResponseTime;													//����Ӧʱ��	
	int MaxDuration;                                                    //������ʱ��	
	int Extend;                                                         //�ӳ�	
	int MaxVehicle;                                                     //�������	
	int Flow;													        //����
	int Occupancy;													    //ռ����	
	int ProlongGreen;                                                   //�ӳ���	
	int AccumulateInitial;                                              //���۳�ʼ	
	int Queue;                                                          //�Ŷ�	
	int Request;													    //����
	int RedInterval;													//�������	
	int YellowInterval;                                                 //�Ƶ�ʱ��
}stVehicleDetector;
stVehicleDetector gVehicleDetector;
/*���˼����*/        //add by lxp
typedef struct
{	int DetectorNo;													    //��������
	int RequestPhase;													//������λ
	int NoResponseTime;                                                 //����Ӧʱ��
	int MaxDuration;                                                    //������ʱ��	
	int InductionNumber;                                                //��Ӧ��
} stPedestrian;
stPedestrian gPedestrian;
/*��������*/        //add by lxp
typedef struct 
{	int ControlRecord;													//����������
	int LogRecord;
	int CommunicatRecord;													//ͨ�Ź���	
	int DetectorRecord;                                               //���������
} stFaultConfig;
stFaultConfig gFaultConfig;
/*����̬����*/        //add by lxp
typedef struct 
{	
    int addCount;													//��λ������
	int addChannel;                                                 //ͨ��������
	int addProgram;                                                 //����������
	int addSplit;                                                   //���űȱ�
	int nPhaseTurnCount;                                            //���������
	int nTimeInterval;                                              //ʱ�α�����
	int nActionCount;                                               //���������� 
	int nScheduleCount;                                             //���ȼƻ�����
	int nFollowPhaseCount;                                          //������λ����
    int nVehicleDetectorCount;                                      //�������������
    int nPedestrianDetectorCount;                                   //���˼��������
} stTreeDynamicPara;
stTreeDynamicPara gTreeDynamicPara;


char * GetEth0Ip(char *dn_or_ip);
void initReqmethod(char *tmpStr,int reqmethod,int flags,int result);


#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif


#endif

