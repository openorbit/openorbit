/* Main test program file used by all unit tests.
 * This will not deallocate the allocated space on termination
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

Suite *test_suite(int argc, char **argv);

int
main(int argc, char **argv)
{
    int number_failed;
    Suite *s = test_suite(argc, argv);
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_VERBOSE);
    
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
