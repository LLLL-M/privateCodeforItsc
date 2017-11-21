/*_############################################################################
  _## 
  _##  mib_proxy.cpp  
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


#include <agent_pp/mib_proxy.h>
#include <agent_pp/snmp_request.h>
#include <snmp_pp/log.h>

#ifndef _PROXY_FORWARDER

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


//----------------------- MibProxy ------------------------------------

MibProxy::MibProxy(): MibEntry(), translating(FALSE)
{
	community[READING] = "public";
	community[WRITING] = "public";
}

MibProxy::MibProxy(const MibProxy& other)
{
	oid	  = other.oid;
	access	  = other.access;

	ListCursor<MibEntry> cur;
	for (cur.init(&other.notifies); cur.get(); cur.next())
		notifies.add(cur.get());

	source		= other.source;
	translation	= other.translation;
	translating	= other.translating;
	range		= other.range;

	for (int i=0; i<WRITING; i++)
		community[i] = other.community[i];
}


MibProxy::MibProxy(const Oidx& o,
		   mib_access  a,
		   const UdpAddress& src): MibEntry(o, a),
			source(src), translating(FALSE)
{
	community[READING] = "public";
	community[WRITING] = "public";

	determineDefaultRange(o);
}

MibProxy::MibProxy(const Oidx& o,
		   mib_access  a,
		   const Oidx& trans,
		   const UdpAddress& src): MibEntry(o, a),
					source(src),
					translation(trans),
					translating(TRUE)
{
	community[READING] = "public";
	community[WRITING] = "public";

	determineDefaultRange(o);
}


void MibProxy::determineDefaultRange(const Oidx& o)
{
	range = o;
	range[range.len()-1] += 1;
}

OidxPtr MibProxy::max_key()
{
	return &range;
}

Oidx MibProxy::translate(const Oidx& o)
{
	if (translating) {
		Oidx retval(translation);
		for (unsigned int i=translation.len(); i<o.len(); i++) {
			retval += o[i];
		}
		return retval;
	}
	return o;
}

Oidx MibProxy::backward_translate(const Oidx& o)
{
	if (translating) {
		Oidx retval(oid);
		for (unsigned int i=translation.len(); i<o.len(); i++) {
			retval += o[i];
		}
		return retval;
	}
	return o;
}

void MibProxy::get_request(Request* req, int reqind)
{
	Vbx  vb;
	int status, errind;

	if (get_access() >= READONLY) {

		vb.set_oid(translate(req->get_oid(reqind)));

		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxy: get: contacting agent: oid");
		LOG(source.get_printable());
		LOG(vb.get_printable_oid());
		LOG_END;

		status = SnmpRequest::get(source, &vb, 1, errind,
					  community[READING]);

		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxy: agent contacted: source, oid, value, status");
		LOG(source.get_printable());
		LOG(vb.get_printable_oid());
		LOG(vb.get_printable_value());
		LOG(status);
		LOG_END;

		vb.set_oid(backward_translate(vb.get_oid()));
		if (status < 0) status = SNMP_ERROR_RESOURCE_UNAVAIL;
		if (status == SNMP_ERROR_SUCCESS)
		  Mib::requestList->done(req->get_transaction_id(),
					 reqind, vb);
		else
		  Mib::requestList->error(req->get_transaction_id(),
					  reqind, status);

	}
	else
		Mib::requestList->error(req->get_transaction_id(),
					reqind, SNMP_ERROR_NO_ACCESS);
}

/**
 * Find next proxied oid. This method must be called at least once
 * before calling get_next_request, because find_next cashes the
 * retreived values for get_next_request.
 *
 * @param id
 *    a oid for which its successor is searched for
 * @return
 *    the successor within this proxied oid tree or an empty oid otherwise
 */

