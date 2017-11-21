static int CalculateTransitionTime(UInt16 cycleTime, int timeGap, int phaseOffset)
{ 
	static int transCycle = 0;		//��������
	int totalTransitionTime = 0;	//�ܹ���Ҫ���ɵ�ʱ��
	int transitionTime = 0;			//ÿ��������Ҫ����ʱ��
	int excessTime = 0;	//�ӵ�ǰʱ����ʼʱ�����е����ڵĶ���ʱ��

	if (gCalInfo.transitionCycle == 0 || cycleTime == 0)	//���û�����ù��������򲻽��й���
		return 0;
	excessTime = (timeGap - phaseOffset) % cycleTime;
	if (excessTime == 0 || excessTime == 1)
		transCycle = 0;
	else if (excessTime > 1 && excessTime <= 5)
	{	//�������ʱ����5s֮�ڣ�����ͨ���ݼ�Э����λʱ�����ﵽ����
		transCycle = 0;
		transitionTime = -excessTime;
	}
	else
	{
		if (transCycle == 0)
			transCycle = gCalInfo.transitionCycle;
		//����ʱ���ȥ����ʱ�䣬�ټ�����λ��ʱ�伴���ܹ���Ҫ����ʱ��
		totalTransitionTime = (excessTime > 0) ? (cycleTime - excessTime) : abs(excessTime);
		if (transCycle == 1 && totalTransitionTime > ((cycleTime * 3) / 4))
			totalTransitionTime = totalTransitionTime - (INT16)cycleTime;
		else if (transCycle > 1 && totalTransitionTime > (cycleTime / 2))
		{
			totalTransitionTime = totalTransitionTime - (INT16)cycleTime;
		}
		//���ܹ��Ĺ���ʱ��С�ڵ��ڹ������ڻ���С�ڵ���5sʱ������һ�����������
		transitionTime = (abs(totalTransitionTime) <= 5) //totalTransitionTime <= transCycle || 
						 ? totalTransitionTime 
						 : totalTransitionTime / transCycle;
		transCycle--;
	}
#if 0
#define MAX_CYCLE_TIME	0xff	//�������ʱ��
	if (cycleTime + transitionTime > MAX_CYCLE_TIME)
	{	//�������ʱ����Ϲ���ʱ������������ʱ��0xff,�����̹���ʱ�䲢���ӹ�������
		transitionTime = MAX_CYCLE_TIME - cycleTime;
		transCycle++;
	}
#endif	
	INFO("DEBUG cycleTime=%d, timeGap=%d, phaseOffset=%d, totalTransitionTime=%d, transitionTime=%d", cycleTime, timeGap, phaseOffset, totalTransitionTime, transitionTime);
	return transitionTime;
}

static void AllocateTransitionTime(int transitionTime)
{
	StageInfo *stageInfos = gCalInfo.stageInfos;
	int i = 0, j = 0;
	UINT8 phaseId = 0;
	INT32 stageSumTime = 0;
	INT32 stageallocTime[MAX_STAGE_NUM] = {0};
		
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		stageSumTime += gCalInfo.stageInfos[i].runTime;
	}
	stageallocTime[gCalInfo.maxStageNum - 1] = transitionTime;
	for (i = 0; i < gCalInfo.maxStageNum - 1; i++)
	{
		stageallocTime[i] = (INT32)(((double)gCalInfo.stageInfos[i].runTime / (double)stageSumTime) * transitionTime);
		stageallocTime[gCalInfo.maxStageNum - 1] -= stageallocTime[i];
	}
	for (i = 0; i < gCalInfo.maxStageNum; i++)
	{
		for (j = 0; j < stageInfos[i].includeNum; j++)
		{
			phaseId = stageInfos[i].includePhases[j];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			gCalInfo.phaseTimes[phaseId - 1].splitTime += stageallocTime[i];
			gCalInfo.phaseTimes[phaseId - 1].passTimeInfo.greenTime += stageallocTime[i];
		}
		stageInfos[i].runTime += stageallocTime[i];
		stageInfos[i].passTimeInfo.greenTime += stageallocTime[i];
	}
}

static void TransitionDeal(void)
{
	int phaseOffset = gCalInfo.phaseOffset;	//Э����λ��
    int transitionTime = 0;	//ÿ��������Ҫ���ɵ�ʱ��
	StageInfo *stageInfos = gCalInfo.stageInfos;
	UInt8 phaseBeginStageNum = 0;	//��λ��ʼ�׶κ�
	int s, phaseId, i;
	
	if (gCalInfo.coordinatePhaseId == 0 || gCalInfo.coordinatePhaseId > MAX_PHASE_NUM)
		return;
	phaseBeginStageNum = gCalInfo.phaseIncludeStage[gCalInfo.coordinatePhaseId - 1][0];
	if (phaseBeginStageNum == 0 || phaseBeginStageNum > gCalInfo.maxStageNum)
	{
		ERR("transition: phaseBeginStageNum[%d] is invalid!", phaseBeginStageNum);
		return;
	}
	for (s = 1; s < phaseBeginStageNum; s++)
	{
		phaseOffset -= stageInfos[s - 1].runTime;
	}
	transitionTime = CalculateTransitionTime((gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID) ? gCalInfo.inductiveCoordinateCycleTime : gCalInfo.cycleTime, 
					gCalInfo.timeGapSec, phaseOffset);
    if (transitionTime == 0)
		return;
	if (gCalInfo.actionId == INDUCTIVE_COORDINATE_ACTIONID)
		gCalInfo.inductiveCoordinateCycleTime += transitionTime;
	gCalInfo.cycleTime += transitionTime;
	AllocateTransitionTime(transitionTime);
	/*
	if (gCalInfo.phaseTimes[gCalInfo.coordinatePhaseId - 1].splitTime == 0)
	{	//˵���ǰ��׶ν�����ʱ����ֱ���ӳ��׶��̵�ʱ��
		stageInfos[phaseBeginStageNum - 1].runTime += transitionTime;
		stageInfos[phaseBeginStageNum - 1].passTimeInfo.greenTime += transitionTime;
	}
	else
	{	//˵���ǰ���λ������ʱ�����ӳ�Э����λ������ʼ�׶�����������λ���̵�ʱ���Լ����ű�
		for (i = 0; i < stageInfos[phaseBeginStageNum - 1].includeNum; i++)
		{
			phaseId = stageInfos[phaseBeginStageNum - 1].includePhases[i];
			if (phaseId == 0 || phaseId > MAX_PHASE_NUM)
				break;
			gCalInfo.phaseTimes[phaseId - 1].splitTime += transitionTime;	//�������ű�ʱ��
			gCalInfo.phaseTimes[phaseId - 1].passTimeInfo.greenTime += transitionTime;	//�����̵�ʱ��
		}
		stageInfos[phaseBeginStageNum - 1].runTime += transitionTime;
	}	
	*/
}
