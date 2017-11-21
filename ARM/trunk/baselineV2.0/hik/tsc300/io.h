#ifndef __IO_H__
#define __IO_H__

typedef enum
{
	V_KEY_INVALID = 0,
	V_KEY_AUTO = 1,
	V_KEY_MANUAL = 2,
	V_KEY_YELLOWBLINK = 3,
	V_KEY_ALLRED = 4,
	V_KEY_STEP = 5,
	V_KEY_EAST = 6,
	V_KEY_SOUTH = 7,
	V_KEY_WEST = 8,
	V_KEY_NORTH = 9,
	V_D_EAST_WEST = 10,
	V_D_SOUTH_NORTH = 11,
	V_L_EAST_WEST = 12,
	V_L_SOUTH_NORTH = 13,
	V_KEY_MAX = 13
}eKeyStatus;


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
extern void KeyBoardInit(void);
extern unsigned char GetKeyBoardStatus(void);

#endif
