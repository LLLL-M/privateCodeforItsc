#include <cstring>
#include <algorithm>
#include "ipc.h"
#include "protocol.h"
#include "calculate.h"

using namespace HikCalculate;

#include "transition.hpp"		//对于协调绿波相关的处理函数
#include "followphase.hpp"	//对于跟随相位相关的处理函数

inline bool Calculate::IsPhaseIncludeStage(int phaseId, int stageNum)
{
	int i;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_NUM 
		|| stageNum == 0 || stageNum > calInfo.maxStageNum)
		return false;
	for (i = 0; i < calInfo.includeNums[phaseId - 1]; i++)
	{
		if (stageNum == calInfo.phaseIncludeStage[phaseId - 1][i])
			return true;
	}
	return false;
}

inline bool Calculate::IsStageIncludePhase(int stageNum, int phaseId)
{
	int i;
	StageInfo *stageinfo = NULL;
	
	if (phaseId == 0 || phaseId > MAX_PHASE_NUM 
		|| stageNum == 0 || stageNum > calInfo.maxStageNum)
		return false;
	stageinfo = &calInfo.stageInfos[stageNum - 1];
	for (i = 0; i < stageinfo->includeNum; i++)
	{
		if (phaseId == stageinfo->includePhases[i])
			return true;
	}
	return false;
}

inline void Calculate::SendSpecialControlData(UInt8 schemeId, UInt8 leftTime)
{
    LineQueueData data;
    int i;
    LightStatus status = INVALID;

	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: status = YELLOW_BLINK; break;	//黄闪控制
		case ALLRED_SCHEMEID: status = ALLRED; break;		//全红控制
		case TURNOFF_SCHEMEID: status = TURN_OFF; break;	//关灯控制
		default: break;
	}
    std::memset(&data, 0, sizeof(data));
    for (i = 0; i < MAX_CHANNEL_NUM; i++)
    {
		data.allChannels[i] = status;
    }
    data.schemeId = schemeId;
	data.leftTime = leftTime;
    if (0 != ipc.gLfq.lfq_write(static_cast<void *>(&data)))
		ERR("fill line queue fail");
}

void Calculate::YellowBlinkAllRedInit()
{
	UInt8 firstCycle = bootYellowFlashTime + bootAllRedTime;
	int i;
	
	//把黄闪和全红作为第一个周期，把周期的剩余时间写入信息中便于相位控制模块SystemInit函数使用
	if (firstCycle == 0)
	{	//如果没有配置黄闪和全红的启动时间则首先发送1s的关灯数据便于相位控制模块SystemInit函数使用
		SendSpecialControlData(TURNOFF_SCHEMEID, 1);
	}
	else
	{
		//初始化为黄闪控制
		for (i = 0; i < bootYellowFlashTime; i++) 	
			SendSpecialControlData(YELLOWBLINK_SCHEMEID, firstCycle--);
		//初始化为全红控制
		for (i = 0; i < bootAllRedTime; i++) 
			SendSpecialControlData(ALLRED_SCHEMEID, firstCycle--);
	}
	//告知相位控制模块开始读取数据
	ipc.SemPostBeginReadData();
}

inline UInt8 Calculate::GetPhaseEndStageNum(UInt8 phaseId, UInt8 curStageNum)
{
	UInt8 ret = curStageNum;
	UInt8 s;
	
	if (curStageNum == 0 || curStageNum > calInfo.maxStageNum
		|| phaseId == 0 || phaseId > MAX_PHASE_NUM)
		return 0;
	for (s = nextStage(curStageNum); s != curStageNum; s = nextStage(s))
	{
		if (IsPhaseIncludeStage(phaseId, s))
			ret = s;
		else
			break;
	}
	if (s == curStageNum)
		return 0;
	else
		return ret;
}

