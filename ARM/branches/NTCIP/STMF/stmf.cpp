/**	@file stmf.cpp 
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

#include <string>
#include <iostream>
#include "stmf.h"
using namespace std;

static TABLE_OID g_table_oid[] = {
{"4.2.1.1.2.0", 16, 22}, 	//相位
{"4.2.1.1.4.0", 2,  11},
{"4.2.1.2.2.0", 72, 11},  	//检测器
{"4.2.1.4.7.0", 108, 3},    //方案
{"4.2.1.4.9.0", 16, 3},     //绿信比
{"4.2.1.5.3.0", 255, 3},    //动作, timebase
{"4.2.1.7.3.0", 16*4, 1},   //相序
{"4.2.1.8.2.0", 32, 4},    	//通道
{"4.2.1.9.2.0", 16, 5},	  	//跟随相位
{"4.2.6.3.3.2.0", 255, 4}   //调度
};

static OBJECT_INFO g_object_info[] = {
{"4.2.1.1.2.0", 0, 20, 0, 65535},{"4.2.1.1.2.0", 2, 22, 0, 0},  //相位表
{"4.2.1.2.2.0", 0, 4, 0, 65535},  								//检测器表
{"4.2.1.7.3.0", 2, 1, 0, 0},                                    //相序
{"4.2.1.9.2.0", 2, 1, 0, 0},{"4.2.1.9.2.0", 2, 2, 0, 0},        //跟随相位
{"4.2.6.3.3.2.0", 0, 1, 0, 65535},{"4.2.6.3.3.2.0", 1, 3, 0, 2^31}     //调度
};

ASC_BLOCK_HEADER g_stAscBlockHeader = {0};   //块头定义

//snmp procotol
CSnmp::CSnmp()
{}

CSnmp::~CSnmp()
{}

/**	@fn	int ReadNextType(const char *theStringPtr, int &index, unsigned char *theTypePtr)
 *	@brief	<获取数据类型>
 *	@param theStringPtr[in]  ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param index[out]   类型值
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::ReadNextType(const char *theStringPtr, int &index, unsigned char *theTypePtr)
{
    if (NULL == theStringPtr || NULL == theTypePtr)
	{
		return -1;
	}
        
    *theTypePtr = theStringPtr[index++];
    switch (*theTypePtr)
    {
	case ASN_INTEGER:
	case OCTET_STRING:
	case OBJECT_IDENTIFIER:
	case ASN_SEQUENCE:
	case IP_ADDRESS:
	case COUNTER:
	case GAUGE:
	case TIME_TICK:
	case OPAQUE_VALUE:
	case NULL_TYPE:
		return 0;
	default:
		// Unknown Syntax -- but we can continue anyway
		return UNKNOWN_SYNTAX_WARNING;
    }
}

/**	@fn	unsigned long GetLength(const char *pSrc, int &index)
 *	@brief	<获取数据长度>
 *	@param pSrc[in]     ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@return	nDataLen    长度值
 */
unsigned long CSnmp::GetLength(const char *pSrc, int &index)
{
	if (NULL == pSrc)
	{
		return 0;
	}
	BYTE byLenFlag = pSrc[index++];
	unsigned long nDataLen = 0;
	if (byLenFlag & 0x80)		//multi bytes;
	{
		BYTE bylen = byLenFlag & 0x0f;
		for (int i=0; i<bylen; i++)
		{
			nDataLen += pSrc[index+i]<<((bylen-i-1)*8); 
		}
		index += bylen;
	}
	else
	{
		nDataLen = byLenFlag;
	}
	return nDataLen;
}

/**	@fn	int GetVersion(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取协议版本>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括版本信息)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetVersion(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	unsigned long nDataLen = 0;
	BYTE byType = 0;
	nDataLen = GetLength(pSrc, index);
	cout<<"GetVersion_len:"<<nDataLen<<endl;
	if (0 != ReadNextType(pSrc, index, &byType))
	{
		return -1;
	}
	//the length of version
	//the value of version
	index += 2;
	cout<<"GetVersion_index:"<<index<<endl;
	return 0;
}

/**	@fn	int GetCommunity(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取团体名>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括团体名)
 *	@return	!=0:失败; 0:成功
 */
 int CSnmp::GetCommunity(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	BYTE byType = 0;
	if (0 != ReadNextType(pSrc, index, &byType))
	{
		return -1;
	}
	BYTE byLen = pSrc[index++];
	cout<<"index:"<<index<<" GetCommunity_len:"<<(int)byLen<<" type:"<<(int)byType<<endl;
	memcpy(pOut->oid_name, pSrc+index, byLen);
	index += byLen;
	pOut->enterprise_length = byLen; //用于存储团体名长度
	return 0;
}

/**	@fn	int GetCommand(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取操作类型>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括操作类型)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetCommand(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	pOut->command = pSrc[index++]&0xff;
	cout<<"command:0x"<<hex<<pOut->command<<" index:"<<dec<<index<<endl;
	switch (pOut->command)
	{
	case SNMP_GET_REQ_MSG:
	case SNMP_GETNEXT_REQ_MSG:
	case SNMP_SET_REQ_MSG:
		return 0;
	default:
		break;
	}
	return -1;
}

/**	@fn	unsigned long GetValue(const char *pSrc, int &index)
 *	@brief	<获取数据值>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@return	value       数据值
 */
