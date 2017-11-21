/*_############################################################################
  _## 
  _##  notification_originator.cpp  
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


#include <agent_pp/snmp_target_mib.h>
#include <agent_pp/snmp_notification_mib.h>
#include <agent_pp/system_group.h>

#include <agent_pp/notification_originator.h>
#include <agent_pp/snmp_request.h>
#include <agent_pp/snmp_community_mib.h>
#include <agent_pp/notification_log_mib.h>
#include <agent_pp/vacm.h>
#include <snmp_pp/log.h>

#define NO_TRAP 0
#define TRAP    1
#define INFORM  2

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif

/*------------------ class NotificationOriginator -----------------------*/

NotificationOriginator::NotificationOriginator()
{
#ifdef _SNMPv3
	localEngineID = 0;
#endif
}

NotificationOriginator::~NotificationOriginator()
{
#ifdef _SNMPv3
	if (localEngineID)
		delete localEngineID;
#endif
}


void NotificationOriginator::generate(Vbx* vbs, int size, const Oidx& id,
				      const Oidx& enterprise,
				      const OctetStr& contextName) {
	generate(vbs, size, id, sysUpTime::get(), enterprise, contextName);
}

void NotificationOriginator::generate(Vbx* vbs, int size, const Oidx& id,
				      unsigned int timestamp,
				      const OctetStr& contextName) {
	generate(vbs, size, id, timestamp, "", contextName);
}

int NotificationOriginator::notify(const OctetStr& context,
				   const Oidx& oid,
				   Vbx* vbs, int sz,
				   unsigned int timestamp)
{
	if (timestamp == 0)
		timestamp = sysUpTime::get();
	return generate(vbs, sz, oid, timestamp, Oidx(), context);
}

int NotificationOriginator::generate(Vbx* vbs, int size, const Oidx& id,
				     unsigned int timestamp,
				     const Oidx& enterprise,
				     const OctetStr& contextName)
{
	// We have to be careful here about synchronisation because,
	// we may be called after an interrupt
	// Therefore synch everything or use synch methods like
	// MibTable::get_rows_cloned();
	List<MibTableRow>* typeList =
	  snmpNotifyEntry::instance->get_rows_cloned();
	ListCursor<MibTableRow> typeCur;

	List<MibTableRow>* list =
	  snmpTargetAddrEntry::instance->get_rows_cloned();
	ListCursor<MibTableRow> cur;


#ifdef _SNMPv3
	if (!localEngineID) {
		if (!v3MP::I) {
			LOG_BEGIN(ERROR_LOG | 0);
			LOG("NotificationOriginator: v3MP has to be initialized before notifications can be sent");
			LOG_END;
		}
		localEngineID = new OctetStr(v3MP::I->get_local_engine_id());
	}
#endif

	int totalStatus = SNMP_ERROR_SUCCESS;

	NotificationOriginatorParams nop(vbs, size, id, timestamp, enterprise, contextName);

	for (cur.init(list); cur.get(); cur.next()) {

	  int notify = NO_TRAP;

	  // look for tags that identify notifications
	  for (typeCur.init(typeList); typeCur.get(); typeCur.next()) {

		OctetStr tag;
		typeCur.get()->first()->get_value(tag);
		char* tagstr = new char[tag.len()+1];
		strncpy(tagstr, (char*)tag.data(), tag.len());
		tagstr[tag.len()] = 0;

		if (((SnmpTagList*)cur.get()->get_nth(4))->
		    contains(tagstr)) {
			// determine notification type
			typeCur.get()->get_nth(1)->get_value(notify);

			nop.target = 0;
			if (check_access(cur, nop)) {
				int status = send_notify(cur, nop, notify);
				if (status != SNMP_ERROR_SUCCESS)
					totalStatus = status;
				delete nop.target;
			}
		}
		delete[] tagstr;
	  }
	  if (notify == NO_TRAP) {
		LOG_BEGIN(DEBUG_LOG | 6);
		LOG("NotificationOriginator: generate: could not find valid tag");
		LOG_END;
		continue;
	  }
	}

	typeList->clearAll();
	delete typeList;
	list->clearAll();
	delete list;

	return totalStatus;
}


