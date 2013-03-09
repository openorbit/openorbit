/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <check.h>
#include "physics/physics.h"
#include "vmath/vmath.h"

#define IN_RANGE(v, a, b) ((a <= v) && (v <= b))

START_TEST(test_create_obj)
{
  PLobject2 *obj = plObject3d(PL_SEGMENT_LEN * 4.0 + 1000.0,
                              - (PL_SEGMENT_LEN * 2.0 + 1000.0),
                              0.0);
  fail_unless(v3i_get(obj->p.seg, 0) == 4, "calculation of i failed, %d", (int)v3i_get(obj->p.seg, 0));
  fail_unless(v3i_get(obj->p.seg, 1) == -2, "calculation of j failed");
  fail_unless(v3i_get(obj->p.seg, 2) == 0, "calculation of k failed");

  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 0), 999.99, 1000.01), "calculation of x failed %f", vf3_get(obj->p.offs, 0));
  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 1), -1000.01, -999.99), "calculation of y failed");
  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 2), 0.0, 0.0), "calculation of z failed");


  plObjectDelete(obj);
}
END_TEST

START_TEST(test_translate_obj)
{
  PLobject2 *obj = plObject3d(PL_SEGMENT_LEN * 4.0 + 1000.0,
                              - (PL_SEGMENT_LEN * 2.0 + 1000.0),
                              0.0);
  
  fail_unless(v3i_get(obj->p.seg, 0) == 4,
              "i was not set properly (%d)", (int)v3i_get(obj->p.seg, 0));
  fail_unless(v3i_get(obj->p.seg, 1) == -2,
              "j was not set properly (%d)", (int)v3i_get(obj->p.seg, 1));
  fail_unless(v3i_get(obj->p.seg, 2) == 0,
              "k was not set properly (%d)", (int)v3i_get(obj->p.seg, 2));
  
  
  float3 dp = vf3_set(PL_SEGMENT_LEN * 1.0,
                         PL_SEGMENT_LEN * 1.0,
                         - PL_SEGMENT_LEN * 1.0);
  plTranslateObject3fv(obj, dp);
  
  fail_unless(v3i_get(obj->p.seg, 0) == 5, "i was not incremented (%d)", (int)v3i_get(obj->p.seg, 0));
  fail_unless(v3i_get(obj->p.seg, 1) == -2, "j should not be incremented (%d)", (int)v3i_get(obj->p.seg, 1));
  fail_unless(v3i_get(obj->p.seg, 2) == -1, "k was not decremented (%d)", (int)v3i_get(obj->p.seg, 2));
  
  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 0), 999.99, 1000.01),
              "calculation of x failed (%f)", vf3_get(obj->p.offs, 0));
  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 1), 23.9, 24.1),
              "calculation of y failed (%f)", vf3_get(obj->p.offs, 1));
  fail_unless(IN_RANGE(vf3_get(obj->p.offs, 2), -0.001, 0.001),
              "calculation of z failed (%f)", vf3_get(obj->p.offs, 2));
}
END_TEST

START_TEST(test_obj_dist)
{
  PLobject2 *obj = plObject3d(0.0,
                              -(PL_SEGMENT_LEN - 100.0f),
                              0.0);
  PLobject2 *obj2 = plObject3d(0.0,
                               -(PL_SEGMENT_LEN + 100.0f),
                               0.0);


  float3 dist0 = plObjectDistance(obj, obj2);

  fail_unless(IN_RANGE(vf3_get(dist0, 0), 0.0, 0.00),
              "calculation of x failed (%f)", vf3_get(dist0, 0));
  fail_unless(IN_RANGE(vf3_get(dist0, 1), 199.9, 200.1),
              "calculation of y failed (%f)", vf3_get(dist0, 1));
  fail_unless(IN_RANGE(vf3_get(dist0, 2), 0.0, 0.00),
              "calculation of z failed (%f)", vf3_get(dist0, 2));


  float3 dp = vf3_set(0.0,
                        150.0,
                        0.0);
  plTranslateObject3fv(obj2, dp);

  float3 dist = plObjectDistance(obj, obj2);

  fail_unless(IN_RANGE(vf3_get(dist, 0), 0.0, 0.00),
              "calculation of x failed (%f)", vf3_get(dist, 0));
  fail_unless(IN_RANGE(vf3_get(dist, 1), 49.9, 50.1),
              "calculation of y failed (%f)", vf3_get(dist, 1));
  fail_unless(IN_RANGE(vf3_get(dist, 2), 0.0, 0.00),
              "calculation of z failed (%f)", vf3_get(dist, 2));

}
END_TEST
Suite
*test_suite (void)
{
    Suite *s = suite_create ("Physics Name");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test(tc_core, test_create_obj);

    tcase_add_test(tc_core, test_translate_obj);
    tcase_add_test(tc_core, test_obj_dist);
    suite_add_tcase(s, tc_core);
    
    return s;
}

