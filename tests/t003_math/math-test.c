/* Open Orbit - An interactive spacecraft simulator.
* Copyright (C) 2006  Mattias Holm (mattias.holm@contra.nu)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of version 2 of the GNU General Public
* License as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdlib.h>
#include <check.h>

#import <math/linalg.h>
#import <math/quaternions.h>

#define ALMOST_EQUAL(x, y, tol) (((y - (tol)) < (x)) && ((x) < ((y) + (tol))))

START_TEST(test_m_v_mul)
{
    vector_t r, v;
    matrix_t a;
    
    v.s.x = 0.2; v.s.y = 0.3; v.s.z = 0.4; v.s.w = 0.5;
    a.a[0][0] = a.a[0][1] = a.a[0][2] = a.a[0][3] = 1.0f;
    a.a[1][0] = a.a[1][1] = a.a[1][2] = a.a[1][3] = 2.0f;
    a.a[2][0] = a.a[2][1] = a.a[2][2] = a.a[2][3] = 3.0f;
    a.a[3][0] = a.a[3][1] = a.a[3][2] = a.a[3][3] = 4.0f;
    
    m_v_mul(r.a, a.a, v.a);
    
    fail_unless( ALMOST_EQUAL(r.a[0], 1.4f, 0.000001f), "Vector [0] outside bounds.");
    fail_unless( ALMOST_EQUAL(r.a[1], 2.8f, 0.000001f), "Vector [1] outside bounds.");
    fail_unless( ALMOST_EQUAL(r.a[2], 4.2f, 0.000001f), "Vector [2] outside bounds.");
    fail_unless( ALMOST_EQUAL(r.a[3], 5.6f, 0.000001f), "Vector [3] outside bounds.");
}
END_TEST

START_TEST(test_q_mul)
{
    quat_arr_t r, a, b;
    
    QUAT_X(a) = 1.4; QUAT_Y(a) = 5.7; QUAT_Z(a) = 2.0; QUAT_W(a) = -2.1;
    QUAT_X(b) = 0.2; QUAT_Y(b) = 0.1; QUAT_Z(b) = 1.0; QUAT_W(b) = 8.0;
    
    q_mul(r, a, b);
    
    fail_unless( ALMOST_EQUAL(QUAT_X(r), 16.280f, 0.00001f),
                 "Vector [0] of quaternion outside bounds (%f)", QUAT_X(r));
    fail_unless( ALMOST_EQUAL(QUAT_Y(r), 44.390f, 0.00001f),
                 "Vector [1] of quaternion outside bounds (%f)", QUAT_Y(r));
    fail_unless( ALMOST_EQUAL(QUAT_Z(r), 12.900f, 0.00001f),
                 "Vector [2] of quaternion outside bounds (%f)", QUAT_Z(r));
    fail_unless( ALMOST_EQUAL(QUAT_W(r), -19.65f, 0.00001f),
                 "Scalar part of quaternion outside bounds (%f)", QUAT_W(r));
    
}
END_TEST

START_TEST(test_m_mul)
{

}
END_TEST


START_TEST(test_m_add)
{

}
END_TEST

START_TEST(test_v_add)
{

}
END_TEST

START_TEST(test_v_sub)
{

}
END_TEST

START_TEST(test_v_s_mul)
{

}
END_TEST

START_TEST(test_v_cross)
{

}
END_TEST

START_TEST(test_v_dot)
{

}
END_TEST

START_TEST(test_v_normalise)
{

}
END_TEST

START_TEST(test_v_abs)
{

}
END_TEST

START_TEST(test_m_axis_rot_x)
{

}
END_TEST

START_TEST(test_m_axis_rot_y)
{

}
END_TEST

START_TEST(test_m_axis_rot_z)
{

}
END_TEST

START_TEST(test_m_vec_rot_x)
{

}
END_TEST

START_TEST(test_m_vec_rot_y)
{

}
END_TEST

START_TEST(test_m_vec_rot_z)
{

}
END_TEST

START_TEST(test_m_unit)
{

}
END_TEST

START_TEST(test_m_zero)
{
    matrix_t m;
    
    m.a[0][0] = 1.0;
    m.a[0][3] = 54.0;
    
    m_zero(m.a);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless(m.a[i][j] == 0.0,
                "All matrix elements should be EXACTLY ZERO");
        }
    }
}
END_TEST

START_TEST(test_v_cpy)
{
    vec_arr_t a, b;
 
    for (int i = 0 ; i < 4 ; i ++) {
        b[i] = (float) i + 1.0;
    }
    
    v_cpy(a, b);
    
    for (int i = 0 ; i < 4 ; i ++) {
        fail_unless(a[i] == b[i],
            "Copying vector did not produce exact copy");
    }
}
END_TEST

START_TEST(test_m_cpy)
{
    matrix_t a, b, z;
    m_zero(a.a);
    m_zero(z.a);

    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            b.a[i][j] = (float) i * (float)j + 1.0;
        }
    }
    
    m_cpy(a.a, b.a);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless(a.a[i][j] == b.a[i][j],
                "Copying matirix did not produce exact copy");
            fail_if(a.a[i][j] == z.a[i][j],
                "Copying matirix B to A produced A with elements valued 0.0");
        }
    }
}
END_TEST

START_TEST(test_v_set)
{
    vector_t v;
}
END_TEST


START_TEST(test_m_eq)
{
    vector_t v;
}
END_TEST


START_TEST(test_v_eq)
{
    vector_t a, b;
}
END_TEST



Suite
*test_suite (void)
{
    Suite *s = suite_create ("Math Test");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    /* Matrix-vector functions */
    tcase_add_test(tc_core, test_m_v_mul);
    tcase_add_test(tc_core, test_m_mul);
    tcase_add_test(tc_core, test_m_add);
    tcase_add_test(tc_core, test_v_add);
    tcase_add_test(tc_core, test_v_sub);
    tcase_add_test(tc_core, test_v_s_mul);
    tcase_add_test(tc_core, test_v_cross);
    tcase_add_test(tc_core, test_v_dot);
    tcase_add_test(tc_core, test_v_normalise);
    tcase_add_test(tc_core, test_v_abs);
    tcase_add_test(tc_core, test_m_unit);
    tcase_add_test(tc_core, test_m_zero);
    tcase_add_test(tc_core, test_v_cpy);
    tcase_add_test(tc_core, test_m_cpy);
    tcase_add_test(tc_core, test_v_set);
    tcase_add_test(tc_core, test_m_eq);
    tcase_add_test(tc_core, test_v_eq);

    /* Quaternion functions */
    tcase_add_test(tc_core, test_q_mul);
    
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

