#include <unistd.h>
#include <semaphore.h>
#include "hikmsg.h"
#include "its.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define LOFF		0
#define LGREEN		1
#define LYELLOW		4
#define LRED		2
#define LREDYELLOW	6

extern int msgid;
extern sem_t gSemForDrv;

void ItsFaultCheck(UInt16 *lightValues)
{
}
void ItsLight(int boardNum, unsigned short *poutLamp)
{
}
/*****************************************************************************
 �� �� ��  : put_lamp_value
 ��������  : ��Ҫ��������һ�����ĳ���Ƶ�״ֵ̬
 �������  : UInt16 *lights  ����һ���״̬��ָ��
             int n           �������ĸ��ƣ�ֻ����0��1��2��3
             UInt16 value    Ҫ���õĵƵ�״ֵ̬
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void put_lamp_value(UInt16 *lights, int n, UInt16 value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}

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
static void Light(UInt8 *allChannel, UInt16 *lightValues, UInt8 times)
{
	int i;
	UInt16 value;
	UInt16 *nOutLamp = lightValues;
	UInt8 half = LOOP_TIMES_PER_SECOND / 2;

	for (i = 0; i < MAX_CHANNEL_NUM; i++) 
	{
		//����ͨ������״̬�ҳ����ͨ��Ӧ��������ֵ
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK: value = (times > half) ? LGREEN : LOFF; break;	
			case YELLOW: value = LYELLOW; break;
			case YELLOW_BLINK: value = (times > half) ? LYELLOW : LOFF; break;
			case RED: value = LRED; break;
			case RED_BLINK: value = (times > half) ? LRED : LOFF; break;
			case RED_YELLOW: value = LREDYELLOW; break;
			case ALLRED: value = LRED; break;
			case OFF_GREEN: value = (times == 2) ? LOFF : LGREEN; break;
			case OFF_YELLOW: value = (times == 2) ? LOFF : LYELLOW; break;
			case OFF_RED: value = (times == 2) ? LOFF : LRED;  break;
			//case INVALID: value = LRED; break;	//Ĭ��û�����õ�ͨ��ȫ��Ϊ���
			case INVALID: value = LOFF; break;	//Ĭ��û�����õ�ͨ��ȫ��Ϊ���
			default: value = LOFF; break;
		}
		//�����ͨ����ֵ
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			//PhaseLampOutput((i + 1) / 4, *nOutLamp);
			nOutLamp++;
		}
	}
	ItsLight(MAX_BOARD_NUM, lightValues);
	//���ذ�����ָʾ��
	usleep(50000); //��ʱ50ms�����ܹ��Ѹղŵ�Ƶ�ʵ����Ϣ�����������Լ��
	ItsFaultCheck(lightValues);
}

/*****************************************************************************
 �� �� ��  : PhaseDriverModule
 ��������  : ���ģ����̴߳���������Ҫ���յ����Ϣ��Ȼ����Ϣ���ݽ��е��
 �������  : void *arg  
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��2��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void *PhaseDriverModule(void *arg)
{
	UInt32 *countvalue = (UInt32 *)arg;
	struct msgbuf msg, timer;
	UInt16 lightValues[MAX_BOARD_NUM] = {0};	//��ŵ�����ݵ�����
	UInt8 times = 0;

	memset(&msg, 0, sizeof(msg));
	while (1)
	{
		if (times % LOOP_TIMES_PER_SECOND == 0)
		{
			msgrcv(msgid, &msg, MSGSIZE, MSG_CHANNEL_STATUS, 0);	//�������յ����Ϣ
			times = 0;
		}
		times++;
		Light(msg.msgAllChannels, lightValues, times);
		sem_wait(&gSemForDrv);
		(*countvalue)++;
	}
}
