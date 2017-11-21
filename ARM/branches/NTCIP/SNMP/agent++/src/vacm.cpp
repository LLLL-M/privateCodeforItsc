/*_############################################################################
  _## 
  _##  vacm.cpp  
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
#include <agent_pp/agent++.h>
#include <agent_pp/tools.h>
#include <agent_pp/vacm.h>
#include <agent_pp/snmp_textual_conventions.h>
#include <snmp_pp/log.h>

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


const index_info       	iVacmSecurityToGroupTable[2]	=
{ { sNMP_SYNTAX_INT, FALSE, 1, 1 }, { sNMP_SYNTAX_OCTETS, FALSE, 1, 32 } };
const unsigned int	lVacmSecurityToGroupTable	= 2;

const index_info       	iVacmContextTable[1]		=
{ { sNMP_SYNTAX_OCTETS, FALSE, 0, 32 } };
const unsigned int	lVacmContextTable		= 1;

const index_info       	iVacmAccessTable[4]		=
{ { sNMP_SYNTAX_OCTETS, FALSE, 1, 32 }, { sNMP_SYNTAX_OCTETS, FALSE, 0, 32 },
  { sNMP_SYNTAX_INT, FALSE, 1, 1 }, { sNMP_SYNTAX_INT, FALSE, 1, 1 } };
const unsigned int	lVacmAccessTable		= 4;

const index_info       	iVacmViewTreeFamilyTable[2]	=
{ { sNMP_SYNTAX_OCTETS, FALSE, 1, 32 }, { sNMP_SYNTAX_OID, FALSE, 0, 95 } };
const unsigned int	lVacmViewTreeFamilyTable	= 2;


SnmpUnavailableContexts::SnmpUnavailableContexts():
  MibLeaf(oidSnmpUnavailableContexts, READONLY, new Counter32(0))
{}

void SnmpUnavailableContexts::incValue()
{
  *((SnmpInt32*)value) = (long)*((SnmpInt32*)value) + 1;
}

SnmpInt32 SnmpUnavailableContexts::getValue()
{
  return *((SnmpInt32*)value);
}

SnmpUnknownContexts::SnmpUnknownContexts():
  MibLeaf(oidSnmpUnknownContexts, READONLY, new Counter32(0))
{}

void SnmpUnknownContexts::incValue()
{
  *((SnmpInt32*)value) = (long)*((SnmpInt32*)value) + 1;
}

SnmpInt32 SnmpUnknownContexts::getValue()
{
  return *((SnmpInt32*)value);
}




/*********************************************************************

               VacmContextTable

 ********************************************************************/
VacmContextTable::VacmContextTable(): MibTable(oidVacmContextEntry,
					       iVacmContextTable,
					       lVacmContextTable)
{
  // vacmContextName
  add_col(new SnmpAdminString("1", READONLY, new OctetStr(""),
			      VMODE_DEFAULT, 0, 32));

  add_row("0"); // add default context ""

}

VacmContextTable::~VacmContextTable()
{

}

boolean VacmContextTable::addNewRow(const OctetStr& context)
{
  Oidx newIndex = Oidx::from_string(context, TRUE);

  if (find_index(newIndex))
    return FALSE;
  else {
    MibTableRow *mtr = add_row(newIndex);
    mtr->get_nth(0)->replace_value(new OctetStr(context));
    return TRUE;
  }
}

void VacmContextTable::deleteRow(const OctetStr& context)
{
  remove_row(Oidx::from_string(context, TRUE));
}

boolean VacmContextTable::isContextSupported(const OctetStr& context)
{
  OidListCursor<MibTableRow> cur;
  for (cur.init(&content); cur.get(); cur.next()) {
    Vbx v=cur.get()->get_nth(0)->get_value();
    OctetStr os;
    v.get_value(os);
    if ((os.len() == context.len()) && (os == context))
      return TRUE;
  }
  return FALSE;

}
/*********************************************************************

               VacmSecurityToGroupTable

 ********************************************************************/

VacmSecurityToGroupTable::VacmSecurityToGroupTable():
  StorageTable(oidVacmSecurityToGroupEntry, iVacmSecurityToGroupTable,
	       lVacmSecurityToGroupTable)
{
  Oidx tmpoid = Oidx(oidVacmSecurityToGroupEntry);

  // vacmSecurityModel
  add_col(new SnmpInt32MinMax("1", NOACCESS, 0, VMODE_NONE, 1, 3));
  // vacmSecurityName
  add_col(new SnmpAdminString("2", NOACCESS, new OctetStr(""),
			      VMODE_NONE, 1, 32));
  // vacmGroupName
  add_col(new SnmpAdminString("3", READCREATE, new OctetStr(""),
			      VMODE_NONE, 1, 32));
  // vacmSecurityToGroupStorageType
  add_storage_col(new StorageType("4", 3));
  // vacmSecurityToGroupStatus
  add_col(new snmpRowStatus("5"));

}

