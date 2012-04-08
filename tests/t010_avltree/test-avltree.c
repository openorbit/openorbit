
#include <gencds/avl-tree.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <check.h>

START_TEST(test_str_tree)
{
  avl_tree_t *tree = avl_str_new(); 
  avl_insert(tree, "hello", (void*)1234);
  avl_insert(tree, "foo", (void*)12345);
  avl_insert(tree, "bar", (void*)123456);
  avl_insert(tree, "foobaz", (void*)1234567);
  avl_insert(tree, "foobar", (void*)12345678);
 
  fail_unless(avl_find(tree, "hello") == (void*)1234,
              "could not find obj 'hello'");
  fail_unless(avl_find(tree, "foo") == (void*)12345,
              "could not find obj 'foo'");
  fail_unless(avl_find(tree, "bar") == (void*)123456,
              "could not find obj 'bar'");
  fail_unless(avl_find(tree, "foobaz") == (void*)1234567,
              "could not find obj 'foobaz'");
  fail_unless(avl_find(tree, "foobar") == (void*)12345678,
              "could not find obj 'foobar'");
  avl_delete(tree);
}
END_TEST

START_TEST(test_tree)
{
  avl_tree_t *tree = avl_new();
  
  avl_insert(tree, (void*)1, (void*)1234);
  avl_insert(tree, (void*)2, (void*)12345);
  avl_insert(tree, (void*)3, (void*)123456);
  avl_insert(tree, (void*)4, (void*)1234567);
  avl_insert(tree, (void*)5, (void*)12345678);
 
  fail_unless(avl_find(tree, (void*)1) == (void*)1234,
              "could not find obj 1");
  fail_unless(avl_find(tree, (void*)2) == (void*)12345,
              "could not find obj 2");
  fail_unless(avl_find(tree, (void*)3) == (void*)123456,
              "could not find obj 3");
  fail_unless(avl_find(tree, (void*)4) == (void*)1234567,
              "could not find obj 4");
  fail_unless(avl_find(tree, (void*)5) == (void*)12345678,
              "could not find obj 5");
  avl_delete(tree);
}
END_TEST

START_TEST(test_uuid_tree)
{
  avl_tree_t *tree = avl_uuid_new();

  uuid_t u1, u2, u3, u4, u5;
  uuid_generate(u1);
  uuid_generate(u2);
  uuid_generate(u3);
  uuid_generate(u4);
  uuid_generate(u5);

  avl_insert(tree, u1, (void*)1234);
  avl_insert(tree, u2, (void*)12345);
  avl_insert(tree, u3, (void*)123456);
  avl_insert(tree, u4, (void*)1234567);
  avl_insert(tree, u5, (void*)12345678);

  fail_unless(avl_find(tree, u1) == (void*)1234, "could not find u1");
  fail_unless(avl_find(tree, u2) == (void*)12345, "could not find u2");
  fail_unless(avl_find(tree, u3) == (void*)123456, "could not find u3");
  fail_unless(avl_find(tree, u4) == (void*)1234567, "could not find u4");
  fail_unless(avl_find(tree, u5) == (void*)12345678, "could not find u5");
  avl_delete(tree);
}
END_TEST


Suite
*test_suite (void)
{
  Suite *s = suite_create ("AVL Tree Test");
  
  /* Core test case */
  TCase *tc_core = tcase_create ("Create, insert and find");

  tcase_add_test(tc_core, test_tree);
  tcase_add_test(tc_core, test_uuid_tree);
  tcase_add_test(tc_core, test_str_tree);

  suite_add_tcase(s, tc_core);
  
  return s;
}
