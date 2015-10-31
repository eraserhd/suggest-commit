#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char HOOK_PROGRAM[] = ".git/hooks/prepare-commit-msg";

void install()
{
	FILE *in, *out;
	char buf[BUFSIZ];
	size_t amount_read;
	
	if (!(in = fopen(BINARY_PATH "/suggest-commits", "rb"))) {
		perror(BINARY_PATH "/suggest-commits");
		exit(1);
	}
	if (!(out = fopen(HOOK_PROGRAM, "wb"))) {
		perror(HOOK_PROGRAM);
		exit(1);
	}

	for (;;) {
		amount_read = fread(buf, 1, sizeof(buf), in);
		if (ferror(in)) {
			perror("fread");
			exit(1);
		}
		if (amount_read == 0)
			break;
		if (fwrite(buf, 1, amount_read, out) < amount_read) {
			perror("fwrite");
			exit(1);
		}
	}

	fclose(in);

	fchmod(fileno(out), 0755);
	fclose(out);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		install();
	exit(0);
}
