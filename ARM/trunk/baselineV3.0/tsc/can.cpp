#include "can.h"

#if defined(__linux__) && defined(__arm__)  //这是arm交叉编译gcc内置的宏定义
#include <sys/types.h>       /* basic system data types */
#include <sys/socket.h>      /* basic socket definitions */
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/if.h>
#include <sys/time.h>
#include <cstring>
#include <cstdlib>
#include "libsocketcan.h"
#include "device.h"
//#include "detector.h"
#include "singleton.h"
#include "device.h"

void Can::Init()
{
	struct sockaddr_can addr;
    struct ifreq ifr;
	struct timeval timeout = {2, 0};
#ifdef HARDWARE_MODEL
#if (HARDWARE_MODEL == 500)		//TSC500
	UInt32 bitrate = 600000;
#elif (HARDWARE_MODEL == 300)	//TSC300
	UInt32 bitrate = 500000;
#endif
#endif
	can_do_stop("can0");
	if (can_set_bitrate("can0", bitrate) < 0)
	{
		ERR("set can0 bitrate(%u) fail!", bitrate);
		exit(1);
	}
	if (can_do_start("can0") < 0)
	{
		ERR("can0 start fail!");
		exit(1);
	}
    canfd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
	if (canfd == -1)
	{
		perror("create can socket fail!");
		exit(1);
	}
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(canfd, SIOCGIFINDEX, &ifr) == -1)
	{
		perror("ioctl get can index fail!");
		exit(1);
	}
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(canfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind can socket fail!");
		exit(1);
	}
	//设置接收超时时间为2s
	setsockopt(canfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

Can::Can() : dev(Singleton<hik::device>::GetInstance())
{
	Init();
#if 0
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		faultcheck[i].SetId(i + 1);
#endif
}

Can::~Can()
{
	if (canfd != -1)
		close(canfd);
}

void Can::SetCtrlKeyLedByCan(bitset<8> &ctrlKey)
{
	struct can_frame frame;
	frame.can_id = 0x110;
	frame.can_dlc = 1;
	frame.data[0] = (UInt8)ctrlKey.to_ulong();
	write(canfd, &frame, sizeof(struct can_frame));
}

void Can::Send(LightBits &bits)
{
	static bool flag = true;
	if (flag)
	{
		dev.led_on(hik::CANSEND_LED);
		dev.led_on(hik::CANRECV_LED);
	}
	else
	{
		dev.led_off(hik::CANSEND_LED);
		dev.led_off(hik::CANRECV_LED);
	}
	flag = !flag;

	int pos = 0;
	auto assign = [&](UInt8 &data, int n = 8){
		int i;
		data = 0;
		for (i = 0; i < n; i++)
			data |= (UInt8)bits[pos++] << i;
	};
	struct can_frame sframe;
#ifdef HARDWARE_MODEL
#if (HARDWARE_MODEL == 500)	//TSC500
	sframe.can_id = 0x101;
	sframe.can_dlc = 8;
	sframe.data[0] = 1;
	assign(sframe.data[1]);
	assign(sframe.data[2]);
	assign(sframe.data[3]);
	assign(sframe.data[4]);
	assign(sframe.data[5]);
	assign(sframe.data[6]);
	assign(sframe.data[7], 6);
	write(canfd, &sframe, sizeof(struct can_frame));
	
	sframe.can_id = 0x101;
	sframe.can_dlc = 7;
	sframe.data[0] = 2;
	assign(sframe.data[1]);
	assign(sframe.data[2]);
	assign(sframe.data[3]);
	assign(sframe.data[4]);
	assign(sframe.data[5]);
	assign(sframe.data[6]);
	sframe.data[7] = 0;
	write(canfd, &sframe, sizeof(struct can_frame));
#elif (HARDWARE_MODEL == 300)	//TSC300
	sframe.can_id = 0x101;
	sframe.can_dlc = 7;
	sframe.data[0] = 1;
	assign(sframe.data[1]);
	assign(sframe.data[2]);
	assign(sframe.data[3]);
	assign(sframe.data[4]);
	assign(sframe.data[5]);
	assign(sframe.data[6]);
	sframe.data[7] = 0;
	write(canfd, &sframe, sizeof(struct can_frame));
	//这个是为6个灯控板准备的
	sframe.can_id = 0x101;
	sframe.can_dlc = 4;
	sframe.data[0] = 3;
	assign(sframe.data[1]);
	assign(sframe.data[2]);
	assign(sframe.data[3]);
	sframe.data[4] = 0;
	sframe.data[5] = 0;
	sframe.data[6] = 0;
	sframe.data[7] = 0;
	write(canfd, &sframe, sizeof(struct can_frame));
#endif
#endif

#if 0
	for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		faultcheck[i].SetVal(bits[i * 3] | (bits[i * 3 + 1] << 1) | (bits[i * 3 + 2] << 2));
#endif
}

void Can::SendCanMsgToBoard(UInt32 can_id, void *data, int size)
{
	struct can_frame canfram;
	
	canfram.can_id = can_id;
	canfram.can_dlc = size;
	memcpy(canfram.data, data, size);
	write(canfd, &canfram, sizeof(struct can_frame));
	msleep(100);
}

void Can::Run()
{
	//bool flag = true;
	struct can_frame frame;
	int times = 0;
/* FAULT_CHECK
	auto Check = [&](){
#ifdef HARDWARE_MODEL
#if (HARDWARE_MODEL == 500)	//TSC500
		int volt = (frame.can_id >> 3) & 0x3ffff;
		int num = 6;	//每块灯控板上的通道个数
#elif (HARDWARE_MODEL == 300)	//TSC300
		int volt = (frame.can_id >> 3) & 0xfff;
		int num = 4;	//每块灯控板上的通道个数
#endif
#endif
		int index = ((frame.can_id & 0x7) - 1) * num;
		if (index < 0 || index >= MAX_CHANNEL_NUM)
			return;
		for (int i = 0; i < num; i++)
		{
			faultcheck[index++].Check((volt >> (i * 3)) & 0x7, frame.data[i]);
		}
	};
*/
#if 0
	DetectorArray &detectorArray = Singleton<DetectorArray>::GetInstance();
	bitset<24> vehBoard1(0), vehBoard2(0);
	auto DetectorBoardDeal = [&detectorArray](bitset<24> &old, bitset<24> &now, UInt8 startVehNum){
		for (int i = 0; i < 24; i++)
		{
			if (!old[i] && now[i])
			{
				old[i] = 1;
				detectorArray.Enter(startVehNum + i);
				//INFO("veh detector %d enter ###", startVehNum + i);
			}
			else if (old[i] && !now[i])
			{
				old[i] = 0;
				detectorArray.Leave(startVehNum + i);
				//INFO("veh detector %d leave $$$", startVehNum + i);
			}
		}
	};
#endif
	while (1)
	{
#if 0
		if (flag)
			dev.led_on(hik::CANRECV_LED);
		else
			dev.led_off(hik::CANRECV_LED);
		flag = !flag;
#endif
		if (read(canfd, &frame, sizeof(struct can_frame)) > 0)
		{
#ifdef HARDWARE_MODEL
#if (HARDWARE_MODEL == 500)	//TSC500
			if ((frame.can_id & 0x3fffff) > 0x200000 && GET_BIT(frame.can_id, 21) == 1)
#elif (HARDWARE_MODEL == 300)	//TSC300
			if (GET_BIT(frame.can_id, 15) == 1 && frame.can_dlc == 4)
#endif
#endif
			{//第15位为1并且长度为4的can消息表示是灯控板电流及电压反馈
				//Check();
			}
#if 0
			else if((frame.can_id == 0x201) || (frame.can_id == 0x202))//车检板信息反馈
			{	//保存过车信息
				const UInt32 *data = (UInt32 *)frame.data;
				bitset<24> now(*data);
				if (frame.can_id == 0x201)
					DetectorBoardDeal(vehBoard1, now, 1);
				else
					DetectorBoardDeal(vehBoard2, now, 25);
			}
#endif
			else if(frame.can_id == 0x401 && frame.can_dlc == 1)
			{//前面板5个按键状态
				//if ((frame.data[0] & 0x1f) != 0)
					//Device::SetCtrlKeyStatus(frame.data[0] & 0x1f);
			}
			times = 0;
		}
		else
		{	
#if 0
			if (errno == EAGAIN)
			{//接收超时,2s未收到数据，说明can异常了
				if (++times <= 10)
					ERR("CAN can't receive data with 2s, it will restart!!!!");
				else
				{	//can重启连续超过10次，则进行系统重启
#if 0
					if (gStructBinfileMisc.cIsCanRestartHiktscAllowed == 1)
					{
						ERR("The TSC will reboot because of can restart beyond 10 times!");
						sync();
						system("reboot");
					}
#endif
				}
				if (times & 0x1)
				{
					shutdown(canfd, SHUT_RDWR);//shut down the fd
					close(canfd);//必须在执行shutdown后再次close，否则fd会被一直占用最终超过系统限制
					Init();	//重启初始化can
				}
				else
				{
					system("ifconfig can0 down");
					sleep(5);
				}	
			}
#endif
		}
	}
}

#else
Can::Can() {}
Can::~Can() {}
void Can::Send(LightBits &bits) {}
void Can::SetCtrlKeyLedByCan(bitset<8> &ctrlKey) {}
void Can::SendCanMsgToBoard(UInt32 can_id, void *data, int size) {}
void Can::Run() {}
#endif
