#include <cxxtest/TestSuite.h>
#include "suggest.h"

class t_tests : public CxxTest::TestSuite {
public:
    void test_Detects_midje_facts()
    {
        TS_ASSERT_EQUALS("midje is awesome!", test_name("\t (fact\"midje is awesome!\" "));
    }

    void test_Detects_cxxtest_facts() {
        TS_ASSERT_EQUALS("Detects cxxtest facts", test_name("void te" "st_Detects_cxxtest_facts()"));
    }

};
