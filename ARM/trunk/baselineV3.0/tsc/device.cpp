#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <fstream>
#include "device.h"
#include "linux.h"

#define LED_OFFSET			127
#define DOOR_OFFSET			0	
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
#define HAL_ALARM_SET          _IOW(ALARM_IOC_MAGIC, 1, struct alarm_ctrl *)
#define HAL_ALARM_GET          _IOR(ALARM_IOC_MAGIC, 2, struct alarm_ctrl *)

#define BUF_SIZE       		128	


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

	device::device()
	{
		//gpio init
		hikiofd = open("/dev/hikio", O_RDWR);
	    if(hikiofd < 0) {
	    	printf("Open hikio device error!\n");
	    }
	#if 1
		gpsfd = -1;
	#else
	    //uart5(gps) init
	    if((gpsfd = open("/dev/uart5", O_RDWR | O_NOCTTY | O_NDELAY))<0)
		{
			printf("gps cannot open!\n");
		}
		uart232_config(gpsfd, 9600);
	#endif
		//uart6(volt) init
		if((voltfd = open("/dev/uart6", O_RDWR | O_NOCTTY | O_NDELAY))<0)
		{
			printf("volt cannot open!\n");
		}
		uart232_config(voltfd, 4800);
		//uart4(keyboard) init
		if((keyboardfd = open("/dev/uart4", O_RDWR | O_NOCTTY | O_NDELAY))<0)
		{
			printf("keyboard cannot open!\n");
		}
		uart232_config(keyboardfd, 9600);
		//uart3(wireless) init
		if((wirelessfd = open("/dev/uart3", O_RDWR | O_NOCTTY | O_NDELAY))<0)
		{
			printf("wireless cannot open!\n");
		}
		uart232_config(wirelessfd, 9600);
		//alarm init
		alarmfd = open("/dev/alarm", O_RDWR);
	    if(alarmfd < 0) {
	    	printf("Open alarm device error!\n");
	    }
	    button["Auto"] = AUTO_KEY;
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

	device::~device()
	{
		close(hikiofd);
		close(gpsfd);
		close(voltfd);
		close(alarmfd);
		close(keyboardfd);
		close(wirelessfd);
	}

	int device::uart232_config(int fd, int baudrate)
	{
		int ret;
		UART232CTL ctrl;
		ctrl.baudrate = baudrate;
		ctrl.data = 3;  		//8bit data: 3    7bit data: 2
		ctrl.stop = 0;  		// 1bit stop: 0    1.5bit: 1
		ctrl.parity = 0; 		//no parity: x0b  Odd: 01b        Even: 11b
		ctrl.flowcontrol = 0;

		ret = ioctl(fd, HAL_UART_CONFIG, &ctrl);
		if (ret) {
			perror("ioctl error:");
			printf("unknow HAL_UART_CONFIG\n");
			ret = ioctl(fd, IOCTL_INIT_UART232, &ctrl);	
			if (ret) printf("unknow IOCTL_INIT_UART232\n");
		}

		return ret;
	}

	inline KeyType device::find_key(const char *keystr)
	{
		if (keystr == nullptr)
			return NONE_KEY;
		auto it = button.find(keystr);
		if (it == button.end())
			return NONE_KEY;
		return it->second;
	}

	void device::led_on(LedType type)
	{
		int ret;	
		struct gpio_ctrl ctl = {(unsigned int)type + LED_OFFSET, 1};
		
	    ret = ioctl(hikiofd, HAL_GPIO_SET_VALUE, &ctl);
	    if(ret) {  
	        printf("ioctl HAL_GPIO_SET_VALUE failed!\n");
	    }
	    return;
	}

	void device::led_off(LedType type)
	{
		int ret;	
		struct gpio_ctrl ctl = {(unsigned int)type + LED_OFFSET, 0};
		
	    ret = ioctl(hikiofd, HAL_GPIO_SET_VALUE, &ctl);
	    if(ret) {  
	        printf("ioctl HAL_GPIO_SET_VALUE failed!\n");
	    }
	    return;
	}

	std::bitset<4> device::get_door_status()
	{
		int ret;	
		std::bitset<4> doorstatus;
		struct alarm_ctrl ctrl = {0};
		ctrl.direction = 1;// 1 报警输入 0 报警输出
				
		for(int i = 0 ; i < 4 ; i++)
		{
			ctrl.idx = i + DOOR_OFFSET;//范围0~23
		
			ret = ioctl(alarmfd, HAL_ALARM_GET , &ctrl);
			if(ret) {
				printf("ioctl HAL_ALARM_GET failed!\n");
			}
			doorstatus[i] = ctrl.state;
		}
	    return doorstatus;
	}

	std::bitset<16> device::get_ped_status()
	{
		int ret;	
		std::bitset<16> pedstatus;
		struct alarm_ctrl ctrl = {0};
		ctrl.direction = 1;// 1 报警输入 0 报警输出
				
		for(int i = 0 ; i < 16 ; i++)
		{
			ctrl.idx = i + PED_OFFSET;//范围0~23
		
			ret = ioctl(alarmfd, HAL_ALARM_GET , &ctrl);
			if(ret) {
				printf("ioctl HAL_ALARM_GET failed!\n");
			}
			pedstatus[i] = ctrl.state;
		}
	    return pedstatus;
	}

	YellowFlashBoardStatus device::yellowflash_board_status()
	{
		int ret;	
		bool normal_light,yellow_light;
		struct gpio_ctrl ctl = {HYF_NL_GPIO_NUM, 0};
		
	    ret = ioctl(hikiofd, HAL_GPIO_GET_VALUE, &ctl);
	    if(ret) {  
	        printf("ioctl HAL_GPIO_GET_VALUE failed!\n");
	    }
	    normal_light = ctl.param;

	    ctl.gpio = HYF_YL_GPIO_NUM;
	    ctl.param = 0;
		
	    ret = ioctl(hikiofd, HAL_GPIO_GET_VALUE, &ctl);
	    if(ret) {  
	        printf("ioctl HAL_GPIO_GET_VALUE failed!\n");
	    }
	    yellow_light = ctl.param;

	    if ((normal_light && yellow_light) || (~normal_light && ~yellow_light))
	    	return HAS_EXCEPTION;
	    else if (yellow_light)
	    	return NO_YELLOWFLASH;
	    else
	    	return HAS_YELLOWFLASH;
	}

	bool device::get_gps(int zone)
	{
		int i = 5;  //read 5 times to ensure the GNRMC message can be fully read
		int ret;
		
		char buf_r[BUF_SIZE];
		std::size_t start;
		std::string gpsinfo;
		std::string sub;

		unsigned int curTime, startTime;
		startTime = (unsigned int)time(nullptr);

	    while(i) {  
	    	memset(buf_r, 0, BUF_SIZE);
	        curTime = time(nullptr);
	        if( abs(curTime - startTime) >= 2) 
	        {
	        	printf("The gps module isn't insert!\n");
	        	return false;
	        }
	        ret = read(gpsfd, buf_r, BUF_SIZE);  
	  
	        if(ret == 0)  
	        {
	        	usleep(200 * 1000);
	            continue;  
	        }
	        buf_r[ret]='\0';  
	  
	        gpsinfo+= buf_r;

	        start = gpsinfo.find("$GNRMC");

	        if (start != std::string::npos) 
	        {
	        	i--;  	
	        }

	        if (i == 0)
	        {
	   		 	sub = gpsinfo.substr(start);
	       		//printf("%c\n",sub[18] ); 	
	        }
	    }

	    if(sub[18] == 'V')
	    {
	    	printf("The Gps cannot receive signal !\n");
	    	return false;
	    }
	    else if (sub[18] == 'A')
	    {
	    	struct tm now;
		    memset(&now, 0, sizeof(tm));

		    now.tm_sec = std::stoi(sub.substr(11,2));
		    now.tm_min = std::stoi(sub.substr(9,2));
		    now.tm_hour = std::stoi(sub.substr(7,2));

		    ret = 8;
	     	std::size_t found = sub.find_first_of(",");
		    while(ret--)
		    {
		    	sub[found] = '|';
		    	found = sub.find_first_of(",");
		    }
		    
		    now.tm_mday = std::stoi(sub.substr(found + 1,2));
		    now.tm_mon = std::stoi(sub.substr(found + 3,2)) - 1;
		    now.tm_year = std::stoi("20" + sub.substr(found + 5,2)) - 1900;

		    time_t t = mktime(&now);  

		   /* printf("UTC:%d\n",t);

		    std::string hour = sub.substr(7,2);
		    std::string min = sub.substr(9,2);
		    std::string sec = sub.substr(11,2);

   	    	std::string year = "20" + sub.substr(found + 5,2);
		    std::string month = sub.substr(found + 3,2);
		    std::string day = sub.substr(found + 1,2);

		    printf("year: %s, month: %s, day: %s \nhour: %s, min: %s, sec: %s\n", 
		    		year.data(),month.data(),day.data(), hour.data(), min.data(), sec.data());*/
			
			#if defined(__linux__) && defined(__arm__)
			struct timeval tv;
			tv.tv_sec = t + zone * 3600;
			tv.tv_usec = 0;
			if (abs(time(nullptr) - tv.tv_sec) > 1 )
			{
				settimeofday(&tv, nullptr);
				hik::rtc rtc;
				rtc.set(tv.tv_sec);
			}
			#endif
			printf("Set Time succeed!\n");
	    }
		return true;
	}

	KeyType device::read_keyboard()
	{
		char keystr[40] = {0};
		if (read(keyboardfd, keystr, 40) > 0)
			return find_key(keystr);
		return NONE_KEY;
	}

	KeyType device::read_wireless()
	{
		char keystr[40] = {0};
		if (read(wirelessfd, keystr, 40) > 0)
			return find_key(keystr);
		return NONE_KEY;
	}

	void device::feedback_keystatus(KeyType keystatus)
	{
		for (auto &it : button)
		{
			if (it.second == keystatus)
			{
#if 0
				write(keyboardfd, it.first, it.first.size());
				write(wirelessfd, it.first, it.first.size());
#else
				char buf[8] = "Button";
				buf[6] = (char)it.second;
				write(keyboardfd, buf, 7);
#endif
				break;
			}
		}
	}

	unsigned short device::read_volt()
	{
		unsigned int curTime, startTime;
		startTime = (unsigned int)time(nullptr);

		char buf_w[4]; 
		memset(buf_w, 0 ,4);
		sprintf(buf_w,"%c%c", 0x35,0x07);

		int written, n;
		char one,two,three;

		do
		{
			written = write(voltfd, buf_w, 4);
			if (written < 0)
			{
				return -1;
			}
			curTime = time(nullptr);
			if( abs(curTime - startTime) >= 2) 
				break;
			n = read(voltfd, buf_w, 4);
		}
		while(n <= 0);

		for(int i=0; i < n; i++) 
		{
			sscanf(buf_w,"%c%c%c", &one,&two,&three);
		}		
		return (three * 65536 + two *256 + one)* 220 / VOLT_RATIO;
	}

	unsigned short device::read_temperature()
	{
		char buffer[5];  
		std::ifstream in("/proc/debug/temp1");  
		if (! in.is_open())  
		{
			printf("Error opening file"); 
			return 0; 
		}  

		in.getline (buffer,5);  
		return atoi(buffer);
	}

	void device::enable_hard_yellowflash(bool on)
	{
		struct pwm pwm_test;
		if (on)
			pwm_test = {0, 2000000, 1000000};
		else
			pwm_test = {0, 2000000, 2000000};

		int ret = ioctl(hikiofd, HAL_SET_PWM, &pwm_test);
	    if(ret) {  
	        printf("ioctl HAL_SET_PWM failed!\n");
	    }
	}

	bool device::wifi_exist()
	{
		return hik::dir_exist("/sys/bus/usb/devices/1-1.4");
	}

	bool device::sim_exist()
	{
		return hik::dir_exist("/sys/bus/usb/devices/1-1.3");
	}
}

