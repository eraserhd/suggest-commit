#include "config.h"
#include "suggest.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>
using namespace std;
#define ALL(c) (c).begin(),(c).end()
#define TR(c,i) for (typeof((c).begin()) i = (c).begin(); i != (c).end(); ++i)

const char HOOK_PROGRAM[] = ".git/hooks/prepare-commit-msg";

struct Diff {
    vector<string> additions;
    vector<string> deletions;
};

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

string normalize_space(string const& line)
{
    string::const_iterator begin = line.begin();
    string::const_iterator end = line.end();
    while (begin < end && isspace(*begin))
        ++begin;

    string result;
    bool in_space = false;
    for (; begin < end; ++begin) {
        if (isspace(*begin)) {
            in_space = true;
            continue;
        } else {
            if (in_space) {
                result += ' ';
                in_space = false;
            }

            result += *begin;
        }
    }

    return result;
}

Diff read_diff()
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
    Diff result;
    while (getline(in, line)) {
        if (line.empty())
            continue;

        switch (line[0]) {
        case '+':
            result.additions.push_back(normalize_space(line.substr(1)));
            break;
        case '-':
            result.deletions.push_back(normalize_space(line.substr(1)));
            break;
        }
    }

    return result;
}

string best_added_test_name(Diff const& diff)
{
    vector<string> deleted_tests;
    for (vector<string>::const_iterator it = diff.deletions.begin();
         it != diff.deletions.end();
         ++it)
    {
        string name = test_name(*it);
        if (name != "")
            deleted_tests.push_back(name);
    }

    string best_name = "";
    int highest_distance = -1;

    for (vector<string>::const_iterator it = diff.additions.begin();
         it != diff.additions.end();
         ++it)
    {
        string name = test_name(*it);
        if (name.empty())
            continue;

        int lowest = 99999999;
        for (vector<string>::const_iterator dptr = deleted_tests.begin();
             dptr != deleted_tests.end();
             ++dptr)
        {
            int d = edit_distance(name, *dptr);
            if (d < lowest)
                lowest = d;
        }

        if (lowest > highest_distance) {
            best_name = name;
            highest_distance = lowest;
        }
    }

    return best_name;
}

void prepare()
{
    Diff diff = read_diff();

    cerr << best_added_test_name(diff) << endl;
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