VacmSecurityToGroupTable::~VacmSecurityToGroupTable()
{
}

boolean VacmSecurityToGroupTable::ready_for_service(Vbx* pvbs, int sz)
{
  // check if GroupName is set

  if (!pvbs[2].valid())
    return FALSE;
  if (!(generator.get_nth(2)->value_ok(pvbs[2])))
    return FALSE;

  return TRUE;
}

void VacmSecurityToGroupTable::row_added(MibTableRow* new_row,
					 const Oidx& ind, MibTable*)
{
  Oidx o = Oidx(ind);
  MibLeaf* ml;
  ml = new_row->get_nth(0);
  ml->set_value(o[0]);

  ml = new_row->get_nth(1);
  o = o.cut_left(2);
  ml->set_value(o.as_string());
}

boolean VacmSecurityToGroupTable::could_ever_be_managed(const Oidx& o,
							int& result)

{
  if (!MibTable::could_ever_be_managed(o, result)) return FALSE;
  Oidx tmpoid(o);

  // check oid through value_ok() of the INDEX-objects
  Vbx v;
  v.set_value(SnmpInt32(o[oid.len()+1]));
  if (!(generator.get_nth(0)->value_ok(v)))
    return FALSE;
  v.set_value(o.cut_left(oid.len()+3).as_string());

  if (!(generator.get_nth(1)->value_ok(v)))
    return FALSE;

  return TRUE;
}

boolean VacmSecurityToGroupTable::getGroupName(const int& securityModel, const OctetStr& securityName, OctetStr& groupName)
{

  Oidx o=oid; // base
  OctetStr os = securityName;
  o += 3;       // col GroupName
  o += securityModel;
  o += os.len();
  for (unsigned int i=0; i<os.len(); i++)
    o+=os[i];
  //int n,m;
  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: getGroupName: (model) (name)");
  LOG(securityModel);
  LOG(OctetStr(securityName).get_printable());
  LOG(o.get_printable());
  LOG_END;

  MibLeaf* leaf;
  if ((leaf = find(o)) == 0)
    return FALSE;
  leaf->get_value().get_value(groupName);
  return TRUE;
}

boolean VacmSecurityToGroupTable::isGroupNameOK(const OctetStr& os)
{
  Vbx v("0");
  v.set_value(os);
  if (generator.get_nth(2)->value_ok(v))
    return TRUE;
  return FALSE;
}

boolean VacmSecurityToGroupTable::addNewRow(const int securityModel,
                                            const OctetStr& securityName,
                                            const OctetStr& groupName,
                                            const int storageType)
{
  Oidx newIndex;
  newIndex += securityModel;
  newIndex += Oidx::from_string(securityName, TRUE);

  if (find_index(newIndex))
  {
    LOG_BEGIN(WARNING_LOG | 5);
    LOG("Vacm: Security to group mapping already exists (security name)");
    LOG(securityName.get_printable());
    LOG_END;

    return FALSE;
  }
  else
  {
    MibTableRow *newRow = add_row(newIndex);

    newRow->get_nth(2)->replace_value(new OctetStr(groupName));
    newRow->get_nth(3)->replace_value(new SnmpInt32(storageType));
    newRow->get_nth(4)->replace_value(new SnmpInt32(1));

    return TRUE;
  }
}

void VacmSecurityToGroupTable::deleteRow(const int securityModel, const OctetStr& securityName)
{
  Oidx o;
  o += securityModel;
  o += Oidx::from_string(securityName, TRUE);
  remove_row(o);
}

/* ********************************************************************
 **********************************************************************

                           VacmAccessTable

 **********************************************************************
 ******************************************************************** */

VacmAccessTableStatus::VacmAccessTableStatus(const Oidx& o, int _base_len)
  : snmpRowStatus(o, READCREATE)
{
  base_len = _base_len;
}

MibEntryPtr VacmAccessTableStatus::clone()
{
  snmpRowStatus* other = new VacmAccessTableStatus(oid, base_len);
  other->set_reference_to_table(my_table);
  return other;
}

