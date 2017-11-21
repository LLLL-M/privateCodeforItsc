/*_############################################################################
  _## 
  _##  mib_context.cpp  
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


#include <string.h>
#include <agent_pp/mib_context.h>
#include <agent_pp/tools.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


/*--------------------------- class MibGroup --------------------------*/

MibGroup::MibGroup(const Oidx& o): MibEntry(o, NOACCESS)
{
	persistencyName = 0;
	timeout = 0;
}

MibGroup::MibGroup(const Oidx& o, const OctetStr& p): MibEntry(o, NOACCESS)
{
	persistencyName = new OctetStr(p);
	timeout = 0;
}

MibGroup::~MibGroup()
{
	if (persistencyName) delete persistencyName;
	content.clear();
}

mib_type MibGroup::type() const
{
	return AGENTPP_GROUP;
}

MibEntryPtr MibGroup::add(MibEntryPtr item)
{
	if ((item->type() == AGENTPP_LEAF) &&
	    (item->get_access() == NOACCESS)) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("MibGroup: cannot add not-accessible MIB object (oid) to (group)");
		LOG(item->key()->get_printable());
		LOG(oid.get_printable());
		LOG_END;
		return 0;
	}
	/* This has been removed in v3.5.15 because it had more drawbacks than
	   use.
	// check if item is in subtree of this group
	// if not we assume that the oid of item is a sub id to this group
	if (!(item->key()->in_subtree_of(oid))) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("MibGroup: cannot add MIB object with (oid) to (group)");
		LOG(item->key()->get_printable());
		LOG(oid.get_printable());
		LOG_END;
		return 0;
	}
	*/
	if (item->type() == AGENTPP_GROUP) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("MibGroup: cannot add a MibGroup to a MibGroup");
		LOG_END;
		return 0;
	}

	/* check if the oid already exists */
    ListCursor<MibEntry> cur;
    for (cur.init(&content); cur.get(); cur.next()) {
        if (*cur.get()->key() == *item->key()) {
            LOG_BEGIN(ERROR_LOG | 1);
            LOG("MibGroup: cannot add MIB object (oid) to (group) twice");
            LOG(item->key()->get_printable());
            LOG(oid.get_printable());
            LOG_END;
            return 0;
        }
    }

	return content.add(item);
}

void MibGroup::remove(const Oidx& o)
{
	ListCursor<MibEntry> cur;
	for (cur.init(&content); cur.get(); cur.next()) {
		if (*cur.get()->key() == o) {
			delete content.remove(cur.get());
			break;
		}
	}
}


ListCursor<MibEntry> MibGroup::get_content()
{
	return ListCursor<MibEntry>(&content);
}

void MibGroup::clearAll()
{
	content.clearAll();
}

void MibGroup::load_from_file(const char* fname)
{
	FILE *f;
	char *buf;
	long size, bytes;
	char header[16];

	if ((f = fopen(fname, "rb")) == 0)
		return;

	size  = AgentTools::file_size(f);
	int n = 0;
	ListCursor<MibEntry> cur;
	for (cur.init(&content); ((cur.get()) && (size>0)); cur.next(), n++) {

		// skip volatile objects
		if (cur.get()->is_volatile()) continue;

		// read ASN.1 sequence header
		bytes = fread(header, sizeof(char), 2, f);
		if (bytes != 2) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("MibGroup: loading persistent data failed, cannot read length (file)(oid)(size)");
			LOG(fname);
			LOG(cur.get()->key()->get_printable());
			LOG(size);
			LOG_END;
			fclose(f);
			return;
		}
		bytes = 0;
		char hlen = header[1];
		int sz = size; // sz must be total amount of data readable
		if (hlen & ASN_LONG_LEN) {
			hlen &= ~ASN_LONG_LEN;	/* turn MSb off */
			bytes = fread(header+2, sizeof(char), hlen, f);
		}
		else hlen = 0;
		unsigned char type = 0;
		asn_parse_header((unsigned char*)header, &sz, &type);
		if ((bytes != hlen) ||
		    (type != (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR))){
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("MibGroup: loading from file failed (file)(oid)(n)(header)(type)");
			LOG(fname);
			LOG(cur.get()->key()->get_printable());
			LOG(n);
			LOG(OctetStr((unsigned char*)header, bytes).get_printable());
			LOG(type);
			LOG_END;
			fclose(f);
			return;
		}
		buf   = new char[sz+16];
		memcpy(buf, header, 2+hlen);
		char* cp = buf+2+hlen;
		bytes = fread(cp, sizeof(char), sz, f);
		if (bytes != sz) {
			LOG_BEGIN(ERROR_LOG | 1);
			LOG("MibGroup: loading from file failed (file)(oid)(n)(read)(size)");
			LOG(fname);
			LOG(cur.get()->key()->get_printable());
			LOG(n);
			LOG(bytes);
			LOG(sz);
			LOG_END;
			delete [] buf;
			fclose(f);
			return;
		}
		int len = sz+2+hlen;
		while ((cur.get()) && (!cur.get()->deserialize(buf, len))) {
			cur.next();
		}
		delete [] buf;
	}
	fclose(f);
}

