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
#include "physics/physics.h"
#include "vmath/vmath.h"

#define IN_RANGE(v, a, b) ((a <= v) && (v <= b))

START_TEST(test_create_obj)
{
  PLobject2 *obj = plObject3d(PL_CHUNK_RAD * 4.0 + 1000.0,
                              - (PL_CHUNK_RAD * 2.0 + 1000.0),
                              0.0);
  fail_unless(obj->i == 2, "calculation of i failed");
  fail_unless(obj->j == -1, "calculation of j failed");
  fail_unless(obj->k == 0, "calculation of k failed");

  fail_unless(IN_RANGE(vd3_get(obj->p, 0), 999.99, 1000.01), "calculation of x failed");
  fail_unless(IN_RANGE(vd3_get(obj->p, 1), -1000.01, -999.99), "calculation of y failed");
  fail_unless(IN_RANGE(vd3_get(obj->p, 2), 0.0, 0.0), "calculation of z failed");


  plObjectDelete(obj);
}
END_TEST

START_TEST(test_translate_obj)
{
  PLobject2 *obj = plObject3d(PL_CHUNK_RAD * 4.0 + 1000.0,
                              - (PL_CHUNK_RAD * 2.0 + 1000.0),
                              0.0);
  
  fail_unless(obj->i == 2, "i was not set properly (%d)", (int)obj->i);
  fail_unless(obj->j == -1, "j was not set properly (%d)", (int)obj->j);
  fail_unless(obj->k == 0, "k was not set properly (%d)", (int)obj->k);
  
  
  PLdouble3 dp = vd3_set(PL_CHUNK_LEN * 1.0,
                         PL_CHUNK_LEN * 1.0,
                         - PL_CHUNK_LEN * 1.0);
  plTranslateObject3dv(obj, dp);
  
  fail_unless(obj->i == 3, "i was not incremented (%d)", (int)obj->i);
  fail_unless(obj->j == 0, "j was not incremented (%d)", (int)obj->j);
  fail_unless(obj->k == -1, "k was not decremented (%d)", (int)obj->k);
  
  fail_unless(IN_RANGE(vd3_get(obj->p, 0), -PL_CHUNK_RAD + 999.99,- PL_CHUNK_RAD + 1000.01),
              "calculation of x failed (%f)", vd3_get(obj->p, 0));
  fail_unless(IN_RANGE(vd3_get(obj->p, 1),- PL_CHUNK_RAD + -1000.01, -999.99),
              "calculation of y failed (%f)", vd3_get(obj->p, 1));
  fail_unless(IN_RANGE(vd3_get(obj->p, 2), -0.001, 0.001),
              "calculation of z failed (%f)", vd3_get(obj->p, 2));
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
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

