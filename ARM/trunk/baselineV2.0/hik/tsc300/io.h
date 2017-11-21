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


//写主控板运行指示灯
extern void Hiktsc_Running_Status(void);

//读门开关信号，预留
extern unsigned short DoorCheck();

//读行人按钮信号
extern unsigned short PedestrianCheck();

//根据实际的键盘按钮按下的状态来点灯
extern void ProcessKeyBoardLight(void);

/*********************************************************************************
*
*  根据按键的状态去处理按键的指示灯
	5个参数分别为5个按键当前的状态
	返回值:
	0: 无键按下.
	1:自动键按下,且已放开.
	2:手动键按下,且已放开.
	3:黄闪键按下,且已放开.
	4:全红键按下,且已放开.
	5:步进键按下,且已放开.
*
***********************************************************************************/
extern int ProcessKeyBoard();

//IO初始化
extern void IO_Init();
extern void KeyBoardInit(void);
extern unsigned char GetKeyBoardStatus(void);

#endif
