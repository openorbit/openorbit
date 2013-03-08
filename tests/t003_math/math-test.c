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

#define ALMOST_EQUAL(x, y, tol) (((y - (tol)) <= (x)) && ((x) <= ((y) + (tol))))

START_TEST(test_m_v_mul)
{
  float4 r, v;
  float4x4 a;

  v.x = 0.2; v.y = 0.3; v.z = 0.4; v.w = 0.5;
  a[0][0] = a[0][1] = a[0][2] = a[0][3] = 1.0f;
  a[1][0] = a[1][1] = a[1][2] = a[1][3] = 2.0f;
  a[2][0] = a[2][1] = a[2][2] = a[2][3] = 3.0f;
  a[3][0] = a[3][1] = a[3][2] = a[3][3] = 4.0f;

  // first, test function
  r = mf4_v_mul(a, v);

  fail_unless( ALMOST_EQUAL(r[0], 1.4f, 0.000001f),
              "Vector [0] outside bounds. v[0] = (%f)", r.x);
  fail_unless( ALMOST_EQUAL(r[1], 2.8f, 0.000001f),
              "Vector [1] outside bounds. v[1] = (%f)", r.y);
  fail_unless( ALMOST_EQUAL(r[2], 4.2f, 0.000001f),
              "Vector [2] outside bounds. v[2] = (%f)", r.z);
  fail_unless( ALMOST_EQUAL(r[3], 5.6f, 0.000001f),
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
  float4x4 r0, a, b, ex;

  a[0][0] = a[0][1] = a[0][2] = a[0][3] = 1.0f;
  a[1][0] = a[1][1] = a[1][2] = a[1][3] = 2.0f;
  a[2][0] = a[2][1] = a[2][2] = a[2][3] = 3.0f;
  a[3][0] = a[3][1] = a[3][2] = a[3][3] = 4.0f;

  b[0][0] = b[1][0] = b[2][0] = b[3][0] = 1.0f;
  b[0][1] = b[1][1] = b[2][1] = b[3][1] = 2.0f;
  b[0][2] = b[1][2] = b[2][2] = b[3][2] = 3.0f;
  b[0][3] = b[1][3] = b[2][3] = b[3][3] = 4.0f;

  ex[0][0] = 2.0; ex[0][1] = 3.0; ex[0][2] = 4.0; ex[0][3] = 5.0;
  ex[1][0] = 3.0; ex[1][1] = 4.0; ex[1][2] = 5.0; ex[1][3] = 6.0;
  ex[2][0] = 4.0; ex[2][1] = 5.0; ex[2][2] = 6.0; ex[2][3] = 7.0;
  ex[3][0] = 5.0; ex[3][1] = 6.0; ex[3][2] = 7.0; ex[3][3] = 8.0;

  // check m_add function
  mf4_add(r0, a, b);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      fail_unless( ALMOST_EQUAL(r0[i][j], ex[i][j], 0.000001f),
                  "(r0[%d][%d] == %f) != (ex[%d][%d] == (%f)",
                  i, j, r0[i][j],
                  i, j, ex[i][j]);
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
  float4x4 m;

  m[0][0] = 1.0;
  m[0][3] = 54.0;

  mf4_zero(m);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      fail_unless(m[i][j] == 0.0,
                  "All matrix elements should be EXACTLY ZERO");
    }
  }
}
END_TEST


START_TEST(test_m_cpy)
{
  float4x4 a, b, z;
  mf4_zero(a);
  mf4_zero(z);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      b[i][j] = (float) i * (float)j + 1.0;
    }
  }

  mf4_cpy(a, b);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      fail_unless(a[i][j] == b[i][j],
                  "Copying matirix did not produce exact copy");
      fail_if(a[i][j] == z[i][j],
              "Copying matirix B to A produced A with elements valued 0.0");
    }
  }
}
END_TEST

START_TEST(test_v_set)
{

}
END_TEST


START_TEST(test_m_eq)
{

}
END_TEST


START_TEST(test_v_eq)
{

}
END_TEST

START_TEST(test_m_det)
{
  float4x4 m = {{ 1.0, -2.0,  0.0, 3.0},
                { 0.0,  4.0, -1.0, 2.0},
                {-1.0,  0.0, -3.0, 1.0},
                { 0.0,  2.0,  5.0, 0.0}};


  float det = mf4_det(m);
  fail_unless(ALMOST_EQUAL(det, -96.0, 0.0001),
              "Deteriminant expected -96.0, but got %f", det);

}
END_TEST

