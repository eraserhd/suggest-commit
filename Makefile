
prefix=/usr/local
bin=$(prefix)/bin

test_HEADERS = $(wildcard t_*.h)

all: test suggest-commits

suggest-commits: suggest-commits.cc config.h
	$(CXX) -o suggest-commits suggest-commits.cc

config.h: Makefile
	echo "#define BINARY_PATH \"$(bin)\"" > config.h

install: all
	install -m 0755 suggest-commits $(bin)/

clean:
	rm -f config.h suggest-commits

tests.cc: $(test_HEADERS)
	./cxxtest/bin/cxxtestgen --error-printer -o tests.cc $(test_HEADERS)

tests: tests.cc
	$(CXX) -Icxxtest -o tests tests.cc

test: tests
	./tests

.PHONY: all clean install test
