

#include <stdlib.h>
#include <check.h>

#include "libgencds/list.h"


START_TEST(test_list_append)
{
    list_t *list = list_new();
    
    fail_unless(list_first(list) == NULL,
                "First of empty list should be NULL");
    fail_unless(list_last(list) == NULL, "Last of empty list should be NULL");
    
    list_append(list, (void*)1);

    fail_unless(list_entry_data(list_first(list)) == (void*)1,
        "Data in first of list should be 1");
    fail_unless(list_entry_data(list_last(list)) == (void*)1,
        "Data in last of list should be 1");

    list_append(list, (void*)2);

    fail_unless(list_entry_data(list_first(list)) == (void*)1,
        "Data in first of list should be 1");
    fail_unless(list_entry_data(list_last(list)) == (void*)2,
        "Data in last of list should be 2");

    list_delete(list);
}
END_TEST

START_TEST(test_list_insert)
{
    list_t *list = list_new();
    
    fail_unless(list_first(list) == NULL,
                "First of empty list should be NULL");
    fail_unless(list_last(list) == NULL, "Last of empty list should be NULL");
    
    list_insert(list, (void*)1);

    void *adr = list_entry_data(list_first(list));
    fail_unless(adr == (void*)1,
        "Data in first of list should be 1 (value is %x)",
        (unsigned)adr);
    
    adr = list_entry_data(list_last(list));
    fail_unless(adr == (void*)1,
        "Data in last of list should be 1 (value is %x)",
        (unsigned)adr);

    list_insert(list, (void*)2);

    adr = list_entry_data(list_first(list));
    fail_unless(adr == (void*)2,
        "List insertion failed, first of list should be 2 (value is %x)",
        (unsigned)adr);
    adr = list_entry_data(list_last(list));
    fail_unless(adr == (void*)1,
        "List insertion failed, last of list should be 1 (value is %x)",
        (unsigned)adr);

    list_delete(list);
}
END_TEST


START_TEST(test_list_remove_entry)
{
    list_t *list = list_new();
    
    list_insert(list, (void*)1);
    
    list_remove_entry(list, list_first(list));
    
    fail_unless(list_first(list) == NULL,
                "The first and only entry in the list was removed, "
                "but it is not null");

    fail_unless(list_last(list) == NULL,
                "The last and only entry in the list was removed, "
                "but it is not null");
    

    list_delete(list);
}
END_TEST


// For testing the apply function
void
apply_fn(int *n, int m)
{
    (*n) += m;
}

#define ARRAY_LEN 16
START_TEST(test_list_apply)
{
    int test_array[ARRAY_LEN];

    list_t *list = list_new();
    
    for (int i = 0 ; i < ARRAY_LEN ; i ++) {
        test_array[i] = i;
        list_append(list, &test_array[i]);
    }
    
    LIST_APPLY(list, apply_fn, 10);
    
    for (int i = 0 ; i < ARRAY_LEN ; i ++) {
        fail_unless(test_array[i] == (i + 10),
            "LIST_APPLY did not produce expected results for array[%d] == %d"
            " but should be %d", i, test_array[i], i + 10);
    }
    
    list_delete(list);
}
END_TEST

START_TEST(test_list_rank)
{
    int test_array[ARRAY_LEN];

    list_t *list = list_new();
    
    for (int i = 0 ; i < ARRAY_LEN ; i ++) {
        test_array[i] = i;
        list_append(list, &test_array[i]);
    }
    
    
    for (int i = 0 ; i < ARRAY_LEN ; i ++) {
        list_entry_t *e = list_rank(list, i);
        int *data = list_entry_data(e);
        fail_unless( *data == i,
            "The result data accessible through list_rank is erroneous"
            " rank no %d, *data == %d (should be %d)",
            i, *data, i);
    }
    
    list_delete(list);
}
END_TEST


Suite
*test_suite (void)
{
    Suite *s = suite_create ("List Test");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_list_append);
    tcase_add_test(tc_core, test_list_insert);
    tcase_add_test(tc_core, test_list_apply);
    tcase_add_test(tc_core, test_list_rank);
    tcase_add_test(tc_core, test_list_remove_entry);
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

