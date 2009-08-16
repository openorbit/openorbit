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

#define OFFS_X(c) (((double*)&(c)->offs)[0])
#define OFFS_Y(c) (((double*)&(c)->offs)[1])
#define OFFS_Z(c) (((double*)&(c)->offs)[2])

#define SEG_X(c) (((int16_t*)&(c)->seg)[0])
#define SEG_Y(c) (((int16_t*)&(c)->seg)[1])
#define SEG_Z(c) (((int16_t*)&(c)->seg)[2])

PLshort3
vs3_set(int16_t a, int16_t b, int16_t c)
{
  union {
    PLshort3 v;
    struct {
      int16_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

PLocttree*
plOcttree(double width, int levels)
{
  PLocttree *octtree = malloc(sizeof(PLocttree));
  octtree->level = levels;
  return octtree;
}

void
plLwcNormalise(PLlwcoord *coord)
{
  if (fabs(OFFS_X(coord)) >= PL_CHUNK_RAD) {
    SEG_X(coord) += (int16_t) (OFFS_X(coord) / PL_CHUNK_RAD);
    OFFS_X(coord) = fmod(OFFS_X(coord), PL_CHUNK_RAD);
  }
  if (fabs(OFFS_Y(coord)) >= PL_CHUNK_RAD) {
    SEG_Y(coord) += (int16_t) (OFFS_Y(coord) / PL_CHUNK_RAD);
    OFFS_Y(coord) = fmod(OFFS_Y(coord), PL_CHUNK_RAD);
  }
  if (fabs(OFFS_Z(coord)) >= PL_CHUNK_RAD) {
    SEG_Z(coord) += (int16_t) (OFFS_Z(coord) / PL_CHUNK_RAD);
    OFFS_Z(coord) = fmod(OFFS_Z(coord), PL_CHUNK_RAD);
  }
}

void
plLwcTranslate(PLlwcoord *coord, PLdouble3 offs)
{
  coord->offs = vd3_add(coord->offs, offs);
  plLwcNormalise(coord);
}

PLdouble3
plLwcGlobal(const PLlwcoord *coord)
{
  PLdouble3 p = coord->offs;
  return vd3_add(p, vd3_s_mul(p, PL_CHUNK_RAD));
}

PLdouble3
plLwcRelVec(const PLlwcoord *coord, PLshort3 seg)
{
  PLdouble3 r = coord->offs;
  PLshort3 segdiff = coord->seg - seg;
  PLdouble3 segdiffr = vd3_set((double)((int16_t*)&segdiff)[0],
                               (double)((int16_t*)&segdiff)[1],
                               (double)((int16_t*)&segdiff)[2]);
  r = vd3_add(r, vd3_s_mul(segdiffr, PL_CHUNK_RAD));
  return r;
}

PLdouble3
plLwcDist(const PLlwcoord *a, const PLlwcoord * b)
{
  PLdouble3 diff = vd3_sub(a->offs, b->offs);
  PLshort3 segdiff = a->seg - b->seg;
  PLdouble3 segdiffr = vd3_set((double)((int16_t*)&segdiff)[0],
                               (double)((int16_t*)&segdiff)[1],
                               (double)((int16_t*)&segdiff)[2]);

  return vd3_add(diff, vd3_s_mul(segdiffr, PL_CHUNK_RAD));
}

PLdouble3
plGetObjGlobal3d(PLobject2 *obj)
{
  return plLwcGlobal(&obj->p);
}

PLdouble3
plObjectDistance(PLobject2 *a, PLobject2 *b)
{
  return plLwcDist(&a->p, &b->p);
}

PLobject2*
plObject3d(double x, double y, double z)
{
  PLobject2 *obj = malloc(sizeof(PLobject2));
  obj->p.offs = vd3_set(x, y, z);
  obj->p.seg = vs3_set(0, 0, 0);
  plLwcNormalise(&obj->p);
  return obj;
}

void
plObjectDelete(PLobject2 *obj)
{
  free(obj);
}

void
plTranslateObject3dv(PLobject2 *obj, PLdouble3 dp)
{
  plLwcTranslate(&obj->p, dp);
}

PLdouble3
plObjectGlobal(PLobject2 *obj)
{
  return plLwcGlobal(&obj->p);
}

