/*hdr
**
**	
**
**	FILE NAME:	
**
**	AUTHOR:		
**
**	DATE:		
**
**	FILE DESCRIPTION:
**			
**			
**
**	FUNCTIONS:	
**			
**			
**			
**	NOTES:		
*/  

#include "CPLD.h"


#define DEVICE_NAME "/dev/CPLD_IO"

//\xBB\xC6\xC9\xC1\xCA\xE4\xB3\xF6
#define YELLOW_CONTROL_OUTPUT1_1      0x1001
#define YELLOW_CONTROL_OUTPUT2_1      0x1002

#define YELLOW_CONTROL_OUTPUT1_0      0x0001
#define YELLOW_CONTROL_OUTPUT2_0      0x0002

//5\xB8\xF6LED\xCA\xE4\xB3\xF6
#define LED_OUTPUT1_1             	  0x1010
#define LED_OUTPUT2_1             	  0x1011
#define LED_OUTPUT3_1             	  0x1012
#define LED_OUTPUT4_1             	  0x1013
#define LED_OUTPUT5_1             	  0x1014

#define LED_OUTPUT1_0             	  0x0010
#define LED_OUTPUT2_0             	  0x0011
#define LED_OUTPUT3_0             	  0x0012
#define LED_OUTPUT4_0             	  0x0013
#define LED_OUTPUT5_0             	  0x0014

//20\xB8\xF6IO\xCA\xE4\xB3\xF6
#define IO_OUTPUT1_1              	  0x1020
#define IO_OUTPUT2_1              	  0x1021
#define IO_OUTPUT3_1              	  0x1022
#define IO_OUTPUT4_1              	  0x1023
#define IO_OUTPUT5_1              	  0x1024
#define IO_OUTPUT6_1              	  0x1025
#define IO_OUTPUT7_1              	  0x1026
#define IO_OUTPUT8_1              	  0x1027
#define IO_OUTPUT9_1              	  0x1028
#define IO_OUTPUT10_1             	  0x1029
#define IO_OUTPUT11_1             	  0x102a
#define IO_OUTPUT12_1             	  0x102b
#define IO_OUTPUT13_1             	  0x102c
#define IO_OUTPUT14_1             	  0x102d
#define IO_OUTPUT15_1             	  0x102e
#define IO_OUTPUT16_1             	  0x1030
#define IO_OUTPUT17_1             	  0x1031
#define IO_OUTPUT18_1             	  0x1032
#define IO_OUTPUT19_1             	  0x1033
#define IO_OUTPUT20_1             	  0x1034

#define IO_OUTPUT1_0              	  0x0020
#define IO_OUTPUT2_0              	  0x0021
#define IO_OUTPUT3_0              	  0x0022
#define IO_OUTPUT4_0              	  0x0023
#define IO_OUTPUT5_0              	  0x0024
#define IO_OUTPUT6_0              	  0x0025
#define IO_OUTPUT7_0              	  0x0026
#define IO_OUTPUT8_0             	  0x0027
#define IO_OUTPUT9_0              	  0x0028
#define IO_OUTPUT10_0             	  0x0029
#define IO_OUTPUT11_0             	  0x002a
#define IO_OUTPUT12_0             	  0x002b
#define IO_OUTPUT13_0             	  0x002c
#define IO_OUTPUT14_0             	  0x002d
#define IO_OUTPUT15_0             	  0x002e
#define IO_OUTPUT16_0             	  0x0030
#define IO_OUTPUT17_0             	  0x0031
#define IO_OUTPUT18_0             	  0x0032
#define IO_OUTPUT19_0             	  0x0033
#define IO_OUTPUT20_0             	  0x0034

//32\xB8\xF6IO\xCA\xE4\xC8\xEB
#define IO_INPUT1_TO_INPUT8     	  0x2040  
#define IO_INPUT9_TO_INPUT16    	  0x2041 
#define IO_INPUT17_TO_INPUT24   	  0x2042 
#define IO_INPUT25_TO_INPUT32   	  0x2043 

