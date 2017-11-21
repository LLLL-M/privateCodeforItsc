/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : light.h
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��12��1��
  ����޸�   :
  ��������   : light.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __LIGHT_H__
#define __LIGHT_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef struct 
{
	unsigned short L0:3;
	unsigned short L1:3;
	unsigned short L2:3;
	unsigned short L3:3;
	unsigned short unused:4;
} lamp_t;

typedef struct 
{
	unsigned short greenTime;
	unsigned short greenBlinkTime;
	unsigned short yellowTime;
	unsigned short allRedTime;
	//����
    unsigned short pedestrianPassTime;//���˷���ʱ��
    unsigned short pedestrianClearTime;//�������ʱ��
} lightTime_t;

typedef enum 
{
	TURN_OFF = 0,
	GREEN,
	GREEN_BLINK,
	YELLOW,
	YELLOW_BLINK,
	RED
} lightStatus;

typedef struct 
{
	unsigned short channel[36];	//ÿ����λ���Ƶ�ͨ��
	int num;	//ÿ����λ���Ƶ�ͨ������
} PhaseChannel, *PPhaseChannel;
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
//������Ҫ���͵ĵ������
extern void ControlLampLight(lightStatus *allChannel, unsigned short *nOutLampArray);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __LIGHT_H__ */

