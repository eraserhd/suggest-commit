#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include "suggest.hh"

int main(int argc, char *argv[])
{
    std::istreambuf_iterator begin(cin);
    std::istreambuf_iterator end;

    std::string suggestion = CommitSuggester().suggest(begin, end);
    std::cout << suggestion << std::endl;
    return 0;
}
