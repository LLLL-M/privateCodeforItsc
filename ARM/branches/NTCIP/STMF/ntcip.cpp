/**	@file ntcip.cpp 
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief 
*
*	@author		zhaiyunfeng
*	@date		2013/04/01
*
*	@note 下面的note和warning为可选项目
*	@note 这里填写本文件的详细功能描述和注解
*	@note 历史记录：
*	@note V2.0.0  添加了一个导出接口
*
*	@warning 这里填写本文件相关的警告信息
 */

#include "iostream"
#include<iomanip>
#include "string"
#include "ntcip.h"
using namespace std;

CNtcip::CNtcip(int local_port, int remote_port): m_local_port(local_port), m_remote_port(remote_port)
{
	m_pStmf = NULL;
	m_pNet  = NULL;
	m_pCallBackFun = NULL;
	memset(buf, 0, MAX_PACKET);
	//m_pSeries = NULL;
	StartService();
}

CNtcip::~CNtcip()
{
	try
	{
		StopService();
	}
	catch(...)
	{
	}
	m_pStmf = NULL;
	m_pNet  = NULL;
	m_pCallBackFun = NULL;
}

/**	@fn	void StartService()
 *	@brief	<开始网口任务>
 *	@return	none
 */
void CNtcip::StartNet()
{
	m_pNet = new CNet(m_local_port);
	m_pNet->RegisterFun(CNtcip::HandleRequest, this);
	m_pNet->CreateTask();
}

/**	@fn	void StartSeries()
 *	@brief	<开始串口任务>
 *	@return	none
 */
void CNtcip::StartSeries()
{
	//codeing
}

/**	@fn	void StartService()
 *	@brief	<开始任务>
 *	@return	none
 */
void CNtcip::StartService()
{
	StartNet();
	StartSeries();
}

/**	@fn	void StopService()
 *	@brief	<停止任务>
 *	@return	none
 */
void CNtcip::StopService()
{
	if (NULL != m_pNet)
	{
		delete m_pNet;
		m_pNet = NULL;
	}
	/*if (NULL != m_pSeries)
	{
		delete m_pSeries;
		m_pSeries = NULL;
	}*/
}

int CNtcip::SendRequest(const char*pSrc, int nDataLen)
{
	if (NULL == m_pNet || NULL == pSrc)
	{
		return -1;
	}
	int rc = m_pNet->Respond(pSrc, nDataLen);
	return rc;
}

/**	@fn	int Request(PDU *pdu)
 *	@brief	<参数操作请求>
 *	@param pdu[in]  协议数据单元
 *	@return	!=0:失败; 0:成功
 */
int CNtcip::Request(const PDU *pdu)
{
	char type[16] = "";
	int rc = GetProtocol(type, this);
	if (rc < 0)
	{
		return -1;
	}
	char *buf_tmp = new char[MAX_PACKET];
	if (NULL == buf_tmp)
	{
		delete m_pStmf;
		m_pStmf = NULL;
		return -1;
	}
	int nOutDataLen = 0;
	m_pStmf->Encode(pdu, buf_tmp, nOutDataLen);
	SendRequest(buf_tmp, nOutDataLen);
	delete m_pStmf;
	m_pStmf = NULL;
	return 0;
}

/**	@fn	void RegisterFunction(FN_CALLBACK  pFun)
 *	@brief	<注册回调函数>
 *	@param pFun[in]  回调函数地址
 *	@return	!=0:失败; 0:成功
 */
int CNtcip::RegisterFunction(FN_CALLBACK  pFun)
{
	m_pCallBackFun = pFun;
	return 0;
}


/**	@fn	int Trap(const PDU *pdu)
 *	@brief	<trap消息>
 *	@param pdu[in]  ntcip数据
 *	@return	!=0:失败; 0:成功
 */
int CNtcip::Trap(const PDU *pdu)
{
	//coding....
	Request(pdu);
	return 0;
}