inline void Calculate::SetPedestrianTime(PhaseTimeInfo *times)
{
	int timeGap = 0;
	UInt8 pedestrianPassTime = times->pedestrianPassTime;
	UInt8 pedestrianClearTime = times->pedestrianClearTime;

	if (times->splitTime == 0)
		return;
/*如果相位设置了"行人自动请求"项，或者行人放行时间和行人清空时间之和大于机动车相位的绿灯和绿闪时间之和，那么行人放行时间和行人清空时间需要依赖机动车相位的绿灯和绿闪时间来调整 */
	//if (pedestrianClearTime == 0)
		//pedestrianClearTime = 10;	//如果没有配置行人清空，则默认10s
	if ((times->pedAutoRequestFlag == 1) 
		|| (times->passTimeInfo.greenTime + times->passTimeInfo.greenBlinkTime < pedestrianPassTime + pedestrianClearTime)) 
	{					
		timeGap = times->passTimeInfo.greenTime + times->passTimeInfo.greenBlinkTime - pedestrianClearTime;
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
}

void Calculate::SetStagePhaseTime(UInt8 stageNum)
{
	int i, s;
	UInt8 endStageNum = 0;
	UInt16 tmpGreenTime = 0;
	PhaseTimeInfo *phaseTimes = calInfo.phaseTimes;
	PassTimeInfo *passTimeInfo;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		passTimeInfo = &phaseTimes[i].passTimeInfo;
		if (IsPhaseIncludeStage(i + 1, stageNum) == false
			|| (passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + passTimeInfo->yellowTime 
				+ passTimeInfo->redYellowTime + passTimeInfo->allRedTime > 0))
			continue;
		/*判断相位从当前阶段开始终止运行的阶段号，如果返回的阶段号为0，则说明此相位一直在运行*/
		endStageNum = GetPhaseEndStageNum(i + 1, stageNum);
		if (endStageNum == 0)
		{
			phaseTimes[i].splitTime = calInfo.cycleTime;
			passTimeInfo->greenTime = calInfo.cycleTime;
			phaseTimes[i].pedestrianPassTime = calInfo.cycleTime;
			continue;
		}
		else if (endStageNum == stageNum)
		{
			std::memcpy(passTimeInfo, &calInfo.stageInfos[stageNum - 1].passTimeInfo, sizeof(PassTimeInfo));
			phaseTimes[i].splitTime = calInfo.stageInfos[stageNum - 1].runTime;
		}
		else
		{	//说明此相位跨阶段运行
			tmpGreenTime = 0;
			for (s = stageNum; s != endStageNum; s = nextStage(s))
			{
				tmpGreenTime += calInfo.stageInfos[s - 1].runTime;
			}
			std::memcpy(passTimeInfo, &calInfo.stageInfos[endStageNum - 1].passTimeInfo, sizeof(PassTimeInfo));
			phaseTimes[i].splitTime = tmpGreenTime + calInfo.stageInfos[endStageNum - 1].runTime;
			passTimeInfo->greenTime += tmpGreenTime;
		}
		SetPedestrianTime(&phaseTimes[i]);//设置行人相关的时间
	}
}

inline LightStatus Calculate::GetPhaseStatusByTime(PassTimeInfo *passTimeInfo)
{
    if (passTimeInfo->greenTime)
    {
		passTimeInfo->greenTime--;
		return GREEN;
	}
	if (passTimeInfo->greenBlinkTime)
    {
		passTimeInfo->greenBlinkTime--;
		return GREEN_BLINK;
	}
	if (passTimeInfo->yellowTime)
    {
		passTimeInfo->yellowTime--;
		return YELLOW;
	}
	if (passTimeInfo->redYellowTime)
    {
		passTimeInfo->redYellowTime--;
		return RED_YELLOW;
	}
	if (passTimeInfo->allRedTime)
    {
		passTimeInfo->allRedTime--;
		return ALLRED;
	}
	return RED;
}

