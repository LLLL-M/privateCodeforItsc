/*_############################################################################
  _## 
  _##  snmp_textual_conventions.cpp  
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
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#endif
#include <agent_pp/snmp_textual_conventions.h>
#include <agent_pp/system_group.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
using namespace Agentpp;
#endif

/*--------------------- class snmpDisplayString -------------------------*/


SnmpDisplayString::SnmpDisplayString(const Oidx& id, mib_access a, 
				     OctetStr* s):
  MibLeaf(id, a, s)
{
	min_size = 0;
	max_size = 255;
}

SnmpDisplayString::SnmpDisplayString(const Oidx& id, mib_access a, 
				     OctetStr* s, boolean d):
  MibLeaf(id, a, s, d)
{
	min_size = 0;
	max_size = 255;
}

SnmpDisplayString::SnmpDisplayString(const Oidx& id, mib_access a, 
				     OctetStr* s, boolean d, 
				     int min_sz, int max_sz):
  MibLeaf(id, a, s, d)
{
	min_size = min_sz;
	max_size = max_sz;
}

SnmpDisplayString::~SnmpDisplayString()
{
}

MibEntryPtr SnmpDisplayString::clone()
{
	MibEntryPtr other = 
	  new SnmpDisplayString(oid, access, (OctetStr*)value->clone(), 
				value_mode);
	((SnmpDisplayString*)other)->set_reference_to_table(my_table);
	return other;
}

int SnmpDisplayString::prepare_set_request(Request* req, int& ind) 
{
	int s = MibLeaf::prepare_set_request(req, ind);
	if (s != SNMP_ERROR_SUCCESS) return s;
	OctetStr ostr;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) 
		return SNMP_ERROR_WRONG_TYPE;
	if ((ostr.len() < min_size) ||
	    (ostr.len() > max_size)) return SNMP_ERROR_WRONG_LENGTH;
	return SNMP_ERROR_SUCCESS;
}

boolean SnmpDisplayString::value_ok(const Vbx& vb)
{
	OctetStr ostr;
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) return FALSE;
	for (unsigned int i=0; i<ostr.len(); i++) {
		if (ostr[i] > 127u)
			return FALSE;
		// check for CR NULL or CR LF
		if (ostr[i] == '\r') {
			if (i+1 == ostr.len()) return FALSE;
			if ((ostr[i+1] != 0) && (ostr[i+1] != '\n'))
				return FALSE;
		}
	}
	return TRUE;
}

/*--------------------- class SnmpAdminString -------------------------*/


SnmpAdminString::SnmpAdminString(const Oidx& id, mib_access a, 
				 OctetStr* s):
  OctetStrMinMax(id, a, s, VMODE_NONE, 0, 255)
{
}

SnmpAdminString::SnmpAdminString(const Oidx& id, mib_access a, 
				 OctetStr* s, int m):
  OctetStrMinMax(id, a, s, m, 0, 255)
{
}

SnmpAdminString::SnmpAdminString(const Oidx& id, mib_access a, 
				 OctetStr* s, int m, 
				 int min_sz, int max_sz):
  OctetStrMinMax(id, a, s, m, min_sz, max_sz)
{
}

SnmpAdminString::~SnmpAdminString()
{
}

MibEntryPtr SnmpAdminString::clone()
{
	MibEntryPtr other = 
	  new SnmpAdminString(oid, access, (OctetStr*)value->clone(), 
			      value_mode, min, max);
	((SnmpAdminString*)other)->set_reference_to_table(my_table);
	return other;
}

OctetStr SnmpAdminString::get() 
{
  return *((OctetStr*)value);
}



/*--------------------- class SnmpEngineID -------------------------*/


SnmpEngineID::SnmpEngineID(const Oidx& id, mib_access a, 
				     OctetStr* s):
  MibLeaf(id, a, s)
{
}

SnmpEngineID::SnmpEngineID(const Oidx& id, mib_access a, 
				     OctetStr* s, boolean d):
  MibLeaf(id, a, s, d)
{
}

