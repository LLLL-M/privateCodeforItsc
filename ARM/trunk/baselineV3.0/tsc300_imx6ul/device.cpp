#include <functional>
#include "linux.h"
#include "device.h"
#include "hik.h"

#define LED_OFFSET			127
#define DOOR_OFFSET			20	
#define PED_OFFSET			0	
#define VOLT_RATIO 			2883530

#define HYF_NL_GPIO_NUM		135
#define HYF_YL_GPIO_NUM		136

#define	UART_IOC_MAGIC  	'U' 
#define MISC_IOC_MAGIC   	'M'
#define GPIO_IOC_MAGIC    	'G'
#define	ALARM_IOC_MAGIC   	'A'
#define	IOCTL_INIT_UART232	0x3
#define HAL_SET_PWM         _IOW(MISC_IOC_MAGIC, 2, struct pwm*)
#define HAL_UART_CONFIG     _IOW(UART_IOC_MAGIC, 1, UART232CTL *)
#define HAL_GPIO_GET_VALUE  _IOR(GPIO_IOC_MAGIC, 3, struct gpio_ctrl*)
#define HAL_GPIO_SET_VALUE  _IOW(GPIO_IOC_MAGIC, 4, struct gpio_ctrl*)
#define HAL_ALARM_SET     	_IOW(ALARM_IOC_MAGIC, 1, struct alarm_ctrl *)
#define HAL_ALARM_GET     	_IOR(ALARM_IOC_MAGIC, 2, struct alarm_ctrl *)

#define WIRELESS_SEQ_LEN	9 	//无线遥控器序列号长度

namespace hik
{
	struct gpio_ctrl {
		unsigned int gpio;  ///< GPIO no
		unsigned int param; ///< set / get value / pinmux
	};

	struct pwm {
		int chan;
		int period;  ///< unit: ns
		int duty;
	};

	/*uart working mode*/
	typedef struct _uart232ctl{
		int     baudrate;
		int     data;
		int     stop;
		int     parity;
		int     flowcontrol;
	} UART232CTL;

	struct alarm_ctrl {
		unsigned int idx;
		unsigned int state;
		unsigned int direction; ///< alarm out or alarm in
	};

	int device::open_uart232(const char *devname, int baudRate, int dataBits, int stopBits, char parity)
	{
		int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
		if (fd == -1)
			return -1;

		UART232CTL ctrl;
		ctrl.baudrate = baudRate;
		ctrl.data = dataBits - 5;  		//8bit data: 3    7bit data: 2
		ctrl.stop = 0;  		// 1bit stop: 0    1.5bit: 1
		ctrl.parity = 0; 		//no parity: x0b  Odd: 01b        Even: 11b
		ctrl.flowcontrol = 0;
		if (-1 == ioctl(fd, HAL_UART_CONFIG, &ctrl))
		{
			ioctl(fd, IOCTL_INIT_UART232, &ctrl);
			close(fd);
			return -1;
		}
		return fd;
	}

	device::device()
	{
		//gpio init
		hikiofd = open("/dev/hikio", O_RDWR);
		//uart5(GPS) init
		gpsfd = open_uart232("/dev/uart5", 9600, 8, 1, 'N');
		//uart6(volt) init
		voltfd = open_uart232("/dev/uart6", 4800, 8, 1, 'N');
		//uart4(keyboard) init
		keyboardfd = open_uart232("/dev/uart4", 9600, 8, 1, 'N');
		write(keyboardfd, "Auto", 4);	//键盘板初始时点亮自动led
		//uart3(wireless) init
		wirelessfd = open_uart232("/dev/uart3", 9600, 8, 1, 'N');
		memset(wirelessbuf, 0, sizeof(wirelessbuf));
		//alarm init
		alarmfd = open("/dev/alarm", O_RDWR);
		//temperature init
		tempfd = open("/proc/debug/temp1", O_RDONLY);

		canfd = can_init(500000);
		canrecv.start(bind(&device::can_recv, ref(*this)));	//启动can接收线程
		cansndled = true;
	}

	device::~device()
	{
		if (hikiofd != -1)
			close(hikiofd);
		if (gpsfd != -1)
			close(gpsfd);
		if (keyboardfd != -1)
			close(keyboardfd);
		if (voltfd != -1)
			close(voltfd);
		if (alarmfd != -1)
			close(alarmfd);
		if (wirelessfd != -1)
			close(wirelessfd);
		if (tempfd != -1)
			close(tempfd);
		if (canfd != -1)
			close(canfd);
	}

	void device::ledctrl(LedType type, bool on)	//led控制，type为led类型，on=true时led亮灯，on=false时led灭灯
	{
		struct gpio_ctrl ctl = {(unsigned int)type + LED_OFFSET, on};
		if (hikiofd != -1)
			ioctl(hikiofd, HAL_GPIO_SET_VALUE, &ctl);
	}

	std::bitset<4> device::get_door_status()	//4个开关门状态, 0:关门，1:开门
	{	
		std::bitset<4> doorstatus(0);
		struct alarm_ctrl ctrl = {0, 0, 1};	//ctrl.direction = 1; 1 报警输入 0 报警输出
		/*报警输入总共24路，把最后的4路做为几个开关门的输入*/
		for (int i = 0 ; i < 4 ; i++)
		{
			ctrl.idx = i + DOOR_OFFSET;//范围0~23
			if (0 == ioctl(alarmfd, HAL_ALARM_GET , &ctrl))
				doorstatus[i] = ctrl.state;
		}
	    return doorstatus;
	}