/**	@fn	int GetProtocol(const char *pData, CNtcip *pNtcip)
 *	@brief	<获取协议类型>
 *	@param pData[in]  ntcip数据
 *	@param pNtcip[in] ntcip协议处理类
 *	@return	!=0:失败; 0:成功
 */
int CNtcip::GetProtocol(const char *pData, CNtcip *pNtcip)
{
	int rc = 0;
	switch ((BYTE)pData[0])
	{
	case SNMP_SEQUENCE:
		pNtcip->m_pStmf = new CSnmp;
		if (NULL == pNtcip->m_pStmf)
		{
			cout<<"new CSnmp ERROR"<<endl;
			rc = -1;
		}
		cout<<"SNMP......."<<endl;
		break;
	case SFMP_GET_REQ_MSG:
	case SFMP_SET_REQ_MSG:
	case SFMP_SET_REQ_NOPLY_MSG:
		pNtcip->m_pStmf = new CSfmp;
		if (NULL == pNtcip->m_pStmf)
		{
			cout<<"new CSfmp ERROR"<<endl;
			rc = -1;
		}
		cout<<"SFMP......."<<endl;
		break;
	default:
		//log error
		rc = -1;
	}
	return rc;
}

/**	@fn	void HandleRequest(const char *pData, int nDataLen, void *usrData)
 *	@brief	<协议处理>
 *	@param pData[in]  ntcip数据
  *	@param nDataLen[in]  ntcip数据长度
 *	@param usrData[in] 自定义数据
 *	@return	none
 */
void CALLBACK CNtcip::HandleRequest(const char *pData, int nDataLen, void *usrData)
{
	if (NULL == pData || NULL == usrData)
	{
		cout<<"ERROR: pData or usrData is NULL!"<<endl;
		return;
	}
	CNtcip *pNtcip = (CNtcip*)usrData;
	//获取协议类型
	if (0 != pNtcip->GetProtocol(pData, pNtcip))
	{
		cout<<"protocol type error:"<<pData[0]<<endl;
		return;
	}
	PDU pdu;
	memset(&pdu, 0, sizeof(PDU));
	if (0 != pNtcip->m_pStmf->Decode(pData, nDataLen, &pdu)) //Request Error ,decode中动态分配内存给PDU,在本函数最后释放
	{
		cout<<"Decode Error......."<<endl;
		delete pNtcip->m_pStmf;
		return;
	}
	//configure
	if (0 != strcmp((const char*)pdu.oid_name, ASC_BLOCK_OID))
	{
		pNtcip->m_pCallBackFun(&pdu);
	}
	else
	{
		cout<<"block_header info."<<endl;
	}
	cout<<"HandleRequest_1"<<endl;
	BYTE byType = pData[0];  //包类型
	if ((SFMP_SET_REQ_NOPLY_MSG != byType) && (SNMP_TRP_REQ_MSG != pdu.command))     //0xA0 Set No Reply
	{	
		memset(pNtcip->buf, 0, MAX_PACKET);
		int nOutDataLen = 0;
		pNtcip->m_pStmf->Encode(&pdu, pNtcip->buf, nOutDataLen);
	#if 0
 		for (int i=0; i<nDataLen; i++)
		{
			cout<<setw(2)<<setfill('0')<<hex<<((int)pData[i]&0xff)<<" ";
		}
		cout<<dec<<endl;
		pNtcip->m_pNet->Respond(pData, nOutDataLen);
	#else
		cout<<"nOutDataLen:"<<nOutDataLen<<endl;
		pNtcip->m_pNet->Respond(pNtcip->buf, nOutDataLen);
	#endif	
	}
	cout<<"clear pdu....."<<endl;
	//消空pdu中vector表
	pdu.vb.clear();
	delete pNtcip->m_pStmf;
	pNtcip->m_pStmf = NULL;
	cout<<"exit CNtcip::HandleRequest"<<endl;
}
