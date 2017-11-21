/*_############################################################################
  _## 
  _##  snmp_request.cpp  
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


#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#include <agent_pp/agent++.h>

#ifdef _THREADS
#ifndef _WIN32THREADS
#include <pthread.h>
#endif
#endif

#include <agent_pp/snmp_request.h>
#include <agent_pp/snmp_counters.h>
#include <agent_pp/system_group.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


//------------------------------ SnmpRequest -------------------------------

#ifdef _THREADS
#ifdef _WIN32THREADS
static void inform_caller(void* ptr)
#else
extern "C" {
static void *inform_caller(void* ptr)
#endif
{
	InformInfo* callInfo = (InformInfo*)ptr;
	LOG_BEGIN(EVENT_LOG | 2);
	LOG("SnmpRequest: inform thread started");
	LOG_END;
	int status;
	status = SnmpRequest::process_trap(callInfo->target, callInfo->vbs,
					   callInfo->sz, callInfo->oid, "", TRUE);
	delete callInfo;
	LOG_BEGIN(EVENT_LOG | 2);
	LOG("SnmpRequest: inform thread finished (status)");
	LOG(status);
	LOG_END;
#ifndef _WIN32THREADS
	return 0;
}
#endif
}
#endif

InformInfo::InformInfo(CTarget& t, Vbx* v, int s, const Oidx& o)
{
	target = t;
	vbs = new Vbx[s];
	// deep copy here because v may be delete while inform thread
	// is running
	for (int i=0; i<s; i++)
		vbs[i] = v[i];
	sz = s;
	oid = o;
}

InformInfo::~InformInfo()
{
	delete[] vbs;
}


int SnmpRequest::process(int rtype, const UdpAddress& address,
			 Vbx* vbs, int& sz, Vbx* out, int& errindex,
			 const OctetStr& community,
			 const int non_repeaters,
			 const int max_reps)
{
	if (!address.valid()) {
		return SNMP_CLASS_INVALID_ADDRESS;
	}
	CTarget target(address);

	int retries = DEFAULT_RETRIES;
	int timeout = DEFAULT_TIMEOUT;

	snmp_version version;
	if (rtype == sNMP_PDU_GETBULK) {
	  version = version2c;
	  timeout *= 2;
	}
	else
	  version = version1;

	int status;

	Snmpx* snmp = get_new_snmp(status);
	if (status != SNMP_CLASS_SUCCESS) {
		if (snmp) delete snmp;
		return status;
	}
	Pdux pdu;
	// the request id is set by Snmpx

	for (int i=0; i<sz; i++)
		pdu += vbs[i];

	target.set_version(version);    // set the SNMP version SNMPV1 or V2
        target.set_retry(retries);      // set the number of auto retries
        target.set_timeout(timeout);    // set timeout
        target.set_readcommunity(community);   // set read community

	switch (rtype) {
	case sNMP_PDU_GET:
		status = snmp->get(pdu,target);
		break;
	case sNMP_PDU_GETBULK:
		status = snmp->get_bulk(pdu,target,non_repeaters,max_reps);
		sz = pdu.get_vb_count();
		break;
	case sNMP_PDU_GETNEXT:
		status = snmp->get_next(pdu,target);
		break;
	case sNMP_PDU_SET:
		target.set_writecommunity(community);
		status = snmp->set(pdu,target);
		break;
	}

	MibIIsnmpCounters::incOutPkts();
	if (status != SNMP_CLASS_TIMEOUT)
		MibIIsnmpCounters::incInPkts();

	switch (pdu.get_error_status()) {
	case SNMP_ERROR_NO_SUCH_NAME: {
		MibIIsnmpCounters::incInNoSuchNames();
		break;
	}
	case SNMP_ERROR_BAD_VALUE: {
		MibIIsnmpCounters::incInBadValues();
		break;
	}
	case SNMP_ERROR_TOO_BIG: {
		MibIIsnmpCounters::incInTooBigs();
		break;
	}
	default: {
		if (pdu.get_error_status() != SNMP_ERROR_SUCCESS)
			MibIIsnmpCounters::incInGenErrs();
		break;
	}
	}
	delete snmp;
	for (int j=0; j < sz; j++) {
		pdu.get_vb(out[j], j);
	}

	errindex = pdu.get_error_index();

	return status;
}

int SnmpRequest::process_trap(SnmpTarget& target,
			      Vbx* vbs, int sz,
			      const Oidx& oid,
			      const Oidx& enterprise, boolean ack)
{
	int status;

	Snmpx* snmp = get_new_snmp(status);
	// check construction status

	if (status != SNMP_CLASS_SUCCESS) {
		if (snmp) delete snmp;
		return status;
	}
	Pdux pdu;
	// the request id is set by Snmpx

	for (int i=0; i<sz; i++)
		pdu += vbs[i];

	pdu.set_notify_timestamp(sysUpTime::get());
	pdu.set_notify_id(oid);
	pdu.set_notify_enterprise(enterprise);

	if (ack)
		status = snmp->inform(pdu, target);
	else
		status = snmp->trap(pdu, target);

	if (status == SNMP_CLASS_SUCCESS) {
		MibIIsnmpCounters::incOutPkts();
		MibIIsnmpCounters::incOutTraps();
	}
	delete snmp;

	return status;
}




int SnmpRequest::get(const UdpAddress& address, Vbx* vbs, int sz, int& errind)
{
	MibIIsnmpCounters::incOutGetRequests();
	return process(sNMP_PDU_GET, address, vbs, sz, vbs, errind, "public");
}

int SnmpRequest::next(const UdpAddress& address, Vbx* vbs, int sz, int& errind)
{
	MibIIsnmpCounters::incOutGetNexts();
	return process(sNMP_PDU_GETNEXT, address, vbs, sz, vbs,
		       errind, "public");
}

int SnmpRequest::getbulk(const UdpAddress& address, Vbx* vbs, int& sz,
			 Vbx* out, int& errind,
			 const int non_repeater, const int max_reps)
{
	MibIIsnmpCounters::incOutGetNexts();
	return process(sNMP_PDU_GETBULK, address, vbs, sz, out,
		       errind, "public",
		       non_repeater, max_reps);
}

int SnmpRequest::set(const UdpAddress& address, Vbx* vbs, int sz, int& errind)
{
	MibIIsnmpCounters::incOutSetRequests();
	return process(sNMP_PDU_SET, address, vbs, sz, vbs, errind, "public");
}

int SnmpRequest::get(const UdpAddress& address, Vbx* vbs, int sz, int& errind,
		     const OctetStr& community)
{
	MibIIsnmpCounters::incOutGetRequests();
	return process(sNMP_PDU_GET, address, vbs, sz, vbs, errind, community);
}

int SnmpRequest::next(const UdpAddress& address, Vbx* vbs, int sz, int& errind,
		     const OctetStr& community)
{
	MibIIsnmpCounters::incOutGetNexts();
	return process(sNMP_PDU_GETNEXT, address, vbs, sz, vbs,
		       errind, community);
}

int SnmpRequest::getbulk(const UdpAddress& address, Vbx* vbs, int& sz,
			 Vbx* out, int& errind, const OctetStr& community,
			 const int non_repeater, const int max_reps)
{
	MibIIsnmpCounters::incOutGetNexts();
	return process(sNMP_PDU_GETBULK, address, vbs, sz, out, errind,
		       community,
		       non_repeater, max_reps);
}

int SnmpRequest::set(const UdpAddress& address, Vbx* vbs, int sz, int& errind,
		     const OctetStr& community)
{
	MibIIsnmpCounters::incOutSetRequests();
	return process(sNMP_PDU_SET, address, vbs, sz, vbs, errind, community);
}

int SnmpRequest::trap(SnmpTarget& target, Vbx* vbs, int sz,
		      const Oidx& oid, const Oidx& enterprise)
{
	return process_trap(target, vbs, sz, oid, enterprise);
}

void SnmpRequest::inform(CTarget& target, Vbx* vbs, int sz, const Oidx& oid)
{
	MibIIsnmpCounters::incOutTraps();

#ifdef _THREADS
	InformInfo* callInfo = new InformInfo(target, vbs, sz, oid);

#ifdef _WIN32THREADS
	_beginthread(inform_caller, 0, callInfo);
#else
    static pthread_attr_t* attr = 0;

	pthread_t thread;
	if (!attr) {
	  attr = new pthread_attr_t;
	  pthread_attr_init(attr);
	  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
	}
	pthread_create(&thread, attr,
		       &inform_caller,
		       (void*) callInfo);
#endif
#else
	process_trap(target, vbs, sz, oid, "", TRUE);
#endif
}

int SnmpRequest::gettable(const UdpAddress& address, Vbx* vbs, int sz,
			  Vbx* buf, int& bufsz, const OctetStr& community,
			  const int max_reps)
{
	Vbx* out = new Vbx[max_reps*sz];
	Vbx* in  = new Vbx[sz];

	for (int j=0; j<sz; j++)
	  in[j] = vbs[j];

	int bufptr = 0;
	int osz = 0;
	int errind;
	int err = 0;

	while (err == SNMP_ERROR_SUCCESS) {
	  MibIIsnmpCounters::incOutGetNexts();
	  osz = sz;
	  err = process(sNMP_PDU_GETBULK, address, in, osz, out, errind,
			community, 0, max_reps);

	  if (err == SNMP_ERROR_SUCCESS) {
	    int i=0;
	    for (i=0; i<osz; i++, bufptr++) {

	      if (!out[i].get_oid().in_subtree_of(vbs[i%sz].get_oid())) break;

	      buf[bufptr] = out[i];

	      if (out[i].get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW)
		break;
	    }
	    if (i<osz) break;
	    for (int k=0; k<sz; k++) {
	      in[k] = buf[bufptr-sz+k];
	    }
	  }
	}
	bufsz = bufptr;

	delete [] in;
	delete [] out;

	return err;
}

Snmpx *SnmpRequest::get_new_snmp(int &status)
{
    Snmpx *snmpx;

    status = SNMP_CLASS_ERROR;

#ifdef USE_LISTEN_ADDRESS_FOR_SENDING_TRAPS
    if (Mib::instance &&
	Mib::instance->get_request_list() &&
	Mib::instance->get_request_list()->get_snmp())
    {
	UdpAddress addr = Mib::instance->get_request_list()->get_snmp()->
                                         get_listen_address();
	addr.set_port(0);
	snmpx = new Snmpx(status, addr);
    }
    else
    {
	LOG_BEGIN(WARNING_LOG | 1);
	LOG("SnmpRequestV3: Binding to all interfaces");
	LOG_END;
	snmpx = new Snmpx(status, 0);
    }
#else
    snmpx = new Snmpx(status, 0);
#endif
    return snmpx;
}

#ifdef _SNMPv3

//------------------------------ SnmpRequestV3 -------------------------------

SnmpRequestV3::SnmpRequestV3()
{
	int status;
	snmp = SnmpRequest::get_new_snmp(status);
	if (status != 0) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("SnmpRequestV3: cannot init SNMP port (status)");
		LOG(status);
		LOG_END;
	}
}

SnmpRequestV3::~SnmpRequestV3()
{
	if (snmp) delete snmp;
}

int SnmpRequestV3::send(UTarget& target, Pdux& pdu)
{
	int status;

	Snmpx* snmp = SnmpRequest::get_new_snmp(status);
	if (status != SNMP_CLASS_SUCCESS) {
		if (snmp) delete snmp;
		return status;
	}

	switch (pdu.get_type()) {
	case sNMP_PDU_GET: {
		MibIIsnmpCounters::incOutGetRequests();
		status = snmp->get(pdu, target);
		break;
	}
    case sNMP_PDU_GETNEXT:
    case sNMP_PDU_GETBULK: {
        MibIIsnmpCounters::incOutGetNexts();
        status = snmp->get_next(pdu, target);
        break;
    }
	case sNMP_PDU_SET: {
		MibIIsnmpCounters::incOutSetRequests();
		status = snmp->set(pdu, target);
		break;
	}
	case sNMP_PDU_V1TRAP:
	case sNMP_PDU_TRAP: {
		MibIIsnmpCounters::incOutTraps();
		status = snmp->trap(pdu, target);
		break;
	}
	case sNMP_PDU_INFORM: {
		MibIIsnmpCounters::incOutTraps();
		status = snmp->inform(pdu, target);
		break;
	}
	}
	if (status == SNMP_CLASS_SUCCESS) MibIIsnmpCounters::incOutPkts();
	delete snmp;

	return status;
}

int SnmpRequestV3::send_request(UTarget& target, Pdux& pdu,
                                const int non_repeaters, const int repetitions)
{
	int status = SNMP_CLASS_INVALID_PDU;
	switch (pdu.get_type()) {
	case sNMP_PDU_GET: {
		MibIIsnmpCounters::incOutGetRequests();
		status = snmp->get(pdu, target);
		break;
	}
	case sNMP_PDU_GETNEXT: {
		MibIIsnmpCounters::incOutGetNexts();
		status = snmp->get_next(pdu, target);
		break;
    }
    case sNMP_PDU_GETBULK: {
        MibIIsnmpCounters::incOutGetNexts();
        if (target.get_version() == version1)
            status = snmp->get_next(pdu, target);
        else
            status = snmp->get_bulk(pdu, target, non_repeaters, repetitions);
        break;
	}
	case sNMP_PDU_SET: {
		MibIIsnmpCounters::incOutSetRequests();
		status = snmp->set(pdu, target);
		break;
	}
	case sNMP_PDU_V1TRAP:
	case sNMP_PDU_TRAP: {
		MibIIsnmpCounters::incOutTraps();
		status = snmp->trap(pdu, target);
		break;
	}
	case sNMP_PDU_INFORM: {
		MibIIsnmpCounters::incOutTraps();
		status = snmp->inform(pdu, target);
		break;
	}
	}
	if (status == SNMP_CLASS_SUCCESS) MibIIsnmpCounters::incOutPkts();

	return status;
}

#endif

#ifdef AGENTPP_NAMESPACE
}
#endif