int CSnmp::GetValue(const char *pSrc, int &index)
{
	if (NULL == pSrc)
	{
		return -1;
	}
	//unsigned long nDataLen = 0;
	//nDataLen = GetLength(pSrc, index);
	BYTE byType = 0;
	if (0 != ReadNextType(pSrc, index, &byType))
	{
		return -1;
	}
	BYTE bylen = GetLength(pSrc, index);
	unsigned long value = 0;
	for (int i=0; i<bylen; i++)
	{
		value += pSrc[index+i]<<((bylen-i-1)*8); 
	}
	index += bylen;
	return value;
}

/**	@fn	int GetCommand(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取请求ID>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括操作类型)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetRequestID(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	pOut->reqid = GetValue(pSrc, index);
	cout<<"pOut->reqid: "<<pOut->reqid<<", index: "<<index<<endl;
	return 0;
}

/**	@fn	int GetErrorStatus(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取错误状态>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括操作类型)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetErrorStatus(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	pOut->errstat = GetValue(pSrc, index);
	cout<<"pOut->errstat: "<<pOut->errstat<<", index: "<<index<<endl;
	return 0;
}

/**	@fn	int GetErrorIndex(const char *pSrc, int &index, PDU *pOut)
 *	@brief	<获取错误索引>
 *	@param pSrc[in]  	ntcip数据
 *	@param index[in]  	ntcip数据索引
 *	@param pOut[out]    协议数据单元(包括操作类型)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetErrorIndex(const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	pOut->errindex = GetValue(pSrc, index);
	cout<<"pOut->errindex: "<<pOut->errindex<<", index: "<<index<<endl;
	return 0;
}

/**	@fn	int GetPDU(const char *pSrc, int nDataLen, int &index, PDU *pOut)
 *	@brief	<获取协议数据单元>
 *	@param pSrc[in]  		ntcip数据
  *	@param nDataLen[in]  	ntcip数据长度
 *	@param index[in]  		ntcip数据索引
 *	@param pOut[out]    	协议数据单元(包括操作类型)
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::GetPDU(const char *pSrc, int nDataLen, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut || nDataLen <= 0)
	{
		return -1;
	}
	BYTE byType = 0;
	int count = 0;
	if (0 != ReadNextType(pSrc, index, &byType))
	{
		return -2;
	}
	if (ASN_SEQUENCE != byType)
	{
		cout<<"GetPDU error, type:0x"<<hex<<byType<<dec<<endl;
		return -3;
	}
	unsigned long nLen = 0;
	nLen = GetLength(pSrc, index);
	if (nLen == 0)
	{
		return -4;
	}
	//PDU
	while (index < nDataLen)
	{
		struct variable_list  stVB = {0};
		if (0 != ReadNextType(pSrc, index, &byType))
		{
			return -5;
		}
		if (ASN_SEQUENCE != byType)
		{
			cout<<"GetPDU error:index:%d, type:0x"<<count<<hex<<byType<<dec<<endl;
			return -6;
		}
		unsigned long Len = GetLength(pSrc, index);
		if (0 != ReadNextType(pSrc, index, &byType))
		{
			return -7;
		}
		Len = GetLength(pSrc, index);
		//OID
		memcpy(stVB.name, pSrc+index, Len);
		index += Len;
		//value
		if (0 != ReadNextType(pSrc, index, &byType))
		{
			return -8;
		}
		Len = GetLength(pSrc, index);
		stVB.type = byType;
		stVB.value_len = Len;
		if (Len > 0)
		{
			stVB.unValue.nValue = pSrc[index++];
		}
		pOut->vb.push_back(stVB);
	}
	return 0;
}

/**	@fn	int GetPDU(const char *pSrc, int nDataLen, int &index, PDU *pOut)
 *	@brief	<SNMP解码>
 *	@param pSrc[in]  		ntcip数据
 *	@param nDataLen[in]  	ntcip数据长度
 *	@param pOut[out]    	协议数据单元
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::Decode(const char *pSrc, int nDataLen, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	int rc = -1;
	//协议头
	int index = 0;
	BYTE byType = pSrc[index++];
	if (ASN_SEQUENCE != byType)
	{
		cout<<"SNMP data error, type:0x"<<hex<<byType<<dec<<endl;
		return -2;
	}
	
	//版本号
	rc = GetVersion(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP version error"<<endl;
		return -3;
	}	
	
	//团体名
	rc = GetCommunity(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP Community error"<<endl;
		return -4;
	}
	
	//操作类型
	rc = GetCommand(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP Command error"<<endl;
		return -5;
	}
	
	if (GetLength(pSrc, index) <= 0)
	{
		cout<<"SNMP GetLength error"<<endl;
		return -6;
	}
	
	//请求ID
	rc = GetRequestID(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP GetRequestID error"<<endl;
		return -7;
	}
	
	//错误状态
	rc = GetErrorStatus(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP GetErrorStatus error"<<endl;
		return -8;
	}
	
	//错误索引
	rc = GetErrorIndex(pSrc, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP GetErrorIndex error"<<endl;
		return -9;
	}
	
	//PDU
	rc = GetPDU(pSrc, nDataLen, index, pOut);
	if (rc != 0)
	{
		cout<<"SNMP GetPDU error"<<endl;
		return -10;
	}
	return 0;
}

/**	@fn	int GetSnmpLength(string *theLengthPtr, unsigned long theNum)
 *	@brief	<将长度编码>
 *	@param theLengthPtr[out]  	编码后长度数据
 *	@param theNum[in]  	长度值
 *	@return	!=0:失败; 0:成功
 */
