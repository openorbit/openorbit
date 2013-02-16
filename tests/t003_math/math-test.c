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
#include <stdio.h>

#include <check.h>

#import <vmath/vmath.h>

#define ALMOST_EQUAL(x, y, tol) (((y - (tol)) < (x)) && ((x) < ((y) + (tol))))

START_TEST(test_m_v_mul)
{
    vector_t r, v;
    matrix_t a;
    
    v.x = 0.2; v.y = 0.3; v.z = 0.4; v.w = 0.5;
    a.a[0][0] = a.a[0][1] = a.a[0][2] = a.a[0][3] = 1.0f;
    a.a[1][0] = a.a[1][1] = a.a[1][2] = a.a[1][3] = 2.0f;
    a.a[2][0] = a.a[2][1] = a.a[2][2] = a.a[2][3] = 3.0f;
    a.a[3][0] = a.a[3][1] = a.a[3][2] = a.a[3][3] = 4.0f;

    // first, test function
    r = m_v_mul(&a, v);
    
    fail_unless( ALMOST_EQUAL(r.a[0], 1.4f, 0.000001f),
        "Vector [0] outside bounds. v[0] = (%f)", r.x);
    fail_unless( ALMOST_EQUAL(r.a[1], 2.8f, 0.000001f),
        "Vector [1] outside bounds. v[1] = (%f)", r.y);
    fail_unless( ALMOST_EQUAL(r.a[2], 4.2f, 0.000001f),
        "Vector [2] outside bounds. v[2] = (%f)", r.z);
    fail_unless( ALMOST_EQUAL(r.a[3], 5.6f, 0.000001f),
        "Vector [3] outside bounds. v[3] = (%f)", r.w);

}
END_TEST

START_TEST(test_q_mul)
{
    quaternion_t r, a, b;
    
    a.x = 1.4; a.y = 5.7; a.z = 2.0; a.w = -2.1;
    b.x = 0.2; b.y = 0.1; b.z = 1.0; b.w = 8.0;
    
    r = q_mul(a, b);
    
    fail_unless( ALMOST_EQUAL(r.x, 16.280f, 0.00001f),
                 "Vector [0] of quaternion outside bounds (%f)", r.x);
    fail_unless( ALMOST_EQUAL(r.y, 44.390f, 0.00001f),
                 "Vector [1] of quaternion outside bounds (%f)", r.y);
    fail_unless( ALMOST_EQUAL(r.z, 12.900f, 0.00001f),
                 "Vector [2] of quaternion outside bounds (%f)", r.z);
    fail_unless( ALMOST_EQUAL(r.w, -19.65f, 0.00001f),
                 "Scalar part of quaternion outside bounds (%f)", r.w);
    
}
END_TEST

// TODO: M_MUL test
START_TEST(test_m_mul)
{
  float4x4 a = {{ 1,  2,  3,  4},
                { 5,  6,  7,  8},
                { 9, 10, 11, 12},
                {13, 14, 15, 16}};

  float4x4 b = {{21, 22, 23, 24},
                {25, 26, 27, 28},
                {29, 30, 31, 32},
                {33, 34, 35, 36}};

  float4x4 exp = {{ 290,  300,  310,  320},
                  { 722,  748,  774,  800},
                  {1154, 1196, 1238, 1280},
                  {1586, 1644, 1702, 1760}};

  mf4_mul2(a, b);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0; j < 4; j ++) {
      fail_unless(ALMOST_EQUAL(exp[i][j], a[i][j], 0.01));
    }
  }
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
    m_add(&r0, &a, &b);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless( ALMOST_EQUAL(r0.a[i][j], ex.a[i][j], 0.000001f),
                "(r0[%d][%d] == %f) != (ex[%d][%d] == (%f)",
                i, j, r0.a[i][j],
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
    
    m_zero(&m);
    
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless(m.a[i][j] == 0.0,
                "All matrix elements should be EXACTLY ZERO");
        }
    }
}
END_TEST


START_TEST(test_m_cpy)
{
    matrix_t a, b, z;
    m_zero(&a);
    m_zero(&z);

    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            b.a[i][j] = (float) i * (float)j + 1.0;
        }
    }
    
    m_cpy(&a, &b);
    
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