//5\xB8\xF6\xBC\xFC\xC5\x{0330}\x{5c34}\xBC\xFC\xBD\xD3\xCA\xD5
#define KEYBOARD_INPUT1         	  0x2110
#define KEYBOARD_INPUT2         	  0x2111
#define KEYBOARD_INPUT3         	  0x2112
#define KEYBOARD_INPUT4         	  0x2113
#define KEYBOARD_INPUT5         	  0x2114

//5\xB8\xF6\xBC\xFC\xC5\x{0330}\x{5c34}\xBC\xFC\xB5\x{0235}\xE3\xC1\xC1
#define KEYBOARD_OUTPUT1_1        	  0x1100
#define KEYBOARD_OUTPUT2_1        	  0x1101
#define KEYBOARD_OUTPUT3_1        	  0x1102
#define KEYBOARD_OUTPUT4_1        	  0x1103
#define KEYBOARD_OUTPUT5_1        	  0x1104

#define KEYBOARD_OUTPUT1_0        	  0x0100
#define KEYBOARD_OUTPUT2_0        	  0x0101
#define KEYBOARD_OUTPUT3_0        	  0x0102
#define KEYBOARD_OUTPUT4_0        	  0x0103
#define KEYBOARD_OUTPUT5_0        	  0x0104

int fd = 0;
int auto_pressed = 1;
int manual_pressed = 0;
int flashing_pressed = 0;
int allred_pressed = 0;
int step_by_step_pressed = 0;
static int HiktcsRunningFlag = 0;
time_t lasttime = 0;
time_t system_begin_time = 0;
int backup_count = 0;



/*********************************************************************************
*
* 	CPU\xD3\xEBCPLD\xBD\xBB\xBB\xA5\xB3\xF5\x{02bc}\xBB\xAF
*
***********************************************************************************/
void CPLD_IO_Init()
{
	//\xBB\xF1\x{0221}\xB5\xB1\x{01f0}\x{02b1}\xBC\xE4\xC3\xEB\xCA\xFD
	(void) time(&system_begin_time); 
	fd = open(DEVICE_NAME,O_RDONLY);
	if(fd == -1)
	{
		perror("Open device /dev/CPLD_IO error!");
		exit(1);
	}
}

/*********************************************************************************
*
* 	\x{03e8}\xC3\xF0GPS\x{05b8}\x{02be}\xB5\xC6
*
***********************************************************************************/
void Set_LED2_OFF()
{
	if(fd != -1)
	{
		//\x{03e8}\xC3\xF0GPS\x{05b8}\x{02be}\xB5\xC6
		(void) ioctl(fd,LED_OUTPUT2_0);
	}
}


			

/*********************************************************************************
*
* 	CPU\xBF\xD8\xD6\xC6CPLD\xB7\xA2\xCB\x{037b}\xC6\xC9\xC1\xBF\xD8\xD6\xC6\xD0\x{017a}\x{0178}\xF8\xB5\xE7\x{0534}\xB0\x{5863}
*   CPLD\xC8\xE7\xB9\xFB\xBC\xEC\xB2\x{2d7d}\xB8\x{07f5}\x{0378}\x{07f5}\xCD\xC2\xF6\xB3\x{58ec}\xD4\xF2\xCA\xE4\xB3\xF6\xB8\x{07f5}\xE7\x{01bd}\xA3\xBB\xC8\xE7\xB9\xFB\x{00fb}\xD3\x{043c}\xEC\xB2\x{2d7d}\xA3\xAC\xD4\xF2\xCA\xE4\xB3\xF6\xB5\x{0375}\xE7\x{01bd}\xA1\xA3
*
***********************************************************************************/
void HardflashDogCtrl(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB7\xA2\xCB\x{0378}\x{07f5}\xE7\x{01bd}
			ioctl(fd,YELLOW_CONTROL_OUTPUT1_1);
			ioctl(fd,YELLOW_CONTROL_OUTPUT2_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xB7\xA2\xCB\x{0375}\x{0375}\xE7\x{01bd}
			ioctl(fd,YELLOW_CONTROL_OUTPUT1_0);
			ioctl(fd,YELLOW_CONTROL_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
*	\x{0434}\xD6\xF7\xBF\x{0630}\xE5\xD4\xCB\xD0\xD0\x{05b8}\x{02be}\xB5\x{01a1}\xA3
*
***********************************************************************************/

void Hiktsc_Running_Status(void)
{

	if (HiktcsRunningFlag == 1)
	{
		HiktcsRunningFlag = 0;
		if(fd != -1)
		{
			ioctl(fd,LED_OUTPUT1_1);
		}
	}
	else
	{
		HiktcsRunningFlag = 1;
		if(fd != -1)
		{
			ioctl(fd,LED_OUTPUT1_0);
		}
	}
	return;
}

/*********************************************************************************
*
* 	\xB5\xE3\xC1\xC1\xBB\xF2\xD5\xDF\x{03e8}\xC3\x{32f7d}\xF8\xB5\xC6
*
***********************************************************************************/
void SetStepbyStepLight(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB5\xE3\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT1_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xC3\xF0\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT1_0);
		}
	}
}

