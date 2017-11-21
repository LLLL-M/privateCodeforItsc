/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : light.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年12月1日
  最近修改   :
  功能描述   : light.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __LIGHT_H__
#define __LIGHT_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

typedef struct 
{
	unsigned short greenTime;
	unsigned short greenBlinkTime;
	unsigned short yellowTime;
	unsigned short allRedTime;
	//行人
    unsigned short pedestrianPassTime;//行人放行时间
    unsigned short pedestrianClearTime;//行人清空时间
} lightTime_t;

typedef enum 
{
	TURN_OFF = 0,
	GREEN,
	GREEN_BLINK,
	YELLOW,
	YELLOW_BLINK,
	RED
} lightStatus;

typedef struct 
{
	unsigned short channel[36];	//每个相位控制的通道
	int num;	//每个相位控制的通道个数
} PhaseChannel, *PPhaseChannel;
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
//设置需要发送的点灯数组
extern void ControlLampLight(lightStatus *allChannel, unsigned short *nOutLampArray);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LIGHT_H__ */

