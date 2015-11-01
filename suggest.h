#ifndef suggest_h_INCLUDED
#define suggest_h_INCLUDED

#include <algorithm>
#include <string>
#include <vector>
#include <regex>

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
    static const std::regex cxxtest_fact("\\s*void\\s+test_?([A-Za-z0-9_]+)\\s*\\(\\s*\\)\\s*");

    std::smatch matches;
    if (std::regex_match(line, matches, midje_fact))
        return matches.str(1);
    if (std::regex_match(line, matches, cxxtest_fact))
        return decode_JUnit_style_name(matches.str(1));

    return "";
}

#endif
