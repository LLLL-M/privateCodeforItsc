/******************************************************************************

                  版权所有 (C), 2001-2014, HikVision

 ******************************************************************************
  文 件 名   : Unit.h
  版 本 号   : 初稿
  作    者   : xiaowh
  生成日期   : 2014年7月17日
  最近修改   :
  功能描述   : Unit.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月17日
    作    者   : xiaowh
    修改内容   : 创建文件

******************************************************************************/

#ifndef __UNIT_H__
#define __UNIT_H__

#include "Util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */



typedef struct {

    unsigned short cIsPedestrianAutoClear;//行人自动清空 fuck ,被字节对齐给坑了
    unsigned short nBootYellowLightTime;//启动黄灯时间
    
    unsigned short nDemotionTime;//降级时间
    unsigned short nMinRedTime;//最小红灯时间
    unsigned short nLightFreq;//闪光频率
    unsigned short nGatherCycle;//采集周期
    unsigned short nBootAllRedTime;//启动全红时间
    unsigned short nTransitCycle;//过嘟周期

}UnitPara,*PUnitPara;



extern int LoadDefaultUnitPara(PUnitPara pData);

extern int DestroyUnit(PUnitPara pData);
extern PUnitPara InitUnit();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UNIT_H__ */
