#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include "its.h"

typedef struct _PassTimeInfo	//放行时间信息
{
	UInt16 greenTime;			//绿灯时间
	UInt8 greenBlinkTime;		//绿闪时间
	UInt8 yellowTime;			//黄灯时间
	UInt8 redYellowTime;		//红黄时间
	UInt8 allRedTime;			//全红时间
} PassTimeInfo;

typedef struct _PhaseTimeInfo
{
	UInt16 splitTime;			//绿信比时间
	PassTimeInfo passTimeInfo;	//放行时间信息
    UInt8 pedestrianPassTime;	//行人放行时间
    UInt8 pedestrianClearTime;	//行人清空时间
	UInt8 pedAutoRequestFlag;	//行人自动请求标志，0:没有设置请求,1:设置了请求
	UInt64 vehicleDetectorBits;	//相位对应的车检器号
	UInt8 unitExtendGreen;		//单位延长绿
	UInt8 maxExtendGreen;		//最大可以延长的绿灯时间
	UInt8 maxExtendGreen2;      //自适应控制最大可以延长的绿灯时间
	UInt32 motorChannelBits;	//相位对应的机动车通道，bit0-bit31分别代表通道1-32
	UInt32 pedChannelBits;		//相位对应的行人通道，bit0-bit31分别代表通道1-32
} PhaseTimeInfo;

typedef struct
{
	UInt16 phaseBits;			//跟随相位跟随的母相位，bit0-bit15分别代表相位1-16
	UInt32 motorChannelBits;	//跟随相位对应的机动车通道，bit0-bit31分别代表通道1-32
	UInt32 pedChannelBits;		//跟随相位对应的行人通道，bit0-bit31分别代表通道1-32
} FollowPhaseInfo;

typedef struct _StageInfo
{
	PassTimeInfo passTimeInfo;					//放行时间信息
	UInt16 runTime;								//阶段运行时间
	Boolean isBarrierStart;						//阶段是否是屏障起始
	Boolean isBarrierEnd;						//阶段是否是屏障结束
	UInt8 includeNum;							//阶段包含相位的个数
	UInt8 includePhases[MAX_PHASE_NUM];	//阶段包含相位
} StageInfo;

typedef struct _CalInfo
{
	UInt8 timeIntervalId;					//时段表号
	UInt8 actionId;							//动作号
	UInt8 schemeId;							//方案号
	UInt8 splitId;							//绿信比号
	UInt8 phaseTurnId;						//相序号
	UInt8 phaseTableId;						//相位表号
	UInt8 channelTableId;					//通道表号
	UInt16 cycleTime;						//周期时间
	UInt16 inductiveCoordinateCycleTime;	//感应协调控制周期时间
	
	UInt8 transitionCycle;					//过渡周期
	UInt8 coordinatePhaseId;				//协调相位
	UInt16 isCoordinatePhase;				//是否是协调相位,每bit代表一个相位,0:不是,1:是
	UInt16 phaseOffset;						//相位差
	int timeGapSec;							//当前时间与时段起始时间差，单位为s
	
	UInt8 collectCycle;						//采集周期
	UInt8 checkTime;						//检测时间

	UInt16 isIgnorePhase;					//是否是忽略相位,每bit代表一个相位,0:不是,1:是

    UInt32 motorChanType;                   //配置的机动车类型和跟随类型通道，每bit代表一个通道
    UInt32 pedChanType;                     //配置的行人类型通道和行人跟随类型通道，每bit代表一个通道
	
	PhaseTimeInfo phaseTimes[MAX_PHASE_NUM];	//相位时间相关的信息
	FollowPhaseInfo followPhaseInfos[MAX_FOLLOWPHASE_NUM];	//跟随相位的相关信息
	
	UInt8 maxStageNum;						//最大阶段号
	StageInfo stageInfos[MAX_STAGE_NUM];	//阶段相关信息
	
	UInt8 includeNums[MAX_PHASE_NUM];						//每个相位所包含的阶段号个数
	UInt8 phaseIncludeStage[MAX_PHASE_NUM][MAX_STAGE_NUM];	//相位所包含的阶段号
} CalInfo;


#endif