/*********************************************************************************
*
* 	\xB5\xE3\xC1\xC1\xBB\xF2\xD5\xDF\x{03e8}\xC3\xF0\xBB\xC6\xC9\xC1\xB5\xC6
*
***********************************************************************************/
void SetFlashingLight(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB5\xE3\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT2_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xC3\xF0\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT2_0);
		}
	}
}

/*********************************************************************************
*
* 	\xB5\xE3\xC1\xC1\xBB\xF2\xD5\xDF\x{03e8}\xC3\xF0\xD7\x{0536}\xAF\xB5\xC6
*
***********************************************************************************/
void SetAutoLight(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB5\xE3\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT3_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xC3\xF0\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT3_0);
		}
	}
}

/*********************************************************************************
*
* 	\xB5\xE3\xC1\xC1\xBB\xF2\xD5\xDF\x{03e8}\xC3\xF0\xCA\x{05b6}\xAF\xB5\xC6
*
***********************************************************************************/
void SetManualLight(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB5\xE3\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT4_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xC3\xF0\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT4_0);
		}
	}
}

/*********************************************************************************
*
* 	\xB5\xE3\xC1\xC1\xBB\xF2\xD5\xDF\x{03e8}\xC3\xF0\x{022b}\xBA\xEC\xB5\xC6
*
***********************************************************************************/
void SetAllRedLight(int value)
{
	if(value == 1)
	{
		if(fd != -1)
		{
			//\xB5\xE3\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT5_1);
		}
	}
	else
	{
		if(fd != -1)
		{
			//\xC3\xF0\xB5\xC6
			ioctl(fd,KEYBOARD_OUTPUT5_0);
		}
	}
}

