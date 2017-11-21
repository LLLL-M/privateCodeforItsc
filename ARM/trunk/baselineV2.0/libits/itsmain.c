
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>

#include "its.h"
#include "lfq.h"
#include "hikmsg.h"
#include "LogSystem.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define PTHREAD_STACK_SIZE	(2 << 20)	//2M
#define LINE_QUEUE_SIZE		(768 * 1024)	//768k

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
 
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void *FaultLogModule(void *arg);
extern void *PhaseDriverModule(void *arg);
extern void *CalculateModule(void *arg);
extern void *VehicleCheckModule(void *arg);
extern void *PhaseControlModule(void *arg);
extern void *StrategyControlModule(void *arg);
extern void *TimerModule(void *arg);
extern void *RedSignalCheckModule(void *arg);
extern void *ChannelHandleModule(void *arg);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
//��Ϣ����id
int msgid = -1;
static void *gConfig = NULL;	//�źŻ����ò������ṹȫ�ֱ���ָ�룬�������������Ϣ
static int gConfigSize = 0;	//�źŻ����õĴ�С
//��дȫ��������Ϣ����
pthread_rwlock_t gConfigLock = PTHREAD_RWLOCK_INITIALIZER;
//���Զ��о��
void *gHandle = NULL;
//��ʱģ�鷢�͸���λ���ƺ���λ����ģ��������ź���
sem_t gSemForCtl;	//��������λ����ģ�鷢�Ͷ�ʱ1s���ź�
sem_t gSemForDrv;	//��������λ����ģ�鷢�Ͷ�ʱ250ms���ź�
sem_t gSemForVeh;	//�����ڴ����ڼ�⳵������������
sem_t gSemForChan;	//������ͨ������ģ�鷢�Ͷ�ʱ1s���ź�


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : getMsgId
 ��������  : ����һ����Ϣ���в�����msgid������Ѿ�������ֱ�ӷ����Ѿ�������msgid
 �������  : int proj_id  ������һ�����һ���ַ�
 �� �� ֵ  : ���ش����õ���Ϣ���е�msgid
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static int GetMsgId(int proj_id)
{
	key_t key = ftok("/dev/null", proj_id);
	int id = -1;
	
	if (key == -1) 
	{
		return -1;
	}
	while (id == -1)
	{
		id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);
		if (id == -1)
		{	
			if (errno == EEXIST)
			{	//�����Ѿ����ڴ���Ϣ���У�����ɾ���˶����ٴ����Դ��������Ϣ����	
				id = msgget(key, 0666);
				INFO("excute msgctl to delete msg queue before, ret = %d", msgctl(id, IPC_RMID, 0));
				id = -1;
			}
			else
				return -1;
		}
	}
	return id;
}

void *ItsAllocConfigMem(void *config, int configSize)
{
	return NULL;
}

static void InitGolobalVar(void *config, int configSize)
{
	InitLogSystem("/home/Log/",3,1);
	msgid = GetMsgId('a');
	if (msgid == -1) 
	{
		log_error("get msgid fail, error info: %s\n", strerror(errno));
		exit(1);
	}
	if (sem_init(&gSemForCtl, 0, 0) == -1
		|| sem_init(&gSemForDrv, 0, 0) == -1
		|| sem_init(&gSemForVeh, 0, 0) == -1
		|| sem_init(&gSemForChan, 0, 0) == -1)
	{
		log_error("int sem fail, error info: %s\n", strerror(errno));
		exit(1);
	}
	
    //��ʼ���ڴ�����ȫ�ֵ��źŻ����ò���
	gConfig = ItsAllocConfigMem(config, configSize);
	if(gConfig == NULL)
	{
		log_error("error to alloc memory for gConfig  : %s\n",strerror(errno));
		exit(1);
	}
	gConfigSize = configSize;
    
	//Ϊ���Զ��������ڴ棬����ʼ�����Զ���
	gHandle = calloc(1, LINE_QUEUE_SIZE);
	if (NULL == gHandle)
	{
		log_error("init line queue fail because of no enough memory");
		exit(1);
	}
	lfq_init(gHandle, LINE_QUEUE_SIZE, sizeof(LineQueueData));
	INFO("init libits global variable successful! msgid = %d, blocknum = %u\n", msgid, LINE_QUEUE_SIZE / sizeof(LineQueueData));
}

