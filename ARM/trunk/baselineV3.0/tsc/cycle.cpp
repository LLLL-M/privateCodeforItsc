#include "cycle.h"

void Cycle::CalCountdown()
{
	if (beginTime == 0)
		beginTime = std::time(nullptr);
	if (ringTable.empty())
		return;
	cycleTime = ringTable[0].Total();
	leftTime = ringTable[0].Left();

	for (auto &ring: ringTable)
		ring.CalPhaseCountdown(cycleTime);
	/*先把通道在所有环中的倒计时存入到对应的infos中，存入是按照放行期在前，红灯时期依次靠后的顺序排列(排序的实现在ChannelInfo类中opeator<()函数)*/
	for (auto &it: phaseTable)
	{
		Phase &phase = it.second;
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			if (phase.channelBits[i])
			{
				Channel &channel = channelTable[i];
				if (channel.type == CHANNEL_TYPE_CAR)
					channel.AddInfo(phase.MotorColorStep(), phase.Inuse() ? phase.Left() : phase.Total(), phase.Green());
				else if (channel.type == CHANNEL_TYPE_PED)
					channel.AddInfo(phase.PedColorStep(), phase.Inuse() ? phase.Left() : phase.Total(), phase.Green());
			}
		}
	}
	/*对每个通道infos中的倒计时进行统一的计算得出最终的倒计时和状态*/
	for (auto &channel: channelTable)
		channel.CalInfo();
}

void Cycle::SpecialCtrl(UInt8 mode)
{
	TscStatus st;
	switch (mode)
	{
		case TURNOFF_MODE: st = TURN_OFF; break;
		case YELLOWBLINK_MODE: st = YELLOW_BLINK; break;
		case ALLRED_MODE: st = ALLRED; break;
		default: st = INVALID;
	}
	for (auto &it: phaseTable)
		it.second.SetStatus(st);
	for (auto &channel: channelTable)
	{
		if (channel.specifyStatus == INVALID)
			channel.status = st;
		else
			channel.status = channel.specifyStatus;
	}
	specialCtrl = true;
}

void Cycle::Excute()
{
	if (specialCtrl)
		return;

	std::vector<UInt8> curPhases;
	size_t barrierEndCount = 0;
	for (auto &ring: ringTable)
	{
		ring.Excute();
		if (ring.barrierEnd)
			barrierEndCount++;
		curPhases.push_back(ring.CurPhaseId());
	}

	if (barrierEndCount == ringTable.size())	//如果所有环的屏障都结束了则清除屏障结束标志表明可以进入下一个屏障了
		std::for_each(ringTable.begin(), ringTable.end(), [](Ring &ring){ring.barrierEnd = false;});
	//根据当前运行的相位设置当前阶段号
	for (auto &stage: stages)
	{
		if (operator==(stage.phase, curPhases))
		{
			curStage = stage.id;
			break;
		}
	}
	CalCountdown();
#if 0
	INFO("**************************");
	for (auto &it : phaseTable)
		it.second.MotorColorStep().Print();
	cout << "cycleTime: " << cycleTime << ", leftTime: " << leftTime << endl;
	INFO("**************************\n");
#endif
}
