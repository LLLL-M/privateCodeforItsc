/*_############################################################################
  _## 
  _##  snmp_pp_ext.cpp  
  _## 
  _##
  _##  AGENT++ API Version 3.5.31
  _##  -----------------------------------------------
  _##  Copyright (C) 2000-2010 Frank Fock, Jochen Katz
  _##  
  _##  LICENSE AGREEMENT
  _##
  _##  WHEREAS,  Frank  Fock  and  Jochen  Katz  are  the  owners of valuable
  _##  intellectual  property rights relating to  the AGENT++ API and wish to
  _##  license AGENT++ subject to the  terms and conditions set forth  below;
  _##  and
  _##
  _##  WHEREAS, you ("Licensee") acknowledge  that Frank Fock and Jochen Katz
  _##  have the right  to grant licenses  to the intellectual property rights
  _##  relating to  AGENT++, and that you desire  to obtain a license  to use
  _##  AGENT++ subject to the terms and conditions set forth below;
  _##
  _##  Frank  Fock    and Jochen   Katz   grants  Licensee  a  non-exclusive,
  _##  non-transferable, royalty-free  license  to use   AGENT++ and  related
  _##  materials without  charge provided the Licensee  adheres to all of the
  _##  terms and conditions of this Agreement.
  _##
  _##  By downloading, using, or  copying  AGENT++  or any  portion  thereof,
  _##  Licensee  agrees to abide  by  the intellectual property  laws and all
  _##  other   applicable laws  of  Germany,  and  to all of   the  terms and
  _##  conditions  of this Agreement, and agrees  to take all necessary steps
  _##  to  ensure that the  terms and  conditions of  this Agreement are  not
  _##  violated  by any person  or entity under the  Licensee's control or in
  _##  the Licensee's service.
  _##
  _##  Licensee shall maintain  the  copyright and trademark  notices  on the
  _##  materials  within or otherwise  related   to AGENT++, and  not  alter,
  _##  erase, deface or overprint any such notice.
  _##
  _##  Except  as specifically   provided in  this  Agreement,   Licensee  is
  _##  expressly   prohibited  from  copying,   merging,  selling,   leasing,
  _##  assigning,  or  transferring  in  any manner,  AGENT++ or  any portion
  _##  thereof.
  _##
  _##  Licensee may copy materials   within or otherwise related   to AGENT++
  _##  that bear the author's copyright only  as required for backup purposes
  _##  or for use solely by the Licensee.
  _##
  _##  Licensee may  not distribute  in any  form  of electronic  or  printed
  _##  communication the  materials  within or  otherwise  related to AGENT++
  _##  that  bear the author's  copyright, including  but  not limited to the
  _##  source   code, documentation,  help  files, examples,  and benchmarks,
  _##  without prior written consent from the authors.  Send any requests for
  _##  limited distribution rights to fock@agentpp.com.
  _##
  _##  Licensee  hereby  grants  a  royalty-free  license  to  any  and   all 
  _##  derivatives  based  upon this software  code base,  that  may  be used
  _##  as a SNMP  agent development  environment or a  SNMP agent development 
  _##  tool.
  _##
  _##  Licensee may  modify  the sources  of AGENT++ for  the Licensee's  own
  _##  purposes.  Thus, Licensee  may  not  distribute  modified  sources  of
  _##  AGENT++ without prior written consent from the authors. 
  _##
  _##  The Licensee may distribute  binaries derived from or contained within
  _##  AGENT++ provided that:
  _##
  _##  1) The Binaries are  not integrated,  bundled,  combined, or otherwise
  _##     associated with a SNMP agent development environment or  SNMP agent
  _##     development tool; and
  _##
  _##  2) The Binaries are not a documented part of any distribution material. 
  _##
  _##
  _##  THIS  SOFTWARE  IS  PROVIDED ``AS  IS''  AND  ANY  EXPRESS OR  IMPLIED
  _##  WARRANTIES, INCLUDING, BUT NOT LIMITED  TO, THE IMPLIED WARRANTIES  OF
  _##  MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE  ARE DISCLAIMED.
  _##  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  _##  INDIRECT,   INCIDENTAL,  SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
  _##  (INCLUDING,  BUT NOT LIMITED  TO,  PROCUREMENT OF SUBSTITUTE  GOODS OR
  _##  SERVICES; LOSS OF  USE,  DATA, OR PROFITS; OR  BUSINESS  INTERRUPTION)
  _##  HOWEVER CAUSED  AND ON ANY THEORY  OF  LIABILITY, WHETHER IN CONTRACT,
  _##  STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  _##  IN  ANY WAY OUT OF  THE USE OF THIS  SOFTWARE,  EVEN IF ADVISED OF THE
  _##  POSSIBILITY OF SUCH DAMAGE. 
  _##
  _##
  _##  Stuttgart, Germany, Thu Sep  2 00:07:56 CEST 2010 
  _##  
  _##########################################################################*/

#include <agent_pp/agent++.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#else
#include "snmp_pp/IPv6Utility.h"
#endif
#include <stdio.h>
#include <string.h>

#include <errno.h>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#ifdef _THREADS
#include <agent_pp/threads.h>
#endif
#include <stdlib.h>
#ifdef HAVE_POLL_SYSCALL
#include <poll.h>
#endif

#include <agent_pp/snmp_pp_ext.h>
#include <snmp_pp/msgqueue.h>
#include <snmp_pp/smival.h>
#include <snmp_pp/snmp_pp.h>
#include <snmp_pp/snmpmsg.h>
#include <snmp_pp/mp_v3.h>
#include <agent_pp/v3_mib.h>
#include <snmp_pp/v3.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
namespace Snmp_pp {
#endif
extern int send_snmp_request(SnmpSocket, unsigned char*,
			     size_t, NS_SNMP Address&);
#ifdef SNMP_PP_NAMESPACE
}
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif

/**
  * class Pdux
  */

