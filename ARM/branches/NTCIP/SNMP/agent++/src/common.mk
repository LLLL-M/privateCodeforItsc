  ############################################################################
  ## 
  ##  common.mk  
  ## 
  ##
  ##  AGENT++ API Version 3.5.31
  ##  -----------------------------------------------
  ##  Copyright (C) 2000-2010 Frank Fock, Jochen Katz
  ##  
  ##  LICENSE AGREEMENT
  ##
  ##  WHEREAS,  Frank  Fock  and  Jochen  Katz  are  the  owners of valuable
  ##  intellectual  property rights relating to  the AGENT++ API and wish to
  ##  license AGENT++ subject to the  terms and conditions set forth  below;
  ##  and
  ##
  ##  WHEREAS, you ("Licensee") acknowledge  that Frank Fock and Jochen Katz
  ##  have the right  to grant licenses  to the intellectual property rights
  ##  relating to  AGENT++, and that you desire  to obtain a license  to use
  ##  AGENT++ subject to the terms and conditions set forth below;
  ##
  ##  Frank  Fock    and Jochen   Katz   grants  Licensee  a  non-exclusive,
  ##  non-transferable, royalty-free  license  to use   AGENT++ and  related
  ##  materials without  charge provided the Licensee  adheres to all of the
  ##  terms and conditions of this Agreement.
  ##
  ##  By downloading, using, or  copying  AGENT++  or any  portion  thereof,
  ##  Licensee  agrees to abide  by  the intellectual property  laws and all
  ##  other   applicable laws  of  Germany,  and  to all of   the  terms and
  ##  conditions  of this Agreement, and agrees  to take all necessary steps
  ##  to  ensure that the  terms and  conditions of  this Agreement are  not
  ##  violated  by any person  or entity under the  Licensee's control or in
  ##  the Licensee's service.
  ##
  ##  Licensee shall maintain  the  copyright and trademark  notices  on the
  ##  materials  within or otherwise  related   to AGENT++, and  not  alter,
  ##  erase, deface or overprint any such notice.
  ##
  ##  Except  as specifically   provided in  this  Agreement,   Licensee  is
  ##  expressly   prohibited  from  copying,   merging,  selling,   leasing,
  ##  assigning,  or  transferring  in  any manner,  AGENT++ or  any portion
  ##  thereof.
  ##
  ##  Licensee may copy materials   within or otherwise related   to AGENT++
  ##  that bear the author's copyright only  as required for backup purposes
  ##  or for use solely by the Licensee.
  ##
  ##  Licensee may  not distribute  in any  form  of electronic  or  printed
  ##  communication the  materials  within or  otherwise  related to AGENT++
  ##  that  bear the author's  copyright, including  but  not limited to the
  ##  source   code, documentation,  help  files, examples,  and benchmarks,
  ##  without prior written consent from the authors.  Send any requests for
  ##  limited distribution rights to fock@agentpp.com.
  ##
  ##  Licensee  hereby  grants  a  royalty-free  license  to  any  and   all 
  ##  derivatives  based  upon this software  code base,  that  may  be used
  ##  as a SNMP  agent development  environment or a  SNMP agent development 
  ##  tool.
  ##
  ##  Licensee may  modify  the sources  of AGENT++ for  the Licensee's  own
  ##  purposes.  Thus, Licensee  may  not  distribute  modified  sources  of
  ##  AGENT++ without prior written consent from the authors. 
  ##
  ##  The Licensee may distribute  binaries derived from or contained within
  ##  AGENT++ provided that:
  ##
  ##  1) The Binaries are  not integrated,  bundled,  combined, or otherwise
  ##     associated with a SNMP agent development environment or  SNMP agent
  ##     development tool; and
  ##
  ##  2) The Binaries are not a documented part of any distribution material. 
  ##
  ##
  ##  THIS  SOFTWARE  IS  PROVIDED ``AS  IS''  AND  ANY  EXPRESS OR  IMPLIED
  ##  WARRANTIES, INCLUDING, BUT NOT LIMITED  TO, THE IMPLIED WARRANTIES  OF
  ##  MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE  ARE DISCLAIMED.
  ##  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  ##  INDIRECT,   INCIDENTAL,  SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
  ##  (INCLUDING,  BUT NOT LIMITED  TO,  PROCUREMENT OF SUBSTITUTE  GOODS OR
  ##  SERVICES; LOSS OF  USE,  DATA, OR PROFITS; OR  BUSINESS  INTERRUPTION)
  ##  HOWEVER CAUSED  AND ON ANY THEORY  OF  LIABILITY, WHETHER IN CONTRACT,
  ##  STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  ##  IN  ANY WAY OUT OF  THE USE OF THIS  SOFTWARE,  EVEN IF ADVISED OF THE
  ##  POSSIBILITY OF SUCH DAMAGE. 
  ##
  ##
  ##  Stuttgart, Germany, Thu Sep  2 00:07:56 CEST 2010 
  ##  
  ##########################################################################*

