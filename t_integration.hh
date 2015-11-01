#include <cxxtest/TestSuite.h>
#include "suggest.hh"
#include <string>

std::string test_diff;

struct TestSystem {
    typedef std::string::const_iterator diff_iterator_type;

    struct diff_iterators_type {
        diff_iterator_type begin;
        diff_iterator_type end;

        diff_iterators_type(diff_iterator_type const& begin,
                            diff_iterator_type const& end)
            : begin(begin)
            , end(end)
        {}
    };

    static diff_iterators_type diff_iterators() {
        return diff_iterators_type(test_diff.begin(), test_diff.end());
    }
};

class t_integration : public CxxTest::TestSuite {
public:

    void test_Has_non_zero_exit() {
        test_diff =
"diff --git a/t_distance.h b/t_distance.h\n"
"new file mode 100644\n"
"index 0000000..7e4b821\n"
"--- /dev/null\n"
"+++ b/t_distance.h\n"
"@@ -0,0 +1,9 @@\n"
"+#include <cxxtest/TestSuite.h>\n"
"+\n"
"+class Test_distance : public CxxTest::TestSuite\n"
"+{\n"
"+public:\n"
"+    void te" "st_foo() {\n"
"+        TS_ASSERT(true);\n"
"+    }\n"
"+};\n";
        char *argv[] = {"prepare-commit-msg"};
        const int rc = CommitSuggester<TestSystem>::main(1, argv);
        TS_ASSERT_EQUALS(0, rc);
    }

};