void Pdux::clear()
{
	error_status = 0;
	error_index = 0;
	request_id = 0;
	pdu_type = 0;
	notify_timestamp = 0;

	validity = FALSE;

	// init all instance vars to null and invalid
	for (int z=0; z < vb_count; z++) delete vbs[z];

	vb_count = 0;
#ifdef _SNMPv3
        security_level = 0;
        context_name = "";
        context_engine_id = "";
#endif
}

// append vb, but leaves Vb const
Pdux& Pdux::operator+=(const Vb &vb)
{
	*((Pdu*)this)+=vb;
	return *this;
}


/**
  * class Vbx
  */


boolean Vbx::equal(Vbx* avbs, Vbx* bvbs, int sz)
{
	for (int i=0; i<sz; i++) {
		if (avbs[i].get_syntax() != bvbs[i].get_syntax())
			return FALSE;
		else if (strcmp(avbs[i].get_printable_value(),
				bvbs[i].get_printable_value()))
			return FALSE;
	}
	return TRUE;
}

void Vbx::clear()
{
    free_vb(); // clear only value, NOT the OID!
}

int operator==(const Vbx& a, const Vbx& b)
{
	return (!strcmp(a.get_printable_value(),
			b.get_printable_value()));
}


int Vbx::to_asn1(Vbx* vbs, int sz, unsigned char*& buf, int& length)
{
	unsigned char packet[MAX_SNMP_PACKET];
	unsigned char* cp = packet;
	snmp_pdu* pdu = snmp_pdu_create(0);
	struct variable_list *vp;
	int len = MAX_SNMP_PACKET;

	for (int i=0; i<sz; i++) {
		SmiVALUE smival;
		int status = convertVbToSmival(vbs[i], &smival);
		if (status != SNMP_CLASS_SUCCESS) {
			freeSmivalDescriptor(&smival);
			snmp_free_pdu(pdu);
			return status;
		}
		snmp_add_var(pdu,
			     vbs[i].get_oid().oidval()->ptr,
			     vbs[i].get_oid().oidval()->len,
			     &smival);	
		freeSmivalDescriptor(&smival);	
	}
	// asn1 encode
	// cp = packet;
	for(vp = pdu->variables; vp; vp = vp->next_variable) {
		cp = snmp_build_var_op(cp,
				       vp->name,
				       &vp->name_length,
				       vp->type,
				       vp->val_len,
				       (unsigned char *)vp->val.string,
				       &len);
		if (cp == NULL) {
			snmp_free_pdu(pdu);
			return SNMP_CLASS_ERROR;
		}
	}
	snmp_free_pdu(pdu);
	length = cp - packet;
	// encode the total len
	buf = new unsigned char[length+4];
	len = length+4;
	cp = asn_build_long_len_sequence(buf,
					 &len,
					 (unsigned char)
					 (ASN_SEQUENCE | ASN_CONSTRUCTOR),
					 length,
					 3);
	if (cp == NULL) {
	        delete[] buf;
		return SNMP_CLASS_ERROR;
	}
	memcpy(cp, packet, length);
	length += 4;
	return SNMP_CLASS_SUCCESS;
}

unsigned char * Vbx::asn_build_long_len_sequence( unsigned char *data,
						  int *datalength,
						  unsigned char type,
						  int length,
						  int lengthOfLength)
{
	unsigned char * data_with_length;

	if (*datalength < 2 ) /* need at least two octets for a sequence */
	{
	    ASNERROR( "build_sequence" );
	    return NULL;
	}
	*data++ = type;
	(*datalength)--;
	
	data_with_length = asn_build_long_length( data, datalength, length,
						  lengthOfLength);
	if( data_with_length == NULL )
	{
	    (*datalength)++; 
	    /* correct datalength to emulate old behavior of build_sequence */
	    return NULL;
	}
	return data_with_length;
}

unsigned char* Vbx::asn_build_long_length( unsigned char *data,
					   int *datalength,
					   int length,
					   int lengthOfLength)
{
    unsigned char    *start_data = data;
    
    /* no indefinite lengths sent */
    switch (lengthOfLength) {
	case 1: {
	    if (*datalength < 1){
		ASNERROR("build_length");
		return NULL;
	    }	
	    *data++ = (unsigned char)length;
	    break;
	}
	case 2: {
	    if (*datalength < 2){
		ASNERROR("build_length");
		return NULL;
	    }	
	    *data++ = (unsigned char)(0x01 | ASN_LONG_LEN);
	    *data++ = (unsigned char)length;
	    break;
	}
	case 3: { 
	    if (*datalength < 3){
		ASNERROR("build_length");
		return NULL;
	    }	
	    *data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
	    *data++ = (unsigned char)((length >> 8) & 0xFF);
	    *data++ = (unsigned char)(length & 0xFF);
	    break;
	}
	case 4: {
	  if (*datalength < 4){
	      ASNERROR("build_length");
	      return NULL;
	  }	
	  *data++ = (unsigned char)(0x03 | ASN_LONG_LEN);
	  *data++ = (unsigned char)((length >> 16) & 0xFF);
	  *data++ = (unsigned char)((length >> 8) & 0xFF);
	  *data++ = (unsigned char)(length & 0xFF);
	  break;
	}
	default: {
	    if (*datalength < 5){
		ASNERROR("build_length");
		return NULL;
	    }	
	    *data++ = (unsigned char)(0x04 | ASN_LONG_LEN);
	    *data++ = (unsigned char)((length >> 24) & 0xFF);
	    *data++ = (unsigned char)((length >> 16) & 0xFF);
	    *data++ = (unsigned char)((length >> 8) & 0xFF);
	    *data++ = (unsigned char)(length & 0xFF);
	}
	}
	*datalength -= (data - start_data);
	return data;
}


