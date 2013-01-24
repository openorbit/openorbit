/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <vmath/vmath.h>

#include "lwcoord.h"
#include <stdio.h>

#ifndef __has_feature
#define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif


#if __has_feature(attribute_ext_vector_type)
#define OFFS_X(c) ((c)->offs.x)
#define OFFS_Y(c) ((c)->offs.y)
#define OFFS_Z(c) ((c)->offs.z)

#define SEG_X(c) ((c)->seg.x)
#define SEG_Y(c) ((c)->seg.y)
#define SEG_Z(c) ((c)->seg.z)

#define OFFS_X64(c) ((c)->offs.x)
#define OFFS_Y64(c) ((c)->offs.y)
#define OFFS_Z64(c) ((c)->offs.z)

#define SEG_X64(c) ((c)->seg.x)
#define SEG_Y64(c) ((c)->seg.y)
#define SEG_Z64(c) ((c)->seg.z)

#else

#define OFFS_X(c) (((float*)&(c)->offs)[0])
#define OFFS_Y(c) (((float*)&(c)->offs)[1])
#define OFFS_Z(c) (((float*)&(c)->offs)[2])

#define SEG_X(c) (((int32_t*)&(c)->seg)[0])
#define SEG_Y(c) (((int32_t*)&(c)->seg)[1])
#define SEG_Z(c) (((int32_t*)&(c)->seg)[2])


#define OFFS_X64(c) (((double*)&(c)->offs)[0])
#define OFFS_Y64(c) (((double*)&(c)->offs)[1])
#define OFFS_Z64(c) (((double*)&(c)->offs)[2])

#define SEG_X64(c) (((int64_t*)&(c)->seg)[0])
#define SEG_Y64(c) (((int64_t*)&(c)->seg)[1])
#define SEG_Z64(c) (((int64_t*)&(c)->seg)[2])
#endif
void
lwc_set(lwcoord_t *coord, float x, float y, float z)
{
  coord->offs = vf3_set(x, y, z);
  coord->seg = vi3_set(0, 0, 0);

  lwc_normalise(coord);
}

void
lwc_normalise(lwcoord_t *coord)
{
  if (fabsf(OFFS_X(coord)) >= OO_LW_SEGMENT_LEN) {
    SEG_X(coord) += (int32_t) (OFFS_X(coord) / OO_LW_SEGMENT_LEN);
    OFFS_X(coord) = fmodf(OFFS_X(coord), OO_LW_SEGMENT_LEN);
  }
  if (fabsf(OFFS_Y(coord)) >= OO_LW_SEGMENT_LEN) {
    SEG_Y(coord) += (int32_t) (OFFS_Y(coord) / OO_LW_SEGMENT_LEN);
    OFFS_Y(coord) = fmodf(OFFS_Y(coord), OO_LW_SEGMENT_LEN);
  }
  if (fabsf(OFFS_Z(coord)) >= OO_LW_SEGMENT_LEN) {
    SEG_Z(coord) += (int32_t) (OFFS_Z(coord) / OO_LW_SEGMENT_LEN);
    OFFS_Z(coord) = fmodf(OFFS_Z(coord), OO_LW_SEGMENT_LEN);
  }
}

void
lwc_mul(lwcoord_t *lwc, float b)
{
  float3 p = lwc_global(lwc);
  p = p * b;
  lwc_set(lwc, p.x, p.y, p.z);
}

void
lwc_div(lwcoord_t *lwc, float b)
{
  float3 p = lwc_global(lwc);
  p = p / b;
  lwc_set(lwc, p.x, p.y, p.z);
}

void
lwc_dump(const lwcoord_t *lwc)
{
  fprintf(stderr, "lwc: [%d %d %d]/[%f %f %f]\n",
          lwc->seg.x, lwc->seg.y, lwc->seg.z,
          lwc->offs.x, lwc->offs.y, lwc->offs.z);
}


void
lwc_translate3fv(lwcoord_t *coord, float3 offs)
{
  coord->offs = vf3_add(coord->offs, offs);
  lwc_normalise(coord);
}

void
lwc_translate3f(lwcoord_t *coord, float dx, float dy, float dz)
{
  coord->offs = vf3_add(coord->offs, vf3_set(dx, dy, dz));
  lwc_normalise(coord);
}

