/*_############################################################################
  _## 
  _##  vacm.h  
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
#ifndef vacm_h_
#define vacm_h_

#include <agent_pp/mib.h>
#include <agent_pp/snmp_textual_conventions.h>
#include <agent_pp/snmp_pp_ext.h>

#ifdef AGENTPP_NAMESPACE
namespace Agentpp
{
#endif

#define oidSnmpModules                    "1.3.6.1.6.3"
#define oidSnmpVacmMIB                    "1.3.6.1.6.3.16"
#define oidVacmMIBObjects                 "1.3.6.1.6.3.16.1"
#define oidVacmMIBConformance             "1.3.6.1.6.3.16.2"

#define oidVacmContextTable               "1.3.6.1.6.3.16.1.1"
#define oidVacmContextEntry               "1.3.6.1.6.3.16.1.1.1"
#define oidVacmContextName                "1.3.6.1.6.3.16.1.1.1.1"

#define oidVacmSecurityToGroupTable       "1.3.6.1.6.3.16.1.2"
#define oidVacmSecurityToGroupEntry       "1.3.6.1.6.3.16.1.2.1"
#define oidVacmSecurityModel              "1.3.6.1.6.3.16.1.2.1.1" // 1.index
#define oidVacmSecurityName               "1.3.6.1.6.3.16.1.2.1.2" // 2.index
#define oidVacmGroupName                  "1.3.6.1.6.3.16.1.2.1.3"
#define oidVacmSecurityToGroupStorageType "1.3.6.1.6.3.16.1.2.1.4"
#define oidVacmSecurityToGroupStatus      "1.3.6.1.6.3.16.1.2.1.5"

#define oidVacmAccessTable                "1.3.6.1.6.3.16.1.4"
#define oidVacmAccessEntry                "1.3.6.1.6.3.16.1.4.1"   // groupName 1. index
#define oidVacmAccessContextPrefix        "1.3.6.1.6.3.16.1.4.1.1" // 2. index
#define oidVacmAccessSecurityModel        "1.3.6.1.6.3.16.1.4.1.2" // 3. index
#define oidVacmAccessSecurityLevel        "1.3.6.1.6.3.16.1.4.1.3" // 4. index
#define oidVacmAccessContextMatch         "1.3.6.1.6.3.16.1.4.1.4"
#define oidVacmAccessReadViewName         "1.3.6.1.6.3.16.1.4.1.5"
#define oidVacmAccessWriteViewName        "1.3.6.1.6.3.16.1.4.1.6"
#define oidVacmAccessNotifyViewName       "1.3.6.1.6.3.16.1.4.1.7"
#define oidVacmAccessStorageType          "1.3.6.1.6.3.16.1.4.1.8"
#define oidVacmAccessStatus               "1.3.6.1.6.3.16.1.4.1.9"

#define oidVacmMIBViews                   "1.3.6.1.6.3.16.1.5"
#define oidVacmViewSpinLock               "1.3.6.1.6.3.16.1.5.1"
#define oidVacmViewTreeFamilyTable        "1.3.6.1.6.3.16.1.5.2"
#define oidVacmViewTreeFamilyEntry        "1.3.6.1.6.3.16.1.5.2.1"
#define oidVacmViewTreeFamilyViewName     "1.3.6.1.6.3.16.1.5.2.1.1" // 1. index
#define oidVacmViewTreeFamilySubtree      "1.3.6.1.6.3.16.1.5.2.1.2" // 2. index
#define oidVacmViewTreeFamilyMask         "1.3.6.1.6.3.16.1.5.2.1.3"
#define oidVacmViewTreeFamilyType         "1.3.6.1.6.3.16.1.5.2.1.4"
#define oidVacmViewTreeFamilyStorageType  "1.3.6.1.6.3.16.1.5.2.1.5"
#define oidVacmViewTreeFamilyStatus       "1.3.6.1.6.3.16.1.5.2.1.6"

#define oidVacmMIBCompliances             "1.3.6.1.6.3.16.2.1"
#define oidVacmMIBGroups                  "1.3.6.1.6.3.16.2.2"

// also defined in snmp++/include/v3.h
#ifndef oidSnmpUnavailableContexts
#define oidSnmpUnavailableContexts        "1.3.6.1.6.3.12.1.4.0"
#define oidSnmpUnknownContexts            "1.3.6.1.6.3.12.1.5.0"
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef boolean
#define boolean int
#endif

#define VACM_accessAllowed          0
#define VACM_notInView              1
#define VACM_noSuchView             2
#define VACM_noSuchContext          3
#define VACM_noGroupName            4
#define VACM_noAccessEntry          5
#define VACM_otherError             6

#define VACM_viewFound              7

static const char * vacmErrs[] = {
  "accessAllowed",  // 0
  "notInView",      // 1
  "noSuchView ",    // 2
  "noSuchContext",  // 3
  "noGroupName",    // 4
  "noAccessEntry",  // 5
  "otherError",     // 6
  "viewFound"       // 7 (just for safety)
};

#define mibView_read     1
#define mibView_write    2
#define mibView_notify   3

#define storageType_other              1
#define storageType_volatile           2
#define storageType_nonVolatile        3
#define storageType_permanent          4
#define storageType_readOnly           5

#define match_exact  1
#define match_prefix 2

#define view_included   1
#define view_excluded   2


class SnmpUnknownContexts;
class SnmpUnavailableContexts;
class VacmContextTable;
class VacmSecurityToGroupTable;
class VacmAccessTable;
class VacmViewTreeFamilyTable;
class Mib;
class Oidx;


class AGENTPP_DECL Vacm
{
public:
  typedef struct
  {
      VacmContextTable* contextTable;
      VacmSecurityToGroupTable* securityToGroupTable;
      VacmAccessTable* accessTable;
      VacmViewTreeFamilyTable* viewTreeFamilyTable;
      SnmpUnknownContexts* snmpUnknownContexts;
      SnmpUnavailableContexts* snmpUnavailableContexts;
  } ClassPointers;

  Vacm();
  Vacm(Mib&);
  virtual ~Vacm();

  /**
   * Does the user have the rights to access the Oid.
   *
   * All parameters are used to check if access is allowed.
   *
   * @param[in] securityModel The security model used
   * @param[in] securityName  The security name of the user
   * @param[in] securityLevel The security level
   * @param[in] viewType Read, write or notify view
   * @param[in] context The MIB context
   * @param[in] o The Oid the user wants to access
   *
   * @return true if access is allowed
   */
  virtual int isAccessAllowed(const int securityModel,
                              const NS_SNMP OctetStr &securityName,
                              const int securityLevel, const int viewType,
                              const NS_SNMP OctetStr &context, const Oidx &o);

  /**
   * Is the oid within the view.
   *
   * Check if the given Oid is within the named view.
   *
   * @param[in] viewName Name of the view
   * @param[in] o Oid to check
   *
   * @return true if the oid is within the view
   */
  virtual int isAccessAllowed(const NS_SNMP OctetStr &viewName, const Oidx &o);

  /**
   * Get the view name for the given params
   *
   * @param[in] securityModel The security model used
   * @param[in] securityName  The security name of the user
   * @param[in] securityLevel The security level
   * @param[in] viewType Select one of mibView_read, write or notify
   * @param[in] context The MIB context
   * @param[out] viewName If found, the name of the matching view
   *
   * @return VACM_viewFound on success, error codes on failure
   */
  virtual int getViewName(const int securityModel, const NS_SNMP OctetStr &securityName,
                          const int securityLevel, const int viewType,
                          const NS_SNMP OctetStr &context, NS_SNMP OctetStr &viewName);

  void      incUnknownContexts();
  NS_SNMP SnmpInt32     getUnknownContexts();
  static const char*    getErrorMsg(int nr) { return vacmErrs[nr];};

  /**
   * Add a new context to the VacmContextTable.
   *
   * @param[in] newContext vacmContextName
   *
   * @return true, if the new context could be added. If the context already
   *         exists, false is returned.
   */
  boolean addNewContext(const NS_SNMP OctetStr &newContext);

  /**
   * Delete the given context from the VacmContextTable.
   *
   * @param[in] context vacmContextName
   */
  void deleteContext(const NS_SNMP OctetStr &context);

  /**
   * Add a new security name to group mapping to the VacmSecurityToGroupTable.
   *
   * @param[in] securityModel   vacmSecurityModel
   * @param[in] securityName    vacmSecurityName
   * @param[in] groupName       vacmGroupName
   * @param[in] storageType     vacmSecurityToGroupStorageType
   */
  boolean addNewGroup(const int securityModel,
                      const NS_SNMP OctetStr& securityName,
                      const NS_SNMP OctetStr& groupName,
                      const int storageType);

  /**
   * Delete the given securityName from the VacmSecurityToGroupTable.
   */
  void deleteGroup(const int securityModel, const NS_SNMP OctetStr& securityName);

  /**
   * Add a new entry to the VacmAccessTable.
   *
   * @param[in] groupName     vacmGroupName
   * @param[in] prefix        vacmAccessContextPrefix
   * @param[in] securityModel vacmAccessSecurityModel
   * @param[in] securityLevel vacmAccessSecurityLevel
   * @param[in] match         vacmAccessContextMatch
   * @param[in] readView      vacmAccessReadViewName
   * @param[in] writeView     vacmAccessWriteViewName
   * @param[in] notifyView    vacmAccessNotifyViewName
   * @param[in] storageType   vacmAccessStorageType (storageType_volatile, ...)
   */
  boolean addNewAccessEntry(const NS_SNMP OctetStr& groupName,
                            const NS_SNMP OctetStr& prefix,
                            const int securityModel, const int securityLevel,
                            const int match,
                            const NS_SNMP OctetStr& readView,
                            const NS_SNMP OctetStr& writeView,
                            const NS_SNMP OctetStr& notifyView,
                            const int storageType);

  /**
   * Delete a entry from the VacmAccessTable.
   *
   * @param[in] groupName     vacmGroupName
   * @param[in] prefix        vacmAccessContextPrefix
   * @param[in] securityModel vacmAccessSecurityModel
   * @param[in] securityLevel vacmAccessSecurityLevel
   *
   */
  void deleteAccessEntry(const NS_SNMP OctetStr& groupName,
                         const NS_SNMP OctetStr& prefix,
                         const int securityModel, const int securityLevel);

  /**
   * Add a new view to the VacmViewTreeFamilyTable.
   *
   * @param[in] viewName    vacmViewTreeFamilyViewName
   * @param[in] subtree     vacmViewTreeFamilySubtree
   * @param[in] mask        vacmViewTreeFamilyMask
   * @param[in] type        vacmViewTreeFamilyType (view_included or view_excluded)
   * @param[in] storageType vacmViewTreeFamilyStorageType (storageType_volatile, ...)
   *
   * @return true if the new row could be added. False, if it already exists.
   */
  boolean addNewView(const NS_SNMP OctetStr& viewName,
                     const Oidx& subtree,
                     const NS_SNMP OctetStr& mask,
                     const int type, const int storageType);

  /**
   * Delete the subtree from the view from the VacmViewTreeFamilyTable.
   *
   * @param[in] viewName    vacmViewTreeFamilyViewName
   * @param[in] subtree     vacmViewTreeFamilySubtree
   */
  void deleteView(const NS_SNMP OctetStr& viewName, const Oidx& subtree);

  /**
   * Get the pointers to the VACM SNMP tables used by the VACM.
   *
   * @return
   *    a Vacm::ClassPointers instance containing the pointers of the
   *    SNMP tables used by the VACM.
   */
  ClassPointers get_vacm_tables() { return vcp; }

 protected:
  ClassPointers vcp;
};


