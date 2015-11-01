#include <cxxtest/TestSuite.h>
#include "suggest.h"

class t_edit_distance : public CxxTest::TestSuite
{
public:
    void test_Equal_strings_have_zero_distance() {
        TS_ASSERT_EQUALS(0, edit_distance("foo", "foo"));
        TS_ASSERT_EQUALS(0, edit_distance("", ""));
    }
};