void CSnmp::GetSnmpLength(string *theLengthPtr, unsigned long theNum)
{
	if (NULL == theLengthPtr)
	{
		return;
	}
    string				length_str;
	string 				strRC;
    unsigned long		temp = 0;
    char		next_byte = 0;
	char		counter = 0;
	
    if (theLengthPtr->length())
	{
		cout<<"The GetStmpLength Function was not passed an empty string"<<endl;
		return;
	}
    if (theNum < 128)
	{
		strRC = theLengthPtr->append(1,(unsigned char) theNum); // only put on one byte
		if (strRC.length() <= 0)
		{
			return;
		}
	}   
    else
    {
        while (theNum)
        {
   	        temp = theNum >> 8;
		    next_byte = theNum - temp*256; // next byte = the 8 lowest order bits
            strRC = theLengthPtr->insert((unsigned long)0, (unsigned long)1, next_byte);
			if (strRC.length() <= 0)
			{
				return;
			}
            theNum = temp;						// remove the lowest 8 bits from theNum
		    counter++;							// count the bytes
        }
        counter += 128; 						// Add the length code
        strRC = theLengthPtr->insert ((unsigned long)0, (unsigned long)1, counter);
		if (strRC.length() <= 0)
		{
			return;
		}
	}
}

/**	@fn	int EncodePdu(const PDU *pIn, string &pdu)
 *	@brief	<PDU编码>
 *	@param pIn[in]  	编码前PDU数据
 *	@param pdu[out]  	编码后PDU数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodePdu(const PDU *pIn, string &pdu)
{
	if (NULL == pIn || pIn->vb.size() == 0)
	{
		return -1;
	}
	string strRC;
	for (unsigned int i=0; i<pIn->vb.size(); i++)
	{
		string strtmp, pdu_len;
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, (char)pIn->vb[i].unValue.nValue);
		if (strRC.length() <= 0)
		{
			return -2;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, (char)pIn->vb[i].value_len);
		if (strRC.length() <= 0)
		{
			return -3;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, (char)pIn->vb[i].type);
		if (strRC.length() <= 0)
		{
			return -4;
		}
		strRC = strtmp.insert(0, (char*)pIn->vb[i].name, strlen((const char*)pIn->vb[i].name)+1);
		if (strRC.length() <= 0)
		{
			return -5;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, strlen((const char*)pIn->vb[i].name)+1);
		if (strRC.length() <= 0)
		{
			return -6;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, '\x06');
		if (strRC.length() <= 0)
		{
			return -7;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, (char)strtmp.length());
		if (strRC.length() <= 0)
		{
			return -8;
		}
		strRC = strtmp.insert((unsigned long)0, (unsigned long)1, (char)ASN_SEQUENCE);
		if (strRC.length() <= 0)
		{
			return -9;
		}
		strRC = pdu.append(strtmp);
		if (strRC.length() <= 0)
		{
			return -10;
		}
	}
	string pdu_len;
	GetSnmpLength(&pdu_len, pdu.length());
	strRC = pdu.insert(0, pdu_len, 0, pdu_len.length());
	if (strRC.length() <= 0)
	{
		return -11;
	}
	strRC = pdu.insert((unsigned long)0, (unsigned long)1, (char)ASN_SEQUENCE);
	if (strRC.length() <= 0)
	{
		return -12;
	}
	return 0;
}

/**	@fn	int EncodeErrindex(const PDU *pIn, string &err_index)
 *	@brief	<错误索引编码>
 *	@param pIn[in]  		编码前PDU数据
 *	@param err_index[out]  	编码后错误索引数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodeErrindex(const PDU *pIn, string &err_index)
{
	if (NULL == pIn)
	{
		return -1;
	}
	string strRC;
	strRC = err_index.insert((unsigned long)0, (unsigned long)1, (char)pIn->errindex);
	if (strRC.length() <= 0)
	{
		return -2;
	}
	strRC = err_index.insert((unsigned long)0, (unsigned long)1, (char)1);
	if (strRC.length() <= 0)
	{
		return -3;
	}
	strRC = err_index.insert((unsigned long)0, (unsigned long)1, (char)ASN_INTEGER);
	if (strRC.length() <= 0)
	{
		return -4;
	}
	return 0;
}

/**	@fn	int EncodeErrstatus(const PDU *pIn, string &err_status)
 *	@brief	<错误状态编码>
 *	@param pIn[in]  			编码前PDU数据
 *	@param err_status[out]  	编码后错误状态数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodeErrstatus(const PDU *pIn, string &err_status)
{
	if (NULL == pIn)
	{
		return -1;
	}
	string strRC;
	unsigned long len = err_status.length();
	strRC = err_status.insert((unsigned long)0, (unsigned long)1, (char)pIn->errstat);
	if (strRC.length() <= len)
	{
		return -2;
	}
	len = err_status.length();
	strRC = err_status.insert((unsigned long)0, (unsigned long)1, (char)1);
	if (strRC.length() <= len)
	{
		return -3;
	}
	len = err_status.length();
	strRC = err_status.insert((unsigned long)0, (unsigned long)1, (char)ASN_INTEGER);
	if (strRC.length() <= len)
	{
		return -4;
	}
	
	return 0;
}

/**	@fn	int EncodeRequestID(const PDU *pIn, string &requestid)
 *	@brief	<请求ID编码>
 *	@param pIn[in]  		编码前PDU数据
 *	@param requestid[out]  	编码后请求ID数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodeRequestID(const PDU *pIn, string &requestid)
{
	if (NULL == pIn)
	{
		return -1;
	}
	string strRC;
	unsigned long len = requestid.length();
	strRC = requestid.insert((unsigned long)0, (unsigned long)1, (char)pIn->reqid);
	if (strRC.length() <= len)
	{
		return -2;
	}
	len = requestid.length();
	
	if (pIn->reqid < 128)
	{
		strRC = requestid.insert((unsigned long)0, (unsigned long)1, (char)1);
		if (strRC.length() <= len)
		{
			return -3;
		}
		len = requestid.length();
	}
	else
	{
		strRC = requestid.insert((unsigned long)0, (unsigned long)1, (char)0);
		if (strRC.length() <= len)
		{
			return -4;
		}
		len = requestid.length();
		strRC = requestid.insert((unsigned long)0, (unsigned long)1, (char)2);
		if (strRC.length() <= len)
		{
			return -5;
		}
		len = requestid.length();
	}
	strRC = requestid.insert((unsigned long)0, (unsigned long)1, (char)2);
	if (strRC.length() <= len)
	{
		return -6;
	}
	return 0;
}

/**	@fn	int EncodeRequestID(const PDU *pIn, string &requestid)
 *	@brief	<团体名编码>
 *	@param pIn[in]  		编码前PDU数据
 *	@param str_snmp[out]  	编码后团体名数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodeCommunity(const PDU *pIn, string &str_snmp)
{
	if (NULL == pIn)
	{
		return -1;
	}
	string strRC;
	unsigned long len = str_snmp.length();
	strRC = str_snmp.insert(0, (const char*)pIn->oid_name, 0, pIn->enterprise_length);
	if (strRC.length() <= len)
	{
		return -2;
	}
	len = str_snmp.length();
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)pIn->enterprise_length);
	if (strRC.length() <= len)
	{
		return -3;
	}
	len = str_snmp.length();
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)4);
	if (strRC.length() <= len)
	{
		return -4;
	}
	return 0;
}

/**	@fn	int EncodeVersion(const PDU *pIn, string &str_snmp)
 *	@brief	<版本编码>
 *	@param pIn[in]  		编码前PDU数据
 *	@param str_snmp[out]  	编码后版本数据
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::EncodeVersion(const PDU *pIn, string &str_snmp)
{
	if (NULL == pIn)
	{
		return -1;
	}
	string strRC;
	unsigned long len = str_snmp.length();
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)0);
	if (strRC.length() <= len)
	{
		return -2;
	}
	len = str_snmp.length();
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)1);
	if (strRC.length() <= len)
	{
		return -3;
	}
	len = str_snmp.length();
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)2);
	if (strRC.length() <= len)
	{
		return -4;
	}
	return 0;
}

/**	@fn	int Encode(const PDU *pIn, char *pOut, int &nDataLen)
 *	@brief	<SNMP数据编码>
 *	@param pIn[in]  		编码前PDU数据
 *	@param pOut[out]  		编码后数据
 *	@param nDataLen[out]  	编码后数据长度
 *	@return	!=0:失败; 0:成功
 */
