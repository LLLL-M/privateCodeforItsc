LOCALIP=`ifconfig ppp0|grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d "addr:"`
route del default gw $LOCALIP
killall -9 pppd
