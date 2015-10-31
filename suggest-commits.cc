#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>
using namespace std;

const char HOOK_PROGRAM[] = ".git/hooks/prepare-commit-msg";

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

pair<vector<string>, vector<string> > read_diff()
{
    char path[64] = "/tmp/suggest.XXXXXX";
    if (NULL == mktemp(path)) {
        perror("mktemp");
        exit(1);
    }

    string cmd = string("git diff -b --cached > ") + path;
    int rc = system(cmd.c_str());
    if (rc != 0) {
        cerr << "unable to get diff: " << rc << endl;
        exit(1);
    }

    ifstream in(path);
    string line;
    pair<vector<string>, vector<string> > result;
    while (getline(in, line)) {
        if (line.empty())
            continue;

        switch (line[0]) {
        case '+':
            result.first.push_back(line.substr(1));
            break;
        case '-':
            result.second.push_back(line.substr(1));
            break;
        }
    }

    return result;
}

void prepare()
{
    pair<vector<string>, vector<string> > diff = read_diff();
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        install();
    else if (argc > 2) {
        // We don't interfere with any of the "special" types, like
        // merges and squash commits, which have a second argument. 
        exit(0);
    } else
        prepare();
    exit(0);
}
