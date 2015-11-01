#include <cxxtest/TestSuite.h>
#include "suggest.hh"

class t_edit_distance : public CxxTest::TestSuite
{
public:
    void test_Equal_strings_have_no_distance() {
        TS_ASSERT_EQUALS(0, edit_distance("foo", "foo"));
        TS_ASSERT_EQUALS(0, edit_distance("", ""));
    }

    void test_Scores_deletions_as_1() {
        TS_ASSERT_EQUALS(1, edit_distance("foo", "fo"));
    }

    void test_Scores_additions_as_1() {
        TS_ASSERT_EQUALS(1, edit_distance("foo", "foxo"));
    }

    void test_Scores_changes_as_1() {
        TS_ASSERT_EQUALS(1, edit_distance("foo", "fxo"));
    }
};