float3
lwc_global(const lwcoord_t *coord)
{
  float3 p = coord->offs;
  float3 seg = v3i_to_v3f(coord->seg);

  return vf3_add(p, vf3_s_mul(seg, OO_LW_SEGMENT_LEN));
}

float3
lwc_relvec(const lwcoord_t *coord, int3 seg)
{
  float3 r = coord->offs;
  int3 segdiff = coord->seg - seg;
  float3 segdiffr = vf3_set((float)v3i_x(segdiff),
                            (float)v3i_y(segdiff),
                            (float)v3i_z(segdiff));
  r = vf3_add(r, vf3_s_mul(segdiffr, OO_LW_SEGMENT_LEN));
  return r;
}

float3
lwc_dist(const lwcoord_t *a, const lwcoord_t * b)
{
  float3 diff = vf3_sub(a->offs, b->offs);
  int3 segdiff = a->seg - b->seg;
  float3 segdiffr = vf3_set((float)v3i_x(segdiff),
                            (float)v3i_y(segdiff),
                            (float)v3i_z(segdiff));

  return vf3_add(diff, vf3_s_mul(segdiffr, OO_LW_SEGMENT_LEN));
}


int
lwc_octant(const lwcoord_t *a, const lwcoord_t * b)
{
  float3 rel = lwc_dist(b, a);

  int octant = 0;
  if (rel.x >= 0.0) {
    octant += 1;
  }

  if (rel.y >= 0.0) {
    octant += 2;
  }

  if (rel.z >= 0.0) {
    octant += 4;
  }

  return octant;
}

void
lwc_set64(lwcoord64_t *coord, double x, double y, double z)
{
  coord->offs = vd3_set(x, y, z);
  coord->seg = vl3_set(0, 0, 0);

  lwc_normalise64(coord);
}


void
lwc_normalise64(lwcoord64_t *coord)
{
  if (fabs(OFFS_X64(coord)) >= OO_LW_SEGMENT_LEN64) {
    OFFS_X64(coord) += (int32_t) (OFFS_X64(coord) / OO_LW_SEGMENT_LEN64);
    OFFS_X64(coord) = fmod(OFFS_X64(coord), OO_LW_SEGMENT_LEN64);
  }
  if (fabs(OFFS_Y64(coord)) >= OO_LW_SEGMENT_LEN64) {
    SEG_Y64(coord) += (int32_t) (OFFS_Y64(coord) / OO_LW_SEGMENT_LEN64);
    OFFS_Y64(coord) = fmod(OFFS_Y64(coord), OO_LW_SEGMENT_LEN64);
  }
  if (fabs(OFFS_Z64(coord)) >= OO_LW_SEGMENT_LEN64) {
    SEG_Z64(coord) += (int32_t) (OFFS_Z64(coord) / OO_LW_SEGMENT_LEN64);
    OFFS_Z64(coord) = fmod(OFFS_Z64(coord), OO_LW_SEGMENT_LEN64);
  }
}

void
lwc_translate64(lwcoord64_t *coord, double3 offs)
{
  coord->offs = vd3_add(coord->offs, offs);
  lwc_normalise64(coord);
}

double3
lwc_global64(const lwcoord64_t *coord)
{
  double3 p = coord->offs;
  double3 seg = v3l_to_v3d(coord->seg);
  return vd3_add(p, vd3_s_mul(seg, OO_LW_SEGMENT_LEN64));
}

double3
lwc_relvec64(const lwcoord64_t *coord, long3 seg)
{
  double3 r = coord->offs;
  long3 segdiff = coord->seg - seg;

  double3 segdiffr = vd3_set((double)v3l_get(segdiff, 0),
                             (double)v3l_get(segdiff, 1),
                             (double)v3l_get(segdiff, 2));

  r = vd3_add(r, vd3_s_mul(segdiffr, OO_LW_SEGMENT_LEN64));
  return r;
}

double3
lwc_dist64(const lwcoord64_t *a, const lwcoord64_t * b)
{
  double3 diff = vd3_sub(a->offs, b->offs);
  long3 segdiff = a->seg - b->seg;
  double3 segdiffr = vd3_set((double)v3l_get(segdiff, 0),
                             (double)v3l_get(segdiff, 1),
                             (double)v3l_get(segdiff, 2));

  return vd3_add(diff, vd3_s_mul(segdiffr, OO_LW_SEGMENT_LEN64));
}
