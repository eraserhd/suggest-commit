#include <cxxtest/TestSuite.h>
#include "suggest.hh"
#include <string>

struct TestSystem {
    typedef std::string::const_iterator diff_iterator_type;

    std::string test_diff;
    std::string test_message;

    TestSystem()
        : test_message("\n# Here are some instructions\n# For how to commit...\n")
    {}

    struct diff_iterators_type {
        diff_iterator_type begin;
        diff_iterator_type end;

        diff_iterators_type(diff_iterator_type const& begin,
                            diff_iterator_type const& end)
            : begin(begin)
            , end(end)
        {}
    };

    diff_iterators_type message_iterators(std::string const& filename) {
        return diff_iterators_type(test_message.begin(), test_message.end());
    }

    std::string diff_contents() {
        return test_diff;
    }

    void write_message(std::string const& filename, std::string const& message) {
        test_message = message;
    }
};

const char SIMPLE_CXXTEST_ADDITION[] =
"diff --git a/t_distance.h b/t_distance.h\n"
"new file mode 100644\n"
"index 0000000..7e4b821\n"
"--- /dev/null\n"
"+++ b/t_distance.h\n"
"@@ -0,0 +1,9 @@\n"
"+#include <cxxtest/TestSuite.h>\n"
"+\n"
"+class Test_distance : public Cx" "xTest::TestSuite\n"
"+{\n"
"+public:\n"
"+    void te" "st_foo() {\n"
"+        TS_ASSERT(true);\n"
"+    }\n"
"+};\n";

struct Test {
    Test& with_diff(std::string const& diff) {
        suggester.test_diff = diff;
        return *this;
    }

    std::string message() {
        run();
        return suggester.test_message;
    }

    int result_code() {
        run();
        return main_result;
    }

private:
    CommitSuggester<TestSystem> suggester;
    int main_result;

    void run() {
        char arg1[] = "prepare-commit-msg";
        char arg2[] = ".git/foo";
        char *argv[] = {arg1, arg2};
        main_result = suggester.main(2, argv);
    }
};

class t_integration : public CxxTest::TestSuite {
public:

    void test_Has_non_zero_exit() {
        TS_ASSERT_EQUALS(0, Test().with_diff(SIMPLE_CXXTEST_ADDITION).result_code());
    }

    void test_Suggests_obvious_cxxtest_fact() {
        TS_ASSERT_EQUALS("foo\n#", Test().with_diff(SIMPLE_CXXTEST_ADDITION).message().substr(0, 5));
    }
};

