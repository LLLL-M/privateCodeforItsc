#include <cstring>
#include "frame.h"
#include "tsc.h"
#include "sock.h"
#include "singleton.h"

atomic_bool Frame::online;

bool Frame::FrameHeadCheck()
{
	if (head.version != 0x10)
	{
		ERR("version must be 0x10, but it's %#x", head.version);
		return false;
	}
	if (head.sendFlag != 0x20)
	{
		ERR("sendFlag must be 0x20, but it's %#x", head.sendFlag);
		return false;
	}
	if (head.recvFlag != 0x10)
	{
		ERR("recvFlag must be 0x10, but it's %#x", head.recvFlag);
		return false;
	}
	if (head.dataLinkCode < COMMUNICATION_RULE_LINK || head.dataLinkCode > INTERVENE_COMMAND_LINK)
	{
		ERR("dataLinkCode[%d] is out of range [%d, %d]", head.dataLinkCode, COMMUNICATION_RULE_LINK, INTERVENE_COMMAND_LINK);
		return false;
	}
	if (head.areaNumber != 0x01 || head.roadNumber != 0x01)
	{
		ERR("areaNumber[%d] != 0x01 or roadNumber[%d] != 0x01", head.areaNumber, head.roadNumber);
		return false;
	}
	if (head.objectId == 0 && head.operatorType == 0)
	{	//发送错误信息时会回复:操作类型=0,对象标识=0
		return false;
	}
	else
	{
		if (head.objectId < ONLINE || head.objectId > DETECTOR)
		{
			ERR("objectId[%d] is out of range [%d, %d]", head.objectId, ONLINE, DETECTOR);
			return false;
		}
		if ((head.objectId == ONLINE 
					&& head.operatorType != GB_QUERY_REPONSE 
					&& head.operatorType != GB_SET_REPONSE)
				|| (head.objectId != ONLINE 
					&& head.operatorType != GB_QUERY_REQ 
					&& head.operatorType != GB_SET_REQ))
		{
			ERR("operatorType[%#x] don't match objectId[%d]", head.operatorType, head.objectId);
			return false;
		}
	}
	if (head.operatorType == GB_SET_REQ && (head.reserve[0] != '1' || head.reserve[1] != '2' || head.reserve[2] != '3' || head.reserve[3] != '4' || head.reserve[4] != '5'))
	{
		ERR("reserve[%s] isn't '12345'!", head.reserve);
		return false;
	}
	if (head.dataLinkCode == COMMUNICATION_RULE_LINK && head.objectId == ONLINE 
		&& head.operatorType == GB_SET_REPONSE 
		&& (data[0] != '6' || data[1] != '7' || data[2] != '8' || data[3] != '9'))
	{
		ERR("identify code[%c%c%c%c] isn't '6789'", data[0], data[1], data[2], data[3]);
		return false;
	}
	//INFO("check head successful!");
	return true;
}

void Frame::FillHead(OperateType operatorType, UInt8 objectId)
{
	FrameHead &h = head;
	
	memset(&h, 0, sizeof(h));
	h.version = 0x10;
	h.sendFlag = 0x10;
	h.recvFlag = 0x20;
	if (objectId == ONLINE)
		h.dataLinkCode = COMMUNICATION_RULE_LINK;
	else if (objectId == TRAFFIC_FLOW_INFO || objectId == WORK_STATUS 
			|| objectId == LAMP_STATUS || objectId == CURRENT_TINE
			|| objectId == SIGNAL_MACHINE_FAULT || objectId == SIGNAL_MACHINE_VER)
		h.dataLinkCode = BASIC_INFORMATION_LINK;
	else if (objectId == SIGNAL_LAMP_GROUP || objectId == PHASE
			|| objectId == SIGNAL_TIMING_SCHEME || objectId == SCHEME_SCHEDULE)
		h.dataLinkCode = FEATURES_PARAMETER_LINK;
	else if (objectId == WORK_MODE || objectId == FEATURES_PARAMETER
			|| objectId == SIGNAL_MACHINE_ID_CODE || objectId == REMOTE_CONTROL
			|| objectId == DETECTOR)
		h.dataLinkCode = INTERVENE_COMMAND_LINK;
	else
	{
		//ERR("objectId %d is invalid when send frame", objectId);
		//return;
	}
	h.areaNumber = 1;//tsc->basic.areaNumber;	这两处暂时改为默认值,如果有需求则改为类静态变量去赋值
	h.roadNumber = 1;//tsc->basic.roadNumber;
	h.operatorType = operatorType;
	h.objectId = objectId;
	data.clear();
	data.append((const char *)&h, sizeof(FrameHead));
}

