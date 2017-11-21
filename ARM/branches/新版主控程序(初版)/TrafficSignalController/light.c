/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : light.c
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年12月1日
  最近修改   :
  功能描述   : 该文件主要用来点灯使用
  函数列表   :
              allRed
              ControlLampLight
              get_lamp_value
              put_lamp_value
              yellowBlink
  修改历史   :
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "light.h"
#include "HikConfig.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define LOFF		0
#define LGREEN		1
#define LYELLOW		4
#define LRED		2
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*****************************************************************************
 函 数 名  : get_lamp_value
 功能描述  : 主要用来获取一组灯中具体某个灯的状态值
 输入参数  : volatile unsigned short *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
 返 回 值  : 返回某个灯的状态值
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline unsigned short get_lamp_value(unsigned short *lights, int n)
{
	lamp_t *p = (lamp_t *)(lights);
	unsigned short value = 0;
	switch (n) 
	{
		case 0:	value = p->L0; break;
		case 1:	value = p->L1; break;
		case 2:	value = p->L2; break;
		case 3:	value = p->L3; break;
		default: break;
	}
	
	return value;
}

/*****************************************************************************
 函 数 名  : put_lamp_value
 功能描述  : 主要用来设置一组灯中某个灯的状态值
 输入参数  : volatile unsigned short *lights  描述一组灯状态的指针
             int n                            具体是哪个灯，只能是0、1、2、3
             unsigned short value             要设置的灯的状态值
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
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
 函 数 名  : ControlLampLight
 功能描述  : 根据所有通道的状态来设置点灯的数组
 输入参数  : lightStatus *allChannel  		描述所有通道状态的数组指针
 			 unsigned short *nOutLampArray	存放点灯数据的数组
 返 回 值  : 
 修改历史  
  1.日    期   : 2014年12月1日
    作    者   : Jicky
    修改内容   : 新生成函数

*****************************************************************************/
void ControlLampLight(lightStatus *allChannel, unsigned short *nOutLampArray)
{
	int i;
	unsigned short tmp, value;
	unsigned short *nOutLamp = nOutLampArray;
	
	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		//根据通道所处状态找出这个通道应该所赋的值
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK:
						tmp = get_lamp_value(nOutLamp, i % 4);
						value = (tmp == LOFF) ? LGREEN : LOFF;
						break;	
			case YELLOW:	value = LYELLOW; break;
			case YELLOW_BLINK:
							tmp = get_lamp_value(nOutLamp, i % 4);
							value = (tmp == LOFF) ? LYELLOW: LOFF;
							break;
			case RED:	value = LRED; break;;
			default: value = LOFF; break;
		}
		//给这个通道赋值
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			nOutLamp++;
		}
	}
}

