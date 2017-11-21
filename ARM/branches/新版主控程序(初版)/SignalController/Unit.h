/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : Unit.h
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��17��
  ����޸�   :
  ��������   : Unit.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��7��17��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

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

    unsigned short cIsPedestrianAutoClear;//�����Զ���� fuck ,���ֽڶ��������
    unsigned short nBootYellowLightTime;//�����Ƶ�ʱ��
    
    unsigned short nDemotionTime;//����ʱ��
    unsigned short nMinRedTime;//��С���ʱ��
    unsigned short nLightFreq;//����Ƶ��
    unsigned short nGatherCycle;//�ɼ�����
    unsigned short nBootAllRedTime;//����ȫ��ʱ��
    unsigned short nTransitCycle;//�������

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
