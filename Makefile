
prefix=/usr/local
bin=$(prefix)/bin

test_HEADERS = $(wildcard t_*.hh)

all: test suggest-commit

suggest-commit: suggest.cc suggest.hh
	$(CXX) -g -o suggest-commit suggest.cc

install: all
	install -m 0755 suggest-commit $(bin)/

clean:
	rm -rf suggest-commit tests.cc tests *.dSYM

tests.cc: $(test_HEADERS)
	./cxxtest/bin/cxxtestgen --error-printer -o tests.cc $(test_HEADERS)

tests: tests.cc suggest.hh
	$(CXX) -g -Icxxtest -o tests tests.cc

test: tests
	./tests

.PHONY: all clean install test
