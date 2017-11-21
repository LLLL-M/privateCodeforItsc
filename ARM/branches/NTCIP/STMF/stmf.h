/**	@file stmf.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief 
*
*	@author		zhaiyunfeng
*	@date		2013/04/01
*
*	@note �����note��warningΪ��ѡ��Ŀ
*	@note SFMP,SNMPЭ������
*	@note ��ʷ��¼��
*	@note V2.0.0  �����һ�������ӿ�
*
*	@warning ������д���ļ���صľ�����Ϣ
 */

#ifndef _STMF_H_
#define _STMF_H_

#include <string>
#include "datatype.h"
using namespace std;

#define ASC_BLOCK_OID "4.2.1.B.1.0" //��ͷOID

typedef struct _OBJECT_INFO_
{
	BYTE byOid[62];			//��OID���ߵ����OID
	BYTE byType;			//0:2�ֽ�; 1:4�ֽ�; 2:�ַ���
	BYTE byIndex;			//��OIDʱͨ�������������ж���,�����ʱ�ж����1��ʼ���,����ǵ������Ϊ0.
	int  rangeDown;			//ȡֵ��Χ����  byTypeȡֵΪ 0:2�ֽ�; 1:4�ֽ�ʱ�õ�������
	int  rangeUp;			//ȡֵ��Χ����	byTypeȡֵΪ 0:2�ֽ�; 1:4�ֽ�ʱ�õ�������
}OBJECT_INFO;

typedef struct _OBJECT_TABLE_OID_
{
	BYTE byOid[62];			//��OID
	BYTE byRowNum;			//��������
	BYTE byNum;				//�����ж�������
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
	virtual int Decode(const char *pSrc, int nDataLen, PDU *pOut) = 0; //����ӿ�
	virtual int Encode(const PDU *pIn, char *pOut, int &nDataLen) = 0; //����ӿ�
};

//����������˵��������
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
