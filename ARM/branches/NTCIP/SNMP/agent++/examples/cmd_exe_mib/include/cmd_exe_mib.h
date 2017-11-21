/*_############################################################################
  _## 
  _##  cmd_exe_mib.h  
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


#include <agent_pp/mib.h>
#include <agent_pp/snmp_textual_conventions.h>

#ifdef AGENTPP_NAMESPACE
namespace Agentpp {
#endif


/**
 *  cmdExecutionCmdConfigName
 *
"The name of the command."
 */


class cmdExecutionCmdConfigName: public MibLeaf {

 public:
	cmdExecutionCmdConfigName(const Oidx&);
	virtual ~cmdExecutionCmdConfigName();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
};


/**
 *  cmdExecutionCmdConfigLine
 *
"The command's command line."
 */


class cmdExecutionCmdConfigLine: public MibLeaf {

 public:
	cmdExecutionCmdConfigLine(const Oidx&);
	virtual ~cmdExecutionCmdConfigLine();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
	virtual int        	set(const Vbx&);
	virtual boolean    	value_ok(const Vbx&);
};



/**
 *  cmdExecutionCmdNextIndex
 *
"Identifies a hint for the next value of 
 cmdExecutionCmdIndex to be used in a row creation 
 attempt for the cmdExecutionCmdTable table. If new rows 
 can not be created, this object will have a value of 0"
 */


class cmdExecutionCmdNextIndex: public MibLeaf {

 public:
	cmdExecutionCmdNextIndex();
	virtual ~cmdExecutionCmdNextIndex();

	static cmdExecutionCmdNextIndex* instance;

	virtual void       	get_request(Request*, int);
	virtual long       	get_state();
	virtual void       	set_state(long);
};


/**
 *  cmdExecutionCmdIndex
 *
"The index for the command."
 */


class cmdExecutionCmdIndex: public MibLeaf {

 public:
	cmdExecutionCmdIndex(const Oidx&);
	virtual ~cmdExecutionCmdIndex();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
	virtual long       	get_state();
	virtual void       	set_state(long);
};


/**
 *  cmdExecutionCmdName
 *
"The index of the command in cmdExecutionCmdConfigTable 
 to be executed."
 */


class cmdExecutionCmdName: public MibLeaf {

 public:
	cmdExecutionCmdName(const Oidx&);
	virtual ~cmdExecutionCmdName();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
	virtual int        	set(const Vbx&);
	virtual boolean    	value_ok(const Vbx&);
};


/**
 *  cmdExecutionCmdStatus
 *
"The status of the command."
 */


class cmdExecutionCmdStatus: public MibLeaf {

 public:
	cmdExecutionCmdStatus(const Oidx&);
	virtual ~cmdExecutionCmdStatus();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
	virtual long       	get_state();
	virtual void       	set_state(long);
	
};


/**
 *  cmdExecutionCmdRunTime
 *
"The execution time of the command."
 */


class cmdExecutionCmdRunTime: public MibLeaf {

 public:
	cmdExecutionCmdRunTime(const Oidx&);
	virtual ~cmdExecutionCmdRunTime();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);

	virtual long		get_state();
	virtual void		set_state(long);
	virtual void            start();
	virtual void		end();
 private:
	time_t  start_time;
	time_t  end_time;
};


/**
 *  cmdExecutionCmdRowStatus
 *
"The row status of this conceptual row. When this row
 is activated the specified command is executed. After
 execution the row status is notInService."
 */


class cmdExecutionCmdRowStatus: public snmpRowStatus {

 public:
	cmdExecutionCmdRowStatus(const Oidx&);
	virtual ~cmdExecutionCmdRowStatus();

	virtual MibEntryPtr	clone();
	virtual long       	get_state();
	virtual void       	set_state(long);
	virtual int        	set(const Vbx&);
	virtual int		prepare_set_request(Request*, int&); 
};


/**
 *  cmdExecutionOutputLineNumber
 *
"The line number of the output entry."
 */


class cmdExecutionOutputLineNumber: public MibLeaf {

 public:
	cmdExecutionOutputLineNumber(const Oidx&);
	virtual ~cmdExecutionOutputLineNumber();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
	virtual long       	get_state();
	virtual void       	set_state(long);
};


/**
 *  cmdExecutionOutputLine
 *
"The output line."
 */


class cmdExecutionOutputLine: public MibLeaf {

 public:
	cmdExecutionOutputLine(const Oidx&);
	virtual ~cmdExecutionOutputLine();

	virtual MibEntryPtr	clone();
	virtual void       	get_request(Request*, int);
};


/**
 *  cmdExecutionCmdConfigEntry
 *
"An entry in the available command table."
 */


class cmdExecutionCmdConfigEntry: public StorageTable {

 public:
	cmdExecutionCmdConfigEntry();
	virtual ~cmdExecutionCmdConfigEntry();

	static cmdExecutionCmdConfigEntry* instance;

	virtual boolean		deserialize(char*, int&);
	virtual void        	row_added(MibTableRow*, const Oidx&);
	virtual void        	row_delete(MibTableRow*, const Oidx&);
	virtual void       	set_row(int index, const char* p0, int p1, int p2);
	virtual boolean		contains(Oidx);
	virtual NS_SNMP OctetStr	get_command_line(const NS_SNMP OctetStr&);
};


/**
 *  cmdExecutionCmdEntry
 *
"An entry in the command execution table. Each row contains 
 information about an UNIX command that should be executed
 (rowStatus is notInService and status is idle(1))
 or is executing (rowStatus is active and status is 
 running(2)) or has been executed (rowStatus is 
 notInService and status is finished(3))."
 */


class cmdExecutionCmdEntry: public MibTable {
friend class cmdExecutionCmdRowStatus;
 public:
	cmdExecutionCmdEntry();
	virtual ~cmdExecutionCmdEntry();

	static cmdExecutionCmdEntry* instance;

	virtual void        	row_added(MibTableRow*, const Oidx&);
	virtual void        	row_delete(MibTableRow*, const Oidx&);
 protected:
	ThreadPool*		threadPool;
};


/**
 *  cmdExecutionOutputEntry
 *
"An entry of the output list."
 */


class cmdExecutionOutputEntry: public MibTable {

 public:
	cmdExecutionOutputEntry();
	virtual ~cmdExecutionOutputEntry();

	static cmdExecutionOutputEntry* instance;

	virtual void        	row_added(MibTableRow*, const Oidx&);
	virtual void        	row_delete(MibTableRow*, const Oidx&);
	virtual void       	set_row(int index, char* p0);
	virtual void		remove_all(const Oidx&);
};


class command_execution_mib: public MibGroup
{
  public:
	command_execution_mib();
	virtual ~command_execution_mib() { }
};

class CmdThread: public Runnable {
 public:
	CmdThread(MibTableRow* r) { row_ptr = r; }
	virtual ~CmdThread() { }
	virtual void run();
 protected:
	MibTableRow* row_ptr;
};

#ifdef AGENTPP_NAMESPACE
}
#endif


/**
 * cmd_exe_mib.h generated by AgentGen 0.98 Sat May 08 18:46:48 GMT+04:30 1999.
 */

