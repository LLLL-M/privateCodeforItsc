#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/can.h>
#include <functional>
#include <sys/ioctl.h>
#include "CPLD.h"
#include "device.h"
#include "hik.h"

namespace hik
{
	int device::Opentty(const char *devname, int baudRate, int dataBits, int stopBits, char parity)
	{
		struct termios options;
		int fd = -1;
		
		fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd == -1)
		{
			//ERR("open %s fail!\n", devname);
			return -1;
		}
		if (tcgetattr(fd, &options) != 0) 
		{  
	        //ERR("get serial options error\n");
			close(fd);
			return -1; 
	    }
		//忽略CR回车符
		options.c_iflag |= IGNCR;
		//使用串口非规范模式
		options.c_lflag &= ~ICANON;
		//设置本地接收模式
		options.c_cflag |= CLOCAL | CREAD;
		//设置波特率
		switch (baudRate) 
	 	{ 
	    	case 2400: 
	    		cfsetispeed(&options, B2400); 
	    		cfsetospeed(&options, B2400); 
	    		break; 
	    	case 4800: 
	    		cfsetispeed(&options, B4800); 
	    		cfsetospeed(&options, B4800); 
	    		break; 
	    	case 9600: 
	    		cfsetispeed(&options, B9600); 
	    		cfsetospeed(&options, B9600); 
	    		break; 
	    	case 115200: 
	    		cfsetispeed(&options, B115200); 
	    		cfsetospeed(&options, B115200); 
	    		break; 
	    	case 460800: 
	    		cfsetispeed(&options, B460800); 
	    		cfsetospeed(&options, B460800); 
	    		break; 
	    	default: 
	    		cfsetispeed(&options, B9600); 
	    		cfsetospeed(&options, B9600); 
	    		break; 
	 	}
		//设置数据位
		options.c_cflag &= ~CSIZE;
	 	switch (dataBits) 
	 	{
			case 5: options.c_cflag |= CS5; break;
	    	case 6: options.c_cflag |= CS6; break;
	    	case 7: options.c_cflag |= CS7; break;
	    	case 8: options.c_cflag |= CS8; break;
	 	}
		//设置停止位
		switch (stopBits)
		{
			case 1: options.c_cflag &= ~CSTOPB; break;
			case 2: options.c_cflag |= CSTOPB; break;
		}
		//设置奇偶校验位 
	 	switch (parity) 
	 	{ 
	    	case 'O': //奇数 
	    		options.c_cflag |= PARENB; 
	    		options.c_cflag |= PARODD; 
	    		options.c_iflag |= (INPCK | ISTRIP); 
	    		break; 
	    	case 'E': //偶数 
	    		options.c_iflag |= (INPCK | ISTRIP); 
	    		options.c_cflag |= PARENB; 
	    		options.c_cflag &= ~PARODD; 
	    		break; 
	    	case 'N':  //无奇偶校验位 
	    		options.c_cflag &= ~PARENB; 
	    		break; 
	 	}
		/*设置超时等待时间和最小接收字符*/ 
		options.c_cc[VTIME] = 0;
		options.c_cc[VMIN] = 0;
		/*处理未接收字符*/ 
	 	tcflush(fd, TCIFLUSH);
		/*激活新配置*/ 
		if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
		{ 
			//ERR("serial attr set failed!\n");
			close(fd);
			return -1;
		}
		else
		{
			//INFO("serial attr set successful!\n");
			return fd;
		}	
	}

	KeyType device::transfer(int arg)
	{
		if (arg == 0x1)
			return AUTO_KEY;
		else if (arg == 0x2)
			return MANUAL_KEY;
		else if (arg == 0x4)
			return YELLOWBLINK_KEY;
		else if (arg == 0x8)
			return ALLRED_KEY;
		else if (arg == 0x10)
			return STEP_KEY;
		else
			return NONE_KEY;
	}

	device::device()
	{
		hikiofd = open("/dev/CPLD_IO", O_RDWR);
		gpsfd = Opentty("/dev/ttyS6", 9600, 8, 1, 'N');
		pwmflag = false;
		hardyellowflash = false;

		canfd = can_init(600000);
		count = 0;
		canrecv.start(bind(&device::can_recv, ref(*this)));	//启动can接收线程
		feedback_keystatus(AUTO_KEY);
	}
	
	device::~device() 
	{
		if (hikiofd != -1)
			close(hikiofd);
		if (gpsfd != -1)
			close(gpsfd);
		if (canfd != -1)
			close(canfd);
	}

	void device::ledctrl(LedType type, bool on)	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯
	{
		int cmd = 0;
		switch (type)
		{
			case GPS_LED: cmd = on ? LED_OUTPUT2_1 : LED_OUTPUT2_0; break;
			case RUNNING_LED: cmd = on ? LED_OUTPUT1_1 : LED_OUTPUT1_0; break;
			default: return;
		}
		ioctl(hikiofd, cmd);
	}

	std::bitset<16> device::get_ped_status()	//16路行人按键状态，0:按键未按下，1:按键按下
	{
		int arg = 0xff;
		ioctl(hikiofd, IO_INPUT1_TO_INPUT8, &arg);
		return (~arg) & 0xff;;
	}

	//void set_output(bitset<8> &);	//设置8路输出

	bool device::get_gps(int zone/* = 8*/)	//获取gps信息
	{
		return parse_gps(gpsfd, "$GPRMC", zone, "/dev/rtc0");
	}

	KeyType device::read_keyboard()		//读取键盘板数据
	{
		int arg = 1;
		ioctl(hikiofd, KEYBOARD_INPUT3, &arg);	//获取自动按键状态
		if (arg == 0)
			return AUTO_KEY;
		ioctl(hikiofd, KEYBOARD_INPUT4, &arg);	//获取手动按键状态
		if (arg == 0)
			return MANUAL_KEY;
		ioctl(hikiofd, KEYBOARD_INPUT2, &arg);	//获取黄闪按键状态
		if (arg == 0)
			return YELLOWBLINK_KEY;
		ioctl(hikiofd, KEYBOARD_INPUT5, &arg);	//获取全红按键状态
		if (arg == 0)
			return ALLRED_KEY;
		ioctl(hikiofd, KEYBOARD_INPUT1, &arg);	//获取步进按键状态
		if (arg == 0)
			return STEP_KEY;
		return NONE_KEY;
	}

	KeyType device::read_wireless()		//读取无线遥控器数据
	{
		int arg = 0x1f;
		ioctl(hikiofd, IO_INPUT9_TO_INPUT16, &arg);
		return transfer((~arg) & 0x1f);
	}

	void device::feedback_keystatus(KeyType key)	//反馈按键状态
	{
		int cmd1 = KEYBOARD_OUTPUT3_0;
		int cmd2 = KEYBOARD_OUTPUT4_1;
		int cmd3 = KEYBOARD_OUTPUT2_0;
		int cmd4 = KEYBOARD_OUTPUT5_0;
		int cmd5 = KEYBOARD_OUTPUT1_0;
		switch (key)
		{
			case AUTO_KEY: cmd1 = KEYBOARD_OUTPUT3_1; cmd2 = KEYBOARD_OUTPUT4_0; break;
			case MANUAL_KEY: cmd2 = KEYBOARD_OUTPUT4_1; break;
			case YELLOWBLINK_KEY: cmd3 = KEYBOARD_OUTPUT2_1; break;
			case ALLRED_KEY: cmd4 = KEYBOARD_OUTPUT5_1; break;
			case STEP_KEY: cmd5 = KEYBOARD_OUTPUT1_1; break;
			default: return;
		}
		ioctl(hikiofd, cmd1);
		ioctl(hikiofd, cmd2);
		ioctl(hikiofd, cmd3);
		ioctl(hikiofd, cmd4);
		ioctl(hikiofd, cmd5);
	}

	void device::hard_yellowflash_heartbeat()
	{
		if (!hardyellowflash)
		{
			if (pwmflag)
			{
				ioctl(hikiofd, YELLOW_CONTROL_OUTPUT1_1);
				ioctl(hikiofd, YELLOW_CONTROL_OUTPUT2_1);
			}
			else
			{
				ioctl(hikiofd, YELLOW_CONTROL_OUTPUT1_0);
				ioctl(hikiofd, YELLOW_CONTROL_OUTPUT2_0);
			}
			pwmflag = !pwmflag;
		}
	}

	void device::enable_hard_yellowflash(bool on)		//开启主控输出硬黄闪信号
	{
		hardyellowflash = on;
	}

	void device::status2data(const ChannelArray &lamps, std::bitset<64> &lightdata1, std::bitset<64> &lightdata2)
	{
		auto Value = [this](TscStatus st)->char{
			switch (st)
			{
				case GREEN: return 0x1;
				case RED: return 0x2;
				case YELLOW: return 0x4;
				case GREEN_BLINK: return count >= HALF_TIME ? 0x1 : 0;
				case RED_BLINK: return count >= HALF_TIME ? 0x2 : 0;
				case YELLOW_BLINK: return count >= HALF_TIME ? 0x4 : 0;
				case ALLRED: return 0x2;
				case TURN_OFF: return 0;
				case RED_YELLOW: return 0x6;
				case RED_GREEN: return 0x3;
				case YELLOW_GREEN: return 0x5;
				case RED_YELLOW_GREEN: return 0x7;
				default: break;
			}
			return 0;
		};

		int i, pos = 8;	//偏移8bit即偏移首字节
		for (i = 0; i < FIRST_GROUP; i++)
		{
			auto v = Value(lamps[i].status);
			lightdata1[pos++] = v & 0x1;
			lightdata1[pos++] = v & 0x2;
			lightdata1[pos++] = v & 0x4;
		}

		pos = 8;	//偏移8bit即偏移首字节
		for (; i < FIRST_GROUP + SECOND_GROUP; i++)
		{
			auto v = Value(lamps[i].status);
			lightdata2[pos++] = v & 0x1;
			lightdata2[pos++] = v & 0x2;
			lightdata2[pos++] = v & 0x4;
		}
	}

	void device::light()
	{
		struct can_frame frame1 = {0x101, 8, {0}};	//第一帧
		struct can_frame frame2 = {0x101, 7, {0}};	//第二帧
		uint64_t *data1 = (uint64_t *)frame1.data;
		uint64_t *data2 = (uint64_t *)frame2.data;
		std::bitset<64> lightdata1(0x1);//第1个can帧的数据部分首字节为1
		std::bitset<64> lightdata2(0x2);//第2个can帧的数据部分首字节为2

		status2data(cache.load(), lightdata1, lightdata2);

		*data1 = lightdata1.to_ullong();
		write(canfd, &frame1, sizeof(struct can_frame));

		*data2 = lightdata2.to_ullong();
		write(canfd, &frame2, sizeof(struct can_frame));

		if (++count >= LIGHT_PER_SEC)
			count = 0;
	}

	void device::can_recv()
	{
		struct can_frame frame;

		while (1)
		{
			if (read(canfd, &frame, sizeof(struct can_frame)) > 0)
			{
				if ((frame.can_id & 0x3fffff) > 0x200000 && GET_BIT(frame.can_id, 21) == 1)
				{	//灯控板反馈的电流电压信息
				}
				else if((frame.can_id == 0x201) || (frame.can_id == 0x202))//车检板信息反馈
				{	//车检板反馈的过车状态
				}
			}
		}
	}
}