boolean NotificationOriginator::check_access(ListCursor<MibTableRow>& cur,
		NotificationOriginatorParams& nop)
{
	Vbx*& vbs = nop.vbs;
	int& size = nop.size;
	const Oidx& id = nop.id;
	OctetStr& securityName = nop.securityName;
	int& securityModel = nop.securityModel;
	int& securityLevel = nop.securityLevel;
	int& mpModel = nop.mpModel;
#ifdef _SNMPv3
	const OctetStr& contextName = nop.contextName;
	UTarget*& target = nop.target;
#else
	CTarget*& target = nop.target;
#endif

	  snmpTargetAddrParams* paramsPtr =
	    (snmpTargetAddrParams*)cur.get()->get_nth(5);
	  OctetStr paramsStr;
	  paramsPtr->get_value(paramsStr);

	  // Check whether trap oid passes filter
	  Oidx targetOid(Oidx::from_string(paramsStr, FALSE));
	  if (!snmpNotifyFilterEntry::passes_filter(targetOid, id, vbs, size)){

		LOG_BEGIN(INFO_LOG | 2);
		LOG("NotificationOriginator: generate: event did not pass notification filter (trapoid)(filter)");
		LOG(Oidx(id).get_printable());
		LOG(paramsStr.get_printable());
		LOG_END;
		return FALSE;
	  }

	  OctetStr targetAddress;
	  cur.get()->get_nth(1)->get_value(targetAddress);

	  snmpTargetParamsEntry::instance->start_synch();
	  MibTableRow* paramsRow =
	    snmpTargetParamsEntry::instance->
	    find_index(Oidx::from_string(paramsStr, FALSE));

	  if ((!paramsRow) ||
	      (paramsRow->get_row_status()->get() != rowActive)) {

	    snmpTargetParamsEntry::instance->end_synch();
	    LOG_BEGIN(WARNING_LOG | 3);
	    LOG("NotificationOriginator: generate: target addr parameter row not found.");
	    LOG(paramsStr.get_printable());
	    LOG((paramsRow) ? "no active row found" : "missing row");
	    LOG_END;
	    return FALSE;
	  }

	  paramsRow->get_nth(0)->get_value(mpModel);
	  paramsRow->get_nth(2)->get_value(securityName);
	  paramsRow->get_nth(1)->get_value(securityModel);
	  paramsRow->get_nth(3)->get_value(securityLevel);

	  snmpTargetParamsEntry::instance->end_synch();

	  boolean accessAllowed = TRUE;
#ifdef _SNMPv3
	  if ((!Mib::instance) ||
	      (!Mib::instance->get_request_list()) ||
	      (!Mib::instance->get_request_list()->get_vacm())) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("NotificationOriginator: Mib or its requestList are not initialized, aborting!");
		LOG_END;
		return FALSE;
	  }
	  Vacm* vacm = Mib::instance->get_request_list()->get_vacm();

	  for (int i=0; i<size; i++) {

	    if (vacm->isAccessAllowed(securityModel,
				      securityName,
				      securityLevel,
				      mibView_notify,
				      contextName,
				      vbs[i].get_oid()) !=
		VACM_accessAllowed) {
	      accessAllowed = FALSE;
	      break;
	    }
	  }

	  if (accessAllowed) {
	    accessAllowed =
	      (vacm->isAccessAllowed(securityModel, securityName,
				     securityLevel, mibView_notify,
				     contextName, id) ==
	       VACM_accessAllowed);
	  }
#endif
	  if (!accessAllowed) {
	    LOG_BEGIN(EVENT_LOG | 2);
	    LOG("Notification not sent (reason) (addr) (params)");
	    LOG("no access");
	    LOG(targetAddress.get_printable());
	    LOG(paramsStr.get_printable());
	    LOG_END;

	    return FALSE;
	  }

	  target = 0;
	  Address* address =
	    snmpTargetAddrEntry::instance->get_address(cur.get());
	  if (address) {
#ifdef _SNMPv3
		target = new UTarget(*address, securityName, securityModel);
#else
		target = new CTarget(*address, securityName, securityName);
#endif
		delete address;
	  }
	  else return FALSE;

	  return TRUE;
}