int Vbx::from_asn1(Vbx*& vbs, int& sz, unsigned char*& data, int& length)
{
	oid objid[ASN_MAX_NAME_LEN], *op;
	unsigned char *var_val;
	// get the vb list
	unsigned char type;
	struct variable_list *vp = 0;
	int seqLength = length;
	data = asn_parse_header(data, &seqLength, &type);
	if ((data == NULL) || (seqLength > length))
	    return SNMP_CLASS_ERROR;
	if (type != (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR))
	    return SNMP_CLASS_ERROR;
	snmp_pdu* pdu = snmp_pdu_create(0);
	sz = 0;
	length -= seqLength + 4;
	while(seqLength > 0) {
		sz++;
		if (pdu->variables == NULL) {
			pdu->variables = vp =
			  (struct variable_list *)
			  malloc(sizeof(struct variable_list));
		}
		else {
			vp->next_variable =
			  (struct variable_list *)
			  malloc(sizeof(struct variable_list));
			vp = vp->next_variable;
		}
		vp->next_variable = NULL;
		vp->val.string = NULL;
		vp->name = NULL;
		vp->name_length = ASN_MAX_NAME_LEN;
		data = snmp_parse_var_op(data,
					 objid,
					 &vp->name_length,
					 &vp->type,
					 &vp->val_len,
					 &var_val,
					 (int *)&seqLength);
		if (data == NULL)
		{
		    snmp_free_pdu(pdu);
#ifdef _SNMPv3
		    return SNMP_CLASS_ASN1ERROR;
#else
		    return SNMP_CLASS_ERROR;
#endif
		}
		op = (oid *)malloc((unsigned)vp->name_length * sizeof(oid));
		// fixed
		memcpy((char *)op, (char *)objid,
		       vp->name_length * sizeof(oid));
		vp->name = op;

		int len = MAX_SNMP_PACKET;
		switch((short)vp->type){
		case ASN_INTEGER:
		  vp->val.integer = (long *)malloc(sizeof(long));
		  vp->val_len = sizeof(long);
		  asn_parse_int(var_val, &len, &vp->type, vp->val.integer);
		  break;

		case SMI_COUNTER:
		case SMI_GAUGE:
		case SMI_TIMETICKS:
		case SMI_UINTEGER:
		  vp->val.integer = (long *)malloc(sizeof(long));
		  vp->val_len = sizeof(long);
		  asn_parse_unsigned_int(var_val, &len, &vp->type, vp->val.integer);
		  break;

		case SMI_COUNTER64:
		  vp->val.counter64 = (struct counter64 *)malloc( sizeof(struct counter64) );
		  vp->val_len = sizeof(struct counter64);
		  asn_parse_unsigned_int64(var_val, &len, &vp->type,
					   vp->val.counter64);
		  break;
	
		case ASN_OCTET_STR:
		case SMI_IPADDRESS:
		case SMI_OPAQUE:
		case SMI_NSAP:
		  vp->val.string = (unsigned char *)malloc((unsigned)vp->val_len);
		  asn_parse_string(var_val, &len, &vp->type, vp->val.string, &vp->val_len);
		  break;

		case ASN_OBJECT_ID:
		  vp->val_len = ASN_MAX_NAME_LEN;
		  asn_parse_objid(var_val, &len, &vp->type, objid, &vp->val_len);
		  //vp->val_len *= sizeof(oid);
		  vp->val.objid = (oid *)malloc((unsigned)vp->val_len * sizeof(oid));
		  // fixed
		  memcpy((char *)vp->val.objid,
			 (char *)objid,
			 vp->val_len * sizeof(oid));
		  break;

		case SNMP_NOSUCHOBJECT:
		case SNMP_NOSUCHINSTANCE:
		case SNMP_ENDOFMIBVIEW:
		case ASN_NULL:
		  break;

		default:
		  ASNERROR("bad type returned ");
		  snmp_free_pdu(pdu);
#ifdef _SNMPv3
		  return SNMP_CLASS_ASN1ERROR;
#else
		  return SNMP_CLASS_ERROR;
#endif
		  break;
		}	
	}
	// build vbs
	vbs = new Vbx[sz];
	Oidx tempoid;
	int i=0;
	for(vp = pdu->variables; vp; vp = vp->next_variable, i++) {

	  // extract the oid portion
	  tempoid.set_data((unsigned long *)vp->name,
			   (unsigned int) vp->name_length);
	  vbs[i].set_oid(tempoid);
	  // extract the value portion
	  switch(vp->type){
	
	    // octet string
	  case sNMP_SYNTAX_OPAQUE:
	    {
	      OpaqueStr octets( (unsigned char *) vp->val.string,
				(unsigned long) vp->val_len);
	      vbs[i].set_value(octets);
	    }
	    break;
	  case sNMP_SYNTAX_OCTETS:
	    {
	      OctetStr octets( (unsigned char *) vp->val.string,
			       (unsigned long) vp->val_len);
	      vbs[i].set_value(octets);
	    }
	    break;

	    // object id
	  case sNMP_SYNTAX_OID:
	    {
	      Oid oid( (unsigned long*) vp->val.objid,
		       (int) vp->val_len);
	      vbs[i].set_value( oid);
	    }
	    break;

	    // timeticks
	  case sNMP_SYNTAX_TIMETICKS:
	    {
	      TimeTicks timeticks( (unsigned long) *(vp->val.integer));
	      vbs[i].set_value( timeticks);
	    }
	    break;

	    // 32 bit counter
	  case sNMP_SYNTAX_CNTR32:
	    {
	      Counter32 counter32( (unsigned long) *(vp->val.integer));
	      vbs[i].set_value( counter32);
	    }
	    break;

	    // 32 bit gauge
	  case sNMP_SYNTAX_GAUGE32:
	    {
	      Gauge32 gauge32( (unsigned long) *(vp->val.integer));
	      vbs[i].set_value( gauge32);
	    }
	    break;

	    // ip address
	  case sNMP_SYNTAX_IPADDR:
	    {
	      char buffer[20];
	      sprintf( buffer,"%d.%d.%d.%d",
		       vp->val.string[0],
		       vp->val.string[1],
		       vp->val.string[2],
		       vp->val.string[3]);
	      IpAddress ipaddress( buffer);
	      vbs[i].set_value( ipaddress);
	    }
	    break;

	    // 32 bit integer
	  case sNMP_SYNTAX_INT:
	    {
	      SnmpInt32 int32( (long) *(vp->val.integer));
	      vbs[i].set_value( int32);
	    }
	    break;

	    // v2 counter 64's
	  case sNMP_SYNTAX_CNTR64:
	    { // Frank Fock (was empty before)
	      Counter64 c64(((counter64*)vp->val.counter64)->high,
			    ((counter64*)vp->val.counter64)->low);
	      vbs[i].set_value( c64);
	      break;
	    }
	  case sNMP_SYNTAX_NULL:
	    vbs[i].set_null();
	    break;
	
	    // v2 vb exceptions
	  case sNMP_SYNTAX_NOSUCHOBJECT:
	  case sNMP_SYNTAX_NOSUCHINSTANCE:
	  case sNMP_SYNTAX_ENDOFMIBVIEW:
#ifdef SNMP_PP_V3
	    vbs[i].set_exception_status( vp->type);
#else
	    set_exception_status( &vbs[i], vp->type);
#endif
	    break;

	  default:
	    vbs[i].set_null();

	  } // end switch
	}

	snmp_free_pdu(pdu);
	return SNMP_CLASS_SUCCESS;
}