//找到相位从下一阶段开始，下次开始运行要经历的时间
UInt16 Calculate::FindPhaseNextRunPassTime(UInt8 phaseId, UInt8 curStageNum)
{
	int s = nextStage(curStageNum);
	UInt16 ret = 0;

	if (curStageNum == 0 || curStageNum > calInfo.maxStageNum)
		return 0;
	if (IsStageIncludePhase(curStageNum, phaseId))
	{	//如果当前阶段包含此phaseId,则跳过连续放行的阶段
		for (; s != curStageNum; s = nextStage(s))
		{
			if (IsStageIncludePhase(s, phaseId) == false)
				break;
		}
	}
	for (; s != curStageNum; s = nextStage(s))
	{
		if (IsStageIncludePhase(s, phaseId))
			break;
		else
			ret += calInfo.stageInfos[s - 1].runTime;
	}
	return ret;
}

void Calculate::SetChannelInfo(LineQueueData *data, UInt32 channelBits, UInt8 status, UInt16 leftTime)
{
	int i;
	
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (GET_BIT(channelBits, i) == 1)
		{
			if (((data->allChannels[i] == status)
					&& (((status == GREEN || status == GREEN_BLINK 
								|| status == YELLOW || status == RED_YELLOW)
							&& data->channelCountdown[i] < leftTime)
						|| ((status == ALLRED || status == RED)
							&& data->channelCountdown[i] > leftTime)))
				|| ((data->allChannels[i] != status)
					&& ((data->allChannels[i] == INVALID || data->allChannels[i] == RED)
						|| (data->allChannels[i] == ALLRED && status != RED)
						|| (data->allChannels[i] == RED_YELLOW && (status == YELLOW
																 || status == GREEN_BLINK
																 || status == GREEN))
						|| (data->allChannels[i] == YELLOW && (status == GREEN_BLINK
																 || status == GREEN))
						|| (data->allChannels[i] == GREEN_BLINK && status == GREEN))))
			{
				data->allChannels[i] = status;
				data->channelCountdown[i] = leftTime;
				if (data->allChannels[i] == YELLOW && ptl.isYellowFlash)
					data->allChannels[i] = YELLOW_BLINK;
				if (data->allChannels[i] == RED && ptl.redFlashSec >= data->channelCountdown[i])
					data->allChannels[i] = RED_BLINK;
			}
		}
	}
}

