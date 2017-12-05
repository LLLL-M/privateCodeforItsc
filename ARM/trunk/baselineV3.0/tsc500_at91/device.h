#pragma once

#include <atomic>
#include "thread.h"
#include "dev.h"

namespace hik
{
	class device : public dev
	{
	private:
		int hikiofd;
		int gpsfd;
		//int rs485fd1;
		bool pwmflag;
		volatile bool hardyellowflash;	//硬黄闪使能标志

		enum 
		{ 
			HALF_TIME = LIGHT_PER_SEC / 2,	//每秒点灯一半的次数，闪烁时使用
			FIRST_GROUP = 18, 	//第1个can帧包含的灯组数目
			SECOND_GROUP = 14 	//第2个can帧包含的灯组数目
		};
		int canfd;
		int count;
		thread canrecv;

		int Opentty(const char *devname, int baudRate, int dataBits, int stopBits, char parity);

		KeyType transfer(int arg);

		void can_recv();

		void status2data(const ChannelArray &lamps, std::bitset<64> &lightdata1, std::bitset<64> &lightdata2);

	public:
		device();
		
		~device();

		void ledctrl(LedType type, bool on);	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯

		std::bitset<16> get_ped_status();	//16路行人按键状态，0:按键未按下，1:按键按下

		//void set_output(bitset<8> &);	//设置8路输出

		bool get_gps(int zone = 8);		//获取gps信息

		KeyType read_keyboard();		//读取键盘板数据

		KeyType read_wireless();		//读取无线遥控器数据

		void feedback_keystatus(KeyType key);	//反馈按键状态

		void hard_yellowflash_heartbeat();

		void enable_hard_yellowflash(bool on);		//开启主控输出硬黄闪信号

		void light();
	};
}