SnmpEngineID::~SnmpEngineID()
{
}

MibEntryPtr SnmpEngineID::clone()
{
	MibEntryPtr other = 
	  new SnmpEngineID(oid, access, (OctetStr*)value->clone(), 
			   value_mode);
	((SnmpEngineID*)other)->set_reference_to_table(my_table);
	return other;
}

int SnmpEngineID::prepare_set_request(Request* req, int& ind) 
{
	int s = MibLeaf::prepare_set_request(req, ind);
	if (s != SNMP_ERROR_SUCCESS) return s;

	OctetStr ostr;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) 
		return SNMP_ERROR_WRONG_TYPE;
	if ((ostr.len() < 5) ||
	    (ostr.len() > 32)) return SNMP_ERROR_WRONG_LENGTH;
	return SNMP_ERROR_SUCCESS;
}

OctetStr SnmpEngineID::create_engine_id(const OctetStr& userText) 
{
	// 8 = v3EngineID, 1370h = 4976 = AGENT++ enterprise ID
	OctetStr engineID((const unsigned char*)"\x80\x00\x13\x70\x05", 5);
	engineID += userText;
	return engineID;
}


OctetStr SnmpEngineID::create_engine_id(unsigned short p) 
{
	// 8 = v3EngineID, 1370h = 4976 = AGENT++ enterprise ID
	OctetStr engineID((const unsigned char*)"\x80\x00\x13\x70\x05", 5);
	unsigned char port[3];
	port[0] = p/256;
	port[1] = p%256;
	port[2] = 0;
	char hname[255];
	size_t len = 255;
	if (gethostname(hname, len) == 0) {
		OctetStr host((unsigned char*)hname, 
			      (strlen(hname)>23) ? 23 : strlen(hname));
		engineID += OctetStr(host);
		engineID += OctetStr(port, 2);
	}
	else {
		time_t ct = time(0);
		char* tp = ctime(&ct);
		OctetStr t((unsigned char*)tp, 
			   (strlen(tp)>23) ? 23 : strlen(tp));
		engineID += t;
		engineID += OctetStr(port, 2);
	}
	return engineID;
}

/*--------------------------- class snmpSpinLock -------------------------*/

/**
 *  SnmpTagValue
 *
 */

SnmpTagValue::SnmpTagValue(const Oidx& id):
   MibLeaf(id, READCREATE, new OctetStr(""), VMODE_DEFAULT)
{
}

SnmpTagValue::SnmpTagValue(const Oidx& id, mib_access a, OctetStr* v, int m):
   MibLeaf(id, a, v, m)
{
}

SnmpTagValue::~SnmpTagValue()
{
}

MibEntryPtr SnmpTagValue::clone()
{
	MibEntryPtr other = new SnmpTagValue(oid);
	((SnmpTagValue*)other)->replace_value(value->clone());
	((SnmpTagValue*)other)->set_reference_to_table(my_table);
	return other;
}

boolean SnmpTagValue::value_ok(const Vbx& vb)
{
	OctetStr ostr;
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) return FALSE;
	int length = ostr.len();
	if (length == 0) return TRUE;
	if ((length < 0) || (length > 255)) return FALSE; 	

	for (int i=0; i<length; i++) {
		if (is_delimiter(ostr[i])) return FALSE;
	}
	return TRUE;
}

boolean SnmpTagValue::is_delimiter(char c) 
{
	return ((c == 32) || (c == 9) || (c == 13) || (c == 11));
}

int SnmpTagValue::prepare_set_request(Request* req, int& ind) 
{
	int s = MibLeaf::prepare_set_request(req, ind);
	if (s != SNMP_ERROR_SUCCESS) return s;

	OctetStr ostr;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) 
		return SNMP_ERROR_WRONG_TYPE;
	if ((ostr.len()<= 0) || (ostr.len() > 255))
		return SNMP_ERROR_WRONG_LENGTH;
	return SNMP_ERROR_SUCCESS;
}