VacmAccessTable::VacmAccessTable(VacmSecurityToGroupTable* stogt):
  StorageTable(oidVacmAccessEntry, iVacmAccessTable, lVacmAccessTable)
{
  securityToGroupTable = stogt;

  Oidx tmpoid = Oidx(oidVacmSecurityToGroupEntry);

  // vacmAccessContextPrefix
  add_col(new SnmpAdminString("1", NOACCESS, new OctetStr(""),
			      VMODE_DEFAULT, 0, 32));
  // vacmAccessSecurityModel
  add_col(new SnmpInt32MinMax("2", NOACCESS, 0, VMODE_DEFAULT, 0, 3));
  // vacmAccessSecurityLevel
  add_col(new SnmpInt32MinMax("3", NOACCESS, 0, VMODE_DEFAULT, 0, 3));

  // vacmAccessContextMatch
  add_col(new SnmpInt32MinMax("4", READCREATE, 1, VMODE_DEFAULT, 1, 2));
  // vacmAccessReadViewName
  add_col(new SnmpAdminString("5", READCREATE, new OctetStr(""),
			      VMODE_DEFAULT, 0, 32));
  // vacmAccessWriteViewName
  add_col(new SnmpAdminString("6", READCREATE, new OctetStr(""),
			      VMODE_DEFAULT, 0, 32));
  // vacmAccessNotifyViewName
  add_col(new SnmpAdminString("7", READCREATE, new OctetStr(""),
			      VMODE_DEFAULT, 0, 32));
  // vacmAccessStorageType
  add_storage_col(new StorageType("8", 3));
  // vacmAccessStatus
  add_col(new VacmAccessTableStatus("9", tmpoid.len()));
}

VacmAccessTable::~VacmAccessTable()
{
}

boolean VacmAccessTable::ready_for_service(Vbx* pvbs, int sz)
{
  // A row is always ready for service
  return TRUE;
}

void VacmAccessTable::row_added(MibTableRow* new_row,
				const Oidx& ind, MibTable*)
{
  // GroupName (erster Index) muss nicht gesetzt werden.

  Oidx o = Oidx(ind);
  MibLeaf* ml;

  ml = new_row->get_nth(0);

  Oidx o2 = o;
  o2 = o2.cut_left(o[0]+2);
  o2 = o2.cut_right(2);
  ml->set_value(o2.as_string());

  ml = new_row->get_nth(1);
  ml->set_value(o[o.len()-2]);

  ml = new_row->get_nth(2);
  ml->set_value(o[o.len()-1]);
}

boolean VacmAccessTable::could_ever_be_managed(const Oidx& o, int& result)
{
  if (!MibTable::could_ever_be_managed(o, result)) return FALSE;
  Oidx tmpoid(o);

  // check oid through value_ok() of the INDEX-objects
  if (!(securityToGroupTable->isGroupNameOK(o.cut_right(3+o[oid.len()+2+o[oid.len()+1]]).cut_left(oid.len()+2).as_string())))
    return FALSE;

  Vbx v;
  v.set_value(o.cut_right(2).cut_left(oid.len()+3+o[oid.len()+1]).as_string());
  if (!(generator.get_nth(0)->value_ok(v)))
    return FALSE;

  v.set_value(SnmpInt32(o[o.len()-2]));
  if (!(generator.get_nth(1)->value_ok(v)))
    return FALSE;

  v.set_value(SnmpInt32(o[o.len()-1]));
  if (!(generator.get_nth(2)->value_ok(v)))
    return FALSE;

  return TRUE;
}


