/*
  Copyright 2008, 2009, 2010 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
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
#include <vmath/lwcoord.h>

#include "physics/reftypes.h"
#include "physics/areodynamics.h"
#include "physics/collision.h"

typedef struct pl_keplerelems_t {
  double ecc;
  double a; // Semi-major
  double b; // Auxillary semi-minor
  double inc;
  double longAsc;
  double argPeri;
  double meanAnomalyOfEpoch;
  quaternion_t qOrbit; // Orbital quaternion, this is derived information
} pl_keplerelems_t ;

struct pl_astrobody_t {
  char *name;
  pl_world_t *world;
  pl_system_t *sys;
  pl_object_t obj;
  double GM;
  PLatmosphere *atm;
  pl_keplerelems_t *kepler;
  //sg_object_t *drawable; //!< Link to scenegraph drawable object representing this
  //                      //!< object.
  //sg_light_t *lightSource; //!< Light source if the object emits light

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

struct pl_system_t {
  pl_world_t *world;
  pl_system_t *parent;
  sg_scene_t *scene;

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

  pl_astrobody_t *orbitalBody; // The body actually orbiting at this point, note that it is
  double orbitalPeriod;
};


struct pl_world_t {
  const char *name;
  pl_system_t *rootSys;
  pl_collisioncontext_t *collCtxt;
  obj_array_t objs; // All objects in world, even ones not placed in subsystems
  obj_array_t partSys; // All particle systems in world
};

pl_world_t* pl_new_world(const char *name, sg_scene_t *sc,
                         double m, double gm, double radius,
                         double siderealPeriod, double obliquity,
                         double eqRadius, double flattening);

pl_system_t* pl_new_root_system(pl_world_t *world, sg_scene_t *sc,
                                const char *name,
                                double m, double gm, double obliquity,
                                double siderealPeriod,
                                double eqRadius, double flattening);


pl_system_t* pl_new_orbit(pl_world_t *world, sg_scene_t *sc, const char *name,
                          double m, double gm,
                          double orbitPeriod, double obliquity,
                          double siderealPeriod,
                          double semiMaj, double semiMin,
                          double inc, double ascendingNode,
                          double argOfPeriapsis,
                          double meanAnomaly,
                          double eqRadius, double flattening);
pl_system_t* pl_new_sub_orbit(pl_system_t *orb, sg_scene_t *sc,
                              const char *name, double m, double gm,
                              double orbitPeriod, double obliquity,
                              double siderealPeriod,
                              double semiMaj, double semiMin,
                              double inc, double ascendingNode,
                              double argOfPeriapsis,
                              double meanAnomaly,
                              double eqRadius, double flattening);

pl_system_t* pl_world_get_system(pl_world_t *world, const char *name);
pl_astrobody_t* pl_world_get_object(pl_world_t *world, const char *name);
float3 pl_astrobody_get_pos(const pl_astrobody_t *obj);
float3 pl_world_get_pos_for_name(const pl_world_t *world, const char *name);
void pl_world_get_pos_for_name3f(const pl_world_t *world, const char *name,
                                 float *x, float *y, float *z);
pl_object_t* pl_astrobody_get_obj(pl_astrobody_t *abody);

float3 pl_compute_current_velocity(pl_astrobody_t *ab);

void pl_world_step(pl_world_t *world, double dt);
void pl_world_clear(pl_world_t *world);

double pl_orbital_period(double a, double GM);
void pl_sys_init(pl_system_t *sys);


#endif /* ! _ORBIT_H_ */