void Calculate::SetStagePhaseInfo(UInt8 stageNum, UInt16 stageLeftTime, LineQueueData *data)
{
	int i, s;
	LightStatus status;
	PhaseTimeInfo *times = NULL;
	PassTimeInfo *passTimeInfo = NULL;
	PhaseInfo *phaseinfos = data->phaseInfos;
	UInt8 phaseId = 0;
	
	if (stageNum == 0 || stageNum > calInfo.maxStageNum)
		return;
	for (i = 0; i < calInfo.stageInfos[stageNum - 1].includeNum; i++)
	{
		phaseId = calInfo.stageInfos[stageNum - 1].includePhases[i];
		if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
			break;
		times = &calInfo.phaseTimes[phaseId - 1];
		passTimeInfo = &times->passTimeInfo;
		status = GetPhaseStatusByTime(passTimeInfo);
		phaseinfos[phaseId - 1].phaseStatus = status;
		if (status == GREEN || status == GREEN_BLINK)	//绿灯倒计时
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + 1;
		else if (status == YELLOW)	//黄灯倒计时
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->yellowTime + 1;
		else if (status == RED_YELLOW)	//红黄倒计时
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->redYellowTime + 1;
		else	//红灯倒计时
			phaseinfos[phaseId - 1].phaseLeftTime = passTimeInfo->allRedTime + 1 + FindPhaseNextRunPassTime(phaseId, stageNum);
		phaseinfos[phaseId - 1].splitTime = times->splitTime;
		phaseinfos[phaseId - 1].phaseSplitLeftTime = passTimeInfo->greenTime + passTimeInfo->greenBlinkTime + passTimeInfo->yellowTime + passTimeInfo->redYellowTime + passTimeInfo->allRedTime + 1;
		if (times->pedestrianPassTime)
		{
			times->pedestrianPassTime--;
			status = GREEN;
		}
		else
		{
			if (times->pedestrianClearTime) 
			{
				times->pedestrianClearTime--;
				status = GREEN_BLINK;
			}
			else
				status = RED;
		}
		phaseinfos[phaseId - 1].pedestrianPhaseStatus = status;
		if (status == GREEN || status == GREEN_BLINK)	//行人绿灯倒计时
			phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = times->pedestrianPassTime + times->pedestrianClearTime + 1;
		else	//行人红灯倒计时
			phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = phaseinfos[phaseId - 1].phaseSplitLeftTime + FindPhaseNextRunPassTime(phaseId, stageNum);
		SetChannelInfo(data, times->motorChannelBits, phaseinfos[phaseId - 1].phaseStatus, phaseinfos[phaseId - 1].phaseLeftTime);
		SetChannelInfo(data, times->pedChannelBits, phaseinfos[phaseId - 1].pedestrianPhaseStatus, phaseinfos[phaseId - 1].pedestrianPhaseLeftTime);
		phaseinfos[phaseId - 1].vehicleDetectorBits = times->vehicleDetectorBits;
		phaseinfos[phaseId - 1].unitExtendGreen = times->unitExtendGreen;
		phaseinfos[phaseId - 1].maxExtendGreen = times->maxExtendGreen;
	}
	//设置未放行相位的状态和倒计时
	for (s = nextStage(stageNum); s != stageNum; s = nextStage(s))
	{
		for (i = 0; i < calInfo.stageInfos[s - 1].includeNum; i++)
		{
			phaseId = calInfo.stageInfos[s - 1].includePhases[i];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			if (IsPhaseIncludeStage(phaseId, stageNum) == false)
			{	//如果相位不在当前阶段而在其他阶段，则设置此相位的信息
				phaseinfos[phaseId - 1].splitTime = calInfo.phaseTimes[phaseId - 1].splitTime;
				phaseinfos[phaseId - 1].phaseSplitLeftTime = phaseinfos[phaseId - 1].splitTime;
				phaseinfos[phaseId - 1].phaseStatus = RED;
				phaseinfos[phaseId - 1].phaseLeftTime = FindPhaseNextRunPassTime(phaseId, stageNum) + stageLeftTime;
				phaseinfos[phaseId - 1].pedestrianPhaseStatus = RED;
				phaseinfos[phaseId - 1].pedestrianPhaseLeftTime = phaseinfos[phaseId - 1].phaseLeftTime;
				SetChannelInfo(data, calInfo.phaseTimes[phaseId - 1].motorChannelBits, RED, phaseinfos[phaseId - 1].phaseLeftTime);
				SetChannelInfo(data, calInfo.phaseTimes[phaseId - 1].pedChannelBits, RED, phaseinfos[phaseId - 1].phaseLeftTime);
			}
		}
	}
}

