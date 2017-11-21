#include <unistd.h>
#include <semaphore.h>
#include "hikmsg.h"
#include "its.h"

/*----------------------------------------------*
 * 宏定义                                       *
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
 函 数 名  : put_lamp_value
 功能描述  : 主要用来设置一组灯中某个灯的状态值
 输入参数  : UInt16 *lights  描述一组灯状态的指针
             int n           具体是哪个灯，只能是0、1、2、3
             UInt16 value    要设置的灯的状态值
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

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
 函 数 名  : Light
 功能描述  : 根据所有通道的状态来设置点灯的数组
 输入参数  : UInt8 *allChannel  		描述所有通道状态的数组指针
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static void Light(UInt8 *allChannel, UInt16 *lightValues, UInt8 times)
{
	int i;
	UInt16 value;
	UInt16 *nOutLamp = lightValues;
	UInt8 half = LOOP_TIMES_PER_SECOND / 2;

	for (i = 0; i < MAX_CHANNEL_NUM; i++) 
	{
		//根据通道所处状态找出这个通道应该所赋的值
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
			//case INVALID: value = LRED; break;	//默认没有配置的通道全部为红灯
			case INVALID: value = LOFF; break;	//默认没有配置的通道全部为灭灯
			default: value = LOFF; break;
		}
		//给这个通道赋值
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			//PhaseLampOutput((i + 1) / 4, *nOutLamp);
			nOutLamp++;
		}
	}
	ItsLight(MAX_BOARD_NUM, lightValues);
	//主控板运行指示灯
	usleep(50000); //延时50ms好让能够把刚才点灯的实际信息反馈过来用以检测
	ItsFaultCheck(lightValues);
}

/*****************************************************************************
 函 数 名  : PhaseDriverModule
 功能描述  : 点灯模块的线程处理函数，主要接收点灯消息，然后消息内容进行点灯
 输入参数  : void *arg  
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月2日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void *PhaseDriverModule(void *arg)
{
	UInt32 *countvalue = (UInt32 *)arg;
	struct msgbuf msg, timer;
	UInt16 lightValues[MAX_BOARD_NUM] = {0};	//存放点灯数据的数组
	UInt8 times = 0;

	memset(&msg, 0, sizeof(msg));
	while (1)
	{
		if (times % LOOP_TIMES_PER_SECOND == 0)
		{
			msgrcv(msgid, &msg, MSGSIZE, MSG_CHANNEL_STATUS, 0);	//阻塞接收点灯消息
			times = 0;
		}
		times++;
		Light(msg.msgAllChannels, lightValues, times);
		sem_wait(&gSemForDrv);
		(*countvalue)++;
	}
}