boolean VacmAccessTable::getViewName(const OctetStr& group,
                                     const OctetStr& context,
                                     const int securityModel,
                                     const int securityLevel,
                                     const int viewType,
                                     OctetStr& viewName)
{
  boolean found = FALSE; // TODO: use foundRow pointer!
  boolean foundMatchModel = FALSE;
  boolean foundMatchContextExact = FALSE;
  unsigned int foundContextPrefixLength = 0;
  unsigned int foundSecurityLevel = 0;
  MibTableRow* foundRow = NULL;

  unsigned int groupLen = group.len();
  Oidx ind;

  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: getViewName: (group) (context) (model) (level) (type)");
  LOG(OctetStr(group).get_printable());
  LOG(OctetStr(context).get_printable());
  LOG(securityModel);
  LOG(securityLevel);
  LOG(viewType);
  LOG_END;

  OidListCursor<MibTableRow> cur;
  cur.init(&content);
  Oidx groupIndex(Oidx::from_string(group));
  if (!cur.lookup(&groupIndex)) {
      cur.init(&content);
  }
  for (; cur.get(); cur.next()) {
    if (cur.get()->get_row_status()->get() != rowActive) {
      continue;
    }

    if (cur.get()->get_index()[0] == groupLen) {
      ind = cur.get()->get_index();

      if (ind.cut_right(ind[ind[0]+1]+3).cut_left(1).as_string() == group) {

	LOG_BEGIN(DEBUG_LOG | 7);
	LOG("Vacm: getViewName: (matched group)");
	LOG(OctetStr(ind.cut_right(ind[ind[0]+1]+3).cut_left(1).as_string()).get_printable());
	LOG_END;

        if ((((int)ind[ind.len() - 2] == securityModel) ||
#ifdef _SNMPv3
             ((int)ind[ind.len() - 2] == SNMP_SECURITY_MODEL_ANY)
#else
             (ind[ind.len() - 2] == 0)
#endif
             ) &&
            ((int)ind[ind.len() - 1] <= securityLevel)) {
          OctetStr pref=OctetStr(ind.cut_left(ind[0]+2).cut_right(2).as_string());
          int exactMatch;
          cur.get()->get_nth(3)->get_value(exactMatch);

	  LOG_BEGIN(DEBUG_LOG | 8);
	  LOG("Vacm: getViewName: (matching)(prefix)(context)");
	  LOG(exactMatch);
	  LOG(pref.get_printable());
	  LOG(OctetStr(context).get_printable());
	  LOG_END;

          if (((exactMatch == 1) && (pref == context)) ||
              ((exactMatch == 2) &&
	       ((pref.len() <= context.len()) &&
		(pref.nCompare(pref.len(), context) == 0)))) {

	    LOG_BEGIN(DEBUG_LOG | 7);
	    LOG("Vacm: getViewName: (matched context)");
	    LOG(pref.get_printable());
	    LOG_END;

            if (found) { // found a row before
              boolean replace = FALSE;
              if ((!foundMatchModel) && ((int)ind[ind.len() - 2] == securityModel))
                replace = TRUE;
              else
                if ((!foundMatchModel) || ((int)ind[ind.len() - 2] == securityModel)) {
                  if ((!foundMatchContextExact) && (exactMatch == 1))
                    replace = TRUE;
                  else
		    if ((!foundMatchContextExact) || (exactMatch == 1)) {
                      if (foundContextPrefixLength < pref.len())
                        replace = TRUE;
                      else
                        if (foundContextPrefixLength ==  pref.len())
                          if (foundSecurityLevel < ind[ind.len() - 1])
                            replace = TRUE;
		    }
		}

              if (replace) {
                foundMatchModel = ((int)ind[ind.len() - 2] == securityModel);
                foundMatchContextExact = (exactMatch == 1);
                foundContextPrefixLength = pref.len();
                foundSecurityLevel = ind[ind.len() - 1];
                foundRow = cur.get();
              }
            }
            else { // this is the first row that was found
              found = TRUE;
              foundMatchModel = ((int)ind[ind.len() - 2] == securityModel);
              foundMatchContextExact = (exactMatch == 1);
              foundContextPrefixLength = pref.len();
              foundSecurityLevel = ind[ind.len() - 1];
              foundRow = cur.get();
            }

          }
        }
      }
    }
  }
  if (found) {
    switch (viewType) {
      case mibView_read: { foundRow->get_nth(4)->get_value(viewName); break; }
      case mibView_write: { foundRow->get_nth(5)->get_value(viewName); break; }
      case mibView_notify:{ foundRow->get_nth(6)->get_value(viewName); break; }
    }
    return TRUE;
  }
  return FALSE;
}

boolean VacmAccessTable::addNewRow(const OctetStr& groupName,
                                   const OctetStr& prefix,
                                   const int securityModel, const int securityLevel,
                                   const int match, const OctetStr& readView,
                                   const OctetStr& writeView,
                                   const OctetStr& notifyView, const int storageType)
{
  Oidx newIndex;

  newIndex = Oidx::from_string(groupName, TRUE);
  newIndex += Oidx::from_string(prefix, TRUE);
  newIndex += securityModel;
  newIndex += securityLevel;

  if (find_index(newIndex))
    return FALSE;
  else {
    MibTableRow *newRow = add_row(newIndex);

    newRow->get_nth(3)->replace_value(new SnmpInt32(match));
    newRow->get_nth(4)->replace_value(new OctetStr(readView));
    newRow->get_nth(5)->replace_value(new OctetStr(writeView));
    newRow->get_nth(6)->replace_value(new OctetStr(notifyView));
    newRow->get_nth(7)->replace_value(new SnmpInt32(storageType));
    newRow->get_nth(8)->replace_value(new SnmpInt32(1));

    return TRUE;
  }
}
void VacmAccessTable::deleteRow(const OctetStr& groupName, const OctetStr& prefix,
                                const int securityModel, const int securityLevel)
{
  Oidx o;

  o =  Oidx::from_string(groupName, TRUE);
  o += Oidx::from_string(prefix, TRUE);
  o += securityModel;
  o += securityLevel;

  remove_row(o);
}


