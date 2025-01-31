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

INCLUDE		= -I../include -I../../../include \
		  -I../../../../snmp++/include

SNMPLIBPATH	= ../../../../snmp++/lib
AGENTLIBPATH	= ../../../lib
LIBDESPATH	= ../../../../libdes
LIBTOMCRYPTPATH	= ../../../../crypt

LIBDES		= $(LIBDESPATH)/libdes.a
LIBTOMCRYPT	= $(LIBTOMCRYPTPATH)/libtomcrypt.a

# Set crypto lib to use
ifneq ($(wildcard $(LIBTOMCRYPT)),)
CRYPTOLIB	= -L$(LIBTOMCRYPTPATH) -ltomcrypt
else
ifneq ($(wildcard $(LIBDES)),)
CRYPTOLIB	= -L$(LIBDESPATH) -ldes
else
CRYPTOLIB	=
endif
endif

LIB	        = -L$(AGENTLIBPATH) -L$(SNMPLIBPATH) \
		  -lpthread -lagent++ -lsnmp++ $(CRYPTOLIB) $(SYSLIBS)

AGENTOBJ	= agent_copy.o

EXECUTABLE	= agent_copy

HEADERS		= 

all:		$(EXECUTABLE)

clean:  
	$(RM) *.o *~ ../include/*~

clobber:	clean
		$(RM) $(EXECUTABLE)

#compile rules
.SUFFIXES:	.cpp

$(EXECUTABLE):  $(AGENTOBJ) $(SNMPLIBPATH)/libsnmp++.a \
		$(AGENTLIBPATH)/libagent++.a
		$(CPP) $(CFLAGS) -o $@ $(AGENTOBJ) $(LIB) 

%.o:		%.cpp $(HEADERS)
		$(RM) $@
		$(CPP) $(CFLAGS) $(CLINK) $@ $(INCLUDE) $< 
