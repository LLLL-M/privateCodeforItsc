/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : DataExchange.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��11��25��
  ����޸�   :
  ��������   : DataExchange.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/


#ifndef __DATAEXCHANGE_H__
#define __DATAEXCHANGE_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "hikConfig.h"
#include "Util.h"
#include "main.h"
#include "parse_ini.h"

//#include "WebsCallback.h"
#include	"../uemf.h"
#include	"../wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>

#include "../webs.h"


#include    "../inifile.h"
#include    "../util_xml.h"


#define BUF_SIZE 256 

#define VERSION_WEB_APP "1.0.0.2"


typedef struct {    
	unsigned char nPhaseID;//��λ��    
	unsigned char nCircleID;//����    
	unsigned char nArrayConcurrentPase[NUM_PHASE];//������λ
} ConcurrentPhaseItem,*PConcurrentPhaseItem;//������λ��




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void ClearConcurrentPhaseArrayItem(SignalControllerPara *pData,unsigned short nPhaseId);
extern int getChannelTableInfo(stChannelTable *pstChannelTable, int i);
extern int getCoordinateInfo(stCoordinate *pstCoordinate,char *strConfigPath);
extern int getFaultConfigInfo(stFaultConfig *pstFaultConfig,char *strConfigPath);
extern int getFaultDetectionSetInfo(stFaultDetectionSet *pstFaultDetectionSet,char *strConfigPath);
extern int getGreenRatioInfo(stGreenRatio *pstGreenRatio,int iSplit);
extern int getLoginInfo(char_t * username,char_t * password);
extern int getOverlappingInfo(stOverlapping *pstOverlapping,int Id);
extern int getPedestrianInfo(stPedestrian *pstPedestrian,int id);
extern int getPhaseTableInfo(stPhaseTable *pstPhaseTable,int i);
extern int getProgramTableInfo(stProgramTable *pstProgramTable, int num);
extern int getringAndPhaseInfo(PConcurrentPhaseItem item);
extern int getSchedulingInfo(stScheduling *pstScheduling,int Id);
extern int getSequenceTableInfo(stSequenceTable *pstSequenceTable,int nPhaseTurnId);
extern int getTimeBasedActionTableInfo(stTimeBasedActionTable *pstTimeBasedActionTable,int num);
extern int getTimeIntervalInfo(PTimeIntervalItem pItem,int id);
extern int getTreeDynamicParameter(stTreeDynamicPara *pstTreeDynamicPara);
extern int getUnitParamsInfo(stUnitParams *pstUnitParams);
extern int getVehicleDetectorInfo(stVehicleDetector *pstVehicleDetector,int id);
extern int saveCoordinate2Ini(stCoordinate stCoordinateEx,char * strConfigPath);
extern int saveFaultConfig2Ini(stFaultConfig stFaultConfigEx,char * strConfigPath);
extern int saveFaultDetectionSet2Ini(stFaultDetectionSet stFaultDetectionSetEx,char * strConfigPath);
extern int setPedestrianInfo(stPedestrian *pstPedestrianEx,int id);
extern int saveTreeDynamicParameter(stTreeDynamicPara *stTreeDynamicParaEx);
extern int setVehicleDetectorInfo(stVehicleDetector *pstVehicleDetector,int id);
extern int setChannelTableInfo(stChannelTable *pstChannelTable, int i);
extern int setGreenRatioInfo(stGreenRatio *pstGreenRatio,int iSplit);
extern int setOverlappingInfo(stOverlapping *pstOverlapping,int Id);
extern int setPhaseTableInfo(stPhaseTable *pstPhaseTable,int i);
extern int setProgramTableInfo(stProgramTable *pstProgramTable, int num);
extern int setringAndPhaseInfo(PConcurrentPhaseItem item);
extern int setSchedulingInfo(stScheduling *pstScheduling,int Id);
extern int setSequenceTableInfo(stSequenceTable *pstSequenceTable,int nPhaseTurnId);
extern int setTimeBasedActionTableInfo(stTimeBasedActionTable *pstTimeBasedActionTable,int num);
extern int setTimeIntervalInfo(PTimeIntervalItem pItem,int id);
extern int setUnitParamsInfo(stUnitParams *pstUnitParams);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DATAEXCHANGE_H__ */
