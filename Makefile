
prefix=/usr/local
bin=$(prefix)/bin

test_HEADERS = $(wildcard t_*.h)

all: test suggest-commits

suggest-commits: suggest.cc suggest.h config.h
	$(CXX) -o suggest-commits suggest.cc

config.h: Makefile
	echo "#define BINARY_PATH \"$(bin)\"" > config.h

install: all
	install -m 0755 suggest-commits $(bin)/

clean:
	rm -f config.h suggest-commits tests.cc tests

tests.cc: $(test_HEADERS)
	./cxxtest/bin/cxxtestgen --error-printer -o tests.cc $(test_HEADERS)

tests: tests.cc suggest.h config.h
	$(CXX) -Icxxtest -o tests tests.cc

test: tests
	./tests

.PHONY: all clean install test
