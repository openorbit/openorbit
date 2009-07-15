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
#include <stdio.h>

#include <check.h>
#include <gencds/object-manager2.h>

START_TEST(test_tree_insert)
{
  OMtree *tree = omTreeNew();
  omTreeInsert2(tree, 5, omObjDescNew("obj0", NULL, (void*)0x80000000));
  omTreeInsert2(tree, 3, omObjDescNew("obj1", NULL, (void*)0x70000001));
  omTreeInsert2(tree, 6, omObjDescNew("obj2", NULL, (void*)0x70000002));
  omTreeInsert2(tree, 20, omObjDescNew("obj3", NULL, (void*)0x70000003));
  omTreeInsert2(tree, 4, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 9, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 10, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 11, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 12, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 13, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 14, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 15, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 16, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 17, omObjDescNew("obj3", NULL, (void*)0x60000004));
  omTreeInsert2(tree, 18, omObjDescNew("obj3", NULL, (void*)0x60000004));
  
  FILE *file = fopen("/Users/holm/Desktop/tree.dot", "w");
  omDbgDumpTree(file, tree);
  fclose(file);
}
END_TEST

Suite
*test_suite (void)
{
    Suite *s = suite_create ("Object Manager2");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_tree_insert);
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

