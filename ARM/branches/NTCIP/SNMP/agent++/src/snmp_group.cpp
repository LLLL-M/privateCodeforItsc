/*_############################################################################
  _## 
  _##  snmp_group.cpp  
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

#include <agent_pp/snmp_counters.h>
#include <agent_pp/snmp_group.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
using namespace Agentpp;
#endif


void snmpInPkts::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inPkts();
	MibLeaf::get_request(req, index);
}

void snmpOutPkts::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outPkts();
	MibLeaf::get_request(req, index);
}

void snmpInBadVersions::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inBadVersions();
	MibLeaf::get_request(req, index);
}

void snmpInBadCommunityNames::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inBadCommunityNames();
	MibLeaf::get_request(req, index);
}

void snmpInBadCommunityUses::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inBadCommunityUses();
	MibLeaf::get_request(req, index);
}

void snmpInASNParseErrs::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inASNParseErrs();
	MibLeaf::get_request(req, index);
}

void snmpInTooBigs::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inTooBigs();
	MibLeaf::get_request(req, index);
}

void snmpInNoSuchNames::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inNoSuchNames();
	MibLeaf::get_request(req, index);
}

void snmpInBadValues::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inBadValues();
	MibLeaf::get_request(req, index);
}

void snmpInReadOnlys::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inReadOnlys();
	MibLeaf::get_request(req, index);
}

void snmpInGenErrs::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inGenErrs();
	MibLeaf::get_request(req, index);
}

void snmpInTotalReqVars::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inTotalReqVars();
	MibLeaf::get_request(req, index);
}

void snmpInTotalSetVars::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inTotalSetVars();
	MibLeaf::get_request(req, index);
}

void snmpInGetRequests::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inGetRequests();
	MibLeaf::get_request(req, index);
}

void snmpInGetNexts::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inGetNexts();
	MibLeaf::get_request(req, index);
}

void snmpInSetRequests::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inSetRequests();
	MibLeaf::get_request(req, index);
}

void snmpInGetResponses::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inGetResponses();
	MibLeaf::get_request(req, index);
}

void snmpInTraps::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::inTraps();
	MibLeaf::get_request(req, index);
}

void snmpOutTooBigs::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outTooBigs();
	MibLeaf::get_request(req, index);
}

void snmpOutNoSuchNames::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outNoSuchNames();
	MibLeaf::get_request(req, index);
}

void snmpOutBadValues::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outBadValues();
	MibLeaf::get_request(req, index);
}

void snmpOutGenErrs::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outGenErrs();
	MibLeaf::get_request(req, index);
}

void snmpOutGetRequests::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outGetRequests();
	MibLeaf::get_request(req, index);
}

void snmpOutGetNexts::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outGetNexts();
	MibLeaf::get_request(req, index);
}

void snmpOutSetRequests::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outSetRequests();
	MibLeaf::get_request(req, index);
}

void snmpOutGetResponses::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outGetResponses();
	MibLeaf::get_request(req, index);
}

void snmpOutTraps::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::outTraps();
	MibLeaf::get_request(req, index);
}

void snmpSilentDrops::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::silentDrops();
	MibLeaf::get_request(req, index);
}

void snmpProxyDrops::get_request(Request* req, int index)
{
	*((Counter32*)value) = MibIIsnmpCounters::proxyDrops();
	MibLeaf::get_request(req, index);
}


/**
 *  snmpEnableAuthenTraps
 *
 */

snmpEnableAuthenTraps* snmpEnableAuthenTraps::instance = 0;

snmpEnableAuthenTraps::snmpEnableAuthenTraps():
   MibLeaf(oidSnmpEnableAuthenTraps, READWRITE, new SnmpInt32(2))
{
	instance = this;
}

snmpEnableAuthenTraps::~snmpEnableAuthenTraps()
{
}

long snmpEnableAuthenTraps::get_state()
{
	return (long)*((SnmpInt32*)value);
}

boolean snmpEnableAuthenTraps::value_ok(const Vbx& vb)
{
	long v;
	if (vb.get_value(v) != SNMP_CLASS_SUCCESS)
	    return FALSE;
	if ((v != 1)
	     && (v != 2)) return FALSE;
	return TRUE;
}


/**********************************************************************
 * 
 *  class snmpGroup
 *
 **********************************************************************/


snmpGroup::snmpGroup(): MibGroup(oidSnmpGroup, "snmpGroup")
{
	MibIIsnmpCounters::reset();

	add(new snmpInPkts());
	add(new snmpOutPkts());
	add(new snmpInBadVersions());
	add(new snmpInBadCommunityNames());
	add(new snmpInBadCommunityUses());
	add(new snmpInASNParseErrs());
	add(new snmpInTooBigs());
	add(new snmpInNoSuchNames());
	add(new snmpInBadValues());
	add(new snmpInReadOnlys());
	add(new snmpInGenErrs());
	add(new snmpInTotalReqVars());
	add(new snmpInTotalSetVars());
	add(new snmpInGetRequests());
	add(new snmpInGetNexts());
	add(new snmpInSetRequests());
	add(new snmpInGetResponses());
	add(new snmpInTraps());
	add(new snmpOutTooBigs());
	add(new snmpOutNoSuchNames());
	add(new snmpOutBadValues());
	add(new snmpOutGenErrs());
	add(new snmpOutGetRequests());
	add(new snmpOutGetNexts());
	add(new snmpOutSetRequests());
	add(new snmpOutGetResponses());
	add(new snmpOutTraps());
	add(new snmpEnableAuthenTraps());
	add(new snmpSilentDrops());
	add(new snmpProxyDrops());
}	