void MibGroup::save_to_file(const char* fname)
{
	FILE *f;
	char *buf = 0;
	int bytes = 0;

	if ((f = fopen(fname, "wb")) == 0) {
		return;
	}
	ListCursor<MibEntry> cur;
	for (cur.init(&content); cur.get(); cur.next()) {
		// skip volatile objects
		if (cur.get()->is_volatile()) continue;

		if ((cur.get()->serialize(buf, bytes)) && (buf)) {
			fwrite(buf, sizeof(char), bytes, f);
			delete [] buf;
			buf = 0;
		}
	}
      	fclose(f);
}



/*--------------------------- class MibContext --------------------------*/


MibContext::MibContext()
{
	context = "";
	contextKey = Oidx::from_string(context);
	persistencyPath = 0;
}

MibContext::MibContext(const OctetStr& c)
{
	context = c;
	contextKey = Oidx::from_string(c);
	persistencyPath = 0;
}

MibContext::~MibContext()
TS_SYNCHRONIZED(
{
	if (persistencyPath) {
		save_to(*persistencyPath);
		delete persistencyPath;
	}
})

boolean MibContext::init_from(const OctetStr& p)
{
	if (persistencyPath)
		delete persistencyPath;
	persistencyPath = new OctetStr(p);
	OidListCursor<MibGroup> cur;
	for (cur.init(&groups); cur.get(); cur.next()) {
		if (cur.get()->is_persistent()) {
			OctetStr path(*persistencyPath);
			path += cur.get()->get_persistency_name();
			path += ".";
			path += context;
			cur.get()->load_from_file(path.get_printable());
		}
	}
	return TRUE;
}

boolean MibContext::load_from(const OctetStr& p)
{
	OidListCursor<MibGroup> cur;
	for (cur.init(&groups); cur.get(); cur.next()) {
		if (cur.get()->is_persistent()) {
			// clear old data
			ListCursor<MibEntry> entries =
			    cur.get()->get_content();
			for (;entries.get(); entries.next()) {
			    entries.get()->start_synch();
			    entries.get()->reset();
			}
			// load data from persistent storage
			OctetStr path(p);
			path += cur.get()->get_persistency_name();
			path += ".";
			path += context;
			cur.get()->load_from_file(path.get_printable());

			entries = cur.get()->get_content();
			for (;entries.get(); entries.next()) {
			    entries.get()->end_synch();
			}
		}
	}
	return TRUE;
}

boolean MibContext::save_to(const OctetStr& p)
{
	OidListCursor<MibGroup> cur;
	for (cur.init(&groups); cur.get(); cur.next()) {
		if (cur.get()->is_persistent()) {
			OctetStr path(p);
			path += cur.get()->get_persistency_name();
			path += ".";
			path += context;
			cur.get()->save_to_file(path.get_printable());
		}
	}
	return TRUE;
}

OidxPtr MibContext::key()
{
	return &contextKey;
}

int MibContext::find(const Oidx& oid, MibEntryPtr& entry)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	MibEntry* e = content.find(&tmpoid);
	if (!e) return sNMP_SYNTAX_NOSUCHOBJECT;
	entry = e;
	return SNMP_ERROR_SUCCESS;
})