SNMPPATH        = ../../snmp++/lib
SNMPINC		= ../../snmp++/include

LIBSNMP         = -L$(SNMPPATH) -lsnmp++ 

SNMPLIBPATH     = $(SNMPPATH)/libsnmp++.a
SNMPLIBS        = libsnmp++.a 

LIBPATH		= ../lib
LIBAGENTPLUS	= $(LIBPATH)/libagent++.a
SHLIBAGENTPLUS	= $(LIBPATH)/libagent++.so

INC		= ../include
INCLUDE		= -I$(INC) -I$(SNMPINC)

# agent++ lib headers
HEADERS		= $(wildcard $(INC)/agent_pp/*.h)

# agent++ lib sources
AGENTPP_SRCS	= $(wildcard *.cpp)

# Object Files produced
#OBJS		= $(AGENTPP_SRCS:.cpp=.o)
#SHOBJS		= $(AGENTPP_SRCS:.cpp=_sh.o)

OBJS		= mib.o threads.o tools.o mib_entry.o snmp_group.o \
		  request.o snmp_pp_ext.o snmp_request.o mib_proxy.o \
		  snmp_counters.o system_group.o \
		  mib_avl_map.o mib_map.o mib_context.o \
		  v3_mib.o vacm.o snmp_textual_conventions.o \
		  snmp_target_mib.o snmp_notification_mib.o \
		  notification_originator.o avl_map.o map.o sim_mib.o \
		  agentpp_simulation_mib.o mib_policy.o \
		  snmp_proxy_mib.o snmp_community_mib.o \
		  mib_complex_entry.o proxy_forwarder.o \
		  notification_log_mib.o agentpp_config_mib.o

SHOBJS		= mib_sh.o threads_sh.o tools_sh.o mib_entry_sh.o \
		  snmp_group_sh.o request_sh.o snmp_pp_ext_sh.o \
		  snmp_request_sh.o mib_proxy_sh.o snmp_counters_sh.o \
		  system_group_sh.o mib_avl_map_sh.o mib_map_sh.o \
		  v3_mib_sh.o vacm_sh.o avl_map_sh.o map_sh.o \
		  snmp_textual_conventions_sh.o snmp_target_mib_sh.o \
		  snmp_notification_mib_sh.o notification_originator_sh.o \
		  sim_mib_sh.o agentpp_simulation_mib_sh.o mib_policy_sh.o \
		  snmp_proxy_mib_sh.o snmp_community_mib_sh.o \
		  mib_context_sh.o mib_complex_entry_sh.o \
		  proxy_forwarder_sh.o notification_log_mib_sh.o \
		  agentpp_config_mib_sh.o

#
# Installation directories
#
ifndef INSTPREFIX
INSTPREFIX	= /usr/local
endif

ifndef INSTLIBPATH
INSTLIBPATH	= $(INSTPREFIX)/lib
endif

ifndef INSTINCPATH
INSTINCPATH	= $(INSTPREFIX)/include
endif

.SUFFIXES:	.cpp

all:		lib shlib

$(LIBAGENTPLUS):	$(OBJS)
			ar -rv $(LIBAGENTPLUS) $(OBJS)

$(SHLIBAGENTPLUS):	$(SHOBJS)
			$(CPP) $(SHARED) -o $(SHLIBAGENTPLUS) $(SHOBJS)

lib:		$(LIBAGENTPLUS)

shlib:		$(SHLIBAGENTPLUS)

clean:  
		$(RM) *.o *~ $(INC)/agent_pp/*~ core core.*

clobber: 	clean
		$(RM) $(LIBAGENTPLUS) $(SHLIBAGENTPLUS) 

install-common:
	install -d $(DESTDIR)$(INSTLIBPATH)
	install -d $(DESTDIR)$(INSTINCPATH)/agent_pp
	install $(INC)/agent_pp/*.h $(DESTDIR)$(INSTINCPATH)/agent_pp

install-static: lib install-common
	install $(LIBAGENTPLUS) $(DESTDIR)$(INSTLIBPATH)

install-shared: shlib install-common
	install $(SHLIBAGENTPLUS) $(DESTDIR)$(INSTLIBPATH)

install: install-static install-shared

#depedencies rules

#$(OBJS):	$(HEADERS)

#compile rules

%.o:		%.cpp $(INC)/agent_pp/*.h
		$(RM) $@
		$(CPP) $(CFLAGS) $(CLINK) $@ $(INCLUDE) $<

%_sh.o:		%.cpp $(INC)/agent_pp/*.h
		$(RM) $@
		$(CPP) $(CFLAGS) $(SHARED) $(CLINK) $@ $(INCLUDE) $<

