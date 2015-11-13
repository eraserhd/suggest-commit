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
	int in_header = 0;

	while (NULL != fgets(line, sizeof(line), stdin)) {
		if (in_header) {
			if (line[0] == '+')
				in_header = 0;
			continue;
		}
		if (line[0] == 'd') {
			in_header = 1;
			continue;
		}

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
    void (* name_formatter) (char *name);
    regex_t compiled;
} test_pattern_t;

static void format_string_name(char *name)
{
	char *src;
	char *dst;

	for (src = dst = name; *src; ++src) {
		if (*src == '\\') {
			++src;
			if (*src == '\0')
				break;
		}

		*dst++ = *src;
	}
	*dst = '\0';
}

static void format_xunit_name(char *name)
{
	char *p;
	for (p = name; *p; ++p)
		if (*p == '_')
			*p = ' ';
}

test_pattern_t test_patterns[] = {
	{ "\\s*\\((?:fact|property)\\s*\"(.*)\"\\s*",
          format_string_name
	},
	{ "\\s*void\\s+test_?([A-Za-z0-9_]+)\\s*\\(\\s*\\)\\s*\\{?\\s*",
	  format_xunit_name
	},
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
	    p->name_formatter(name);
            return 0;
        }
    }

    return -1;
}

/*****************************************************************************
 * Identifying the best test name
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

int suggest_test_name(char *message, size_t message_size)
{
	change_t *add;
	change_t *del;
	char add_name[256];
	char del_name[256];
	int distance;
	int smallest_del_distance;
	int least_likely_del_distance = -1;
	int found = 0;

	compile_test_patterns();
	for (add = changes; add; add = add->next) {
		if (add->type == DELETION)
			continue;
		if (0 != extract_test_name(add_name, sizeof(add_name), add->line)) 
			continue;

		smallest_del_distance = 9999999;
		for (del = changes; del; del = del->next) {
			if (del->type == ADDITION)
				continue;
			if (0 != extract_test_name(del_name, sizeof(del_name), del->line))
				continue;

			distance = edit_distance(add_name, del_name);
			if (distance < smallest_del_distance)
				smallest_del_distance = distance;
		}

		if (smallest_del_distance > least_likely_del_distance) {
			strncpy(message, add_name, message_size);
			least_likely_del_distance = smallest_del_distance;
			found = 1;
		}
	}

	free_test_patterns();
	return found ? 0 : -1;
}

/*****************************************************************************
 * Detecting renames
 */

typedef struct token_tag {
	struct token_tag *next;
	char token[1];
} token_t;

const char WORD_CHARS[] =
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"0123456789_-?!";

int iswordchar(char c)
{
	return !!strchr(WORD_CHARS, c);
}

void add_token(token_t** tokens, char *text)
{
	token_t *t = malloc(sizeof(token_t) + strlen(text));
	strcpy(t->token, text);
	t->next = *tokens;
	*tokens = t;
}

token_t *tokenize(type_t type)
{
	token_t *result = NULL;
	change_t *change;
	char single_char_token[2] = {'\0', '\0'};
	char *start;
	char *token;
	size_t length;

	for (change = changes; change; change = change->next) {
		if (change->type != type)
			continue;

		start = change->line;

		while (*start) {
			while (*start && isspace(*start))
				++start;
			if (!*start)
				break;

			length = strspn(start, WORD_CHARS);
			if (length == 0) {
				single_char_token[0] = *start;
				add_token(&result, single_char_token);
				++start;
			} else {
				token = (char *)malloc(length+1);
				strncpy(token, start, length);
				token[length] = '\0';
				add_token(&result, token);
				free(token);
				start += length;
			}
		}

	}

	return result;
}

void free_tokens(token_t *tokens)
{
	token_t *tmp;

	while (tokens) {
		tmp = tokens->next;
		free(tokens);
		tokens = tmp;
	}
}

int suggest_rename(char *message, size_t message_size)
{
	token_t *addition_tokens = tokenize(ADDITION);
	token_t *deletion_tokens = tokenize(DELETION);
	token_t *a;
	token_t *d;
	char *found_a;
	char *found_d;
	int found = 0;
	int consistent;

	a = addition_tokens;
	d = deletion_tokens;
	while (a && d) {
		consistent = 0;
		if (found) {
			consistent = !strcmp(a->token, found_a) &&
				     !strcmp(d->token, found_d);
		}

		if (strcmp(a->token, d->token) && !consistent) {

			if (!found) {
				found = 1;
				found_a = a->token;
				found_d = d->token;
				snprintf(message,
					 message_size,
					 "Rename `%s` to `%s`",
					 d->token,
					 a->token);
			} else {
				found = 0;
				goto fail;
			}
		}

		a = a->next;
		d = d->next;
	}

	if (a || d)
		found = 0;

fail:
	free_tokens(addition_tokens);
	free_tokens(deletion_tokens);
	return found ? 0 : -1;
}

/*****************************************************************************
 * Main!
 */

int suggest(char *message, size_t message_size)
{
	if (0 == suggest_rename(message, message_size))
		return 0;
	return suggest_test_name(message, message_size);
}

int main(int argc, char *argv[])
{
    char message[256];

    parse_diff();
    if (0 == suggest(message, sizeof(message)))
        printf("%s\n", message);
    free_diff();
    return 0;
}
