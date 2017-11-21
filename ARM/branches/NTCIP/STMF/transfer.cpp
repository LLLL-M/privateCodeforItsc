/**	@file transfer.cpp 
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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <memory.h>
#include <iostream>
#include "transfer.h"
using namespace std;

//net
CNet::CNet(int port): m_port(port)
{
	memset(&from_addr, 0, sizeof(sockaddr_in));
	fromlen = sizeof(from_addr);
	m_fd = -1;
	m_pUsrData = NULL;
	m_pCallBackFun = NULL;
	receive_buffer_len = 0;
	memset(receive_buffer, 0, MAX_PACKET);
}

CNet::~CNet()
{
	m_pUsrData = NULL;
	m_pCallBackFun = NULL;
}

/**	@fn	void RegisterFun(CALLBACK_FUN pFun, void *pUsrData)
 *	@brief	<注册回调函数>
 *	@param pFun[in]  回调函数地址
 *	@param pUsrData[in]  用户数据
 *	@return	none
 */
void CNet::RegisterFun(CALLBACK_FUN pFun, void *pUsrData)
{
	m_pCallBackFun = pFun;
	m_pUsrData = pUsrData;
}

/**	@fn	int Respond(const char *pData, int nDataLen)
 *	@brief	<反馈结果>
 *	@param pData[in]  ntcip数据
 *	@param nDataLen[in]  ntcip数据长度
 *	@return	!=0:失败; 0:成功
 */
int CNet::Respond(const char *pData, int nDataLen)
{
	cout<<"Respond.........."<<endl;
	cout<<"nDataLen="<<nDataLen<<endl;
	cout<<"ip:"<<inet_ntoa(from_addr.sin_addr)<<",port:"<<ntohs(from_addr.sin_port)<<endl;
	int rc = sendto(m_fd, (char*)pData, nDataLen, 0, (struct sockaddr*) &from_addr, sizeof(from_addr));
	cout<<"sendto:"<<rc<<endl;
	return rc;
}

/**	@fn	void CreateTask()
 *	@brief	<创建数据接收任务>
 *	@return	none
 */
void CNet::CreateTask()
{	
	pthread_t thread;
	pthread_attr_t attr;
	int stacksize = 1024*1024;
	int rval = pthread_attr_init(&attr);
    if (rval != 0)
    {
        return;
    }
	rval = pthread_attr_setstacksize(&attr, stacksize);
    if (rval != 0)
    {
        pthread_attr_destroy(&attr);
        return;
    }
	if (0 != pthread_create(&thread, &attr, Task_Receive, this))
	{
		//log error
	}
}

/**	@fn	void* Task_Receive(void *arg)
 *	@brief	<数据接收任务函数>
 *	@param arg[in]  用户数据
 *	@return	NULL
 */
void* CNet::Task_Receive(void *arg)
{
	CNet *pNet = (CNet*)arg;
	struct sockaddr_in server_addr;
	int server_len = sizeof(server_addr);
	pNet->m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (pNet->m_fd < 0)
	{
		cout<<"create socket failed"<<endl;
		return NULL;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port = htons(pNet->m_port);
	if (bind(pNet->m_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <0)
	{
		cout<<"bind() failed"<<endl;
		close(pNet->m_fd);
		return NULL;
	}
	while (1)
	{
		do
		{
			pNet->receive_buffer_len = recvfrom(pNet->m_fd, pNet->receive_buffer, 
				MAX_PACKET, 0, (struct sockaddr*)&(pNet->from_addr), (socklen_t*)&pNet->fromlen);
		} while (pNet->receive_buffer_len < 0 && EINTR == errno);
		cout<<"---------------------------------------------------------"<<endl;
		cout<<"receive_buffer_len="<<pNet->receive_buffer_len<<endl;
		pNet->m_pCallBackFun(pNet->receive_buffer, pNet->receive_buffer_len, pNet->m_pUsrData);
	}
}