class AGENTPP_DECL SnmpUnavailableContexts: public MibLeaf
{
public:
    SnmpUnavailableContexts();
    void incValue();
    NS_SNMP SnmpInt32 getValue();
};

class AGENTPP_DECL SnmpUnknownContexts: public MibLeaf
{
public:
    SnmpUnknownContexts();
    void incValue();
    NS_SNMP SnmpInt32 getValue();
};

/*********************************************************************

 VacmContextTable

 ********************************************************************/
class AGENTPP_DECL VacmContextTable: public MibTable
{

public:
    VacmContextTable();
    ~VacmContextTable();

    boolean isContextSupported(const NS_SNMP OctetStr& context);
    boolean addNewRow(const NS_SNMP OctetStr& context);
    void deleteRow(const NS_SNMP OctetStr& context);
};

/*********************************************************************

 VacmSecurityToGroupTable

 ********************************************************************/
class AGENTPP_DECL VacmSecurityToGroupTable: public StorageTable
{

public:
    VacmSecurityToGroupTable();
    virtual ~VacmSecurityToGroupTable();

    virtual boolean ready_for_service(Vbx*, int);
    virtual boolean could_ever_be_managed(const Oidx&, int&);
    virtual void row_added(MibTableRow*, const Oidx&, MibTable*);
    boolean getGroupName(const int& securiyModel,
                         const NS_SNMP OctetStr& securityName,
                         NS_SNMP OctetStr& groupName);
    boolean isGroupNameOK(const NS_SNMP OctetStr&);
    boolean addNewRow(const int securityModel,
                      const NS_SNMP OctetStr& securityName,
                      const NS_SNMP OctetStr& groupName, const int storageType);
    void deleteRow(const int securityModel,
                     const NS_SNMP OctetStr& securityName);
};