/*****************************************************************************
 �� �� ��  : TimerModule
 ��������  : ��ʱ��ģ����̴߳���������Ҫ�������ղ��Ե���ģ��Ķ�ʱ��Ϣ��
             Ȼ��ʱ1s������ٸ�����ģ�鷢�Ͷ�ʱ��ɵ���Ϣ
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *TimerModule(void *arg)
{
	UInt32 *countvalue = (UInt32 *)arg;
	struct msgbuf msg;
	struct timespec start = {0, 0}, end = {0, 0};
	long gap, times = 0;

	msgrcv(msgid, &msg, MSGSIZE, MSG_START_TIMER, 0);	//�����ȴ���λ����ģ�鷢�͵�������ʱ����Ϣ
	INFO("Timer begin to run!");
	//�ȷ���һ�ζ�ʱ�źŸ���λ����ģ��
	sem_post(&gSemForCtl);
	clock_gettime(CLOCK_MONOTONIC, &start);	//��¼����ʼʱ��
	while (1)	//ÿDELAY_TIME msѭ��һ��
	{
		usleep(DELAY_TIME_USECS_BEFORE);	//����ʱDELAY_TIME_USECS_BEFORE=200ms,������50ms�ھ�ȷ��ʱ
		while (1)
		{
			usleep(10000);	//ÿ����ʱ10ms
			clock_gettime(CLOCK_MONOTONIC, &end);
			gap = (end.tv_sec - start.tv_sec) * ONE_SECOND_NSECS + end.tv_nsec - start.tv_nsec;
			if (gap >= DELAY_TIME_NSECS)
			{	      
				if (++times % LOOP_TIMES_PER_SECOND == 0)
				{	//����λ����ģ�鷢�Ͷ�ʱ�ź�,ÿ1sһ��
					sem_post(&gSemForCtl);
					times = 0;
				}
                //����λ����ģ�鷢�Ͷ�ʱ�ź�,ÿ250msһ��
				sem_post(&gSemForDrv);
				start.tv_nsec += DELAY_TIME_NSECS;
				if (start.tv_nsec >= ONE_SECOND_NSECS)
				{
					start.tv_nsec -= ONE_SECOND_NSECS;
					start.tv_sec += 1;
				}
				(*countvalue)++;
				break;
			}
		}
	}
	pthread_exit(NULL);
}

/*****************************************************************************
 �� �� ��  : CreateThread
 ��������  : �̴߳�����Ҫ���õĺ�������Ҫ��������һ���߳�
 �������  : struct threadInfo *info	����߳��й���Ϣ��ָ��
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void CreateThread(struct threadInfo *info)
{
	if (info->func == NULL)
		return;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE);
	if (pthread_create(&info->id, &attr, info->func, (void *)&info->countvalue) == -1) 
	{
		log_error("create %s module fail, error info:%s\n", info->moduleName, strerror(errno));
		exit(1);
	}
	pthread_detach(info->id);
	pthread_attr_destroy(&attr);
	INFO("create %s pthread successful!\n", info->moduleName);
}

/*****************************************************************************
 �� �� ��  : itstaskmain
 ��������  : ��ģ��ĳ�ʼ��������ڣ���Ҫ������ʼ������ģ����߳�
 �������  : int argc      
             char *argv[]  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
#define MAX_THREAD_NUM	16
static struct threadInfo gThreads[MAX_THREAD_NUM] = {
	{0, FaultLogModule, "fault log", 0},
	{0, TimerModule, "timer", 0},
	{0, RedSignalCheckModule, "red light signal check", 0},
	{0, PhaseDriverModule, "phase driver", 0},
	{0, ChannelHandleModule, "ChannelHandle", 0}, 	
	{0, VehicleCheckModule, "vehicle check", 0},
	{0, PhaseControlModule, "phase control", 0},
	{0, StrategyControlModule, "strategy control", 0},
	{0, CalculateModule, "calculate", 0},
};
static int gThreadNum = 9;	//�̸߳�����Ҫ������ĸ���һһ��Ӧ
Boolean ItsInit(struct threadInfo *threads, int num, void *config, int configSize)
{
	int i;

	if (num + gThreadNum > MAX_THREAD_NUM || configSize < 0)
	{
		ERR("regsiter thread num[%d] exceed the limit[%d] or configSize[%d] is invalid!", num, MAX_THREAD_NUM - gThreadNum, configSize);
		return FALSE;
	}
	if (threads == NULL || config == NULL)
	{
		ERR("the argument threads or config is NULL");
		return FALSE;
	}
	fclose(stdout);
	InitGolobalVar(config, configSize);

	if (threads != NULL && num > 0)
	{
		memcpy(&gThreads[gThreadNum], threads, sizeof(struct threadInfo) * num);
		gThreadNum += num;
	}
	for (i = 0; i < gThreadNum; i++)
		CreateThread(gThreads + i);
	log_debug("all threads init successful! ");
	return TRUE;
}

void ItsExit(void)
{
	int i;
	for (i = 0; i < gThreadNum; i++)
	{
		if (gThreads[i].id != 0)
			pthread_cancel(gThreads[i].id);
	}
	if (msgid != -1)
	{
		msgctl(msgid, IPC_RMID, NULL);
		msgid = -1;
	}
	sem_destroy(&gSemForCtl);
	sem_destroy(&gSemForDrv);
	sem_destroy(&gSemForVeh);
	sem_destroy(&gSemForChan);
	if (gConfig != NULL)
	{
		free(gConfig);
		gConfig = NULL;
	}
	if (gHandle != NULL)
	{
		free(gHandle);
		gHandle = NULL;
	}
	INFO("The Its resource is free!");
}

#define IS_CORE_MODULE(F) ((F) == TimerModule \
				|| (F) == PhaseDriverModule \
				|| (F) == PhaseControlModule \
				|| (F) == StrategyControlModule \
				|| (F) == CalculateModule)
void ItsThreadCheck(void)
{
	static UInt32 countvalues[MAX_THREAD_NUM] = {0};
	struct msqid_ds msgstat;
	struct msgbuf msg;
	int i, m; 
	int semvalForDrv = -1, semvalForCtl = -1, semvalForChan = -1;

	memset(&msgstat, 0, sizeof(msgstat));
	for (i = 0; i < gThreadNum; i++)
	{
		if (pthread_kill(gThreads[i].id, 0) == ESRCH) 
		{
			ItsWriteFaultLog(THREAD_EXCEPTION_EXIT, i);
			if (IS_CORE_MODULE(gThreads[i].func))
			{	//����Ǽ�������ģ��ҵ����Զ�����ϵͳ
				log_error("%s module has already stop, reboot system automatically!", gThreads[i].moduleName);
				sync();
				#if defined(__linux__) && defined(__arm__)
				system("reboot");
				#endif
			}
			else
			{
				log_error("%s module has already stop, recreate it!", gThreads[i].moduleName);
				CreateThread(gThreads + i);
			}
		}
		if (IS_CORE_MODULE(gThreads[i].func))
		{
			if (countvalues[i] != gThreads[i].countvalue)
				countvalues[i] = gThreads[i].countvalue;
			else
			{
				if (gThreads[i].countvalue == 0 && countvalues[i] == 0)
					continue;	//˵��ģ��û��ʹ�ü���ֵ,��ô�㲻���κδ���
				/*���ģ��ļ���ֵ��ʹ�õ���ģ��ļ���ֵ��˺����ļ���ֵ�����˵��ģ�鱻������,
				 * Ŀǰֻ�ж�ʱģ�顢��λ����ģ�顢��λ����ģ���õ��˼���ֵcountvalue,
				 * ����������ģ������ʱ�䲻�ᳬ��1s,����������main����ÿ��3s���һ�ζ��߼���,
				 * �������˵��ģ���Ѿ����ֹ�����,Ĭ���������ϵͳ����һֱ����*/
				msgctl(msgid, IPC_STAT, &msgstat);
				sem_getvalue(&gSemForDrv, &semvalForDrv);
				sem_getvalue(&gSemForCtl, &semvalForCtl);
				sem_getvalue(&gSemForChan, &semvalForChan);
				log_error("checked %s module running unnormally, reboot system automatically!\n"\
					"msgnum:%lu, semvalForCtl:%d, semvalForChan:%d, semvalForDrv:%d", 
					gThreads[i].moduleName, msgstat.msg_qnum, semvalForCtl, semvalForChan, semvalForDrv);
				for (m = 0; m < msgstat.msg_qnum; m++)
				{			
					memset(&msg, 0, sizeof(msg));
					if (-1 != msgrcv(msgid, &msg, MSGSIZE, 0, IPC_NOWAIT))
						log_error("left msg type = %d ", msg.mtype);
				}
				sync();
				#if defined(__linux__) && defined(__arm__)
				system("reboot");
				#endif
			}
		}
	}
}

