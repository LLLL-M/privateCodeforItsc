#pragma once 

#include <vector>
#include "thread.h"
#include "dev.h"
#include "faultcheck.h"
#include "sem.h"

#define TSC300_IMX6UL	//新版TSC300平台

namespace hik
{	
	typedef struct
	{
		uint32_t  checksum;	//4字节校验值，接管信息每字节相加之和(不包含接管信息头)
		uint16_t  arraynum;	//数组个数(即点灯状态变化个数)
		uint16_t  framenum;	//后续需要接收的接管信息帧个数
	} TakeOverInfoHead;

	#pragma pack(push, 1)  //1字节对齐，如下结构体共26字节
	typedef struct
	{
		uint8_t		cmd[MAX_CHANNEL_NUM / 2];	//24个字节共192bit点灯指令；（TSC300按照112bit按顺序接收）
		uint16_t	duration;	//点灯指令持续的时间，单位为秒
	} TakeOverInfo;

	typedef struct
	{
		uint16_t green;
		uint16_t red;
		uint16_t yellow;
	} ChannelCurVal;
	#pragma pack(pop)

	class device : public dev
	{
	private:
		int hikiofd;
		int gpsfd;
		int keyboardfd;
		int wirelessfd;
		char wirelessbuf[48];
		int voltfd;
		int alarmfd;
		int tempfd;
		//int rs485fd1;

		enum 
		{
			CHANNEL_PER_BOARD = 4, 		//每块板的通道个数
			MAX_BOARD_NUM = 7, 			//最大灯控板个数

			FIRST_GROUP = 16, 	//第1个can帧包含的灯组数目
			SECOND_GROUP = 12, 	//第2个can帧包含的灯组数目
		};
		int canfd;
		thread canrecv;				//can接受线程
		std::atomic_uint usedboard;	//标记所使用的灯控板
		std::vector<TakeOverInfo> takeover;
		bool cansndled;
		
		std::atomic<ChannelCurVal> curvals;		//通道电流值
		sem semForGetCur;			//获取通道电流的信号量

		std::array<FaultCheck, CHANNEL_PER_BOARD * MAX_BOARD_NUM> faultcheck;

		int open_uart232(const char *devname, int baudRate, int dataBits, int stopBits, char parity);

		void status2data(const ChannelArray &lamps, std::bitset<64> &lightdata1, std::bitset<64> &lightdata2);

		bool check(int board, uint8_t (&data)[8], bool curCheck);

		void can_recv();
		
		void send_checkinfo();

	public:
		device();

		~device();

		void ledctrl(LedType type, bool on);	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯

		std::bitset<4> get_door_status();	//4个开关门状态, 0:关门，1:开门

		std::bitset<16> get_ped_status();	//16路行人按键状态，0:按键未按下，1:按键按下

		//void set_output(bitset<8> &);	//设置8路输出

		YellowFlashBoardStatus yellowflash_board_status();		//获取黄闪板状态

		//每500ms读取一次
		bool get_gps(int zone = 8);

		//每300ms读取一次
		KeyType read_keyboard();		//读取键盘板数据

		KeyType read_wireless();		//读取无线遥控器数据

		void feedback_keystatus(KeyType key);	//反馈按键状态

		unsigned short read_volt();		//读取电源板实际电压值，单位为V

		unsigned short read_temperature();	//读取温度值，单位为实际温度值放大100倍

		void enable_hard_yellowflash(bool on);		//开启主控输出硬黄闪信号

		bool wifi_exist();	//wifi模块是否存在

		bool sim_exist();	//4G模块是否存在

		void light();

		std::vector<TakeOverInfo> & get_takeover_info() { return takeover; }

		void send_takeover();

		ChannelCurVal get_channel_cur(uint8_t channel);
	};
}
