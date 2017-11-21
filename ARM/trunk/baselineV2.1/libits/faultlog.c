#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include "hikmsg.h"
#include "its.h"
#include "LogSystem.h"

#define FAULT_LOG_FILE		"/home/faultlog.dat"
#define FAULT_LOG_MAX_SIZE	262144	//256k
#define FAULT_LOG_MAX_NUM	(FAULT_LOG_MAX_SIZE / sizeof(FaultLogInfo))

extern int msgid;

typedef struct faultLogHead
{
	int num;
	FaultLogInfo data[0];
} FaultLogHead;

void ItsReadFaultLog(int startLine, int lineNum, void *netArg, int netArgSize, UploadFaultLogFunc func)
{
	struct msgbuf msg;
	FaultLogInfo zero;	//表明上载故障日志出错，默认应答回复1个空的故障日志

	if (netArg == NULL || func == NULL)
		return;
	if (netArgSize > MSGSIZE - sizeof(struct faultLogMsg))
	{
		ERR("network arguments size exceed msg space");
		memset(&zero, 0, sizeof(FaultLogInfo));
		func(netArg, &zero, sizeof(FaultLogInfo));
	}
	else
	{
		memset(&msg, 0, sizeof(msg));
		msg.mtype = MSG_FAULT_LOG;
		msg.msgFLrwflag = READ_FAULT_LOG;
		msg.msgFLstartLine = startLine;
		msg.msgFLlineNum = lineNum;
		msg.msgFLfunc = func;
		memcpy(msg.msgFLnetArg, netArg, netArgSize);
		if (msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT) != 0)
			log_error("send read fault log msg fail, errno info: %s", strerror(errno));
	}
}

void ItsWriteFaultLog(FaultLogType type, int value)
{
	struct msgbuf msg = {
		.mtype = MSG_FAULT_LOG,
		.msgText = {0},
	};
	msg.msgFLrwflag = WRITE_FAULT_LOG;
	msg.msgFLtype = type;
	msg.msgFLtime = time(NULL);
	msg.msgFLvalue = value;
	if (msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT) != 0)
		log_error("send write fault log msg fail, errno info: %s", strerror(errno));
}

void ItsClearFaultLog(void)
{
	struct msgbuf msg;
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_FAULT_LOG;
	msg.msgFLrwflag = CLEAR_FAULT_LOG;
	if (msgsnd(msgid, &msg, MSGSIZE, IPC_NOWAIT) != 0)
		log_error("send clear fault log msg fail, errno info: %s", strerror(errno));
}

static inline void UploadFaultLogDeal(struct msgbuf *msg, FaultLogHead *head)
{
	FaultLogInfo zero;	//表明上载故障日志出错，默认应答回复1个空的故障日志
	FaultLogInfo *data = NULL;
	int datasize = 0;

	if (msg->msgFLfunc == NULL)
		return;
	if (msg->msgFLstartLine == 0 || abs(msg->msgFLstartLine) > head->num || msg->msgFLlineNum <= 0)
	{
		ERR("msgFLstartLine[%d] or msgFLlineNum[%d] is invalid!", msg->msgFLstartLine, msg->msgFLlineNum);
		memset(&zero, 0, sizeof(FaultLogInfo));
		data = &zero;
		datasize = sizeof(FaultLogInfo);
	}
	else if (msg->msgFLstartLine < 0)
	{
		if (head->num + msg->msgFLstartLine + 1 >= msg->msgFLlineNum)
		{
			data = head->data + (head->num + msg->msgFLstartLine + 1 - msg->msgFLlineNum);
			datasize = msg->msgFLlineNum * sizeof(FaultLogInfo);
		}
		else
		{
			data = head->data;
			datasize = head->num * sizeof(FaultLogInfo);
		}
	}
	else
	{
		data = head->data + (msg->msgFLstartLine - 1);
		datasize = min(head->num - msg->msgFLstartLine + 1, msg->msgFLlineNum) * sizeof(FaultLogInfo);
	}
	msg->msgFLfunc(msg->msgFLnetArg, data, datasize);
}

void *FaultLogModule(void *arg)
{
	struct msgbuf msg;
	int fd = open(FAULT_LOG_FILE, O_RDWR | O_APPEND | O_CREAT, 0666);
	int totalsize = sizeof(FaultLogHead) + FAULT_LOG_MAX_SIZE;
	FaultLogHead tmp, *head = NULL;
	FaultLogInfo *info = NULL;

	if (fd == -1)
	{
		log_error("open %s fail, error info: %s", FAULT_LOG_FILE, strerror(errno));
		pthread_exit(NULL);
	}

	tmp.num = 0;
	read(fd, &tmp, sizeof(tmp));
	if (tmp.num == 0)
		ftruncate(fd, totalsize);
	else
		INFO("the number of fault log is %d", tmp.num);
	head = (FaultLogHead *)mmap(NULL, totalsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (head == MAP_FAILED)
	{
		log_error("mmap %s fail, error info: %s", FAULT_LOG_FILE, strerror(errno));
		close(fd);
		pthread_exit(NULL);
	}
	head->num = tmp.num;

	memset(&msg, 0, sizeof(msg));
	while (1)
	{
		if (-1 == msgrcv(msgid, &msg, MSGSIZE, MSG_FAULT_LOG, 0))
		{
			usleep(100000);
			continue;
		}
		if (msg.msgFLrwflag == WRITE_FAULT_LOG)
		{
			if (head->num + 1 > FAULT_LOG_MAX_NUM)
				head->num = 0;	//写到文件末尾了重头覆盖
			info = head->data + head->num;
			info->nNumber = head->num + 1;
			info->nID = msg.msgFLtype;
			info->nTime = msg.msgFLtime;
			info->nValue = msg.msgFLvalue;
			head->num++;
			msync(info, sizeof(FaultLogInfo), MS_ASYNC);
			msync(head, sizeof(head->num), MS_ASYNC);
		}
		else if (msg.msgFLrwflag == READ_FAULT_LOG)
			UploadFaultLogDeal(&msg, head);
		else if (msg.msgFLrwflag == CLEAR_FAULT_LOG)
			memset(head, 0, totalsize);
	}
}