void Frame::AssembleAndSend()
{
	int i, n = 0;
	UInt32 sum = 0;
	UInt8 checksum = 0;
	string buffer;
	Sock &sock = Singleton<Sock>::GetInstance();
	
	//把数据按照帧格式组装放到buffer里
	buffer.append(1, 0xC0);
	for (i = 0; i < data.size(); i++)
	{
		if ((UInt8)data[i] == 0xC0)
		{
			buffer.append(1, 0xDB);
			buffer.append(1, 0xDC);
			sum += 0xDB + 0xDC;
		}
		else if ((UInt8)data[i] == 0xDB)
		{
			buffer.append(1, 0xDB);
			buffer.append(1, 0xDD);
			sum += 0xDB + 0xDD;
		}
		else
		{
			buffer.append(1, data[i]);
			sum += (UInt8)data[i];
		}
	}
#if 0	//校验码默认为0
	checksum = sum & 0xff;
	if (checksum == 0xC0)
	{
		buffer.append(1, 0xDB);
		buffer.append(1, 0xDC);
	}
	else if (checksum == 0xDB)
	{
		buffer.append(1, 0xDB);
		buffer.append(1, 0xDD);
	}
	else
#endif
		buffer.append(1, checksum);
	buffer.append(1, 0xC0);
	sock.Sendto(buffer.data(), buffer.size(), 0);
}

bool Frame::Parse(UInt8 *buffer, int size)
{
	int i, len;
	UInt32 sum = 0;
	UInt8 checksum = 0;
	
	if (size < sizeof(FrameHead) + 3 || buffer[0] != 0xC0 || buffer[size - 1] != 0xC0)
	{
		//ERR("receive data is invalid!");
		return false;
	}
	//解析校验和以及帧数据长度
	if (buffer[size - 3] == 0xDB && buffer[size - 2] == 0xDC)
	{
		checksum = 0xC0;
		len = size - 4;
	}
	else if (buffer[size - 3] == 0xDB && buffer[size - 2] == 0xDD)
	{
		checksum = 0xDB;
		len = size - 4;
	}
	else
	{
		checksum = buffer[size - 2];
		len = size - 3;
	}
	//解析帧结构,并计算校验和
	data.clear();
	for (i = 1; i <= len; i++)
	{
		//INFO("data: %#x", buffer[i]);
		if (buffer[i] == 0xDB && buffer[i+1] == 0xDC)
		{
			data.append(1, 0xC0);
			sum += 0xDB + 0xDC;
			i++;
		}
		else if (buffer[i] == 0xDB && buffer[i+1] == 0xDD)
		{
			data.append(1, 0xDB);
			sum += 0xDB + 0xDD;
			i++;
		}
		else
		{
			data.append(1, (char)buffer[i]);
			sum += buffer[i];
		}
	}
#if 0	//不进行校验
	if ((sum & 0xff) != checksum)
	{
		ERR("the checksum[%#x] is invalid!", checksum);
		return false;
	}
#endif
	if (data.size() < sizeof(FrameHead))
	{
		ERR("the frame is too short!");
		return false;
	}
	data.copy((char *)&head, sizeof(FrameHead));
	data.erase(0, sizeof(FrameHead));
	return FrameHeadCheck();
}

void Frame::Send(OperateType operatorType, UInt8 objectId)
{
	//填充帧头部
	FillHead(operatorType, objectId);
	AssembleAndSend();		//组装帧并发送
}

void Frame::Send(OperateType operatorType, UInt8 objectId, const string &frameData)
{
	//填充帧头部
	FillHead(operatorType, objectId);
	data.append(frameData);	//填充帧数据
	AssembleAndSend();		//组装帧并发送
}

void Frame::Send(const CurrentWorkStatus &cur)
{
	if (!online)
		return;
	//填充帧头部
	FillHead(GB_ACTIVE_UPLOAD, WORK_STATUS);
	data.append((char *)&cur, sizeof(CurrentWorkStatus));	//填充帧数据
	AssembleAndSend();		//组装帧并发送
}

void Frame::Send(const LampColorStatus &lampStatus)
{
	if (!online)
		return;
	//填充帧头部
	FillHead(GB_ACTIVE_UPLOAD, LAMP_STATUS);
	data.append((char *)&lampStatus, sizeof(LampColorStatus));	//填充帧数据
	AssembleAndSend();		//组装帧并发送
}

void Frame::Send(const vector<TrafficFlowInfo> &vec)
{
	if (!online)
		return;
	//填充帧头部
	FillHead(GB_ACTIVE_UPLOAD, TRAFFIC_FLOW_INFO);
	data.push_back((char)vec.size());	//首字节填充TrafficFlowInfo个数
	data.append((char *)vec.data(), vec.size() * sizeof(TrafficFlowInfo));
	AssembleAndSend();		//组装帧并发送
}

void Frame::Send(const FaultLog &fault)
{
	if (!online)
		return;
	//填充帧头部
	FillHead(GB_ACTIVE_UPLOAD, SIGNAL_MACHINE_FAULT);
	data.push_back(1);	//首字节填充主动上传故障的个数
	data.append((char *)&fault, sizeof(FaultLog));	//填充帧数据
	AssembleAndSend();		//组装帧并发送
}

