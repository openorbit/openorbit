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
#include <string.h>
#include <check.h>

START_TEST(test_apply_force_at_pos)
{
//    ph_obj_t *obj = ph_new_obj();
    //memset(&obj, 0, sizeof(ph_obj_t));
    
//    vector_t pos, f;
//    ph_apply_force_at_pos(&obj, pos, f);
    ;
}
END_TEST

START_TEST(test_apply_force_relative)
{
//    ph_obj_t obj;
//    memset(&obj, 0, sizeof(ph_obj_t));

//    vector_t pos, f;

//    ph_apply_force_relative(&obj, pos, f);
}
END_TEST

START_TEST(test_apply_force)
{
//    ph_obj_t obj;
//    memset(&obj, 0, sizeof(ph_obj_t));
    
//    vector_t f = {4.0, 4.0, 4.0, 0.0};

//    ph_apply_force(&obj, f);
    
//    fail_unless( obj.f_acc.x == 4.0, "apply force failed");
//    fail_unless( obj.f_acc.y == 4.0, "apply force failed");
//    fail_unless( obj.f_acc.z == 4.0, "apply force failed");
//    fail_unless( obj.f_acc.w == 0.0, "apply force failed");

//    vector_t f2 = {1.0, 2.0, 3.0, 0.0};
//    ph_apply_force(&obj, f2);

//    fail_unless( obj.f_acc.x == 5.0, "apply force failed");
//    fail_unless( obj.f_acc.y == 6.0, "apply force failed");
//    fail_unless( obj.f_acc.z == 7.0, "apply force failed");
 //   fail_unless( obj.f_acc.w == 0.0, "apply force failed");
    
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

