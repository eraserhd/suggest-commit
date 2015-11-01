
prefix=/usr/local
bin=$(prefix)/bin

all: test suggest-commit

suggest-commit: suggest.cc
	$(CXX) -g -o suggest-commit suggest.cc

install: all
	install -m 0755 suggest-commit $(bin)/

clean:
	rm -rf suggest-commit tests.cc tests *.dSYM

tests.cc: tests.hh
	./cxxtest/bin/cxxtestgen --error-printer -o tests.cc tests.hh

tests: tests.cc suggest.cc
	$(CXX) -g -Icxxtest -o tests tests.cc

test: tests
	./tests

.PHONY: all clean install test
