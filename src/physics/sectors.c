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

#include "physics.h"
#include "common/lwcoord.h"
#if 0
void
plUpdateDrawable(PLobject2 *obj)
{
  PLint3 camSect;
  // Update drawable position, in this case we must know in which sector the camera is
  PLfloat3 newPos = ooLwcRelVec(&obj->p, camSect);
}
#endif
PLocttree*
plOcttree(double width, int levels)
{
  PLocttree *octtree = malloc(sizeof(PLocttree));
  octtree->level = levels;
  return octtree;
}
#if 0
void
plLwcNormalise(PLlwcoord *coord)
{
  if (fabsf(OFFS_X(coord)) >= PL_SEGMENT_LEN) {
    SEG_X(coord) += (int32_t) (OFFS_X(coord) / PL_SEGMENT_LEN);
    OFFS_X(coord) = fmodf(OFFS_X(coord), PL_SEGMENT_LEN);
  }
  if (fabsf(OFFS_Y(coord)) >= PL_SEGMENT_LEN) {
    SEG_Y(coord) += (int32_t) (OFFS_Y(coord) / PL_SEGMENT_LEN);
    OFFS_Y(coord) = fmodf(OFFS_Y(coord), PL_SEGMENT_LEN);
  }
  if (fabsf(OFFS_Z(coord)) >= PL_SEGMENT_LEN) {
    SEG_Z(coord) += (int32_t) (OFFS_Z(coord) / PL_SEGMENT_LEN);
    OFFS_Z(coord) = fmodf(OFFS_Z(coord), PL_SEGMENT_LEN);
  }
}

void
plLwcTranslate(PLlwcoord *coord, PLfloat3 offs)
{
  coord->offs = vf3_add(coord->offs, offs);
  plLwcNormalise(coord);
}

PLfloat3
plLwcGlobal(const PLlwcoord *coord)
{
  PLfloat3 p = coord->offs;
  return vf3_add(p, vf3_s_mul(p, PL_SEGMENT_LEN));
}

PLfloat3
plLwcRelVec(const PLlwcoord *coord, PLint3 seg)
{
  PLfloat3 r = coord->offs;
  PLint3 segdiff = coord->seg - seg;
  PLfloat3 segdiffr = vf3_set((float)((int32_t*)&segdiff)[0],
                               (float)((int32_t*)&segdiff)[1],
                               (float)((int32_t*)&segdiff)[2]);
  r = vf3_add(r, vf3_s_mul(segdiffr, PL_SEGMENT_LEN));
  return r;
}

PLfloat3
plLwcDist(const PLlwcoord *a, const PLlwcoord * b)
{
  PLfloat3 diff = vf3_sub(a->offs, b->offs);
  PLint3 segdiff = a->seg - b->seg;
  PLfloat3 segdiffr = vf3_set((float)((int32_t*)&segdiff)[0],
                              (float)((int32_t*)&segdiff)[1],
                              (float)((int32_t*)&segdiff)[2]);

  return vf3_add(diff, vf3_s_mul(segdiffr, PL_SEGMENT_LEN));
}



void
plLwcNormalise64(PLlwcoord64 *coord)
{
  if (fabs(OFFS_X64(coord)) >= PL_SEGMENT_LEN64) {
    OFFS_X64(coord) += (int32_t) (OFFS_X64(coord) / PL_SEGMENT_LEN64);
    OFFS_X64(coord) = fmod(OFFS_X64(coord), PL_SEGMENT_LEN64);
  }
  if (fabs(OFFS_Y64(coord)) >= PL_SEGMENT_LEN64) {
    SEG_Y64(coord) += (int32_t) (OFFS_Y64(coord) / PL_SEGMENT_LEN64);
    OFFS_Y64(coord) = fmod(OFFS_Y64(coord), PL_SEGMENT_LEN64);
  }
  if (fabs(OFFS_Z64(coord)) >= PL_SEGMENT_LEN64) {
    SEG_Z64(coord) += (int32_t) (OFFS_Z64(coord) / PL_SEGMENT_LEN64);
    OFFS_Z64(coord) = fmod(OFFS_Z64(coord), PL_SEGMENT_LEN64);
  }
}

void
plLwcTranslate64(PLlwcoord64 *coord, PLdouble3 offs)
{
  coord->offs = vd3_add(coord->offs, offs);
  plLwcNormalise64(coord);
}

PLdouble3
plLwcGlobal64(const PLlwcoord64 *coord)
{
  PLdouble3 p = coord->offs;
  return vd3_add(p, vd3_s_mul(p, PL_SEGMENT_LEN64));
}

PLdouble3
plLwcRelVec64(const PLlwcoord64 *coord, PLlong3 seg)
{
  PLdouble3 r = coord->offs;
  PLlong3 segdiff = coord->seg - seg;
  PLdouble3 segdiffr = vd3_set((double)((int32_t*)&segdiff)[0],
                               (double)((int32_t*)&segdiff)[1],
                               (double)((int32_t*)&segdiff)[2]);
  r = vd3_add(r, vd3_s_mul(segdiffr, PL_SEGMENT_LEN64));
  return r;
}

PLdouble3
plLwcDist64(const PLlwcoord64 *a, const PLlwcoord64 * b)
{
  PLdouble3 diff = vd3_sub(a->offs, b->offs);
  PLlong3 segdiff = a->seg - b->seg;
  PLdouble3 segdiffr = vd3_set((double)((int32_t*)&segdiff)[0],
                               (double)((int32_t*)&segdiff)[1],
                               (double)((int32_t*)&segdiff)[2]);

  return vd3_add(diff, vd3_s_mul(segdiffr, PL_SEGMENT_LEN64));
}

#endif // 0

PLfloat3
plGetObjGlobal3d(PLobject2 *obj)
{
  return ooLwcGlobal(&obj->p);
}

PLfloat3
plObjectDistance(PLobject2 *a, PLobject2 *b)
{
  return ooLwcDist(&a->p, &b->p);
}

PLobject2*
plObject3d(double x, double y, double z)
{
  PLobject2 *obj = malloc(sizeof(PLobject2));
  obj->p.offs = vf3_set(x, y, z);
  obj->p.seg = vi3_set(0, 0, 0);
  ooLwcNormalise(&obj->p);
  return obj;
}

void
plObjectDelete(PLobject2 *obj)
{
  free(obj);
}

void
plTranslateObject3fv(PLobject2 *obj, PLfloat3 dp)
{
  ooLwcTranslate(&obj->p, dp);
}

PLfloat3
plObjectGlobal(PLobject2 *obj)
{
  return ooLwcGlobal(&obj->p);
}

// Computes whether two lines with radius w both having their origin in some LW coord
// intersects at some point. This can be used for implementing a sweeping collission
// detection system in a large world
void
plLwcIntersectionPoint(const PLlwcoord * restrict a, PLfloat3 da, float wa,
                       const PLlwcoord * restrict b, PLfloat3 db, float wb)
{
  // Transform b to a's segment and compute the intersection point
  //  PLfloat3 bPosInASeg = ooLwcRelVec(b, a->seg);
  //PLfloat3 bNext = vf3_add(bPosInASeg, db);
  //PLfloat3 aNext = vf3_add(a->offs, da);
  
  
}
