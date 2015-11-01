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

void add_line(std::string const& line) {
    if (line.empty())
        return;

    switch (line[0]) {
    case '+':
        additions.push_back(line.substr(1));
        break;
    case '-':
        deletions.push_back(line.substr(1));
        break;
    }
}

template<class IteratorT>
static void parse(IteratorT begin, IteratorT end) {
    std::string line;

    for (; begin != end; ++begin) {
        if (*begin == '\n') {
            add_line(line);
            line = "";
        } else
            line += *begin;
    }
    if (!line.empty())
        add_line(line);
}

int edit_distance(std::string const& a, std::string const& b)
{
    std::vector<std::vector<int> > dp(a.size()+1, std::vector<int>(b.size()+1, 0));

    for (int i = 0; i <= a.size(); ++i)
        dp[i][0] = i;
    for (int j = 0; j <= b.size(); ++j)
        dp[0][j] = j;
    for (int i = 1; i <= a.size(); ++i)
        for (int j = 1; j <= b.size(); ++j)
            dp[i][j] = std::min(
                    std::min(
                            dp[i-1][j] + 1,
                            dp[i][j-1] + 1
                        ),
                        dp[i-1][j-1] + (a[i-1] != b[j-1])
                    );

    return dp[a.size()][b.size()];
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

std::string suggest()
{
    return best_added_test_name();
}

template<class IteratorT>
std::string suggest(IteratorT begin, IteratorT end)
{
    parse(begin, end);
    return suggest();
}

int main(int argc, char *argv[])
{
    std::istreambuf_iterator<char> begin(std::cin);
    std::istreambuf_iterator<char> end;
    std::cout << suggest(begin, end) << std::endl;
    return 0;
}
