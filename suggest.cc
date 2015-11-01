#include "suggest.hh"

struct PosixSystemTraits {
};

int main(int argc, char *argv[])
{
    return CommitSuggester<PosixSystemTraits>::main(argc, argv);
}
