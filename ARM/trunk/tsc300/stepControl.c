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

#define MAX_STEP_NUM	16
#define STEP_MSG_TYPE	100
#define MSGSIZE (sizeof(struct msgbuf) - sizeof(long))

typedef enum
{
	STEP_START = 0,	//������ʼ
	STEP_EXCUTE,	//����ִ��
	STEP_END,		//��������
} StepCmd;
struct msgbuf
{
	long type;
	StepCmd cmd;
	UInt8 stageNo;
};
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
	UInt8 stageNeedRunTimes[MAX_STEP_NUM];	//������ʱ����ÿ���׶���Ҫ����ʱ��
	UInt8 phaseSplit[NUM_PHASE];	//��λ�����ű�
	UInt8 phaseStatus[NUM_PHASE];	//��λ״̬
} StepParams;

static StepParams gStepParams;

static int msgid = -1;
UInt8 gStepFlag = 0;	//������־��0����ʾδ������1����ʾ����
UInt16 gLightArray[8] = {0};	//����ʱʹ�õĵ������
pthread_rwlock_t gLightArrayLock = PTHREAD_RWLOCK_INITIALIZER;	//��������������

extern SignalControllerPara *gSignalControlpara;
extern pthread_rwlock_t gConfigLock;
extern pthread_rwlock_t gCountDownLock;
extern PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *gCountDownParams;
extern PhaseTurnItem gPhaseTrunTable[NUM_RING_COUNT]; //��ǰʹ�õ�����

extern void udp_send_change_ctrl();

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
static void SetChannel(UInt8 *phaseStatus)
{    
    int i = 0, j = 0;
    UInt8 channelId = 0;
	PhaseChannelStatus status;
	UInt8 phaseId, motherPhase;
	UInt8 allChannel[NUM_CHANNEL];

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
			case PEDESTRIAN: status = phaseStatus[phaseId - 1]; break;
			case FOLLOW: 
				for (j = 0; j < NUM_PHASE; j++)
				{
					motherPhase = gSignalControlpara->stFollowPhase[phaseId - 1].nArrayMotherPhase[j];
					if (motherPhase == 0) 
						break;
					if (phaseStatus[motherPhase - 1] == GREEN)
					{
						status = GREEN;
						break;
					}
				}
				break;
			default: status = TURN_OFF; break;
		}
		allChannel[i] = (UInt8)status;
    }
	pthread_rwlock_unlock(&gConfigLock);
	SetLightArray(allChannel);
}

//ͨ������ָ���׶���Ҫ���е�ʱ��������ָ���׶ε���λ״̬
static void SetPhaseStatus(UInt8 stageNo, StepParams *p)
{
	int i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt8 phaseEndRunTime = 0;	//��λִ�����Ѿ����е�����ʱ��
	UInt8 stageNeedRunTime = p->stageNeedRunTimes[stageNo - 1];
	
	p->curStageNo = stageNo;
	p->nextStageNo = (stageNo + 1 > p->maxStageNo) ? 1 : (stageNo + 1);
	memset(p->phaseStatus, 0, NUM_PHASE);
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
			phaseEndRunTime += p->phaseSplit[nPhaseId - 1];
			if (phaseEndRunTime > stageNeedRunTime)
			{
				p->phaseStatus[nPhaseId - 1] = (UInt8)GREEN;
				break;
			}
		}
	}
}

