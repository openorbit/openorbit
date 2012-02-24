/*
 Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>
 
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


#ifndef VMATH_INTEGER_H_8UWYEZ4B
#define VMATH_INTEGER_H_8UWYEZ4B

static inline int16_t
v3s_get(short3 s, int i)
{
  union {
    int16_t a[3];
    short3 v;
  } u;
  u.v = s;
  return u.a[i];
}

static inline int32_t
v3i_get(int3 s, int i)
{
  union {
    int32_t a[3];
    int3 v;
  } u;
  u.v = s;
  return u.a[i];
}

static inline int32_t
v3i_x(int3 s)
{
  return s.x;
}

static inline int32_t
v3i_y(int3 s)
{
  return s.y;
}

static inline int32_t
v3i_z(int3 s)
{
  return s.z;
}


static inline int64_t
v3l_get(long3 s, int i)
{
  union {
    int64_t a[3];
    long3 v;
  } u;
  u.v = s;
  return u.a[i];
}


static inline short3
vs3_set(int16_t a, int16_t b, int16_t c)
{
  union {
    short3 v;
    struct {
      int16_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

static inline int3
vi3_set(int32_t a, int32_t b, int32_t c)
{
  union {
    int3 v;
    struct {
      int32_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

static inline long3
vl3_set(int64_t a, int64_t b, int64_t c)
{
  union {
    long3 v;
    struct {
      int64_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

#endif /* end of include guard: VMATH_INTEGER_H_8UWYEZ4B */
