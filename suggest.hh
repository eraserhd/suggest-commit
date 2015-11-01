#ifndef suggest_h_INCLUDED
#define suggest_h_INCLUDED

#include "config.hh"
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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

const char HOOK_PROGRAM[] = ".git/hooks/prepare-commit-msg";

struct Diff {
    std::vector<std::string> additions;
    std::vector<std::string> deletions;

    template<class IteratorT>
    static Diff parse(IteratorT begin, IteratorT end) {
        std::string line;
        Diff diff;

        for (; begin != end; ++begin) {
            if (*begin == '\n') {
                add_line(diff, line);
                line = "";
            } else
                line += *begin;
        }
        if (!line.empty())
            add_line(diff, line);

        return diff;
    }

private:
    static std::string normalize_space(std::string const& line)
    {
        std::string::const_iterator begin = line.begin();
        std::string::const_iterator end = line.end();
        while (begin < end && isspace(*begin))
            ++begin;

        std::string result;
        bool in_space = false;
        for (; begin < end; ++begin) {
            if (std::isspace(*begin)) {
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

    static void add_line(Diff& diff, std::string const& line) {
        if (line.empty())
            return;

        switch (line[0]) {
        case '+':
            diff.additions.push_back(normalize_space(line.substr(1)));
            break;
        case '-':
            diff.deletions.push_back(normalize_space(line.substr(1)));
            break;
        }
    }
};

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

template <class SystemTraits>
struct CommitSuggester : SystemTraits {
    typedef typename SystemTraits::diff_iterators_type diff_iterators_type;

    void install()
    {
        std::ifstream in(BINARY_PATH "/suggest-commits", std::ios::in | std::ios::binary);
        std::ofstream out(HOOK_PROGRAM, std::ios::out | std::ios::binary);

        char buf[BUFSIZ];
        do {
            in.read(buf, sizeof(buf));
            out.write(buf, in.gcount());
        } while (!in.eof() && !in.fail());

        chmod(HOOK_PROGRAM, 0755);
    }

    Diff read_diff()
    {
        std::string diff = SystemTraits::diff_contents();
        return Diff::parse(diff.begin(), diff.end());
    }

    std::string best_added_test_name(Diff const& diff)
    {
        std::vector<std::string> deleted_tests;
        for (std::vector<std::string>::const_iterator it = diff.deletions.begin();
             it != diff.deletions.end();
             ++it)
        {
            std::string name = test_name(*it);
            if (name != "")
                deleted_tests.push_back(name);
        }

        std::string best_name = "";
        int highest_distance = -1;

        for (std::vector<std::string>::const_iterator it = diff.additions.begin();
             it != diff.additions.end();
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

    std::string suggest(Diff const& diff)
    {
        return best_added_test_name(diff);
    }

    void prepare(std::string const& message_filename)
    {
        Diff diff = read_diff();
        std::string suggestion = suggest(diff);

        diff_iterators_type iterators = SystemTraits::message_iterators(message_filename);
        while (iterators.begin != iterators.end && *iterators.begin != '#')
            ++iterators.begin;

        std::string bottom;
        copy(iterators.begin, iterators.end, back_inserter(bottom));

        SystemTraits::write_message(message_filename, suggestion + "\n" + bottom);
    }

    int main(int argc, char *argv[])
    {
        if (argc == 1)
            install();
        else if (argc == 2)
            prepare(argv[1]);
        else {
            // "special" type, do nothing
        }
        return 0;
    }

};

#endif
