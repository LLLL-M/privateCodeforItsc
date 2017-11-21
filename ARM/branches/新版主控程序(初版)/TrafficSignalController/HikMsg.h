/******************************************************************************

                  ��Ȩ���� (C), 2001-2015, ���ݺ����������ּ����ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : HikMsg.h
  �� �� ��   : ����
  ��    ��   : Jicky
  ��������   : 2014��11��29��
  ����޸�   :
  ��������   : HikMsg.h ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��11��29��
    ��    ��   : Jicky
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __HIKMSG_H__
#define __HIKMSG_H__


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//#define MSGZERO 0	//�������͵���Ϣû������
#define MSGSIZE	20	//����������Ϣ���ݵĳ���

#define NUM_BOARD	8

#define ONE_SECOND_NSECS		1000000000					//1s�����ns��ֵ
#define DELAY_TIME				250							//��ʱʱ�䣬��λ��ms����ÿDELAY_TIMME ms��ʱһ��
//ÿ����ʱ֮ǰ����usleep��ʱ��ʱ�䣬Ԥ��1ms����ȷ��ʱ
#define DELAY_TIME_USECS_BEFORE	((DELAY_TIME - 1) * 1000)	
#define DELAY_TIME_NSECS		(DELAY_TIME * 1000000)			//ÿ����ʱʱ�任���������
#define LOOP_TIMES_PER_SECOND	(ONE_SECOND_NSECS / DELAY_TIME_NSECS)	//ÿ������ѯ���͵����ֵ����
/*----------------------------------------------*
 * �����ض����Լ��ṹ�嶨��                     *
 *----------------------------------------------*/
typedef enum 
{
	MSG_TIMER_START = 1,	//��ʱ��ʼ��Ϣ
	MSG_CONFIG_UPDATE,		//�����и���
	MSG_TIMER_COMPLETE,	//��ʱ���
	MSG_LIGHT,		//�����Ϣ
} MsgType;

struct msgbuf
{
	long mtype;
	char mtext[MSGSIZE];
};
/*----------------------------------------------*
 * �ӿں���                                    *
 *----------------------------------------------*/
extern void  *SignalControllerRun(void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HIKMSG_H__ */