/*********************************************************************

               VacmViewTreeFamilyTable

 ********************************************************************/

VacmViewTreeFamilyTableStatus::VacmViewTreeFamilyTableStatus(const Oidx& o,
				int _base_len):snmpRowStatus(o, READCREATE)
{
  base_len = _base_len;
}

int VacmViewTreeFamilyTableStatus::set(const Vbx& vb)
{
	undo = value->clone();
	long rs;
	if (vb.get_value(rs) != SNMP_CLASS_SUCCESS)
	    return SNMP_ERROR_WRONG_TYPE;

	switch (rs) {
	case rowNotInService: {
	  OctetStr viewName = ((SnmpAdminString*)my_row->first())->get();
	  ViewNameIndex* views =
	    ((VacmViewTreeFamilyTable*)my_table)->viewsOf(viewName);
	  if (!views) {
	    LOG_BEGIN(WARNING_LOG | 1);
	    LOG("VacmViewTreeFamilyTableStatus: internal error: view name not found (viewName)");
	    LOG(viewName.get_printable());
	    LOG_END;
	  }
	  else {
	    views->remove(my_row);

	    LOG_BEGIN(INFO_LOG | 2);
	    LOG("VacmViewTreeFamilyTable: (sub)view disabled (viewName)");
	    LOG(viewName.get_printable());
	    LOG_END;
	  }
	  break;
	}
	case rowActive: {
	  OctetStr viewName = ((SnmpAdminString*)my_row->first())->get();
	  ViewNameIndex* views =
	    ((VacmViewTreeFamilyTable*)my_table)->viewsOf(viewName);
	  if (!views) {
	    views = new ViewNameIndex(viewName);
	    views->add(my_row);
	    ((VacmViewTreeFamilyTable*)my_table)->viewNameIndex.add(views);

	    LOG_BEGIN(INFO_LOG | 2);
	    LOG("VacmViewTreeFamilyTable: adding view name (viewName)");
	    LOG(viewName.get_printable());
	    LOG_END;
	  }
	  else {
	    views->add(my_row);

	    LOG_BEGIN(INFO_LOG | 2);
	    LOG("VacmViewTreeFamilyTable: updating view (viewName)");
	    LOG(viewName.get_printable());
	    LOG_END;
	  }
	  break;
	}
	}
	return snmpRowStatus::set(vb);
}

MibEntryPtr VacmViewTreeFamilyTableStatus::clone()
{
  snmpRowStatus* other = new VacmViewTreeFamilyTableStatus(oid, base_len);
  other->set_reference_to_table(my_table);
  return other;
}



VacmViewTreeFamilyTable::VacmViewTreeFamilyTable()
  : StorageTable(oidVacmViewTreeFamilyEntry, iVacmViewTreeFamilyTable,
		 lVacmViewTreeFamilyTable)
{
  Oidx tmpoid = Oidx(oidVacmViewTreeFamilyEntry);

  // VacmViewTreeFamilyViewName
  add_col(new SnmpAdminString("1", NOACCESS, new OctetStr(""),
			      VMODE_DEFAULT, 1, 32));
  // VacmViewTreeFamilySubtree
  add_col(new MibLeaf("2", NOACCESS, new Oid("0.0"),VMODE_DEFAULT));
  // VacmViewTreeFamilyMask
  add_col(new SnmpAdminString("3", READCREATE, new OctetStr(""),
			      VMODE_DEFAULT, 0, 16));
  // VacmViewTreeFamilyType
  add_col(new SnmpInt32MinMax("4", READCREATE, 1, VMODE_DEFAULT, 1, 2));
  // VacmViewTreeFamilyStorageType
  add_storage_col(new StorageType("5", 3));
  // VacmViewTreeFamilyStatus
  add_col(new VacmViewTreeFamilyTableStatus("6", tmpoid.len()));

  buildViewNameIndex();
}