/**
 *  SnmpTagList
 *
 */

SnmpTagList::SnmpTagList(const Oidx& id, mib_access a, OctetStr* v, int m):
   MibLeaf(id, a, v, m)
{
}

SnmpTagList::SnmpTagList(const Oidx& id):
   MibLeaf(id, READCREATE, new OctetStr(""), VMODE_DEFAULT)
{

}

SnmpTagList::~SnmpTagList()
{
}

MibEntryPtr SnmpTagList::clone()
{
	MibEntryPtr other = new SnmpTagList(oid, access, 0, get_value_mode());
	((SnmpTagList*)other)->replace_value(value->clone());
	((SnmpTagList*)other)->set_reference_to_table(my_table);
	return other;
}

boolean SnmpTagList::value_ok(const Vbx& vb)
{
	OctetStr ostr;
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) return FALSE;
	
	// pointer into ostr!
	char* s = (char*)ostr.data();
	if (s) {
		int length = ostr.len();
		if (length > 255) return FALSE; 

		if ((length>0) && 
		    (SnmpTagValue::is_delimiter(s[0]))) return FALSE;
		if ((length>0) && (SnmpTagValue::is_delimiter(s[length-1])))
			return FALSE;
		for (int i=0; i<length; i++) {
			if ((SnmpTagValue::is_delimiter(s[i])) && 
			    ((i+1<length) && 
			     (SnmpTagValue::is_delimiter(s[i+1])))) 
				return FALSE;
		}
	}
	return TRUE;
}

boolean SnmpTagList::contains(const char* tag) 
{
	if (!tag) return FALSE;

	int len = ((OctetStr*)value)->len();
	char* l = new char[len+1];
	strncpy(l, (char*)((OctetStr*)value)->data(), len);
	l[len] = 0;

	LOG_BEGIN(DEBUG_LOG | 10);
	LOG("SnmpTagList: contains: (taglist)(tag)");
	LOG(l);
	LOG(tag);
	LOG_END;

	char* start = l;
	while ((l+strlen(l) - start >= (int)strlen(tag)) && 
	       (start = strstr(start, tag))) {

	  if (((start == l) || (SnmpTagValue::is_delimiter(*(start-1)))) &&
	      ((l+strlen(l)-start == (int)strlen(tag)) ||
	       (SnmpTagValue::is_delimiter(*(start+strlen(tag)))))) {
		delete[] l;
		return TRUE;
	  }
	  start++;
	}
	delete[] l;
	return FALSE;
}



/**
 *  TestAndIncr
 *
 */

TestAndIncr::TestAndIncr(const Oidx& o):
  MibLeaf(o, READWRITE, new SnmpInt32(0))
{
}

TestAndIncr::~TestAndIncr()
{
}

long TestAndIncr::get_state()
{
	return (long)*((SnmpInt32*)value);
}

void TestAndIncr::set_state(long l)
{
	*((SnmpInt32*)value) = l;
}

int TestAndIncr::set(const Vbx& vb)
{
	// place code for handling operations triggered
	// by this set here
	int status = MibLeaf::set(vb);
	if (get_state() == 2147483647) {
		set_state(0);
	}
	else {
		set_state(get_state()+1);
	}
	return status;
}

int TestAndIncr::prepare_set_request(Request* req, int& reqind) 
{
	long v = 0;
	if (req->get_value(reqind).get_value(v) != SNMP_CLASS_SUCCESS)
	    return SNMP_ERROR_WRONG_TYPE;
	if (v != get_state()) return SNMP_ERROR_INCONSIST_VAL;
	return SNMP_ERROR_SUCCESS;
}

boolean TestAndIncr::value_ok(const Vbx& vb)
{
	long v;
	if (vb.get_value(v) != SNMP_CLASS_SUCCESS)
	    return FALSE;
	if ((v < 0) || (v > 2147483647)) return FALSE;
	return TRUE;
}