Oidx MibProxy::find_succ(const Oidx& id, Request*)
{
	int status, errind;
        // todo: initialize status for last else **************

	if (get_access() >= READONLY) {

		Oidx tmpoid;
		// skip oids less than this proxied subtree
		if (id < oid) {
			tmpoid = oid;
		}
		else {
			tmpoid = id;
		}
		lastNext.set_oid(translate(tmpoid));

		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxy: find_next: contacting agent (proxy oid) (src) (oid)");
		LOG(oid.get_printable());
		LOG(source.get_printable());
		LOG(lastNext.get_printable_oid());
		LOG_END;

		status = SnmpRequest::next(source, &lastNext, 1, errind,
					   community[READING]);

		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxy: agent contacted: source, oid, value, status");
		LOG(source.get_printable());
		LOG(lastNext.get_printable_oid());
		LOG(lastNext.get_printable_value());
		LOG(status);
		LOG_END;

		lastNextStatus = status;
		if ((status == SNMP_ERROR_SUCCESS) &&
		    (lastNext.get_oid().in_subtree_of(((translating) ?
						       translation : oid)))) {
			return backward_translate(lastNext.get_oid());
		}
	}
	return Oidx();
}



void MibProxy::get_next_request(Request* req, int reqind)
{
	if (get_access() >= READONLY) {

		lastNext.set_oid(backward_translate(lastNext.get_oid()));
		LOG_BEGIN(DEBUG_LOG | 5);
		LOG("MibProxy: get_next_request: returning: oid, value, status");
		LOG(lastNext.get_printable_oid());
		LOG(lastNext.get_printable_value());
		LOG(lastNextStatus);
		LOG_END;
		if (lastNextStatus != SNMP_ERROR_SUCCESS)
			Mib::requestList->error(req->get_transaction_id(),
						reqind, lastNextStatus);
		else
			Mib::requestList->done(req->get_transaction_id(),
					       reqind, lastNext);
	}
	else
		Mib::requestList->error(req->get_transaction_id(),
					reqind, SNMP_ERROR_NO_ACCESS);
}

int MibProxy::prepare_set_request(Request*, int&)
{
	if (get_access() >= READWRITE) {
		return SNMP_ERROR_SUCCESS;
	}
	return SNMP_ERROR_NO_ACCESS;
}


int MibProxy::commit_set_request(Request* req, int reqind)
{
	int status, errind;
	Vbx vb(req->get_value(reqind));
	vb.set_oid(translate(req->get_oid(reqind)));

	LOG_BEGIN(INFO_LOG | 3);
	LOG("MibProxy: set: contacting agent");
	LOG(source.get_printable());
	LOG(vb.get_printable_oid());
	LOG_END;

	status = SnmpRequest::set(source, &vb, 1, errind,
				  community[WRITING]);

	LOG_BEGIN(INFO_LOG | 3);
	LOG("MibProxy: agent contacted: source, oid, value, status");
	LOG(source.get_printable());
	LOG(vb.get_printable_oid());
	LOG(vb.get_printable_value());
	LOG(status);
	LOG_END;

	vb.set_oid(backward_translate(vb.get_oid()));
	if (status != SNMP_ERROR_SUCCESS) {
	  Mib::requestList->error(req->get_transaction_id(), errind, status);
	  return SNMP_ERROR_COMITFAIL;
	}
	else
	  Mib::requestList->done(req->get_transaction_id(), errind, vb);
	return SNMP_ERROR_SUCCESS;
}

#ifdef _SNMPv3
//----------------------- MibProxyV3 ------------------------------------


MibProxyV3::MibProxyV3(const Oidx& o, mib_access a): MibEntry(o, a)
{
	myProxyInfo = snmpProxyEntry::instance;
	determineDefaultRange(o);
	check_references();
}

MibProxyV3::MibProxyV3(const MibProxyV3& other)
{
	oid	  = other.oid;
	access	  = other.access;

	ListCursor<MibEntry> cur;
	for (cur.init(&other.notifies); cur.get(); cur.next())
		notifies.add(cur.get());
	range	    = other.range;
	myProxyInfo = other.myProxyInfo;
}