/*--------------------------- class Oidx -----------------------------*/

Oidx Oidx::cut_left(const unsigned int index) const
{
	return cut_left(*this, index);
}

Oidx Oidx::cut_right(const unsigned int index) const
{
	return cut_right(*this, index);
}

Oidx Oidx::cut_left(const Oidx& oid, const unsigned int index)
{
	Oidx retval;
	if (oid.valid()) {
		unsigned int i;
		for (i=index; i<oid.smival.value.oid.len; i++)
			retval += oid.smival.value.oid.ptr[i];
	}
	return retval;
}

Oidx Oidx::cut_right(const Oidx& oid, const unsigned int index)
{
	Oidx retval;
	if (oid.valid()) {
		unsigned int i;
		unsigned int s = index;
		if (s>oid.len())
			s = oid.len();
		for (i=0; i<oid.len()-s; i++)
			retval += oid.smival.value.oid.ptr[i];
	}
	return retval;
}

Oidx& Oidx::operator=(unsigned long l)
{
	// delete the old value
	if ( smival.value.oid.ptr ) {
		delete [] smival.value.oid.ptr;
		smival.value.oid.ptr = NULL;
	}
	smival.value.oid.len = 1;
	smival.value.oid.ptr = (SmiLPUINT32)new unsigned long[1];
	smival.value.oid.ptr[0] = l;
	return *this;
}

Oidx& Oidx::operator+=(IpAddress& ip)
{
    for (int i = 0; i < ip.get_length(); i++)
	*this += (unsigned long)ip[i];
    return *this;
}

Oidx& Oidx::operator+=(const unsigned long i)
{
	*((Oid*)this)+=i;
	return *this;
}

Oidx& Oidx::operator+=(const char* a)
{
	*((Oid*)this)+=a;
	return *this;
}

Oidx& Oidx::operator+=(const Oid& o)
{
	*((Oid*)this)+=o;
	return *this;
}

int Oidx::in_subtree_of(const Oidx& o) const
{
	if (len() <= o.len()) return FALSE;
	for (unsigned int i=0; i<o.len(); i++)
		if ((*this)[i] != o[i]) return FALSE;
	return TRUE;
}

int Oidx::is_root_of(const Oidx& o) const
{
	if (len() >= o.len()) return FALSE;
	for (unsigned int i=0; i<len(); i++)
		if ((*this)[i] != o[i]) return FALSE;
	return TRUE;
}

Oidx& Oidx::mask(const OctetStr& mask)
{
	for (unsigned int i=0; (i<len()) && (i<mask.len()*8); i++) {
		char m = 0x80 >> (i%8);
		if (!(mask[i/8] & m)) {
			(*this)[i] = 0ul;
		}
	}
	return *this;
}

int Oidx::compare(const Oidx& other, const OctetStr& mask) const
{
	Oidx maskedOid(*this);
	Oidx maskedOther(other);
	maskedOid.mask(mask);
	maskedOther.mask(mask);
	LOG_BEGIN(DEBUG_LOG | 10);
	LOG("Oidx: compare: (masked oid)(masked other)");
	LOG(maskedOid.get_printable());
	LOG(maskedOther.get_printable());
	LOG_END;
	if (maskedOid == maskedOther)
		return 0;
	else if (maskedOther.in_subtree_of(maskedOid))
		return 1;
	return -1;
}

int Oidx::compare(const Oidx& other, u_int wildcard) const
{
	Oidx maskedOid(*this);
	Oidx maskedOther(other);
	if ((this->len() > wildcard) && (other.len() > wildcard)) {
		maskedOid[wildcard] = 0;
		maskedOther[wildcard] = 0;
	}
	LOG_BEGIN(DEBUG_LOG | 10);
	LOG("Oidx: compare: (masked oid)(masked other)");
	LOG(maskedOid.get_printable());
	LOG(maskedOther.get_printable());
	LOG_END;
	if (maskedOid == maskedOther)
		return 0;
	else if (maskedOther.in_subtree_of(maskedOid))
		return 1;
	return -1;
}

unsigned long Oidx::last() const
{
	// check for len == 0
	if ((!Oid::valid()) || (smival.value.oid.len<1))
		return 0;

	return smival.value.oid.ptr[smival.value.oid.len-1];
}

OctetStr Oidx::as_string() const
{
	OctetStr str;
	for (int i=0; i<(int)len(); i++) {
		str += (unsigned char)(*this)[i];
	}
	return str;
}

Oidx Oidx::from_string(const OctetStr& str, boolean withLength)
{
	Oidx oid;
	if (withLength)
		oid += (long)str.len();
	for (unsigned int i=0; i<str.len(); i++)
		oid += (long)str[i];
	return oid;
}

