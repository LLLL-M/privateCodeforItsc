#ifndef __IO_H__
#define __IO_H__

//����GPSָʾ��
extern void GPS_led_on();

//�ر�GPSָʾ��
extern void GPS_led_off();

//д���ذ�����ָʾ��
extern void Hiktsc_Running_Status(void);

//���ſ����źţ�Ԥ��
extern unsigned short DoorCheck();

//�����˰�ť�ź�
extern unsigned short PedestrianCheck();

//����ʵ�ʵļ��̰�ť���µ�״̬�����
extern void ProcessKeyBoardLight(void);

/*********************************************************************************
*
*  ���ݰ�����״̬ȥ��������ָʾ��
	5�������ֱ�Ϊ5��������ǰ��״̬
	����ֵ:
	0: �޼�����.
	1:�Զ�������,���ѷſ�.
	2:�ֶ�������,���ѷſ�.
	3:����������,���ѷſ�.
	4:ȫ�������,���ѷſ�.
	5:����������,���ѷſ�.
*
***********************************************************************************/
extern int ProcessKeyBoard();

//IO��ʼ��
extern void IO_Init();


#endif
