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


#include <stdlib.h>
#include <check.h>

#include <gencds/heap.h>


typedef struct {
    unsigned int n;
} element_type;

unsigned int
f(void *p)
{
    element_type *ep = p;
    
    return ep->n;
}


START_TEST(test_heap)
{
    heap_t *h = heap_new(9, f);
    element_type elements[512];
    
    for (int i = 0 ; i < 512 ; i ++) {
        elements[i].n = (i & 1) ? i : i + 250;
    }
    
    for (int i = 0 ; i < 511 ; i ++) {
        fail_unless(heap_insert(h, &elements[i]), "Insertion failed, but should not");
    }
    
    fail_if(heap_insert(h, &elements[511]),
            "Insertion should fail on the last element last elem = %d, size = %d",
            h->last, h->size);
    
    element_type *previous = heap_remove(h);
    element_type *current;
    int counter = 1;
    while (current = heap_remove(h)) {
        counter ++;
        fail_unless(previous->n <= current->n, "Heap order is not satisfied");
    }
    
    fail_unless(counter, 511, "We should be able to remove 511 elements.");
}
END_TEST

Suite
*test_suite (int argc, char **argv)
{
    Suite *s = suite_create ("Test Name");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_heap);
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

