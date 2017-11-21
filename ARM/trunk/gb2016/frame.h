#ifndef __FRAME_H__
#define __FRAME_H__

#include <string>
#include <vector>
#include <atomic>
#include "gb2016.h"

class Frame		//协议帧
{
private:
	//帧头部检测
	bool FrameHeadCheck();
	//填充帧头部
	void FillHead(OperateType operatorType, UInt8 objectId);
	//组装帧并发送
	void AssembleAndSend();
public:
	static atomic_bool online;
	FrameHead head;
	string data;
	
	//帧解析
	bool Parse(UInt8 *buffer, int size);
	//帧发送
	void Send(OperateType operatorType, UInt8 objectId);
	void Send(OperateType operatorType, UInt8 objectId, const string &frameData);
	void Send(const CurrentWorkStatus &cur);			//主动上传工作状态
	void Send(const LampColorStatus &lampStatus);		//主动上传灯色状态
	void Send(const vector<TrafficFlowInfo> &vec);		//主动上传交通流信息
	void Send(const FaultLog &fault);					//主动上传信号机故障
};

#endif
