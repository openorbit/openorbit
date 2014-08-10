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
  pl_world_t *world = pl_new_world(1000000.0);
  pl_object_t *obj = pl_new_object(world, "test-object");


  fail_unless(obj->p.seg.x == 0, "calculation of i failed, %d", (int)obj->p.seg.x);
  fail_unless(obj->p.seg.y == 0, "calculation of j failed");
  fail_unless(obj->p.seg.z == 0, "calculation of k failed");

  fail_unless(IN_RANGE(obj->p.offs.x, 0, 0),
              "calculation of x failed %f", obj->p.offs.x);
  fail_unless(IN_RANGE(obj->p.offs.y, 0, 0), "calculation of y failed");
  fail_unless(IN_RANGE(obj->p.offs.z, 0, 0), "calculation of z failed");


  pl_delete_object(obj);
}
END_TEST

Suite
*test_suite (void)
{
    Suite *s = suite_create ("Physics Name");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_create_obj);

    suite_add_tcase(s, tc_core);

    return s;
}