VacmViewTreeFamilyTable::~VacmViewTreeFamilyTable()
{
}

boolean VacmViewTreeFamilyTable::ready_for_service(Vbx* pvbs, int sz)
{
  // Defaultwerte sind ok!
  return TRUE;
}

void VacmViewTreeFamilyTable::row_added(MibTableRow* new_row,
					const Oidx& ind, MibTable*)
{
  Oidx o = Oidx(ind);
  MibLeaf* ml;
  ml = new_row->get_nth(0);
  ml->set_value(o.cut_right(o[o[0]+1]+1).cut_left(1).as_string());

  ml = new_row->get_nth(1);
  ml->set_value(o.cut_left(o[0]+2));
}

void VacmViewTreeFamilyTable::row_activated(MibTableRow* row,
					    const Oidx& ind, MibTable*)
{
    // add row to the index
    OctetStr viewName = ((SnmpAdminString*)row->first())->get();

    ViewNameIndex* views = viewsOf(viewName);
    if (views) views->add(row);
    else viewNameIndex.add(new ViewNameIndex(viewName))->add(row);
}

void VacmViewTreeFamilyTable::row_deactivated(MibTableRow* row,
					      const Oidx& ind, MibTable*)
{
    ViewNameIndex* views = viewsOf(((SnmpAdminString*)row->first())->get());
    if (views) {
	views->remove(row);
	if (views->isEmpty()) delete viewNameIndex.remove(views);
    }
}

void VacmViewTreeFamilyTable::row_delete(MibTableRow* row,
					 const Oidx& ind, MibTable* t)
{
	row_deactivated(row, ind, t);
}

boolean VacmViewTreeFamilyTable::could_ever_be_managed(const Oidx& o,
						       int& result)

{
  if (!MibTable::could_ever_be_managed(o, result)) return FALSE;
  Oidx tmpoid(o);

  // check oid through value_ok() of the INDEX-objects
  Vbx v;
  v.set_value(o.cut_right(1+o[oid.len()+2+
			  o[oid.len()+1]]).cut_left(oid.len()+2).as_string());
  if (!(generator.get_nth(0)->value_ok(v)))
    return FALSE;

  v.set_value(o.cut_left(oid.len()+3+o[oid.len()+1]));
  if (!(generator.get_nth(1)->value_ok(v)))
    return FALSE;

  return TRUE;
}

int VacmViewTreeFamilyTable::isInMibView(const OctetStr& viewName, const Oidx& subtree)
{
  boolean found = FALSE;
  unsigned int foundSubtreeLen = 0;
  MibTableRow* foundRow = NULL;
  Oidx ind;

  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: isInMibView: (viewName) (subtree)");
  LOG(OctetStr(viewName).get_printable());
  LOG(Oid(subtree).get_printable());
  LOG_END;

  ViewNameIndex* views = viewsOf(viewName);

  if (!views) return VACM_noSuchView;


  ListCursor<MibTableRow> cur;
  for (cur.init(&views->views); cur.get(); cur.next()) {

    ind = cur.get()->get_index();
    ind = ind.cut_left(ind[0]+1);

    if (ind[0] > subtree.len())
      continue;
    ind = ind.cut_left(1);
    OctetStr mask;
    cur.get()->get_nth(2)->get_value(mask);
    boolean ok = TRUE;
    for (unsigned int i=0; i<ind.len(); i++) {
      if ((ind[i] != subtree[i]) && (bit(i, mask))) {
	ok = FALSE;
	break;
      }
    }
    if (ok) {
      if (found) { // already found one
	if (foundSubtreeLen <= ind.len()) {
            foundRow = cur.get();
            foundSubtreeLen = ind.len();
	}
      }
      else { // first row found
	found = TRUE;
	foundSubtreeLen = ind.len();
	foundRow = cur.get();
      }
    }
  }
  if (found) {
    int tmpval;
    foundRow->get_nth(3)->get_value(tmpval);
    if (tmpval==1) { //included

      LOG_BEGIN(DEBUG_LOG | 9);
      LOG("Vacm: isInMibView: access allowed");
      LOG_END;
      return VACM_accessAllowed;
    }
    else //excluded
      return VACM_notInView;
  }
  return VACM_notInView;
}

boolean VacmViewTreeFamilyTable::bit(unsigned int nr, OctetStr& o)
{
  // return TRUE if bit is "1" or o is too short
  if (o.len() <= (nr/8))
    return TRUE;
  return (o[nr/8] & (0x01 << (7 - (nr % 8))));
}