/*--------------------------- class storageType -------------------------*/

StorageType::StorageType(const Oidx& o, int def):
  MibLeaf(o, READCREATE, new SnmpInt32(def), VMODE_DEFAULT)
{
}

MibEntryPtr StorageType::clone()
{
	MibEntryPtr other = new StorageType(oid, get_state());
	((StorageType*)other)->replace_value(value->clone());
	((StorageType*)other)->set_reference_to_table(my_table);
	return other;
}

boolean StorageType::value_ok(const Vbx& vb)
{
	long v;
	if (vb.get_value(v) != SNMP_CLASS_SUCCESS)
	    return FALSE;
	if ((v < 1) || (v > 5)) return FALSE;
	if ((valid()) && (get_state() < 4) && (v >=4)) return FALSE;
	if ((valid()) && (get_state() >= 4)) return FALSE;
	return TRUE;
}

boolean StorageType::row_is_volatile()
{
	return (get_state() <= 2);
}

void StorageType::set_state(long state) 
{
	if ((state >= 1) && (state <= 5)) {
		*((SnmpInt32*)value) = state;
	}
}

long StorageType::get_state() 
{
	return (long)*((SnmpInt32*)value);
}


/*--------------------------- class StorageTable ------------------------*/

StorageTable::StorageTable(const StorageTable& other):
  MibTable(other) 
{
	storage_type = other.storage_type;
}

StorageTable::StorageTable(const Oidx& o): MibTable(o)
{
	storage_type = 0;
}

StorageTable::StorageTable(const Oidx& o, unsigned int ilen): 
  MibTable(o, ilen)
{
	storage_type = 0;
}

StorageTable::StorageTable(const Oidx& o, unsigned int ilen, boolean a):
  MibTable(o, ilen, a)
{
	storage_type = 0;
}

StorageTable::StorageTable(const Oidx& o, 
			   const index_info* istruc, unsigned int ilen):
  MibTable(o, istruc, ilen)
{
	storage_type = 0;
}

StorageTable::~StorageTable()
{
}

boolean StorageTable::is_persistent(MibTableRow* row) 
{
	if (row->get_nth(storage_type)) {
		if (((StorageType*)(row->get_nth(storage_type)))->
		    row_is_volatile()) {
			return FALSE;
		}
	}
	return TRUE;
}	

void StorageTable::add_storage_col(StorageType* col) 
{
	storage_type = generator.size();
	MibTable::add_col(col);
}

void StorageTable::set_storage_type(MibTableRow* row, int storageType) 
{
	if (row->get_nth(storage_type)) {
	    ((StorageType*)(row->get_nth(storage_type)))->
		set_state(storageType);
	}
}

void StorageTable::reset() 
{
	OidListCursor<MibTableRow> cur;
	for (cur.init(&content); cur.get();) {
		long type = ((StorageType*)
			     (cur.get()->get_nth(storage_type)))->get_state();
		if ((type != 4) && (type != 5)) {
		    MibTableRow* victim = cur.get();
		    cur.next();
		    delete content.remove(victim);
		}
		else {
		    cur.next();
		}
	}
}


/*------------------------- class SnmpInt32MinMax ------------------------*/

SnmpInt32MinMax::SnmpInt32MinMax(const Oidx& o, mib_access _access, 
				 const int def_val, 
                                 int vmode, int _min, int _max):
  MibLeaf(o, _access, new SnmpInt32(def_val), vmode) 
{
	min = _min;
	max = _max;
}

SnmpInt32MinMax::SnmpInt32MinMax(const Oidx& o, mib_access _access, 
                                 int _min, int _max):
  MibLeaf(o, _access, new SnmpInt32(0), VMODE_NONE) 
{
	min = _min;
	max = _max;
}


