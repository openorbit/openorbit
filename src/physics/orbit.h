/*
  Copyright 2008, 2009, 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdlib.h>
#include <string.h>


#include <vmath/vmath.h>

#include "geo/geo.h"
#include "rendering/scenegraph.h"
#include "common/lwcoord.h"
#include "rendering/reftypes.h"

#include "physics/reftypes.h"
#include "physics/areodynamics.h"
#include "physics/collision.h"

typedef struct PL_keplerian_elements {
  double ecc;
  double a; // Semi-major
  double b; // Auxillary semi-minor
  double inc;
  double longAsc;
  double argPeri;
  double meanAnomalyOfEpoch;
  quaternion_t qOrbit; // Orbital quaternion, this is derived information
} PL_keplerian_elements ;

struct PLastrobody {
  char *name;
  PLworld *world;
  PLsystem *sys;
  PLobject obj;
  double GM;
  PLatmosphere *atm;
  PL_keplerian_elements *kepler;
  SGdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
  SGlight *lightSource; //!< Light source if the object emits light

  // These parameters are used for positioning of objects, they assist
  // calculations where astronomical bodies are assumed to be spheroids. These
  // may also be used for irregular objects in order calculate an approximate
  // gravity field or an approximate latitude, longitude position.
  double eqRad; // Equatorial radius
  double angEcc; // Angular eccentricity of spheroid due to flattening

  // Important parameters for computing rotation exact
  double obliquity;
  double siderealPeriod;

  int tUpdate; // How many steps to next update
  int orbitFixationPeriod; // How many steps between updates
};

struct PLsystem {
  PLworld *world;
  PLsystem *parent;
  SGscene *scene;

  const char *name;
  double effectiveRadius; //!< Radius of the entire system, i.e how far away
                          //!< objects will still be considered to be under its
                          //!< influence. This is set to two times the radius of
                          //!< the largest orbit in the system (in case the
                          //!< system has sattelites), or to the distance it
                          //!< takes for the gravitational influence to diminish
                          //!< to XXX, which ever is greater.

  // TODO: These should be in some kind of oct-tree type structure
  obj_array_t orbits; // suborbits
  obj_array_t astroObjs; // objects in this system
  obj_array_t rigidObjs; // objects in this system

  PLastrobody *orbitalBody; // The body actually orbiting at this point, note that it is
  double orbitalPeriod;
  SGdrawable *orbitDrawable; // Pointer to the drawable representing the ellipsis
};


struct PLworld {
  const char *name;
  PLsystem *rootSys;
  PLcollisioncontext *collCtxt;
  obj_array_t objs; // All objects in world, even ones not placed in subsystems
  obj_array_t partSys; // All particle systems in world
};

PLworld* plNewWorld(const char *name, SGscene *sc,
                    double m, double gm, double radius,
                    double siderealPeriod, double obliquity,
                    double eqRadius, double flattening);

PLsystem* plNewRootSystem(PLworld *world, SGscene *sc, const char *name,
                          double m, double gm, double obliquity, double siderealPeriod,
                          double eqRadius, double flattening);


PLsystem* plNewOrbit(PLworld *world, SGscene *sc, const char *name, double m, double gm,
                     double orbitPeriod, double obliquity, double siderealPeriod,
                     double semiMaj, double semiMin,
                     double inc, double ascendingNode, double argOfPeriapsis,
                     double meanAnomaly,
                     double eqRadius, double flattening);
PLsystem* plNewSubOrbit(PLsystem *orb, SGscene *sc, const char *name, double m, double gm,
                        double orbitPeriod, double obliquity, double siderealPeriod,
                        double semiMaj, double semiMin,
                        double inc, double ascendingNode, double argOfPeriapsis,
                        double meanAnomaly,
                        double eqRadius, double flattening);

PLsystem* plGetSystem(PLworld *world, const char *name);
PLastrobody* plGetObject(PLworld *world, const char *name);
float3 plGetPos(const PLastrobody *obj);
float3 plGetPosForName(const PLworld *world, const char *name);
void plGetPosForName3f(const PLworld *world, const char *name,
                       float *x, float *y, float *z);
PLobject* plObjForAstroBody(PLastrobody *abody);

float3 plComputeCurrentVelocity(PLastrobody *ab);

void plWorldStep(PLworld *world, double dt);
void plWorldClear(PLworld *world);

double plOrbitalPeriod(double a, double GM);
void plSetDrawable(PLastrobody *obj, SGdrawable *drawable);
void plSysInit(PLsystem *sys);


#endif /* ! _ORBIT_H_ */