/**
  * Build the viewNameIndex from the current contents of the table
  */
void VacmViewTreeFamilyTable::buildViewNameIndex()
{
  OidListCursor<MibTableRow> cur;
  viewNameIndex.clear();
  OctetStr viewName;
  for (cur.init(&content); cur.get(); cur.next()) {

    viewName = ((SnmpAdminString*)cur.get()->first())->get();
    ViewNameIndex* views = viewsOf(viewName);
    if (views) views->add(cur.get());
    else viewNameIndex.add(new ViewNameIndex(viewName))->add(cur.get());
  }
}


boolean VacmViewTreeFamilyTable::addNewRow(const OctetStr& viewName, const Oidx& subtree,
                                           const OctetStr& mask, const int type,
                                           const int storageType)
{
  Oidx newIndex;

  newIndex = Oidx::from_string(viewName, TRUE);
  newIndex += subtree.len();
  newIndex += subtree;

  if (find_index(newIndex))
    return FALSE;
  else {
    MibTableRow *newRow = add_row(newIndex);

    newRow->get_nth(2)->replace_value(new OctetStr(mask));
    newRow->get_nth(3)->replace_value(new SnmpInt32(type));
    newRow->get_nth(4)->replace_value(new SnmpInt32(storageType));
    newRow->get_nth(5)->replace_value(new SnmpInt32(1));

    row_activated(newRow, newIndex, 0);
    return TRUE;
  }
}

void VacmViewTreeFamilyTable::row_init(MibTableRow* newRow, const Oidx& ind,
				       MibTable* src)
{
	if (!src) {
		Oidx o(ind);
		o.trim(o.len()-(ind[0]+1)); // cut off subtree
		o = o.cut_left(1); // cut off length
		OctetStr viewName(o.as_string());

		ViewNameIndex* views = viewsOf(viewName);
		if (views) views->add(newRow);
		else viewNameIndex.add(new ViewNameIndex(viewName))->
		       add(newRow);
	}
}

void VacmViewTreeFamilyTable::deleteRow(const OctetStr& viewName, const Oidx& subtree)
{
  Oidx o;

  o = Oidx::from_string(viewName, TRUE);
  o += subtree.len();
  o += subtree;

  remove_row(o);
}


ViewNameIndex* VacmViewTreeFamilyTable::viewsOf(const OctetStr& viewName)
{
  OctetStr vName(viewName);
  ListCursor<ViewNameIndex> cur;
  for (cur.init(&viewNameIndex); cur.get(); cur.next()) {

    LOG_BEGIN(DEBUG_LOG | 8);
    LOG("VacmViewTreeFamilyTable: isInMibView: (viewName) (match)");
    LOG(vName.get_printable());
    LOG(cur.get()->name.get_printable());
    LOG_END;

    if (cur.get()->name == vName) return cur.get();
  }
  return 0;
}

/*********************************************************************

               VacmMIB

 ********************************************************************/
VacmMIB::VacmMIB(Vacm::ClassPointers vcp): MibGroup(oidVacmMIBObjects,
						  "snmpVacmMIB")
{
  add(vcp.contextTable);
  add(vcp.securityToGroupTable);
  add(vcp.accessTable);
  add(vcp.viewTreeFamilyTable);
  add(new TestAndIncr("1.3.6.1.6.3.16.1.5.1.0"));
}

Vacm::Vacm()
{
  vcp.contextTable = 0;
  vcp.securityToGroupTable = 0;
  vcp.accessTable = 0;
  vcp.viewTreeFamilyTable = 0;
  vcp.snmpUnknownContexts = 0;
  vcp.snmpUnavailableContexts = 0;
}

Vacm::Vacm(Mib& mib)
{
  vcp.contextTable = new VacmContextTable();
  vcp.securityToGroupTable =new VacmSecurityToGroupTable();
  vcp.accessTable = new VacmAccessTable(vcp.securityToGroupTable);
  vcp.viewTreeFamilyTable = new VacmViewTreeFamilyTable();
  vcp.snmpUnknownContexts = new SnmpUnknownContexts();
  vcp.snmpUnavailableContexts = new SnmpUnavailableContexts();
  mib.add(new VacmMIB(vcp));
  mib.add(vcp.snmpUnknownContexts);
  mib.add(vcp.snmpUnavailableContexts);
}

Vacm::~Vacm(void)
{

}

