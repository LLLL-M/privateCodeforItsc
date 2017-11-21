/*_############################################################################
  _## 
  _##  agentpp_test_mib.cpp  
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
 

//--AgentGen BEGIN=_BEGIN
#ifdef ___never_defined___
//--AgentGen END

#ifndef _agentpp_test_mib_cpp
#define _agentpp_test_mib_cpp

#include <agentpp_test_mib.h>
#include <agent_pp/module_includes.h>

//--AgentGen BEGIN=_INCLUDE
#endif
#endif

#ifndef _agentpp_test_mib_cpp
#define _agentpp_test_mib_cpp

#include <agentpp_test_mib.h>
//--AgentGen END


#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


// Scalar Objects


/**
 *  agentppTestTimeout
 *
 * "Setting this object will be delayed by the given
 * amount of milliseconds. That is, by setting this
 * object to 1000 the corresponding response to
 * that SET request will be delayed by one second."
 */
 
agentppTestTimeout* agentppTestTimeout::instance = 0;


agentppTestTimeout::agentppTestTimeout():
    MibLeaf(oidAgentppTestTimeout, READWRITE, new Gauge32()){
	// This leaf object is a singleton. In order to access it use
	// the static pointer agentppTestTimeout::instance.
	instance = this;
	//--AgentGen BEGIN=agentppTestTimeout::agentppTestTimeout
	//--AgentGen END

}

agentppTestTimeout::~agentppTestTimeout()
{

	//--AgentGen BEGIN=agentppTestTimeout::~agentppTestTimeout
	//--AgentGen END

	// clear singleton reference
	agentppTestTimeout::instance = 0;
}

void agentppTestTimeout::get_request(Request* req, int ind)
{
	//--AgentGen BEGIN=agentppTestTimeout::get_request
	//--AgentGen END
	MibLeaf::get_request(req, ind);
}

void agentppTestTimeout::set_state(unsigned long l)
{
	//--AgentGen BEGIN=agentppTestTimeout::set_state
	//--AgentGen END
	*((Gauge32*)value) = l;
}


int agentppTestTimeout::set(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestTimeout::set
	unsigned long timeout;
	vb.get_value(timeout);
	struct timeval tvptr;
	tvptr.tv_sec  = timeout/1000;    // wait up to sec seconds
	tvptr.tv_usec = (timeout%1000)*1000;

	select(0, NULL, NULL, NULL, &tvptr);
	//--AgentGen END
	return MibLeaf::set(vb);
}

boolean agentppTestTimeout::value_ok(const Vbx& vb)
{
	unsigned long v;
	vb.get_value(v);
	if (!(((v >= 0ul) && (v <= 1000000ul))))
		 return FALSE;
	//--AgentGen BEGIN=agentppTestTimeout::value_ok
	//--AgentGen END
	return TRUE;
}

int agentppTestTimeout::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	//--AgentGen BEGIN=agentppTestTimeout::prepare_set_request
	//--AgentGen END
	return SNMP_ERROR_SUCCESS;
}

//--AgentGen BEGIN=agentppTestTimeout
//--AgentGen END



// Columnar Objects


/**
 *  agentppTestSharedTableCreationTime
 *
 * "The date and time when this row has been created."
 */
 
agentppTestSharedTableCreationTime::agentppTestSharedTableCreationTime(const Oidx& id):
    DateAndTime(id, READONLY, VMODE_DEFAULT)
{
	//--AgentGen BEGIN=agentppTestSharedTableCreationTime::agentppTestSharedTableCreationTime
	//--AgentGen END

}

agentppTestSharedTableCreationTime::~agentppTestSharedTableCreationTime()
{

	//--AgentGen BEGIN=agentppTestSharedTableCreationTime::~agentppTestSharedTableCreationTime
	//--AgentGen END

}