/*********************************************************************************
*
* 	\xBB\xF1\x{0221}\xB2\xBD\xBD\xF8\xB0\xB4\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xA3\xAC\xB7\xB5\xBB\xD81\xB1\xED\x{02be}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD80\xB1\xED\x{02be}\x{03b4}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD8-1\xB1\xED\x{02be}\xD2\x{cce3}
*
***********************************************************************************/
int GetStepByStepKeyStatus()
{
	int arg = -1;
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT1,&arg);
		//printf("Get keyboard 1 value =%d\n",arg);
		if(arg == 1)
		{
			//\xB8\x{00fc}\xFC\x{00fb}\xD3\x{0430}\xB4\xCF\xC2
			return 0;
		}
		else if(arg == 0)
		{
			//\xB8\x{00fc}\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
			return 1;
		}
		else
		{
			//\xD2\x{cce3}\x{05f4}\x{032c}
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	\xBB\xF1\x{0221}\xBB\xC6\xC9\xC1\xB0\xB4\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xA3\xAC\xB7\xB5\xBB\xD81\xB1\xED\x{02be}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD80\xB1\xED\x{02be}\x{03b4}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD8-1\xB1\xED\x{02be}\xD2\x{cce3}
*
***********************************************************************************/
int GetFlashingKeyStatus()
{
	int arg = -1;
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT2,&arg);
		//printf("Get keyboard 2 value =%d\n",arg);
		if(arg == 1)
		{
			//\xB8\x{00fc}\xFC\x{00fb}\xD3\x{0430}\xB4\xCF\xC2
			return 0;
		}
		else if(arg == 0)
		{
			//\xB8\x{00fc}\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
			return 1;
		}
		else
		{
			//\xD2\x{cce3}\x{05f4}\x{032c}
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	\xBB\xF1\x{0221}\xD7\x{0536}\xAF\xB0\xB4\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xA3\xAC\xB7\xB5\xBB\xD81\xB1\xED\x{02be}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD80\xB1\xED\x{02be}\x{03b4}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD8-1\xB1\xED\x{02be}\xD2\x{cce3}
*
***********************************************************************************/
int GetAutoKeyStatus()
{
	int arg = -1;
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT3,&arg);
		//printf("Get keyboard 3 value =%d\n",arg);
		if(arg == 1)
		{
			//\xB8\x{00fc}\xFC\x{00fb}\xD3\x{0430}\xB4\xCF\xC2
			return 0;
		}
		else if(arg == 0)
		{
			//\xB8\x{00fc}\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
			return 1;
		}
		else
		{
			//\xD2\x{cce3}\x{05f4}\x{032c}
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	\xBB\xF1\x{0221}\xCA\x{05b6}\xAF\xB0\xB4\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xA3\xAC\xB7\xB5\xBB\xD81\xB1\xED\x{02be}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD80\xB1\xED\x{02be}\x{03b4}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD8-1\xB1\xED\x{02be}\xD2\x{cce3}
*
***********************************************************************************/
int GetManualKeyStatus()
{
	int arg = -1;
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT4,&arg);
		//printf("Get keyboard 4 value =%d\n",arg);
		if(arg == 1)
		{
			//\xB8\x{00fc}\xFC\x{00fb}\xD3\x{0430}\xB4\xCF\xC2
			return 0;
		}
		else if(arg == 0)
		{
			return 1;
		}
		else
		{
			//\xD2\x{cce3}\x{05f4}\x{032c}
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*********************************************************************************
*
* 	\xBB\xF1\x{0221}\x{022b}\xBA\x{cc34}\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xA3\xAC\xB7\xB5\xBB\xD81\xB1\xED\x{02be}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD80\xB1\xED\x{02be}\x{03b4}\xB0\xB4\xCF\x{00a3}\xAC\xB7\xB5\xBB\xD8-1\xB1\xED\x{02be}\xD2\x{cce3}
*
***********************************************************************************/
int GetAllRedKeyStatus()
{
	int arg = -1;
	if(fd != -1)
	{
		ioctl(fd,KEYBOARD_INPUT5,&arg);
		//printf("Get keyboard 5 value =%d\n",arg);
		if(arg == 1)
		{
			//\xB8\x{00fc}\xFC\x{00fb}\xD3\x{0430}\xB4\xCF\xC2
			return 0;
		}
		else if(arg == 0)
		{
			//\xB8\x{00fc}\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
			return 1;
		}
		else
		{
			//\xD2\x{cce3}\x{05f4}\x{032c}
			return -1;
		}
	}
	else
	{
		return -1;
	}
}


/*********************************************************************************
*
* 	\xB8\xF9\xBE\xDD\x{02b5}\xBC\x{02b5}\x{013c}\xFC\xC5\x{0330}\xB4\x{0165}\xB0\xB4\xCF\x{00b5}\xC4\x{05f4}\x{032c}\xC0\xB4\xB5\xE3\xB5\xC6
*
***********************************************************************************/
void ProcessKeyBoardLight(void)
{
	
	SetStepbyStepLight(step_by_step_pressed);
	SetFlashingLight(flashing_pressed);
	SetAutoLight(auto_pressed);
	SetManualLight(manual_pressed);
	SetAllRedLight(allred_pressed);
}

/*********************************************************************************
*
*  \xBC\xFC\xC5\x{0334}\xA6\xC0\xED
	\xB7\xB5\xBB\xD8\x{05b5}:
	0: \xCE\x{07bc}\xFC\xB0\xB4\xCF\xC2.
	1:\xD7\x{0536}\xAF\xBC\xFC\xB0\xB4\xCF\xC2,\xC7\xD2\xD2\x{0477}\x{017f}\xAA.
	2:\xCA\x{05b6}\xAF\xBC\xFC\xB0\xB4\xCF\xC2,\xC7\xD2\xD2\x{0477}\x{017f}\xAA.
	3:\xBB\xC6\xC9\xC1\xBC\xFC\xB0\xB4\xCF\xC2,\xC7\xD2\xD2\x{0477}\x{017f}\xAA.
	4:\x{022b}\xBA\xEC\xBC\xFC\xB0\xB4\xCF\xC2,\xC7\xD2\xD2\x{0477}\x{017f}\xAA.
	5:\xB2\xBD\xBD\xF8\xBC\xFC\xB0\xB4\xCF\xC2,\xC7\xD2\xD2\x{0477}\x{017f}\xAA.
*
***********************************************************************************/
int ProcessKeyBoard()
{
	int tmpint = -1;
	time_t curtime = 0;
	time(&curtime);
	//\x{03f5}\x{0373}\xC6\xF0\xC0\xB4\xBA\xF312s\xC4\xDA\xC8\xE7\xB9\xFB\xBC\xEC\xB2\x{2d7d}\xBB\xC6\xC9\xC1\xB0\xB4\x{0165}\xB0\xB4\xCF\xC24\xC3\xEB\xD2\xD4\xC9\xCF\x{02b1}\xBC\x{48ec}\xD4\xF2\x{027e}\xB3\xFD\xC5\xE4\xD6\xC3\xCE\x{013c}\xFE\xB2\xA2\xD6\xD8\xC6\xF4
	if(curtime - system_begin_time < 12)
	{
		if(GetFlashingKeyStatus() == 1)
		{
			//\xBB\xC6\xC9\xC1\xBC\xFC\xB0\xB4\xCF\x{00a3}\xAC\xB8\xB4\x{03bb}\xBC\xC6\xCA\xFD\xBC\xD31
			backup_count ++;
		}
		//\xB4\x{fd7d}\xBC\xFC\xC5\x{0330}\xE5\xBC\xC6\xCA\xFD\xE3\xD0\x{05b5}
		if(backup_count >= 10)
		{
			//\x{03e8}\xC3\xF0\xD7\x{0536}\xAF\xB0\xB4\x{0165}\xB5\x{01b2}\xA2\x{027e}\xB3\xFD\xC5\xE4\xD6\xC3\xCE\x{013c}\xFE
			SetAutoLight(0);
			system("rm -f /home/data/*");
			//\xD6\xD8\xC6\xF4\xD0\x{017a}\x{017b}\xFA
			system("reboot");
		}
	}
	else
	{
		if(auto_pressed == 1)       //\xD7\x{0536}\xAF\xBC\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
		{
			//\xBC\xEC\xB2\xE2\xCA\x{05b6}\xAF\xB0\xB4\x{0165}\xCA\x{01f7}\xF1\xB0\xB4\xCF\xC2
			tmpint = GetManualKeyStatus();
			if(tmpint == 1)
			{
				//\xBC\xEC\xB2\x{2d7d}\xCA\x{05b6}\xAF\xBC\xFC\xB0\xB4\xCF\xC2
				auto_pressed = 0;
				manual_pressed = 1;
				ProcessKeyBoardLight();
				return 2;
			}
			else if(tmpint == 0)
			{
				//\x{00fb}\xD3\x{043c}\xEC\xB2\x{2d7d}\xCA\x{05b6}\xAF\xBC\xFC\xB0\xB4\xCF\xC2
				return 0;
			}	
		}
		else if(manual_pressed == 1)     //\xCA\x{05b6}\xAF\xBC\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
		{
			if(GetAutoKeyStatus() == 1)   //\xBC\xEC\xB2\xE2\xD7\x{0536}\xAF\xBC\xFC\xCA\x{01f7}\xF1\xB0\xB4\xCF\xC2
			{
				//\xBC\xEC\xB2\x{2d7d}\xD7\x{0536}\xAF\xBC\xFC\xB0\xB4\xCF\xC2
				auto_pressed = 1;
				manual_pressed = 0;
				flashing_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				return 1;
			}
			else if(GetFlashingKeyStatus() == 1)   //\xBC\xEC\xB2\xE2\xBB\xC6\xC9\xC1\xBC\xFC\xCA\x{01f7}\xF1\xB0\xB4\xCF\xC2
			{
				//\xBB\xC6\xC9\xC1\xBC\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
				flashing_pressed = 1;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				allred_pressed = 0;
				ProcessKeyBoardLight();
				return 3;	
			}
			else if(GetAllRedKeyStatus() == 1)   //\xBC\xEC\xB2\xE2\x{022b}\xBA\xEC\xBC\xFC\xCA\x{01f7}\xF1\xB0\xB4\xCF\xC2
			{
				//\x{022b}\xBA\xEC\xBC\xFC\xD2\x{047e}\xAD\xB0\xB4\xCF\xC2
				allred_pressed = 1;
				flashing_pressed = 0;
				auto_pressed = 0;
				step_by_step_pressed = 0;
				ProcessKeyBoardLight();
				return 4;			
			}
			else if((GetStepByStepKeyStatus() == 1) && flashing_pressed == 0 
				&& allred_pressed == 0)				//\xBC\xEC\xB2\x{2cbd}\xBD\xF8\xBC\xFC\xCA\x{01f7}\xF1\xB0\xB4\xCF\x{00a3}\xAC\xB2\xBD\xBD\xF8\xB5\x{023c}\xB6\xBD\x{03fb}\xC6\xC9\xC1\xBA\xCD\x{022b}\xBA\xEC\xB5\xCD
			{
				time(&curtime);
				//3\xC3\xEB\xC4\x{06b2}\xBD\xBD\xF8\xBC\xFC\xCE\x{07b7}\xA8\xB6\xE0\xB4\x{03b4}\xA5\xB7\xA2
				if((curtime - lasttime) > 3)
				{
					//\xB2\xBD\xBD\xF8\xBC\xFC\xC6\xF0\x{0427}
					lasttime = curtime;
					step_by_step_pressed = 1;
					allred_pressed = 0;
					flashing_pressed = 0;
					auto_pressed = 0;
					ProcessKeyBoardLight();
					return 5;
				}
			}
		}
	}
	return 0;
}

/*********************************************************************************
*
* 	\xB6\xC1\xD0\xD0\xC8\x{02f0}\xB4\x{0165}\xD0\x{017a}\x{0161}\xA3
*
***********************************************************************************/
int PedestrianCheck()
{
	int tmpint = -1;
	if(fd != -1)
	{
		ioctl(fd,IO_INPUT1_TO_INPUT8,&tmpint);
		//\xB5\xCD8\x{03bb}\x{0221}\xB7\xB4
		tmpint = ((tmpint ^ 0xFFFF)&0xFF);
		return tmpint;
	}
	else
	{
		return 0;
	}
}


/*********************************************************************************
*
* 	\xB6\xC1\xC3\x{017f}\xAA\xB9\xD8\xD0\x{017a}\x{0161}\xA3
*
***********************************************************************************/

int DoorCheck()
{
	int tmpint = -1;
	if(fd != -1)
	{
		ioctl(fd,IO_INPUT9_TO_INPUT16,&tmpint);
		//\xD7\xEE\xB5\xCD\x{03bb}\x{0221}\xB7\xB4
		tmpint = ((tmpint ^ 0xFFFF)&0x01);
		return tmpint;
	}
	else
	{
		return 0;
	}
}

