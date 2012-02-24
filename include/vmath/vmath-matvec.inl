/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit. Open Orbit is free software: you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

  Some files of Open Orbit have relaxed licensing conditions. This file is
  licenced under the 2-clause BSD licence.

  Redistribution and use of this file in source and binary forms, with or
  without modification, are permitted provided that the following conditions are
  met:
 
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Note that this is an umbrella file as well as a default definition file, there
might be more architectural specialised files further down from here
*/

#ifndef VM_V4_NEG
#define VM_V4_NEG

static inline float3
v_neg(float3 v)
{
  float3 nv = -v;
  return nv;
}
#endif /* VM_V4_NEG */


#ifndef VM_M4_MUL
#define VM_M4_MUL
void m_transpose(matrix_t *bT, const matrix_t *b);
static inline void
m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
{
    matrix_t tmp_res, bT;
    m_transpose(&bT, b);
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0; j < 4 ; j ++) {
            tmp_res.a[i][j] = vf4_dot(a->v[i], bT.v[j]);
        }
    }
    *res = tmp_res;
}
#endif

#ifndef VM_M4_V4_MUL
#define VM_M4_V4_MUL

static inline float3
m_v_mul(const matrix_t *a, float3 v)
{
	float3 vr;
	matrix_t p;
  p.v[0] = a->v[0] * v;
  p.v[1] = a->v[1] * v;
  p.v[2] = a->v[2] * v;
  p.v[3] = a->v[3] * v;
    
  vr.x = p.a[0][0] + p.a[0][1] + p.a[0][2] + p.a[0][3];
  vr.y = p.a[1][0] + p.a[1][1] + p.a[1][2] + p.a[1][3];
  vr.z = p.a[2][0] + p.a[2][1] + p.a[2][2] + p.a[2][3];
  vr.w = p.a[3][0] + p.a[3][1] + p.a[3][2] + p.a[3][3];

	return vr;
}
#endif /*VM_M4_V4_MUL*/

#if 0
static inline matrix_t
m_add(const matrix_t a, const matrix_t b)
{
	matrix_t res;
	res.v[0] = a.v[0] + b.v[0];
    res.v[1] = a.v[1] + b.v[1];
    res.v[2] = a.v[2] + b.v[2];
    res.v[3] = a.v[3] + b.v[3];
	return res;
}

static inline vector_t
v_s_add(vector_t a, scalar_t b)
{
	vector_t b = {.a = {b,b,b,b}};
	return a.v + b.v;
}

static inline vector_t
v_add(vector_t a, vector_t b)
{
	return a.v + b.v;
}

static inline vector_t
v_sub(vector_t a, vector_t b)
{
	return a.v - b.v;
}


static inline vector_t
v_s_mul(vector_t a, scalar_t b)
{
	vector_t b = {.a = {b,b,b,b}};
	return a.v * b.v;
}

static inline vector_t
v_s_div(vector_t a, scalar_t b)
{
	vector_t b = {.a = {b,b,b,b}};
	return a.v / b.v;
}

static inline vector_t
v_set(scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
{
	vector_t v = {.a = {v0,v1,v2,v3}};
	return v;
}
#endif