/**	@file oer.cpp
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
#include <string.h>

#include "oer.h"

/********************************build*******************************/

unsigned char *asn_build_int(unsigned char *data, int *datalength,
                             const unsigned char type,
                             const long *intp)
{
	/*
	* ASN.1 integer ::= 0x02 asnlength byte {byte}*
	*/
	if (NULL == data || NULL == datalength || NULL == intp)
	{
		return NULL;
	}
	long integer = *intp;
	unsigned long mask;
	int intsize = sizeof(long);

	/* Truncate "unnecessary" bytes off of the most significant end of this
	* 2's complement integer.  There should be no sequence of 9
	* consecutive 1's or 0's at the most significant end of the
	* integer.
	*/
	mask = 0x1FFul << ((8 * (sizeof(long) - 1)) - 1);
	/* mask is 0xFF800000 on a big-endian machine */
	while ((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1) 
	{
		intsize--;
		integer <<= 8;
	}
	
	data = asn_build_header(data, datalength, type, intsize);
	if (data == NULL)   
	{
		return NULL;
	}
	
	if (*datalength < intsize)
	{
		return NULL;
	}
	*datalength -= intsize;
	mask = 0xFFul << (8 * (sizeof(long) - 1));
	/* mask is 0xFF000000 on a big-endian machine */
	while (intsize--) 
	{
		*data++ = (unsigned char)((integer & mask) >> (8 * (sizeof(long) - 1)));
		integer <<= 8;
	}
	return data;
}

unsigned char *asn_build_unsigned_int(unsigned char *data, // modified data
                                      int *datalength,     // returned buffer length
                                      unsigned char type,  // SMI type
                                      unsigned long *intp) // Uint to encode
{
	/*
	* ASN.1 integer ::= 0x02 asnlength byte {byte}*
	*/
	unsigned long u_integer = *intp;
	long u_integer_len = 0;
	long x = 0;

	// figure out the len
	if (((u_integer >> 24) & 0xFF) != 0)
	{
		u_integer_len = 4;
	}
	else if (((u_integer >> 16) & 0xFF) !=0)
	{
		u_integer_len = 3;
	}
	else if (((u_integer >> 8) & 0xFF) !=0)
	{
		u_integer_len = 2;
	}
	else
	{
		u_integer_len = 1;
	}
	
	// check for 5 byte len where first byte will be a null
	if (((u_integer >> (8 * (u_integer_len -1))) & 0x080) !=0)	
	{
		u_integer_len++;
	}

	// build up the header
	data = asn_build_header(data, datalength, type, u_integer_len);
	if (data == NULL) 
	{
		return NULL;
	}
		
	if (*datalength < u_integer_len)
	{
		return NULL;
	}
	
	// special case, add a null byte for len of 5
	if (u_integer_len == 5) 
	{
		*data++ = 0;
		for (x=1; x<u_integer_len; x++)
		{
			*data++= (unsigned char) (u_integer >> (8 * ((u_integer_len-1)-x)& 0xFF));
		} 
	}
	else
	{
		for (x=0; x<u_integer_len; x++)
		{
			*data++= (unsigned char) (u_integer >> (8 * ((u_integer_len-1)-x)& 0xFF));
		}	  
	}
	*datalength -= u_integer_len;
	return data;
}


unsigned char *asn_build_length(unsigned char *data,
                                int *datalength,
                                int length)
{
	unsigned char *start_data = data;

	/* no indefinite lengths sent */
	if (length < 0x80) 
	{
		if (*datalength < 1)
		{
			ASNERROR("build_length");
			return NULL;
		}	
		*data++ = (unsigned char)length;
	}
	else if (length <= 0xFF) 
	{
		if (*datalength < 2)
		{
			ASNERROR("build_length");
			return NULL;
		}	
		*data++ = (unsigned char)(0x01 | ASN_LONG_LEN);
		*data++ = (unsigned char)length;
	}
	else if (length <= 0xFFFF) 
	{ 	/* 0xFF < length <= 0xFFFF */
		if (*datalength < 3) 
		{
			ASNERROR("build_length");
			return NULL;
		}	
		*data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
		*data++ = (unsigned char)((length >> 8) & 0xFF);
		*data++ = (unsigned char)(length & 0xFF);
	}
	else if (length <= 0xFFFFFF)
	{ 	/* 0xFF < length <= 0xFFFF */
		if (*datalength < 4) 
		{
			ASNERROR("build_length");
			return NULL;
		}	
		*data++ = (unsigned char)(0x03 | ASN_LONG_LEN);
		*data++ = (unsigned char)((length >> 16) & 0xFF);
		*data++ = (unsigned char)((length >> 8) & 0xFF);
		*data++ = (unsigned char)(length & 0xFF);
	}
	else
	{
		if (*datalength < 5) 
		{
			ASNERROR("build_length");
			return NULL;
		}	
		*data++ = (unsigned char)(0x04 | ASN_LONG_LEN);
		*data++ = (unsigned char)((length >> 24) & 0xFF);
		*data++ = (unsigned char)((length >> 16) & 0xFF);
		*data++ = (unsigned char)((length >> 8) & 0xFF);
		*data++ = (unsigned char)(length & 0xFF);
	}
	*datalength -= SAFE_INT_CAST(data - start_data);
	return data;
}

unsigned char *asn_build_header(unsigned char *data,
                                int *datalength,
                                unsigned char type,
                                int length)
{
	if (*datalength < 1)
	{
		return NULL;
	}
	
	*data++ = type;
	(*datalength)--;
	
	return asn_build_length(data, datalength, length);
}

unsigned char *asn_build_string(unsigned char *data,
                                int *datalength,
                                const unsigned char type,
                                const unsigned char *string,
                                const int strlength)
{
	/*
	* ASN.1 octet string ::= primstring | cmpdstring
	* primstring ::= 0x04 asnlength byte {byte}*
	* cmpdstring ::= 0x24 asnlength string {string}*
	* This code will never send a compound string.
	*/
	data = asn_build_header(data, datalength, type, strlength);
	if (data == NULL)      
	{
		return NULL;
	}
	
	if (*datalength < strlength) 
	{
		return NULL;
	}
	
	memcpy(data, string, strlength);
	*datalength -= strlength;
	return data + strlength;
}


/***********************parse****************************/

unsigned char *asn_parse_int(unsigned char *data,
			     int *datalength,
			     unsigned char *type,
			     long *intp)
{
	/*
	* ASN.1 integer ::= 0x02 asnlength byte {byte}*
	*       timestamp   0x43 asnlength byte {byte}*
	*/
	unsigned char *bufp = data;
	unsigned long asn_length = 0;
	long value = 0;

	*type = *bufp++;
	if ((*type != 0x02) 
		&&(*type != 0x43) 
		&&(*type != 0x41))
	{
		ASNERROR("Wrong Type. Not an integer");
		return NULL;
	}
	
	bufp = asn_parse_length(bufp, &asn_length);
	if (bufp == NULL) 
	{
		ASNERROR("bad length");
		return NULL;
	}
	
	if ((asn_length + (bufp - data)) > (unsigned long)(*datalength)) 
	{
		ASNERROR("overflow of message (int)");
		return NULL;
	}
	
	if (asn_length > sizeof(*intp))
	{
		ASNERROR("I don't support such large integers");
		return NULL;
	}
	
	*datalength -= (int)asn_length + SAFE_INT_CAST(bufp - data);
	if (*bufp & 0x80)
	{
		value = -1; /* integer is negative */
	}
	
	while (asn_length--)
	{
		value = (value << 8) | *bufp++;
	}	
	*intp = value;
	
	return bufp;
}

unsigned char *asn_parse_unsigned_int(unsigned char *data,	
                                      int *datalength,
                                      unsigned char *type,
                                      unsigned long *intp)
{
	/*
	* ASN.1 integer ::= 0x02 asnlength byte {byte}*
	*                   0x43 asnlength byte {byte}*
	*/
	unsigned char *bufp = data;
	unsigned long	asn_length;
	unsigned long value = 0;

	// get the type
	*type = *bufp++;
	if ((*type != 0x02) && (*type != 0x43) &&
	  (*type != 0x41) && (*type != 0x42) &&
	  (*type != 0x47))
	{
		ASNERROR("Wrong Type. Not an unsigned integer");
		return NULL;
	}

	// pick up the len
	bufp = asn_parse_length(bufp, &asn_length);
	if (bufp == NULL) 
	{
		ASNERROR("bad length");
		return NULL;
	}

	// check the len for message overflow
	if ((asn_length + (bufp - data)) > (unsigned long)(*datalength))
	{
		ASNERROR("overflow of message (uint)");
		return NULL;
	}

	// check for legal uint size
	if ((asn_length > 5) || ((asn_length > 4) && (*bufp != 0x00)))
	{
		ASNERROR("I don't support such large integers");
		return NULL;
	}

	// check for leading  0 octet
	if (*bufp == 0x00) 
	{
		bufp++;
		asn_length--;
	}

	// fix the returned data length value
	*datalength -= (int)asn_length + SAFE_INT_CAST(bufp - data);

	// calculate the value
	for (long i=0; i<(long)asn_length; i++)
	{
		value = (value << 8) + (unsigned long)*bufp++;
	}
	
	*intp = value;  // assign return value

	return bufp;   // return the bumped pointer
}

unsigned char * asn_parse_length(unsigned char *data,
                                 unsigned long *length)
{
	unsigned char lengthbyte = *data;
	*length = 0;
	if (lengthbyte & ASN_LONG_LEN) 
	{
		lengthbyte &= ~ASN_LONG_LEN;	/* turn MSb off */
		if (lengthbyte == 0)
		{
			ASNERROR("We don't support indefinite lengths");
			return NULL;
		}
		if (lengthbyte > sizeof(int))
		{
			ASNERROR("we can't support data lengths that long");
			return NULL;
		}
		for (int i=0 ; i < lengthbyte ; i++)
		{
			*length = (*length << 8) + *(data + 1 + i);
		}
		// check for length greater than 2^31
		if (*length > 0x80000000ul) 
		{
			ASNERROR("SNMP does not support data lengths > 2^31");
			return NULL;
		}
		return data + lengthbyte + 1;
	}
	else 
	{ 	/* short asnlength */
		*length = (long)lengthbyte;
		return data + 1;
	}
}

unsigned char *asn_parse_header(unsigned char *data,
								int *datalength,
                                unsigned char *type)
{
	unsigned char *bufp = data;
	register int header_len;
	unsigned long asn_length;

	/* this only works on data types < 30, i.e. no extension octets */
	#if 0
	if (IS_EXTENSION_ID(*bufp)) 
	{
		ASNERROR("can't process ID >= 30");
		return NULL;
	}
	#endif
	*type = *bufp;
	bufp = asn_parse_length(bufp + 1, &asn_length);
	if (bufp == NULL)
	{
		return NULL;
	}
	
	header_len = SAFE_INT_CAST(bufp - data);
	if ((unsigned long)(header_len + asn_length) > (unsigned long)*datalength)
	{
		ASNERROR("asn length too long");
		return NULL;
	}
	*datalength = (int)asn_length;
	return bufp;
}

unsigned char *asn_parse_string(unsigned char	*data,
                                int *datalength,
                                unsigned char *type,
                                unsigned char *str,
                                int *strlength)
{
	/*
	* ASN.1 octet string ::= primstring | cmpdstring
	* primstring ::= 0x04 asnlength byte {byte}*
	* cmpdstring ::= 0x24 asnlength string {string}*
	* ipaddress  ::= 0x40 4 byte byte byte byte
	*/
	unsigned char *bufp = data;
	unsigned long	 asn_length;

	*type = *bufp++;
	if ((*type != 0x04) && (*type != 0x24) &&
		(*type != 0x40) && (*type != 0x44) &&
		(*type != 0x45)) 
	{
		ASNERROR("asn parse string: Wrong Type. Not a string");
		return NULL;
	}
	bufp = asn_parse_length(bufp, &asn_length);
	if (bufp == NULL)
	{
		return NULL;
	}
	
	if ((asn_length + (bufp - data)) > (unsigned long)(*datalength)) 
	{
		ASNERROR("asn parse string: overflow of message");
		return NULL;
	}
	if ((int)asn_length > *strlength) 
	{
		ASNERROR("asn parse string: String to parse is longer than buffer, aborting parsing.");
		return NULL;
	}

	memcpy(str, bufp, asn_length);
	*strlength = (int)asn_length;
	*datalength -= (int)asn_length + SAFE_INT_CAST(bufp - data);
	return bufp + asn_length;
}