MibProxyV3::MibProxyV3(snmpProxyEntry* proxyInfo,
		       const Oidx& o,
		       mib_access  a): MibEntry(o, a)
{
	determineDefaultRange(o);
	myProxyInfo = proxyInfo;
	check_references();
}

void MibProxyV3::check_references()
{
	if ((!myProxyInfo) || (!snmpTargetParamsEntry::instance) ||
	    (!snmpTargetAddrEntry::instance)) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("MibProxyV3: internal error: need SNMP-PROXY- and SNMP-TARGET-MIB.");
		LOG_END;
	}
}

void MibProxyV3::determineDefaultRange(const Oidx& o)
{
	range = o;
	range[range.len()-1] += 1;
}

OidxPtr MibProxyV3::max_key()
{
	return &range;
}

OidList<MibTableRow>*  MibProxyV3::get_matches(Request* req)
{
	List<MibTableRow>* list = myProxyInfo->get_rows_cloned(TRUE);
	OidList<MibTableRow>* matches = new OidList<MibTableRow>;
	ListCursor<MibTableRow> cur;
	for (cur.init(list); cur.get(); cur.next()) {
		int type = req->get_type();
		int state = 0;
		cur.get()->get_nth(0)->get_value(state);

		OctetStr contextID, contextName, paramsIn;

		if ((!(((type == sNMP_PDU_GET) ||
			(type == sNMP_PDU_GETNEXT) ||
			(type == sNMP_PDU_GETBULK)) && (state == 1))) &&
		    (!((type == sNMP_PDU_SET) && (state == 2))) &&
		    (!(((type == sNMP_PDU_TRAP) ||
			(type == sNMP_PDU_V1TRAP)) && (state == 3))) &&
		    (!((type == sNMP_PDU_INFORM) && (state == 4))))
			continue;

		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("MibProxyV3: matched proxy (pdu type)(type)");
		LOG(type);
		LOG(state);
		LOG_END;

		cur.get()->get_nth(1)->get_value(contextID);
		cur.get()->get_nth(2)->get_value(contextName);
		cur.get()->get_nth(3)->get_value(paramsIn);

		OctetStr cid;
		req->get_pdu()->get_context_engine_id(cid);

		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("MibProxyV3: matching (context engine id)(match)");
		LOG(cid.get_printable());
		LOG(cid.len());
		LOG(contextID.get_printable());
		LOG(contextID.len());
		LOG_END;

		if (cid != contextID)
			continue;

		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("MibProxyV3: matched (context name)(match)");
		LOG(req->get_pdu()->get_context_name().get_printable());
		LOG(contextName.get_printable());
		LOG_END;

		OctetStr cname;
		req->get_pdu()->get_context_name(cname);
		if (cname != contextName)
			continue;

		if (!match_target_params(req, paramsIn))
			continue;
		matches->add(cur.get()->clone());
	}
	delete list;
	return matches;
}