	std::bitset<16> device::get_ped_status()	//16路行人按键状态，0:按键未按下，1:按键按下
	{	
		std::bitset<16> pedstatus(0);
		struct alarm_ctrl ctrl = {0, 0, 1};	//ctrl.direction = 1; 1 报警输入 0 报警输出
		/*报警输入总共24路，把前面的16路做为行人按钮的输入*/
		for(int i = 0 ; i < 16 ; i++)
		{
			ctrl.idx = i + PED_OFFSET;//范围0~23
			if (0 == ioctl(alarmfd, HAL_ALARM_GET , &ctrl))
				pedstatus[i] = ctrl.state;
		}
	    return pedstatus;
	}

	//void device::set_output(bitset<8> &);	//设置8路输出

	YellowFlashBoardStatus device::yellowflash_board_status()		//获取黄闪板状态
	{
		bool normal_light, yellow_light;
		struct gpio_ctrl ctl = {HYF_NL_GPIO_NUM, 0};
		
	    ioctl(hikiofd, HAL_GPIO_GET_VALUE, &ctl);
	    normal_light = ctl.param;

	    ctl.gpio = HYF_YL_GPIO_NUM;
	    ctl.param = 0;
	    ioctl(hikiofd, HAL_GPIO_GET_VALUE, &ctl);
		yellow_light = ctl.param;


	    if ((normal_light && yellow_light) || (~normal_light && ~yellow_light))
	    	return HAS_EXCEPTION;	//如果正常管脚和黄闪管脚都为1则黄闪板故障，都为0属于严重故障，因为硬件不存在此状态
	    else if (!normal_light && yellow_light)
	    	return NO_YELLOWFLASH;	//如果正常管脚为0且黄闪管脚为1则说明由主控控制输出
	    else
	    	return HAS_YELLOWFLASH;	//如果正常管脚为1且黄闪管脚为0则为硬黄闪控制
	}

	//每500ms读取一次
	bool device::get_gps(int zone/* = 8*/)
	{
		return parse_gps(gpsfd, "$GNRMC", zone, "/dev/rtc");
	}			

	//每300ms读取一次
	KeyType device::read_keyboard()		//读取键盘板数据
	{
		char keystr[40] = {0};
		if (keyboardfd != -1 && read(keyboardfd, keystr, 40) > 0)
			return find_key(keystr);
		return NONE_KEY;
	}

	KeyType device::read_wireless()		//读取无线遥控器数据
	{		
		int n = read(wirelessfd, wirelessbuf, sizeof(wirelessbuf));
		if (n < WIRELESS_SEQ_LEN)	//如果未读取到无线按键的序列号
			return NONE_KEY;
		if (n == 16 && wirelessbuf[15] != '$')
		{	//如果读到16字节且未读取到结尾则说明按键信息未读取完整，需要再读取一次
			usleep(20000);
			read(wirelessfd, wirelessbuf + 16, sizeof(wirelessbuf) - 16);
		}

		char *end = strchr(wirelessbuf, '$');
		if (end == nullptr)
			return NONE_KEY;
		*end = '\0';
		//INFO("recv wirelessbuf is %s", wirelessbuf);
		/*此处需要根据配置进行无线遥控器序列号的判断*/
		return find_key(wirelessbuf + WIRELESS_SEQ_LEN);	//前面9个字节是无线按键序列号
	}

	void device::feedback_keystatus(KeyType type)	//反馈按键状态
	{
		for (auto &it : button)
		{
			if (it.second == type)
			{	
				//wireless status feedback
				size_t offset = WIRELESS_SEQ_LEN;
				strncpy(wirelessbuf + WIRELESS_SEQ_LEN, it.first.c_str(), it.first.length());
				offset += it.first.length();
				wirelessbuf[offset++] = '$';
				//wirelessbuf[offset] = '\0';
				//INFO("feedback wirelessbuf is %s", wirelessbuf);
				write(wirelessfd, wirelessbuf, offset);
				//keyboard status feedback
				write(keyboardfd, it.first.c_str(), it.first.length());
				break;
			}
		}
	}

	unsigned short device::read_volt()		//读取电源板实际电压值，单位为V
	{
		if (voltfd == -1)
			return 0;
		unsigned int value = 0x0735;
		write(voltfd, &value, 4);
		usleep(500);	//延时500us再去读取电压值
		value = 0;
		read(voltfd, &value, 4);
		return (value & 0xffffff) * 220 / VOLT_RATIO;
	}

	unsigned short device::read_temperature()	//读取温度值，单位为实际温度值放大100倍
	{
		char buf[5] = {0};  
		if (4 == read(tempfd, buf, 4))
			return atoi(buf);
		else
			return 0;
	}

	void device::enable_hard_yellowflash(bool on)		//开启主控输出硬黄闪信号	
	{
		if (hikiofd != -1)
		{
			struct pwm pwm_test = {0, 2000000, on ? 1000000 : 2000000};
			ioctl(hikiofd, HAL_SET_PWM, &pwm_test); 
		}
	}

	bool device::wifi_exist()	//wifi模块是否存在
	{
		return hik::dir_exist("/sys/bus/usb/devices/1-1.4");
	}

	bool device::sim_exist()	//4G模块是否存在
	{
		return hik::dir_exist("/sys/bus/usb/devices/1-1.3");
	}

}