void ItsSetConfig(void *config, int configSize)
{
	if (config == NULL || gConfig == NULL || gConfigSize == 0 || configSize != gConfigSize)
		return;
	pthread_rwlock_wrlock(&gConfigLock);
	memcpy(gConfig, config, configSize);
	pthread_rwlock_unlock(&gConfigLock);
}

void ItsGetConfig(void *config, int configSize)
{
	if (config == NULL || gConfig == NULL || gConfigSize == 0 || configSize != gConfigSize)
		return;
	pthread_rwlock_rdlock(&gConfigLock);
	memcpy(config, gConfig, configSize);
	pthread_rwlock_unlock(&gConfigLock);
}

static void SendControlMsg(ControlType type, UInt8 schemeId, int val, Boolean isblock)
{
    struct msgbuf msg;

    memset(&msg, 0, sizeof(msg));
    msg.mtype = MSG_CONTROL_TYPE;
    msg.msgControlType = type;
	switch (schemeId)
	{
		case YELLOWBLINK_SCHEMEID: msg.msgMode = YELLOWBLINK_MODE; msg.msgmSchemeId = schemeId; break;
		case ALLRED_SCHEMEID: msg.msgMode = ALLRED_MODE; msg.msgmSchemeId = schemeId; break;
		case TURNOFF_SCHEMEID: msg.msgMode = TURNOFF_LIGHTS_MODE; msg.msgmSchemeId = schemeId; break;
		case INDUCTIVE_SCHEMEID: msg.msgMode = INDUCTIVE_MODE; msg.msgmSchemeId = val; break;
		case INDUCTIVE_COORDINATE_SCHEMEID: msg.msgMode = INDUCTIVE_COORDINATE_MODE; msg.msgmSchemeId = val; break;//��ӦЭ������
		case STEP_SCHEMEID: msg.msgMode = STEP_MODE; msg.msgStageNum = val; break;
		case SYSTEM_RECOVER_SCHEMEID: msg.msgMode = SYSTEM_MODE; break;
        case SINGLE_ADAPT_SCHEMEID: msg.msgMode = SINGLE_ADAPT_MODE; msg.msgmSchemeId = val; break;
		default: msg.msgMode = (type == AUTO_CONTROL) ? SYSTEM_MODE : MANUAL_MODE; msg.msgmSchemeId = schemeId; break;
	}
	if (msgid == -1)
	{
		msgid = msgget(ftok("/dev/null", 'a'), 0666);
		INFO("create msgid %d", msgid);
	}
	//INFO("func[%s] recv ITS control, mode:%d schemeId:%d, val:%d", __func__, msg.msgMode, schemeId, val);
    if (-1 == msgsnd(msgid, &msg, MSGSIZE, isblock ? 0 : IPC_NOWAIT))
		log_error("ItsCtl msgsnd error: %s, schemeId = %d", strerror(errno), schemeId);
}

void ItsCtl(ControlType type, UInt8 schemeId, int val)
{
	SendControlMsg(type, schemeId, val, TRUE);
}
void ItsCtlNonblock(ControlType type, UInt8 schemeId, int val)
{
	SendControlMsg(type, schemeId, val, FALSE);
}

void ItsChannelCheck(UInt8 channelId, LightStatus status)
{
	struct msgbuf msg;
	int i = 0;
	
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_CHANNEL_CHECK;
	msg.msgChannelId = channelId;
	msg.msgChannelStatus = status;
	for (i = 0; i < 6; i++)	//ͨ�����ʱ������ֻ�������ͨ��6s
		msgsnd(msgid, &msg, MSGSIZE, 0);
}

