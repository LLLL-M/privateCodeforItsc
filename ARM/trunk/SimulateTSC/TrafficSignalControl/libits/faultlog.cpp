#include <cstring>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "protocol.h"
#include "faultlog.h"

#ifdef __linux__
#define FAULT_LOG_FILE		"/home/faultlog.dat"
#else   //__WIN32__
#define FAULT_LOG_FILE		"faultlog.dat"
#endif
#define FAULT_LOG_MAX_SIZE	262144	//256k
#define FAULT_LOG_MAX_NUM	(FAULT_LOG_MAX_SIZE / sizeof(FaultLogInfo))

using namespace HikFaultlog;

std::string & Faultlog::Read(int startLine, UInt32 lineNum, std::string & log)
{
	FaultLogInfo *data = NULL;
	int datasize = 0;

	rwlock.r_lock();
	if (head == NULL || startLine == 0 || std::abs(startLine) > head->num || lineNum <= 0)
	{
		//ERR("msgFLstartLine[%d] or msgFLlineNum[%d] is invalid!", startLine, lineNum);
		//data = head->data;
		datasize = 0;
	}
	else if (startLine < 0)
	{
		if (head->num + startLine + 1 >= lineNum)
		{
			data = head->data + (head->num + startLine + 1 - lineNum);
			datasize = lineNum * sizeof(FaultLogInfo);
		}
		else
		{
			data = head->data;
			datasize = head->num * sizeof(FaultLogInfo);
		}
	}
	else
	{
		data = head->data + (startLine - 1);
		datasize = std::min(head->num - startLine + 1, lineNum) * sizeof(FaultLogInfo);
	}
	if (datasize > 0)
		log.append((char *)data, datasize);
	rwlock.r_unlock();
	return log;
}

bool Faultlog::Write(FaultLogType type, int value)
{
	FaultLogInfo *info = NULL;
	if (head == NULL)
		return false;
	rwlock.w_lock();
	if (head->num + 1 > FAULT_LOG_MAX_NUM)
		head->num = 0;	//写到文件末尾了重头覆盖
	info = head->data + head->num;
	info->nNumber = head->num + 1;
	info->nID = type;
	info->nTime = std::time(NULL);
	info->nValue = value;
	head->num++;
	file.Msync(info, sizeof(FaultLogInfo));
	file.Msync(head, sizeof(head->num));
	rwlock.w_unlock();
	return true;
}

void Faultlog::Clear()
{
	if (head == NULL)
		return;
	rwlock.w_lock();
	std::memset(head, 0, FAULT_LOG_MAX_SIZE);
	rwlock.w_unlock();
}

Faultlog::Faultlog(Protocol & p) : ptl(p)
{
	head = NULL;
	if (file.Open(FAULT_LOG_FILE))
	{
		head = static_cast<FaultLogHead *>(file.Mmap(FAULT_LOG_MAX_SIZE));
		if (head == NULL)
			file.Close();
	}
}

Faultlog::~Faultlog()
{
	if (head != NULL)
	{
		file.Unmap(head, FAULT_LOG_MAX_SIZE);
		file.Close();
		head = NULL;
	}
}
