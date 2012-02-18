all:
	gcc -Wall dns_cached.c -o dns_cached

install:
	install -m 0755 dns_cached /usr/local/sbin/dns_cached

clean:
	rm dns_cached
