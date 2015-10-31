
prefix=/usr/local
bin=$(prefix)/bin

all: suggest-commits

suggest-commits: suggest-commits.cc config.h
	$(CXX) -o suggest-commits suggest-commits.cc

config.h: Makefile
	echo "#define BINARY_PATH \"$(bin)\"" > config.h

install: all
	install -m 0755 suggest-commits $(bin)/

clean:
	rm -f config.h suggest-commits

test: suggest-commits
	./suggest-commits
	git add -A
	git ci

.PHONY: all clean install test
