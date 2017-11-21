/*_############################################################################
  _## 
  _##  system_group.cpp  
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

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <sys/timeb.h>
#endif
#include <agent_pp/system_group.h>

#ifdef WIN32
#ifdef __BCPLUSPLUS__
#define _timeb timeb
#define _ftime ftime
#endif
#endif

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
using namespace Agentpp;
#endif



/**********************************************************************
 *  
 *  class sysUpTime
 * 
 **********************************************************************/

u_int sysUpTime::start = 0;

sysUpTime::sysUpTime(): MibLeaf(oidSysUpTime, READONLY, new TimeTicks(0))
{
	start = get();
}

time_t sysUpTime::get_currentTime()
{
	time_t now;
	time(&now);
	return now;
}

u_int sysUpTime::get()
{
#ifndef _WIN32
	struct timeval ct;
	gettimeofday(&ct, 0);
	return (get_currentTime()*100 + ct.tv_usec/10000) - start;
#else
	struct _timeb tstruct;
	_ftime(&tstruct);
	return (get_currentTime()*100 + tstruct.millitm/10) - start;
#endif
}

void sysUpTime::get_request(Request* req, int ind)
{
	*((TimeTicks*)value) = (unsigned long)get();
	MibLeaf::get_request(req, ind);
}


/**********************************************************************
 *  
 *  class sysDescr
 * 
 **********************************************************************/

sysDescr::sysDescr(const char* str): SnmpDisplayString(oidSysDescr, READONLY, 
						       new OctetStr(str)) { }

/**********************************************************************
 *  
 *  class sysObjectID
 * 
 **********************************************************************/

sysObjectID::sysObjectID(const Oidx& o): MibLeaf(oidSysObjectID, READONLY, 
						 new Oid(o)) { }

/**********************************************************************
 *  
 *  class sysServices
 * 
 **********************************************************************/

sysServices::sysServices(const int s): MibLeaf(oidSysServices, READONLY, 
					       new SnmpInt32(s)) { }


/**
 *  sysOREntry
 *
 */

sysOREntry* sysOREntry::instance = 0;

const index_info indSysOREntry[1] = {
	{ sNMP_SYNTAX_INT, FALSE, 1, 1 } };

sysOREntry::sysOREntry(TimeStamp* lc):
   TimeStampTable(oidSysOREntry, indSysOREntry, 1, lc)
{
	// This table object is a singleton. In order to access it use
	// the static pointer sysOREntry::instance.
	instance = this;

	add_col(new MibLeaf(colSysORID, READONLY, new Oidx()));
	add_col(new MibLeaf(colSysORDescr, READONLY, new OctetStr()));
	add_col(new MibLeaf(colSysORUpTime, READONLY, new TimeTicks()));
}

sysOREntry::~sysOREntry()
{
}

MibTableRow* sysOREntry::find(const Oidx& id) 
{
	OidListCursor<MibTableRow> cur;
	for (cur.init(&content); cur.get(); cur.next()) {
		Oidx other;
		cur.get()->get_nth(0)->get_value(other);
		if (other == id) return cur.get();
	}
	return 0;
}

void sysOREntry::set_row(MibTableRow* r, const Oidx& id, 
			 const OctetStr& descr, int lastUpdate)
{
	r->get_nth(0)->replace_value(new Oid(id));
	r->get_nth(1)->replace_value(new OctetStr(descr));
	r->get_nth(2)->replace_value(new TimeTicks(lastUpdate));
	updated();
}


/**********************************************************************
 *  
 *  class sysGroup
 * 
 **********************************************************************/

sysGroup::sysGroup(const char* descr, const Oidx& o, const int services): 
  MibGroup(oidSysGroup, "systemGroup") 
{
	add(new sysDescr(descr));
	add(new sysObjectID(o));
	add(new sysUpTime());
	add(new SnmpDisplayString(oidSysContact, READWRITE, new OctetStr("")));
	add(new SnmpDisplayString(oidSysName, READWRITE, new OctetStr("")));
	add(new SnmpDisplayString(oidSysLocation, READWRITE,new OctetStr("")));
	add(new sysServices(services));
	TimeStamp* ptr = new TimeStamp(oidSysORLastChange, READONLY, 
				       VMODE_NONE);
	add(ptr);
	add(new sysOREntry(ptr));
}

