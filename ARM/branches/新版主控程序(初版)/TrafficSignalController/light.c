/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : light.c
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��12��1��
  ����޸�   :
  ��������   : ���ļ���Ҫ�������ʹ��
  �����б�   :
              allRed
              ControlLampLight
              get_lamp_value
              put_lamp_value
              yellowBlink
  �޸���ʷ   :
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "light.h"
#include "HikConfig.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define LOFF		0
#define LGREEN		1
#define LYELLOW		4
#define LRED		2
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


/*****************************************************************************
 �� �� ��  : get_lamp_value
 ��������  : ��Ҫ������ȡһ����о���ĳ���Ƶ�״ֵ̬
 �������  : volatile unsigned short *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
 �� �� ֵ  : ����ĳ���Ƶ�״ֵ̬
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline unsigned short get_lamp_value(unsigned short *lights, int n)
{
	lamp_t *p = (lamp_t *)(lights);
	unsigned short value = 0;
	switch (n) 
	{
		case 0:	value = p->L0; break;
		case 1:	value = p->L1; break;
		case 2:	value = p->L2; break;
		case 3:	value = p->L3; break;
		default: break;
	}
	
	return value;
}

/*****************************************************************************
 �� �� ��  : put_lamp_value
 ��������  : ��Ҫ��������һ�����ĳ���Ƶ�״ֵ̬
 �������  : volatile unsigned short *lights  ����һ���״̬��ָ��
             int n                            �������ĸ��ƣ�ֻ����0��1��2��3
             unsigned short value             Ҫ���õĵƵ�״ֵ̬
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
static inline void put_lamp_value(unsigned short *lights, int n, unsigned short value)
{
	lamp_t *p = (lamp_t *)(lights);
	switch (n) 
	{
		case 0:	p->L0 = value; break;
		case 1:	p->L1 = value; break;
		case 2:	p->L2 = value; break;
		case 3:	p->L3 = value; break;
		default: break;
	}
}

/*****************************************************************************
 �� �� ��  : ControlLampLight
 ��������  : ��������ͨ����״̬�����õ�Ƶ�����
 �������  : lightStatus *allChannel  		��������ͨ��״̬������ָ��
 			 unsigned short *nOutLampArray	��ŵ�����ݵ�����
 �� �� ֵ  : 
 �޸���ʷ  
  1.��    ��   : 2014��12��1��
    ��    ��   : Jicky
    �޸�����   : �����ɺ���

*****************************************************************************/
void ControlLampLight(lightStatus *allChannel, unsigned short *nOutLampArray)
{
	int i;
	unsigned short tmp, value;
	unsigned short *nOutLamp = nOutLampArray;
	
	for (i = 0; i < NUM_CHANNEL; i++) 
	{
		//����ͨ������״̬�ҳ����ͨ��Ӧ��������ֵ
		switch (allChannel[i]) 
		{
			case GREEN:	value = LGREEN; break;
			case GREEN_BLINK:
						tmp = get_lamp_value(nOutLamp, i % 4);
						value = (tmp == LOFF) ? LGREEN : LOFF;
						break;	
			case YELLOW:	value = LYELLOW; break;
			case YELLOW_BLINK:
							tmp = get_lamp_value(nOutLamp, i % 4);
							value = (tmp == LOFF) ? LYELLOW: LOFF;
							break;
			case RED:	value = LRED; break;;
			default: value = LOFF; break;
		}
		//�����ͨ����ֵ
		put_lamp_value(nOutLamp, i % 4, value);

		if ((i + 1) % 4 == 0) 
		{
			nOutLamp++;
		}
	}
}

