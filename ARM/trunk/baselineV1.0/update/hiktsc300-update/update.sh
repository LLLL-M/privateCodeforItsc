#!/bin/bash

#清理进程，防止程序正在运行时无法覆盖的问题
killall hikTSC
killall RTSC_sadp
killall GPS
#killall ledctrl
watchdog -t 1 -T 3 /dev/watchdog

rm /root/CPLD* -f 2> /dev/null
echo "Copying Files..." 
cp ./hikTSC  /root/ -f
cp ./hikTSC300 /root -f
cp ./GPS /root -f
#cp ./ledctrl /root/ -f
cp ./RTSC_sadp  /root/ -f
cp ./*.ko  /root/ -f
cp ./S10mdev  /etc/init.d/ -f
cp ./S40network  /etc/init.d/ -f
cp ./sbin/*  /sbin/  -rf
cp ./wpa_supplicant.conf  /etc/ -f
cp ./vsftpd.conf  /etc/  -f
cp ./sshd_config  /etc/  -f
cp ./issue /etc/ -f
cp ./io_test /root -f
cp ./BinaryTextConvert /root -f
cp ./sig.sh /root/ -f

chmod 777 /root/*

#查看interface里面有没有eth0，如果有的话，就删除掉
sed -i "/auto[ \t]*eth0/,/gateway[ \t]*172\.7\.18\.1/d" /etc/network/interfaces
sync

networkcard="eth1"

for name in $networkcard
do
#获取指定网卡的网关
gateway=`grep "iface[ \t]*${name}" -A4 /etc/network/interfaces | awk '{if($1=="gateway") printf $2}'`

#配置中没有配置mac地址，则在配置中加入pre-up ifconfig $name hw ether mac用以固定mac
if ! grep "pre-up ifconfig $name hw ether" -q /etc/network/interfaces
then
mac=`ifconfig $name | awk 'NR==1{print $5}'`
test -z $mac || sed -i "/iface[ \t]*${name}/a pre-up ifconfig ${name} hw ether ${mac}" /etc/network/interfaces
sync
#第一次配置mac时需要更新网关的arp缓存区
test -z $gateway || arping -q -c 4 -w 4 -I $name $gateway &
fi

if ! grep "arping -q -c 4 -w 4 -I $name" -q /etc/init.d/rcS
then
#利用arping命令更新网关的arp缓存区
test -z $gateway || echo "arping -q -c 4 -w 4 -I $name $gateway &" >> ./rcS
fi

done
cp ./rcS  /etc/init.d/  -f
sync

#检查/etc/network/interfaces中是否有3G和wifi相关配置
a=`cat /etc/network/interfaces |grep wpa_supplicant|wc -l`
echo "a= $a"
if [ $a -ge 1 ];
then 
   echo "3G/WIFI ready"   
else echo "auto wlan0">> /etc/network/interfaces
     echo "iface wlan0 inet static">> /etc/network/interfaces
     echo "address 192.168.9.101">> /etc/network/interfaces
     echo "netmask 255.255.255.0">> /etc/network/interfaces
     echo "gateway 192.168.9.1">> /etc/network/interfaces
     echo "pre-up wpa_supplicant -iwlan0 -c/etc/wpa_supplicant.conf&">> /etc/network/interfaces
     echo "post-down killall -9 wpa_supplicant">> /etc/network/interfaces	
fi

sync
echo "Update finished!"