boolean MibProxyV3::match_target_params(Request* req,
					const OctetStr& paramsIn)
{
	snmpTargetParamsEntry::instance->start_synch();
	MibTableRow* paramsRow =
	  snmpTargetParamsEntry::instance->
	  find_index(Oidx::from_string(paramsIn, FALSE));

	if ((!paramsRow) ||
	    (paramsRow->get_row_status()->get() != rowActive)) {

		snmpTargetParamsEntry::instance->end_synch();

		LOG_BEGIN(WARNING_LOG | 3);
		LOG("MibProxyV3: target addr parameter (row) not found.");
		LOG(OctetStr(paramsIn).get_printable());
		LOG((paramsRow) ? "no active row found" :
		    "missing row");
		LOG_END;
		return FALSE;
	}

	int secModel, secLevel, mpModel;
	OctetStr secName;

	paramsRow->get_nth(0)->get_value(mpModel);
	paramsRow->get_nth(1)->get_value(secModel);
	paramsRow->get_nth(2)->get_value(secName);
	paramsRow->get_nth(3)->get_value(secLevel);

	snmpTargetParamsEntry::instance->end_synch();

	LOG_BEGIN(DEBUG_LOG | 6);
	LOG("MibProxyV3: matching (MPModel)(match)(secModel)(match)(secLevel)(match)");
	LOG(req->get_address()->get_version());
	LOG(mpModel);
	LOG(req->get_address()->get_security_model());
	LOG(secModel);
	LOG(req->get_pdu()->get_security_level());
	LOG(secLevel);
	LOG_END;

	if ((req->get_address()->get_version() == version1) &&
	    (mpModel != 0))
		return FALSE;
	if ((req->get_address()->get_version() == version2c) &&
	    (mpModel != 1))
		return FALSE;
	if ((req->get_address()->get_version() == version3) &&
	    (mpModel != 3))
		return FALSE;

	OctetStr sname;
	req->get_address()->get_security_name(sname);

	LOG_BEGIN(DEBUG_LOG | 6);
	LOG("MibProxyV3: matching (secName)(match)");
	LOG(sname.get_printable());
	LOG(secName.get_printable());
	LOG_END;

	if (sname != secName)
		return FALSE;
	if ((secModel != 0) &&
	    (req->get_address()->get_security_model() != secModel))
		return FALSE;
	if (req->get_pdu()->get_security_level() != secLevel)
		return FALSE;
	return TRUE;
}


void MibProxyV3::get_request(Request* req, int reqind)
{
	Vbx  vb(req->get_value(reqind));
	Pdux pdu;
	int status;

	pdu.set_type(sNMP_PDU_GET);
	pdu.set_request_id(req->get_request_id());

	if (get_access() >= READONLY) {

		pdu += vb;
		status = process_single(pdu, req);
		pdu.get_vb(vb, 0);

		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxyV3: agent contacted: (oid), (value), (status)");
		LOG(vb.get_printable_oid());
		LOG(vb.get_printable_value());
		LOG(status);
		LOG_END;

		if (status < 0) status = SNMP_ERROR_RESOURCE_UNAVAIL;
		if (status == SNMP_ERROR_SUCCESS)
		  Mib::requestList->done(req->get_transaction_id(),
					 reqind, vb);
		else
		  Mib::requestList->error(req->get_transaction_id(),
					  reqind, status);
	}
	else
		Mib::requestList->error(req->get_transaction_id(),
					reqind, SNMP_ERROR_NO_ACCESS);
}

/**
 * Find next proxied oid. This method must be called at least once
 * before calling get_next_request, because find_next cashes the
 * retreived values for get_next_request.
 *
 * @param id
 *    a oid for which its successor is searched for
 * @return
 *    the successor within this proxied oid tree or an empty oid otherwise
 */

Oidx MibProxyV3::find_succ(const Oidx& id, Request* req)
{
	if (!req) return Oidx();
	Pdux pdu;
	int status;
	pdu.set_type(sNMP_PDU_GETNEXT);
	pdu.set_request_id(req->get_request_id());
	pdu.set_context_engine_id(req->get_pdu()->get_context_engine_id());
	pdu.set_context_name(req->get_context());

	if (get_access() >= READONLY) {

		Oidx tmpoid;
		// skip oids less than this proxied subtree
		if (id < oid) {
			tmpoid = oid;
		}
		else {
			tmpoid = id;
		}
		lastNext.set_oid(tmpoid);

		pdu += lastNext;
		status = process_single(pdu, req);
		pdu.get_vb(lastNext, 0);

		lastNextStatus = status;
		if ((status == SNMP_ERROR_SUCCESS) &&
		    (lastNext.get_oid().in_subtree_of(oid))) {
			return lastNext.get_oid();
		}
	}
	return Oidx();
}

