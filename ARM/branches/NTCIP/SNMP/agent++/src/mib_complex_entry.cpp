/*_############################################################################
  _## 
  _##  mib_complex_entry.cpp  
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

#include <agent_pp/mib_complex_entry.h>

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


/*------------------------ class MibComplexEntry ------------------------*/

MibComplexEntry::MibComplexEntry(const Oidx& o, mib_access a): MibEntry(o, a)
{
	upperBound = o.next_peer();
}

MibComplexEntry::MibComplexEntry(const MibComplexEntry& other): MibEntry(other)
{
	upperBound = other.upperBound;
}

MibComplexEntry::~MibComplexEntry()
{
}

/*------------------------ class MibStaticEntry ------------------------*/



/*------------------------ class MibStaticTable ------------------------*/

MibStaticTable::MibStaticTable(const Oidx& o): MibComplexEntry(o, NOACCESS)
{
}

MibStaticTable::MibStaticTable(MibStaticTable& other): 
  MibComplexEntry(other)
{
	OidListCursor<MibStaticEntry> cur;
	for (cur.init(&other.contents); cur.get(); cur.next()) {
		contents.add(new MibStaticEntry(*cur.get()));
	}
}

MibStaticTable::~MibStaticTable()
{
}

void MibStaticTable::add(const MibStaticEntry& entry) 
{
	start_synch();
	Oidx tmpoid(entry.get_oid());
	MibStaticEntry* newEntry = new MibStaticEntry(entry);
	// TODO: added here for backward compatibility, should be removed
	// later
	if (oid.is_root_of(tmpoid)) {
		tmpoid = tmpoid.cut_left(oid.len());
		newEntry->set_oid(tmpoid);
	}
	MibStaticEntry* ptr = contents.find(&tmpoid);
	if (ptr) {
		contents.remove(&tmpoid);
	}
	contents.add(newEntry);
	end_synch();
}

void MibStaticTable::remove(const Oidx& o) 
{
	start_synch();
	Oidx tmpoid(o);
	if (oid.is_root_of(tmpoid)) {
		tmpoid = tmpoid.cut_left(oid.len());
		contents.remove(&tmpoid);
	}
	end_synch();	
}

MibStaticEntry* MibStaticTable::get(const Oidx& o, boolean suffixOnly) 
{
	Oidx tmpoid(o);
	if (!suffixOnly) {
		if (!oid.is_root_of(tmpoid))
			return 0;
		tmpoid = tmpoid.cut_left(oid.len());
	}
	return contents.find(&tmpoid);
}

Oidx MibStaticTable::find_succ(const Oidx& o, Request*)
{
	start_synch();
	Oidx tmpoid(o);
	Oidx retval;
	if (tmpoid <= oid) {
		tmpoid = Oidx();
	}
	else if (tmpoid.len() >= oid.len()) {
		tmpoid = tmpoid.cut_left(oid.len());
	}
	else {
		end_synch();
		return retval;
	}
	MibStaticEntry* ptr = contents.find_upper(&tmpoid);
	if ((ptr) && (*ptr->key() == tmpoid)) {
		ptr = contents.find_next(&tmpoid);
	}
	if (ptr) {
		retval = oid;
		retval += ptr->get_oid();
	}
	end_synch();
	return retval;
}

void MibStaticTable::get_request(Request* req, int ind)
{
	Oidx tmpoid(req->get_oid(ind));
	if (oid.is_root_of(tmpoid)) {
		tmpoid = tmpoid.cut_left(oid.len());
	}
	else {
		Vbx vb(req->get_oid(ind));
		vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
		// error status (v1) will be set by RequestList
		req->finish(ind, vb); 
		return;
	}
	MibStaticEntry* entry = contents.find(&tmpoid);
	if (!entry) {
		Vbx vb(req->get_oid(ind));
		// TODO: This error status is just a guess, we cannot
		// determine exactly whether it is a noSuchInstance or
		// noSuchObject. May be a subclass could do a better 
		// job by knowing more details from the MIB structure?
		if (tmpoid.len() == 0) {
			vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
		}
		else {
			Oidx columnID;
			columnID = tmpoid[0];
			entry = contents.find_upper(&columnID);
			if (entry)
				vb.set_syntax(sNMP_SYNTAX_NOSUCHINSTANCE);
			else
				vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
		}
		// error status (v1) will be set by RequestList
		req->finish(ind, vb); 
	}
	else {
		Oidx id(oid);
		id += *entry->key();
		Vbx vb(*entry);
		vb.set_oid(id);
		req->finish(ind, vb);
	}
}

void MibStaticTable::get_next_request(Request* req, int ind)
{
	Oidx tmpoid(req->get_oid(ind));
	if (oid.is_root_of(tmpoid)) {
		tmpoid = tmpoid.cut_left(oid.len());
	}
	else {
		Vbx vb(req->get_oid(ind));
		vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
		// error status (v1) will be set by RequestList
		req->finish(ind, vb); 
	}
	MibStaticEntry* entry = contents.find_upper(&tmpoid);
	if (!entry) {
		Vbx vb(req->get_oid(ind));
		// TODO: This error status is just a guess, we cannot
		// determine exactly whether it is a noSuchInstance or
		// noSuchObject. May be a subclass could do a better 
		// job by knowing more details from the MIB structure?
		vb.set_syntax(sNMP_SYNTAX_NOSUCHINSTANCE);
		// error status (v1) will be set by RequestList
		req->finish(ind, vb); 
	}
	else {
		Oidx id(oid);
		id += *entry->key();
		Vbx vb(*entry);
		vb.set_oid(id);
		req->finish(ind, vb);
	}
}

#ifdef AGENTPP_NAMESPACE
}
#endif

