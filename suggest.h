#ifndef suggest_h_INCLUDED
#define suggest_h_INCLUDED

#include <string>
#include <vector>

int distance(std::string const& a, std::string const& b)
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

#endif
