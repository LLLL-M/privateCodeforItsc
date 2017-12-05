#pragma once

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义

#include <bitset>
#include <string>
#include <map>
#include <atomic>
#include "channel.h"

#define LIGHT_PER_SEC		4 	//每秒点灯次数,此值可改为2或8

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

	class dev
	{
	private:
		
	public:
		dev()
		{
			button["Auto"] = AUTO_KEY;		//这几个按键对应的值必须是1,2,3,4,5，刚好与前面板按键值一样
			button["Manual"] = MANUAL_KEY;
			button["YellowBlink"] = YELLOWBLINK_KEY;
			button["AllRed"] = ALLRED_KEY;
			button["StepByStep"] = STEP_KEY;
			button["East"] = EAST_KEY;
			button["South"] = SOUTH_KEY;
			button["West"] = WEST_KEY;
			button["North"] = NORTH_KEY;
			button["EastAndWestStraight"] = EWS_KEY;
			button["SouthAndNorthStraight"] = SNS_KEY;
			button["EastAndWestLeft"] = EWL_KEY;
			button["SouthAndNorthLeft"] = SNL_KEY;
		}

		virtual ~dev() {}

		virtual void ledctrl(LedType type, bool on) = 0;	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯

		virtual std::bitset<4> get_door_status() {return 0;}	//4个开关门状态, 0:关门，1:开门

		virtual std::bitset<16> get_ped_status() = 0;	//16路行人按键状态，0:按键未按下，1:按键按下

		//void set_output(bitset<8> &);	//设置8路输出

		virtual YellowFlashBoardStatus yellowflash_board_status() {return NO_YELLOWFLASH;}		//获取黄闪板状态

		virtual bool get_gps(int zone = 8) = 0;				

		virtual KeyType read_keyboard() = 0;		//读取键盘板数据

		virtual KeyType read_wireless() = 0;		//读取无线遥控器数据

		virtual void feedback_keystatus(KeyType key) = 0;	//反馈按键状态

		virtual unsigned short read_volt() {return 0;}			//读取电源板实际电压值，单位为V

		virtual unsigned short read_temperature() {return 0;}	//读取温度值，单位为实际温度值放大100倍

		virtual void hard_yellowflash_heartbeat() {}			//硬黄闪心跳信号
		virtual void enable_hard_yellowflash(bool on) = 0;		//开启主控输出硬黄闪信号

		virtual bool wifi_exist() {return false;}	//wifi模块是否存在

		virtual bool sim_exist() {return false;}	//4G模块是否存在

		virtual void light() {}			//点灯操作

		void setlamps(UInt8 ctrlMode);	//设置灯状态

		void setlamps(const ChannelArray &lamps) {cache = lamps;}	//设置灯状态
		
	protected:
		std::map<std::string, hik::KeyType> button;	//存放按键值
		std::atomic<ChannelArray> cache;
		
		KeyType find_key(const char *keystr)
		{
			if (keystr == nullptr)
				return NONE_KEY;
			auto it = button.find(keystr);
			if (it == button.end())
				return NONE_KEY;
			return it->second;
		}

		bool parse_gps(int gpsfd, const char *headname, int zone, const char *rtcname);

		int can_init(unsigned int bitrate, const char *canname = "can0");
	};
}

#endif
