#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
 * Utilities
 */

int min(int a, int b)
{
    return (a < b) ? a : b;
}

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

typedef struct test_pattern_tag {
    const char* pattern;
    regex_t compiled;
} test_pattern_t;

test_pattern_t test_patterns[] = {
    { "\\s*\\(fact\\s*\"(.*)\"\\s*" },
    { "\\s*void\\s+test_?([A-Za-z0-9_]+)\\s*\\(\\s*\\)\\s*\\{?\\s*" },
    { NULL }
};

void compile_test_patterns()
{
    test_pattern_t *p;
    for (p = test_patterns; p->pattern != NULL; ++p) {
        if (regcomp(&p->compiled, p->pattern, REG_EXTENDED|REG_ENHANCED)) {
            fprintf(stderr, "unable to compile regex `%s'\n", p->pattern);
            exit(1);
        }
    }
}

void free_test_patterns()
{
    test_pattern_t *p;
    for (p = test_patterns; p->pattern != NULL; ++p)
        regfree(&p->compiled);
}

int extract_test_name(char* name, size_t name_size, char const* line)
{
    int rc, length;
    regmatch_t matches[2];
    test_pattern_t *p;

    for (p = test_patterns; p->pattern != NULL; ++p) {
        rc = regexec(&p->compiled, line, 2, matches, 0);
        if (0 == rc) {
            length = min(name_size-1, matches[1].rm_eo - matches[1].rm_so);
            strncpy(name, line + matches[1].rm_so, length);
            name[length] = '\0';
            return 0;
        }
    }

    return -1;
}

/*****************************************************************************
 */

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
        for (int j = 1; j <= blen; ++j)
            CELL(i,j) =
                min(
                    min(
                        CELL(i-1,j) + 1,
                        CELL(i,j-1) + 1
                    ),
                    CELL(i-1,j-1) + (a[i-1] != b[j-1])
                   );

    result = CELL(alen, blen);
#undef CELL

    free(dp);
    return result;
}

int suggest(char *message, size_t message_size)
{
    compile_test_patterns();
    for (change_t *it = changes; it != NULL; it = it->next) {
        if (it->type == DELETION)
            continue;

        if (0 == extract_test_name(message, message_size, it->line)) 
            return 0;
    }

    free_test_patterns();
    return -1;
}

/*****************************************************************************
 * Main!
 */

int main(int argc, char *argv[])
{
    char message[256];

    parse_diff();
    if (0 == suggest(message, sizeof(message)))
        printf("%s\n", message);
    free_diff();
    return 0;
}
