/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Main.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��24��
  ����޸�   :
  ��������   : Main.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��24��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

    PPlanScheduleList pPlanScheduleList;//���ȱ�ָ�����
    PTimeIntervalList pTimeIntervalList ;//ʱ�α�ָ�����
    PActionList pActionList;//������ָ�����
    PSchemeList pSchemeList;//������ָ�����
    PChannelList pChannelList ;//ͨ����ָ�����
    PGreenSignalRationList pGreenSignalRationList;//���űȱ�ָ�����
    PPhaseList pPhaseList;//��λ��ָ�����
    PUnitPara pUintPara;//��Ԫ��������
    PFollowPhaseList pFollowPhaseList ;//������λָ�����
    PPhaseTurnList pPhaseTurnList;//�����ָ�����
    
}SignalControllerPara,*PSignalControllerPara;//�źŻ����ò������ṹ




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
