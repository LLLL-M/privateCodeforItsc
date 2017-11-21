#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include "hikmsg.h"

extern sem_t gSemForChan;	//������ͨ������ģ�鷢�Ͷ�ʱ1s���ź�
extern int msgid;

void SetRealTimeInfo(const unsigned char lockflag, const LineQueueData *data)
{
}
void ItsCountDownOutput(LineQueueData *data)
{
}
unsigned char ChannelControl(unsigned char *chan)
{
	return 0;
}
void channelLockTransition(unsigned char lockFlag, unsigned char *curStatus, unsigned char *lockstatus)
{

}
unsigned char GetBootTransitionTime(void)
{
	return 0;
}
static inline void SendChannelStatus(LineQueueData *data)
{
	struct msgbuf msg;
	int ret = msgrcv(msgid, &msg, MSGSIZE, MSG_CHANNEL_CHECK, IPC_NOWAIT);	//����������ͨ�������Ϣ
	int i;

	if (ret > 0)
	{	//������յ�ͨ�������Ϣ����ʹ�ý��յ�ͨ��״̬���
		for (i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			if (data->allChannels[i] != INVALID)
				data->allChannels[i] = (i == msg.msgChannelId - 1) ? msg.msgChannelStatus : TURN_OFF;
		}
	}
	memset(&msg, 0, sizeof(msg));
	memcpy(msg.msgAllChannels, data->allChannels, sizeof(data->allChannels));
	msg.mtype = MSG_CHANNEL_STATUS;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}
UNUSEDATTR static void SendChanStatus2RedCheck(unsigned char *chan)
{
	struct msgbuf msg;

	memset(&msg, 0, sizeof(msg));
	//����ͨ��״̬������źż����
	if(chan == NULL)
		return;
	msg.mtype = MSG_RED_SIGNAL_CHECK;
	memcpy(msg.msgAllChannels, chan, MAX_CHANNEL_NUM);
	msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT);
}
void *ChannelHandleModule(void *arg)
{
	LineQueueData sysdata;
	unsigned char lockstatus[32]={INVALID};
	unsigned char lockFlag = 0;
	unsigned char bootTransitonTime=GetBootTransitionTime();

	while (1)
	{
		if(bootTransitonTime == 0)
		{
			memset(lockstatus, INVALID, sizeof(lockstatus));
			lockFlag = ChannelControl(lockstatus);
		}
		if(!sem_trywait(&gSemForChan))//1s
		{
			if(bootTransitonTime>0)
				bootTransitonTime--;
			memset(&sysdata, 0, sizeof(LineQueueData));
			ItsGetCurRunData(&sysdata);

#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
			if (sysdata.isStep == FALSE)
				ItsCountDownOutput(&sysdata);	//�������ʱ��
#endif
			channelLockTransition(lockFlag, sysdata.allChannels, lockstatus);			
			SetRealTimeInfo(lockFlag, &sysdata);
	
			//INFO("ChanControl: Send channel status...0:%d, 1:%d, time: %ld", sysdata.allChannels[0], sysdata.allChannels[1], time(NULL));
			SendChannelStatus(&sysdata);
#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
			SendChanStatus2RedCheck(sysdata.allChannels);
#endif
		}
		usleep(50000);//50ms
	}
}