int CSnmp::Encode(const PDU *pIn, char *pOut, int &nDataLen)
{
	if (NULL == pIn || NULL == pOut)
	{
		return -1;
	}
	//PDU
	string str_snmp;
	string pdu;
	
	int rc = EncodePdu(pIn, pdu);
	if (0 != rc)
	{
		return -2;
	}
	//error index
	string err_index;
	rc = EncodeErrindex(pIn, err_index);
	if (0 != rc)
	{
		return -3;
	}
	//error status
	string err_status;
	rc = EncodeErrstatus(pIn, err_status);
	if (0 != rc)
	{
		return -4;
	}
	//request id
	string requestid;
	rc = EncodeRequestID(pIn, requestid);
	if (0 != rc)
	{
		return -5;
	}
	string strRC;
	//length 1
	string str_1;
	unsigned long len = str_1.length();
	strRC = str_1.append(requestid);
	if (strRC.length() <= len)
	{
		return -6;
	}
	len = str_1.length();
	strRC = str_1.append(err_status);
	if (strRC.length() <= len)
	{
		return -7;
	}
	len = str_1.length();
	strRC = str_1.append(err_index);
	if (strRC.length() <= len)
	{
		return -8;
	}
	len = str_1.length();
	strRC = str_1.append(pdu);
	if (strRC.length() <= len)
	{
		return -9;
	}

	string str_1_len;
	GetSnmpLength(&str_1_len, str_1.length());
	len = str_snmp.length();
	strRC = str_snmp.append(str_1_len);
	if (strRC.length() <= len)
	{
		return -10;
	}
	len = str_snmp.length();
	strRC = str_snmp.append(str_1);
	if (strRC.length() <= len)
	{
		return -11;
	}
	//command
	if (SNMP_GET_REQ_MSG == pIn->command || SNMP_GETNEXT_REQ_MSG == pIn->command || SNMP_SET_REQ_MSG == pIn->command)
	{
		len = str_snmp.length();
		strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)SNMP_GET_RSP_MSG);
		if (strRC.length() <= len)
		{
			return -12;
		}
	}

	//community
	rc = EncodeCommunity(pIn, str_snmp);
	if (0 != rc)
	{
		return -13;
	}
	//version
	rc = EncodeVersion(pIn, str_snmp);
	if (0 != rc)
	{
		return -14;
	}
	//length 2
	string str_len;
	GetSnmpLength(&str_len, str_snmp.length());
	len = str_snmp.length();
	strRC = str_snmp.insert(0, str_len, 0, str_len.length());
	if (strRC.length() <= len)
	{
		return -15;
	}
	len = str_snmp.length();
	//header type
	strRC = str_snmp.insert((unsigned long)0, (unsigned long)1, (char)ASN_SEQUENCE);
	if (strRC.length() <= len)
	{
		return -16;
	}
	len = str_snmp.length();
	unsigned long count = str_snmp.copy(pOut, str_snmp.length(), 0);
	if (count < len)
	{
		return -17;
	}
	nDataLen = str_snmp.length();
	return 0;
}