Oidx Oidx::successor() const
{
	Oidx o(*this);
	if (o.len()==MAX_OID_LEN) {
		if (o[MAX_OID_LEN-1] == 0xFFFFFFFFul) {
			int i = MAX_OID_LEN-2;
			while ((i>=0) && (o[i] == 0xFFFFFFFFul)) {
				i--;
			}
			if (i>=0) {
				o.trim(MAX_OID_LEN-1-i);
				o[i]++;
			}
		}
		else
			o[MAX_OID_LEN-1]++;
	}
	else
		o += 0ul;
	return o;
}

Oidx Oidx::predecessor() const
{
	Oidx o(*this);
	if (o.len()==0) return o;
	if (o[o.len()-1] == 0) {
		o.trim();
		return o;
	}
	o[o.len()-1]--;
	for (int i=o.len(); i<MAX_OID_LEN; i++)
		o += 0xFFFFFFFFul;
	return o;
}

Oidx Oidx::next_peer() const
{
	Oidx o(*this);
	o[o.len()-1]++;
	return o;
}


/*****************************************************************
 *
 *  class OidxRange
 *
 ****************************************************************/

OidxRange::OidxRange()
{
}

OidxRange::OidxRange(const Oidx& l, const Oidx& u)
{
	lower = l;
	upper = u;
}

OidxRange::OidxRange(const OidxRange& other)
{
	lower = other.lower;
	upper = other.upper;
}

OidxRange::~OidxRange() { }

OidxRange* OidxRange::clone() const
{
	return new OidxRange(*this);
}

int OidxRange::operator==(const OidxRange& other) const
{
	return ((lower == other.lower) && (upper == other.upper));
}

int OidxRange::operator<(const OidxRange& other) const
{
	return (upper <= other.lower);
}

int OidxRange::operator>(const OidxRange& other) const
{
	return (lower >= other.upper);
}

int OidxRange::includes(const Oidx& o) const
{
	return ((lower <= o) && (o <= upper));
}

int OidxRange::includes_excl(const Oidx& o) const
{
	return ((lower <= o) && (o < upper));
}

boolean OidxRange::covers(const OidxRange& other) const
{
	return ((lower <= other.lower) && (upper >= other.upper));
}

boolean OidxRange::overlaps(const OidxRange& other) const
{
	// assumption: lower is always less than upper
	return ((lower < other.upper) && (upper > other.lower));
}

Oidx OidxRange::get_lower() const { return lower; }
Oidx OidxRange::get_upper() const { return upper; }


/*****************************************************************
 *
 *  class Snmpx
 *
 ****************************************************************/


