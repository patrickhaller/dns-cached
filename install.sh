#!/bin/bash

wget http://cr.yp.to/djbdns/djbdns-1.05.tar.gz

tar zxf djbdns-1.05.tar.gz
cd djbdns-1.05
echo gcc -O2 -include /usr/include/errno.h > conf-cc
make

cp dnscache dnscache-conf /usr/local/sbin/

/usr/local/sbin/dnscache-conf nobody nobody /etc/dnscache

if [ "$?" = "0" ]; then
	echo "djbdns installation complete"
	rm -rf djbdns-1.05 djbdns-1.05.tar.gz
else
	echo "djbdns installation failed, exiting..."
	exit 1
fi

gcc -Wall dns_cached.c -o dns_cached
cp dns_cached /usr/local/sbin/.

if [ "$?" = "0" ]; then
	echo "dns_cached installation complete"
	rm -f dns_cached
else
	echo "dns_cached installation failed, exiting..."
	exit 1
fi

echo "testing installation..."

/usr/local/sbin/dns_cached /etc/dnscache &
cp /etc/resolv.conf resolv.conf
echo "nameserver 127.0.0.1" > /etc/resolv.conf
ping -c 1 x.org 
if [ "$?" != "0" ]; then
	echo "dns_cached not working for some reason..."
	kill %1
	mv resolv.conf /etc/resolv.conf
	exit
fi

echo "installing into /etc/inittab"
echo >> /etc/inittab
echo "dn:12345:respawn:/usr/local/sbin/dns_cached /etc/dnscache" >> /etc/inittab
/sbin/telinit q
