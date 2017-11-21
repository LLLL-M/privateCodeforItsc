#include <functional>
#include <algorithm>
#include "index.h"
#include "cycle.h"
#include "tsc.h"
#include "singleton.h"

const ColorStep ColorStepVector::Move(int t)
{
	int sum = 0;
	
	if (vec.size() == 0)
		return cur;
	if (used + t >= total)
		used = used + t - total;
	else if (used + t < 0)
		used = 0;
	else
		used += t;
	for (int i = 0; i < vec.size(); i++)
	{
		sum += vec[i].stepTime;
		if (sum > used)
		{
			cur = vec[i];
			pos = i;
			cur.stepTime = (UInt16)(sum - used);
			cur.countdown -= vec[i].stepTime - cur.stepTime;
			break;
		}
	}
	return cur;
}

void ColorStepVector::CalCountdown()
{
	function<void(ColorStep &)> Cal;
	Index i(vec.size(), 0);
	
	Cal = [&](ColorStep &current){
		const ColorStep &next = vec[i];
		if ((current.status == GREEN && next.status != GREEN && next.status != GREEN_BLINK) 
			|| (current.status == ALLRED && next.status != ALLRED && next.status != RED) 
			|| (current.status != GREEN && current.status != ALLRED && next.status != current.status)
			|| current.countdown >= total)
			return;
		current.countdown += next.stepTime;
		i++;
		Cal(current);
	};
	for (int step = 0; step < vec.size(); step++)
	{
		i = step;
		Cal(vec[step]);
	}
}

void Phase::Fill(PhaseInfo &pi, const UInt8 stage)
{
	phaseId = pi.phaseId;
	stageNum = stage;
	splitTime = pi.splitTime;
	splitLeftTime = (stage == 1) ? splitTime : 0;
	checkTime = pi.checkTime;
	vehDetectorBits = pi.vehDetectorBits;
	unitExtendTime = pi.unitExtendTime;
	maxExtendTime = pi.maxExtendTime;
	pedDetectorBits = pi.pedDetectorBits;
	advanceExtendTime = pi.advanceExtendTime;
	pedResponseTime = pi.pedResponseTime;
	pedPhase = pi.pedPhase;
	pedRequest = pi.pedRequest;
	motorRequest = pi.motorRequest;
	motor.Add(pi.motor);
	pedestrian.Add(pi.pedestrian);
}

Cycle::Cycle(vector<ColorStep> &vec) : rule({0, 0, 0}), maxStageNum(0)
{
	cycleTime = 0;
	for_each(vec.begin(), vec.end(),[this](ColorStep &cs){cycleTime += cs.stepTime;});
	leftTime = cycleTime;
	beginTime = 0;
	curStage = 0;
	curPhase = 0;
	Tsc &tsc = Singleton<Tsc>::GetInstance();
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (tsc.channelTable[i].channelType != NOUSE)
			channelTable[i].colorSteps.Add(vec);
	}
}

Cycle::Cycle(SchemeInfo &si) : rule(si.rule), maxStageNum((UInt8)si.phaseturn.size())
{
	cycleTime = si.cycleTime;
	leftTime = si.cycleTime;
	beginTime = 0;
	curStage = 1;
	curPhase = si.phaseturn[0].phaseId;
	
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		channelTable[i].Fill(si.channelInfo[i]);
	}
	UInt16 t = 0;
	UInt8 stageNum = 1;
	for (int p = 0; p < si.phaseturn.size(); p++)
	{
		auto &phaseInfo = si.phaseturn[p];
		if (phaseInfo.phaseId == 0 || phaseInfo.phaseId > MAX_PHASE_NUM)
			continue;
		phaseturn.push_back(phaseInfo.phaseId);
		auto &phase = phaseTable[phaseInfo.phaseId - 1];
		phase.motor.Add(t, RED);
		phase.pedestrian.Add(t, RED);
		phase.Fill(phaseInfo, stageNum++);
		t += phase.splitTime;
		phase.motor.Add(cycleTime - t, RED);
		phase.pedestrian.Add(cycleTime - t, RED);
		//phase.motor.PrintAll();
		//phase.pedestrian.PrintAll();
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			if (phaseInfo.channelBits.test(i))
			{
				if (si.phaseturn[(p + 1 == si.phaseturn.size()) ? 0 : (p + 1)].channelBits.test(i))	//如果下一阶段也包含此通道,则此阶段通道常绿
					channelTable[i].colorSteps.Add(phase.splitTime, GREEN);
				else
				{
					if (channelTable[i].type == MOTOR)
						channelTable[i].colorSteps.Add(phaseInfo.motor);
					else if (channelTable[i].type == PEDESTRIAN)
						channelTable[i].colorSteps.Add(phaseInfo.pedestrian);
				}
			}
			else
			{
				if (channelTable[i].type != NOUSE)
					channelTable[i].colorSteps.Add(phase.splitTime, RED);
			}
		}
	}
	for (auto &ph : phaseTable)
	{
		ph.motor.CalCountdown();
		ph.pedestrian.CalCountdown();
	}
	for (auto &ch : channelTable)
		ch.colorSteps.CalCountdown();
}	
