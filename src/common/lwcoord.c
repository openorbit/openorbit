/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "lwcoord.h"
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

void
ooLwcSet(OOlwcoord *coord, float x, float y, float z)
{
  coord->offs = vf3_set(x, y, z);
  coord->seg = vi3_set(0, 0, 0);

  ooLwcNormalise(coord);
}

void
ooLwcNormalise(OOlwcoord *coord)
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
ooLwcTranslate(OOlwcoord *coord, float3 offs)
{
  coord->offs = vf3_add(coord->offs, offs);
  ooLwcNormalise(coord);
}

void
ooLwcTranslate3f(OOlwcoord *coord, float dx, float dy, float dz)
{
  coord->offs = vf3_add(coord->offs, vf3_set(dx, dy, dz));
  ooLwcNormalise(coord);
}

float3
ooLwcGlobal(const OOlwcoord *coord)
{
  float3 p = coord->offs;
  float3 seg = v3i_to_v3f(coord->seg);

  return vf3_add(p, vf3_s_mul(seg, OO_LW_SEGMENT_LEN));
}

float3
ooLwcRelVec(const OOlwcoord *coord, int3 seg)
{
  float3 r = coord->offs;
  int3 segdiff = coord->seg - seg;
  float3 segdiffr = vf3_set((float)v3i_get(segdiff, 0),
                            (float)v3i_get(segdiff, 1),
                            (float)v3i_get(segdiff, 2));
  r = vf3_add(r, vf3_s_mul(segdiffr, OO_LW_SEGMENT_LEN));
  return r;
}

float3
ooLwcDist(const OOlwcoord *a, const OOlwcoord * b)
{
  float3 diff = vf3_sub(a->offs, b->offs);
  int3 segdiff = a->seg - b->seg;
  float3 segdiffr = vf3_set((float)v3i_get(segdiff, 0),
                            (float)v3i_get(segdiff, 1),
                            (float)v3i_get(segdiff, 2));

  return vf3_add(diff, vf3_s_mul(segdiffr, OO_LW_SEGMENT_LEN));
}

void
ooLwcSet64(OOlwcoord64 *coord, double x, double y, double z)
{
  coord->offs = vd3_set(x, y, z);
  coord->seg = vl3_set(0, 0, 0);

  ooLwcNormalise64(coord);
}


void
ooLwcNormalise64(OOlwcoord64 *coord)
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
ooLwcTranslate64(OOlwcoord64 *coord, double3 offs)
{
  coord->offs = vd3_add(coord->offs, offs);
  ooLwcNormalise64(coord);
}

double3
ooLwcGlobal64(const OOlwcoord64 *coord)
{
  double3 p = coord->offs;
  double3 seg = v3l_to_v3d(coord->seg);
  return vd3_add(p, vd3_s_mul(seg, OO_LW_SEGMENT_LEN64));
}

double3
ooLwcRelVec64(const OOlwcoord64 *coord, long3 seg)
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
ooLwcDist64(const OOlwcoord64 *a, const OOlwcoord64 * b)
{
  double3 diff = vd3_sub(a->offs, b->offs);
  long3 segdiff = a->seg - b->seg;
  double3 segdiffr = vd3_set((double)v3l_get(segdiff, 0),
                             (double)v3l_get(segdiff, 1),
                             (double)v3l_get(segdiff, 2));

  return vd3_add(diff, vd3_s_mul(segdiffr, OO_LW_SEGMENT_LEN64));
}
