#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "hikmsg.h"
#include "common.h"
#include "LogSystem.h"
#include "platform.h"

#define FAULT_LOG_FILE		"/home/faultlog.dat"
#define FAULT_LOG_MAX_SIZE	262144	//256k
#define FAULT_LOG_MAX_NUM	(FAULT_LOG_MAX_SIZE / sizeof(STRU_FAILURE_INFO))

extern int msgid;

typedef struct faultLogHead
{
	int num;
	STRU_FAILURE_INFO data[0];
} FaultLogHead;

void ItsReadFaultLog(int startLine, int lineNum, int socketfd, struct sockaddr *fromAddr)
{
	struct msgbuf msg;
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_FAULT_LOG;
	msg.msgFLrwflag = READ_FAULT_LOG;
	msg.msgFLstartLine = startLine;
	msg.msgFLlineNum = lineNum;
	msg.msgFLsocketfd = socketfd;
	memcpy(&msg.msgFLdstaddr, fromAddr, sizeof(struct sockaddr));
	msgsnd(msgid, &msg, MSGSIZE, 0);
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
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

void ItsClearFaultLog(void)
{
	struct msgbuf msg;
	memset(&msg, 0, sizeof(msg));
	msg.mtype = MSG_FAULT_LOG;
	msg.msgFLrwflag = CLEAR_FAULT_LOG;
	msgsnd(msgid, &msg, MSGSIZE, 0);
}

static void UploadFaultLogDeal(char *buf, struct msgbuf *msg, FaultLogHead *head)
{
	struct STRU_Extra_Param_Response *response = (struct STRU_Extra_Param_Response *)buf;
	int datalen = 0;
	
	response->unExtraParamHead = 0x6e6e;		//消息头标志
	response->unExtraParamID = 0xc0;			//消息类型ID
	response->unExtraParamValue = 0x15b;		//块数据类型
	if (msg->msgFLstartLine <= 0 || msg->msgFLstartLine > head->num || msg->msgFLlineNum <= 0)
	{
		response->unExtraParamFirst = 0;
		response->unExtraParamTotal = 0;
	}
	else
	{
		response->unExtraParamFirst = msg->msgFLstartLine;
		response->unExtraParamTotal = min(head->num - msg->msgFLstartLine + 1, msg->msgFLlineNum);
	}
	if (response->unExtraParamFirst > 0 && response->unExtraParamTotal > 0)
	{
		datalen = sizeof(STRU_FAILURE_INFO) * response->unExtraParamTotal;
		response->unExtraDataLen = datalen;
		memcpy(response->data, head->data + (response->unExtraParamFirst - 1), datalen);
	}
	sendto(msg->msgFLsocketfd, buf, 
			sizeof(*response) + datalen, 0, 
			&msg->msgFLdstaddr, sizeof(struct sockaddr));
}

void *FaultLogModule(void *arg)
{
	struct msgbuf msg;
	int fd = open(FAULT_LOG_FILE, O_RDWR | O_APPEND | O_CREAT, 0666);
	int totalsize = sizeof(FaultLogHead) + FAULT_LOG_MAX_SIZE;
	FaultLogHead tmp, *head = NULL;
	STRU_FAILURE_INFO *info = NULL;
	char buf[sizeof(struct STRU_Extra_Param_Response) + FAULT_LOG_MAX_SIZE] = {0};

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
			msync(info, sizeof(STRU_FAILURE_INFO), MS_SYNC);
			msync(head, sizeof(head->num), MS_SYNC);
		}
		else if (msg.msgFLrwflag == READ_FAULT_LOG)
			UploadFaultLogDeal(buf, &msg, head);
		else if (msg.msgFLrwflag == CLEAR_FAULT_LOG)
			memset(head, 0, totalsize);
	}
}