MibEntryPtr SnmpInt32MinMax::clone()
{
	MibEntryPtr other = 
	  new SnmpInt32MinMax(oid, access, 0, get_value_mode(), min, max);
	((SnmpInt32MinMax*)other)->replace_value(value->clone());
	((SnmpInt32MinMax*)other)->set_reference_to_table(my_table);
	return other;
}


boolean SnmpInt32MinMax::value_ok(const Vbx& v)
{
	SnmpInt32 si;
	if (v.get_value(si) != SNMP_CLASS_SUCCESS) return FALSE;
	if (((int)si<min) || ((int)si>max)) return FALSE;
	return TRUE;
}

int SnmpInt32MinMax::get_state() 
{
	return *((SnmpInt32*)value);
} 

void SnmpInt32MinMax::set_state(int i)
{
	*((SnmpInt32*)value) = i;
}

/*------------------------- class OctetStrMinMax ------------------------*/

OctetStrMinMax::OctetStrMinMax(const Oidx& o, mib_access _access, 
			       OctetStr* def_val, 
			       int vmode, u_int _min, u_int _max):
  MibLeaf(o, _access, def_val, vmode) 
{
	min = _min;
	max = _max;
}

OctetStrMinMax::OctetStrMinMax(const Oidx& o, mib_access _access, 
			       u_int _min, u_int _max):
  MibLeaf(o, _access, new OctetStr(""), VMODE_NONE) 
{
	min = _min;
	max = _max;
}


MibEntryPtr OctetStrMinMax::clone()
{
	MibEntryPtr other = 
	  new OctetStrMinMax(oid, access, 0, get_value_mode(), min, max);
	((OctetStrMinMax*)other)->replace_value(value->clone());
	((OctetStrMinMax*)other)->set_reference_to_table(my_table);
	return other;
}

int OctetStrMinMax::prepare_set_request(Request* req, int& ind) 
{
	OctetStr ostr;
	Vbx vb(req->get_value(ind));
	if (vb.get_value(ostr) != SNMP_CLASS_SUCCESS) 
		return SNMP_ERROR_WRONG_TYPE;
	if ((ostr.len()<min) || (ostr.len() > max))
		return SNMP_ERROR_WRONG_LENGTH;
	return MibLeaf::prepare_set_request(req, ind);
}

/*----------------- class SnmpMessageProcessingModel -------------------*/

SnmpMessageProcessingModel::SnmpMessageProcessingModel(
  const Oidx& id, mib_access a, int i, int m):
  SnmpInt32MinMax(id, a, i, m, 0, 3)
{
}

SnmpMessageProcessingModel::~SnmpMessageProcessingModel()
{
}

MibEntryPtr SnmpMessageProcessingModel::clone()
{
	MibEntryPtr other = 
	  new SnmpMessageProcessingModel(oid, access, 0, get_value_mode());
	((SnmpMessageProcessingModel*)other)->replace_value(value->clone());
	((SnmpMessageProcessingModel*)other)->set_reference_to_table(my_table);
	return other;
}


/*--------------------- class SnmpSecurityLevel ------------------------*/

SnmpSecurityLevel::SnmpSecurityLevel(const Oidx& id, mib_access a, 
				     int i, int m):
   SnmpInt32MinMax(id, a, i, m, 1, 3)
{
}

SnmpSecurityLevel::~SnmpSecurityLevel()
{
}

MibEntryPtr SnmpSecurityLevel::clone()
{
	MibEntryPtr other = new SnmpSecurityLevel(oid, access, 0, 
					      get_value_mode());
	((SnmpSecurityLevel*)other)->replace_value(value->clone());
	((SnmpSecurityLevel*)other)->set_reference_to_table(my_table);
	return other;
}

/*--------------------- class SnmpSecurityModel ------------------------*/

SnmpSecurityModel::SnmpSecurityModel(const Oidx& id, mib_access a, 
				     int i, int m):
   SnmpInt32MinMax(id, a, i, m, 0, 3)
{
}

SnmpSecurityModel::~SnmpSecurityModel()
{
}

