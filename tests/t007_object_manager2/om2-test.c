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

