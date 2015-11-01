
prefix=/usr/local
bin=$(prefix)/bin

test_HEADERS = $(wildcard t_*.hh)

all: test suggest-commits

suggest-commits: suggest.cc suggest.hh config.hh
	$(CXX) -g -o suggest-commits suggest.cc

config.hh: Makefile
	echo "#define BINARY_PATH \"$(bin)\"" > config.hh

install: all
	install -m 0755 suggest-commits $(bin)/

clean:
	rm -rf config.h suggest-commits tests.cc tests *.dSYM

tests.cc: $(test_HEADERS)
	./cxxtest/bin/cxxtestgen --error-printer -o tests.cc $(test_HEADERS)

tests: tests.cc suggest.hh config.hh
	$(CXX) -g -Icxxtest -o tests tests.cc

test: tests
	./tests

.PHONY: all clean install test
