#ifndef __CPLD_H_
#define __CPLD_H_



#define DEVICE_NAME "/dev/CPLD_IO"

//黄闪输出
#define YELLOW_CONTROL_OUTPUT1_1      0x1001
#define YELLOW_CONTROL_OUTPUT2_1      0x1002
#define YELLOW_CONTROL_OUTPUT1_0      0x0001
#define YELLOW_CONTROL_OUTPUT2_0      0x0002

//rs485 ttyS4 收发使能
#define TTYS4_RECEIVE_ENABLE          0x4001   
#define TTYS4_SEND_ENABLE			  0x4000 

//rs485 ttyS5 收发使能
#define TTYS5_RECEIVE_ENABLE   		0x5001
#define TTYS5_SEND_ENABLE             0x5000
//32个IO输入
#define IO_INPUT1_TO_INPUT8     	  0x2040  
#define IO_INPUT9_TO_INPUT16    	  0x2041 
#define IO_INPUT17_TO_INPUT24   	  0x2042 
#define IO_INPUT25_TO_INPUT32   	  0x2043 
//20个IO输出
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

//5个LED输出
#define LED_OUTPUT1_1             	  0x1010
#define LED_OUTPUT2_1             	  0x1011
#define LED_OUTPUT3_1             	  0x1012
#define LED_OUTPUT4_1             	  0x1013
#define LED_OUTPUT5_1             	  0x1014

#define LED_OUTPUT1_0             	  0x0010
#define LED_OUTPUT2_0             	  0x0011
#define LED_OUTPUT3_0             	  0x0012
#define LED_OUTPUT4_0             	  0x0013


//5个键盘板按键接收
#define KEYBOARD_INPUT1         	  0x2110
#define KEYBOARD_INPUT2         	  0x2111
#define KEYBOARD_INPUT3         	  0x2112
#define KEYBOARD_INPUT4         	  0x2113
#define KEYBOARD_INPUT5         	  0x2114

//5个键盘板按键等点亮
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


void CPLD_IO_Init();
void IO_Input_init();
int RS232_test(char *result);
int RS422_test(char *result);
int RS485_test500(char *result);
void Lamp_light_ctrl(int lightFlag);
int IO_OUT_High_TEST(void);
int IO_OUT_Low_TEST(void);
void IOinput_Check_Ctrl(unsigned char flag);
void Get_CarDetector_state(int *result);
void PhaseLampOutput(int boardNum);
void YelloBlink_init500();
void KeybardInit500();
void set_keyboard500_check_flag(int v);
#endif