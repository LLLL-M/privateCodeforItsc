/*_############################################################################
  _## 
  _##  proxy_forwarder.cpp  
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


#include <agent_pp/proxy_forwarder.h>
#include <agent_pp/snmp_target_mib.h>
#include <agent_pp/snmp_proxy_mib.h>
#include <snmp_pp/log.h>

#ifdef _SNMPv3
#ifdef _PROXY_FORWARDER

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
using namespace Agentpp;
#endif


ProxyForwarder::ProxyForwarder(const OctetStr& contextEngineID, pdu_type t)
{
	regKey = Oidx::from_string(contextEngineID);
	regKey += t;
	snmp = 0;
	initialize();
}

ProxyForwarder::~ProxyForwarder()
{
	if (snmp)
		delete snmp;
}

void ProxyForwarder::initialize()
{
	check_references();
	snmp = new SnmpRequestV3();
}

void ProxyForwarder::check_references()
{
	if ((!snmpProxyEntry::instance) ||
	    (!snmpTargetParamsEntry::instance) ||
	    (!snmpTargetAddrEntry::instance)) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("ProxyForwarder: internal error: need SNMP-PROXY- and SNMP-TARGET-MIB.");
		LOG_END;
	}
}

OidList<MibTableRow>*  ProxyForwarder::get_matches(Request* req)
{
	List<MibTableRow>* list =
	  snmpProxyEntry::instance->get_rows_cloned(TRUE);
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
		LOG("ProxyForwarder: matched proxy (pdu type)(type)");
		LOG(type);
		LOG(state);
		LOG_END;

		cur.get()->get_nth(1)->get_value(contextID);
		cur.get()->get_nth(2)->get_value(contextName);
		cur.get()->get_nth(3)->get_value(paramsIn);

		OctetStr cid;
		req->get_pdu()->get_context_engine_id(cid);

		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("ProxyForwarder: matching (context engine id)(match)");
		LOG(cid.get_printable());
		LOG(cid.len());
		LOG(contextID.get_printable());
		LOG(contextID.len());
		LOG_END;

		if (cid != contextID)
			continue;

		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("ProxyForwarder: matched (context name)(match)");
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

boolean ProxyForwarder::match_target_params(Request* req,
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
		LOG("ProxyForwarder: target addr parameter (row) not found.");
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
	LOG("ProxyForwarder: matching (MPModel)(match)(secModel)(match)(secLevel)(match)");
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
	LOG("ProxyForwarder: matching (secName)(match)");
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

boolean ProxyForwarder::process_multiple(Pdux& pdu, Request* req)
{
       	OidList<MibTableRow>* matches = get_matches(req);
	if (!matches) return FALSE;

	boolean OK = FALSE;
	OidListCursor<MibTableRow> cur;
	for (cur.init(matches); cur.get(); cur.next()) {
		OctetStr out;
		cur.get()->get_nth(5)->get_value(out); // multiple target out

		LOG_BEGIN(DEBUG_LOG | 2);
		LOG("ProxyForwarder: multiple targets (tag)");
		LOG(out.get_printable());
		LOG_END;

		List<MibTableRow>* targets =
		  snmpTargetAddrEntry::instance->
		                          get_rows_cloned_for_tag(out);
		ListCursor<MibTableRow> tcur;
		for (tcur.init(targets); tcur.get(); tcur.next()) {

			OctetStr targetOut;
			OctetStr params;

			targetOut = tcur.get()->get_index().as_string();
			//cur.get()->get_nth(1)->get_value(taddr);
			//cur.get()->get_nth(4)->get_value(tagList);
			tcur.get()->get_nth(5)->get_value(params);

			LOG_BEGIN(DEBUG_LOG | 2);
			LOG("ProxyForwarder: searching target address (name)(params)");
			LOG(targetOut.get_printable());
			LOG(params.get_printable());
			LOG_END;

			if (params.len() == 0)
				continue;

			int secLevel = 0;
			UTarget* target =
			  snmpTargetAddrEntry::instance->
			  get_target(targetOut,
				     snmpTargetParamsEntry::instance,
				     secLevel);
			if (!target)
				continue;
			GenAddress addr;
			target->get_address(addr);

			LOG_BEGIN(INFO_LOG | 2);
			LOG("ProxyForwarder: contacting agent (type)(address)(secName)(secLevel)(context)(contextEngineID)");
			LOG(pdu.get_type());
			LOG(addr.get_printable());
			LOG(target->get_security_name().get_printable());
			LOG(secLevel);
			LOG(pdu.get_context_name().get_printable());
			LOG(pdu.get_context_engine_id().get_printable());
			LOG_END;

			int status;
			status = snmp->send_request(*target, pdu);

			OK = TRUE;
			delete target;
			LOG_BEGIN(INFO_LOG | 3);
			LOG("ProxyForwarder: agent contacted: (status)");
			LOG(status);
			LOG_END;
		}
		delete targets;
	}
	delete matches;
	return OK;
}

boolean ProxyForwarder::process_single(Pdux& pdu, Request* req)
{
	OidList<MibTableRow>* matches = get_matches(req);
	if (!matches) return FALSE;

	MibTableRow* match = matches->first();
	if (!match) {
		LOG_BEGIN(INFO_LOG | 3);
		LOG("ProxyForwarder: no matching proxy entry");
		LOG_END;
		delete matches;
		return FALSE;
	}
	OctetStr out;
	match->get_nth(4)->get_value(out);

	int secLevel = 0;
	UTarget* target =
	  snmpTargetAddrEntry::instance->
	  get_target(out, snmpTargetParamsEntry::instance, secLevel);
	if (!target) {
		LOG_BEGIN(INFO_LOG | 3);
		LOG("ProxyForwarder: no matching single out address entry");
		LOG_END;
		delete matches;
		return FALSE;
	}
	pdu.set_security_level(secLevel);

	GenAddress addr;
	target->get_address(addr);
	LOG_BEGIN(INFO_LOG | 3);
	LOG("ProxyForwarder: get: contacting agent (address)(secName)(secLevel)(context)(contextEngineID)");
	LOG(addr.get_printable());
	LOG(target->get_security_name().get_printable());
	LOG(secLevel);
	LOG(pdu.get_context_name().get_printable());
	LOG(pdu.get_context_engine_id().get_printable());
	LOG_END;

	int status = snmp->send_request(*target, pdu, req->get_non_rep(), req->get_max_rep());
	if (status != SNMP_ERROR_SUCCESS) {
		pdu.set_error_status(SNMP_ERROR_GENERAL_VB_ERR);
		pdu.set_error_index(0);
	}

	delete target;
	Vbx vb;
	pdu.get_vb(vb, 0);

	LOG_BEGIN(INFO_LOG | 3);
	LOG("ProxyForwarder: agent contacted: (oid), (value), (status)");
	LOG(vb.get_printable_oid());
	LOG(vb.get_printable_value());
	LOG(status);
	LOG_END;

	delete matches;

	return TRUE;
}

boolean ProxyForwarder::process_request(Request* req)
{
	Pdux pdu(*req->get_pdu());
	switch (pdu.get_type()) {
	case sNMP_PDU_GET:
	case sNMP_PDU_GETNEXT:
	case sNMP_PDU_GETBULK:
	case sNMP_PDU_SET: {
		boolean success = process_single(pdu, req);
		transform_pdu(pdu, *req->get_pdu());
		return success;
	}
	}
	return process_multiple(pdu, req);
}

void ProxyForwarder::transform_pdu(const Pdux& in, Pdux& out)
{
    out.trim(out.get_vb_count()); // remove all Vbs in ou tpdu
	for (int i=0; i<in.get_vb_count(); i++) {
		Vbx vb;
		in.get_vb(vb, i);
		out += vb;
	}
	out.set_error_status(in.get_error_status());
	out.set_error_index(in.get_error_index());
}


#endif
#endif
