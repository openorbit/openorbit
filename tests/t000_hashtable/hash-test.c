/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */

#include "libgencds/hashtable.h"
#include "libgencds/hashtable-private.h"


#include <stdlib.h>
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
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

