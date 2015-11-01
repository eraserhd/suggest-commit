#include <cxxtest/TestSuite.h>
#include "suggest.hh"

class t_tests : public CxxTest::TestSuite {
public:
    void test_Detects_midje_facts()
    {
        TS_ASSERT_EQUALS("midje is awesome!", test_name("\t (fact\"midje is awesome!\" "));
    }
};
