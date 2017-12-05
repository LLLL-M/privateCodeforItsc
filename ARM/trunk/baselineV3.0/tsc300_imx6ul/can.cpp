#include <sys/socket.h>
#include <linux/can.h>
#include "linux.h"
#include "device.h"
#include "singleton.h"
#include "tsc.h"
#include "hik.h"

#define LIGHT_CANID1	(1u << 5)	//点灯指令的第1帧canid
#define LIGHT_CANID2	(2u << 5)	//点灯指令的第2帧canid
#define STDCANTYPE(V)		((V >> 31) ? -1 : ((V >> 7) & 0xf))		//从canid中获取标准canid的can帧类型
//#define EXTCANTYPE(V)		((V >> 31) ? ((V >> 25) & 0xf) : -1)	//从canid中获取扩展canid的can帧类型

#define VEH_BOARD_FEEDBACK_CANTYPE	1 	//车检板反馈的can帧类型
#define LAMP_BOARD_FEEDBACK_CANTYPE 2 	//灯控板反馈的can帧类型
#define CHANNEL_CUR_CANTYPE			6 	//灯控板回复通道电流信息

#define GET_BOARD_NUMBER(V) ((V > 4) & 0x7) //获取灯控板的编号
#define BOARD_FEEDBACK_TIMEOUT 3 	//灯控板反馈超时时间，单位为秒

#define TAKEOVER_CANID(SEQ) ((1u << 31) | (3 << 25) | (SEQ & 0x1ff))		//接管信息canid

namespace hik
{
	void device::status2data(const ChannelArray &lamps, std::bitset<64> &lightdata1, std::bitset<64> &lightdata2)
	{
		std::bitset<MAX_BOARD_NUM> used(0);

		int i, pos = 0;
		for (i = 0; i < FIRST_GROUP; i++)
		{
			lightdata1[pos++] = lamps[i].status & 0x1;
			lightdata1[pos++] = lamps[i].status & 0x2;
			lightdata1[pos++] = lamps[i].status & 0x4;
			lightdata1[pos++] = lamps[i].status & 0x8;

			if (lamps[i].inuse())
				used[i / CHANNEL_PER_BOARD] = true;
		}

		pos = 0;
		for (; i < FIRST_GROUP + SECOND_GROUP; i++)
		{
			lightdata2[pos++] = lamps[i].status & 0x1;
			lightdata2[pos++] = lamps[i].status & 0x2;
			lightdata2[pos++] = lamps[i].status & 0x4;
			lightdata2[pos++] = lamps[i].status & 0x8;

			if (lamps[i].inuse())
				used[i / CHANNEL_PER_BOARD] = true;
		}

		usedboard = used.to_ulong();
	}

	bool device::check(int board, uint8_t (&data)[8], bool curCheck)
	{
		int index = board * CHANNEL_PER_BOARD;	//通道索引
		uint64_t info = *(uint64_t *)data;
		ChannelArray lamps = cache;

		for (int i = 0; i < CHANNEL_PER_BOARD; i++)
		{	
			if (!lamps[index].inuse())
				continue;
			//一个通道3个led，每个led的反馈信息占用3bit，所以一个通道9bit反馈信息
			if (faultcheck[index].hasfault((info >> (i * 9)) & 0x1ff, curCheck, lamps[i]))
				return false;
		}
		return true;
	}

