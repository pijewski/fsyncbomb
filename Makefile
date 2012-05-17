all:
	gcc -o fsyncbomb fsyncbomb.c

clean:
	rm -f fsyncbomb

check:
	cstyle fsyncbomb.c