void Calculate::RunCycle()
{
	LineQueueData data;
	UInt8 stageNum = 0;		//阶段号
	UInt16 stageStartRunTime = 0;    			//阶段开始时已经运行的周期时间
	int i, t;
	
	for (i = 0; i < MAX_PHASE_NUM; i++)
		SetPedestrianTime(&calInfo.phaseTimes[i]);//设置行人相关的时间
	for (t = 0; t < calInfo.cycleTime; t++) 
	{	//一次循环获取1秒钟各通道的状态以及其他信息发送给相位控制模块
		std::memset(&data, 0, sizeof(data));
		if (t == stageStartRunTime)
		{
			if (stageNum == calInfo.maxStageNum)
				break;
			stageNum++;
			stageStartRunTime += calInfo.stageInfos[stageNum - 1].runTime;
			SetStagePhaseTime(stageNum);
		}
		SetStagePhaseInfo(stageNum, stageStartRunTime - t, &data);
        SetFollowPhaseInfo(&data, stageNum);
		data.cycleTime = calInfo.cycleTime;
		data.inductiveCoordinateCycleTime = calInfo.inductiveCoordinateCycleTime;
		//data.runTime = t + 1;
		data.leftTime = data.cycleTime - t;
		data.schemeId = calInfo.schemeId;
		data.stageNum = stageNum;
		data.maxStageNum = calInfo.maxStageNum;
		data.collectCycle = calInfo.collectCycle;
		data.checkTime = calInfo.checkTime;
		data.phaseTableId = calInfo.phaseTableId;
		data.channelTableId = calInfo.channelTableId;
		data.actionId = calInfo.actionId;
		while (0 != ipc.gLfq.lfq_write(static_cast<void *>(&data)))	//保证一定能把数据写入到队列中
		{
			msleep(500);
			ERR("fill line queue fail");
		}
	}
	//INFO("max stage num is %d", stageNum);
}

void Calculate::run(void *arg)
{	
	UInt16 lastCycleSchemeId = 256;	//上一周期方案号
	UInt8 switchSchemeId = 0;	//切换方案号
	Ipc::CalMsg msg;
	
	YellowBlinkAllRedInit();
    while (1)
    {
		std::memset(&msg, 0, sizeof(msg));
		if (!ipc.MsgRecv(msg))
		{
			msleep(10);
			continue;
		}
		//为防止相位控制模块多次发送计算下一周期消息，因此作一判断
		if (ipc.gLfq.lfq_element_count() > ptl.aheadOfTime)  
		    continue;
		//根据当前时间获取方案，并初始化各相位的绿信比时间段
		if (ptl.FillCalInfo(&calInfo, msg.schemeId, msg.calTime) == true) 
		{	
			if (calInfo.isIgnorePhase > 0)	//表示有设置忽略相位
				ptl.IgnorePhaseDeal(&calInfo);	//对忽略相位进行处理
			//只有非感应、非自适应、非手动运行方案时才可进行协调绿波
			if (calInfo.actionId != INDUCTIVE_ACTIONID	
				&& calInfo.schemeId != SINGLE_ADAPT_SCHEMEID
				&& calInfo.schemeId != YELLOWBLINK_SCHEMEID
				&& calInfo.schemeId != ALLRED_SCHEMEID
				&& calInfo.schemeId != TURNOFF_SCHEMEID
				&& (msg.schemeId == 0 || calInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
				&& calInfo.coordinatePhaseId > 0)
					TransitionDeal();	//对协调相位进行过渡处理
			RunCycle();	//每次轮询一个完整的周期
			switchSchemeId = calInfo.schemeId;
		}
		else
		{
			switchSchemeId = YELLOWBLINK_SCHEMEID;
		}
		if (lastCycleSchemeId != switchSchemeId)
		{
			//log_debug("lastCycleSchemeId=%d, msgschemeid=%d, gSchemeId=%d, cycleTime=%d", lastCycleSchemeId, msg.schemeId, calInfo.schemeId, calInfo.cycleTime);	
		}
		if (calInfo.actionId == INDUCTIVE_ACTIONID)
			ipc.MsgSend(AUTO_CONTROL, INDUCTIVE_SCHEMEID, switchSchemeId);	//感应方案切换
		else if (calInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
			ipc.MsgSend(AUTO_CONTROL, INDUCTIVE_COORDINATE_SCHEMEID, switchSchemeId);	//协调感应方案切换
		else
			ipc.MsgSend(AUTO_CONTROL, switchSchemeId, 0);	//本地方案切换
		lastCycleSchemeId = switchSchemeId;
    }
}

Calculate::Calculate(Protocol & p, Ipc & i) : ptl(p), ipc(i)
{
	p.SetStartTime(bootYellowFlashTime, bootAllRedTime);
	std::memset(&calInfo, 0, sizeof(CalInfo));
	start();
}