int NotificationOriginator::send_notify(ListCursor<MibTableRow>& cur,
		NotificationOriginatorParams& nop, int notify)
{
	Vbx*& vbs = nop.vbs;
	int& size = nop.size;
	const Oidx& id = nop.id;
	const Oidx& enterprise = nop.enterprise;
	OctetStr& securityName = nop.securityName;
	int& mpModel = nop.mpModel;
#ifdef _SNMPv3
	int& securityLevel = nop.securityLevel;
	const OctetStr& contextName = nop.contextName;
	unsigned int& timestamp = nop.timestamp;
	UTarget*& target = nop.target;
#else
	CTarget*& target = nop.target;
#endif

	long targetDomain;
	targetDomain = ((snmpTargetAddrTDomain*)cur.get()->first())->get_state();

	Oidx trapoid(id);
#ifdef _SNMPv3
	Pdux pdu;
	pdu.set_type((notify == TRAP) ? sNMP_PDU_TRAP : sNMP_PDU_INFORM);
	pdu.set_vblist(vbs, size);
	pdu.set_notify_id(id);
	pdu.set_notify_enterprise(enterprise);
	pdu.set_notify_timestamp(timestamp);
	pdu.set_security_level(securityLevel);
	pdu.set_context_name(contextName);

	  // get community info from SNMP-COMMUNITY-MIB
	  if (((mpModel == mpV1) || (mpModel == mpV2c)) &&
	      (snmpCommunityEntry::instance)) {
		if (snmpCommunityEntry::instance->get_community(securityName,
								*localEngineID,
								contextName)) {
			target->set_security_name(securityName);
		}
		else {
			LOG_BEGIN(WARNING_LOG | 2);
			LOG("NotificationOriginator: community lookup failed for (securityName) (context)");
			LOG(securityName.get_printable());
			LOG(OctetStr(contextName).get_printable());
			LOG_END;
		}
	  }
#endif
	  int status = SNMP_ERROR_SUCCESS;

	  if (mpModel == mpV1) {

#ifdef _SNMPv3
	    pdu.set_type(sNMP_PDU_V1TRAP);
#endif
	    target->set_version(version1);

#ifdef _SNMPv3
	    status = SnmpRequestV3::send(*target, pdu);
	    if (nlmLogEntry::instance) {
		nlmLogEntry::instance->
		  add_notification(*target, pdu, *localEngineID);
	    }
#else
	    status = SnmpRequest::process_trap(*target, vbs, size,
						   id, enterprise,
						   (notify != TRAP));
#endif

	    GenAddress address;
	    target->get_address(address);

	    LOG_BEGIN(EVENT_LOG | 1);
	    LOG("NotificationGenerator: sent v1 trap (id)(tdomain)(addr)(vbs)(community)");
	    LOG(trapoid.get_printable());
	    LOG(targetDomain);
	    LOG(address.get_printable());
	    LOG(size);
	    LOG(securityName.get_printable());
	    LOG_END;
	  }
	  else {
#ifdef _SNMPv3
	    if (mpModel == mpV3)
		target->set_version(version3);
	    else
#endif
		target->set_version(version2c);

	    if (notify != TRAP) {
		target->set_retry(((SnmpInt32MinMax*)
				   cur.get()->get_nth(3))->get_state());
		target->set_timeout(((SnmpInt32MinMax*)
				     cur.get()->get_nth(2))->get_state());
	    }

#ifdef _SNMPv3
	    status = SnmpRequestV3::send(*target, pdu);
	    if (nlmLogEntry::instance) {
		nlmLogEntry::instance->
		  add_notification(*target, pdu, *localEngineID);
	    }
#else
	    status = SnmpRequest::process_trap(*target, vbs, size, id, enterprise,
				      (notify != TRAP));
#endif

	    GenAddress address;
	    target->get_address(address);

	    LOG_BEGIN(EVENT_LOG | 1);
	    if (notify == TRAP)
		LOG("NotificationGenerator: sent trap (vers)(id)(tdomain)(addr)(vbs)(community/secName)(status)");
	    else
		LOG("NotificationGenerator: sent inform (vers)(id)(tdomain)(addr)(vbs)(community/secName)(status)");
	    LOG(mpModel);
	    LOG(trapoid.get_printable());
	    LOG(targetDomain);
	    LOG(address.get_printable());
	    LOG(size);
	    LOG(securityName.get_printable());
	    LOG(status);
	    LOG_END;
	  }
	  return status;
}


