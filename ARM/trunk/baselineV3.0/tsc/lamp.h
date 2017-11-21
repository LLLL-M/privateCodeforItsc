#pragma once

#include <bitset>
#include "hik.h"
#include "common.h"

#define MAX_LIGHT_BITS	(MAX_CHANNEL_NUM * 3)	//最大点灯字节数

typedef bitset<MAX_LIGHT_BITS> LightBits;

class Lamp
{
private:
	enum
	{
		LOFF	= 0,
		LGREEN	= 1,
		LRED	= 2,
		LYELLOW	= 4,
		LYELLOWRED = 6,
	} lv = LOFF;	//light value,点灯值,只用3bit
	TscStatus	status = INVALID;	//通道状态
	//UInt16	countdown = 0;		//通道倒计时
	int		count = 0;			//闪烁灭灯次数
public:
	//Lamp() = default;
	UInt8 	id = 0;				//通道编号
	Lamp & operator=(const TscStatus &st)
	{
		status = st;
		switch (status)
		{
			case GREEN: lv = LGREEN; count = 0; break;
			case YELLOW: lv = LYELLOW; count = 0; break;
			case RED: lv = LRED; count = 0; break;
			case ALLRED: lv = LRED; count = 0; break;
			case RED_YELLOW: lv = LYELLOWRED; count = 0; break;
			case GREEN_BLINK: lv = LGREEN; count = 2; break;
			case YELLOW_BLINK: lv = LYELLOW; count = 2; break;
			case RED_BLINK: lv = LRED; count = 2; break;
			case OFF_GREEN: lv = LGREEN; count = 1; break;
			case OFF_YELLOW: lv = LYELLOW; count = 1; break;
			case OFF_RED: lv = LRED; count = 1; break;
			default: lv = LOFF;
		}
		return *this;
	}
#if 0
	Lamp & operator=(const UInt16 &cd)
	{
		countdown = cd;
		return *this;
	}
#endif
	void SetLightValue(LightBits &bits)
	{
		if (id == 0 || id > MAX_CHANNEL_NUM)
			return;
		int v = lv;
		if (count > 0)
		{
			count--;
			v = LOFF;
		}
		bits[id * 3 - 3] = v & 0x1;
		bits[id * 3 - 2] = v & 0x2;
		bits[id * 3 - 1] = v & 0x4;
	}
protected:
};

