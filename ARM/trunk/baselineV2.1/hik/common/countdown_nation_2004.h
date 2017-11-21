/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, hikvision

 ******************************************************************************
  �� �� ��   : countdown_nation_2004.h
  �� �� ��   : ����
  ��    ��   : jgp
  ��������   : 2016��7��14��
  ����޸�   :
  ��������   : ����ʱ������2004Э��ͷ�ļ�����Ҫ����Э����Ҫ�ĺ궨�壬Э��֡
               �ṹ
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��7��14��
    ��    ��   : jgp
    �޸�����   : �����ļ�

******************************************************************************/


#ifndef __COUNTDOWN_NATION_2004_H__
#define __COUNTDOWN_NATION_2004_H__

#define MAX_DIRECTION   8               //����2004 ���֧��8������ʱ��
#define FRAME_HEAD_2004 0xFE            //����2004 ����ʱЭ��֡ͷ��

//16����תѹ��BCD��
#define HEX2BCD(val)  ((((val) / 10) << 4) + (val) % 10)

//����2004 ֡�ṹ ��ÿ֡��ʾһ����������ݣ�ÿ��ͨѶ������8֡����
typedef struct{
    unsigned char frame_head;           //֡ͷ
    unsigned char color_addr;           //��ɫ/��ַ��
    unsigned char data_high;            //BCD���ʾ������ǧλ����λ
    unsigned char data_low;             //BCD���ʾ������ʮλ����λ
    unsigned char check_sum;        
}__attribute__((packed,aligned(1))) CountDownFrame2004;

extern void countDownByNationStandard2004();

#endif

