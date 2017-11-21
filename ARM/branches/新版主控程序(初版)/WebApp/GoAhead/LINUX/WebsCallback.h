/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : WebsCallback.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��11��25��
  ����޸�   :
  ��������   : WebsCallback.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��11��25��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "hikConfig.h"
#include "Util.h"
#include "parse_ini.h"
#include "DataExchange.h"
#include "main.h"
#include "Net.h"

#include	"../uemf.h"
#include	"../wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include "../webs.h"
#include    "../util_xml.h"


/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#ifndef __WEBSCALLBACK_H__
#define __WEBSCALLBACK_H__



#define METHOD_NULL        0x00
#define METHOD_GET           0x01
#define METHOD_POST         0x02
#define METHOD_PUT           0x04
#define METHOD_DELETE     0x10


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int  JudgingType(int flags);
extern void actionDownload(webs_t wp, char_t *path, char_t *query);
extern void channelTable(webs_t wp, char_t *path, char_t *query);
extern void clearAllPara(webs_t wp, char_t *path, char_t *query);
extern void coordinate(webs_t wp, char_t *path, char_t *query);
extern void faultConfig(webs_t wp, char_t *path, char_t *query);
extern void faultDetectionSet(webs_t wp, char_t *path, char_t *query);
extern char * getXMLValue(char * XMLStr,char * rootTag,char * rootTagName,char * returnValue);
extern void greenRatio(webs_t wp, char_t *path, char_t *query);
extern void loginTest(webs_t wp, char_t *path, char_t *query);
extern void overlapping(webs_t wp, char_t *path, char_t *query);
extern void pedestrianDetector(webs_t wp, char_t *path, char_t *query);
extern void PhaseTable(webs_t wp, char_t *path, char_t *query);
extern void programTable(webs_t wp, char_t *path, char_t *query);
extern void resetAllPara(webs_t wp, char_t *path, char_t *query);
extern void ringAndPhase(webs_t wp, char_t *path, char_t *query);
extern void saveAllPara(webs_t wp, char_t *path, char_t *query);
extern void scheduling(webs_t wp, char_t *path, char_t *query);
extern int SendMsgToBoard();
extern void sequenceTable(webs_t wp, char_t *path, char_t *query);
extern int sqliteCallback(void *para, int n_column, char **column_value, char **column_name);
extern void timeBasedActionTable(webs_t wp, char_t *path, char_t *query);
extern void timeInterval(webs_t wp, char_t *path, char_t *query);
extern void TreeDynamicParameter(webs_t wp, char_t *path, char_t *query);
extern void unitParams(webs_t wp, char_t *path, char_t *query);
extern void upldForm(webs_t wp, char_t * path, char_t * query);
extern void vehicleDetector(webs_t wp, char_t *path, char_t *query);
extern void getLibsInfo(webs_t wp, char_t *path, char_t *query);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WEBSCALLBACK_H__ */
