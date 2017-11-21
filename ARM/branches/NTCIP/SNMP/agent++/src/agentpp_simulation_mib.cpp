/*_############################################################################
  _## 
  _##  agentpp_simulation_mib.cpp  
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


#include <agent_pp/agentpp_simulation_mib.h>
#include <agent_pp/sim_mib.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif

/**
 *  agentppSimMode
 *
 */

agentppSimMode* agentppSimMode::instance = 0;

agentppSimMode::agentppSimMode():
   MibLeaf("1.3.6.1.4.1.4976.2.1.1.0", READWRITE, new SnmpInt32(1), TRUE)
{
	instance = this;
}

agentppSimMode::~agentppSimMode()
{
}

void agentppSimMode::init(SnmpSyntax* v, boolean m) 
{
	MibLeaf::init(v, m);
	if (value) {
		switch (get_state()) {
		case 1:
			SimMibLeaf::unset_config_mode();
			break;
		case 2:
			SimMibLeaf::set_config_mode();
			break;
		}
	}				
}


long agentppSimMode::get_state()
{
	return (long)*((SnmpInt32*)value);
}

void agentppSimMode::set_state(long l)
{
	*((SnmpInt32*)value) = l;
	switch (l) {
	case 1:
		SimMibLeaf::unset_config_mode();
		break;
	case 2:
		SimMibLeaf::set_config_mode();
		break;
	}
}

int agentppSimMode::set(const Vbx& vb)
{
	long l = 0;
	if (vb.get_value(l) != SNMP_CLASS_SUCCESS)
	    return SNMP_ERROR_WRONG_TYPE;
	switch (l) {
	case 1:
		SimMibLeaf::unset_config_mode();
		break;
	case 2:
		SimMibLeaf::set_config_mode();
		break;
	}
	return MibLeaf::set(vb);
}

boolean agentppSimMode::value_ok(const Vbx& vb)
{
	long v;
	if (vb.get_value(v) != SNMP_CLASS_SUCCESS)
	    return FALSE;
	if ((v != 1) && (v != 2)) return FALSE;
	return TRUE;
}

/**
 *  agentppSimDeleteRow
 *
 */

agentppSimDeleteRow* agentppSimDeleteRow::instance = 0;

agentppSimDeleteRow::agentppSimDeleteRow(Mib* m):
   MibLeaf(oidAgentppSimDeleteRow, READWRITE, new Oid())
{
	// This leaf object is a singleton. In order to access it use
	// the static pointer agentppSimDeleteRow::instance.
	instance = this;
	mib = m;
}

agentppSimDeleteRow::~agentppSimDeleteRow()
{
}

int agentppSimDeleteRow::commit_set_request(Request* req, int ind)
{
	Oidx toid;
	Vbx vb(req->get_value(ind));
	vb.get_value(toid);
	
	MibEntryPtr entry = 0;
	int status = 
#ifdef _SNMPv3
	  mib->find_managing_object(mib->get_context(req->get_context()),
				    toid,
				    entry, req);
#else
	  mib->find_managing_object(mib->get_default_context(),
				    toid,
				    entry, req);
#endif	  
	if (status != SNMP_ERROR_SUCCESS) return SNMP_ERROR_WRONG_VALUE;
	if (entry->type() != AGENTPP_TABLE) return SNMP_ERROR_WRONG_VALUE;
	MibTable* table = (MibTable*)entry;

	table->start_synch();
	Oidx index = table->index(toid);
	table->remove_row(index);
	table->end_synch();

	return MibLeaf::commit_set_request(req, ind);
}

int agentppSimDeleteRow::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;
	Oidx toid;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(toid) != SNMP_CLASS_SUCCESS)
	    return SNMP_ERROR_WRONG_TYPE;
	MibEntryPtr entry = 0;
	status = 
#ifdef _SNMPv3
	  mib->find_managing_object(mib->get_context(req->get_context()),
				    toid,
				    entry, req);
#else
	  mib->find_managing_object(mib->get_default_context(),
				    toid,
				    entry, req);
#endif	  
	if (status != SNMP_ERROR_SUCCESS) return SNMP_ERROR_WRONG_VALUE;
	if (entry->type() != AGENTPP_TABLE) return SNMP_ERROR_WRONG_VALUE;
	MibTable* table = (MibTable*)entry;
	Oidx index = table->index(toid);
	MibTableRow* r = table->find_index(index);
	if (!r) return SNMP_ERROR_WRONG_VALUE;

	return SNMP_ERROR_SUCCESS;
}



/**
 *  agentppSimDeleteTableContents
 *
 */

agentppSimDeleteTableContents* agentppSimDeleteTableContents::instance = 0;

agentppSimDeleteTableContents::agentppSimDeleteTableContents(Mib* m):
   MibLeaf(oidAgentppSimDeleteTableContents, READWRITE, new Oid())
{
	instance = this;
	mib = m;
}

agentppSimDeleteTableContents::~agentppSimDeleteTableContents()
{
}

int agentppSimDeleteTableContents::commit_set_request(Request* req, int ind)
{
	Oidx toid;
	Vbx vb(req->get_value(ind));
	vb.get_value(toid);
	MibEntryPtr entry = 0;
	int status = 
#ifdef _SNMPv3
	  mib->find_managing_object(mib->get_context(req->get_context()),
				    toid,
				    entry, req);
#else
	  mib->find_managing_object(mib->get_default_context(),
				    toid,
				    entry, req);
#endif	  
	if (status != SNMP_ERROR_SUCCESS) return SNMP_ERROR_WRONG_VALUE;
	if (entry->type() != AGENTPP_TABLE) return SNMP_ERROR_WRONG_VALUE;
	
	MibTable* table = (MibTable*)entry;
	entry->start_synch();
	table->clear();
	entry->end_synch();
	
	return MibLeaf::commit_set_request(req, ind);
}


int agentppSimDeleteTableContents::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	Oidx toid;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(toid) != SNMP_CLASS_SUCCESS)
	    return SNMP_ERROR_WRONG_TYPE;
	MibEntryPtr entry = 0;
	status = 
#ifdef _SNMPv3
	  mib->find_managing_object(mib->get_context(req->get_context()),
				    toid,
				    entry, req);
#else
	  mib->find_managing_object(mib->get_default_context(),
				    toid,
				    entry, req);
#endif	  
	if (status != SNMP_ERROR_SUCCESS) return SNMP_ERROR_WRONG_VALUE;
	if (entry->type() != AGENTPP_TABLE) return SNMP_ERROR_WRONG_VALUE;

	return SNMP_ERROR_SUCCESS;
}


agentpp_simulation_mib::agentpp_simulation_mib(Mib* mib): 
  MibGroup("1.3.6.1.4.1.4976.2.1", "agentppSimMIB")
{
	add(new agentppSimMode());
        add(new agentppSimDeleteRow(mib));
        add(new agentppSimDeleteTableContents(mib));
}

agentpp_simulation_mib::agentpp_simulation_mib(): 
  MibGroup("1.3.6.1.4.1.4976.2.1", "agentppSimMIB")
{
	add(new agentppSimMode());
}

#ifdef AGENTPP_NAMESPACE
}
#endif











