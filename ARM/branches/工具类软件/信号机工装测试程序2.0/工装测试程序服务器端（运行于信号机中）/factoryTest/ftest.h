
#ifndef __FTEST_H_
#define __FTEST_H_



#define MAX_BOARD_NUM 6
#define CHUAN_NUM	32


#define GPIO_DEV	"/dev/gpio"


#define TSC_TYPE_300	1
#define TSC_TYPE_500	2


#define TSC_PROCESS_SHUTDOWN	0
#define TSC_PROCESS_BOOT	1

#define WIRELESS_CHECK_ON 1
#define WIRELESS_CHECK_OFF 0

//int OpenSysDev(void);
//void TscProcessCtrl(char cmd);

int usb_test(char *result);
int TSC_RS485_Test(char *result);
int cur_Volt_Test(char *result);
int wifi_state_check(char *result);
void light_on_inOrder();
void font_board_led_Test();
void wireless_check_ctrl(int flag);
void get_ped_key_status(int *result);
int get_tsc_type(void);
void Key_board_check_ctrl(int val);
void Ftest_init(void);
void Ftest_finished(void);

#endif