/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : countDown_LaiSi.h
  版 本 号   : 初稿
  作    者   : 肖文虎
  生成日期   : 2015年4月2日
  最近修改   :
  功能描述   : countDown_LaiSi.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年4月2日
    作    者   : 肖文虎
    修改内容   : 创建文件

******************************************************************************/

#ifndef __COUNTDOWN_LAISI_H__
#define __COUNTDOWN_LAISI_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_NUM_COUNTDOWN   4
#define MAX_NUM_PHASE       16

#define CFG_NAME_LAISI  "/home/countdown_laisi.ini"
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/


//使用莱斯协议的话，最多支持四个倒计时ID,这些参数是需要从配置文件中读取出来的。
typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //倒计时ID，通过操作倒计时面板上的两个按键来设置倒计时的ID，分别是0 1 2 3 ,数组下标做ID号
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_NUM_PHASE];      //该倒计时对应显示的控制源，一个倒计时可以显示不止一个控制源(相位)，
                                                                        //比如说可以同时显示一个方向的直行和左转倒计时信息,如果控制源有多个，在配置文件中 以逗号隔开，
                                                                        //连续存放，有几个显示几个,相位从1开始。
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //控制源的类型, 可以取ControllerType里面的值。注意暂时我们只考虑机动车、行人、跟随三种控制类型
}CountDownCfgLaiSi;

typedef struct
{
	unsigned char cControllerID;    //控制源ID，其实就是相位或者跟随相位号                    
	unsigned char cColor;           //控制源当前灯色，取值可以是PhaseChannelStatus中的。
	unsigned char cTime;            //控制源倒计时时间，单位是秒 s
}CountDownParamsLaiSi;                   //定义了倒计时参数，

/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void UpdateLaiSiCfg(CountDownCfgLaiSi *pData);
extern void WriteLaiSiCfgToIni();
extern void ReadLaiSiCfgFromIni();
extern void SetCountDownValueLaiSi(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend);
extern void countDownByLaiSiProtocol(unsigned char *pBuf,unsigned char *pLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COUNTDOWN_LAISI_H__ */
