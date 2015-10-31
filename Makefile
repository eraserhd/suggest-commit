
prefix=/usr/local
bin=$(prefix)/bin

all: suggest-commits

suggest-commits: suggest-commits.c config.h
	$(CC) -o suggest-commits suggest-commits.c

config.h: Makefile
	echo "#define BINARY_PATH \"$(bin)\"" > config.h

install: all
	install -m 0755 suggest-commits $(bin)/

clean:
	rm -f config.h suggest-commits

.PHONY: all clean install
