#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <functional>
#include "io_ioctl.h"
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
		hikiofd = open("/dev/gpio", O_RDWR);
		keyboardfd = Opentty("/dev/ttyS3", 9600, 8, 1, 'N');	//打开侧面板串口
		gpsfd = Opentty("/dev/ttyS6", 9600, 8, 1, 'N');
		pwmflag = false;
		hardyellowflash = false;
		
		canfd = can_init(500000);
		count = 0;
		canrecv.start(std::bind(&device::can_recv, std::ref(*this)));	//启动can接收线程
		feedback_keystatus(AUTO_KEY);
	}
	
	device::~device()
	{
		if (hikiofd != -1)
			close(hikiofd);
		if (gpsfd != -1)
			close(gpsfd);
		if (keyboardfd != -1)
			close(keyboardfd);
		if (canfd != -1)
			close(canfd);
	}

	void device::ledctrl(LedType type, bool on)	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯
	{
		int arg = 0;
		switch (type)
		{
			case GPS_LED: arg = on ? ARG_GPS_LED_ON : ARG_GPS_LED_OFF; break;
			case RUNNING_LED: arg = on ? ARG_SYSTEM_RUNNING_LED_ON : ARG_SYSTEM_RUNNING_LED_OFF; break;
			default: return;
		}
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg);
	}

	std::bitset<16> device::get_ped_status()	//16路行人按键状态，0:按键未按下，1:按键按下
	{
		std::bitset<16> value(0);
		int arg = 0;
		for (int i = 1; i <= 8; i++)
		{
			arg = SET_ARG(i, 0);
			ioctl(hikiofd, IO_GET_PIN_STATUS, &arg);
			value[i - 1] = (arg & 0x1);
		}
		return value;
	}

	//void set_output(bitset<8> &);	//设置8路输出

	bool device::get_gps(int zone/* = 8*/)	//获取gps信息
	{
		return parse_gps(gpsfd, "$GPRMC", zone, "/dev/rtc0");
	}

	KeyType device::read_keyboard()		//读取键盘板数据
	{
		int arg = 0;
		ioctl(hikiofd, IO_GET_BUTTON_STATUS, &arg);
		if (arg > 0)
			return transfer(arg);
		else
		{
			char buf[32] = {0};
			read(keyboardfd, buf, sizeof(buf));
			return find_key(buf);
		}
	}

	KeyType device::read_wireless()		//读取无线遥控器数据
	{
		int arg = 0;
		ioctl(hikiofd, IO_GET_WIRELESS_STATUS, &arg);
		return transfer(arg);
	}

	void device::feedback_keystatus(KeyType key)	//反馈按键状态
	{
		if (key == NONE_KEY)
			return;
		if (keyboardfd != -1)
		{	//反馈通过串口连接的键盘板按键led状态
			char buf[8] = "Button";
			buf[6] = (char)key;
			write(keyboardfd, buf, 7);
		}
		/*TSC300还有前面板led指示灯需要通过can通信去点亮*/
		struct can_frame frame = {0x110, 1, {0}};
		int arg1 = ARG_AUTO_LED_OFF;
		int arg2 = ARG_MANUAL_LED_ON;
		int arg3 = ARG_YELLOW_BLINK_LED_OFF;
		int arg4 = ARG_ALL_RED_LED_OFF;
		int arg5 = ARG_STEP_LED_OFF;
		switch (key)
		{
			case AUTO_KEY: arg1 = ARG_AUTO_LED_ON; arg2 = ARG_MANUAL_LED_OFF; frame.data[0] = 0x1; break;
			case MANUAL_KEY: /*arg2 = ARG_MANUAL_LED_ON;*/ frame.data[0] = 0x2; break;
			case YELLOWBLINK_KEY: arg3 = ARG_YELLOW_BLINK_LED_ON; frame.data[0] = 0x4; break;
			case ALLRED_KEY: arg4 = ARG_ALL_RED_LED_ON; frame.data[0] = 0x8; break;
			case STEP_KEY: arg5 = ARG_STEP_LED_ON; frame.data[0] = 0x10; break;
			default: return;
		}
		//反馈之前旧版通过io连接的键盘板led状态
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg1);
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg2);
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg3);
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg4);
		ioctl(hikiofd, IO_SET_PIN_STATUS, &arg5);
		//反馈前面板按键led状态
		write(canfd, &frame, sizeof(struct can_frame));
	}

	void device::hard_yellowflash_heartbeat()
	{
		if (!hardyellowflash)
		{
			int arg = pwmflag ? ARG_YF_CTRL_HIGH : ARG_YF_CTRL_LOW;
			ioctl(hikiofd, IO_SET_PIN_STATUS, &arg);
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

	void device::can_recv()
	{
		struct can_frame frame;

		while (1)
		{
			if (read(canfd, &frame, sizeof(struct can_frame)) > 0)
			{
				if (GET_BIT(frame.can_id, 15) == 1 && frame.can_dlc == 4)
				{	//灯控板反馈的电流电压信息
				}
				else if(frame.can_id == 0x201)//车检板信息反馈
				{	//车检板反馈的过车状态
				}
				else if (frame.can_id == 0x401 && frame.can_dlc == 1)
				{	//前面板5个按键状态
					int status = frame.data[0] & 0x1f;
					if (status != 0)
						ioctl(hikiofd, IO_SET_BUTTON_STATUS, &status);
				}
			}
		}
	}

	void device::light()
	{
		struct can_frame frame1 = {0x101, 7, {0}};	//第一帧
		struct can_frame frame2 = {0x101, 4, {0}};	//第二帧
		uint64_t *data1 = (uint64_t *)frame1.data;
		uint64_t *data2 = (uint64_t *)frame2.data;
		std::bitset<64> lightdata1(0x1);//第1个can帧的数据部分首字节为1
		std::bitset<64> lightdata2(0x3);//第2个can帧的数据部分首字节为3

		status2data(cache.load(), lightdata1, lightdata2);

		*data1 = lightdata1.to_ullong();
		write(canfd, &frame1, sizeof(struct can_frame));

		*data2 = lightdata2.to_ullong();
		write(canfd, &frame2, sizeof(struct can_frame));

		if (++count >= LIGHT_PER_SEC)
			count = 0;
	}
}
