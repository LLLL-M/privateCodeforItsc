/**	@file oer.h
 *	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
 *	@brief OER编解码定义
 *
 *	@author		zhaiyunfeng
 *	@date		2013/04/22
 *
 *	@note 下面的note和warning为可选项目
 *	@note 这里填写本文件的详细功能描述和注解
 *	@note 历史记录：
 *	@note V2.0.0  添加了一个导出接口
 *
 *	@warning 这里填写本文件相关的警告信息
 */


#ifndef _ASN1_H
#define _ASN1_H

#ifndef NULL
#define NULL	0
#endif


#define ASNERROR(string)

#define ASN_LONG_LEN     (0x80)

//! The maximum size of a message that can be sent or received.
#define MAX_SNMP_PACKET 4096

//SFMP信息头用于区分协议类型
#define SFMP_GET_REQUEST      0x80
#define SFMP_SET_REQUEST      0x90
#define SFMP_SET_NOREPLY      0xA0
#define SFMP_GET_RESPONSE     0xC0
#define SFMP_SET_RESPONSE     0xD0
#define SFMP_ERROR_RESPONSE   0xE0
#define SFMP_TRAP             0xF0  //(Reserved for future definition)

/**	@enum Error_Definition
 *  @brief Definition of the Error Status
 *	descrition as below
 */
enum Error_Definition
{
	TOOBIG=1,   //PDU is larger than expected.
	NOSUCHNAME, //object identifier is not supported by the agent.
	BADVALUE,   //invalid value, e.g. (out of range);This error can only occur during a set operation.
	READONLY,   //object could not be written.This error can only occur during a set operation.
	GENERR      //none of above
};

typedef unsigned char OID;
typedef unsigned long DWORD;
typedef int snmp_version;
#define SAFE_INT_CAST(expr)  ((int)(expr))
#define SAFE_UINT_CAST(expr) ((unsigned int)(expr))

// Safe until 32 bit second counter wraps to zero (time functions)
#define SAFE_LONG_CAST(expr)  ((long)(expr))
#define SAFE_ULONG_CAST(expr) ((unsigned long)(expr))


// vb list
struct variable_list 
{
    struct variable_list *next_variable;    // NULL for last variable
    OID        *name;                       // Object identifier of variable
    int        name_length;                 // number of subid's in name
    unsigned char   type;                   // ASN type of variable
    union 
	{                                 // value of variable
	    long    *integer;
	    unsigned char     *string;
	    OID    *objid;
	    unsigned char   *bitstring;
	    struct counter64 *counter64;
    } val;
    int        val_len;
};


// pdu
struct sfmp_pdu
{
	//DWORD  version;      //default:version-1(0)
    DWORD  command;      // pdu type
    DWORD  type;         // preamble
    DWORD  reqid;        // Request id
    DWORD  errstat;      // Error status
    DWORD  errindex;     // Error index
	OID    *name;        // Object identifier of variable
    DWORD  name_length;  // number of subid's in name
	DWORD  value;        // data for use
	DWORD  val_len;      // the length of value
};
#define DLLOPT
// prototypes for encoding routines
DLLOPT unsigned char *asn_parse_int(unsigned char *data, int *datalength,
                                    unsigned char *type,
                                    long *intp);

inline unsigned char *asn_parse_int(unsigned char *data, int *datalength,
                                    unsigned char *type,
                                    unsigned long *intp)
{ return asn_parse_int(data, datalength, type, (long*)intp); }


DLLOPT unsigned char *asn_parse_unsigned_int(unsigned char *data,        
                                             int *datalength,
                                             unsigned char *type,
                                             unsigned long *intp);

inline unsigned char *asn_parse_unsigned_int(unsigned char *data,        
                                             int *datalength,
                                             unsigned char *type,
                                             long *intp)
{ return asn_parse_unsigned_int(data, datalength, type, (unsigned long*)intp); }

DLLOPT unsigned char *asn_build_int(unsigned char *data, int *datalength,
                                    const unsigned char type,
                                    const long *intp);

inline unsigned char *asn_build_int(unsigned char *data, int *datalength,
                                    const unsigned char type,
                                    const unsigned long *intp)
{ return asn_build_int(data, datalength, type, (const long*)intp); }

