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

#ifndef PHYSICS_H_MW8CHG7V
#define PHYSICS_H_MW8CHG7V
#include <ode/ode.h>
#include <vmath/vmath.h>
#include "rendering/scenegraph.h"

typedef float __attribute__((vector_size (16))) PLfloat3;
typedef double __attribute__((vector_size (32))) PLdouble3;
typedef int64_t __attribute__((vector_size (32))) PLlong3;
typedef int32_t __attribute__((vector_size (16))) PLint3;
typedef int16_t __attribute__((vector_size (8))) PLshort3;

typedef struct PLorbsys PLorbsys;

typedef struct PLobject {
  char *name;
  dBodyID id;
  float m;
  OOdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
  PLorbsys *sys;
} PLobject;


// We cannot cope with the precision issues without subdividing the universe
// If we take 1.0 Tm side boxes, we can maintain decent double precision units and still
// use metres. For now, we use 16 bit integers to identify the box, which means that the
// total volume available is something like +/- 200 au on each side
#define PL_CHUNK_LEN 1000000000.0
#define PL_CHUNK_RAD  500000000.0
#define PL_SEGMENT_LEN 1024.0f
typedef struct PLocttree PLocttree;
typedef struct PLobject2 PLobject2;
typedef struct PLlwcoord PLlwcoord;

struct PLlwcoord {
  PLfloat3 offs;
  PLint3 seg;
};

struct PLobject2 {
  struct PLobject2 *next;
  struct PLobject2 *previous;

  PLocttree *octtree;
  char *name;
  dBodyID id;
  float m;
  OOdrawable *drawable;
  
  PLlwcoord p; // Coords for objects within the node (in metres)
               // Position (node in solar system, 1 000 000.0 km chunks), 16 bit
               // integers here gives us a volume of 438 au on each side
               // unfortunatelly 16 bit integers cannot cover the oort cloud as it reach
               // out a couple of thousand au's, but we can change the index to 32 bit
               // later if needed (this would give us 453 ly or for 64 bit 598 Gpc)
               // note that each volume is between +/- 500 Gm
  short boxradius; // How many adjacent boxes does this object fill up (normally 1, but the
                   // sun would use 2)
};

// 50000.0f
// For n-body simulation
struct PLocttree {
  struct PLocttree* child[2][2][2];
  double cogx, cogy, cogz;
  int level;
  PLobject2 *objects;
};

PLobject2* plObject3d(double x,double y, double z);
void plObjectDelete(PLobject2 *obj);
PLfloat3 plObjectDistance(PLobject2 *a, PLobject2 *b);

void plTranslateObject3fv(PLobject2 *obj, PLfloat3 dp);

#endif /* end of include guard: PHYSICS_H_MW8CHG7V */
