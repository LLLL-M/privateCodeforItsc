#include <string.h>
#include <pthread.h>
#include "its.h"
#include "canmsg.h"

static pthread_rwlock_t gCanLock = PTHREAD_RWLOCK_INITIALIZER;//灯控板电压及电流互斥锁
#define BOARD_NUM				6	//灯控板个数
#define CHANNEL_NUM_PER_BOARD	6	//每块灯控板上的通道个数
static UInt32 g_volt[BOARD_NUM] = {0}; 	//灯控板电压
static UInt8 g_cur[BOARD_NUM][MAX_CHANNEL_NUM] = {{0}};	//灯控电流

static inline void parse_lamp_voltcur(struct can_frame *pframe)
{
	UInt8 boardNo = pframe->can_id & 0x7;
	if (boardNo == 0 || boardNo > BOARD_NUM)
		return;
	pthread_rwlock_wrlock(&gCanLock);
	g_volt[boardNo - 1] = (pframe->can_id >> 3) & 0x3ffff;
	memcpy(g_cur[boardNo - 1], pframe->data, CHANNEL_NUM_PER_BOARD);
	pthread_rwlock_unlock(&gCanLock);
}

void *CanRecvModule(void *arg)
{
	struct can_frame frame;
	int fd = GetCanSockfd();
	
	while (1)
	{
		pthread_testcancel();
		if (read(fd, &frame, sizeof(struct can_frame)) > 0)
		{
			if ((frame.can_id & 0x3fffff) > 0x200000 && GET_BIT(frame.can_id, 21) == 1)
			{//灯控板电流及电压反馈
				parse_lamp_voltcur(&frame);
			}
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))//车检板信息反馈
			{	//保存过车信息
				SaveVehDectorData(&frame);
			}
		}
		pthread_testcancel();
	}
	pthread_exit(NULL);
}

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
//发送点灯命令
void ItsLight(int boardNum, unsigned short *poutLamp)
{

	if (NULL == poutLamp)
		return;
	unsigned short *p = poutLamp;
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 8;
	m_frame_send.data[0] = 1;
	m_frame_send.data[1] = p[0]&0xff;
	m_frame_send.data[2] = (((p[1]&0xf)<<4) | ((p[0]>>8)&0xf));
	m_frame_send.data[3] = (p[1]>>4)&0xff;
	m_frame_send.data[4] = p[2]&0xff;
	m_frame_send.data[5] = (((p[3]&0xf)<<4) | ((p[2]>>8)&0xf));
	m_frame_send.data[6] = (p[3]>>4)&0xff;
	m_frame_send.data[7] = p[4]&0x3f;
	CanSend(&m_frame_send);
	
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 7;
	m_frame_send.data[0] = 2;
	m_frame_send.data[1] = (((p[5]&0x3)<<6) | ((p[4]>>6)&0x3f));
	m_frame_send.data[2] = (p[5]>>2)&0xff;
	m_frame_send.data[3] = (((p[6]&0x3f)<<2) | ((p[5]>>10)&0x3));
	m_frame_send.data[4] = (((p[7]&0x3)<<6) | ((p[6]>>6)&0x3f));
	m_frame_send.data[5] = (p[7]>>2)&0xff;
	m_frame_send.data[6] = (p[7]>>10)&0x3;
	CanSend(&m_frame_send);
}
#endif

//获取电压
UInt8 GetChannelVoltage(int channel)
{
	if (channel <= 0 || channel > MAX_CHANNEL_NUM)	
		return 0;
	UInt8 volt;
	UInt8 boardNo = (channel - 1) / CHANNEL_NUM_PER_BOARD;	//通道所在的灯控板编号
	UInt8 number = (channel - 1) % CHANNEL_NUM_PER_BOARD;	//通道位于每个灯控板上的编号，从0开始
	pthread_rwlock_rdlock(&gCanLock);
	volt = (g_volt[boardNo] >> (number * 3)) & 0x7;
	pthread_rwlock_unlock(&gCanLock);
	return volt;
}

//获取通道红灯电流
UInt8 GetChannelRedCurrent(int channel)
{
	if (channel <= 0 || channel > MAX_CHANNEL_NUM)	
		return 0;
	UInt8 cur = 0;
	UInt8 boardNo = (channel - 1) / CHANNEL_NUM_PER_BOARD;	//通道所在的灯控板编号，从0开始
	UInt8 number = (channel - 1) % CHANNEL_NUM_PER_BOARD;	//通道位于每个灯控板上的编号，从0开始
	pthread_rwlock_rdlock(&gCanLock);
	cur = g_cur[boardNo][number];
	pthread_rwlock_unlock(&gCanLock);
	return cur;
}
