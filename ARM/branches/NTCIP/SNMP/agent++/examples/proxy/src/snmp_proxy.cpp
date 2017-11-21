/*_############################################################################
  _## 
  _##  snmp_proxy.cpp  
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

#include <stdlib.h>
#include <signal.h>

#include <agent_pp/agent++.h>
#include <agent_pp/mib_proxy.h>
#include <agent_pp/snmp_group.h>
#include <snmp_pp/log.h>

//#undef _SNMPv3

#ifdef _SNMPv3
#include <agent_pp/v3_mib.h>
#include <snmp_pp/mp_v3.h>
#include <agent_pp/snmp_proxy_mib.h>
#include <agent_pp/snmp_target_mib.h>
#include <agent_pp/snmp_community_mib.h>
#include <agent_pp/snmp_notification_mib.h>
#include <agent_pp/vacm.h>
#endif

#undef _PROXY_FORWARDER
#ifdef _PROXY_FORWARDER
#error "Please #undef _PROXY_FORWARDER in agent++.h in order to use old style proxy objects"
#endif

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#ifdef AGENTPP_NAMESPACE
using namespace Agentpp;
#endif

#define  ASC_PHASE_1      "1.3.6.4.1.2000.1"
#define  ASC_PHASE_2      "1.3.6.4.1.2000.2"
#define  ASC_PHASE_3      "1.3.6.4.1.2000.3"
#define  ASC_PHASE_4      "1.3.6.4.1.2000.4"
#define  ASC_PHASE_5      "1.3.6.4.1.2000.5"
// globals:

u_short		port;
Mib*		mib;
RequestList*	requestList;
boolean		run = TRUE;

UdpAddress	source("127.0.0.1");

//DefaultLog::log = new Log();

static void sig(int signo)
{
	if ((signo == SIGTERM) || (signo == SIGINT) ||
	    (signo == SIGQUIT) || (signo == SIGSEGV)) {

		printf ("\n");
      
		switch (signo) {
		case SIGSEGV: {
			printf ("Segmentation fault, aborting.\n");
			exit(1);
		}
		case SIGTERM: 
		case SIGINT:
		case SIGQUIT: {
			if (run) printf ("User abort\n");
			run = FALSE;
		}
		}
	}
}


void init_signals()
{
	signal (SIGTERM, sig);
	signal (SIGINT, sig);
	signal (SIGQUIT, sig);
	signal (SIGSEGV, sig); 
}	



void init(Mib& mib, UdpAddress& src)
{
   	mib.add(new snmpGroup());
#ifdef _SNMPv3
	mib.add(new snmp_target_mib());
	mib.add(new snmp_community_mib());
	mib.add(new snmp_notification_mib());
	mib.add(new snmp_proxy_mib());
	//mib.add((Agentpp::MibEntry*)new MibProxyV3("1.3.6.1.2.1.1", READCREATE));
	//mib.add((Agentpp::MibEntry*)new MibProxyV3("1.3.6.1.4", READCREATE));

	UsmUserTable *uut = new UsmUserTable();

	uut->addNewRow("unsecureUser",
		       SNMPv3_usmNoAuthProtocol,
		       SNMPv3_usmNoPrivProtocol, "", "");
	
	uut->addNewRow("MD5",
		       SNMPv3_usmHMACMD5AuthProtocol,
		       SNMPv3_usmNoPrivProtocol,
		       "MD5UserAuthPassword", "");

	uut->addNewRow("SHA",
		       SNMPv3_usmHMACSHAAuthProtocol,
		       SNMPv3_usmNoPrivProtocol,
		       "SHAUserAuthPassword", "");
	
	uut->addNewRow("MD5DES",
		       SNMPv3_usmHMACMD5AuthProtocol,
		       SNMPv3_usmDESPrivProtocol,
		       "MD5DESUserAuthPassword",
		       "MD5DESUserPrivPassword");

	uut->addNewRow("SHADES",
		       SNMPv3_usmHMACSHAAuthProtocol,
		       SNMPv3_usmDESPrivProtocol,
		       "SHADESUserAuthPassword",
		       "SHADESUserPrivPassword");
	
	uut->addNewRow("MD5IDEA",
		       SNMPv3_usmHMACMD5AuthProtocol,
		       SNMPv3_usmIDEAPrivProtocol,
		       "MD5IDEAUserAuthPassword",
		       "MD5IDEAUserPrivPassword");
	
	uut->addNewRow("SHAIDEA",
		       SNMPv3_usmHMACSHAAuthProtocol,
		       SNMPv3_usmIDEAPrivProtocol,
		       "SHAIDEAUserAuthPassword",
		       "SHAIDEAUserPrivPassword");

	mib.add(uut);
	mib.add(new V3SnmpEngine());
	mib.add(new MPDGroup());
	mib.add(new UsmStats());
#else
	//mib.add(new MibProxy("1.3.6.1.2.1.1", READCREATE, src));
	//mib.add(new MibProxy("1.3.6.1.4", READCREATE, src));
#endif	
	snmp_community_mib::add_public();
}	
int g_Value1=0,g_Value2=0,g_Value3=0,g_Value4=0,g_Value5=0;

void getValue(char *pstrOid, Vbx &vb, int nType)
{
	if (0 == strcmp(pstrOid, ASC_PHASE_1))
	{
		if (0 == nType)
		{
			vb.set_value(g_Value1);
		}
		else
		{
			//g_Value1 =
			//²âÊÔ´úÂë
		}
	}
}

main (int argc, char* argv[])
{
#if 0	
	if (argc>1)
		port = atoi(argv[1]);
	else
		port = 4700;

	if (port==0) 
#ifdef _SNMPv3
		printf("usage: %s [port [remote hostname [remote port]]]\n", 
		       argv[0]);  
#else
		printf("usage: %s [port]\n", 
		       argv[0]);  
#endif
	source.set_port(161);
	if (argc>2)
		source = argv[2];

	if (argc>3)
		source.set_port(atoi(argv[3]));
#endif
	
	source.set_port(161);
	
	DefaultLog::log()->set_filter(ERROR_LOG, 2);
	DefaultLog::log()->set_filter(WARNING_LOG, 4);
	DefaultLog::log()->set_filter(EVENT_LOG, 4);
	DefaultLog::log()->set_filter(INFO_LOG, 4);
	DefaultLog::log()->set_filter(DEBUG_LOG, 9);


	int status;
	port = 161;
	Snmpx snmp(status, port);

	if (status == SNMP_CLASS_SUCCESS) {

		LOG_BEGIN(EVENT_LOG | 1);
		LOG("main: SNMP listen port");
		LOG(port);
		LOG_END;
	}
	else {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("main: SNMP port init failed");
		LOG(status);
		LOG_END;
		exit(1);
	}

#ifdef _SNMPv3
        char *filename = "snmpv3_boot_counter";
        unsigned int snmpEngineBoots = 0;
        OctetStr engineId(SnmpEngineID::create_engine_id(port));

        // you may use your own methods to load/store this counter
        status = getBootCounter(filename, engineId, snmpEngineBoots);
        if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR)) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("main: Error loading snmpEngineBoots counter (status)");
		LOG(status);
		LOG_END;
		exit(1);
	}

        snmpEngineBoots++;
        status = saveBootCounter(filename, engineId, snmpEngineBoots);
        if (status != SNMPv3_OK) {
		LOG_BEGIN(ERROR_LOG | 0);
		LOG("main: Error saving snmpEngineBoots counter (status)");
		LOG(status);
		LOG_END;
		exit(1);
	}

	int stat;
        v3MP *v3mp = new v3MP(engineId, snmpEngineBoots, stat);

	v3mp->get_usm()->add_usm_user("public",
				      SNMPv3_usmNoAuthProtocol,
				      SNMPv3_usmNoPrivProtocol, "", "" );

#endif

	mib = new Mib();
	requestList = new RequestList();
#ifdef _SNMPv3
	// register v3MP
	requestList->set_v3mp(v3mp);
#endif
	// register request list
	mib->set_request_list(requestList);

	init(*mib, source); // has to be done after mpInit(...)


#ifdef _SNMPv3
	// register VACM
	Vacm* vacm = new Vacm(*mib);
	requestList->set_vacm(vacm);

	// initialize security information
        vacm->addNewContext("");
        // Add new entries to the SecurityToGroupTable.
        // Used to determine the group a given SecurityName belongs to. 
        // User "new" of the USM belongs to newGroup

        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "new",
                         "newGroup", storageType_volatile);

        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "test", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_V2, "public", 
                         "v1v2group", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_V1, "public", 
                         "v1v2group", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "initial", 
                         "initial", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "unsecureUser", 
                         "initial", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5", 
                         "testNoPrivGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHA", 
                         "testNoPrivGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5DES", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHADES", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5IDEA", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHAIDEA", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5AES128",
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHAAES128", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5AES192",
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHAAES192", 
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "MD5AES256",
                         "testGroup", storageType_volatile);
        vacm->addNewGroup(SNMP_SECURITY_MODEL_USM, "SHAAES256", 
                         "testGroup", storageType_volatile);

        // remove a group with:
        //vacm->deleteGroup(SNMP_SECURITY_MODEL_USM, "neu");

        // Set access rights of groups.
        // The group "newGroup" (when using the USM with a security
	// level >= noAuthNoPriv within context "") would have full access  
        // (read, write, notify) to all objects in view "newView". 
        vacm->addNewAccessEntry("newGroup", 
				"other",        // context
				SNMP_SECURITY_MODEL_USM, 
				SecurityLevel_noAuthNoPriv,
				match_exact,  // context must mach exactly
				// alternatively: match_prefix  
				"newView", // readView
				"newView", // writeView
				"newView", // notifyView
				storageType_nonVolatile);
        vacm->addNewAccessEntry("testGroup", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_authPriv, 
				match_prefix,
				"testView", "testView", 
				"testView", storageType_nonVolatile);
        vacm->addNewAccessEntry("testNoPrivGroup", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_authNoPriv, 
				match_prefix,
				"testView", "testView", 
				"testView", storageType_nonVolatile);
        vacm->addNewAccessEntry("testGroup", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_noAuthNoPriv, 
				match_prefix,
				"testView", "testView", 
				"testView", storageType_nonVolatile);
        vacm->addNewAccessEntry("v1v2group", "", 
				SNMP_SECURITY_MODEL_V2, SecurityLevel_noAuthNoPriv, 
				match_exact,
				"v1ReadView", "v1WriteView", 
				"v1NotifyView", storageType_nonVolatile);
        vacm->addNewAccessEntry("v1v2group", "", 
				SNMP_SECURITY_MODEL_V1, SecurityLevel_noAuthNoPriv, 
				match_exact,
				"v1ReadView", "v1WriteView", 
				"v1NotifyView", storageType_nonVolatile);
        vacm->addNewAccessEntry("initial", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_noAuthNoPriv, 
				match_exact,
				"restricted", "", 
				"restricted", storageType_nonVolatile);
        vacm->addNewAccessEntry("initial", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_authNoPriv, 
				match_exact,
				"internet", "internet", 
				"internet", storageType_nonVolatile);
        vacm->addNewAccessEntry("initial", "",
				SNMP_SECURITY_MODEL_USM, SecurityLevel_authPriv, 
				match_exact,
				"internet", "internet", 
				"internet", storageType_nonVolatile);

        // remove an AccessEntry with:
        // vacm->deleteAccessEntry("newGroup", 
	//	      		"",        
	//			SNMP_SECURITY_MODEL_USM, 
	//			SecurityLevel_noAuthNoPriv);


        // Defining Views
        // View "v1ReadView" includes all objects starting with "1.3".
        // If the ith bit of the mask is not set (0), then also all objects
	// which have a different subid at position i are included in the 
	// view.
        // For example: Oid "6.5.4.3.2.1", Mask(binary) 110111 
        //              Then all objects with Oid with "6.5.<?>.3.2.1" 
	//              are included in the view, whereas <?> may be any
	//              natural number.

        vacm->addNewView("v1ReadView", 
			 "1.3",       
			 "",             // Mask "" is same as 0xFFFFFFFFFF...
			 view_included,  // alternatively: view_excluded
			 storageType_nonVolatile);

        vacm->addNewView("v1WriteView", 
			 "1.3",       
			 "",             // Mask "" is same as 0xFFFFFFFFFF...
			 view_included,  // alternatively: view_excluded
			 storageType_nonVolatile);

        vacm->addNewView("v1NotifyView", 
			 "1.3",       
			 "",             // Mask "" is same as 0xFFFFFFFFFF...
			 view_included,  // alternatively: view_excluded
			 storageType_nonVolatile);

        vacm->addNewView("newView", "1.3", "", 
			 view_included, storageType_nonVolatile);
        vacm->addNewView("testView", "1.3.6", "",
			 view_included, storageType_nonVolatile);
        vacm->addNewView("internet", "1.3.6.1","",
			 view_included, storageType_nonVolatile);
        vacm->addNewView("restricted", "1.3.6.1.2.1.1","",
			 view_included, storageType_nonVolatile);
        vacm->addNewView("restricted", "1.3.6.1.2.1.11","", 
			 view_included, storageType_nonVolatile);
        vacm->addNewView("restricted", "1.3.6.1.6.3.10.2.1","", 
			 view_included, storageType_nonVolatile);
        vacm->addNewView("restricted", "1.3.6.1.6.3.11.2.1","",
			 view_included, storageType_nonVolatile);
        vacm->addNewView("restricted", "1.3.6.1.6.3.15.1.1","", 
			 view_included, storageType_nonVolatile);

#endif

	requestList->set_snmp(&snmp);

	init_signals();

	// load persitent objects from disk
	mib->init();
	
	Request* snmprequest;
	while (run) {	  
		snmprequest = requestList->receive(2);
		if (snmprequest != NULL)
		{
			//AfxMessageBox(snmprequest->get_oid(0).get_printable());
			int ind = snmprequest->subrequests();
			printf("[snmprequest->subrequests()=%d]\n", snmprequest->subrequests());
#if 1
			//snmprequest->get_type();
			//snmprequest->get_request_id();
			int nversion = snmprequest->get_snmp_version();
			
// 			CString strType;
// 			strType.Format("type:0x%X", snmprequest->get_type());
// 			AfxMessageBox(strType);
			printf("snmprequest->get_type()=%d\n", snmprequest->get_type());
			printf("[%s]\n", snmprequest->get_value(0).get_printable_value());

			Oidx oidx;
			snmprequest->get_value(0).get_oid(oidx);
			//Oid oid1;
			//oid1 = oidx;
			Vbx vbRespond;
			//vbRespond.set_value("result!!!!");
			if (GET_REQ_MSG == snmprequest->get_type())
			{
				vbRespond.set_value(123);
			}
			if (SET_REQ_MSG == snmprequest->get_type())
			{
				//snmprequest->set_error_index(10);
				if (0 == strlen(snmprequest->get_value(0).get_printable_value()))
				{
					snmprequest->set_error_status(10);
				}
				snmprequest->set_ready(0);
			}
			printf("GET_REQ_MSG=%d, snmprequest->get_type()=%d\n", GET_REQ_MSG, snmprequest->get_type());
			vbRespond.set_oid(oidx);
			snmprequest->finish(0, vbRespond);
#endif
			mib->process_request(snmprequest);
		}
		else 
		{
		    mib->cleanup();
		}
	}
	delete mib;
}