boolean NotificationOriginator::add_v1_trap_destination(const UdpAddress& addr,
    const OctetStr &name, const OctetStr &tag, const OctetStr &community)
{
//	OctetStr name("defaultV1Trap");
//	OctetStr tag("v1trap");
//      OctetStr community("public");
	OctetStr address;
	IpAddress ip(addr);
	for (int i=0; i<addr.get_length()-2; i++) {
		address += (unsigned char)ip[i];
	}
	address += (addr.get_port() >> 8);
	address += (addr.get_port() & 0x00FF);

	if (!snmpTargetParamsEntry::instance ||
	    !snmpTargetAddrEntry::instance)
	    return FALSE;

	if (snmpTargetParamsEntry::instance->add_entry(name, // row index
						       0,    // mpModel
						       1,    // securityModel
						       community, // secName
						       1)) { // secLevel
	    snmpNotifyEntry::instance->add_entry(name, // row index
						 tag,  // tag
						 TRAP);   // type (trap)
	}
	if (snmpTargetAddrEntry::instance->
		add_entry(UdpAddress(addr).get_printable(),  // row index
			  Oidx("1.3.6.1.6.1.1"),    // UDP domain
			  address,                  // target address
			  tag,                      // tag
			  name))                   // params entry
		return TRUE;
	return FALSE;
}


boolean NotificationOriginator::add_v2_trap_destination(const UdpAddress& addr,
    const OctetStr &name, const OctetStr &tag, const OctetStr &community)
{
//	OctetStr name("defaultV2Trap");
//	OctetStr tag("v2trap");
//      OctetStr community("public");
	OctetStr address;
	IpAddress ip(addr);
	for (int i=0; i<addr.get_length()-2; i++) {
		address += (unsigned char)ip[i];
	}
	address += (addr.get_port() >> 8);
	address += (addr.get_port() & 0x00FF);

	if (!snmpTargetParamsEntry::instance ||
	    !snmpTargetAddrEntry::instance)
	    return FALSE;

	if (snmpTargetParamsEntry::instance->add_entry(name, // row index
						       mpV2c,    // mpModel
						       SNMP_SECURITY_MODEL_V2,
						       community, // secName
						       1)) { // secLevel
	    snmpNotifyEntry::instance->add_entry(name, // row index
						 tag,  // tag
						 TRAP);   // type (trap)
	}
	if (snmpTargetAddrEntry::instance->
		add_entry(UdpAddress(addr).get_printable(),  // row index
			  Oidx("1.3.6.1.6.1.1"),    // UDP domain
			  address,                  // target address
			  tag,                      // tag
			  name))                   // params entry
		return TRUE;
	return FALSE;
}

boolean NotificationOriginator::add_v3_trap_destination(const UdpAddress& addr,
    const OctetStr &name, const OctetStr &tag, const OctetStr &secName)
{
//	OctetStr name("defaultV3Trap");
//	OctetStr tag("v3trap");
//      OctetStr secName("MD5DES");
	OctetStr address;
	IpAddress ip(addr);
	for (int i=0; i<addr.get_length()-2; i++) {
		address += (unsigned char)ip[i];
	}
	address += (addr.get_port() >> 8);
	address += (addr.get_port() & 0x00FF);

	if (!snmpTargetParamsEntry::instance ||
	    !snmpTargetAddrEntry::instance)
	    return FALSE;

	if (snmpTargetParamsEntry::instance->add_entry(name, // row index
						       mpV3,    // mpModel
						       SNMP_SECURITY_MODEL_USM,
						       secName, // secName
						       1)) { // secLevel
	    snmpNotifyEntry::instance->add_entry(name, // row index
						 tag,  // tag
						 TRAP);   // type (trap)
	}
	if (snmpTargetAddrEntry::instance->
		add_entry(UdpAddress(addr).get_printable(),  // row index
			  Oidx("1.3.6.1.6.1.1"),    // UDP domain
			  address,                  // target address
			  tag,                      // tag
			  name))                   // params entry
		return TRUE;
	return FALSE;
}

#ifdef AGENTPP_NAMESPACE
}
#endif