	void device::can_recv()
	{
		struct can_frame frame;
		std::array<struct timespec, MAX_BOARD_NUM> recvtime;
		auto & unit = Singleton<Tsc>::GetInstance().GetUnit();
		std::bitset<MAX_BOARD_NUM> used;
		struct timespec now;
		int board = 0;
		uint8_t cantype;
		bool canrcvled = true;

		clock_gettime(CLOCK_MONOTONIC, &now);
		recvtime.fill(now);

		while (1)
		{
			if (read(canfd, &frame, sizeof(struct can_frame)) > 0)
			{
				ledctrl(CANRECV_LED, canrcvled);
				canrcvled = !canrcvled;
				cantype = STDCANTYPE(frame.can_id);
				if (cantype == LAMP_BOARD_FEEDBACK_CANTYPE)
				{	//灯控板反馈的电流电压信息
					board = GET_BOARD_NUMBER(frame.can_id);
					if (board >= MAX_BOARD_NUM)
						continue;
					clock_gettime(CLOCK_MONOTONIC, &recvtime[board]);	//记录接收到相应灯控板反馈信息的时间

					if ((!unit.voltCheck && !unit.curCheck) || yellowflash_board_status() != NO_YELLOWFLASH)
						continue;	//只有黄闪板处于正常工作状态时才能进行检测
					if (!check(board, frame.data, unit.curCheck))
					{
						enable_hard_yellowflash(true);	//执行硬黄闪
						//log.write("volt current check fault, excute hard yellow flash!");
						break;	//执行硬黄闪之后便不再恢复，所以can接收线程退出
					}
				}
				else if(cantype == VEH_BOARD_FEEDBACK_CANTYPE)//车检板信息反馈
				{	//车检板反馈的过车状态
				}
				else if (cantype == CHANNEL_CUR_CANTYPE)	//灯控板回复通道电流信息
				{
					ChannelCurVal *v = (ChannelCurVal *)frame.data;
					curvals = *v;
					semForGetCur.post();
				}
			}
			else
			{	
				if (errno == EAGAIN)
				{//接收超时,2s未收到数据，说明can异常了
					enable_hard_yellowflash(true);	//执行硬黄闪
					//log.write("Can bus communication is exception, excute hard yellow flash!");
					break;	//执行硬黄闪之后便不再恢复，所以can接收线程退出
				}
			}

			used = usedboard.load();
			clock_gettime(CLOCK_MONOTONIC, &now);
			for (int i = 0; i < MAX_BOARD_NUM; i++)
			{
				if (used[i] && (now.tv_sec - recvtime[i].tv_sec >= BOARD_FEEDBACK_TIMEOUT))
				{	//如果灯控板正在使用且超过一定时间还未收到回复信息则硬黄闪
					enable_hard_yellowflash(true);	//执行硬黄闪
					//log.write("light control board %d communication is exception, excute hard yellow flash!", i);
					return;	//执行硬黄闪之后便不再恢复，所以can接收线程退出
				}
			}
		}
	}

	void device::light()
	{
		struct can_frame frame1 = {LIGHT_CANID1, 8, {0}};	//第一帧
		struct can_frame frame2 = {LIGHT_CANID2, 8, {0}};	//第二帧
		uint64_t *data1 = (uint64_t *)frame1.data;
		uint64_t *data2 = (uint64_t *)frame2.data;
		std::bitset<64> lightdata1(0);
		std::bitset<64> lightdata2(0);

		status2data(cache.load(), lightdata1, lightdata2);

		*data1 = lightdata1.to_ullong();
		write(canfd, &frame1, sizeof(struct can_frame));

		*data2 = lightdata2.to_ullong();
		write(canfd, &frame2, sizeof(struct can_frame));
		ledctrl(CANSEND_LED, cansndled);
		cansndled = !cansndled;
	}

