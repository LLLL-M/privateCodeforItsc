/******************************************************************************

                  版权所有 (C), 2001-2020, hikvision

 ******************************************************************************
  文 件 名   : countdown_nation_2004.h
  版 本 号   : 初稿
  作    者   : jgp
  生成日期   : 2016年7月14日
  最近修改   :
  功能描述   : 倒计时器国标2004协议头文件，主要包括协议需要的宏定义，协议帧
               结构
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月14日
    作    者   : jgp
    修改内容   : 创建文件

******************************************************************************/


#ifndef __COUNTDOWN_NATION_2004_H__
#define __COUNTDOWN_NATION_2004_H__

#define MAX_DIRECTION   8               //国标2004 最多支持8个倒计时器
#define FRAME_HEAD_2004 0xFE            //国标2004 倒计时协议帧头；

//16进制转压缩BCD码
#define HEX2BCD(val)  ((((val) / 10) << 4) + (val) % 10)

//国标2004 帧结构 ；每帧表示一个方向的数据，每次通讯最多可有8帧数据
typedef struct{
    unsigned char frame_head;           //帧头
    unsigned char color_addr;           //颜色/地址码
    unsigned char data_high;            //BCD码表示，数据千位，百位
    unsigned char data_low;             //BCD码表示，数据十位，个位
    unsigned char check_sum;        
}__attribute__((packed,aligned(1))) CountDownFrame2004;

extern void countDownByNationStandard2004();

#endif