/*********************************************************************

 VacmAccessTable

 ********************************************************************/
class AGENTPP_DECL VacmAccessTable: public StorageTable
{

public:
    VacmAccessTable(VacmSecurityToGroupTable*);
    virtual ~VacmAccessTable();

    virtual boolean ready_for_service(Vbx*, int);
    virtual boolean could_ever_be_managed(const Oidx&, int&);
    virtual void row_added(MibTableRow*, const Oidx&, MibTable*);
    boolean getViewName(const NS_SNMP OctetStr& group,
                        const NS_SNMP OctetStr& context,
                        const int securityModel,
                        const int securityLevel,
                        const int viewType,
                        NS_SNMP OctetStr& viewName);

    boolean addNewRow(const NS_SNMP OctetStr& groupName,
                      const NS_SNMP OctetStr& prefix,
                      const int securityModel, const int securityLevel,
                      const int match, const NS_SNMP OctetStr& readView,
                      const NS_SNMP OctetStr& writeView,
                      const NS_SNMP OctetStr& notifyView, const int storageType);
    void deleteRow(const NS_SNMP OctetStr& groupName, const NS_SNMP OctetStr& prefix,
                   const int securityModel, const int securityLevel);
protected:
    VacmSecurityToGroupTable* securityToGroupTable;

};

