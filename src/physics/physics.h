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
#include "common/lwcoord.h"
#include "rendering/scenegraph.h"

#ifndef __has_feature
#define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif

#include "mass.h"


#if __has_feature(attribute_ext_vector_type)
typedef float __attribute__((ext_vector_type (4))) PLfloat3;
typedef double __attribute__((ext_vector_type (4))) PLdouble3;
typedef int64_t __attribute__((ext_vector_type (4))) PLlong3;
typedef int32_t __attribute__((ext_vector_type (4))) PLint3;
typedef int16_t __attribute__((ext_vector_type (4))) PLshort3;
typedef PLfloat3 __attribute__((ext_vector_type (4))) PLfloat3x3;
#else
typedef float __attribute__((vector_size (16))) PLfloat3;
typedef double __attribute__((vector_size (32))) PLdouble3;
typedef int64_t __attribute__((vector_size (32))) PLlong3;
typedef int32_t __attribute__((vector_size (16))) PLint3;
typedef int16_t __attribute__((vector_size (8))) PLshort3;

#endif

// We cannot cope with the precision issues without subdividing the universe
// If we take 1.0 Tm side boxes, we can maintain decent double precision units and still
// use metres. For now, we use 16 bit integers to identify the box, which means that the
// total volume available is something like +/- 200 au on each side
#define PL_SEGMENT_LEN 1024.0f
#define PL_SEGMENT_LEN64 1024.0

typedef struct PLocttree PLocttree;
typedef struct PLobject2 PLobject2;
typedef struct OOlwcoord PLlwcoord;
typedef struct OOlwcoord64 PLlwcoord64;

/*! Large world coordinate
  Extends the floating point precision coordinates with additional bits yealding us
  a hybrid of floating point and fixed point coordinates.

  The large world coordinate system works by segmenting the space in multiple segments,
  each segment is a fixed width where we can keep a reasonable floating point precision
  intact and working.

  We might convert this back to doubles in the future, depending physics engine
  constraints, the segment size must however continue to be defined for floats and not
  doubles, at least until GPUs start to support doubles for OpenGL.

  Note that the segments are overlapping so you cannot cull collissions with the segment
  id as is. The overlap occurrs with seg N (-1024.0, -0.0] = seg N-1 [0.0, 1024.0)
*/
//struct PLlwcoord {
//  PLfloat3 offs;
//  PLint3 seg;
//};
//void plLwcNormalise(PLlwcoord *coord);
//void plLwcTranslate(PLlwcoord *coord, PLfloat3 offs);
//PLfloat3 plLwcGlobal(const PLlwcoord *coord);
//PLfloat3 plLwcRelVec(const PLlwcoord *coord, PLint3 seg);
//PLfloat3 plLwcDist(const PLlwcoord *a, const PLlwcoord * b);

// Using 64bit ints for the segment numebers give us a world size of
// 1.949864 Mly which should cover almost the entire distance to the andromeda galaxy
// This precision should be sufficient for anyone...
// Note that the 64 bit lwcoords are also clamping the real component to 1000.0 metres,
// this is in order to be compatible with the rendering engine
//struct PLlwcoord64 {
//  PLdouble3 offs;
//  PLlong3 seg;
//};

//void plLwcNormalise64(PLlwcoord64 *coord);
//void plLwcTranslate64(PLlwcoord64 *coord, PLdouble3 offs);
//PLdouble3 plLwcGlobal64(const PLlwcoord64 *coord);
//PLdouble3 plLwcRelVec64(const PLlwcoord64 *coord, PLlong3 seg);
//PLdouble3 plLwcDist64(const PLlwcoord64 *a, const PLlwcoord64 * b);


typedef struct PLobject {
  char *name;
  dBodyID id; // Using ODE at the moment, but this is not really necisary
  PLlwcoord p; // Large world coordinates
  float m;
  OOdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
                        //  PLorbsys *sys;
} PLobject;

PLobject* plObject3f(float x, float y, float z);
void plForce3f(PLobject *obj, float x, float y, float z);
void plForce3d(PLobject *obj, double x, double y, double z);
void plForce3dv(PLobject *obj, PLdouble3 f);

void plForceRelative3f(PLobject *obj, float x, float y, float z);
void plForceRelative3d(PLobject *obj, double x, double y, double z);
void plForceRelative3dv(PLobject *obj, PLdouble3 f);

void plStepObjectf(PLobject *obj, float dt);
void plStepObjectd(PLobject *obj, double dt);

void plNormaliseObject(PLobject *obj);
void plClearObject(PLobject *obj);

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

typedef struct PLorbitingsystem PLorbitingsystem;
typedef struct PLdynamicobj PLdynamicobj;
typedef struct PLheavyobj PLheavyobj;
typedef struct PLorbitingobj PLorbitingobj;

struct PLdynamicobj {
  char *name;
  PLlwcoord p; // Large world coord
  dBodyID id; // ODE identifier
  OOdrawable *drawable; // Drawable identifier
};

// Assumed to have perfect orbits
struct PLorbitingobj {
  char *name;
  double mass;
  // elipsis
};

struct PLheavyobj {
  char *name;
  double mass;
  double radius;
  double groundLevelPreasure;
  double oneHundredKmPreasure;

  PLlwcoord p; // Large world coord
  dBodyID id; // ODE identifier
  OOdrawable *drawable; // Drawable identifier
  // elipsis
};


struct PLorbitingsystem {
  size_t objCount;
  // Elipsis...
  PLdynamicobj **dynamicObjs;
  PLorbitingobj **orbitingObjs;
  PLheavyobj **heavyObjs; // These cause gravity and have atmospheres et.c.
};

struct PLroot {
  size_t systemCount;
  PLorbitingsystem **systems;
};

typedef struct PLstar PLstar;
typedef struct PLplanet PLplanet;
typedef struct PLmoon PLmoon;

struct PLstar {
  size_t planetCount;
  PLplanet **planets;
};

struct PLplanet {
  PLlwcoord p; // wrt orbit base, typically 0,0,0
//  OOellipse *orbit;
  size_t moonCount;
  PLmoon **moons;
};

struct PLmoon {
  PLlwcoord p; // wrt orbit
//  OOellipse *orbit;
  double m;
};

// Unit conversion macros

static inline double
plMetresToAu(double m)
{
  return m / 149598000000.0;
}

static inline PLdouble3
plMetresToAu_v(PLdouble3 m)
{
  return vd3_s_div(m, 149598000000.0);
}


static inline double
plAuToMetres(double au)
{
  return au * 149598000000.0;
}

static inline PLdouble3
plAuToMetres_v(PLdouble3 au)
{
  return vd3_s_mul(au, 149598000000.0);
}


static inline double
plMetresToPc(double m)
{
  return m / 3.08e16;
}

static inline double
plPcToMetres(double pc)
{
  return pc * 3.08e16;
}

static inline PLdouble3
plMetresToPc_v(PLdouble3 m)
{
  return vd3_s_div(m, 3.08e16);
}

static inline PLdouble3
plPcToMetres_v(PLdouble3 pc)
{
  return vd3_s_mul(pc, 3.08e16);
}



static inline double
plPaToBar(double p)
{
  return p / 100000.0;
}

static inline double
plBarToPa(double p)
{
  return p * 100000.0;
}


static inline double
plCToK(double t)
{
  return t + 273.16;
}

static inline double
plKToC(double t)
{
  return t - 273.16;
}

#endif /* end of include guard: PHYSICS_H_MW8CHG7V */
