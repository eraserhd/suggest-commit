
prefix=/usr/local
bin=$(prefix)/bin

all: test suggest-commit

suggest-commit: suggest.cc
	$(CXX) -g -o suggest-commit suggest.cc

install: all
	install -m 0755 suggest-commit $(bin)/

clean:
	rm -rf suggest-commit tests.cc tests *.dSYM

test: suggest-commit
	@for t in t/*; do \
	  if ./$$t; then \
	    printf 'ok %s.\n' "$$t"; \
	  else \
	    printf 'FAILED %s.\n' "$$t"; \
	  fi; \
	done

.PHONY: all clean install test
