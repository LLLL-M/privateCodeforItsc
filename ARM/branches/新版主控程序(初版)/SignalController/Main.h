/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Main.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月24日
  最近修改   :
  功能描述   : Main.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月24日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "Util.h"
#include "Action.h"
#include "Channel.h"
#include "GreenSignalRatio.h"
#include "LogSystem.h"
#include "MyDateTime.h"
#include "MyLinkList.h"
#include "Phase.h"
#include "Schemes.h"
#include "SqQueue.h"
#include "TaskSchedule.h"
#include "ThreadController.h"
#include "TimeInterval.h"
#include "LampController.h"
#include "FollowPhase.h"
#include "PhaseTurn.h"

#include "Unit.h"


#define LOG_PATH    "./logDir/"
#define LOG_MAX_SIZE    200//M
#define LOG_MAX_SIZE_EACH   10//M

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct {

    PPlanScheduleList pPlanScheduleList;//调度表指针变量
    PTimeIntervalList pTimeIntervalList ;//时段表指针变量
    PActionList pActionList;//动作表指针变量
    PSchemeList pSchemeList;//方案表指针变量
    PChannelList pChannelList ;//通道表指针变量
    PGreenSignalRationList pGreenSignalRationList;//绿信比表指针变量
    PPhaseList pPhaseList;//相位表指针变量
    PUnitPara pUintPara;//单元参数变量
    PFollowPhaseList pFollowPhaseList ;//跟随相位指针变量
    PPhaseTurnList pPhaseTurnList;//相序表指针变量
    
}SignalControllerPara,*PSignalControllerPara;//信号机配置参数主结构




extern void DestroyGolobalVar();
extern void InitGolobalVar();
extern void InitSignalControllerPara();
extern int main(int argc, char **argv);
extern void PowerOnAllRed();
extern void PowerOnYellowLight();
extern void SetGolobalVar();
extern int SignalControllerRun(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */
