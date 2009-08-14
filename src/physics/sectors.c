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

PLocttree*
plOcttree(double width, int levels)
{
  PLocttree *octtree = malloc(sizeof(PLocttree));
  octtree->level = levels;
  return octtree;
}

PLdouble3
plGetObjGlobal3d(PLobject2 *obj)
{
//  double x = obj->x + obj->octtree->x;
//  double y = obj->y + obj->octtree->y;
//  double z = obj->z + obj->octtree->z;
  double x = vd3_get(obj->p, 0) + (double)(obj->i) * PL_CHUNK_LEN;
  double y = vd3_get(obj->p, 1) + (double)(obj->j) * PL_CHUNK_LEN;
  double z = vd3_get(obj->p, 2) + (double)(obj->k) * PL_CHUNK_LEN;

  return vd3_set(x, y, z);
}

PLdouble3
plObjectDistance(PLobject2 *a, PLobject2 *b)
{
//  double dx = (a->x + a->octtree->x) - (b->x + b->octtree->x);
//  double dy = (a->y + a->octtree->y) - (b->y + b->octtree->y);
//  double dz = (a->z + a->octtree->z) - (b->z + b->octtree->z);
  int16_t diff_i = a->i - b->i;
  int16_t diff_j = a->j - b->j;
  int16_t diff_k = a->k - b->k;

  PLdouble3 diff = vd3_sub(a->p, b->p);
  PLdouble3 segDiff = vd3_set((double)diff_i * PL_CHUNK_LEN,
                              (double)diff_j * PL_CHUNK_LEN,
                              (double)diff_k * PL_CHUNK_LEN);
  return vd3_add(segDiff, diff);
}

PLobject2*
plObject3d(double x, double y, double z)
{
  PLobject2 *obj = malloc(sizeof(PLobject2));
  
  int16_t i = x / PL_CHUNK_LEN;
  int16_t j = y / PL_CHUNK_LEN;
  int16_t k = z / PL_CHUNK_LEN;
  PLdouble3 gp = vd3_set(x, y, z);
  PLdouble3 lp = vd3_abs(vd3_s_mod(gp, PL_CHUNK_LEN));
  obj->i = i;
  obj->j = j;
  obj->k = k;
  
  obj->p = lp;
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
  PLdouble3 newPos = vd3_add(obj->p, dp);
  
  int16_t di = vd3_get(newPos, 0) / PL_CHUNK_LEN;
  int16_t dj = vd3_get(newPos, 1) / PL_CHUNK_LEN;
  int16_t dk = vd3_get(newPos, 2) / PL_CHUNK_LEN;

  fprintf(stderr, "old p: %f %f %f\n",
          vd3_get(obj->p, 0),
          vd3_get(obj->p, 1),
          vd3_get(obj->p, 2));
  fprintf(stderr, "delta p: %f %f %f\n",
          vd3_get(dp, 0),
          vd3_get(dp, 1),
          vd3_get(dp, 2));
  fprintf(stderr, "delta ijk: %d %d %d\n",
          (int)di, (int)dj, (int)dk);
  

  PLdouble3 localPos = vd3_s_mod(newPos, PL_CHUNK_LEN);
  obj->p = localPos;
  obj->i += di;
  obj->j += dj;
  obj->k += dk;

  fprintf(stderr, "new p: %f %f %f\n",
          vd3_get(newPos, 0),
          vd3_get(newPos, 1),
          vd3_get(newPos, 2));
  
  fprintf(stderr, "new lp: %f %f %f\n",
          vd3_get(localPos, 0),
          vd3_get(localPos, 1),
          vd3_get(localPos, 2));

#if 0
  // BUG: Cannot translate object more than one sector
  PLdouble3 newPos = vd3_add(obj->p, dp);
  fprintf(stderr, "new: %f %f %f\n", vd3_get(newPos, 0), vd3_get(newPos, 1), vd3_get(newPos, 2));
  PLdouble3 modPos = vd3_s_mod(newPos, PL_CHUNK_RAD);
  fprintf(stderr, "mod: %f %f %f\n", vd3_get(modPos, 0), vd3_get(modPos, 1), vd3_get(modPos, 2));
  PLdouble3 edges = vd3_set(PL_CHUNK_RAD, PL_CHUNK_RAD, PL_CHUNK_RAD);
  fprintf(stderr, "edges: %f %f %f\n", vd3_get(edges, 0), vd3_get(edges, 1), vd3_get(edges, 2));
  
  PLdouble3 negSignedEdges = vd3_copysign(edges, vd3_neg(modPos));
  fprintf(stderr, "negEdges: %f %f %f\n", vd3_get(negSignedEdges, 0), vd3_get(negSignedEdges, 1), vd3_get(negSignedEdges, 2));

  PLdouble3 newPosLocal = vd3_add(negSignedEdges, modPos);
  fprintf(stderr, "newPosLocal: %f %f %f\n", vd3_get(newPosLocal, 0), vd3_get(newPosLocal, 1), vd3_get(newPosLocal, 2));

  if (vd3_get(newPos, 0) > PL_CHUNK_RAD) obj->i ++;
  else if (vd3_get(newPos, 0) < -PL_CHUNK_RAD) obj->i --;

  if (vd3_get(newPos, 1) > PL_CHUNK_RAD) obj->j ++;
  else if (vd3_get(newPos, 1) < -PL_CHUNK_RAD) obj->j --;

  if (vd3_get(newPos, 2) > PL_CHUNK_RAD) obj->k ++;
  else if (vd3_get(newPos, 2) < -PL_CHUNK_RAD) obj->k --;
  obj->p = newPosLocal;
#endif
}

PLdouble3
plObjectGlobal(PLobject2 *obj)
{
  PLdouble3 globalBoxPos = vd3_set((double)obj->i * PL_CHUNK_LEN,
                                   (double)obj->j * PL_CHUNK_LEN,
                                   (double)obj->k * PL_CHUNK_LEN);
  return vd3_add(globalBoxPos, obj->p);
}