MibEntryPtr agentppTestSharedTableCreationTime::clone()
{
	MibEntryPtr other = new agentppTestSharedTableCreationTime(oid);
	((agentppTestSharedTableCreationTime*)other)->replace_value(value->clone());
	((agentppTestSharedTableCreationTime*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSharedTableCreationTime::clone
	//--AgentGen END
	return other;
}


//--AgentGen BEGIN=agentppTestSharedTableCreationTime
//--AgentGen END




/**
 *  agentppTestSharedTableDelay
 *
 * "The number of 1/100 seconds that a request to this
 * row will be delayed before it is processed."
 */
 
agentppTestSharedTableDelay::agentppTestSharedTableDelay(const Oidx& id):
    MibLeaf(id, READCREATE, new SnmpInt32(0), VMODE_DEFAULT){
	//--AgentGen BEGIN=agentppTestSharedTableDelay::agentppTestSharedTableDelay
	//--AgentGen END

}

agentppTestSharedTableDelay::~agentppTestSharedTableDelay()
{

	//--AgentGen BEGIN=agentppTestSharedTableDelay::~agentppTestSharedTableDelay
	//--AgentGen END

}

MibEntryPtr agentppTestSharedTableDelay::clone()
{
	MibEntryPtr other = new agentppTestSharedTableDelay(oid);
	((agentppTestSharedTableDelay*)other)->replace_value(value->clone());
	((agentppTestSharedTableDelay*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSharedTableDelay::clone
	//--AgentGen END
	return other;
}


//--AgentGen BEGIN=agentppTestSharedTableDelay
//--AgentGen END




/**
 *  agentppTestSharedTableSession
 *
 * "This object denotes the AgentX session ID of the
 * session on whose behalf this row has been created."
 */
 
agentppTestSharedTableSession::agentppTestSharedTableSession(const Oidx& id):
    MibLeaf(id, READONLY, new Gauge32()){
	//--AgentGen BEGIN=agentppTestSharedTableSession::agentppTestSharedTableSession
	//--AgentGen END

}

agentppTestSharedTableSession::~agentppTestSharedTableSession()
{

	//--AgentGen BEGIN=agentppTestSharedTableSession::~agentppTestSharedTableSession
	//--AgentGen END

}

MibEntryPtr agentppTestSharedTableSession::clone()
{
	MibEntryPtr other = new agentppTestSharedTableSession(oid);
	((agentppTestSharedTableSession*)other)->replace_value(value->clone());
	((agentppTestSharedTableSession*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSharedTableSession::clone
	//--AgentGen END
	return other;
}


//--AgentGen BEGIN=agentppTestSharedTableSession
//--AgentGen END




/**
 *  agentppTestSharedTableRowStatus
 *
 * "The row status of the row."
 */
 
agentppTestSharedTableRowStatus::agentppTestSharedTableRowStatus(const Oidx& id):
    snmpRowStatus(id, READCREATE)
{
	//--AgentGen BEGIN=agentppTestSharedTableRowStatus::agentppTestSharedTableRowStatus
	//--AgentGen END

}

agentppTestSharedTableRowStatus::~agentppTestSharedTableRowStatus()
{

	//--AgentGen BEGIN=agentppTestSharedTableRowStatus::~agentppTestSharedTableRowStatus
	//--AgentGen END

}

MibEntryPtr agentppTestSharedTableRowStatus::clone()
{
	MibEntryPtr other = new agentppTestSharedTableRowStatus(oid);
	((agentppTestSharedTableRowStatus*)other)->replace_value(value->clone());
	((agentppTestSharedTableRowStatus*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSharedTableRowStatus::clone
	//--AgentGen END
	return other;
}


//--AgentGen BEGIN=agentppTestSharedTableRowStatus
//--AgentGen END




/**
 *  agentppTestRowCreation
 *
 * "This object can be set to the index of a new row
 * in the agentppTestSharedTable. If a row with the
 * set index already exists, this object will return zero,
 * otherwise it will return the last value set."
 */
 
agentppTestRowCreation::agentppTestRowCreation(const Oidx& id):
    MibLeaf(id, READWRITE, new Gauge32(0), VMODE_DEFAULT){
	//--AgentGen BEGIN=agentppTestRowCreation::agentppTestRowCreation
	//--AgentGen END

}

agentppTestRowCreation::~agentppTestRowCreation()
{

	//--AgentGen BEGIN=agentppTestRowCreation::~agentppTestRowCreation
	//--AgentGen END

}

MibEntryPtr agentppTestRowCreation::clone()
{
	MibEntryPtr other = new agentppTestRowCreation(oid);
	((agentppTestRowCreation*)other)->replace_value(value->clone());
	((agentppTestRowCreation*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestRowCreation::clone
	//--AgentGen END
	return other;
}


//--AgentGen BEGIN=agentppTestRowCreation
//--AgentGen END




/**
 *  agentppTestSparseCol1
 *
 * "By setting this object to its current value the object
 * becomes notAccessible allowing testing of
 * sparse table implementation."
 */
 
agentppTestSparseCol1::agentppTestSparseCol1(const Oidx& id):
    MibLeaf(id, READCREATE, new SnmpInt32(1), VMODE_DEFAULT){
	//--AgentGen BEGIN=agentppTestSparseCol1::agentppTestSparseCol1
	//--AgentGen END

}

agentppTestSparseCol1::~agentppTestSparseCol1()
{

	//--AgentGen BEGIN=agentppTestSparseCol1::~agentppTestSparseCol1
	//--AgentGen END

}

MibEntryPtr agentppTestSparseCol1::clone()
{
	MibEntryPtr other = new agentppTestSparseCol1(oid);
	((agentppTestSparseCol1*)other)->replace_value(value->clone());
	((agentppTestSparseCol1*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSparseCol1::clone
	//--AgentGen END
	return other;
}

void agentppTestSparseCol1::get_request(Request* req, int ind)
{
	//--AgentGen BEGIN=agentppTestSparseCol1::get_request
	//--AgentGen END
	MibLeaf::get_request(req, ind);
}

long agentppTestSparseCol1::get_state()
{
	//--AgentGen BEGIN=agentppTestSparseCol1::get_state
	//--AgentGen END
	return (long)*((SnmpInt32*)value);
}
	
void agentppTestSparseCol1::set_state(long l)
{
	//--AgentGen BEGIN=agentppTestSparseCol1::set_state
	//--AgentGen END
	*((SnmpInt32*)value) = l;
}
	

int agentppTestSparseCol1::set(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol1::set
	SnmpInt32 v;
	vb.get_value(v);
	if (v == get_state())
	    set_access(NOACCESS);
	//--AgentGen END
	return MibLeaf::set(vb);
}

boolean agentppTestSparseCol1::value_ok(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol1::value_ok
	//--AgentGen END
	return TRUE;
}

int agentppTestSparseCol1::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	//--AgentGen BEGIN=agentppTestSparseCol1::prepare_set_request
	//--AgentGen END
	return SNMP_ERROR_SUCCESS;
}

//--AgentGen BEGIN=agentppTestSparseCol1
//--AgentGen END




/**
 *  agentppTestSparseCol2
 *
 * "By setting this object to its current value the object
 * becomes notAccessible allowing testing of
 * sparse table implementation."
 */
 
agentppTestSparseCol2::agentppTestSparseCol2(const Oidx& id):
    MibLeaf(id, READCREATE, new Gauge32(2), VMODE_DEFAULT){
	//--AgentGen BEGIN=agentppTestSparseCol2::agentppTestSparseCol2
	//--AgentGen END

}

agentppTestSparseCol2::~agentppTestSparseCol2()
{

	//--AgentGen BEGIN=agentppTestSparseCol2::~agentppTestSparseCol2
	//--AgentGen END

}

MibEntryPtr agentppTestSparseCol2::clone()
{
	MibEntryPtr other = new agentppTestSparseCol2(oid);
	((agentppTestSparseCol2*)other)->replace_value(value->clone());
	((agentppTestSparseCol2*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSparseCol2::clone
	//--AgentGen END
	return other;
}

void agentppTestSparseCol2::get_request(Request* req, int ind)
{
	//--AgentGen BEGIN=agentppTestSparseCol2::get_request
	//--AgentGen END
	MibLeaf::get_request(req, ind);
}

unsigned long agentppTestSparseCol2::get_state()
{
	//--AgentGen BEGIN=agentppTestSparseCol2::get_state
	//--AgentGen END
	return (unsigned long)*((Gauge32*)value);
}

void agentppTestSparseCol2::set_state(unsigned long l)
{
	//--AgentGen BEGIN=agentppTestSparseCol2::set_state
	//--AgentGen END
	*((Gauge32*)value) = l;
}


int agentppTestSparseCol2::set(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol2::set
	Gauge32 v;
	vb.get_value(v);
	if (v == get_state())
	    set_access(NOACCESS);
	//--AgentGen END
	return MibLeaf::set(vb);
}

boolean agentppTestSparseCol2::value_ok(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol2::value_ok
	//--AgentGen END
	return TRUE;
}

int agentppTestSparseCol2::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	//--AgentGen BEGIN=agentppTestSparseCol2::prepare_set_request
	//--AgentGen END
	return SNMP_ERROR_SUCCESS;
}

//--AgentGen BEGIN=agentppTestSparseCol2
//--AgentGen END




/**
 *  agentppTestSparseCol3
 *
 * "By setting this object to its current value the object
 * becomes notAccessible allowing testing of
 * sparse table implementation."
 */
 
agentppTestSparseCol3::agentppTestSparseCol3(const Oidx& id):
    MibLeaf(id, READCREATE, new OctetStr("3"), VMODE_DEFAULT){
	//--AgentGen BEGIN=agentppTestSparseCol3::agentppTestSparseCol3
	//--AgentGen END

}

agentppTestSparseCol3::~agentppTestSparseCol3()
{

	//--AgentGen BEGIN=agentppTestSparseCol3::~agentppTestSparseCol3
	//--AgentGen END

}

MibEntryPtr agentppTestSparseCol3::clone()
{
	MibEntryPtr other = new agentppTestSparseCol3(oid);
	((agentppTestSparseCol3*)other)->replace_value(value->clone());
	((agentppTestSparseCol3*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSparseCol3::clone
	//--AgentGen END
	return other;
}

void agentppTestSparseCol3::get_request(Request* req, int ind)
{
	//--AgentGen BEGIN=agentppTestSparseCol3::get_request
	//--AgentGen END
	MibLeaf::get_request(req, ind);
}

OctetStr agentppTestSparseCol3::get_state() {
	//--AgentGen BEGIN=agentppTestSparseCol3::get_state
	//--AgentGen END
	return *((OctetStr*)value);
}
void agentppTestSparseCol3::set_state(const OctetStr& s)
{
	//--AgentGen BEGIN=agentppTestSparseCol3::set_state
	//--AgentGen END
	*((OctetStr*)value) = s;
}


int agentppTestSparseCol3::set(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol3::set
	OctetStr v;
	vb.get_value(v);
	if (v == get_state())
	    set_access(NOACCESS);
	//--AgentGen END
	return MibLeaf::set(vb);
}

boolean agentppTestSparseCol3::value_ok(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseCol3::value_ok
	//--AgentGen END
	return TRUE;
}

int agentppTestSparseCol3::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = MibLeaf::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	Vb vb(req->get_value(ind));
	OctetStr v;
	vb.get_value(v);
	if (!(((v.len() >= 0) && (v.len() <= 255))))
		 return SNMP_ERROR_WRONG_LENGTH;
	//--AgentGen BEGIN=agentppTestSparseCol3::prepare_set_request
	//--AgentGen END
	return SNMP_ERROR_SUCCESS;
}

//--AgentGen BEGIN=agentppTestSparseCol3
//--AgentGen END




/**
 *  agentppTestSparseRowStatus
 *
 * "Use this column to create a row in the test table."
 */
 
agentppTestSparseRowStatus::agentppTestSparseRowStatus(const Oidx& id):
    snmpRowStatus(id, READCREATE)
{
	//--AgentGen BEGIN=agentppTestSparseRowStatus::agentppTestSparseRowStatus
	//--AgentGen END

}

agentppTestSparseRowStatus::~agentppTestSparseRowStatus()
{

	//--AgentGen BEGIN=agentppTestSparseRowStatus::~agentppTestSparseRowStatus
	//--AgentGen END

}

MibEntryPtr agentppTestSparseRowStatus::clone()
{
	MibEntryPtr other = new agentppTestSparseRowStatus(oid);
	((agentppTestSparseRowStatus*)other)->replace_value(value->clone());
	((agentppTestSparseRowStatus*)other)->set_reference_to_table(my_table);
	//--AgentGen BEGIN=agentppTestSparseRowStatus::clone
	//--AgentGen END
	return other;
}

long agentppTestSparseRowStatus::get_state()
{
	//--AgentGen BEGIN=agentppTestSparseRowStatus::get_state
	//--AgentGen END
	return (long)*((SnmpInt32*)value);
}
	
void agentppTestSparseRowStatus::set_state(long l)
{
	//--AgentGen BEGIN=agentppTestSparseRowStatus::set_state
	//--AgentGen END
	*((SnmpInt32*)value) = l;
}
	

int agentppTestSparseRowStatus::set(const Vbx& vb)
{
	//--AgentGen BEGIN=agentppTestSparseRowStatus::set
	//--AgentGen END
	return snmpRowStatus::set(vb);
}

int agentppTestSparseRowStatus::prepare_set_request(Request* req, int& ind)
{
	int status;
	if ((status = snmpRowStatus::prepare_set_request(req, ind)) !=
	    SNMP_ERROR_SUCCESS) return status;

	//--AgentGen BEGIN=agentppTestSparseRowStatus::prepare_set_request
	//--AgentGen END
	return SNMP_ERROR_SUCCESS;
}

//--AgentGen BEGIN=agentppTestSparseRowStatus
//--AgentGen END



// Tables


/**
 *  agentppTestSharedEntry
 *
 * "A row of a shared table. Each row is allocated and
 * registered in random intervals."
 */
 
agentppTestSharedEntry* agentppTestSharedEntry::instance = 0;

const index_info indAgentppTestSharedEntry[1] = {
	{ sNMP_SYNTAX_INT, FALSE, 1, 1 }};

agentppTestSharedEntry::agentppTestSharedEntry():
   MibTable(oidAgentppTestSharedEntry, indAgentppTestSharedEntry, 1)
{
	// This table object is a singleton. In order to access it use
	// the static pointer agentppTestSharedEntry::instance.
	instance = this;

	add_col(new agentppTestSharedTableCreationTime(colAgentppTestSharedTableCreationTime));
	add_col(new agentppTestSharedTableDelay(colAgentppTestSharedTableDelay));
	add_col(new agentppTestSharedTableSession(colAgentppTestSharedTableSession));
	add_col(new agentppTestSharedTableRowStatus(colAgentppTestSharedTableRowStatus));
	//--AgentGen BEGIN=agentppTestSharedEntry::agentppTestSharedEntry
	//--AgentGen END
}

agentppTestSharedEntry::~agentppTestSharedEntry()
{
	//--AgentGen BEGIN=agentppTestSharedEntry::~agentppTestSharedEntry
	//--AgentGen END
	// clear singleton reference
	agentppTestSharedEntry::instance = 0;
}

//--AgentGen BEGIN=agentppTestSharedEntry
//--AgentGen END


/**
 *  agentppTestSessionsEntry
 *
 * "A row of this table is created by each subagent
 * session that implements the AGENTPP-TEST-MIB."
 */
 
agentppTestSessionsEntry* agentppTestSessionsEntry::instance = 0;

const index_info indAgentppTestSessionsEntry[1] = {
	{ sNMP_SYNTAX_INT, FALSE, 1, 1 }};

agentppTestSessionsEntry::agentppTestSessionsEntry():
   MibTable(oidAgentppTestSessionsEntry, indAgentppTestSessionsEntry, 1)
{
	// This table object is a singleton. In order to access it use
	// the static pointer agentppTestSessionsEntry::instance.
	instance = this;

	add_col(new agentppTestRowCreation(colAgentppTestRowCreation));
	//--AgentGen BEGIN=agentppTestSessionsEntry::agentppTestSessionsEntry
	//--AgentGen END
}

agentppTestSessionsEntry::~agentppTestSessionsEntry()
{
	//--AgentGen BEGIN=agentppTestSessionsEntry::~agentppTestSessionsEntry
	//--AgentGen END
	// clear singleton reference
	agentppTestSessionsEntry::instance = 0;
}

//--AgentGen BEGIN=agentppTestSessionsEntry
//--AgentGen END


/**
 *  agentppTestSparseEntry
 *
 * "A row of a sparese table can be created and
 * deleted via its row status column."
 */
 
agentppTestSparseEntry* agentppTestSparseEntry::instance = 0;

const index_info indAgentppTestSparseEntry[1] = {
	{ sNMP_SYNTAX_OCTETS, FALSE, 0, 255 }
};

agentppTestSparseEntry::agentppTestSparseEntry():
   MibTable(oidAgentppTestSparseEntry, indAgentppTestSparseEntry, 1)
{
	// This table object is a singleton. In order to access it use
	// the static pointer agentppTestSparseEntry::instance.
	instance = this;

	add_col(new agentppTestSparseCol1(colAgentppTestSparseCol1));
	add_col(new agentppTestSparseCol2(colAgentppTestSparseCol2));
	add_col(new agentppTestSparseCol3(colAgentppTestSparseCol3));
	add_col(new agentppTestSparseRowStatus(colAgentppTestSparseRowStatus));
	//--AgentGen BEGIN=agentppTestSparseEntry::agentppTestSparseEntry
	//--AgentGen END
}

agentppTestSparseEntry::~agentppTestSparseEntry()
{
	//--AgentGen BEGIN=agentppTestSparseEntry::~agentppTestSparseEntry
	//--AgentGen END
	// clear singleton reference
	agentppTestSparseEntry::instance = 0;
}

void agentppTestSparseEntry::get_request(Request* req, int ind)
{
	//--AgentGen BEGIN=agentppTestSparseEntry::get_request
	//--AgentGen END
	MibTable::get_request(req, ind);
}

int agentppTestSparseEntry::prepare_set_request(Request* req, int& ind)
{
	//--AgentGen BEGIN=agentppTestSparseEntry::prepare_set_request
	//--AgentGen END
	return MibTable::prepare_set_request(req, ind);
}

//--AgentGen BEGIN=agentppTestSparseEntry
//--AgentGen END

// Notifications

// Group
agentpp_test_mib::agentpp_test_mib():
   MibGroup("1.3.6.1.4.1.4976.6.3", "agentpp_test_mib")
{
	//--AgentGen BEGIN=agentpp_test_mib::agentpp_test_mib
	//--AgentGen END
	add(new agentppTestTimeout());
	add(new agentppTestSharedEntry());
	add(new agentppTestSessionsEntry());
	add(new agentppTestSparseEntry());
	//--AgentGen BEGIN=agentpp_test_mib::agentpp_test_mib:post
	//--AgentGen END
}

//--AgentGen BEGIN=agentpp_test_mib
//--AgentGen END



//--AgentGen BEGIN=_CLASSES
//--AgentGen END


#ifdef AGENTPP_NAMESPACE
}
#endif


//--AgentGen BEGIN=_END
//--AgentGen END

#endif
