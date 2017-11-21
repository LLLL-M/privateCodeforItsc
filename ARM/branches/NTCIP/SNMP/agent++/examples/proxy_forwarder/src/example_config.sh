#!/bin/sh
# 
# You will need NET-SNMP 5.4.x installed for runnig this script
# It can be downloaded from http://www.net-snmp.org
#
# This configures a ProxyForwarder from SNMPv2c on port 4700
# to SNMPv1 on port 161 (localhost)
# To test the example run the command below after having
# executed this script.
#
# snmpgetnext -v2c -c proxy_me localhost:4700 .1.3
# snmpstatus -v2c -c proxy_me localhost:4700
# snmpstatus -v2c -c public localhost:4700
#
# see too:
# http://net-snmp.sourceforge.net/wiki/index.php?title=Snmpd_proxy&printable=yes

SET="snmpset -O s -v 2c -c public localhost:4700 "

set -x
set -e


echo "TargetParamsTable:"

# SNMP table: snmpTargetParamsTable
#               index MPModel SecurityModel SecurityName SecurityLevel StorageType RowStatus
#     'defaultV1Trap'       0             1       public  noAuthNoPriv nonVolatile    active
#  'remote_params_in'       1             2     proxy_me  noAuthNoPriv nonVolatile    active
# 'remote_params_out'       0             1       public  noAuthNoPriv nonVolatile    active

$SET snmpTargetParamsRowStatus.\'remote_params_in\' i destroy || echo ignored
$SET snmpTargetParamsRowStatus.\'remote_params_in\' i 5
$SET snmpTargetParamsRowStatus.\'remote_params_out\' i destroy || echo ignored
$SET snmpTargetParamsRowStatus.\'remote_params_out\' i 5
$SET snmpTargetParamsMPModel.\'remote_params_in\' i 1	# SNMPv1(1)
$SET snmpTargetParamsMPModel.\'remote_params_out\' i 0	# SNMPv3(3) XXX
$SET snmpTargetParamsSecurityModel.\'remote_params_in\' i 2	# SNMPv2c(2)
$SET snmpTargetParamsSecurityModel.\'remote_params_out\' i 1	# SNMPv1(1) USM(3)
$SET snmpTargetParamsSecurityName.\'remote_params_in\' s "proxy_me"
$SET snmpTargetParamsSecurityName.\'remote_params_out\' s "public"
$SET snmpTargetParamsSecurityLevel.\'remote_params_in\' i 1
$SET snmpTargetParamsSecurityLevel.\'remote_params_out\' i 1	# noAuthNoPriv(1) autNoPriv(2)
$SET snmpTargetParamsStorageType.\'remote_params_in\' i 3
$SET snmpTargetParamsStorageType.\'remote_params_out\' i 3
$SET snmpTargetParamsRowStatus.\'remote_params_in\' i 1
$SET snmpTargetParamsRowStatus.\'remote_params_out\' i 1

echo "TargetAddrTable:"

# SNMP table: snmpTargetAddrTable
#              index       TDomain             TAddress Timeout RetryCount    TagList            Params StorageType
#    '127.0.0.1/162' snmpUDPDomain "7F 00 00 01 00 A2 "    1500          3     v1trap     defaultV1Trap nonVolatile
#      'localAccess' snmpUDPDomain "7F 00 00 01 00 A1 "    1500          3     access       localAccess nonVolatile
#'remote_target_out' snmpUDPDomain "7F 00 00 01 00 A1 "    1500          3 Proxy_Me_p remote_params_out nonVolatile

$SET snmpTargetAddrRowStatus.\'remote_target_out\' i destroy || echo ignored
$SET snmpTargetAddrRowStatus.\'remote_target_out\' i 5
$SET snmpTargetAddrTDomain.\'remote_target_out\' o .1.3.6.1.6.1.1
$SET snmpTargetAddrTAddress.\'remote_target_out\' x 7F00000100A1 
$SET snmpTargetAddrTimeout.\'remote_target_out\' i 1500
$SET snmpTargetAddrRetryCount.\'remote_target_out\' i 3
$SET snmpTargetAddrTagList.\'remote_target_out\' s "Proxy_Me_p"
$SET snmpTargetAddrParams.\'remote_target_out\' s "remote_params_out"
$SET snmpTargetAddrStorageType.\'remote_target_out\' i 3
$SET snmpTargetAddrRowStatus.\'remote_target_out\' i 1

