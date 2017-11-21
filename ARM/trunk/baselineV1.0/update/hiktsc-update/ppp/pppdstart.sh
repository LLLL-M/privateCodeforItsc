sleep 10
pppd call gprs > /etc/ppp/login.log &
sleep 10
cp /etc/ppp/resolv.conf /etc -f
LOCALIP=`ifconfig ppp0|grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d "addr:"`
route add default gw $LOCALIP
