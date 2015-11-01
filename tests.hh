#include <cxxtest/TestSuite.h>
#include "suggest.hh"
#include <string>

class t_integration : public CxxTest::TestSuite {
public:
    void test_Suggests_obvious_cxxtest_fact() {
        TS_ASSERT_EQUALS("foo", suggest("\n+   void te" "st_foo() {\n+ adfaf"));
    }

    void test_Detects_midje_facts() {
        TS_ASSERT_EQUALS("midje is awesome!", suggest("+\t (fact\"midje is awesome!\" "));
    }
};

