#ifndef __COMMUNICATION_H__
#define	__COMMUNICATION_H__

#include "thread.h"
#include "timer.h"
#include "tsc.h"
#include "sock.h"
#include "frame.h"
#include "singleton.h"
#include "log.h"

class YellowFault : public HikThread::Thread
{
private:
	Sock sock;
	Log	 &log;

	void Parse(const char *buf, const int len)
	{
		if (len != 8)
			return;
		string cmd(buf, 7);
		int channel = buf[7];
		char msg[128] = {0};

		if (cmd == "turnoff")
		{
			sprintf(msg, "the channel %d yellow light occur fault!", channel);
			log.Write(YELLOW_LIGHT_FAULT, msg, channel);
		}
		else if (cmd == "turn on")
		{
			sprintf(msg, "the channel %d yellow light fault is clear!", channel);
			log.Write(YELLOW_LIGHT_FAULT_CLEAR, msg, channel);
		}
		else
			cerr << "the data[" << cmd << "] is invalid!" << endl;
	}

public:
	YellowFault() : log(Singleton<Log>::GetInstance())
	{
		sock.CreateUdpSocket(20001);
	}

	void run(void *arg)
	{
		char buf[32] = {0};
		int len = 0;

		while (true)
		{
			memset(buf, 0, sizeof(buf));
			len = sock.Recvfrom(buf, sizeof(buf), 0);
			Parse(buf, len);
		}
	}

protected:
};

class Communication: public HikThread::Thread, public Timer
{
private:
	volatile bool 	online;	//是否已联机
	Tsc 	&tsc;
	Sock	&sock;
	Log		&log;
	UInt32	sendHeartbeat;	//发送心跳数
	UInt32	recvHeartbeat;	//接收心跳数
	Frame 	heartbeatFrame;	//用于发送心跳包
public:
	Communication(Tsc &t, Sock &s) : tsc(t), sock(s), log(Singleton<Log>::GetInstance())
	{
		online = false;
		Frame::online = false;
		sendHeartbeat = 0;
		recvHeartbeat = 0;
	}
	~Communication() {}
	void timeout()
	{
		if (online)
		{
			if (sendHeartbeat < recvHeartbeat + 3)
			{
				sendHeartbeat++;
				heartbeatFrame.Send(GB_QUERY_REQ, ONLINE);	//发送心跳信息
				return;
			}
			else
			{	//联机断开
				online = false;
				Frame::online = false;
				sendHeartbeat = 0;
				recvHeartbeat = 0;
				log.Write(COMMUNICATION_DISCONNECT, "The connection break off!!!");
				ControlRule rule = tsc.curRule;
				if (rule.ctrlType == CLIENT_CONTROL)
					tsc.Downgrade();	//如果当前是上位机控制则进行降级
			}
		}	
		heartbeatFrame.Send(GB_SET_REQ, ONLINE);	//发送联机信息
	}
	
	void run(void *arg);
	
protected:

};

void Communication::run(void *arg)
{
	int len = 0;
	UInt8 recvbuf[8192];
	Frame frame;
	FrameHead &head = frame.head;
	OperateType operatorType;
	string sendData;
	
	timerstart(5000);
	heartbeatFrame.Send(GB_SET_REQ, ONLINE);	//发送联机信息
	while (1)
	{
		operatorType = GB_ERR_REPONSE;	//默认出错应答
		memset(recvbuf, 0, sizeof(recvbuf));
		len = sock.Recvfrom(recvbuf, sizeof(recvbuf), 0);
		if (len <= 0)
			continue;
		if (frame.Parse(recvbuf, len))
		{
			if (!online)
			{
				if (head.objectId == ONLINE && head.operatorType == GB_SET_REPONSE)
				{
					online = true;	//联机成功
					Frame::online = true;
					log.Write(COMMUNICATION_CONNECT, "connect host computer successful!");
				}
#if 0
				else
					ERR("objectId[%d] != %d or operatorType[%#x] != %#x when offline!", head.objectId, ONLINE, head.operatorType, GB_SET_REPONSE);
#endif
				continue;
			}
			if (head.objectId == ONLINE && head.operatorType == GB_QUERY_REPONSE)
			{
				recvHeartbeat++;
				continue;
			}
			if (head.operatorType == GB_SET_REQ)
			{
				if (tsc.Write(head.objectId, frame.data))
					operatorType = GB_SET_REPONSE;
				else
					ERR("Set objectId[%d] error!", head.objectId);
			}
			else if (head.operatorType == GB_QUERY_REQ)
				operatorType = GB_QUERY_REPONSE;	
		}
		if (operatorType == GB_QUERY_REPONSE)
		{
			sendData.clear();
			tsc.Read(head.objectId, frame.data, sendData);	//填充数据内容
			frame.Send(operatorType, head.objectId, sendData);
		}
		else if (operatorType == 0 || head.objectId == 0)
		{
		}
		else
		{
			//INFO("operatorType = %#x", operatorType);
			frame.Send(operatorType, head.objectId);
		}
	}
}

#endif
