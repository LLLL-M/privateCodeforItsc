#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "platform.h"
#include "HikConfig.h"
#include "specialControl.h"
#include "configureManagement.h"

#define MAX_STEP_NUM	16
#define STEP_MSG_TYPE	100

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define LOFF		0
#define LGREEN		1
#define LYELLOW		4
#define LRED		2
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef struct stepParameterInfo
{
	UInt8 cycleTime;	//��ǰ����ʱ��
	UInt8 runTime;		//�Ѿ����е�ʱ��
	UInt8 maxStageNo;	//���׶κ�
	UInt8 curStageNo;	//��ǰ���еĽ׶κ�
	UInt8 nextStageNo;	//��һ���������еĽ׶κ�
	UInt8 stageIncludePhase[MAX_STEP_NUM][NUM_RING_COUNT];	//�׶�����������λ��
	UInt8 stageStartRunTime[MAX_STEP_NUM];	//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt8 phaseStatus[NUM_PHASE];	//��λ״̬
	UInt8 nextPhaseIds[NUM_PHASE];	//���ÿ����λ����֮�󼴽����е���һ��λ��
} StepParams;

static StepParams gStepParams;

int msgid = -1;
UInt8 gStepFlag = 0;	//������־��0����ʾδ������1����ʾ����
UInt16 gLightArray[8] = {0};	//����ʱʹ�õĵ������
pthread_rwlock_t gLightArrayLock = PTHREAD_RWLOCK_INITIALIZER;	//��������������
static UInt8 gTransitionFlag = 0; //�������ɱ�־

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;
extern UInt8 outPutCount; 	//����ʱ�ӿڵ��ü���
extern pthread_rwlock_t gCountDownLock;
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS gCountDownParamsSend ;       //������udpserver���õĵ���ʱ����
extern PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //��ǰʹ�õ�����
extern void FindNextPhaseIdInPhaseTrun(PhaseTurnItem *phaseTrunTable, UInt8 *nextPhaseIds);
extern Boolean IsNextPhaseInFollowPhaseList(FollowPhaseItem *followPhaseItem, UInt8 nextPhaseId);
extern STRUCT_BINFILE_CUSTOM gStructBinfileCustom;      //�����/home/custom.dat�����е����нṹ��,������Ե���ʱ��Э�������/��Դ��ڲ���������/���ͨ����������������/ͨ��������ʶ
/*****************************************************************************
 �� �� ��  : Light
 ��������  : ��������ͨ����״̬�����õ�Ƶ�����
 �������  : UInt8 *allChannel  		��������ͨ��״̬������ָ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static void SetLightArray(UInt8 *allChannel)
{
	int i;
	UInt16 tmp, value;
	UInt16 *nOutLamp = gLightArray;
	
	pthread_rwlock_wrlock(&gLightArrayLock);
	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		//����ͨ������״̬�ҳ����ͨ��Ӧ��������ֵ
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK:
						tmp = get_lamp_value(nOutLamp, i % 4);
						value = (tmp == LOFF) ? LGREEN : LOFF;
						break;	
			case YELLOW:	value = LYELLOW; break;
			case YELLOW_BLINK:
							tmp = get_lamp_value(nOutLamp, i % 4);
							value = (tmp == LOFF) ? LYELLOW: LOFF;
							break;
			case RED:	value = LRED; break;
			case ALLRED: value = LRED; break;
			default: value = LOFF; break;
		}
		//�����ͨ����ֵ
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			nOutLamp++;
		}
	}
	pthread_rwlock_unlock(&gLightArrayLock);
}
//������λ״̬����ͨ��״̬,������õ������
static void SetChannel(StepParams *p)
{    
    int i = 0, j = 0;
    UInt8 channelId = 0;
	PhaseChannelStatus status, newStatus, motherPhaseStatus;
	UInt8 phaseId, motherPhase;
	UInt8 allChannel[NUM_CHANNEL];
	UInt8 followPhaseStatus[NUM_PHASE] = {0};
	UInt8 *phaseStatus = p->phaseStatus;
	UInt8 *nextPhaseIds = p->nextPhaseIds;

	pthread_rwlock_rdlock(&gConfigLock);
    for(i = 0; i < NUM_CHANNEL; i++)
    {
    	channelId = gSignalControlpara->stChannel[i].nChannelID;
		phaseId = gSignalControlpara->stChannel[i].nControllerID;
		if (channelId == 0 || phaseId == 0)
		{
			allChannel[i] = TURN_OFF;
			continue;
		}	
		
		status = RED;
		switch (gSignalControlpara->stChannel[i].nControllerType) 
		{
			case MOTOR: status = phaseStatus[phaseId - 1]; break;
			case PEDESTRIAN: 
				status = phaseStatus[phaseId - 1]; 
				if (status == YELLOW)
					status = RED;
				break;
			case FOLLOW: 
				for (j = 0; j < NUM_PHASE; j++)
				{
					motherPhase = gSignalControlpara->stFollowPhase[phaseId - 1].nArrayMotherPhase[j];
					if (motherPhase == 0) 
						break;
					motherPhaseStatus = phaseStatus[motherPhase - 1];
					if (motherPhaseStatus != RED && motherPhaseStatus != INVALID && motherPhaseStatus != TURN_OFF)
					{	//��ĸ��λ��������ʱ���������������һ��λҲ�ڸ�����λ���У���ô��ʱ������λ״̬Ӧ�ǳ���GREEN����֮����״̬��ĸ��λ״̬һ��
						newStatus = IsNextPhaseInFollowPhaseList(&gSignalControlpara->stFollowPhase[phaseId - 1], nextPhaseIds[motherPhase - 1]) ? GREEN : motherPhaseStatus;
						if (status == RED)
						{
							status = newStatus;
							continue;
						}
						//���Դ���໷ʱĸ��λͬʱ���е����
						if ((newStatus == GREEN) 
								|| (newStatus == GREEN_BLINK && (status == YELLOW || status == ALLRED))
								|| (newStatus == YELLOW && status == ALLRED))
							status = newStatus;
					}
				}
				followPhaseStatus[phaseId - 1] = (status == ALLRED) ? RED : status;
				break;
			default: status = TURN_OFF; break;
		}
		allChannel[i] = (UInt8)status;
    }
	pthread_rwlock_unlock(&gConfigLock);
	SetLightArray(allChannel);

	pthread_rwlock_wrlock(&gCountDownLock);
	gCountDownParams->ucCurRunningTime = p->stageStartRunTime[p->curStageNo - 1];
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (phaseStatus[i] == ALLRED)
			phaseStatus[i] = RED;
		gCountDownParams->stVehPhaseCountingDown[i][0] = phaseStatus[i];
		gCountDownParams->stVehPhaseCountingDown[i][1] = 0;
		if (gCountDownParams->stPedPhaseCountingDown[i][0] != INVALID)
			gCountDownParams->stPedPhaseCountingDown[i][0] = (phaseStatus[i] == YELLOW) ? RED : phaseStatus[i];
		gCountDownParams->stPedPhaseCountingDown[i][1] = 0;
		gCountDownParams->stPhaseRunningInfo[i][1] = 0;
		gCountDownParams->ucOverlap[i][0] = followPhaseStatus[i];
		gCountDownParams->ucOverlap[i][1] = 0;
	}
	pthread_rwlock_unlock(&gCountDownLock);
}

//ͨ������ָ���׶���Ҫ���е�ʱ��������ָ���׶ε���λ״̬
static inline void SetPhaseStatus(UInt8 stageNo, StepParams *p)
{
	int i = 0;
	UInt8 phaseId = 0;
	
	p->curStageNo = stageNo;
	p->nextStageNo = (stageNo + 1 > p->maxStageNo) ? 1 : (stageNo + 1);
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (p->phaseStatus[i] != INVALID)
			p->phaseStatus[i] = RED;
	}
	for (i = 0; i < NUM_RING_COUNT; i++)
	{
		phaseId = p->stageIncludePhase[stageNo - 1][i];
		if (phaseId == 0)
			break;
		p->phaseStatus[phaseId - 1] = GREEN;
	}
}

//���ö�����ʱ����ÿ���׶���Ҫ���е�ʱ�䣬�����ݵ�ǰ����ʱ��������ҳ���һ�����еĽ׶κ�
static void SetStepParams(StepParams *p, UInt8 *phaseSplits)
{
	int i = 0, ring = 0, n = 0;
    UInt8 nPhaseId = 0;
	UInt8 stageStartRunTime = 0;	//�׶ο�ʼʱ�Ѿ����е�����ʱ��
	UInt8 stageEndRunTime = 0;	//�׶ν����Ѿ����е�����ʱ��
	UInt8 stageNo = 0;	//�׶κ�
	UInt8 phaseEndRunTime = 0;	//ÿ��������ִ����ÿ����λ���е�ʱ��
	UInt8 curStageNo = 1;
	
	INFO("runTime = %d, cycleTime = %d\n", p->runTime, p->cycleTime);
	while (stageStartRunTime < p->cycleTime) 
	{	
		stageNo++;
		n = 0;
		for (ring = 0; ring < NUM_RING_COUNT; ring++) 
		{	
			phaseEndRunTime = 0;
			for (i = 0; i < NUM_PHASE; i++) 
			{
				nPhaseId = gPhaseTrunTable[ring].nTurnArray[i];
				if (nPhaseId == 0) 
				{
					break;
				}
				phaseEndRunTime += phaseSplits[nPhaseId - 1];
				if (phaseEndRunTime > stageStartRunTime)
				{
					if (stageEndRunTime == stageStartRunTime)
						stageEndRunTime = phaseEndRunTime;
					else
						stageEndRunTime = min(stageEndRunTime, phaseEndRunTime);
					p->stageIncludePhase[stageNo - 1][n++] = nPhaseId;
					break;
				}
			}
		}
		if ((p->runTime > stageStartRunTime) && (p->runTime <= stageEndRunTime))
			curStageNo = stageNo;
		p->stageStartRunTime[stageNo - 1] = stageStartRunTime + 1;	//��ʱ��1��ʼ
		stageStartRunTime = stageEndRunTime;
	}
	p->curStageNo = curStageNo;
	p->nextStageNo = (curStageNo + 1 > stageNo) ? 1 : (curStageNo + 1);
	p->maxStageNo = stageNo;
	INFO("curStageNo = %d, maxStageNo = %d", p->curStageNo, p->maxStageNo);
	for (i = 0; i < MAX_STEP_NUM; i++)
	{
		if (p->stageIncludePhase[i][0] == 0)
			break;
		INFO("stage %d include phase : %d, %d, %d, %d", i + 1,
				p->stageIncludePhase[i][0],
				p->stageIncludePhase[i][1],
				p->stageIncludePhase[i][2],
				p->stageIncludePhase[i][3]);
	}
}
//����������ʼ��
static void StepParamsInit(StepParams *p)
{
	int i;
	UInt8 phaseSplit[NUM_PHASE];	//��λ�����ű�
	
	memset(p, 0, sizeof(StepParams));
	pthread_rwlock_wrlock(&gCountDownLock);
	p->cycleTime = gCountDownParams->ucCurCycleTime;
	p->runTime = gCountDownParams->ucCurRunningTime;
	for (i = 0; i < NUM_PHASE; i++)
	{
		phaseSplit[i] = gCountDownParams->stPhaseRunningInfo[i][0];
		p->phaseStatus[i] = gCountDownParams->stVehPhaseCountingDown[i][0];
	}
	//gCountDownParams->ucPlanNo = SPECIAL_CONTROL_STEP;
	memcpy(gCountDownParams->ucCurPlanDsc, "����", sizeof(gCountDownParams->ucCurPlanDsc));
	outPutCount = 0;	//��յ���ʱ���ü���
	pthread_rwlock_unlock(&gCountDownLock);
	SetStepParams(p, phaseSplit);
	gCountDownParams->ucCurRunningTime = p->stageStartRunTime[p->curStageNo - 1];
//	SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_TURN_OFF);	//���͹صƿ�����Ϣ
	gStructBinfileCustom.cSpecialControlSchemeId = 249;
	FindNextPhaseIdInPhaseTrun(gPhaseTrunTable, p->nextPhaseIds);
}
//����Ҫ�����������Ƶơ�ȫ��Ĺ�����
static void StepTransition(StepParams *p)
{
	int i, j, n = 0;
	UInt8 phaseId = 0;
	UInt8 phaseIndexs[NUM_PHASE] = {0};	//���е���һ�׶�ʱ��λ״̬��ı����λ������
	PhaseItem *phaseItem = NULL;
	UInt8 greenBlinkTime, yellowTime, allRedTime;
	Boolean ret;
	
	if (p->curStageNo == p->nextStageNo)
		return;
	gTransitionFlag = 1;
	for (i = 0; i < NUM_RING_COUNT; i++)
	{
		phaseId = p->stageIncludePhase[p->curStageNo - 1][i];
		if (phaseId == 0)
			break;
		ret = FALSE;
		for (j = 0; j < NUM_RING_COUNT; j++)
		{	//�����ǰ�׶������е���λ����һ�׶���Ҳ������
			if (phaseId == p->stageIncludePhase[p->nextStageNo - 1][j])
			{
				ret = TRUE;
				break;
			}
		}
		if (ret == FALSE)
			phaseIndexs[n++] = phaseId - 1;
	}
	pthread_rwlock_rdlock(&gConfigLock);
	phaseItem = &gSignalControlpara->stPhase[phaseIndexs[0]];
	greenBlinkTime = gSignalControlpara->AscSignalTransTable[phaseIndexs[0]].nGreenLightTime;
	yellowTime = phaseItem->nYellowTime;
	allRedTime = phaseItem->nAllRedTime;
	pthread_rwlock_unlock(&gConfigLock);
	//������λ״̬����һ�׶�Ҫ�ı����λ���ξ����������Ƶơ�ȫ��Ĺ���
	for (i = 0; i < n; i++)
		p->phaseStatus[phaseIndexs[i]] = GREEN_BLINK;	//����Ҫ�ı����λ״̬����Ϊ����
	for (i = 0; i < greenBlinkTime * 2; i++)
	{	//��ΪҪ�̵���˸������ÿ��250ms����һ��ͨ��
		SetChannel(p);
		usleep(500000);
	}
	for (i = 0; i < n; i++)
		p->phaseStatus[phaseIndexs[i]] = YELLOW;	//����Ҫ�ı����λ״̬����Ϊ�Ƶ�
	SetChannel(p);
	sleep(yellowTime);	//����ʱ�ﵽ�Ƶ�����ʱ��
	for (i = 0; i < n; i++)
		p->phaseStatus[phaseIndexs[i]] = ALLRED;	//����Ҫ�ı����λ״̬����Ϊȫ��
	SetChannel(p);
	sleep(allRedTime);	//����ʱ�ﵽȫ������ʱ��
	SetPhaseStatus(p->nextStageNo, p);
	SetChannel(p);	//������ɺ�������һ�׶ε�ͨ��
	gTransitionFlag = 0;
}
#if 0
//��ת��ָ���׶�
static void udp_send_step(int iStep)
{
	//����UDP������
	int socketFd = -1;
	//�������һ��ֵΪ�׶κ�
	char JumpToiStep[46]={0x30,0x2C,0x02,0x01,0x00,0x04,0x06,0x70,
						0x75,0x62,0x6C,0x69,0x63,0xA3,0x1F,0x02,
						0x01,0x0D,0x02,0x01,0x00,0x02,0x01,0x00,
						0x30,0x14,0x30,0x12,0x06,0x0D,0x2B,0x06,
						0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
						0x50,0x02,0x00,0x02,0x01,0x00};
	struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	socklen_t localLen = sizeof(localAddr);
	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( -1 == socketFd )
	{
		printf("socket udp init error!!!\n");
		return;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//ʹ��161�˿�
	localAddr.sin_port = htons (161);

	// 1.�򱾻�161�˿ڷ��Ͳ���
	//�׶κŸ�ֵ
	JumpToiStep[45] = iStep;

	printf("��ת����iStep=%d\n",iStep);
	int len2 = sendto(socketFd,JumpToiStep,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len2 < 0)
	{
		printf("Send flashing signal failed!!!\n");
	}
	close(socketFd);
}
//ȡ������
static void udp_send_cancel_jump()
{
	//����UDP������
	int socketFd = -1;
	//ȡ������
	unsigned char cancelJumpStep[46]={0x30,0x2C,0x02,0x01,0x00,0x04,0x06,0x70,
							0x75,0x62,0x6C,0x69,0x63,0xA3,0x1F,0x02,
							0x01,0x0C,0x02,0x01,0x00,0x02,0x01,0x00,
							0x30,0x14,0x30,0x12,0x06,0x0D,0x2B,0x06,
							0x01,0x04,0x01,0x89,0x36,0x04,0x02,0x01,
							0x50,0x05,0x00,0x02,0x01,0x00};
    struct sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = sizeof(localAddr);
    socketFd = socket (AF_INET, SOCK_DGRAM, 0);
    if ( -1 == socketFd )
    {
		printf("socket udp init error!!!\n");
       	return;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	//ʹ��161�˿�
    localAddr.sin_port = htons (161);
	//�򱾻�161�˿ڷ���ȡ������
	printf("ȡ������\n");
	int len = sendto(socketFd,cancelJumpStep,46,0,(struct sockaddr *)&localAddr,localLen);
	if(len < 0)
	{
		printf("Send flashing signal failed!!!\n");
	}
	close(socketFd);
	
}
#endif
//�����̴߳�����
static void *StepProcess(void *arg)
{
	struct msgbuf msg;
	StepParams *p = &gStepParams;
	
	while (1)
	{
		msgrcv(msgid, &msg, MSGSIZE, STEP_MSG_TYPE, 0);
		if (msg.cmd == STEP_START)
		{
		    INFO("===> step  start ....\n");
			SetChannel(p);
			gStepFlag = 1;
		}
		else if (msg.cmd == STEP_EXCUTE)
		{
			if (msg.stageNo > 0)
				p->nextStageNo = msg.stageNo;
			gStepFlag = 1;
			INFO("===> step  excute  \n");
			StepTransition(p);
		}
		else if (msg.cmd == STEP_END)
		{
			if (gStepFlag == 0)
				continue;
            SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_TURN_OFF);	//���͹صƿ�����Ϣ				
			INFO("===> step  cancel  \n");
			p->nextStageNo = 1;	//��������ʱָ���ӽ׶�1��ʼ����
			StepTransition(p);
			gCountDownParams->ucCurRunningTime = gCountDownParams->ucCurCycleTime;
			outPutCount = 0x3f;//�ı�outPutCount��ֵ����Ҫ���������δ˴θ�Ӧ���Ʒ���ʱ���ڵ���ʱ�ӿ���չʾ��
			//gStructBinfileCustom.cSpecialControlSchemeId = SPECIAL_CONTROL_INDUCTION;       //���¸�Ӧ���Ʒ�����
			SendSpecialCtrolUdpMsg(SPECIAL_CONTROL_INDUCTION);	//���͸�Ӧ������Ϣ
			gStepFlag = 0;
			
			sleep(1);
			outPutCount = 0;
           // INFO("StepProcess --->   %d \n",gStructBinfileCustom.cSpecialControlSchemeId);
			//����ϴ�������������л���������ô����������Ӧ���л���������ƶ�����ϵͳ����
			if(gStructBinfileCustom.cSpecialControlSchemeId != SPECIAL_CONTROL_YELLOW_BLINK 
						&& gStructBinfileCustom.cSpecialControlSchemeId != SPECIAL_CONTROL_TURN_OFF
						&& gStructBinfileCustom.cSpecialControlSchemeId != SPECIAL_CONTROL_ALL_RED)
			{
    			gStructBinfileCustom.cSpecialControlSchemeId = SPECIAL_CONTROL_SYSTEM;       //����ϵͳ���Ʒ�����
			}
			
			SendSpecialCtrolUdpMsg(gStructBinfileCustom.cSpecialControlSchemeId);	//����ϵͳ������Ϣ
		}
	}
	pthread_exit(NULL);
}

void StepPthreadInit(void)
{
	key_t key = ftok("/dev/null", 'a');
	pthread_t id;
	
	if (key == -1) 
	{
		ERR("ftok fail when create message queue");
		exit(1);
	}
	while (msgid == -1)
	{
		msgid = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
		if (msgid == -1)
		{	
			if (errno == EEXIST)
			{	//�����Ѿ����ڴ���Ϣ���У�����ɾ���˶����ٴ����Դ��������Ϣ����	
				msgid = msgget(key, 0666);
				msgctl(msgid, IPC_RMID, 0);
				msgid = -1;
			}
			else
			{
				ERR("create message queue fail!");
				exit(1);
			}
		}
	}
	INFO("create message queue successful! msgid = %d", msgid);
	
	if (pthread_create(&id, NULL, StepProcess, NULL) != 0)
	{
		ERR("step pthread create fail");
		exit(1);
	}
	pthread_detach(id);
	INFO("create step pthread successful!");
}

//��鲽���Ƿ���Ч�������������Ƶơ�ȫ��ʱ�ǲ��ܽ��в�����
Boolean IsStepValid(UInt8 stageNo, UInt8 flag)
{
	UInt8 status;
	Boolean ret = TRUE;
	struct msgbuf msg;
	int i;

	if (gTransitionFlag == 1) //��������ʱ�ٴβ�����Ч
		return FALSE;
		
	pthread_rwlock_rdlock(&gCountDownLock);//�����������ƵĻ���������ж��ǲ�����Ч�ģ��������϶�������Ʒ�ʽ���ж�
	for (i = 0; i < NUM_PHASE; i++)
	{
		status = gCountDownParams->stVehPhaseCountingDown[i][0];
		if (status == GREEN_BLINK || status == YELLOW 
			|| (status == RED && gCountDownParams->stPhaseRunningInfo[i][1] != 0))
		{
			ret = FALSE;
			break;
		}	
	}
	pthread_rwlock_unlock(&gCountDownLock);

    if(gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_YELLOW_BLINK 
						|| gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_TURN_OFF
						|| gStructBinfileCustom.cSpecialControlSchemeId == SPECIAL_CONTROL_ALL_RED)
    {
        ret = FALSE;
    }
	
	if (ret == TRUE)
	{
		msg.mtype = STEP_MSG_TYPE;
		msg.stageNo = stageNo;
		if (flag == 0)
			StepParamsInit(&gStepParams);
		if (stageNo == 0)
		{	//��������
			msg.cmd = (flag == 0) ? STEP_START : STEP_EXCUTE;
		}
		else
		{	//ֱ�Ӳ�������Ӧ�׶�		
			if (stageNo > gStepParams.maxStageNo)
			{
				ERR("No such stage number %d", stageNo);
				ret = FALSE;
			}
			else if (stageNo == gStepParams.curStageNo)
			{
				msg.cmd = STEP_START;
			}
			else
				msg.cmd = STEP_EXCUTE;
		}
		if (ret == TRUE)	
			msgsnd(msgid, &msg, MSGSIZE, 0);
	}
	return ret;
}
//ȡ������
void StepCancel(void)
{
	struct msgbuf msg = {STEP_MSG_TYPE, STEP_END, 0};
	msgsnd(msgid, &msg, MSGSIZE, 0);
	//memset(gStepParams, 0, sizeof(StepParams));
}
