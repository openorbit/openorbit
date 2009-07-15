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


#include <stdlib.h>
#include <check.h>

#include <gencds/heap.h>


typedef struct {
    unsigned int n;
} element_type;

uint64_t
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

