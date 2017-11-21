/*_############################################################################
  _## 
  _##  snmp_notification_mib.cpp  
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

#include <agent_pp/snmp_notification_mib.h>
#include <agent_pp/snmp_target_mib.h>
#include <agent_pp/mib_complex_entry.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


/**
 *  snmpNotifyType
 *
 */


/**
 *  snmpNotifyRowStatus
 *
 */


/**
 *  snmpNotifyFilterProfileName
 *
 */


/**
 *  snmpNotifyFilterProfileRowStatus
 *
 */


/**
 *  snmpNotifyFilterMask
 *
 */



/**
 *  snmpNotifyFilterType
 *
 */


/**
 *  snmpNotifyFilterRowStatus
 *
 */



/**
 *  snmpNotifyEntry
 *
 */

snmpNotifyEntry* snmpNotifyEntry::instance = 0;

snmpNotifyEntry::snmpNotifyEntry():
   StorageTable("1.3.6.1.6.3.13.1.1.1", iSnmpAdminString, 1)
{
	// This table object is a singleton. In order to access it use
	// the static pointer snmpNotifyEntry::instance.
	instance = this;

	add_col(new SnmpTagValue("2"));
	add_col(new SnmpInt32MinMax("3", READCREATE, 1, VMODE_DEFAULT,
				    1, 2));
	add_storage_col(new StorageType("4", 3));
	add_col(new snmpRowStatus("5"));
}

snmpNotifyEntry::~snmpNotifyEntry()
{
}

MibTableRow* snmpNotifyEntry::add_entry(const OctetStr& name,
					const OctetStr& tag,
					const int type)
{
	start_synch();
	Oidx index = Oidx::from_string(name, FALSE);
	MibTableRow* r = find_index(index);
	if (r) {
		end_synch();
		return 0;
	}
	r = add_row(index);
	r->get_nth(0)->replace_value(new OctetStr(tag));
	r->get_nth(1)->replace_value(new SnmpInt32(type));
	// leave default value untouched (storage type)
	r->get_nth(3)->replace_value(new SnmpInt32(rowActive));
	end_synch();
	return r;
}


/**
 *  snmpNotifyFilterProfileEntry
 *
 */

snmpNotifyFilterProfileEntry* snmpNotifyFilterProfileEntry::instance = 0;

const index_info indSnmpNotifyFilterProfileEntry[1] = {
  { sNMP_SYNTAX_OCTETS, TRUE, 1, 32 } };

snmpNotifyFilterProfileEntry::snmpNotifyFilterProfileEntry():
   StorageTable("1.3.6.1.6.3.13.1.2.1", indSnmpNotifyFilterProfileEntry, 1)
{
	// This table object is a singleton. In order to access it use
	// the static pointer snmpNotifyFilterProfileEntry::instance.
	instance = this;

	add_col(new SnmpAdminString("1", READCREATE, new OctetStr(""),
				    VMODE_NONE, 1, 32));
	add_storage_col(new StorageType("2", 3));
	add_col(new snmpRowStatus("3"));
}

snmpNotifyFilterProfileEntry::~snmpNotifyFilterProfileEntry()
{
}



/**
 *  snmpNotifyFilterEntry
 *
 */

snmpNotifyFilterEntry* snmpNotifyFilterEntry::instance = 0;

const index_info	iSnmpNotifyFilterEntry[2] = 
{ { sNMP_SYNTAX_OCTETS, FALSE, 0, 32 }, { sNMP_SYNTAX_OID, TRUE, 1, 95 } };

snmpNotifyFilterEntry::snmpNotifyFilterEntry():
   StorageTable("1.3.6.1.6.3.13.1.3.1", iSnmpNotifyFilterEntry, 2)
{
	// This table object is a singleton. In order to access it use
	// the static pointer snmpNotifyFilterEntry::instance.
	instance = this;

	add_col(new OctetStrMinMax("2", READCREATE, new OctetStr(""),
				   VMODE_DEFAULT, 0, 16));
	add_col(new SnmpInt32MinMax("3", READCREATE, 1, VMODE_DEFAULT, 1, 2));
	add_storage_col(new StorageType("4", 3));
	add_col(new snmpRowStatus("5"));
}

snmpNotifyFilterEntry::~snmpNotifyFilterEntry()
{
}