//���ö�����ʱ����ÿ���׶���Ҫ���е�ʱ�䣬�����ݵ�ǰ����ʱ��������ҳ���һ�����еĽ׶κ�
static void SetStepParams(StepParams *p)
{
	int t = 0, i = 0, ring = 0;
    UInt8 nPhaseId = 0;
	UInt8 phaseEndRunTime = 0;	//��λִ�����Ѿ����е�����ʱ��
	UInt8 stageNo = 1;	//�׶κ�
	UInt8 nStageNeedRunTime = 0;	//����ָ���׶���Ҫ���е�ʱ��
	UInt8 curStageNo = 1;
	
	memset(p->stageNeedRunTimes, 0, MAX_STEP_NUM);
	for (t = 0; t < p->cycleTime; t++) 
	{	
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
				phaseEndRunTime += p->phaseSplit[nPhaseId - 1];
				if (t == phaseEndRunTime && phaseEndRunTime > nStageNeedRunTime)
				{
					stageNo++;
					nStageNeedRunTime = phaseEndRunTime;
					p->stageNeedRunTimes[stageNo - 1] = nStageNeedRunTime;
				}
			}
		}
		if (t + 1 == p->runTime)
			curStageNo = stageNo;
	}
	p->curStageNo = curStageNo;
	p->nextStageNo = (curStageNo + 1 > stageNo) ? 1 : (curStageNo + 1);
	p->maxStageNo = stageNo;
}
//����������ʼ��
static void StepParamsInit(StepParams *p)
{
	int i;
	memset(p, 0, sizeof(StepParams));
	pthread_rwlock_rdlock(&gCountDownLock);
	p->cycleTime = gCountDownParams->ucCurCycleTime;
	p->runTime = gCountDownParams->ucCurRunningTime;
	for (i = 0; i < NUM_PHASE; i++)
	{
		p->phaseSplit[i] = gCountDownParams->stPhaseRunningInfo[i][0];
		p->phaseStatus[i] = gCountDownParams->stVehPhaseCountingDown[i][0];
	}
	pthread_rwlock_unlock(&gCountDownLock);
	SetStepParams(p);
}
//��������Ҫ�����������Ƶơ�ȫ��Ĺ�����
static void OneStepTransition(StepParams *p)
{
	int i, n = 0;
	UInt8 curPhaseStatus[NUM_PHASE];	//���Ա��浱ǰ�׶ε���λ״̬
	UInt8 phaseIndexs[NUM_PHASE] = {0};	//���е���һ�׶�ʱ��λ״̬��ı����λ������
	PhaseItem *phaseItem = NULL;
	UInt8 greenBlinkTime, yellowTime, allRedTime;
	
	memcpy(curPhaseStatus, p->phaseStatus, sizeof(p->phaseStatus));
	SetPhaseStatus(p->nextStageNo, p);
	for (i = 0; i < NUM_PHASE; i++)
	{
		if (curPhaseStatus[i] != p->phaseStatus[i])
			phaseIndexs[n++] = i;
	}
	pthread_rwlock_rdlock(&gConfigLock);
	phaseItem = &gSignalControlpara->stPhase[phaseIndexs[0]];
	greenBlinkTime = gSignalControlpara->AscSignalTransTable[phaseIndexs[0]].nGreenLightTime;
	yellowTime = phaseItem->nYellowTime;
	allRedTime = phaseItem->nAllRedTime;
	pthread_rwlock_unlock(&gConfigLock);
	//������λ״̬����һ�׶�Ҫ�ı����λ���ξ����������Ƶơ�ȫ��Ĺ���
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = GREEN_BLINK;	//����Ҫ�ı����λ״̬����Ϊ����
	for (i = 0; i < greenBlinkTime * 4; i++)
	{	//��ΪҪ�̵���˸������ÿ��250ms����һ��ͨ��
		SetChannel(curPhaseStatus);
		usleep(250000);
	}
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = YELLOW;	//����Ҫ�ı����λ״̬����Ϊ�Ƶ�
	SetChannel(curPhaseStatus);
	sleep(yellowTime);	//����ʱ�ﵽ�Ƶ�����ʱ��
	for (i = 0; i < n; i++)
		curPhaseStatus[phaseIndexs[i]] = ALLRED;	//����Ҫ�ı����λ״̬����Ϊȫ��
	SetChannel(curPhaseStatus);
	sleep(allRedTime);	//����ʱ�ﵽȫ������ʱ��
	SetChannel(p->phaseStatus);	//������ɺ�������һ�׶ε�ͨ��
}
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
			SetChannel(p->phaseStatus);
			gStepFlag = 1;
		}
		else if (msg.cmd == STEP_EXCUTE)
		{
			if (msg.stageNo == 0)
			{	//��������
				OneStepTransition(p);
			}
			else
			{	
				SetPhaseStatus(msg.stageNo, p);
				SetChannel(p->phaseStatus);
				gStepFlag = 1;
			}
		}
		else if (msg.cmd == STEP_END)
		{
			udp_send_change_ctrl();
			usleep(50000);
			udp_send_step(p->curStageNo);
			usleep(50000);
			udp_send_cancel_jump();
			gStepFlag = 0;
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
	
	pthread_rwlock_rdlock(&gCountDownLock);
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
	if (ret == TRUE)
	{
		msg.type = STEP_MSG_TYPE;
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