MibEntryPtr SnmpSecurityModel::clone()
{
	MibEntryPtr other = new SnmpSecurityModel(oid, access, 0, 
					      get_value_mode());
	((SnmpSecurityModel*)other)->replace_value(value->clone());
	((SnmpSecurityModel*)other)->set_reference_to_table(my_table);
	return other;
}

/*------------------------ class TimeStamp -----------------------------*/

TimeStamp::TimeStamp(const Oidx& id, mib_access a, int m):
   MibLeaf(id, a, new TimeTicks(0), m)
{
}

TimeStamp::~TimeStamp()
{
}

void TimeStamp::update()
TS_SYNCHRONIZED(
{
	*((TimeTicks*)value) = sysUpTime::get();
})

MibEntryPtr TimeStamp::clone()
{
	MibEntryPtr other = new TimeStamp(oid, access, value_mode);
	((TimeStamp*)other)->replace_value(value->clone());
	((TimeStamp*)other)->set_reference_to_table(my_table);
	return other;
}

/*----------------------- class TimeStampTable --------------------------*/

TimeStampTable::TimeStampTable(const Oidx& o, const index_info* inf, 
			       unsigned int sz, TimeStamp* lc):
   MibTable(o, inf, sz)
{
	lastChange = lc;
}

TimeStampTable::~TimeStampTable()
{
	lastChange = 0;
}

void TimeStampTable::row_added(MibTableRow*, const Oidx&, 
			       MibTable*)
{
	lastChange->update();
}

void TimeStampTable::row_delete(MibTableRow*, const Oidx&, 
				MibTable*)
{
	lastChange->update();
}

void TimeStampTable::updated()
{
	lastChange->update();
}
	
/*----------------------- class DateAndTime --------------------------*/

DateAndTime::DateAndTime(const Oidx& id, 
			 mib_access a, 
			 int mode):
   MibLeaf(id, a, new OctetStr(), mode)
{
	update();
}

DateAndTime::~DateAndTime()
{
}

MibEntryPtr DateAndTime::clone()
{
	MibEntryPtr other = new DateAndTime(oid, access, value_mode);
	((DateAndTime*)other)->replace_value(value->clone());
	((DateAndTime*)other)->set_reference_to_table(my_table);
	return other;
}

OctetStr DateAndTime::get_state()
{
	return *((OctetStr*)value);
}

void DateAndTime::set_state(const OctetStr& s)
{
	*((OctetStr*)value) = s;
}

void DateAndTime::update() 
{
	time_t c = sysUpTime::get_currentTime();
	struct tm* dt = localtime(&c);
	if (!dt) return; // TODO: possibly log an error; Use localtime_r!
	OctetStr val;
	val += (unsigned char)((dt->tm_year+1900) >> 8) & 0xFF;
	val += (unsigned char)(dt->tm_year+1900) & 0xFF;
	val += (unsigned char)dt->tm_mon+1;
	val += (unsigned char)dt->tm_mday;
	val += (unsigned char)dt->tm_hour;
	val += (unsigned char)dt->tm_min;
	val += (unsigned char)dt->tm_sec;
	val += (unsigned char)0;
#if defined __FreeBSD__ || defined __APPLE__
	if (dt->tm_gmtoff >= 0)
		val += '+';
	else
		val += '-';
	unsigned int tz = (unsigned int)abs(dt->tm_gmtoff);
	long timezone = dt->tm_gmtoff;
#else
	// initialize timezone needed?
	// tzset();
#ifdef __CYGWIN__
	long timezone = _timezone;
#endif
	if (timezone < 0)
		val += '+';
	else
		val += '-';
	unsigned int tz = (unsigned int)abs(timezone);
#endif
	val += (unsigned char)((tz / 3600) + 
			       ((dt->tm_isdst > 0)? ((timezone>0)?-1:1) : 0));
	val += (unsigned char)((tz % 3600)/60);
	set_state(val);
}


