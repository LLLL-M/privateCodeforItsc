#pragma once

#include <bitset>
#include <string>
#include <ctime>
#include <map>

namespace hik
{
	typedef enum
	{
		NONE_KEY = 0,			//无按键
		AUTO_KEY = 1,			//自动按键
		MANUAL_KEY = 2,			//手动按键
		YELLOWBLINK_KEY = 3,	//黄闪按键
		ALLRED_KEY = 4,			//全红按键
		STEP_KEY = 5,			//步进按键
		EAST_KEY = 6,			//东方向通行按键
		SOUTH_KEY = 7,			//南方向通行按键
		WEST_KEY = 8,			//西方向通行按键
		NORTH_KEY = 9,			//北方向通行按键
		EWS_KEY = 10,			//东西直行按键
		SNS_KEY = 11,			//南北直行按键
		EWL_KEY = 12,			//东西左转按键
		SNL_KEY = 13,			//南北左转按键
		FUNCTION1_KEY = 14,		//功能键1
		FUNCTION2_KEY = 15,		//功能键2
		FUNCTION3_KEY = 16,		//功能键3
		FUNCTION4_KEY = 17,		//功能键4
	} KeyType;

	typedef enum 
	{
		RUNNING_LED = 1,
		CANRECV_LED = 2,
		CANSEND_LED = 3,
		GPS_LED = 4,
		RF_LED = 5,
		SIM_LED = 6,
		WIFI_LED = 7,
	} LedType;

	typedef enum
	{
		FRONT_DOOR = 0,
		BACK_DOOR = 1,
		KEYBOARD_DOOR = 2,
		POWER_DOOR = 3,
	} DoorType;

	typedef enum
	{
		NO_YELLOWFLASH = 1,		//未黄闪
		HAS_YELLOWFLASH = 2,	//已黄闪
		HAS_EXCEPTION = 3,		//有异常
	} YellowFlashBoardStatus;

	class device
	{
	private:
		int hikiofd;
		int gpsfd;
		int keyboardfd;
		int wirelessfd;
		int voltfd;
		int alarmfd;
		//int rs485fd1;
		std::map<std::string, hik::KeyType> button;	//存放按键值

		int uart232_config(int fd, int baudrate);

		KeyType find_key(const char *keystr);
	public:
		device();
		~device();

		void led_on(LedType type);

		void led_off(LedType type);

		std::bitset<4> get_door_status();	//4个开关门状态, 0:关门，1:开门

		std::bitset<16> get_ped_status();	//16路行人按键状态，0:按键未按下，1:按键按下

		//void set_output(bitset<8> &);	//设置8路输出

		YellowFlashBoardStatus yellowflash_board_status();		//获取黄闪板状态

		bool get_gps(int zone = 8);				

		KeyType read_keyboard();		//读取键盘板数据

		KeyType read_wireless();		//读取无线遥控器数据

		void feedback_keystatus(KeyType key);	//反馈按键状态

		unsigned short read_volt();			//读取电源板实际电压值，单位为V

		unsigned short read_temperature();	//读取温度值，单位为实际温度值放大100倍

		void enable_hard_yellowflash(bool on);		//开启主控输出硬黄闪信号

		bool wifi_exist();	//wifi模块是否存在

		bool sim_exist();	//4G模块是否存在
	};
}
