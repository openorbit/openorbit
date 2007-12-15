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

    // first, test function
    m_v_mul(r.a, a.a, v.a);
    
    fail_unless( ALMOST_EQUAL(r.a[0], 1.4f, 0.000001f),
        "Vector [0] outside bounds. v[0] = (%f)", r.s.x);
    fail_unless( ALMOST_EQUAL(r.a[1], 2.8f, 0.000001f),
        "Vector [1] outside bounds. v[1] = (%f)", r.s.y);
    fail_unless( ALMOST_EQUAL(r.a[2], 4.2f, 0.000001f),
        "Vector [2] outside bounds. v[2] = (%f)", r.s.z);
    fail_unless( ALMOST_EQUAL(r.a[3], 5.6f, 0.000001f),
        "Vector [3] outside bounds. v[3] = (%f)", r.s.w);

    // then, test optimised macro and compare with function, we allow some
    // tolerance
    vector_t r2;
    M_V_MUL(r2, a, v);
    
    fail_unless( ALMOST_EQUAL(r2.a[0], r.a[0], 0.000001f),
        "(r2[0] == %f) != (r[0] == (%f)", r2.s.x, r.s.x);
    fail_unless( ALMOST_EQUAL(r2.a[1], r.a[1], 0.000001f),
        "(r2[0] == %f) != (r[0] == (%f)", r2.s.x, r.s.x);
    fail_unless( ALMOST_EQUAL(r2.a[2], r.a[2], 0.000001f),
        "(r2[0] == %f) != (r[0] == (%f)", r2.s.x, r.s.x);
    fail_unless( ALMOST_EQUAL(r2.a[3], r.a[3], 0.000001f),
        "(r2[0] == %f) != (r[0] == (%f)", r2.s.x, r.s.x);
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

// TODO: M_MUL test
START_TEST(test_m_mul)
{

}
END_TEST

START_TEST(test_m_add)
{
    matrix_t r0, r1, a, b, ex;
    
    a.a[0][0] = a.a[0][1] = a.a[0][2] = a.a[0][3] = 1.0f;
    a.a[1][0] = a.a[1][1] = a.a[1][2] = a.a[1][3] = 2.0f;
    a.a[2][0] = a.a[2][1] = a.a[2][2] = a.a[2][3] = 3.0f;
    a.a[3][0] = a.a[3][1] = a.a[3][2] = a.a[3][3] = 4.0f;
    
    b.a[0][0] = b.a[1][0] = b.a[2][0] = b.a[3][0] = 1.0f;
    b.a[0][1] = b.a[1][1] = b.a[2][1] = b.a[3][1] = 2.0f;
    b.a[0][2] = b.a[1][2] = b.a[2][2] = b.a[3][2] = 3.0f;
    b.a[0][3] = b.a[1][3] = b.a[2][3] = b.a[3][3] = 4.0f;
    
    ex.a[0][0] = 2.0; ex.a[0][1] = 3.0; ex.a[0][2] = 4.0; ex.a[0][3] = 5.0;
    ex.a[1][0] = 3.0; ex.a[1][1] = 4.0; ex.a[1][2] = 5.0; ex.a[1][3] = 6.0;
    ex.a[2][0] = 4.0; ex.a[2][1] = 5.0; ex.a[2][2] = 6.0; ex.a[2][3] = 7.0;
    ex.a[3][0] = 5.0; ex.a[3][1] = 6.0; ex.a[3][2] = 7.0; ex.a[3][3] = 8.0;
    
    // check m_add function
    m_add(r0.a, a.a, b.a);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless( ALMOST_EQUAL(r0.a[i][j], ex.a[i][j], 0.000001f),
                "(r0[%d][%d] == %f) != (ex[%d][%d] == (%f)",
                i, j, r0.a[i][j],
                i, j, ex.a[i][j]);
            
        }
    }
    
    // check potentially optimised macro
    M_ADD(r1, a, b);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless( ALMOST_EQUAL(r1.a[i][j], ex.a[i][j], 0.000001f),
                "(r1[%d][%d] == %f) != (ex[%d][%d] == (%f)",
                i, j, r1.a[i][j],
                i, j, ex.a[i][j]);
        }
    }
}
END_TEST

// TODO: V_ADD test
START_TEST(test_v_add)
{

}
END_TEST

// TODO: V_SUB test
START_TEST(test_v_sub)
{

}
END_TEST

// TODO: V_S_MUL test
START_TEST(test_v_s_mul)
{

}
END_TEST

// TODO: V_CROSS test
START_TEST(test_v_cross)
{

}
END_TEST

// TODO: V_DOT test
START_TEST(test_v_dot)
{

}
END_TEST

// TODO: V_NORMALISE test
START_TEST(test_v_normalise)
{

}
END_TEST

// TODO: V_ABS test
START_TEST(test_v_abs)
{

}
END_TEST

// TODO: M_UNIT test
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