DLLOPT unsigned char *asn_build_unsigned_int(unsigned char *data,
                                             int *datalength,
                                             unsigned char type,
                                             unsigned long *intp);

DLLOPT unsigned char *asn_parse_string(unsigned char *data, int *datalength,
                                       unsigned char *type,
                                       unsigned char *string,
                                       int *strlength);

DLLOPT unsigned char *asn_build_string(unsigned char *data, int *datalength,
                                       const unsigned char type,
                                       const unsigned char *string,
                                       const int strlength);

DLLOPT unsigned char *asn_parse_header(unsigned char *data, int *datalength,
                                       unsigned char *type);

DLLOPT unsigned char *asn_build_header(unsigned char *data, int *datalength,
                                       unsigned char type, int length);

DLLOPT unsigned char *asn_build_sequence(unsigned char *data,
                                         int *datalength,
                                         unsigned char type,
                                         int length);

DLLOPT unsigned char *asn_parse_length(unsigned char *data,
                                       unsigned long *length);

DLLOPT unsigned char *asn_build_length(unsigned char *data, int *datalength,
                                       int length);

DLLOPT unsigned char *asn_parse_objid(unsigned char *data, int *datalength,
                                      unsigned char *type,
                                      OID *objid, int *objidlength);

DLLOPT unsigned char *asn_build_objid(unsigned char *data, int *datalength,
                                      unsigned char type,
                                      OID *objid, int objidlength);

DLLOPT unsigned char *asn_parse_null(unsigned char *data, int *datalength,
                                     unsigned char *type);

DLLOPT unsigned char *asn_build_null(unsigned char *data,int *datalength,
                                     unsigned char type);

DLLOPT unsigned char *asn_parse_bitstring(unsigned char *data, int *datalength,
                                          unsigned char *type,
                                          unsigned char *string,
                                          int *strlength);

DLLOPT unsigned char *asn_build_bitstring(unsigned char *data, int *datalength,
                                          unsigned char type,
                                          unsigned char *string,
                                          int strlength);

DLLOPT unsigned char *asn_parse_unsigned_int64(unsigned char *data,
                                               int *datalength,
                                               unsigned char *type,
                                               struct counter64 *cp);

DLLOPT unsigned char *asn_build_unsigned_int64(unsigned char *data,
                                               int *datalength,
                                               unsigned char type,
                                               struct counter64 *cp);

DLLOPT struct sfmp_pdu *sfmp_pdu_create(int command);

DLLOPT void sfmp_free_pdu(struct sfmp_pdu *pdu);

DLLOPT int sfmp_build(struct sfmp_pdu *pdu,
                      unsigned char *packet,
                      int *out_length,
                      const long version,
                      const unsigned char* community, const int community_len);

/*DLLOPT void sfmp_add_var(struct sfmp_pdu *pdu,
                         OID *name, int name_length,
                         SmiVALUE *smival);*/

DLLOPT int sfmp_parse(struct sfmp_pdu *pdu,
                      unsigned char *data, int data_length,
                      unsigned char *community_name, int &community_len,
                      snmp_version &version);

DLLOPT unsigned char *build_vb(struct sfmp_pdu *pdu,
			       unsigned char *buf, int *buf_len);

DLLOPT unsigned char *build_data_pdu(struct sfmp_pdu *pdu,
				     unsigned char *buf, int *buf_len,
				     unsigned char *vb_buf, int vb_buf_len);

DLLOPT unsigned char *snmp_build_var_op(unsigned char *data,
                                        OID * var_name, int *var_name_len,
                                        unsigned char var_val_type,
                                        int var_val_len,
                                        unsigned char *var_val,
                                        int *listlength);

DLLOPT unsigned char *sfmp_parse_var_op(unsigned char *data,
                                        OID *var_name, int *var_name_len,
                                        unsigned char  *var_val_type,
                                        int *var_val_len,
                                        unsigned char  **var_val,        
                                        int *listlength);

DLLOPT int sfmp_parse_data_pdu(struct sfmp_pdu *pdu,
                               unsigned char *&data, int &length);
                               
DLLOPT int sfmp_parse_vb(struct sfmp_pdu *pdu,
                         unsigned char *&data, int &data_len);

DLLOPT void clear_pdu(struct sfmp_pdu *pdu, bool clear_all = false);


#endif

