

#include <stdlib.h>
#include <check.h>


START_TEST(test_foo)
{
    fail_unless( 0 == 0, "mega error");
}
END_TEST

Suite
*test_suite (void)
{
    Suite *s = suite_create ("Test Name");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_foo);
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