#ifdef _SNMPv3
int Snmpx::receive(struct timeval *tvptr, Pdux& pdu, UTarget& target)
{
#if (defined __GNUC__ || defined __FreeBSD__ || defined _AIX) && ! defined __MINGW32__
  socklen_t fromlen;
#else
  int fromlen;
#endif

  UdpAddress fromaddr;
  snmp_version version;
  OctetStr community;

  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len; // len of received data

  SnmpMessage snmpmsg;

  int nfound = 0;
  bool can_receive_ipv4 = false;
#ifdef SNMP_PP_IPv6
  bool can_receive_ipv6 = false;
#endif

#ifdef HAVE_POLL_SYSCALL
  int nfds = 0;
  struct pollfd readfds[2];
  int timeout = tvptr ? (tvptr->tv_sec * 1000 + tvptr->tv_usec / 1000) : -1;

  memset(readfds, 0, 2 * sizeof(struct pollfd));
  if (iv_snmp_session != INVALID_SOCKET)
  {
      readfds[nfds].fd = iv_snmp_session;
      readfds[nfds].events = POLLIN;
      nfds++;
  }
#ifdef SNMP_PP_IPv6
  if (iv_snmp_session_ipv6 != INVALID_SOCKET)
  {
      readfds[nfds].fd = iv_snmp_session_ipv6;
      readfds[nfds].events = POLLIN;
      nfds++;
  }
#endif
#else // HAVE_POLL_SYSCALL
  fd_set readfds;
  int max_fd = -1;

  FD_ZERO (&readfds);

  if (iv_snmp_session != INVALID_SOCKET)
  {
    FD_SET(iv_snmp_session, &readfds);
    max_fd = iv_snmp_session;
  }

#ifdef SNMP_PP_IPv6
  if (iv_snmp_session_ipv6 != INVALID_SOCKET)
  {
    FD_SET(iv_snmp_session_ipv6, &readfds);
    if (max_fd == -1)
      max_fd = iv_snmp_session_ipv6;
    else
      max_fd = iv_snmp_session > iv_snmp_session_ipv6
                                   ? iv_snmp_session : iv_snmp_session_ipv6;
  }
#endif
#endif // HAVE_POLL_SYSCALL

  do
  {
#ifdef HAVE_POLL_SYSCALL
    nfound = poll(readfds, nfds, timeout);

    if (nfound == -1)
    {
      if (errno != EINTR)
	return SNMP_CLASS_TL_FAILED;
      continue;
    }
    else if (nfound <= 0)
      return SNMP_CLASS_TL_FAILED;

    if ((iv_snmp_session != INVALID_SOCKET) &&
	(readfds[0].revents & POLLIN))
	can_receive_ipv4 = true;
#ifdef SNMP_PP_IPv6
    if ((iv_snmp_session_ipv6 != INVALID_SOCKET) &&
	(readfds[nfds-1].revents & POLLIN))
	can_receive_ipv6 = true;
#endif // SNMP_PP_IPv6

#else // HAVE_POLL_SYSCALL
    nfound = select(max_fd+1, &readfds, 0, 0, tvptr);

    if (nfound == -1)
    {
      if (errno != EINTR)
	return SNMP_CLASS_TL_FAILED;
      continue;
    }
    else if (nfound <= 0)
      return SNMP_CLASS_TL_FAILED;

    if ((iv_snmp_session != INVALID_SOCKET) &&
	(FD_ISSET(iv_snmp_session, &readfds)))
	can_receive_ipv4 = true;
#ifdef SNMP_PP_IPv6
    if ((iv_snmp_session_ipv6 != INVALID_SOCKET) &&
	(FD_ISSET(iv_snmp_session_ipv6, &readfds)))
	can_receive_ipv6 = true;
#endif // SNMP_PP_IPv6
#endif // HAVE_POLL_SYSCALL

    if (can_receive_ipv4)
    {
	struct sockaddr_in from_addr;
	fromlen = sizeof(from_addr);
	do
	{
	  receive_buffer_len = (long)recvfrom(iv_snmp_session,
					      (char *) receive_buffer,
					      MAX_SNMP_PACKET, 0,
					      (struct sockaddr*)&from_addr,
					      &fromlen);
	} while (receive_buffer_len < 0 && EINTR == errno);

	if (receive_buffer_len <= 0 )		// error or no data pending
	  return SNMP_CLASS_TL_FAILED;

	if (receive_buffer_len >= MAX_SNMP_PACKET)
	  return SNMP_ERROR_TOO_BIG;

	debugprintf(1, "++ AGENT++: data received from %s port %d.",
		    IpAddress(inet_ntoa(from_addr.sin_addr)).get_printable(),
		    ntohs(from_addr.sin_port));
	debughexprintf(5, receive_buffer, receive_buffer_len);

	OctetStr engine_id;
	OctetStr security_name;
	long int security_model;

	// copy fromaddress and remote port
	char* addr = inet_ntoa (from_addr.sin_addr);
	fromaddr = addr;
	fromaddr.set_port(ntohs(from_addr.sin_port));

	// Frank: warum nicht Ergebnis pr∆fen?
	snmpmsg.load(receive_buffer, receive_buffer_len);

	target.set_address(fromaddr);

	int status = SNMP_CLASS_SUCCESS;
	if (snmpmsg.is_v3_message() == TRUE)
	{
	  status = snmpmsg.unloadv3(pdu, version, engine_id,
				    security_name, security_model,
				    fromaddr, *this);
	  if ((status != SNMP_CLASS_SUCCESS) &&
	      (status != SNMP_ERROR_TOO_BIG))
	    return status;

	  target.set_security_name(security_name);
	  target.set_engine_id(engine_id);
	}
	else
	{
	  status = snmpmsg.unload( pdu, community, version);
	  if ((status != SNMP_CLASS_SUCCESS) &&
	      (status != SNMP_ERROR_TOO_BIG))
	    return status;
	  target.set_security_name(community);
	  if (version == version1)
	    security_model = SNMP_SECURITY_MODEL_V1;
	  else
	    security_model = SNMP_SECURITY_MODEL_V2;
	  pdu.set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
	  pdu.set_context_engine_id("");
	  pdu.set_context_name("");
	}
	target.set_security_model(security_model);
	target.set_version(version);

	// v3 support
	if (version == version3)
	{
	  debugprintf(1,"Snmpx::receive:");
	  debugprintf(2," engine_id: (%s), security_name: (%s),\n"
		      " security_model: (%li) security_level (%i)",
		      engine_id.get_printable(),
		      security_name.get_printable(),
		      security_model, pdu.get_security_level());
	  debugprintf(2, " Addr = %s, port = %i\n",
		      inet_ntoa (from_addr.sin_addr),fromaddr.get_port());
	}
	return status;   // Success! return
    }
#ifdef SNMP_PP_IPv6
    if (can_receive_ipv6)
    {
	struct sockaddr_in6 from_addr;
	fromlen = sizeof((from_addr));
	do
	{
	  receive_buffer_len = (long)recvfrom(iv_snmp_session_ipv6,
					      (char *) receive_buffer,
					      MAX_SNMP_PACKET, 0,
					      (struct sockaddr*)&from_addr,
					      &fromlen);
	} while (receive_buffer_len < 0 && EINTR == errno);

	if (receive_buffer_len <= 0 )		// error or no data pending
	  return SNMP_CLASS_TL_FAILED;

	if (receive_buffer_len >= MAX_SNMP_PACKET)
	  return SNMP_ERROR_TOO_BIG;

	OctetStr engine_id;
	OctetStr security_name;
	long int security_model;
	char addr[INET6_ADDRSTRLEN+1];

	// copy fromaddress and remote port
	inet_ntop(AF_INET6, &from_addr.sin6_addr, addr, INET6_ADDRSTRLEN);
	fromaddr = addr;
	fromaddr.set_port(ntohs(from_addr.sin6_port));

	debugprintf(1, "++ AGENT++: ipv6 data received from %s",
		    fromaddr.get_printable());
	debughexprintf(5, receive_buffer, receive_buffer_len);

	// Frank: warum nicht Ergebnis pruefen?
	int status = snmpmsg.load(receive_buffer, receive_buffer_len);
	if (status != SNMP_CLASS_SUCCESS)
	    return status;

	status = SNMP_CLASS_SUCCESS;
	if (snmpmsg.is_v3_message() == TRUE)
	{
	  status = snmpmsg.unloadv3(pdu, version, engine_id,
				    security_name, security_model,
				    fromaddr, *this);
	  if ((status != SNMP_CLASS_SUCCESS) &&
	      (status != SNMP_ERROR_SUCCESS))
	    return status;

	  target.set_security_name(security_name);
	  target.set_engine_id(engine_id);
	}
	else
	{
	  status = snmpmsg.unload( pdu, community, version);
	  if ((status != SNMP_CLASS_SUCCESS) &&
	      (status != SNMP_ERROR_SUCCESS))
	    return status;
	  target.set_security_name(community);
	  if (version == version1)
	    security_model = SNMP_SECURITY_MODEL_V1;
	  else
	    security_model = SNMP_SECURITY_MODEL_V2;
	  pdu.set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
	  pdu.set_context_engine_id("");
	  pdu.set_context_name("");
	}
	target.set_security_model(security_model);
	target.set_version(version);
	target.set_address(fromaddr);

	// v3 support
	if (version == version3) {
	  debugprintf(1,"Snmpx::receive:");
	  debugprintf(2," engine_id: (%s), security_name: (%s),\n"
		      " security_model: (%li) security_level (%i)",
		      engine_id.get_printable(),
		      security_name.get_printable(),
		      security_model, pdu.get_security_level());
	}
	return status;   // Success! return
    }
#endif // SNMP_PP_IPv6
  } while(1);
}

