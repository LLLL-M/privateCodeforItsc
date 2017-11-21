#ifndef _AGENT_H_
#define _AGENT_H_

#include "stmf.h"
#include "transfer.h"


#define CALLBACK
typedef int (*FN_CALLBACK)(PDU *pdu);

class CNtcip
{
public:
	CNtcip(int local_port, int remote_port);
	virtual ~CNtcip();
	static void CALLBACK HandleRequest(const char *pData, int nDataLen, void *usrData);
	int RegisterFunction(FN_CALLBACK  pFun);
	int Request(const PDU *pdu);
	int Trap(const PDU *pdu);  //trap��Ϣ
private:
	CNtcip(const CNtcip &pdu) {}
	CNtcip& operator=(const CNtcip &pdu) 
	{
		if (this == &pdu)
		{
			return *this;
		}
		return *this;
	}
	void StartService();  //�������ݽ����߳�
	void StopService();
	int  GetProtocol(const char *pData, CNtcip *pNtcip);
	void StartNet();
	void StartSeries();
	int  SendRequest(const char*pOut, int nDataLen);
private:
	FN_CALLBACK  	m_pCallBackFun;
	int 			m_local_port;
	int 			m_remote_port;
	CStmf 			*m_pStmf;
	CNet 	    	*m_pNet;
	//CSeries         *m_pSeries;
	char        	buf[MAX_PACKET];
};

#if 0
class CAgent :public CNtcip
{
public:
	CAgent(int listen_port=161, int trap_port=162); //���ؼ����˿�Ĭ��ֵ161�������Ĭ��TRAP�˿�162
	~CAgent();
	int Trap(const PDU *pdu);  //trap��Ϣ
private:
	void StartService();  //�������ݽ����߳�
	void StopService();
	CAgent(const CAgent &pdu);
	CAgent& CAgent=(const CAgent &pdu);
};

class CStation :public CNtcip
{
public:
	CStation(int listen_port=162, int remote_port=161); //���ؼ����˿�Ĭ��ֵ162,����˿�Ĭ��161
	~CStation();
private:
	void StartService();  //�������ݽ����߳�
	void StopService();
	CStation(const CStation &pdu);
	CStation& CAgent=(const CStation &pdu);
};
#endif

#endif
