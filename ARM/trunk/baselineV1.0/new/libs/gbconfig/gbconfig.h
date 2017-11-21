/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : HikConfig.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2014��12��2��
  ����޸�   :
  ��������   : HikConfig.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

  2.��    ��   : 2014��12��3��
    ��    ��   : Ф�Ļ�
    �޸�����   : �������ĳ�����ͷ�ļ������޸�
******************************************************************************/

#ifndef __HIKCONFIG_H__
#define __HIKCONFIG_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "gb.h"
#include "parse_ini.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


typedef struct {
    unsigned char month[12];
    unsigned char day[31];
    unsigned char week[7];
}PlanTime;//�ƻ�ʱ��


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HIKCONFIG_LIB_VERSION	"1.1.0"

#define CFG_NAME "/home/hikconfig.dat"
#define CFG_BAK_NAME "/home/hikconfig.bak"

#define BIT(v, n) (((v) >> (n)) & 0x1)		//ȡv�ĵ� n bitλ

//��λ�Ƿ�ʹ�ܣ��������Ϊ��λѡ���ʹ���ˣ��򷵻�ֵΪ1�����򷵻�ֵΪ0
#define IS_PHASE_INABLE(option)     ((((option)&&0x01) == 1 ) ? 1 : 0)

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/

 
extern Boolean LoadGbDataFromCfg(GbConfig *pSignalControlpara, const char *path);

/*Write*/
extern Boolean WriteGbConfigFile(GbConfig *param, const char *path);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKCONFIG_H__ */
