/*
  Copyright 2008, 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*
    Large orbital objects
*/
#ifndef _ORBIT_H_
#define _ORBIT_H_

#include <ode/ode.h>
#include <openorbit/openorbit.h>
#include <vmath/vmath.h>
#include "physics.h"
#include "geo/geo.h"
#include "rendering/scenegraph.h"


static inline v4f_t
ode2v4(const dReal *vec)
{
  return v4f_make(vec[0], vec[1], vec[2], vec[3]);
}

static inline v4f_t
ode2v3(const dReal *vec)
{
  return v4f_make(vec[0], vec[1], vec[2], 1.0f);
}

typedef struct PLworld__ PLworld;
typedef struct PLorbsys__ PLorbsys;
typedef struct PLobject__ PLobject__;

typedef struct PL_keplerian_elements {
  double ecc;
  double a; // Semi-major
  double b; // Auxillary semi-minor
  double inc;
  double longAsc;
  double argPeri;
  double meanAnomalyOfEpoch;
} PL_keplerian_elements ;

struct PLobject__ {
  char *name;
  PLworld *world;
  PLorbsys *sys;
  dBodyID id; // Using ODE at the moment, but this is not really necisary
  PLlwcoord p; // Large world coordinates
  double m;
  double GM;
  PL_keplerian_elements *kepler;
  OOdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
};


struct PLorbsys__ {
  PLworld *world;
  PLorbsys *parent;

  const char *name;

  obj_array_t orbits; // suborbits
  obj_array_t objs; // objects in this system

  PLobject__ *orbitalBody; // The body actually orbiting at this point, note that it is
  double orbitalPeriod;
  OOellipse *orbitalPath; // Contains the actual ellipsis for the orbit
  OOdrawable *orbitDrawable; // Pointer to the drawable representing the ellipsis
};

struct PLworld__ {
  dWorldID world;
  const char *name;
  OOscene *scene; // Scene in the sg
  obj_array_t orbits;
  obj_array_t objs;
  PLobject__ *centralBody;
  SGlight *centralLightSource;
};

PLworld* plNewWorld(const char *name, OOscene *sc,
                    double m, double gm, double radius,
                    double siderealPeriod, double obliquity);

PLorbsys* plNewOrbit(PLworld *world, const char *name, double m, double gm,
                     double orbitPeriod, double obliquity,
                     double semiMaj, double semiMin,
                     double inc, double ascendingNode, double argOfPeriapsis,
                     double meanAnomaly);
PLorbsys* plNewSubOrbit(PLorbsys *orb, const char *name, double m, double gm,
                        double orbitPeriod, double obliquity,
                        double semiMaj, double semiMin,
                        double inc, double ascendingNode, double argOfPeriapsis,
                        double meanAnomaly);

PLobject__* plGetObject(PLworld *world, const char *name);
float3 plGetPos(const PLobject__ *obj);
float3 plGetPosForName(const PLworld *world, const char *name);
void plGetPosForName3f(const PLworld *world, const char *name,
                       float *x, float *y, float *z);


/*!
 Loads an hrml description of a solar system and builds a solar system graph
 it also connects the physics system to the graphics system.
 
 This function does not belong in the physics system, but will be here for
 now beeing.
 */
PLworld* ooOrbitLoad(OOscenegraph *sg, const char *fileName);

void plWorldStep(PLworld *world, double dt);
void plWorldClear(PLworld *world);



#endif /* ! _ORBIT_H_ */
