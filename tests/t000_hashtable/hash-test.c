/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gencds/hashtable.h>
#include "libgencds/hashtable-private.h"


#include <stdlib.h>
#include <string.h>
#include <check.h>



unsigned int
collision_hash(const char *key)
{
    return 0;
}



// tests that the hashing function isn't overly bad
START_TEST(test_hash_func)
{
    int a = hashtable_string_hash("foo") & 511;
    int b = hashtable_string_hash("bar") & 511;
    int c = hashtable_string_hash("foobar") & 511;
    int d = hashtable_string_hash("rab") & 511;
    int e = hashtable_string_hash("oof") & 511;
    int f = hashtable_string_hash("raboof") & 511;
    
    fail_unless(a != e, "foo == oof");
    fail_unless(b != d, "bar == rab");
    fail_unless(c != f, "foobar == raboof)");
    fail_unless(a != c, "foo == foobar");
    fail_unless(a != b, "foo == bar");
}
END_TEST

START_TEST(test_collisions)
{
    hashtable_t *ht = hashtable_new(collision_hash, hashtable_string_comp,
        hashtable_string_key_copy, hashtable_string_key_del,
        512);
    
    hashtable_insert(ht,"foo",NULL);
    hashtable_insert(ht,"bar",NULL);
    hashtable_insert(ht,"foobar",NULL);
    hashtable_insert(ht,"rabfoo",NULL);
    hashtable_insert(ht,"rab",NULL);
    hashtable_insert(ht,"oof",NULL);
    
    hashentry_t *entry = ht->t[0];
    int entry_count = 0;
    
    while (entry) {
        entry_count ++;
        entry = entry->next;
    }
    
    fail_unless(entry_count == 6, "6 entries expected");

    hashtable_delete(ht);
}
END_TEST

START_TEST(test_insert_and_lookup)
{
    hashtable_t *ht = hashtable_new_with_str_keys(512);

    hashtable_insert(ht, "foo", (void*)6);
    hashtable_insert(ht, "bar", (void*)1);
    hashtable_insert(ht, "foobar", (void*)2);
    hashtable_insert(ht, "rabfoo", (void*)3);
    hashtable_insert(ht, "rab", (void*)4);
    hashtable_insert(ht, "oof", (void*)5);
    
    fail_unless( hashtable_lookup(ht, "foobar") == (void*)2,
        "foobar not found");
    fail_unless( hashtable_lookup(ht, "bar") == (void*)1, "bar not found");
    fail_unless( hashtable_lookup(ht, "foo") == (void*)6, "foo not found");
    fail_unless( hashtable_lookup(ht, "oof") == (void*)5, "oof not found");
    fail_unless( hashtable_lookup(ht, "rab") == (void*)4, "rab not found");
    fail_unless( hashtable_lookup(ht, "rabfoo") == (void*)3,
        "rabfoo not found");
    fail_unless( hashtable_lookup(ht, "random string") == (void*)0,
        "whoa, something is not right here");
    
    
    hashtable_delete(ht);
}
END_TEST

START_TEST(test_insert_and_remove)
{
    hashtable_t *ht = hashtable_new_with_str_keys(512);
    
    fail_unless( hashtable_lookup(ht, "key0") == (void*)0, "ahrg!!!");
    hashtable_insert(ht, "key0", (void*)42);
    fail_unless( hashtable_lookup(ht, "key0") == (void*)42,
        "meaning of life is not 42");

    hashtable_insert(ht, "key1", (void*)24);
    fail_unless( hashtable_lookup(ht, "key1") == (void*)24,
        "dont feel like writing error msgs now!!!");
    
    fail_unless( hashtable_remove(ht, "key0") == (void*)42,
        "when removing key0 the meaning of life should be releaved");
    
    fail_unless( hashtable_lookup(ht, "key0") == (void*)0,
        "key was not removed by remove");
    fail_unless( hashtable_lookup(ht, "key1") == (void*)24,
        "ooups, managed to delete key1 when deleting key0");

    
    hashtable_delete(ht);
}
END_TEST


START_TEST(test_list_duality)
{
    hashtable_t *ht = hashtable_new_with_str_keys(512);
    
    hashtable_insert(ht, "key0", (void*)42);
    list_entry_t* entry = hashtable_first(ht);
    void *key = hashtable_entry_key(entry);
    void *data = hashtable_entry_data(entry);
    fail_unless(strcmp((char*)key, "key0") == 0,
                "The first entry's key is not the one inserted");
    fail_unless(data == (void*)42,
                "The first entry's data is not the data inserted");
    
    // insert a new entry and remove the old one
    hashtable_insert(ht, "key1", (void*)43);
    hashtable_entry_remove(ht, entry);
    entry = hashtable_first(ht);
    key = hashtable_entry_key(entry);
    data = hashtable_entry_data(entry);
    fail_unless(strcmp((char*)key, "key1") == 0,
                "The first entry's key is not the one inserted for key1");
    fail_unless(data == (void*)43,
                "The first entry's data is not the data inserted for key1");
    
    list_entry_t *looked_up_entry = hashtable_lookup_list_entry(ht, "key1");
    fail_unless(looked_up_entry == entry,
                "Tried to look up entry for key1, but returned entry is not the expected entry");
    
    void *old_deleted_entry = hashtable_lookup(ht, "key0");
    fail_unless(old_deleted_entry == NULL,
                "deleted entry successfully looked up (something rotten here)");
    
    
    
    hashtable_delete(ht);
}
END_TEST

Suite
*test_suite (int argc, char **argv)
{
    Suite *s = suite_create ("Hashtable");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test(tc_core, test_insert_and_lookup);
    tcase_add_test(tc_core, test_collisions);
    tcase_add_test(tc_core, test_hash_func);
    tcase_add_test(tc_core, test_insert_and_remove);
    tcase_add_test(tc_core, test_list_duality);
    suite_add_tcase(s, tc_core);
    
    return s;
}