boolean snmpNotifyFilterEntry::passes_filter(const Oidx& target, 
					     const Oidx& oid,
					     const Vbx* vbs,
					     unsigned int vb_count)
{
	Oidx index;
	index += oid.len();
	index += oid;

	snmpNotifyFilterProfileEntry::instance->start_synch();
	MibTableRow* found = 
	  snmpNotifyFilterProfileEntry::instance->find_index(target);

	// no filter -> passes filter
	if (!found) {
		snmpNotifyFilterProfileEntry::instance->end_synch();
		return TRUE;
	} 
	OctetStr profileName;
	found->first()->get_value(profileName);
	snmpNotifyFilterProfileEntry::instance->end_synch();

	Oidx profileOid;
	profileOid = Oidx::from_string(profileName);

	LOG_BEGIN(DEBUG_LOG | 4);
	LOG("NotificationOriginator: filter: using (profile) (as oid)");
	LOG(profileName.get_printable_hex());
	LOG(profileOid.get_printable());
	LOG_END;

	List<MibTableRow>* list = 
	  snmpNotifyFilterEntry::instance->get_rows_cloned(&profileOid, 
							   rowActive);
	ListCursor<MibTableRow> cur;

	// no filter -> passes filter
	if (list->size() == 0) {
		delete list;
		return TRUE;
	}

	OidList<MibStaticEntry> matches; 
	OidList<MibStaticEntry>* oidmatches = 
	  new OidList<MibStaticEntry>[vb_count]; 
	for (cur.init(list); cur.get(); cur.next()) {

		Oidx     subtree  = cur.get()->get_index();
		// no need to check: if (subtree.len()<3) continue;
		subtree = subtree.cut_left(subtree[0]+1);

		OctetStr filterMask;
		long     filterType;
		
		cur.get()->first()->get_value(filterMask);
		cur.get()->get_nth(1)->get_value(filterType);
		
		// check if oid is in the filter specified by filterMask
		// and subtree.
		if (subtree.compare(oid, filterMask) >= 0) {
			Oidx sid;
			sid += subtree.len();
			sid += subtree;
			MibStaticEntry* match = 
			  new MibStaticEntry(sid, SnmpInt32(filterType));
			matches.add(match);

			LOG_BEGIN(INFO_LOG | 4);
			LOG("NotificationOriginator: filter: (trapoid)(subtree)(filterMask)(filterType)(match)");
			LOG(Oidx(oid).get_printable());
			LOG(subtree.get_printable());
			LOG(filterMask.get_printable_hex());
			LOG(filterType);
			LOG(sid.get_printable());
			LOG_END;
		}
		else {

			LOG_BEGIN(INFO_LOG | 5);
			LOG("NotificationOriginator: filter: (trapoid)(subtree)(filterMask)(filterType)(match)");
			LOG(Oidx(oid).get_printable());
			LOG(subtree.get_printable());
			LOG(filterMask.get_printable_hex());
			LOG(filterType);
			LOG("no match");
			LOG_END;
		}
		for (unsigned int i=0; i<vb_count; i++) {
			if (subtree.compare(vbs[i].get_oid(),filterMask) >= 0){
				Oidx sid;
				sid += subtree.len();
				sid += subtree;
				MibStaticEntry* match = 
				  new MibStaticEntry(sid, 
						     SnmpInt32(filterType));
				oidmatches[i].add(match);
			}
		} 
	}
	delete list;
	if (matches.size() == 0) {
		delete[] oidmatches;
		return FALSE;
	}
	long pass = 0;
	matches.last()->get_value(pass);
	if (pass == 1) {
		for (unsigned int i=0; i<vb_count; i++) {
			if (oidmatches[i].size() > 0) {
				oidmatches[i].last()->get_value(pass);
				if (pass == 2) {
					delete[] oidmatches;
					return FALSE;
				}
			}
		} 
	}
	delete[] oidmatches;
	return (pass == 1);
}	

snmp_notification_mib::snmp_notification_mib(): MibGroup("1.3.6.1.6.3.13.1",
							 "snmpNotificationMIB")
{
	add(new snmpNotifyEntry());
	add(new snmpNotifyFilterProfileEntry());
	add(new snmpNotifyFilterEntry());
}

#ifdef AGENTPP_NAMESPACE
}
#endif