START_TEST(test_m_det)
{
    matrix_t m = {.a =
         {S_CONST( 1.0),S_CONST(-2.0),S_CONST( 0.0),S_CONST(3.0),
          S_CONST( 0.0),S_CONST( 4.0),S_CONST(-1.0),S_CONST(2.0),
          S_CONST(-1.0),S_CONST( 0.0),S_CONST(-3.0),S_CONST(1.0),
          S_CONST( 0.0),S_CONST( 2.0),S_CONST( 5.0),S_CONST(0.0)}};
    
    
    scalar_t det = m_det(&m);
    fail_unless( ALMOST_EQUAL(det, S_CONST(-96.0), 0.0001f),
                "Deteriminant expected -96.0, but got %f", det);
    
}
END_TEST

START_TEST(test_m_subdet3)
{
    matrix_t m = {.a =
         {S_CONST( 1.0),S_CONST(-2.0),S_CONST( 0.0),S_CONST(3.0),
          S_CONST( 0.0),S_CONST( 4.0),S_CONST(-1.0),S_CONST(2.0),
          S_CONST(-1.0),S_CONST( 0.0),S_CONST(-3.0),S_CONST(1.0),
          S_CONST( 0.0),S_CONST( 2.0),S_CONST( 5.0),S_CONST(0.0)}};

    scalar_t det = m_subdet3(&m, 0, 0);
    fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
                "Sub deteriminant[0][0] expected -10.0, but got %f", det);

    det = m_subdet3(&m, 0, 1);
    fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
                "Sub deteriminant[0][1] expected -10.0, but got %f", det);
    
    det = m_subdet3(&m, 0, 2);
    fail_unless( ALMOST_EQUAL(det, S_CONST(-4.0), 0.0001f),
                "Sub deteriminant[0][2] expected 0.0, but got %f", det);

    det = m_subdet3(&m, 0, 3);
    fail_unless( ALMOST_EQUAL(det, S_CONST(22.0), 0.0001f),
                "Sub deteriminant[0][3] expected 22.0, but got %f", det);

    det = m_subdet3(&m, 2, 1);
    fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
                "Sub deteriminant[2][1] expected 10.0, but got %f", det);
}
END_TEST


START_TEST(test_m_inv)
{
    matrix_t m = {
        .a = {S_CONST(1.0),S_CONST(0.0),S_CONST(0.0),S_CONST(0.0),
              S_CONST(1.0),S_CONST(2.0),S_CONST(0.0),S_CONST(0.0),
              S_CONST(2.0),S_CONST(1.0),S_CONST(3.0),S_CONST(0.0),
              S_CONST(1.0),S_CONST(2.0),S_CONST(1.0),S_CONST(4.0)}};
    matrix_t m_ex = {
        .a = {S_CONST( 1.000),S_CONST(0.0),S_CONST( 0.0),S_CONST(0.0),
              S_CONST(-0.500),S_CONST(0.5),S_CONST(0.0),S_CONST(0.0),
              S_CONST(-0.500),S_CONST(-1.0)/S_CONST(6.0),
                S_CONST(1.0)/S_CONST(3.0),S_CONST(0.0),
              S_CONST( 0.125),S_CONST(-5.0)/S_CONST(24.0),
                S_CONST( -1.0)/S_CONST(12.0),S_CONST(0.25)}};

    matrix_t inv = m_inv(&m);

    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            fail_unless( ALMOST_EQUAL(inv.a[i][j], m_ex.a[i][j], 0.000001f),
                        "inv[%d][%d] = %f, should be %f",
                        i, j, inv.a[i][j], m_ex.a[i][j]);
        }
    }
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
    tcase_add_test(tc_core, test_m_cpy);
    tcase_add_test(tc_core, test_v_set);
    tcase_add_test(tc_core, test_m_eq);
    tcase_add_test(tc_core, test_v_eq);
    tcase_add_test(tc_core, test_m_det);
    tcase_add_test(tc_core, test_m_subdet3);
    tcase_add_test(tc_core, test_m_inv);

    /* Quaternion functions */
    tcase_add_test(tc_core, test_q_mul);
    
    
    suite_add_tcase(s, tc_core);
    
    return s;
}