START_TEST(test_m_subdet3)
{
  float4x4 m = {{ 1.0, -2.0,  0.0, 3.0},
                { 0.0,  4.0, -1.0, 2.0},
                {-1.0,  0.0, -3.0, 1.0},
                { 0.0,  2.0,  5.0, 0.0}};

  float det = mf4_subdet3(m, 0, 0);
  fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
              "Sub deteriminant[0][0] expected -10.0, but got %f", det);

  det = mf4_subdet3(m, 0, 1);
  fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
              "Sub deteriminant[0][1] expected -10.0, but got %f", det);

  det = mf4_subdet3(m, 0, 2);
  fail_unless( ALMOST_EQUAL(det, S_CONST(-4.0), 0.0001f),
              "Sub deteriminant[0][2] expected 0.0, but got %f", det);

  det = mf4_subdet3(m, 0, 3);
  fail_unless( ALMOST_EQUAL(det, S_CONST(22.0), 0.0001f),
              "Sub deteriminant[0][3] expected 22.0, but got %f", det);

  det = mf4_subdet3(m, 2, 1);
  fail_unless( ALMOST_EQUAL(det, S_CONST(-10.0), 0.0001f),
              "Sub deteriminant[2][1] expected 10.0, but got %f", det);
}
END_TEST


START_TEST(test_m_inv)
{
  float4x4 m = {{1.0, 0.0, 0.0, 0.0},
                {1.0, 2.0, 0.0, 0.0},
                {2.0, 1.0, 3.0, 0.0},
                {1.0, 2.0, 1.0, 4.0}};
  float4x4 m_ex = {{ 1.000, 0.0, 0.0, 0.0},
                   {-0.500, 0.5, 0.0, 0.0},
                   {-0.500, -1.0/6.0, 1.0/3.0, 0.0},
                   {0.125, -5.0/24.0, -1.0/12.0, 0.25}};

  float4x4 inv;
  mf4_inv(inv, m);

  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      fail_unless( ALMOST_EQUAL(inv[i][j], m_ex[i][j], 0.000001f),
                  "inv[%d][%d] = %f, should be %f",
                  i, j, inv[i][j], m_ex[i][j]);
    }
  }
}
END_TEST

START_TEST(test_q_slerp)
{
  quaternion_t q0 = q_rot(1.0, 0.0, 0.0, 0.0);
  quaternion_t q1 = q_rot(1.0, 0.0, 0.0, M_PI_2);

  quaternion_t q = q_slerp(q0, q1, 0.0);
  fail_unless(ALMOST_EQUAL(q.x, q0.x, 0.000001f), "q0.x (%f != %f)", q.x, q0.x);
  fail_unless(ALMOST_EQUAL(q.y, q0.y, 0.000001f), "q0.y (%f != %f)", q.x, q0.x);
  fail_unless(ALMOST_EQUAL(q.z, q0.z, 0.000001f), "q0.z (%f != %f)", q.x, q0.x);
  fail_unless(ALMOST_EQUAL(q.w, q0.w, 0.000001f), "q0.w (%f != %f)", q.x, q0.x);

  q = q_slerp(q0, q1, 1.0);
  fail_unless(ALMOST_EQUAL(q.x, q1.x, 0.000001f), "q1.x (%f != %f)", q.x, q1.x);
  fail_unless(ALMOST_EQUAL(q.y, q1.y, 0.000001f), "q1.y (%f != %f)", q.y, q1.y);
  fail_unless(ALMOST_EQUAL(q.z, q1.z, 0.000001f), "q1.z (%f != %f)", q.z, q1.z);
  fail_unless(ALMOST_EQUAL(q.w, q1.w, 0.000001f), "q1.w (%f != %f)", q.w, q1.w);

  q = q_slerp(q0, q1, 0.5);
  quaternion_t q_expect = q_rot(1.0, 0.0, 0.0, M_PI_4);
  fail_unless(ALMOST_EQUAL(q.x, q_expect.x, 0.000001f), "q_exp.x (%f != %f)", q.x, q_expect.x);
  fail_unless(ALMOST_EQUAL(q.y, q_expect.y, 0.000001f), "q_exp.y (%f != %f)", q.y, q_expect.y);
  fail_unless(ALMOST_EQUAL(q.z, q_expect.z, 0.000001f), "q_exp.z (%f != %f)", q.z, q_expect.z);
  fail_unless(ALMOST_EQUAL(q.w, q_expect.w, 0.000001f), "q_exp.w (%f != %f)", q.w, q_expect.w);
}
END_TEST

