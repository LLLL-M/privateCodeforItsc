/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : countDown_LaiSi.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��4��2��
  ����޸�   :
  ��������   : countDown_LaiSi.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��4��2��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __COUNTDOWN_LAISI_H__
#define __COUNTDOWN_LAISI_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_NUM_COUNTDOWN   4
#define MAX_NUM_PHASE       16

#define CFG_NAME_LAISI  "/home/countdown_laisi.ini"
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/


//ʹ����˹Э��Ļ������֧���ĸ�����ʱID,��Щ��������Ҫ�������ļ��ж�ȡ�����ġ�
typedef struct
{
    unsigned char cDeviceId[MAX_NUM_COUNTDOWN];                          //����ʱID��ͨ����������ʱ����ϵ��������������õ���ʱ��ID���ֱ���0 1 2 3 ,�����±���ID��
    unsigned char cControllerID[MAX_NUM_COUNTDOWN][MAX_NUM_PHASE];      //�õ���ʱ��Ӧ��ʾ�Ŀ���Դ��һ������ʱ������ʾ��ֹһ������Դ(��λ)��
                                                                        //����˵����ͬʱ��ʾһ�������ֱ�к���ת����ʱ��Ϣ,�������Դ�ж�����������ļ��� �Զ��Ÿ�����
                                                                        //������ţ��м�����ʾ����,��λ��1��ʼ��
    unsigned char cControllerType[MAX_NUM_COUNTDOWN];                   //����Դ������, ����ȡControllerType�����ֵ��ע����ʱ����ֻ���ǻ����������ˡ��������ֿ�������
}CountDownCfgLaiSi;

typedef struct
{
	unsigned char cControllerID;    //����ԴID����ʵ������λ���߸�����λ��                    
	unsigned char cColor;           //����Դ��ǰ��ɫ��ȡֵ������PhaseChannelStatus�еġ�
	unsigned char cTime;            //����Դ����ʱʱ�䣬��λ���� s
}CountDownParamsLaiSi;                   //�����˵���ʱ������

/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void UpdateLaiSiCfg(CountDownCfgLaiSi *pData);
extern void WriteLaiSiCfgToIni();
extern void ReadLaiSiCfgFromIni();
extern void SetCountDownValueLaiSi(PHASE_COUNTING_DOWN_FEEDBACK_PARAMS *pCountDownParamsSend);
extern void countDownByLaiSiProtocol(unsigned char *pBuf,unsigned char *pLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COUNTDOWN_LAISI_H__ */
