

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
		case 1: color = "绿"; break;\
		case 2: color = "红"; break;\
		case 3: color = "黄"; break;\
		case 4: color = "绿闪"; break;\
		case 6: color = "全红"; break;\
		default: color = "";\
	}\
	color;})

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef enum{

    SelfLearning = 0,           //自学习方式
    FullPulse = 1,              //全程倒计时
    HalfPulse = 2,              //半程倒计时
    NationStandard = 3,         //国家标准
    LaiSiStandard = 4,          //莱斯标准
    HisenseStandard = 5,        //海信标准
    NationStandard2004 = 6,     //国标2004

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
