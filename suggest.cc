#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include "suggest.hh"

struct PosixSystemTraits {
    typedef std::istreambuf_iterator<char> diff_iterator_type;

    struct diff_iterators_type {
        diff_iterator_type begin;
        diff_iterator_type end;

        diff_iterators_type(std::shared_ptr<std::ifstream> const& input_stream)
            : input_stream(input_stream)
            , begin(*input_stream)
            , end()
        {}

    private:
        std::shared_ptr<std::ifstream> input_stream;
    };

    diff_iterators_type message_iterators(std::string const& filename) {
        std::shared_ptr<std::ifstream> input_stream(new std::ifstream(filename.c_str()));
        return diff_iterators_type(input_stream);
    }

    diff_iterators_type diff_iterators() {
        std::string path = temporary_filename();
        std::string cmd = std::string("git diff -b --cached > ") + path;
        if (system(cmd.c_str()) != 0)
            throw std::runtime_error("git diff command failed");
        return message_iterators(path);
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