MibEntry* MibContext::seek(const Oidx& oid)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	return content.seek(&tmpoid);
})


int MibContext::find_lower(const Oidx& oid, MibEntryPtr& entry)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	MibEntry* e = content.find_lower(&tmpoid);
	if (!e) return sNMP_SYNTAX_NOSUCHOBJECT;
	entry = e;
	return SNMP_ERROR_SUCCESS;
})

int MibContext::find_upper(const Oidx& oid, MibEntryPtr& entry)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	MibEntry* e = content.find_upper(&tmpoid);
	if (!e) return sNMP_SYNTAX_NOSUCHOBJECT;
	entry = e;
	return SNMP_ERROR_SUCCESS;
})

MibEntry* MibContext::find_next(const Oidx& oid)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	return content.find_next(&tmpoid);
})

OidListCursor<MibEntry>  MibContext::get_content()
{
	return OidListCursor<MibEntry>(&content);
}

OidListCursor<MibGroup>  MibContext::get_groups()
{
	return OidListCursor<MibGroup>(&groups);
}

MibEntry* MibContext::add(MibEntry* item)
{
    ThreadSynchronize _ts_synchronize(*this);
	if ((item->type() == AGENTPP_LEAF) &&
	    (item->get_access() == NOACCESS)) {
		LOG_BEGIN(ERROR_LOG | 1);
		LOG("MibContext: cannot add not-accessible MIB object (oid) to (context)");
		LOG(item->key()->get_printable());
		LOG(context.get_printable());
		LOG_END;
		return 0;
	}
	LOG_BEGIN(DEBUG_LOG | 3);
	LOG("MibContext: adding MIB object (context)(oid)");
	LOG(context.get_printable());
	LOG(item->key()->get_printable());
	LOG_END;

	MibEntryPtr e = content.find(item->key());
	if (e) {
		LOG_BEGIN(WARNING_LOG | 1);
		LOG("MibContext: duplicate registration (context)(oid)");
		LOG(context.get_printable());
		LOG(item->key()->get_printable());
		LOG_END;
		return 0;
	}

	if (item->type() == AGENTPP_GROUP) {
	    MibGroup *mg = (MibGroup*)item;
	    if (groups.find(mg->key()))
	    {
	        LOG_BEGIN(WARNING_LOG | 1);
	        LOG("MibContext: duplicate MibGroup registration (context)(oid)");
	        LOG(context.get_printable());
	        LOG(item->key()->get_printable());
	        LOG_END;
	        return 0;
	    }
		ListCursor<MibEntry> cur(mg->get_content());
		for (; cur.get(); cur.next())
			content.add(cur.get());
		groups.add(mg);
	}
	else {
		return content.add(item);
	}
	return item;
}

MibEntry* MibContext::remove(const Oidx& oid)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	MibEntry* victim = content.find(&tmpoid);
	if (victim) return content.remove(victim);
	return 0;
})


MibEntry* MibContext::get(const Oidx& oid)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	return content.find(&tmpoid);
})

MibGroup* MibContext::find_group(const Oidx& oid)
{
	Oidx tmpoid(oid);
	return groups.find(&tmpoid);
}

boolean MibContext::remove_group(const Oidx& oid)
TS_SYNCHRONIZED(
{
	Oidx tmpoid(oid);
	MibGroup* victim = groups.find(&tmpoid);
	if (victim) {
		ListCursor<MibEntry> cur(victim->get_content());
		for (; cur.get(); cur.next()) {
			MibEntry* v = content.find(cur.get()->key());
			if (v)
				delete content.remove(v);
		}
		delete groups.remove(victim);
		return TRUE;
	}
	return FALSE;
})

MibGroup* MibContext::find_group_of(const Oidx& oid)
{
	Oidx tmpoid(oid);
	OidListCursor<MibGroup> cur;
	for (cur.init(&groups); cur.get(); cur.next()) {
		if (oid.in_subtree_of(*cur.get()->key())) {
			ListCursor<MibEntry> c(cur.get()->get_content());
			for (; c.get(); c.next()) {
				if (*c.get()->key() == oid) return cur.get();
			}
		}
	}
	return 0;
}
#ifdef AGENTPP_NAMESPACE
}
#endif


