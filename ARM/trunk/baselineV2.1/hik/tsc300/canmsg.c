#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "its.h"
#include "canmsg.h"
#include "configureManagement.h"

static pthread_rwlock_t gCanLock = PTHREAD_RWLOCK_INITIALIZER;//�ƿذ��ѹ������������
#define BOARD_NUM				6	//�ƿذ����
#define CHANNEL_NUM_PER_BOARD	4	//ÿ��ƿذ��ϵ�ͨ������
static UInt16 g_volt[BOARD_NUM] = {0}; 	//�ƿذ��ѹ
static UInt8 g_cur[BOARD_NUM][MAX_CHANNEL_NUM] = {{0}};	//�ƿص���

extern STRUCT_BINFILE_CONFIG gStructBinfileConfigPara;//�����������
extern STRUCT_BINFILE_MISC gStructBinfileMisc;         //���Ӳ���
extern void Set_front_board_keys(UInt8 status);

static inline void parse_lamp_voltcur(struct can_frame *pframe)
{
	UInt8 boardNo = pframe->can_id & 0x7;
	if (boardNo == 0 || boardNo > BOARD_NUM)
		return;
	pthread_rwlock_wrlock(&gCanLock);
	g_volt[boardNo - 1] = (pframe->can_id >> 3) & 0xfff;
	memcpy(g_cur[boardNo - 1], pframe->data, CHANNEL_NUM_PER_BOARD);
	pthread_rwlock_unlock(&gCanLock);
}

void *CanRecvModule(void *arg)
{
	struct can_frame frame;
	int fd = GetCanSockfd();
	int times = 0;
	
	while (1)
	{
		pthread_testcancel();
		if (read(fd, &frame, sizeof(struct can_frame)) > 0)
		{
			if (GET_BIT(frame.can_id, 15) == 1 && frame.can_dlc == 4) 
			{//��15λΪ1���ҳ���Ϊ4��can��Ϣ��ʾ�ǵƿذ��������ѹ����
				parse_lamp_voltcur(&frame);
			}
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))//�������Ϣ����
			{	//���������Ϣ
				SaveVehDectorData(&frame);
			}
			else if(frame.can_id == 0x401 && frame.can_dlc == 1) 
			{//ǰ���5������״̬
				if ((frame.data[0] & 0x1f) != 0)
					Set_front_board_keys(frame.data[0] & 0x1f);
			}
			times = 0;
		}
		else
		{	
			if (errno == EAGAIN)
			{//���ճ�ʱ,2sδ�յ����ݣ�˵��can�쳣��
				if (++times <= 10)
					log_error("CAN can't receive data with 2s, it will restart!!!!");
				else
				{	//can������������10�Σ������ϵͳ����
					if (gStructBinfileMisc.cIsCanRestartHiktscAllowed == 1)
					{
						log_error("The TSC will reboot because of can restart beyond 10 times!");
						sync();
						system("reboot");
					}
				}
				if (times & 0x1)
				{
					shutdown(fd, SHUT_RDWR);//shut down the fd
					close(fd);//������ִ��shutdown���ٴ�close������fd�ᱻһֱռ�����ճ���ϵͳ����
					CanInit(500000);	//������ʼ��can
					fd = GetCanSockfd();
				}
				else
				{
					system("ifconfig can0 down");
					sleep(5);
				}	
			}
		}
		pthread_testcancel();
	}
	pthread_exit(NULL);
}

#if defined(__linux__) && defined(__arm__)  //����arm�������gcc���õĺ궨��
//���͵������
void ItsLight(int boardNum, unsigned short *poutLamp)
{
	if (NULL == poutLamp)
		return;
	unsigned short *p = poutLamp;
	struct can_frame m_frame_send;
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 7;
	m_frame_send.data[0] = gStructBinfileConfigPara.sSpecialParams.iSignalMachineType & 0xff;
	m_frame_send.data[1] = p[0]&0xff;
	m_frame_send.data[2] = ((p[1]&0xf)<<4) | ((p[0]>>8)&0xf);
	m_frame_send.data[3] = (p[1]>>4)&0xff;
	m_frame_send.data[4] = p[2]&0xff;
	m_frame_send.data[5] = ((p[3]&0xf)<<4) | ((p[2]>>8)&0xf);
	m_frame_send.data[6] = (p[3]>>4)&0xff;
	CanSend(&m_frame_send);

	//�����Ϊ6���ƿذ�׼����
	memset(&m_frame_send, 0 , sizeof(struct can_frame));
	m_frame_send.can_id = 0x101;
	m_frame_send.can_dlc = 4;
	m_frame_send.data[0] = 3;
	m_frame_send.data[1] = p[4]&0xff;
	m_frame_send.data[2] = ((p[5]&0xf)<<4) | ((p[4]>>8)&0xf);
	m_frame_send.data[3] = (p[5]>>4)&0xff;
	CanSend(&m_frame_send);
}
#endif

//��ȡ��ѹ
UInt8 GetChannelVoltage(int channel)
{
	if (channel <= 0 || channel > MAX_CHANNEL_NUM)	
		return 0;
	UInt8 volt;
	UInt8 boardNo = (channel - 1) / CHANNEL_NUM_PER_BOARD;	//ͨ�����ڵĵƿذ���
	UInt8 number = (channel - 1) % CHANNEL_NUM_PER_BOARD;	//ͨ��λ��ÿ���ƿذ��ϵı�ţ���0��ʼ
	pthread_rwlock_rdlock(&gCanLock);
	volt = (g_volt[boardNo] >> (number * 3)) & 0x7;
	pthread_rwlock_unlock(&gCanLock);
	return volt;
}

//��ȡͨ����Ƶ���
UInt8 GetChannelRedCurrent(int channel)
{
	if (channel <= 0 || channel > MAX_CHANNEL_NUM)	
		return 0;
	UInt8 cur = 0;
	UInt8 boardNo = (channel - 1) / CHANNEL_NUM_PER_BOARD;	//ͨ�����ڵĵƿذ��ţ���0��ʼ
	UInt8 number = (channel - 1) % CHANNEL_NUM_PER_BOARD;	//ͨ��λ��ÿ���ƿذ��ϵı�ţ���0��ʼ
	pthread_rwlock_rdlock(&gCanLock);
	cur = g_cur[boardNo][number];
	pthread_rwlock_unlock(&gCanLock);
	return cur;
}