/************************************************SFMP PROTOCOL INTERFACE******************************************************/
//sfmp procotol
CSfmp::CSfmp()
{
}

CSfmp::~CSfmp()
{
}

/**	@fn	bool IsTableOid(const char *pOidKeyStr, BYTE *pRowNum, BYTE *pColumNum)
 *	@brief	<判断是否为表>
 *	@param pOidKeyStr[in]  		OID
 *	@param pRowNum[out]  		表行数
 *	@param pColumNum[out]  		表列数
 *	@return	false:失败; true:成功
 */
bool CSfmp::IsTableOid(const char *pOidKeyStr, BYTE *pRowNum, BYTE *pColumNum)
{
	if (NULL == pOidKeyStr || NULL == pRowNum || NULL == pColumNum)
	{
		return false;
	}
	//如果OID不在表范围则返回假,否则为真
	int n = sizeof(g_table_oid) / sizeof(TABLE_OID);
	for (int i=0; i<n; i++)
	{
		if (0 == strcmp((const char*)g_table_oid[i].byOid, pOidKeyStr))
		{
			*pRowNum = g_table_oid[i].byRowNum;
			*pColumNum = g_table_oid[i].byNum;
			return true;
		}
	}
	return false;
}

/**	@fn	int IsSpecialType(BYTE byIndex, const char *pOidKeyStr)
 *	@brief	<判断列字段是否需要特殊处理>
 *	@param byIndex[in]  	列字段索引
 *	@param pOidKeyStr[in]  	OID
 *	@return	<0:失败; >=0:成功
 */
int CSfmp::IsSpecialType(BYTE byIndex, const char *pOidKeyStr)
{
	if (NULL == pOidKeyStr)
	{
		return -1;
	}
	int n = sizeof(g_object_info) / sizeof(OBJECT_INFO);
	for (int i=0; i<n; i++)
	{
		if ((0 == strcmp((const char*)g_object_info[i].byOid, pOidKeyStr)) && (g_object_info[i].byIndex == byIndex))
		{
			return i;
		}
	}
	return -1;
}

/**	@fn	void Oid_To_String(const char *oid, int oid_len, char *stroid)
 *	@brief	<oid转字符串>
 *	@param oid[in]  	OID
  *	@param oid_len[in]  OID长度
 *	@param stroid[in]  	字符串
 *	@return	none
 */
int CSfmp::Oid_To_String(const char *poid, int oid_len, char *stroid)
{
	if (NULL == poid || NULL == stroid)
	{
		return -1;
	}
	int j = 0;
	int i = 0;
	for (i=0; i<oid_len; i++)
	{
		printf("(%d) ", poid[i]);
		if (poid[i]>=10 && poid[i] <=15)
		{
			stroid[j++] = poid[i]-10+'A';
		}
		else
		{
			stroid[j++] = poid[i]+'0';
		}
		
		if (i == oid_len -1)
		{
			break;
		}
		stroid[j++] = '.';
	}
	return 0;
}

/**	@fn	int ParseParamter(BYTE byColumIndex, const char *pSrc, int &index, PDU *pOut)
 *	@brief	<解析参数>
 *	@param byColumIndex[in]  	列字段索引
 *	@param pSrc[in]  			NTCIP数据
 *	@param index[in]  			NTCIP数据索引
 *	@param pOut[in]  			协议数据单元
 *	@return	!0:失败; 0:成功
 */
