/**	@file stmf.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief 
*
*	@author		zhaiyunfeng
*	@date		2013/04/01
*
*	@note 下面的note和warning为可选项目
*	@note SFMP,SNMP协议编解码
*	@note 历史记录：
*	@note V2.0.0  添加了一个导出接口
*
*	@warning 这里填写本文件相关的警告信息
 */

#ifndef _STMF_H_
#define _STMF_H_

#include <string>
#include "datatype.h"
using namespace std;

#define ASC_BLOCK_OID "4.2.1.B.1.0" //块头OID

typedef struct _OBJECT_INFO_
{
	BYTE byOid[62];			//表OID或者单结点OID
	BYTE byType;			//0:2字节; 1:4字节; 2:字符串
	BYTE byIndex;			//表OID时通过此索引区分列对象,表对象时列对象从1开始编号,如果是单结点则为0.
	int  rangeDown;			//取值范围下限  byType取值为 0:2字节; 1:4字节时用到上下限
	int  rangeUp;			//取值范围上限	byType取值为 0:2字节; 1:4字节时用到上下限
}OBJECT_INFO;

typedef struct _OBJECT_TABLE_OID_
{
	BYTE byOid[62];			//表OID
	BYTE byRowNum;			//表中行数
	BYTE byNum;				//行中列对象数量
}TABLE_OID;

typedef struct _ASC_BLOCK_HEADER_
{
	BYTE ascBlockDataType;
	BYTE ascBlockDataCount;
	BYTE ascBlockDataID[68];
	BYTE ascBlockIndex[5];
	BYTE ascBlockQuantity[5];
}ASC_BLOCK_HEADER;

class CStmf
{
public:
	CStmf(){}
	virtual ~CStmf(){}
	virtual int Decode(const char *pSrc, int nDataLen, PDU *pOut) = 0; //解码接口
	virtual int Encode(const PDU *pIn, char *pOut, int &nDataLen) = 0; //编码接口
};

//各函数功能说明见定义
//snmp procotol
class CSnmp :public CStmf
{
public:
	CSnmp();
	~CSnmp();
	int ReadNextType(const char *theStringPtr, int &index, unsigned char *theTypePtr);
	int GetVersion(const char *pSrc, int &index, PDU *pOut);
	int GetCommunity(const char *pSrc, int &index, PDU *pOut);
	int GetCommand(const char *pSrc, int &index, PDU *pOut);
	int GetRequestID(const char *pSrc, int &index, PDU *pOut);
	int GetErrorStatus(const char *pSrc, int &index, PDU *pOut);
	int GetErrorIndex(const char *pSrc, int &index, PDU *pOut);
	int GetPDU(const char *pSrc, int nDataLen, int &index, PDU *pOut);
	int GetValue(const char *pSrc, int &index);
	unsigned long GetLength(const char *pSrc, int &index);
	virtual int Decode(const char *pSrc, int nDataLen, PDU *pOut);
	
	virtual int Encode(const PDU *pIn, char *pOut, int &nDataLen);
	void GetSnmpLength(string *theLengthPtr, unsigned long theNum);
	int  EncodePdu(const PDU *pIn, string &pdu);
	int  EncodeErrindex(const PDU *pIn, string &err_index);
	int  EncodeErrstatus(const PDU *pIn, string &err_status);
	int  EncodeRequestID(const PDU *pIn, string &requestid);
	int  EncodeCommunity(const PDU *pIn, string &str_snmp);
	int  EncodeVersion(const PDU *pIn, string &str_snmp);
};

//sfmp procotol
class CSfmp :public CStmf
{
public:
	CSfmp();
	~CSfmp();
	bool IsTableOid(const char *pOidKeyStr, BYTE *pRowNum, BYTE *pColumNum);
	int	 IsSpecialType(BYTE byIndex, const char *pOidKeyStr);
	int Oid_To_String(const char *oid, int oid_len, char *stroid);
	int  ParseSequence(const char *pSrc, int index, BYTE byColumNum, PDU *pOut);
	int  ParseParamter(BYTE byColumIndex, const char *pSrc, int &index, PDU *pOut);
	virtual int Decode(const char *pSrc, int nDataLen, PDU *pOut);
	int  ParseBlockHeader(const char *pSrc, int &index);
	int  CheckBlock(char *oid, int oid_len);
	
	//encode
	int  EncodeHeader(unsigned long command, int &index, char *pOut);
	BYTE EncodePreamble(const PDU *pIn);
	int  StrOidTrimDot(const char *strOid, char *oid);
	int  EncodeParameter(BYTE byColumIndex, int vec_index, int &index, const PDU *pIn, char *pOut);
	int  EncodeSequence(const PDU *pIn, int &index, BYTE &byRowNum, BYTE &byColumNum, char *pOut);
	virtual int Encode(const PDU *pIn, char *pOut, int &nDataLen);
};

#endif
