#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex.h>
#include <string>
#include <utility>
#include <vector>

/*****************************************************************************
 * Parsing and representation of diffs
 */

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

/*****************************************************************************
 * Finding and extracting test names
 */

struct test_pattern_tag {
    const char* pattern;
    regex_t compiled;
} test_patterns[] = {
    { "\\s*\\(fact\\s*\"(.*)\"\\s*" },
    { "\\s*void\\s+test_?([A-Za-z0-9_]+)\\s*\\(\\s*\\)\\s*\\{?\\s*" },
};

void compile_test_patterns()
{
    int i;
    for (i = 0; i < sizeof(test_patterns)/sizeof(test_patterns[0]); ++i) {
        if (regcomp(&test_patterns[i].compiled, test_patterns[i].pattern, REG_EXTENDED|REG_ENHANCED)) {
            fprintf(stderr, "unable to compile regex `%s'\n", test_patterns[i].pattern);
            exit(1);
        }
    }
}

void free_test_patterns()
{
    int i;
    for (i = 0; i < sizeof(test_patterns)/sizeof(test_patterns[0]); ++i)
        regfree(&test_patterns[i].compiled);
}

std::string test_name(std::string const& line)
{
    int i, rc;
    regmatch_t matches[2];

    for (i = 0; i < sizeof(test_patterns)/sizeof(test_patterns[0]); ++i) {
        rc = regexec(&test_patterns[i].compiled, line.c_str(), 2, matches, 0);
        if (0 == rc) {
            return line.substr(matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
        }
    }

    return "";
}


int edit_distance(const char* a, const char* b)
{
    int alen = strlen(a);
    int blen = strlen(b);
    int *dp = (int *)malloc(sizeof(int) * (alen + 1) * (blen + 1));
    int result, tmp;

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

std::string suggest()
{
    compile_test_patterns();
    for (change_t *it = changes; it != NULL; it = it->next) {
        if (it->type == DELETION)
            continue;

        std::string name = test_name(it->line);
        if (name != "")
            return name;
    }

    free_test_patterns();
    return "";
}

int main(int argc, char *argv[])
{
    parse_diff();
    std::cout << suggest() << std::endl;
    free_diff();
    return 0;
}
