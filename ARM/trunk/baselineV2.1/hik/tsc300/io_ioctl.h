#ifndef __IO_IOCTL_H__
#define __IO_IOCTL_H__

#define XRIO_0		1
#define XRIO_1		2
#define XRIO_2		3
#define XRIO_3		4
#define XRIO_4		5
#define XRIO_5		6
#define XRIO_6		7
#define XRIO_7		8

#define PeriphKey_0		9
#define PeriphKey_1		10
#define PeriphKey_2		11
#define PeriphKey_3		12
#define PeriphKey_4		13

#define PeriphLED_0		14
#define PeriphLED_1		15
#define PeriphLED_2		16
#define PeriphLED_3		17
#define PeriphLED_4		18

#define LED_0			19
#define LED_1			20

#define YF_CTRL			21

#define TTYS4			22
#define TTYS5			23

#define SET_ARG(port, status)	(((port) << 1) | status)

/******************���������س���ʹ��ioctlʱ�Ĳ���ֵ********************/

/*                   [2]    [14]   [8]   [8]  */
/*                  31-30  29-16  15-8   7-0  */
/***    ioctl-cmd =  dir | size | type | nr    ***/
/*      _IOC(dir, type, nr, size)   dir[2]: none(00), w(01), r(10), wr(11)  */
#ifdef __KERNEL__	//for linux driver
#include <asm/ioctl.h>
#else
#include <sys/ioctl.h>	//for linux application
#endif
#define IO_GET_PIN_STATUS			_IOC(3, 114, 100, 60)
#define IO_SET_PIN_STATUS			_IOC(3, 114, 100, 61)
#define IO_GET_BUTTON_STATUS		_IOC(3, 114, 100, 62)
#define IO_SET_BUTTON_STATUS		_IOC(3, 114, 100, 63)
#define IO_GET_WIRELESS_STATUS		_IOC(3, 114, 100, 64)

//ϵͳ����ָʾ��
#define ARG_SYSTEM_RUNNING_LED_ON	SET_ARG(LED_0, 1)
#define ARG_SYSTEM_RUNNING_LED_OFF	SET_ARG(LED_0, 0)
//GPSָʾ��
#define ARG_GPS_LED_ON		SET_ARG(LED_1, 1)
#define ARG_GPS_LED_OFF		SET_ARG(LED_1, 0)
//��������
#define ARG_YF_CTRL_HIGH	SET_ARG(YF_CTRL, 1)
#define ARG_YF_CTRL_LOW		SET_ARG(YF_CTRL, 0)

#define ARG_MANUAL_KEY_STATUS			SET_ARG(PeriphKey_3, 0)	//�ֶ���ť״̬
#define ARG_AUTO_KEY_STATUS				SET_ARG(PeriphKey_2, 0)	//�Զ���ť״̬
#define ARG_YELLOW_BLINK_KEY_STATUS		SET_ARG(PeriphKey_1, 0)	//������ť״̬
#define ARG_ALL_RED_KEY_STATUS			SET_ARG(PeriphKey_4, 0)	//ȫ�찴ť״̬
#define ARG_STEP_KEY_STATUS				SET_ARG(PeriphKey_0, 0)	//������ť״̬

#define ARG_MANUAL_LED_ON			SET_ARG(PeriphLED_0, 0)		//�����ֶ�LED��
#define ARG_AUTO_LED_ON				SET_ARG(PeriphLED_1, 0)		//�����Զ�LED��
#define ARG_YELLOW_BLINK_LED_ON		SET_ARG(PeriphLED_2, 0)		//��������LED��
#define ARG_ALL_RED_LED_ON			SET_ARG(PeriphLED_3, 0)		//����ȫ��LED��
#define ARG_STEP_LED_ON				SET_ARG(PeriphLED_4, 0)		//��������LED��

#define ARG_MANUAL_LED_OFF			SET_ARG(PeriphLED_0, 1)		//Ϩ���ֶ�LED��
#define ARG_AUTO_LED_OFF			SET_ARG(PeriphLED_1, 1)		//Ϩ���Զ�LED��
#define ARG_YELLOW_BLINK_LED_OFF	SET_ARG(PeriphLED_2, 1)		//Ϩ�����LED��
#define ARG_ALL_RED_LED_OFF			SET_ARG(PeriphLED_3, 1)		//Ϩ��ȫ��LED��
#define ARG_STEP_LED_OFF			SET_ARG(PeriphLED_4, 1)		//Ϩ�𲽽�LED��

//ΪRS485Ԥ��
#define ARG_TTYS4_RECEIVE_ENABLE	SET_ARG(TTYS4, 0)	//rs485 ttyS4 ��ʹ��
#define ARG_TTYS4_SEND_ENABLE		SET_ARG(TTYS4, 1)	//rs485 ttyS4 ��ʹ��
#define ARG_TTYS5_RECEIVE_ENABLE	SET_ARG(TTYS5, 0)	//rs485 ttyS5 ��ʹ��
#define ARG_TTYS5_SEND_ENABLE		SET_ARG(TTYS5, 1)	//rs485 ttyS5 ��ʹ��

#endif
