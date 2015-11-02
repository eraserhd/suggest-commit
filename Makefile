
prefix=/usr/local
bin=$(prefix)/bin

all: test suggest-commit

suggest-commit: suggest.c
	$(CC) -g -o suggest-commit suggest.c

install: all
	install -m 0755 suggest-commit $(bin)/

clean:
	rm -rf suggest-commit *.dSYM

test: suggest-commit
	@for t in t/*; do \
	  if ./$$t; then \
	    printf 'ok %s.\n' "$$t"; \
	  else \
	    printf 'FAILED %s.\n' "$$t"; \
	  fi; \
	done

.PHONY: all clean install test
