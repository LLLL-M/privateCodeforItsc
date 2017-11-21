#ifndef _DATATYPE_H_
#define _DATATYPE_H_

#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_OID_SIZE     128
#define MAX_VALUE_SIZE   32
#define MAX_PHASE_NUM     16

typedef struct sockaddr_in  ipaddr;
typedef unsigned char oid;
typedef unsigned char BYTE;
typedef int INTEGER;

//SNMP
#define SNMP_SEQUENCE			 0x30
#define SNMP_GET_REQ_MSG         0XA0
#define SNMP_GETNEXT_REQ_MSG     0xA1
#define SNMP_GET_RSP_MSG         0xA2
#define SNMP_SET_REQ_MSG         0xA3
#define SNMP_TRP_REQ_MSG         0xA4
//SFMP
#define	SFMP_GET_REQ_MSG 		 0x80
#define SFMP_SET_REQ_MSG 		 0x90
#define SFMP_SET_REQ_NOPLY_MSG   0xA0
#define SFMP_GET_RSP_MSG         0xC0
#define SFMP_SET_RSP_MSG         0xD0
#define SFMP_ERR_RSP_MSG         0xE0
#define SFMP_TRP_REQ_MSG 		 0xF0  //(Reserved for future definition)

const unsigned char ASN_INTEGER = 2;
const unsigned char OCTET_STRING = 4;
const unsigned char NULL_TYPE = 5;
const unsigned char OBJECT_IDENTIFIER = 6;
const unsigned char IA5STRING = 22;
const unsigned char ASN_SEQUENCE = '\x30';
const unsigned char SEQUENCE_OF = '\x30';
const unsigned char IP_ADDRESS = '\x40';
const unsigned char COUNTER = '\x41';
const unsigned char GAUGE = '\x42';
const unsigned char OPAQUE_VALUE = '\x44';
const unsigned char UNKNOWN_SYNTAX = 0;
const unsigned char TIME_TICK = '\x43';

const long STRING_NOT_EMPTY_ERROR = 1;
const long UNKNOWN_OID_STYLE_ERROR = 2;
const long VALUE_TOO_BIG_ERROR = 3;
const long LENGTH_ERROR = 4;
const long INVALID_FIXED_LENGTH_ERROR = 5;
const long UNKNOWN_SYNTAX_ERROR = 6;
const long SYNTAX_ERROR = 7;
const long STYLE_ERROR = 8;
const long SYNTAX_WARNING = 9;
const long UNEXPECTED_VALUE_WARNING = 10;
const long UNKNOWN_SYNTAX_WARNING = 11;


struct counter64 
{
    unsigned long high;
    unsigned long low;
};
// vb list
struct variable_list 
{
    oid        			name[MAX_OID_SIZE];	   // Object identifier of variable
    unsigned long 		type;                     // ASN type of variable
	union
	{
		int 			nValue;
		BYTE        	byValue[MAX_VALUE_SIZE]; 
	}unValue;    
    unsigned long       value_len;
};

// pdu
typedef struct _stmf_pdu_ 
{
    unsigned long   command;      // pdu type
	unsigned long   version;      
    unsigned long   reqid;        // Request id
    unsigned long   errstat;  	  // Error status
    unsigned long   errindex; 	  // Error index
	oid        		oid_name[MAX_OID_SIZE];	//OID
    // Trap information
    oid        		enterprise[MAX_OID_SIZE];	// System OID
    unsigned long   enterprise_length;
    ipaddr     		agent_addr;       			// address of object generating trap
    unsigned long   trap_type;        			// trap type
    unsigned long   specific_type;    			// specific type
    unsigned long 	time;             			// Uptime
    // vb list
	std::vector<variable_list> vb;
}PDU;

#endif