int CSfmp::ParseParamter(BYTE byColumIndex, const char *pSrc, int &index, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	variable_list vb;
	memset(&vb, 0, sizeof(variable_list));
	int special_table_index = IsSpecialType(byColumIndex, (const char*)pOut->oid_name);
	if (special_table_index < 0)
	{
		vb.unValue.nValue = pSrc[index++];
		vb.type = ASN_INTEGER;
		vb.value_len = 1;
	}
	else
	{
		printf("(j=%d)need to handle specially, buffer_index=%d, special_table_index:%d\n", byColumIndex, index, special_table_index);
		switch (g_object_info[special_table_index].byType)
		{
		case 0:
			vb.value_len = 2;
			vb.type = ASN_INTEGER;
			vb.unValue.nValue = (pSrc[index] << 8) | pSrc[index+1];
			break;
		case 1:
			vb.value_len = 4;
			vb.type = ASN_INTEGER;
			vb.unValue.nValue = (pSrc[index] << 24) |(pSrc[index+1] << 16) |(pSrc[index+2] << 8) | pSrc[index+3];
			break;
		case 2:
			vb.value_len = pSrc[index++];
			vb.type = OCTET_STRING;
			memcpy(vb.unValue.byValue, pSrc+index, vb.value_len);
			for (unsigned int i=0; i<vb.value_len; i++)
			{
				printf("%02x ", vb.unValue.byValue[i]);
			}
			printf("\n");
			break;
		default:
			break;
		}								
		index += vb.value_len;
		printf("need to handle specially.\n");
	}
	pOut->vb.push_back(vb);
	return 0;
}

/**	@fn	int ParseSequence(const char *pSrc, int index_, BYTE byColumNum_, PDU *pOut)
 *	@brief	<解析序列>
 *	@param pSrc[in]  			NTCIP数据
 *	@param index_[in]  			NTCIP数据索引
 *	@param byColumNum_[in]  	列字段个数
 *	@param pOut[in]  			协议数据单元
 *	@return	!0:失败; 0:成功
 */
int CSfmp::ParseSequence(const char *pSrc, int index_, BYTE byColumNum_, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	int index = index_;
	BYTE byColumNum = byColumNum_;
	BYTE bySeqByte = pSrc[index++]; //行数字段所占字节个数
	BYTE bySeqNum = pSrc[index++];  //表中的行数
	printf("bySeqByte=%d, bySeqNum=%d, byColumNum=%d\n", bySeqByte, bySeqNum, byColumNum);
	for (BYTE i=0; i<bySeqNum; i++)
	{
		BYTE byPreamble = pSrc[index++]; 
		printf("\nbyPreamble=%d, index=%d\n", byPreamble, index);
 		for (BYTE j=0; j<byColumNum; j++)  //j=0相位号
		{
			if (0 != ParseParamter(j+1, pSrc, index, pOut))
			{
				return -2;
			}
		}	 	
	}
	return 0;
}

/**	@fn	int ParseBlockHeader(const char *pSrc, int &index)
 *	@brief	<解析数据块头>
 *	@param pSrc[in]  			NTCIP数据
 *	@param index[in]  			NTCIP数据索引
 *	@return	!0:失败; 0:成功
 */
int CSfmp::ParseBlockHeader(const char *pSrc, int &index)
{
	if (NULL == pSrc)
	{
		return -1;
	}
	int len = pSrc[index++];
	int end = index + len -1;
	cout<<"index:"<<index<<"end"<<end<<"len:"<<len<<endl;
	//oid
	BYTE bylen = pSrc[index++];
	memcpy(g_stAscBlockHeader.ascBlockDataID, pSrc+index, bylen);
	for (int i=0; i<bylen; i++)
	{
		printf("[%d] ", g_stAscBlockHeader.ascBlockDataID[i]);
	}
	cout<<endl;
	index += bylen;
	cout<<"index"<<index<<endl;
	int i = 0;
	while (index < end)
	{
		bylen = pSrc[index++];
		printf("bylen=%d\n", bylen);
		g_stAscBlockHeader.ascBlockIndex[i] = pSrc[index++];
		bylen = pSrc[index++];
		printf("bylen=%d\n", bylen);
		g_stAscBlockHeader.ascBlockQuantity[i] = pSrc[index++];
		printf("%d,%d\n", g_stAscBlockHeader.ascBlockIndex[i], g_stAscBlockHeader.ascBlockQuantity[i]);
		g_stAscBlockHeader.ascBlockDataCount++;
		i++;
	}
	printf("ascBlockDataCount:%d,i:%d\n", g_stAscBlockHeader.ascBlockDataCount, i);
	return 0;
}

/**	@fn	int CheckBlock(char *oid, int oid_len)
 *	@brief	<检验数据体与数据头是否匹配>
 *	@param oid[in]  			OID
 *	@param oid_len[in]  		OID长度
 *	@return	!0:失败; 0:成功
 */
int CSfmp::CheckBlock(char *poid, int oid_len)
{
	if (NULL == poid)
	{
		return -1;
	}
	printf("header_oid:");
	for (int i=0; i<oid_len; i++)
	{
		printf("%d ", g_stAscBlockHeader.ascBlockDataID[i]);
 		if (g_stAscBlockHeader.ascBlockDataID[i] != poid[i])
		{
			return -1;
		} 
	}
	printf("\n");
	return 0;
}

/**	@fn	int Decode(const char *pSrc, int nDataLen, PDU *pOut)
 *	@brief	<SFMP解码>
 *	@param pSrc[in]  		NTCIP数据
  *	@param nDataLen[in]  	NTCIP数据长度
 *	@param pOut[out]  		协议数据单元
 *	@return	!0:失败; 0:成功
 */
