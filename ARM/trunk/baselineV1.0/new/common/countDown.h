/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : countDown.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年4月22日
  最近修改   :
  功能描述   : countDown.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年4月22日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//rs485 ttyS4 收发使能
#define TTYS4_RECEIVE_ENABLE          0x4001   
#define TTYS4_SEND_ENABLE			  0x4000 
//rs485 ttyS5 收发使能
#define TTYS5_RECEIVE_ENABLE   		  0x5001
#define TTYS5_SEND_ENABLE             0x5000
#define DEVICE_NAME_CPLD              "/dev/CPLD_IO"


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

}CountDownMode;



//分别定义了其他类型、机动车、行人、及跟随倒计时参数，这个需要每秒进行更新,相位ID作为数组下标。
typedef struct
{
	unsigned char cControllerID;    //控制源ID，其实就是相位或者跟随相位号                    
	unsigned char cColor;           //控制源当前灯色，取值可以是PhaseChannelStatus中的。
	unsigned char cTime;            //控制源倒计时时间，单位是秒 s
}CountDownParams;                  


/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void Send485Data(unsigned char *cSendBuf,int nLen);
extern void CountDownInterface();

extern unsigned char GetRuningPhaseId(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor);


extern int GetCoundownNum();


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COUNTDOWN_H__ */