START_TEST(test_mf4_q_convert)
{
  /* First test for unit matrix to unit quaternion */
  float4x4 m;
  mf4_ident(m);
  quaternion_t q = mf4_q_convert(m);
  quaternion_t q_exp = Q_IDENT;

  fail_unless(q.x == q_exp.x);
  fail_unless(q.y == q_exp.y);
  fail_unless(q.z == q_exp.z);
  fail_unless(q.w == q_exp.w);


  // Second test, 90 degree around x
  m[0] = vf4_set(1,0, 0,0);
  m[1] = vf4_set(0,0,-1,0);
  m[2] = vf4_set(0,1, 0,0);
  m[3] = vf4_set(0,0, 0,1);

  q_exp.x = 0.7071067811865475;
  q_exp.y = 0.0;
  q_exp.z = 0.0;
  q_exp.w = 0.7071067811865476;

  q = mf4_q_convert(m);

  fail_unless(ALMOST_EQUAL(q.x, q_exp.x, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.y, q_exp.y, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.z, q_exp.z, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.w, q_exp.w, 0.0000001));
}
END_TEST

START_TEST(test_mf3_q_convert)
{
  /* First test for unit matrix to unit quaternion */
  float3x3 m;
  mf3_ident(m);
  quaternion_t q = mf3_q_convert(m);
  quaternion_t q_exp = Q_IDENT;

  fail_unless(q.x == q_exp.x);
  fail_unless(q.y == q_exp.y);
  fail_unless(q.z == q_exp.z);
  fail_unless(q.w == q_exp.w);


  // Second test, 90 degree around x
  m[0] = vf3_set(1,0, 0);
  m[1] = vf3_set(0,0,-1);
  m[2] = vf3_set(0,1, 0);

  q_exp.x = 0.7071067811865475;
  q_exp.y = 0.0;
  q_exp.z = 0.0;
  q_exp.w = 0.7071067811865476;

  q = mf3_q_convert(m);

  fail_unless(ALMOST_EQUAL(q.x, q_exp.x, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.y, q_exp.y, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.z, q_exp.z, 0.0000001));
  fail_unless(ALMOST_EQUAL(q.w, q_exp.w, 0.0000001));
}
END_TEST

START_TEST(test_el_cs_area)
{
  // Last value is the expected, rest are parameters in order
  double testdata[][5] = {
    // Circles (a = 1, e = 0)
    {1.0, 0.0, M_PI_4, 0.0,           M_PI_4/2.0},
    {1.0, 0.0, M_PI_2, 0.0,           M_PI_4},
    {1.0, 0.0, M_PI_4, -M_PI_4,       M_PI_4},
    {1.0, 0.0, M_PI-M_PI_4, 0.0,      M_PI_2-M_PI_4/2.0},
    {1.0, 0.0, M_PI, 0.0,             M_PI_2},
    {1.0, 0.0, M_PI+M_PI_2, 0.0,      M_PI_2+M_PI_4},
    {1.0, 0.0, M_PI_2+M_PI_4, 0.0,    M_PI_4+M_PI_4/2.0},
    {1.0, 0.0, M_PI_2, -M_PI,         M_PI_4+M_PI_2},
    {1.0, 0.0, M_PI_2, -M_PI_2,       M_PI_2},

    // Ellipses, a = 1, b=0.5
    {1.0, sqrt(1-0.5*0.5), M_PI, 0.0,  1.570796327/2.0},
    {1.0, sqrt(1-0.5*0.5), M_PI_2, -M_PI_2,  1.570796327/2.0},
    {1.0, sqrt(1-0.5*0.5), M_PI_2+M_PI_4, -M_PI_4,  1.570796327/2.0},
  };

  for (int i = 0 ; i < sizeof(testdata)/sizeof(testdata[0]) ; i ++) {
    double res = el_cs_area(testdata[i][0], testdata[i][1], testdata[i][2],
                            testdata[i][3]);
    fail_unless(ALMOST_EQUAL(res, testdata[i][4], 0.00000001),
                "%d el_cs_area(%f, %f, %f, %f) = %f != %f",
                i, testdata[i][0], testdata[i][1], testdata[i][2],
                testdata[i][3], res, testdata[i][4]);
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
  tcase_add_test(tc_core, test_q_slerp);
  tcase_add_test(tc_core, test_mf4_q_convert);
  tcase_add_test(tc_core, test_mf3_q_convert);

  /* Ellipse functions */
  tcase_add_test(tc_core, test_el_cs_area);

  suite_add_tcase(s, tc_core);

  return s;
}