int CSfmp::Decode(const char *pSrc, int nDataLen, PDU *pOut)
{
	if (NULL == pSrc || NULL == pOut)
	{
		return -1;
	}
	printf("CSfmp::Decode_1\n");
	//解析操作类型
	int index = 0;
	int rc = -1;
	pOut->command = pSrc[index++]&0xff;
	BYTE byPreamble = pSrc[index++];
	printf("CSfmp::Decode_2\n");
	if (byPreamble & 32)  //非默认团体名
	{
		int name_len = pSrc[index++];
		memcpy(pOut->enterprise, (char*)pSrc+index, name_len);
		pOut->enterprise_length = name_len;
		index += name_len;
		//request
		pOut->reqid = pSrc[index++];
	}
	else
	{
		//request
		pOut->reqid = pSrc[index++];
	}
	printf("CSfmp::Decode_3\n");
	printf("index=%d\n", index);
	int oid_len = pSrc[index++];
	printf("command=%x,reqid=%x,oid_len=%x, index=%d\n", pOut->command, pOut->reqid, oid_len, index);
	char oid_tmp[64]="";
	char oid_new[64]="";
	memcpy(oid_tmp, (char*)pSrc+index, oid_len);
	printf("CSfmp::Decode_4\n");
	index += oid_len;
	printf("CSfmp::Decode_5\n");
	//OID
	rc = Oid_To_String(oid_tmp, oid_len, oid_new);
	if (0 != rc)
	{
		return -2;
	}
	strcpy((char*)pOut->oid_name, oid_new);
	printf("oid_new:%s\n", oid_new);
	
	BYTE byColumNum = 0;
	BYTE byRowNum = 0;
	if (IsTableOid(oid_new, &byRowNum, &byColumNum))
	{
		//按表解析
		printf("this is a table_oid.\n");
		//先判断与块头信息是否一致
 		if (CheckBlock(oid_tmp, oid_len) != 0)
		{
			printf("block_oid is not same as header_oid.\n");
			return -1;
		}
		if (SFMP_GET_REQ_MSG == pOut->command)
		{
			printf("SFMP_GET_REQ_MSG == pOut->command\n");
			return 0;
		}
		rc = ParseSequence(pSrc, index, byColumNum, pOut);
		if (0 != rc)
		{
			return -3;
		}
	}
	else
	{
		printf("this is not a table_oid.\n");
		if (0 == strcmp(oid_new, ASC_BLOCK_OID))
		{
			rc = ParseBlockHeader(pSrc, index);
			if (0 != rc)
			{
				return -4;
			}
		}
		else
		{
			rc = ParseParamter(0, pSrc, index, pOut);
			if (0 != rc)
			{
				return -5;
			}
		}
	}
	return 0;
}

/**	@fn	BYTE EncodePreamble(const PDU *pIn)
 *	@brief	<Preamble编码>
 *	@param pSrc[in]  		NTCIP数据
 *	@return	preamble
 */
BYTE CSfmp::EncodePreamble(const PDU *pIn)
{
	if (NULL == pIn)
	{
		return 0;
	}
	BYTE preamble = 0;
	//extension
	//version
	//community name
	if (strlen((const char*)pIn->enterprise) > 0)  
	{
		preamble |= 32;
	}
	//request number
	preamble |= 16;
	//error-data
	if (pIn->errstat !=0 || pIn->errindex != 0)
	{
		preamble |= 8;
	}
	//message-oid
	/*if (strlen((const char*)pIn->oid_name) > 0)
	{
		preamble |= 4;
	}*/
	//data
	if (pIn->vb.size() > 0)
	{
		if (SFMP_SET_REQ_MSG != pIn->command)
		{
			preamble |= 2;
		}	
	}
	//reserved
	return preamble;
}

/**	@fn	int StrOidTrimDot(const char *strOid, char *oid)
 *	@brief	<点分字符串转换OID>
 *	@param strOid[in]  		点分字符串
 *	@param oid[out]  		OID
 *	@return	OID长度
 */
int CSfmp::StrOidTrimDot(const char *strOid, char *poid)
{
	if (NULL == strOid || NULL == poid)
	{
		return -1;
	}
	int count = 0;
	for (int i=0; strOid[i]!='\0'; i++)
	{
		if (strOid[i] != '.')
		{
			poid[i] = strOid[i] - '0';
			count++;
		}	
	}
	return count;
}

/**	@fn	int EncodeParameter(BYTE byColumIndex, int vec_index, int &index, const PDU *pIn, char *pOut)
 *	@brief	<SFMP参数编码>
 *	@param byColumIndex[in]  列字段索引
 *	@param vec_index[in]  	列字段在VECTOR中的索引
 *	@param index[out]  		NTCIP数据索引
 *	@param pIn[in]  		PDU数据
 *	@param pOut[out]  		编码后数据
 *	@return	!0:失败; 0:成功
 */