	void device::send_checkinfo()
	{
		struct can_frame frame1 = {(4u << 7) | (2 << 5) | (1 << 3), 8, {0}};
		struct can_frame frame2 = {(4u << 7) | (2 << 5) | (2 << 3), 8, {0}};
#if 0
		write(canfd, &frame1, sizeof(struct can_frame));
		write(canfd, &frame2, sizeof(struct can_frame));
#else
		uint64_t *data1 = (uint64_t *)frame1.data;
		uint64_t *data2 = (uint64_t *)frame2.data;
		ChannelArray lamps = cache;
		auto & unit = Singleton<Tsc>::GetInstance().GetUnit();
		std::bitset<64> bits(0);
		int i = 0, pos;

		bits[0] = unit.voltCheck;
		bits[1] = unit.curCheck;
		pos = 8;	//偏移8bit即一字节，首字节存放检测开关
		while (pos + 3 < 64)
		{
			bits[pos++] = lamps[i].inuse();							//绿灯
			bits[pos++] = lamps[i].inuse();							//红灯
			bits[pos++] = (lamps[i].inuse() && !lamps[i].isped());	//黄灯，且行人灯不接黄灯
			i++;
		}
		bits[pos++] = lamps[i].inuse();		//(64 - 8) % 3 =剩余2bit，一个绿灯一个红灯
		bits[pos++] = lamps[i].inuse();
		*data1 = bits.to_ullong();
		write(canfd, &frame1, sizeof(struct can_frame));

		bits = 0;
		pos = 0;
		bits[pos++] = (lamps[i].inuse() && !lamps[i].isped());	//第二帧第1bit为通道未组完黄灯
		for (i = i + 1; i < FIRST_GROUP + SECOND_GROUP; i++)
		{
			bits[pos++] = lamps[i].inuse();							//绿灯
			bits[pos++] = lamps[i].inuse();							//红灯
			bits[pos++] = (lamps[i].inuse() && !lamps[i].isped());	//黄灯，且行人灯不接黄灯
		}
		*data2 = bits.to_ullong();
		usleep(10000);
		write(canfd, &frame2, sizeof(struct can_frame));
#endif
		//INFO("send takeover ***************");
	}

	void device::send_takeover()
	{
		if (takeover.size() < 3)
		{	//接管信息太少，不发送
			takeover.clear();
			return;
		}
		static_assert(sizeof(TakeOverInfo) == 26, "sizeof(TakeOverInfo) != 26");
		static_assert(sizeof(TakeOverInfoHead) == 8, "sizeof(TakeOverInfo) != 8");
		
		struct can_frame frame = {TAKEOVER_CANID(0), 8, {0}};
		TakeOverInfoHead head;		
		size_t bytes = sizeof(TakeOverInfo) * takeover.size();
		const uint8_t *data = (const uint8_t *)takeover.data();
		
		head.checksum = 0;
		for (size_t i = 0; i < bytes; i++)
			head.checksum += data[i];
		head.arraynum = takeover.size();
		head.framenum = bytes / 8 + ((bytes % 8) ? 1 : 0);
		memcpy(frame.data, &head, 8);

		//INFO("bytes = %u, checksum = %#x, arraynum = %u, framenum = %u", bytes, head.checksum, head.arraynum, head.framenum);
		write(canfd, &frame, sizeof(frame));	//发送接管信息头
		usleep(10000);	//延时10ms
		for (size_t seq = 1; seq <= head.framenum; seq++)
		{
			frame.can_id = TAKEOVER_CANID(seq);
			if (bytes >= 8)
			{
				memcpy(frame.data, data, 8);
				data += 8;
				bytes -= 8;
			}
			else
			{
				memset(frame.data, 0, 8);
				memcpy(frame.data, data, bytes);
			}
			write(canfd, &frame, sizeof(frame));	//发送接管信息
			usleep(10000);	//延时10ms
		}
		takeover.clear();

		sleep(1);
		send_checkinfo();
	}

	ChannelCurVal device::get_channel_cur(uint8_t channel)
	{
		ChannelCurVal vals = {0, 0, 0};
		if (channel == 0 || channel > CHANNEL_PER_BOARD * MAX_BOARD_NUM)
			return vals;
		uint8_t board = (channel - 1) / CHANNEL_PER_BOARD;
		uint8_t group = (channel - 1) % CHANNEL_PER_BOARD + 1;
		struct can_frame frame = {(1u << 30) | (5 << 7) | (board << 4) | group, 0, {0}};
		write(canfd, &frame, sizeof(frame));
		semForGetCur.waitfor(250);	//等待250ms获取电流信息
		return curvals;
	}
}