echo "CommunityTable:"

# SNMP table: snmpCommunityTable
#        index       Name        SecurityName                                 ContextEngineID        ContextName Tran
# 'Proxy_Me_c' "proxy_me" dummy_security_name                       "dummy_context_engine_id" dummy_context_name
#     'public'   "public"              public "80 00 13 70 05 77 64 30 30 32 30 37 32 12 5C "

$SET snmpCommunityStatus.\'Proxy_Me_c\' i destroy || echo ignored
$SET snmpCommunityStatus.\'Proxy_Me_c\' i 5
$SET snmpCommunityName.\'Proxy_Me_c\' s "proxy_me"
$SET snmpCommunitySecurityName.\'Proxy_Me_c\' s dummy_security_name
$SET snmpCommunityContextEngineID.\'Proxy_Me_c\' s "dummy_context_engine_id"
$SET snmpCommunityContextName.\'Proxy_Me_c\' s "dummy_context_name"
#$SET snmpCommunityTransportTag.\'Proxy_Me_c\' s "" 
$SET snmpCommunityStorageType.\'Proxy_Me_c\' i 3
$SET snmpCommunityStatus.\'Proxy_Me_c\' i 1


echo "ProxyTable:"

# SNMP table: snmpProxyTable
#       index Type           ContextEngineID        ContextName   TargetParamsIn   SingleTargetOut MultipleTargetOut StorageType RowStatus
#'Proxy_Me_p' read "dummy_context_engine_id" dummy_context_name remote_params_in remote_target_out                   nonVolatile    active

$SET SNMP-PROXY-MIB::snmpProxyRowStatus.\'Proxy_Me_p\' i destroy || echo ignored
$SET SNMP-PROXY-MIB::snmpProxyRowStatus.\'Proxy_Me_p\' i createAndWait
$SET SNMP-PROXY-MIB::snmpProxyType.\'Proxy_Me_p\' i read
$SET SNMP-PROXY-MIB::snmpProxyContextEngineID.\'Proxy_Me_p\' s "dummy_context_engine_id"
$SET SNMP-PROXY-MIB::snmpProxyContextName.\'Proxy_Me_p\' s "dummy_context_name"
$SET SNMP-PROXY-MIB::snmpProxyTargetParamsIn.\'Proxy_Me_p\' s "remote_params_in"
$SET SNMP-PROXY-MIB::snmpProxySingleTargetOut.\'Proxy_Me_p\' s "remote_target_out"
### $SET SNMP-PROXY-MIB::snmpProxyMultipleTargetOut.\'Proxy_Me_p\' s ""
$SET SNMP-PROXY-MIB::snmpProxyStorageType.\'Proxy_Me_p\' i nonVolatile
$SET SNMP-PROXY-MIB::snmpProxyRowStatus.\'Proxy_Me_p\' i active

echo "done."


snmptable -v 2c -c public -O s -C bi localhost:4700 TargetParamsTable
snmptable -v 2c -c public -O s -C bi localhost:4700 TargetAddrTable
snmptable -v 2c -c public -O s -C bi localhost:4700 CommunityTable
snmptable -v 2c -c public -O s -C bi -m +SNMP-PROXY-MIB localhost:4700 ProxyTable

snmpstatus -v2c -c proxy_me localhost:4700 system
# [UDP: [127.0.0.1]:4700->[0.0.0.0]]=>[CYGWIN_NT-5.1 wd002072 1.5.25(0.156/4/2) 2008-06-12 19:34 i686] Up: 17:10:55.57
# Interfaces: 2, Recv/Trans packets: 1044686/891643 | IP: 981282/890669

snmpstatus -v2c -c public localhost:4700
# [UDP: [127.0.0.1]:4700->[0.0.0.0]]=>[AGENT++v3.4.2 Proxy Forwarder - Use 'MD5' as SNMPv3 user and 'MD5UserAuthPassword' as authentication] Up: 2:05:17.66
# Interfaces: 0, Recv/Trans packets: 0/0 | IP: 0/0

####
exit
####
