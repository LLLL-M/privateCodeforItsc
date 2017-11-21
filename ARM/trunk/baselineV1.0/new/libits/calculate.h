#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include "hik.h"
#include "HikConfig.h"
#include "gb.h"
#include "its.h"

#ifndef MAX_STAGE_NUM
#define MAX_STAGE_NUM	16
#endif
#ifndef MAX_PHASE_LIST_NUM
#define MAX_PHASE_LIST_NUM	16
#endif

extern SignalControllerPara *gRunConfigPara;
extern GbConfig *gRunGbconfig;
#define GET_PROTOCOL_PARAM(NTCIP_PARAM, GB2007_PARAM) ({\
	(gRunConfigPara != NULL) ? (NTCIP_PARAM) : \
		((gRunGbconfig != NULL) ? (GB2007_PARAM) : (0));})

typedef struct _PhaseTimeInfo
{
	UInt8 splitTime;
	UInt8 greenTime;
	UInt8 greenBlinkTime;
	UInt8 yellowTime;
	UInt8 allRedTime;
	//行人
    UInt8 pedestrianPassTime;//行人放行时间
    UInt8 pedestrianClearTime;//行人清空时间
	UInt8 maxExtendGreen;	//最大可以延长的绿灯时间
} PhaseTimeInfo;

typedef struct _StageInfo
{
	UInt8 runTime;					//阶段运行时间
	UInt8 greenTime;
	Boolean isBarrierStart;			//阶段是否是屏障起始
	Boolean isBarrierEnd;			//阶段是否是屏障结束
	UInt8 includeNum;				//阶段包含相位的个数
	UInt8 includePhases[MAX_PHASE_LIST_NUM];			//阶段包含相位
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
	UInt8 cycleTime;						//周期时间
	
	UInt8 coordinatePhaseId;				//协调相位
	UInt16 isCoordinatePhase;				//是否是协调相位,每bit代表一个相位,0:不是,1:是
	UInt8 phaseOffset;						//相位差
	int timeGapSec;							//当前时间与时段起始时间差，单位为s

	UInt16 isIgnorePhase;					//是否是忽略相位,每bit代表一个相位,0:不是,1:是
	
	PhaseTimeInfo phaseTimes[NUM_PHASE];	//相位时间相关的信息
	
	UInt8 maxStageNum;						//最大阶段号
	StageInfo stageInfos[MAX_STAGE_NUM];	//阶段相关信息
	
	UInt8 includeNums[NUM_PHASE];						//每个相位所包含的阶段号个数
	UInt8 phaseIncludeStage[NUM_PHASE][MAX_STAGE_NUM];	//相位所包含的阶段号
} CalInfo;

extern CalInfo gCalInfo;

static inline void SetPedestrianTime(int pedAutoRequestFlag, PhaseTimeInfo *times, 
					UInt8 pedestrianPassTime, UInt8 pedestrianClearTime)
{
	int timeGap = 0;
	
	/*如果相位设置了"行人自动请求"项，或者行人放行时间和行人清空时间之和大于机动车相位的绿灯和绿闪时间之和，那么行人放行时间和行人清空时间需要依赖机动车相位的绿灯和绿闪时间来调整 */
	if ((pedAutoRequestFlag == 1) 
		|| (times->greenTime + times->greenBlinkTime < pedestrianPassTime + pedestrianClearTime)) 
	{					
		timeGap = times->greenTime + times->greenBlinkTime - pedestrianClearTime;
		if (timeGap > 0) 
		{
			times->pedestrianPassTime = (UInt8)timeGap;
			times->pedestrianClearTime = pedestrianClearTime;
		} 
		else 
		{
			times->pedestrianPassTime = 0;
			times->pedestrianClearTime = pedestrianClearTime + timeGap;
		}
	} 
	else 
	{	
		times->pedestrianPassTime = pedestrianPassTime;
		times->pedestrianClearTime = pedestrianClearTime;
	}
}

/*****************************************************************************
 函 数 名  : GetPhaseStatusByTime
 功能描述  : 获取当前时间段内被使用通道的状态
 输入参数  : PhaseTimeInfo *times  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2014年11月27日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline LightStatus GetPhaseStatusByTime(PhaseTimeInfo *times)
{
    if (times->greenTime)
    {
		times->greenTime--;
		return GREEN;
	}
	if (times->greenBlinkTime)
    {
		times->greenBlinkTime--;
		return GREEN_BLINK;
	}
	if (times->yellowTime)
    {
		times->yellowTime--;
		return YELLOW;
	}
	if (times->allRedTime)
    {
		times->allRedTime--;
		return ALLRED;
	}
	return RED;
}

static inline Boolean IsPhaseIncludeStage(int phaseId, int stageNum)
{
	int i;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_LIST_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	for (i = 0; i < gCalInfo.includeNums[phaseId - 1]; i++)
	{
		if (stageNum == gCalInfo.phaseIncludeStage[phaseId - 1][i])
			return TRUE;
	}
	return FALSE;
}

static inline Boolean IsStageIncludePhase(int stageNum, int phaseId)
{
	int i;
	StageInfo *stageinfo = NULL;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_LIST_NUM 
		|| stageNum == 0 || stageNum > gCalInfo.maxStageNum)
		return FALSE;
	stageinfo = &gCalInfo.stageInfos[stageNum - 1];
	for (i = 0; i < stageinfo->includeNum; i++)
	{
		if (phaseId == stageinfo->includePhases[i])
			return TRUE;
	}
	return FALSE;
}

#define NEXT_STAGE(_CUR)	((_CUR) % gCalInfo.maxStageNum + 1)

extern void SetFollowPhaseInfo(PhaseInfo *phaseInfos, UInt8 curStage);

#endif
