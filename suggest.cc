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

typedef enum type_tag {
    DELETION = -1,
    ADDITION = 1
} type_t;

typedef struct change_tag {
    struct change_tag * next;
    type_t              type;
    char                line[1];
} change_t;

static change_t *changes = NULL;
static change_t **next_change = &changes;

void parse_diff()
{
    char line[256];
    char *nl;

    while (NULL != fgets(line, sizeof(line), stdin)) {
        if (line[0] != '+' && line[0] != '-')
            continue;

        if ((nl = strpbrk(line, "\r\n")))
            *nl = '\0';

        *next_change = (change_t *)malloc(sizeof(change_t) + strlen(line));
        (*next_change)->next = NULL;
        (*next_change)->type = (line[0] == '+') ? ADDITION : DELETION;
        strcpy((*next_change)->line, line+1);

        next_change = &(*next_change)->next;
    }
}

void free_diff()
{
    change_t *p;

    while (changes != NULL) {
        p = changes;
        changes = changes->next;
        free(p);
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
    for (change_t *it = changes; it != NULL; it = it->next) {
        if (it->type == DELETION)
            continue;

        std::string name = test_name(it->line);
        if (name != "")
            return name;
    }

    return "";
}

std::string suggest()
{
    return best_added_test_name();
}

int main(int argc, char *argv[])
{
    parse_diff();
    std::cout << suggest() << std::endl;
    free_diff();
    return 0;
}
