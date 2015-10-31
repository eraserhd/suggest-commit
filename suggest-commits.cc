#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

const char  HOOK_PROGRAM[] = ".git/hooks/prepare-commit-msg";

void install()
{
    ifstream in(BINARY_PATH "/suggest-commits", ios::in | ios::binary);
    ofstream out(HOOK_PROGRAM, ios::out | ios::binary);

    char buf[BUFSIZ];
    do {
        in.read(buf, sizeof(buf));
        out.write(buf, in.gcount());
    } while (!in.eof() && !in.fail());

    chmod(HOOK_PROGRAM, 0755);
}

void prepare()
{
	FILE *in = popen("git diff --cached", "r");
	int rc;


	rc = pclose(in);
	if (rc != 0) {
		fprintf(stderr, "error retrieving diff: %d\n", rc);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		install();
	else if (argc > 2) {
		/* We don't interfere with any of the "special" types, like
		 * merges and squash commits, which have a second argument. */
		exit(0);
	} else
		prepare();
	exit(0);
}
