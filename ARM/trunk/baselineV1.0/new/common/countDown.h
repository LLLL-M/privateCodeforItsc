/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : countDown.h
  �� �� ��   : ����
  ��    ��   : Ф�Ļ�
  ��������   : 2015��4��22��
  ����޸�   :
  ��������   : countDown.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2015��4��22��
    ��    ��   : Ф�Ļ�
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "HikConfig.h"
#include "platform.h"
#include "parse_ini.h"

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//rs485 ttyS4 �շ�ʹ��
#define TTYS4_RECEIVE_ENABLE          0x4001   
#define TTYS4_SEND_ENABLE			  0x4000 
//rs485 ttyS5 �շ�ʹ��
#define TTYS5_RECEIVE_ENABLE   		  0x5001
#define TTYS5_SEND_ENABLE             0x5000
#define DEVICE_NAME_CPLD              "/dev/CPLD_IO"


#define GET_COLOR(val) ({\
	char *color = NULL;\
	switch (val) \
	{\
		case 1: color = "��"; break;\
		case 2: color = "��"; break;\
		case 3: color = "��"; break;\
		case 4: color = "����"; break;\
		case 6: color = "ȫ��"; break;\
		default: color = "";\
	}\
	color;})

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum{

    SelfLearning = 0,           //��ѧϰ��ʽ
    FullPulse = 1,              //ȫ�̵���ʱ
    HalfPulse = 2,              //��̵���ʱ
    NationStandard = 3,         //���ұ�׼
    LaiSiStandard = 4,          //��˹��׼
    HisenseStandard = 5,        //���ű�׼

}CountDownMode;



//�ֱ������������͡������������ˡ������浹��ʱ�����������Ҫÿ����и���,��λID��Ϊ�����±ꡣ
typedef struct
{
	unsigned char cControllerID;    //����ԴID����ʵ������λ���߸�����λ��                    
	unsigned char cColor;           //����Դ��ǰ��ɫ��ȡֵ������PhaseChannelStatus�еġ�
	unsigned char cTime;            //����Դ����ʱʱ�䣬��λ���� s
}CountDownParams;                  


/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void Send485Data(unsigned char *cSendBuf,int nLen);
extern void CountDownInterface();

extern unsigned char GetRuningPhaseId(unsigned char cDeviceId,unsigned char *pPhaseCountDownTime,unsigned char *pPhaseColor);


extern int GetCoundownNum();


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COUNTDOWN_H__ */