void MibProxyV3::get_next_request(Request* req, int reqind)
{
	if (get_access() >= READONLY) {

		LOG_BEGIN(DEBUG_LOG | 5);
		LOG("MibProxyV3: next: returning: oid, value, status");
		LOG(lastNext.get_printable_oid());
		LOG(lastNext.get_printable_value());
		LOG(lastNextStatus);
		LOG_END;
		if (lastNextStatus != SNMP_ERROR_SUCCESS)
			Mib::requestList->error(req->get_transaction_id(),
						reqind, lastNextStatus);
		else
			Mib::requestList->done(req->get_transaction_id(),
					  reqind, lastNext);
	}
	else
		Mib::requestList->error(req->get_transaction_id(),
					reqind, SNMP_ERROR_NO_ACCESS);
}

int MibProxyV3::prepare_set_request(Request*, int&)
{
	if (get_access() >= READWRITE) {
		return SNMP_ERROR_SUCCESS;
	}
	return SNMP_ERROR_NO_ACCESS;
}


int MibProxyV3::commit_set_request(Request* req, int reqind)
{
	Vbx vb(req->get_value(reqind));
	Pdux pdu;
	int status;

	pdu.set_type(sNMP_PDU_SET);
	pdu.set_request_id(req->get_request_id());

	pdu += vb;
	status = process_single(pdu, req);
	pdu.get_vb(vb, 0);
	if (status == sNMP_SYNTAX_NOSUCHOBJECT) {
		status = SNMP_ERROR_SUCCESS;
		vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
	}
	if (status < 0) status = SNMP_ERROR_RESOURCE_UNAVAIL;
	if (status == SNMP_ERROR_SUCCESS)
	  Mib::requestList->done(req->get_transaction_id(), reqind, vb);
	else
	  Mib::requestList->error(req->get_transaction_id(),
				  reqind, status);
	return status;
}


int MibProxyV3::process_single(Pdux& pdu, Request* req)
{
       	OidList<MibTableRow>* matches = get_matches(req);
	if (!matches) return sNMP_SYNTAX_NOSUCHOBJECT;

	MibTableRow* match = matches->first();
	if (!match) {
		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxyV3: no matching proxy entry");
		LOG_END;
		return sNMP_SYNTAX_NOSUCHOBJECT;
	}
	OctetStr out;
	match->get_nth(4)->get_value(out);

	int secLevel = 0;
	UTarget* target =
	  snmpTargetAddrEntry::instance->
	  get_target(out, snmpTargetParamsEntry::instance, secLevel);
	if (!target) {
		LOG_BEGIN(INFO_LOG | 3);
		LOG("MibProxyV3: no matching single out address entry");
		LOG_END;
		return sNMP_SYNTAX_NOSUCHOBJECT;
	}
	pdu.set_security_level(secLevel);

	GenAddress addr;
	target->get_address(addr);
	LOG_BEGIN(INFO_LOG | 3);
	LOG("MibProxyV3: get: contacting agent (address)(secName)(secLevel)(context)(contextEngineID)");
	LOG(addr.get_printable());
	LOG(target->get_security_name().get_printable());
	LOG(secLevel);
	LOG(pdu.get_context_name().get_printable());
	LOG(pdu.get_context_engine_id().get_printable());
	LOG_END;

	int status = SnmpRequestV3::send(*target, pdu);

	delete target;
	Vbx vb;
	pdu.get_vb(vb, 0);

	LOG_BEGIN(INFO_LOG | 3);
	LOG("MibProxyV3: agent contacted: (oid), (value), (status)");
	LOG(vb.get_printable_oid());
	LOG(vb.get_printable_value());
	LOG(status);
	LOG_END;
	return status;
}

#endif // _SNMPv3

#ifdef AGENTPP_NAMESPACE
}
#endif

#endif // _PROXY_FORWARDER