#else // _SNMPv3 is not defined

int Snmpx::receive(struct timeval *tvptr, Pdux& pdu, UdpAddress& fromaddr,
		   snmp_version& version, OctetStr& community)
{
#ifndef MAX_SNMP_PACKET
#define MAX_SNMP_PACKET 2048
#endif
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  //should be same as MAX_SNMP_PACKET found in uxsnmp.C

  long receive_buffer_len; // len of received data

#if (defined __GNUC__ || defined __FreeBSD__ || defined _AIX) && ! defined __MINGW32__
  socklen_t fromlen;
#else
  int fromlen;
#endif

  SnmpMessage snmpmsg;

  int nfound = 0;
  bool can_receive_ipv4 = false;
#ifdef SNMP_PP_IPv6
  bool can_receive_ipv6 = false;
#endif

#ifdef HAVE_POLL_SYSCALL
  int nfds = 0;
  struct pollfd readfds[2];
  int timeout = tvptr ? (tvptr->tv_sec * 1000 + tvptr->tv_usec / 1000) : -1;

  memset(readfds, 0, 2 * sizeof(struct pollfd));
  if (iv_snmp_session != INVALID_SOCKET)
  {
      readfds[nfds].fd = iv_snmp_session;
      readfds[nfds].events = POLLIN;
      nfds++;
  }
#ifdef SNMP_PP_IPv6
  if (iv_snmp_session_ipv6 != INVALID_SOCKET)
  {
      readfds[nfds].fd = iv_snmp_session_ipv6;
      readfds[nfds].events = POLLIN;
      nfds++;
  }
#endif
#else // HAVE_POLL_SYSCALL
  fd_set readfds;
  int max_fd = -1;

  FD_ZERO (&readfds);

  if (iv_snmp_session != INVALID_SOCKET)
  {
    FD_SET(iv_snmp_session, &readfds);
    max_fd = iv_snmp_session;
  }

#ifdef SNMP_PP_IPv6
  if (iv_snmp_session_ipv6 != INVALID_SOCKET)
  {
    FD_SET(iv_snmp_session_ipv6, &readfds);
    if (max_fd == -1)
      max_fd = iv_snmp_session_ipv6;
    else
      max_fd = iv_snmp_session > iv_snmp_session_ipv6
                                   ? iv_snmp_session : iv_snmp_session_ipv6;
  }
#endif
#endif // HAVE_POLL_SYSCALL

  do
  {
#ifdef HAVE_POLL_SYSCALL
    nfound = poll(readfds, nfds, timeout);

    if (nfound == -1)
    {
      if (errno != EINTR)
	return SNMP_CLASS_TL_FAILED;
      continue;
    }
    else if (nfound <= 0)
      return SNMP_CLASS_TL_FAILED;

    if ((iv_snmp_session != INVALID_SOCKET) &&
	(readfds[0].revents & POLLIN))
	can_receive_ipv4 = true;
#ifdef SNMP_PP_IPv6
    if ((iv_snmp_session_ipv6 != INVALID_SOCKET) &&
	(readfds[nfds-1].revents & POLLIN))
	can_receive_ipv6 = true;
#endif // SNMP_PP_IPv6

#else // HAVE_POLL_SYSCALL
    nfound = select(max_fd+1, &readfds, 0, 0, tvptr);

    if (nfound == -1)
    {
      if (errno != EINTR)
	return SNMP_CLASS_TL_FAILED;
      continue;
    }
    else if (nfound <= 0)
      return SNMP_CLASS_TL_FAILED;

    if ((iv_snmp_session != INVALID_SOCKET) &&
	(FD_ISSET(iv_snmp_session, &readfds)))
	can_receive_ipv4 = true;
#ifdef SNMP_PP_IPv6
    if ((iv_snmp_session_ipv6 != INVALID_SOCKET) &&
	(FD_ISSET(iv_snmp_session_ipv6, &readfds)))
	can_receive_ipv6 = true;
#endif // SNMP_PP_IPv6
#endif // HAVE_POLL_SYSCALL

    if (can_receive_ipv4)
    {
	struct sockaddr_in from_addr;
	fromlen = sizeof(from_addr);
	do
	{
	  receive_buffer_len = (long)recvfrom(iv_snmp_session,
					      (char *) receive_buffer,
					      MAX_SNMP_PACKET, 0,
					      (struct sockaddr*)&from_addr,
					      &fromlen);
	} while (receive_buffer_len < 0 && EINTR == errno);

	if (receive_buffer_len <= 0 )		// error or no data pending
	  return SNMP_CLASS_TL_FAILED;

	if (receive_buffer_len >= MAX_SNMP_PACKET)
	  return SNMP_ERROR_TOO_BIG;

	// copy fromaddress and remote port
	char* addr = inet_ntoa (from_addr.sin_addr);
	fromaddr = addr;
	fromaddr.set_port(ntohs(from_addr.sin_port));

	debugprintf(1, "++ AGENT++: data received from %s.",
		    fromaddr.get_printable());
	debughexprintf(5, receive_buffer, receive_buffer_len);

	snmpmsg.load(receive_buffer, receive_buffer_len);

	// return the status of unload method
	return snmpmsg.unload(pdu, community, version);
    }
#ifdef SNMP_PP_IPv6
    if (can_receive_ipv6)
    {
	struct sockaddr_in6 from_addr;
	fromlen = sizeof(from_addr);
	do
	{
	  receive_buffer_len = (long) recvfrom(iv_snmp_session_ipv6,
					       (char *) receive_buffer,
					       MAX_SNMP_PACKET, 0,
					       (struct sockaddr*)&from_addr,
					       &fromlen);
	} while (receive_buffer_len < 0 && EINTR == errno);

	if (receive_buffer_len <= 0 )		// error or no data pending
	  return SNMP_CLASS_TL_FAILED;

	if (receive_buffer_len >= MAX_SNMP_PACKET)
	  return SNMP_ERROR_TOO_BIG;

	char addr[INET6_ADDRSTRLEN+1];

	// copy fromaddress and remote port
	inet_ntop(AF_INET6, &from_addr.sin6_addr, addr, INET6_ADDRSTRLEN);
	fromaddr = addr;
	fromaddr.set_port(ntohs(from_addr.sin6_port));

	debugprintf(1, "++ AGENT++: data received from %s.",
		    fromaddr.get_printable());
	debughexprintf(5, receive_buffer, receive_buffer_len);

	snmpmsg.load(receive_buffer, receive_buffer_len);

	// return the status of unload method
	return snmpmsg.unload(pdu, community, version);
    }
#endif // SNMP_PP_IPv6
  } while(1);
}

