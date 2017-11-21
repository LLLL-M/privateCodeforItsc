

#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "common.h"
#include "HikConfig.h"
#include "platform.h"
#include "configureManagement.h"
#include "hik.h"
#include "tsc.h"


#define GET_COLOR(val) ({\
	char *color = NULL;\
	switch (val) \
	{\
		case 1: color = "��"; break;\
		case 2: color = "��"; break;\
		case 3: color = "��"; break;\
		case 4: color = "����"; break;\
		case 6: color = "ȫ��"; break;\
		default: color = "";\
	}\
	color;})

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum{

    SelfLearning = 0,           //��ѧϰ��ʽ
    FullPulse = 1,              //ȫ�̵���ʱ
    HalfPulse = 2,              //��̵���ʱ
    NationStandard = 3,         //���ұ�׼
    LaiSiStandard = 4,          //��˹��׼
    HisenseStandard = 5,        //���ű�׼
    NationStandard2004 = 6,     //����2004

}CountDownMode;


class CountDown
{
public:

	CountDownCfg        g_CountDownCfg;
	UInt8 gChannelStatus[MAX_CHANNEL_NUM];
	UInt16 gChannelCountdown[MAX_CHANNEL_NUM];
	int fd_485;

	CountDown();
	
	
    void CountDownInterface(LineQueueData *data);

	void SetCountdownValue(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor);

	int GetCountdownNum();

private:
	void SaveCountDownValue(const LineQueueData *data);
	
};


#endif /* __COUNTDOWN_H__ */
