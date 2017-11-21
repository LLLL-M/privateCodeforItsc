#ifndef _TRANSFER_H_
#define _TRANSFER_H_

#include <netinet/in.h>

#define MAX_PACKET  4096
typedef void (*CALLBACK_FUN)(const char *pData, int nDataLen, void *usrData);
#if 0
class CTransfer
{
public:
	CTransfer(int port);
	virtual ~CTransfer();
	void RegisterFun(CALLBACK_FUN pFun);
	virtual void 
	virtual int  Respond(const char *pData);
	virtual void Task_Receive();
public:
	CALLBACK_FUN m_pCallBackFun;
};
#endif

//net
class CNet //:public CTransfer
{
public:
	CNet(int port);
	~CNet();
	void RegisterFun(CALLBACK_FUN pFun, void *pUsrData);
	int  Respond(const char *pData, int nDataLen);
	void CreateTask();
	static void* Task_Receive(void *p);
private:
	int m_port;
	int m_fd;
	struct sockaddr_in from_addr;
	int fromlen;
	void *m_pUsrData;
	CALLBACK_FUN m_pCallBackFun;
	long receive_buffer_len;
	char receive_buffer[MAX_PACKET];
};

//series
#if 0   //暂时不实现.
class CSeries
{
public:
	CSeries(int pound);
	~CSeries();
	void RegisterFun(CALLBACK_FUN pFun, void *pUsrData);
	int  Respond(const char *pData, int nDataLen);
	static void Task_Receive();
private:
	int m_pound;
	void *m_pUsrData;
	CALLBACK_FUN m_pCallBackFun;
};
#endif

#endif

