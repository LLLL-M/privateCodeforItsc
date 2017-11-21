/******************************************************************************

                  版权所有 (C), 2001-2015, 杭州海康威视数字技术股份有限公司

 ******************************************************************************
  文 件 名   : HikMsg.h
  版 本 号   : 初稿
  作    者   : Jicky
  生成日期   : 2014年11月29日
  最近修改   :
  功能描述   : HikMsg.h 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年11月29日
    作    者   : Jicky
    修改内容   : 创建文件

******************************************************************************/

#ifndef __HIKMSG_H__
#define __HIKMSG_H__


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//#define MSGZERO 0	//表明发送的消息没有数据
#define MSGSIZE	20	//表明发送消息数据的长度

#define NUM_BOARD	8

#define ONE_SECOND_NSECS		1000000000					//1s换算的ns数值
#define DELAY_TIME				250							//延时时间，单位是ms，即每DELAY_TIMME ms延时一次
//每次延时之前先用usleep延时的时间，预留1ms做精确延时
#define DELAY_TIME_USECS_BEFORE	((DELAY_TIME - 1) * 1000)	
#define DELAY_TIME_NSECS		(DELAY_TIME * 1000000)			//每次延时时间换算的纳秒数
#define LOOP_TIMES_PER_SECOND	(ONE_SECOND_NSECS / DELAY_TIME_NSECS)	//每秒钟轮询发送点灯数值次数
/*----------------------------------------------*
 * 类型重定义以及结构体定义                     *
 *----------------------------------------------*/
typedef enum 
{
	MSG_TIMER_START = 1,	//定时开始消息
	MSG_CONFIG_UPDATE,		//配置有更新
	MSG_TIMER_COMPLETE,	//定时完成
	MSG_LIGHT,		//点灯消息
} MsgType;

struct msgbuf
{
	long mtype;
	char mtext[MSGSIZE];
};
/*----------------------------------------------*
 * 接口函数                                    *
 *----------------------------------------------*/
extern void  *SignalControllerRun(void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKMSG_H__ */

