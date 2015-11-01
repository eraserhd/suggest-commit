#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include "suggest.hh"

struct PosixSystemTraits {
    std::string file_contents(std::string const& filename) {
        std::ifstream in(path.c_str());
        std::istreambuf_iterator<char> begin(in);
        std::istreambuf_iterator<char> end;

        std::string contents;
        copy(begin, end, back_inserter(contents));
        return contents; 
    }

    std::string diff_contents() {
        std::string path = temporary_filename();
        std::string cmd = std::string("git diff -b --cached > ") + path;
        if (system(cmd.c_str()) != 0)
            throw std::runtime_error("git diff command failed");

        return file_contents(path);
    }

    void write_message(std::string const& filename, std::string const& message) {
        std::ofstream out(filename.c_str());
        out << message << std::flush;
        out.close();
    }

private:
    std::string temporary_filename() {
        char path[64] = "/tmp/suggest.XXXXXX";
        if (NULL == mktemp(path))
            throw std::runtime_error("mktemp failed");

        return path;
    }
};

int main(int argc, char *argv[])
{
    return CommitSuggester<PosixSystemTraits>().main(argc, argv);
}
