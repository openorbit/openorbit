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
#include <string.h>
#include <check.h>
#include "physics/dynamics.h"

START_TEST(test_apply_force_at_pos)
{
    ph_obj_t obj;
    memset(&obj, 0, sizeof(ph_obj_t));
    
    vector_t pos, f;
    ph_apply_force_at_pos(&obj, pos, f);
}
END_TEST

START_TEST(test_apply_force_relative)
{
    ph_obj_t obj;
    memset(&obj, 0, sizeof(ph_obj_t));

    vector_t pos, f;

    ph_apply_force_relative(&obj, pos, f);
}
END_TEST

START_TEST(test_apply_force)
{
    ph_obj_t obj;
    memset(&obj, 0, sizeof(ph_obj_t));
    
    vector_t f = {4.0, 4.0, 4.0, 0.0};

    ph_apply_force(&obj, f);
    
    fail_unless( obj.f_acc.s.x == 4.0, "apply force failed");
    fail_unless( obj.f_acc.s.y == 4.0, "apply force failed");
    fail_unless( obj.f_acc.s.z == 4.0, "apply force failed");
    fail_unless( obj.f_acc.s.w == 0.0, "apply force failed");

    vector_t f2 = {1.0, 2.0, 3.0, 0.0};
    ph_apply_force(&obj, f2);

    fail_unless( obj.f_acc.s.x == 5.0, "apply force failed");
    fail_unless( obj.f_acc.s.y == 6.0, "apply force failed");
    fail_unless( obj.f_acc.s.z == 7.0, "apply force failed");
    fail_unless( obj.f_acc.s.w == 0.0, "apply force failed");
    
}
END_TEST

Suite
*test_suite (void)
{
    Suite *s = suite_create ("Physics Name");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_apply_force);
    tcase_add_test(tc_core, test_apply_force_relative);
    tcase_add_test(tc_core, test_apply_force_at_pos);
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

