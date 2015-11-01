#include <cxxtest/TestSuite.h>
#include "suggest.hh"
#include <string>

const std::string SIMPLE_CXXTEST_ADDITION =
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

class t_integration : public CxxTest::TestSuite {
public:
    void test_Suggests_obvious_cxxtest_fact() {
        TS_ASSERT_EQUALS("foo", suggest(SIMPLE_CXXTEST_ADDITION));
    }

    void test_Detects_midje_facts() {
        TS_ASSERT_EQUALS("midje is awesome!", test_name("\t (fact\"midje is awesome!\" "));
    }
};

