
    void test_Detects_midje_facts() {
        TS_ASSERT_EQUALS("midje is awesome!", test_name("\t (fact\"midje is awesome!\" "));
    }