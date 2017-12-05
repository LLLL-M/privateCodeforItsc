#include "ring.h"

void Ring::Excute()
{
	auto JudgeBarrierEnd = [this]{
		if (pos >= turn.size())
			return;
		Phase &phase = mp[turn[pos]];
		Phase &nextPhase = (pos + 1 == turn.size()) ? mp[turn[0]] : mp[turn[pos + 1]];
		barrierEnd = (phase.Over() && nextPhase.barrier != phase.barrier);	//如果当前相位已结束并且下一放行的相位屏障号不等于当前屏障号代表当前屏障结束
	};

	if (turn.empty())
		return;
	if (barrierEnd)
	{
		Phase &curPhase = mp[turn[pos]];
		if (curPhase.MotorStatus() == YELLOW)
			curPhase.SetStatus(ALLRED);	//如果相位以黄灯作为结束，在屏障已结束等待其他环结束时把状态置为ALLRED
		return;
	}
	if (runStatus == MOVEING)
	{
		for (unsigned int i = pos; i < turn.size(); i++)
		{
			if (mp[turn[i]].Move())	//相位移动1s
			{
				pos = i;
				JudgeBarrierEnd();
				return;
			}
		}
	}
	else if (runStatus == JUMPING)
	{
		Phase &curPhase = mp[turn[pos]];
		if (!curPhase.TransitionPeriod())
		{	//如果非过渡期则跳转到过渡时期
			curPhase.JumpToTransitionPeriod();//跳到过渡期
			JudgeBarrierEnd();
			return;
		}
		if (curPhase.Move())
		{
			JudgeBarrierEnd();
			return;
		}
		for (unsigned int i = 0; i < turn.size(); i++)
		{
			Phase &phase = mp[turn[i]];
			if (phase.phaseId == jumpPhase)
			{
				phase.Reset();
				phase.Move();//跳到某个相位之后必须先move一次才能得到第1s的状态
				runStatus = STAYING;
				pos = i;
				JudgeBarrierEnd();
				return;
			}
			else
				phase.SetOver();
		}
	}
}

void Ring::CalPhaseCountdown(const UInt16 cycleTime)
{
	for (unsigned int i = 0; i < turn.size(); i++)
	{
		Phase &phase = mp[turn[i]];
		if (i < pos)
			phase.SetCountdown(cycleTime - (Sum(i, pos) + mp[turn[pos]].Used()));
		else if (i == pos)
		{
			if (phase.MotorStatus() == ALLRED)
			{
				phase.SetCountdown(cycleTime - phase.Used());
				continue;	//因为机动车红灯时所对应的行人相位必定也是红灯了
			}
			if (phase.PedStatus() == RED || phase.PedStatus() == ALLRED)
				phase.SetPedCountdown(cycleTime - phase.Used());
		}
		else
			phase.SetCountdown(mp[turn[pos]].Left() + Sum(pos + 1, i));
	}
}
