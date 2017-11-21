/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : inifile.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��7��31��
  ����޸�   :
  ��������   : inifile.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��7��31��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __INIFILE_H__
#define __INIFILE_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "configureManagement.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void get_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom);
extern void get_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc);
extern void get_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config);
extern void set_custom_params(char *fileName,STRUCT_BINFILE_CUSTOM *pCustom);
extern void set_desc_params(char *fileName,STRUCT_BINFILE_DESC *pDesc);
extern void set_special_params(char *fileName,STRUCT_BINFILE_CONFIG *config);
extern void ReadMiscCfgFromIni(char *fileName,STRUCT_BINFILE_MISC *cfg);
extern void WriteMiscCfgToIni(char *fileName,STRUCT_BINFILE_MISC *cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INIFILE_H__ */