#endif


#ifdef _SNMPv3

int Snmpx::send (Pdux pdu, SnmpTarget* target)
{
#ifdef _THREADS
  static ThreadManager smutex;
#endif
  SnmpMessage snmpmsg;
  int status;

  GenAddress gen_address;
  snmp_version version;
  OctetStr community;
  OctetStr engine_id;
  OctetStr security_name;
  int security_model;
  CTarget* ctarget = NULL;
  UTarget* utarget = NULL;

  version = target->get_version();
  target->get_address(gen_address);
  UdpAddress udp_address(gen_address);

  switch (target->get_type())
  {
    case SnmpTarget::type_ctarget:
      ctarget = (CTarget*)target;
      break;
    case SnmpTarget::type_utarget:
      utarget = (UTarget*)target;
      break;
    case SnmpTarget::type_base:
      debugprintf(0, "-- SNMP++, do not use SnmpTarget,"
                  " use a  CTarget or UTarget");
      return SNMP_CLASS_INVALID_TARGET;
    default:
      // target is not known
      debugprintf(0, "-- SNMP++, type of target is unknown!");
      return SNMP_CLASS_UNSUPPORTED;
  }

  if (ctarget) // is it a CTarget?
  {
    debugprintf(0, "Snmpx::send called with CTarget");
    ctarget->get_readcommunity(community);
    if (version == version3)
    {
      debugprintf(0, "-- SNMP++, use UTarget for SNMPv3");
      return SNMP_CLASS_INVALID_TARGET;
    }
  }
  else if (utarget) // is it a UTarget?
  {
    debugprintf(0, "Snmpx::send called with UTarget");
    utarget->get_security_name(security_name);
    security_model = utarget->get_security_model();
    utarget->get_engine_id(engine_id);
    if (version != version3)
    {
      community = security_name;
      if ((security_model != SNMP_SECURITY_MODEL_V1) &&
	  (security_model != SNMP_SECURITY_MODEL_V2))
      {
	debugprintf(0, "-- SNMP++, Target contains invalid"
		    " security_model/version combination");
	return SNMP_CLASS_INVALID_TARGET;
      }
    }
  }
  else // target is neither CTarget nor UTarget:
  {
      debugprintf(0, "-- SNMP++, Resolve Fail");
      return SNMP_CLASS_INVALID_TARGET;
  }

  if (version == version3)
  { // v3 support
    debugprintf(1, "Snmpx::send:");
    debugprintf(2, " engine_id (%s), security_name (%s),\n"
                "security_model (%i) security_level (%i)",
                engine_id.get_printable(), security_name.get_printable(),
                security_model, pdu.get_security_level());
    debugprintf(2, " Addr/Port = %s\n",udp_address.get_printable());
    status = snmpmsg.loadv3( pdu, engine_id, security_name,
                             security_model, version);
  }
  else
    status = snmpmsg.load( pdu, community, version);
  if (status != SNMP_CLASS_SUCCESS)
    return status;

#ifdef _THREADS
  smutex.start_synch();
#endif

#ifdef SNMP_PP_IPv6
  if (udp_address.get_ip_version() == Address::version_ipv6)
    status = send_snmp_request(iv_snmp_session_ipv6,
			       snmpmsg.data(), (size_t)snmpmsg.len(),
			       udp_address);
  else
#endif
    status = send_snmp_request(iv_snmp_session,
			       snmpmsg.data(), (size_t)snmpmsg.len(),
			       udp_address);
#ifdef _THREADS
  smutex.end_synch();
#endif

  if (status != 0)
    return SNMP_CLASS_TL_FAILED;

  return SNMP_CLASS_SUCCESS;
}

#else  // _SNMPv3 is not defined

int Snmpx::send (Pdux pdu,
		 UdpAddress udp_address,
		 snmp_version version,
		 OctetStr community)

{
#ifdef _THREADS
  static ThreadManager smutex;
#endif
  SnmpMessage snmpmsg;
  int status;
  status = snmpmsg.load( pdu, community, version);
  if ( status != SNMP_CLASS_SUCCESS)
    return status;

#ifdef _THREADS
  smutex.start_synch();
#endif

#ifdef SNMP_PP_IPv6
  if (udp_address.get_ip_version() == Address::version_ipv6)
    status = send_snmp_request(iv_snmp_session_ipv6,
			       snmpmsg.data(), (size_t)snmpmsg.len(),
			       udp_address);
  else
#endif
    status = send_snmp_request(iv_snmp_session,
			       snmpmsg.data(), (size_t)snmpmsg.len(),
			       udp_address);
#ifdef _THREADS
  smutex.end_synch();
#endif

  if (status != 0)
    return SNMP_CLASS_TL_FAILED;

  return SNMP_CLASS_SUCCESS;
}

#endif

#ifdef AGENTPP_NAMESPACE
}
#endif