class AGENTPP_DECL VacmAccessTableStatus: public snmpRowStatus
{
public:
    VacmAccessTableStatus(const Oidx&, int);
    virtual ~VacmAccessTableStatus() {};

    virtual MibEntryPtr clone();

protected:
    int base_len;

};

class AGENTPP_DECL ViewNameIndex
{

public:
    ViewNameIndex(const NS_SNMP OctetStr& vname): name(vname) {}
    ~ViewNameIndex() { /* avoid deletion of original rows: */views.clear();}

    void add(MibTableRow* row)    {   views.add(row);}
    void remove(MibTableRow* row)    {   views.remove(row);}
    boolean isEmpty()    {   return views.empty();}

    NS_SNMP OctetStr name;
    List<MibTableRow> views;
};

/*********************************************************************

 VacmViewTreeFamilyTable

 ********************************************************************/
#if !defined (AGENTPP_DECL_TEMPL_LIST_VIEWNAMEINDEX)
#define AGENTPP_DECL_TEMPL_LIST_VIEWNAMEINDEX
AGENTPP_DECL_TEMPL template class AGENTPP_DECL List<ViewNameIndex>;
#endif

class AGENTPP_DECL VacmViewTreeFamilyTable: public StorageTable
{
    friend class VacmViewTreeFamilyTableStatus;
public:
    VacmViewTreeFamilyTable();
    virtual ~VacmViewTreeFamilyTable();

    virtual boolean ready_for_service(Vbx*, int);
    virtual boolean could_ever_be_managed(const Oidx&, int&);
    virtual void row_added(MibTableRow*, const Oidx&, MibTable*);
    virtual void row_activated(MibTableRow*, const Oidx&, MibTable*);
    virtual void row_deactivated(MibTableRow*, const Oidx&, MibTable*);
    virtual void row_delete(MibTableRow*, const Oidx&, MibTable*);
    virtual void row_init(MibTableRow*, const Oidx&, MibTable* t=0);
    boolean isInMibView(const NS_SNMP OctetStr&, const Oidx&);
    boolean addNewRow(const NS_SNMP OctetStr& viewName, const Oidx& subtree,
                      const NS_SNMP OctetStr& mask, const int type,
                      const int storageType);
    void deleteRow(const NS_SNMP OctetStr& viewName, const Oidx& subtree);
    virtual void clear()    {   StorageTable::clear(); buildViewNameIndex();}
    virtual void reset()    {   StorageTable::reset(); buildViewNameIndex();}

protected:
    boolean bit(unsigned int, NS_SNMP OctetStr&);
    ViewNameIndex* viewsOf(const NS_SNMP OctetStr& viewName);
    void buildViewNameIndex();

    List<ViewNameIndex> viewNameIndex;
};

class AGENTPP_DECL VacmViewTreeFamilyTableStatus: public snmpRowStatus
{
public:
    VacmViewTreeFamilyTableStatus(const Oidx&, int);
    virtual ~VacmViewTreeFamilyTableStatus() {}
    virtual int set(const Vbx&);
    virtual MibEntryPtr clone();

protected:
    int base_len;
};

/*********************************************************************

 VacmMIB

 ********************************************************************/
class AGENTPP_DECL VacmMIB: public MibGroup
{

public:
    VacmMIB(Vacm::ClassPointers);
};

#ifdef AGENTPP_NAMESPACE
}
#endif
#endif