boolean Vacm::addNewContext(const OctetStr &newContext)
{
  return vcp.contextTable->addNewRow(newContext);
}

void Vacm::deleteContext(const OctetStr &context)
{
  vcp.contextTable->deleteRow(context);
}

boolean Vacm::addNewGroup(const int securityModel, const OctetStr& securityName,
                          const OctetStr& groupName, const int storageType)
{
  return vcp.securityToGroupTable->addNewRow(securityModel, securityName, groupName, storageType);
}

void Vacm::deleteGroup(const int securityModel, const OctetStr& securityName)
{
  vcp.securityToGroupTable->deleteRow(securityModel, securityName);
}

boolean Vacm::addNewAccessEntry(const OctetStr& groupName, const OctetStr& prefix,
                                const int securityModel, const int securityLevel,
                                const int match, const OctetStr& readView, const OctetStr& writeView,
                                const OctetStr& notifyView, const int storageType)
{
  return vcp.accessTable->addNewRow(groupName, prefix, securityModel, securityLevel,
                                      match, readView, writeView, notifyView, storageType);
}

void Vacm::deleteAccessEntry(const OctetStr& groupName, const OctetStr& prefix,
                                 const int securityModel, const int securityLevel)
{
  vcp.accessTable->deleteRow(groupName, prefix, securityModel, securityLevel);
}

boolean Vacm::addNewView(const OctetStr& viewName, const Oidx& subtree,
                         const OctetStr& mask, const int type, const int storageType)
{
  return vcp.viewTreeFamilyTable->addNewRow(viewName, subtree, mask, type, storageType);
}

void Vacm::deleteView(const OctetStr& viewName, const Oidx& subtree)
{
  vcp.viewTreeFamilyTable->deleteRow(viewName, subtree);
}

void Vacm::incUnknownContexts()
{
  vcp.snmpUnknownContexts->incValue();
}

SnmpInt32 Vacm::getUnknownContexts()
{
  return vcp.snmpUnknownContexts->getValue();
}

int Vacm::isAccessAllowed(const int securityModel, const OctetStr &securityName,
                              const int securityLevel, const int viewType,
                              const OctetStr &context, const Oidx &o)
{
  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: Access requested for: (model) (name) (level) (type) (context) (oid)");
  LOG(securityModel);
  LOG(securityName.get_printable());
  LOG(securityLevel);
  LOG(viewType);
  LOG(context.get_printable());
  LOG(o.get_printable());
  LOG_END;

  if (!(vcp.contextTable->isContextSupported(context)))
    return VACM_noSuchContext;

  OctetStr groupName;
  if (!(vcp.securityToGroupTable->getGroupName(securityModel, securityName, groupName)))
    return VACM_noGroupName;

  OctetStr viewName;
  if (!(vcp.accessTable->getViewName(groupName, context,
                                     securityModel, securityLevel, viewType, viewName)))
    return VACM_noAccessEntry;

  if (viewName.len() == 0)
    return VACM_noSuchView;

  return (vcp.viewTreeFamilyTable->isInMibView(viewName, o));
}

int Vacm::getViewName(const int securityModel, const OctetStr &securityName,
                         const int securityLevel,
                         const int viewType, const OctetStr &context, OctetStr &viewName)
{
  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: getViewName for: (model) (name) (level) (type) (context)");
  LOG(securityModel);
  LOG(securityName.get_printable());
  LOG(securityLevel);
  LOG(viewType);
  LOG(context.get_printable());
  LOG_END;

  if (!(vcp.contextTable->isContextSupported(context)))
    return VACM_noSuchContext;

  OctetStr groupName;
  if (!(vcp.securityToGroupTable->getGroupName(securityModel, securityName, groupName)))
    return VACM_noGroupName;

  if (!(vcp.accessTable->getViewName(groupName, context,
                                     securityModel, securityLevel, viewType, viewName)))
    return VACM_noAccessEntry;

  if (viewName.len() == 0)
    return VACM_noSuchView;

  return VACM_viewFound;
}


int Vacm::isAccessAllowed(const OctetStr &viewName, const Oidx &o)
{
  LOG_BEGIN(DEBUG_LOG | 7);
  LOG("Vacm: Access requested for: (viewName) (oid)");
  LOG(viewName.get_printable());
  LOG(o.get_printable());
  LOG_END;

  return (vcp.viewTreeFamilyTable->isInMibView(viewName, o));
}

#ifdef AGENTPP_NAMESPACE
}
#endif
