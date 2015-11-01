#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <utility>
#include <vector>

std::vector<std::string> additions;
std::vector<std::string> deletions;

void parse_diff()
{
    char line[256];

    while (NULL != fgets(line, sizeof(line), stdin)) {
        switch (line[0]) {
        case '+':
            additions.push_back(line+1);
            break;
        case '-':
            deletions.push_back(line+1);
            break;
        }
    }
}

int edit_distance(const char* a, const char* b)
{
    int alen = strlen(a);
    int blen = strlen(b);
    int *dp = (int *)malloc(sizeof(int) * (alen + 1) * (blen + 1));
    int result, tmp;

    memset(dp, 0, sizeof(int) * (alen + 1) * (blen + 1));

#define CELL(i,j) dp[((i)*(alen+1)+(j))]
    for (int i = 0; i <= alen; ++i)
        CELL(i,0) = i;
    for (int j = 0; j <= blen; ++j)
        CELL(0,j) = j;
    for (int i = 1; i <= alen; ++i)
        for (int j = 1; j <= blen; ++j) {
            CELL(i,j) = CELL(i-1,j) + 1;

            tmp = CELL(j,i-1) + 1;
            if (tmp < CELL(i,j))
                CELL(i,j) = tmp;

            tmp = CELL(i-1,j-1) + (a[i-1] != b[j-1]);
            if (tmp < CELL(i,j))
                CELL(i,j) = tmp;
        }

    result = CELL(alen, blen);
#undef CELL

    free(dp);
    return result;
}

std::string decode_JUnit_style_name(std::string const& name)
{
    std::string result(name);
    std::replace(result.begin(), result.end(), '_', ' ');
    return result;
}

std::string test_name(std::string const& line)
{
    static const std::regex midje_fact("\\s*\\(fact\\s*\"(.*)\"\\s*");
    static const std::regex cxxtest_fact("\\s*void\\s+test_?([A-Za-z0-9_]+)\\s*\\(\\s*\\)\\s*\\{?\\s*");

    std::smatch matches;
    if (std::regex_match(line, matches, midje_fact))
        return matches.str(1);
    if (std::regex_match(line, matches, cxxtest_fact))
        return decode_JUnit_style_name(matches.str(1));

    return "";
}

std::string best_added_test_name()
{
    std::vector<std::string> deleted_tests;
    for (std::vector<std::string>::const_iterator it = deletions.begin();
         it != deletions.end();
         ++it)
    {
        std::string name = test_name(*it);
        if (name != "")
            deleted_tests.push_back(name);
    }

    std::string best_name = "";
    int highest_distance = -1;

    for (std::vector<std::string>::const_iterator it = additions.begin();
         it != additions.end();
         ++it)
    {
        std::string name = test_name(*it);
        if (name.empty())
            continue;


        int lowest = 99999999;
        for (std::vector<std::string>::const_iterator dptr = deleted_tests.begin();
             dptr != deleted_tests.end();
             ++dptr)
        {
            int d = edit_distance(name.c_str(), dptr->c_str());
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

std::string suggest()
{
    return best_added_test_name();
}

int main(int argc, char *argv[])
{
    parse_diff();
    std::cout << suggest() << std::endl;
    return 0;
}