int CSfmp::EncodeParameter(BYTE byColumIndex, int vec_index, int &index, const PDU *pIn, char *pOut)
{
	if (NULL == pIn || NULL == pOut)
	{
		return -1;
	}
	
	int special_table_index = IsSpecialType(byColumIndex, (const char*)pIn->oid_name);
	if (special_table_index < 0)
	{
		pOut[index++] = pIn->vb[vec_index].unValue.nValue;
	}
	else
	{
		switch (g_object_info[special_table_index].byType)
		{
		case 0:   //2 bytes
			pOut[index++] = (pIn->vb[vec_index].unValue.nValue >> 8) & 0xff;
			pOut[index++] = pIn->vb[vec_index].unValue.nValue & 0xff;
			break;
		case 1: // 4 bytes;
			pOut[index++] = (pIn->vb[vec_index].unValue.nValue >> 24) & 0xff;
			pOut[index++] = (pIn->vb[vec_index].unValue.nValue >> 16) & 0xff;
			pOut[index++] = (pIn->vb[vec_index].unValue.nValue >> 8) & 0xff;
			pOut[index++] = pIn->vb[vec_index].unValue.nValue & 0xff;
			break;
		case 2:
			pOut[index++] = pIn->vb[vec_index].value_len;
			memcpy(pOut+index, pIn->vb[vec_index].unValue.byValue, pIn->vb[vec_index].value_len);
			break;
		default:
			break;
		}								
		index += pIn->vb[vec_index].value_len;
	}
	return 0;
}

/**	@fn	int EncodeSequence(const PDU *pIn, int &index, BYTE &byRowNum, BYTE &byColumNum, char *pOut)
 *	@brief	<SFMP序列编码>
 *	@param pIn[in]  		PDU数据
 *	@param index[out]  		NTCIP数据索引
 *	@param byRowNum[in]  	行数
 *	@param byColumNum[in]  	列字段个数
 *	@param pOut[out]  		编码后数据
 *	@return	!0:失败; 0:成功
 */
int CSfmp::EncodeSequence(const PDU *pIn, int &index, BYTE &byRowNum, BYTE &byColumNum, char *pOut)
{
	if (NULL == pIn || pIn->vb.size() <= 0)
	{
		return -1;
	}
	int vec_index = 0;
	for (int i=0; i<byRowNum; i++)
	{
		pOut[index++] = 0;
		for (int j=0; j<byColumNum; j++)
		{
			if (0 != EncodeParameter(j+1, vec_index, index, pIn, pOut))
			{
				return -2;
			}
			vec_index++;
		}
	}
	return 0;
}

/**	@fn	int EncodeHeader(unsigned long command, int &index, char *pOut)
 *	@brief	<SFMP数据包头编码>
 *	@param command[in]  	反馈的数据类型
 *	@param index[out]  		NTCIP数据索引
 *	@param pOut[out]  		编码后数据
 *	@return	!0:失败; 0:成功
 */
int CSfmp::EncodeHeader(unsigned long command, int &index, char *pOut)
{
	if (NULL == pOut)
	{
		return -1;
	}
	switch (command)
	{
	case SFMP_GET_REQ_MSG:
		pOut[index++] = (char)SFMP_GET_RSP_MSG;
		break;
	case SFMP_SET_REQ_MSG:
		pOut[index++] = (char)SFMP_SET_RSP_MSG;
		break;
	default:
		break;
	}
	return 0;
}

/**	@fn	int Encode(const PDU *pIn, char *pOut, int &nDataLen)
 *	@brief	<SFMP数据编码>
 *	@param pIn[in]  		PDU
 *	@param nDataLen[out]  	NTCIP数据
 *	@param pOut[out]  		NTCIP数据长度
 *	@return	!0:失败; 0:成功
 */
int CSfmp::Encode(const PDU *pIn, char *pOut, int &nDataLen)
{
	if (NULL == pIn || NULL == pOut)
	{
		return -1;
	}
	printf("CSfmp::Encode_1\n");
	BYTE byPreamble = 0;
	int index = 0;  //buf索引
	int rc = EncodeHeader(pIn->command, index, pOut);
	if (0 != rc)
	{
		return -2;
	}
	printf("CSfmp::Encode_2\n");
	byPreamble = EncodePreamble(pIn);
	printf("CSfmp::Encode_3\n");
	pOut[index++] = byPreamble;
	if (byPreamble & 32)  //非默认团体名
	{
		pOut[index++] = pIn->enterprise_length;
		memcpy(pOut+index, pIn->enterprise, pIn->enterprise_length);
		index += pIn->enterprise_length;
	}
	printf("CSfmp::Encode_4\n");
	//request
	if (byPreamble & 16)
	{
		pOut[index++] = pIn->reqid;
	}
	printf("CSfmp::Encode_5\n");
	//error-data
	if (SFMP_SET_REQ_MSG == pIn->command)
	{
		nDataLen = 3;
		return 0;
	}
	printf("CSfmp::Encode_6\n");
	//oid
	if (byPreamble & 4)
	{
		char oid_tmp[32]="";
		int oid_len = StrOidTrimDot((const char*)pIn->oid_name, oid_tmp);	// 去除'.'后的长度
		pOut[index++] = oid_len;
		memcpy(pOut+index, oid_tmp, oid_len);
		index += oid_len;
	}
	printf("CSfmp::Encode_7\n");
	//data
	BYTE byRowNum = 0;
	BYTE byColumNum = 0;
	if (IsTableOid((const char*)pIn->oid_name, &byRowNum, &byColumNum))  //表操作
	{
		printf("CSfmp::Encode_8\n");
		pOut[index++] = 1;
		pOut[index++] = byRowNum;
		if (0 != EncodeSequence(pIn, index, byRowNum, byColumNum, pOut))
		{
			return -3;
		}
	}
	else
	{
		//....
	}
	printf("CSfmp::Encode_9\n");
	nDataLen = index;
	return 0;
}

