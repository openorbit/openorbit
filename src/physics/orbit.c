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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "orbit.h"

#include "physics.h"

#include <vmath/vmath.h>
#include "sim.h"
#include "sim/simtime.h"
#include "geo/geo.h"
#include <openorbit/log.h>
#include "parsers/hrml.h"
#include "res-manager.h"
#include "rendering/scenegraph.h"
#include "palloc.h"
#include <vmath/lwcoord.h>

/*
 NOTE: Coordinate system are specified in the normal convention used for mission
       analysis. This means that x is positive towards you, y is positvie to the
       right and z positive going upwards. This is a right handed coordinate
       system. Positive x, in our case points towards the reference point of
       ares on the ecliptic.
 */


struct state_vectors {
  double vx, vy, vz, rx, ry, rz;
};

double
pl_gm(double m0, double m1)
{
  return PL_G * (m0 + m1);
}

double
comp_orbital_period(double semimajor, double g, double m1, double m2)
{
  return 2.0 * M_PI * sqrt(pow(semimajor, 3.0)/(g*(m1 + m2)));
}

double
comp_orbital_period_for_planet(double semimajor)
{
  return sqrt(pow(semimajor, 3.0));
}


/*! Computes the orbital period when there is a dominating object in the system
  \param a Semi-major axis of orbit
  \param GM Gravitational parameter (GM) of orbited body
 */
double
pl_orbital_period(double a, double GM)
{
  return 2.0 * M_PI * sqrt((a*a*a) / GM);
}

double
plMeanMotionFromPeriod(double tau)
{
  return (2.0 * M_PI) / tau;
}

/*! Computes the mean motion when there is a dominating object in the system
 \param a Semi-major axis of orbit
 \param u Gravitational parameter (GM) of orbited body
 */
long double
plMeanMotion(long double u, long double a)
{
  return sqrtl(u/(a*a*a));
}

/*!
  Computes the estimate of the next eccentric anomaly
 \param E_i Eccentric anomaly of previous step, initialise to n*t.
 \param ecc Eccentricity of orbital ellipse
 \param m Mean anomaly
 */
long double
plEccAnomalityStep(long double E_i, long double ecc, long double m)
{
  return E_i - ( (E_i-ecc*sinl(E_i)-m) / (1-ecc*cosl(E_i)) );
}

/*!
  Computes the eccentric anomaly for time t, t = 0 is assumed to be when the
  object pass through its periapsis.

  The method solves this by making a few iterations with newton-rapson, for
  equations, see celestial mechanics chapter in Fortescue, Stark and Swinerd's
  Spacecraft Systems Engineering.

  The eccentric anomaly can be used to solve the position of an orbiting object.

  Note on units: n and t should be compatible, n is composed of the GM and a, GM
  in terms is defined in distance / time, and a is the orbits semi-major axis.
  Thus good units for usage are for example: time = earth days or years, and
  distance = m, km or au.

  \param ecc Eccentricity of orbit
  \param n Mean motion around object
  \param t Absolute time for which we want the eccentric anomaly.
 */
long double
pl_ecc_anomaly(long double ecc, long double n, long double t)
{
  // 7.37 mm accuracy for an object at the distance of the dwarf-planet Pluto
#define ERR_LIMIT 0.000000000001l
  long double meanAnomaly = n * t;

  long double E_1 = plEccAnomalityStep(meanAnomaly, ecc, meanAnomaly);
  long double E_2 = plEccAnomalityStep(E_1, ecc, meanAnomaly);

  long double E_i = E_1;
  long double E_i1 = E_2;
  int i = 0;

  while (fabsl(E_i1-E_i) > ERR_LIMIT) {
    E_i = E_i1;
    E_i1 = plEccAnomalityStep(E_i, ecc, meanAnomaly);
    i ++;

    if (i > 10) {
      log_warn("ecc anomaly did not converge in %d iters, err = %.16f", i, fabs(E_i1-E_i));
      break;
    }
  }

  log_trace("ecc anomaly solved in %d iters", i);
  return E_i1;
#undef ERR_LIMIT
}
quaternion_t
pl_orbital_quaternion(pl_keplerelems_t *kepler)
{
  quaternion_t qasc = q_rot(0.0, 0.0, 1.0, kepler->longAsc);
  quaternion_t qinc = q_rot(1.0, 0.0, 0.0, kepler->inc);
  quaternion_t qaps = q_rot(0.0, 0.0, 1.0, kepler->argPeri);

  quaternion_t q = q_mul(qasc, qinc);
  q = q_mul(q, qaps);
  return q;
}

/*!
 \param a Semi-major axis
 \param b Semi-minor axis
 \param ecc Eccentricity of orbit
 \param GM Gravitational parameter of orbited object GM
 \param t Absolute time in seconds.
 */
float3
plOrbitPosAtTime(pl_keplerelems_t *orbit, double GM, double t)
{
  long double meanMotion = plMeanMotion(GM, orbit->a);
  long double eccAnomaly = pl_ecc_anomaly(orbit->ecc, meanMotion, t);

  /* Compute x, y from anomaly, y is pointing in the direction of the
     periapsis */
  double y = orbit->a * cos(eccAnomaly) - orbit->a * orbit->ecc; // NOTE: on the plane we usually do x = a cos t
  double x = -orbit->b * sin(eccAnomaly); // Since we use y as primary axis, x points downwards

  quaternion_t q = orbit->qOrbit;
  float3 v = vf3_set(x, y, 0.0);
  v = v_q_rot(v, q);

  assert(isfinite(v.x));
  assert(isfinite(v.y));
  assert(isfinite(v.z));
  return v;
}

quaternion_t
plSideralRotationAtTime(pl_astrobody_t *ab, double t)
{
  quaternion_t q = ab->kepler->qOrbit;
  q = q_mul(q, q_rot(1.0, 0.0, 0.0, ab->obliquity));
  double rotations = t/ab->siderealPeriod;
  double rotFrac = fmod(rotations, 1.0);
  quaternion_t z_rot = q_rot(0.0f, 0.0f, 1.0f, rotFrac * 2.0 * M_PI);
  return q_mul(q, z_rot);
}

pl_keplerelems_t*
plNewKeplerElements(double ecc, double a, double inc, double longAsc,
                    double argOfPeriapsis, double meanAnomalyOfEpoch)
{
  pl_keplerelems_t *elems = smalloc(sizeof(pl_keplerelems_t));
  elems->ecc = ecc;
  elems->a = a; // Semi-major
  elems->b = ooGeoComputeSemiMinor(a, ecc); // Auxillary semi-minor
  elems->inc = DEG_TO_RAD(inc);
  elems->longAsc = DEG_TO_RAD(longAsc);
  elems->argPeri = DEG_TO_RAD(argOfPeriapsis);
  elems->meanAnomalyOfEpoch = DEG_TO_RAD(meanAnomalyOfEpoch);
  elems->qOrbit = pl_orbital_quaternion(elems);
  return elems;
}

pl_system_t*
pl_world_get_system(pl_world_t *world, const char *name)
{
  char str[strlen(name)+1];
  strcpy(str, name); // TODO: We do not trust the user, should probably
  //       check alloca result
  char *strp = str;
  char *strTok = strsep(&strp, "/");

  // One level name?
  if (strp == NULL) {
    if (!strcmp(strTok, world->rootSys->name)) {
      return world->rootSys;
    }
    return NULL;
  } else if (strcmp(strTok, world->rootSys->name)) {
    return NULL;
  }

  int idx = 0;
  obj_array_t *vec = &world->rootSys->orbits;

  if (vec->length == 0) {
    return NULL;
  }

  pl_system_t *sys = vec->elems[idx];
  strTok = strsep(&strp, "/");

  while (sys) {
    if (!strcmp(sys->name, strTok)) {
      if (strp == NULL) {
        // At the end of the sys path
        return sys;
      }

      // If this is not the lowest level, go one level down
      strTok = strsep(&strp, "/");

      vec = &sys->orbits;
      idx = 0;
      if (vec->length <= 0) return NULL;
      sys = vec->elems[idx];
    } else {
      if (vec == NULL) return NULL;
      idx ++;
      if (vec->length <= idx) return NULL;
      sys = vec->elems[idx];
    }
  }
  return NULL;
}


pl_astrobody_t*
pl_world_get_object(pl_world_t *world, const char *name)
{
  pl_system_t *sys = pl_world_get_system(world, name);
  if (sys) return sys->orbitalBody;
  return NULL;
}



float3
pl_astrobody_get_pos(const pl_astrobody_t *obj)
{
  return lwc_global(&obj->obj.p);
}

float3
pl_world_get_pos_for_name(const pl_world_t *world, const char *name)
{
  pl_astrobody_t *obj = pl_world_get_object((pl_world_t*)world, name);
  assert(obj);

  return pl_astrobody_get_pos(obj);
}

void
pl_world_get_pos_for_name3f(const pl_world_t *world, const char *name,
                  float *x, float *y, float *z)
{
  assert(world && name && x && y && z);

  float3 p = pl_world_get_pos_for_name(world, name);

  // TODO: Move to clang ext vector support
  *x = vf3_x(p);
  *y = vf3_y(p);
  *z = vf3_z(p);
}

void
pl_normalise_object__(pl_astrobody_t *obj)
{
  lwc_normalise(&obj->obj.p);
}

void
pl_object_clear_(PLobject *obj)
{
  obj->f_ack = vf3_set(0.0f, 0.0f, 0.0f);
  obj->t_ack = vf3_set(0.0f, 0.0f, 0.0f);
}

void
pl_sys_update_current_pos(pl_system_t *sys, double dt)
{
  if (sys->parent) {
    double t = sim_time_get_jd();

    if (sys->orbitalBody->tUpdate > 0) {
      lwc_translate3fv(&sys->orbitalBody->obj.p,
                        vf3_s_mul(sys->orbitalBody->obj.v, dt));
      sys->orbitalBody->tUpdate --;
    } else {
      float3 newPos = plOrbitPosAtTime(sys->orbitalBody->kepler,
                                       sys->parent->orbitalBody->GM,
                                       t*PL_SEC_PER_DAY);

      float3 nextPos = plOrbitPosAtTime(sys->orbitalBody->kepler,
                                        sys->parent->orbitalBody->GM,
                                        t*PL_SEC_PER_DAY + (double)sys->orbitalBody->orbitFixationPeriod * dt);

      float3 vel = vf3_s_div(vf3_sub(nextPos, newPos),
                             (double)sys->orbitalBody->orbitFixationPeriod * dt);

      sys->orbitalBody->obj.p = sys->parent->orbitalBody->obj.p;
      lwc_translate3fv(&sys->orbitalBody->obj.p, newPos);

      sys->orbitalBody->obj.v = sys->parent->orbitalBody->obj.v + vel;
      // Reset tUpdate;
      sys->orbitalBody->tUpdate = sys->orbitalBody->orbitFixationPeriod;
    }
    sys->orbitalBody->obj.q = plSideralRotationAtTime(sys->orbitalBody, t);
  } else {
    lwc_set(&sys->orbitalBody->obj.p, 0.0, 0.0, 0.0);
  }
}

// Note that the position can only be changed for an object that is not the root
// root is by def not orbiting anything
void
pl_sys_set_current_pos(pl_system_t *sys)
{
  if (sys->parent) {
    double t = sim_time_get_jd();
    float3 newPos = plOrbitPosAtTime(sys->orbitalBody->kepler,
                                     sys->parent->orbitalBody->GM,
                                     t*PL_SEC_PER_DAY);
    sys->orbitalBody->obj.p = sys->parent->orbitalBody->obj.p;
    lwc_translate3fv(&sys->orbitalBody->obj.p, newPos);

    sys->orbitalBody->obj.q = plSideralRotationAtTime(sys->orbitalBody, t);
    sys->orbitalBody->tUpdate = 0;
  }
}

void
pl_sys_delete(pl_system_t *sys)
{
  for (size_t i = 0 ; i < sys->orbits.length ; i ++) {
    pl_sys_delete(sys->orbits.elems[i]);
  }

  //ooGeoEllipseFree(sys->orbitalPath);
  free((char*)sys->name);
  free(sys);
}

void
pl_world_delete(pl_world_t *world)
{
  pl_sys_delete(world->rootSys);

  free((char*)world->name);
  free(world);
}

/*!
  Creates a new object
  All objects, even the small ones have a GM value

  \param world The world in which the object is placed
  \param name Name of object
  \param m Mass of object in kg
  \param gm Standard gravitational parameter for object (GM), if set to NAN, the
            value will be calculated from m. This allow you to enter more exact
            values, that will not be subject to innacurate floating point
            calculations.
  \param coord The large world coordinate of the object
 */
pl_astrobody_t*
pl_new_obj(pl_world_t*world, const char *name, double m, double gm,
         lwcoord_t * coord,
         quaternion_t q, double siderealPeriod, double obliquity,
         double radius, double flattening)
{
  pl_astrobody_t *obj = smalloc(sizeof(pl_astrobody_t));

  pl_object_init(&obj->obj);
  obj->obj.p.offs = coord->offs;
  obj->obj.p.seg = coord->seg;
  obj->obj.q = q;
  obj->obj.radius = radius;
  obj->obj.name = obj->name = strdup(name);
  obj->sys = NULL;
  obj->world = world;

  // TODO: Ensure quaternion is set for orbit
  //       asc * inc * obl ?

  pl_collide_insert_object(world->collCtxt, &obj->obj);

  obj->obj.m.m = m;

  if (isnormal(gm)) {
    obj->GM = gm;
  } else {
    obj->GM = m * PL_G;
  }
  obj->kepler = NULL;
  obj->eqRad = radius;

  // flattening = ver(angEcc) = 1 - cos(angEcc) => angEcc = acos(1 - flattening)
  obj->angEcc = acos(1.0 - flattening);
  obj->obliquity = DEG_TO_RAD(obliquity);
  obj->siderealPeriod = siderealPeriod;

  // Used for smothening the orbits, this is a rather ugly hack, but should work for now
  obj->tUpdate = 0;
  obj->orbitFixationPeriod = 1000;
  return obj;
}

pl_astrobody_t*
pl_new_obj_in_sys(pl_system_t *sys, const char *name, double m, double gm,
              lwcoord_t *coord, quaternion_t q, double siderealPeriod, double obliquity,
              double radius, double flattening)
{
  pl_astrobody_t *obj = pl_new_obj(sys->world, name, m, gm, coord,
                              q, siderealPeriod, obliquity,
                              radius, flattening);
  obj->sys = sys;

  obj_array_push(&sys->astroObjs, obj);
  return obj;
}


pl_world_t*
pl_new_world(const char *name, sg_scene_t *sc,
           double m, double gm, double radius, double siderealPeriod,
           double obliquity, double eqRadius, double flattening)
{
  pl_world_t *world = smalloc(sizeof(pl_world_t));

  world->name = strdup(name);
  world->collCtxt = pl_new_collision_context();

  world->rootSys = pl_new_root_system(world, sc, name, m, gm,
                                   obliquity, siderealPeriod,
                                   eqRadius, flattening);

  obj_array_init(&world->objs);
  obj_array_init(&world->partSys);
  return world;
}

pl_system_t*
pl_new_orbital_object(pl_world_t *world, sg_scene_t *scene, const char *name,
                      double m, double gm,
                      double orbitPeriod,
                      double obliquity, double siderealPeriod,
                      double semiMaj, double semiMin,
                      double inc, double ascendingNode, double argOfPeriapsis,
                      double meanAnomaly,
                      double eqRadius, double flattening)
{
  assert(world);

  pl_system_t *sys = smalloc(sizeof(pl_system_t));
  obj_array_init(&sys->orbits);
  obj_array_init(&sys->astroObjs);
  obj_array_init(&sys->rigidObjs);

  sys->name = strdup(name);
  sys->world = world;
  sys->scene = scene;
  sys->parent = NULL;

  sys->orbitalPeriod = orbitPeriod;

  // TODO: Stack allocation based on untrusted length should not be here
  char orbitName[strlen(name) + strlen(" Orbit") + 1];
  strcpy(orbitName, name); // safe as size is checked in allocation
  strcat(orbitName, " Orbit");

  lwcoord_t p;
  lwc_set(&p, 0.0, 0.0, 0.0);

  // TODO: Cleanup this quaternion
  // Z-X-Z for euler angles
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(ascendingNode));
  q = q_mul(q, q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(inc)));
  q = q_mul(q, q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(argOfPeriapsis)));
  // TODO: Correct axis?
  q = q_mul(q, q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(obliquity)));

  sys->orbitalBody = pl_new_obj_in_sys(sys/*world->rootSys*/, name, m, gm, &p,
                                   q, siderealPeriod, obliquity,
                                   eqRadius, flattening);
  sys->orbitalBody->kepler = plNewKeplerElements(sqrt((semiMaj*semiMaj-semiMin*semiMin)/(semiMaj*semiMaj)),
                                                 semiMaj, inc, ascendingNode,
                                                 argOfPeriapsis, meanAnomaly);

  return sys;
}

pl_system_t*
pl_new_root_system(pl_world_t *world, sg_scene_t *sc, const char *name, double m,
                   double gm, double obliquity, double siderealPeriod,
                   double eqRadius, double flattening)
{
  assert(world);

  pl_system_t *sys = smalloc(sizeof(pl_system_t));
  obj_array_init(&sys->orbits);
  obj_array_init(&sys->astroObjs);
  obj_array_init(&sys->rigidObjs);

  sys->name = strdup(name);
  sys->world = world;
  sys->parent = NULL;
  sys->scene = sc;
  lwcoord_t p;
  lwc_set(&p, 0.0, 0.0, 0.0);
  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(obliquity));

  sys->orbitalBody = pl_new_obj(world, name, m, gm, &p, q, siderealPeriod, obliquity,
                              eqRadius, flattening);
  sys->orbitalBody->kepler = NULL;

  world->rootSys = sys;
  pl_sys_set_current_pos(sys);
  return sys;
}



pl_system_t*
pl_new_orbit(pl_world_t *world, sg_scene_t *sc, const char *name,
             double m, double gm,
             double orbitPeriod, double obliquity, double siderealPeriod,
             double semiMaj, double semiMin,
             double inc, double ascendingNode, double argOfPeriapsis,
             double meanAnomaly, double eqRadius, double flattening)
{
  assert(world);
  return pl_new_sub_orbit(world->rootSys, sc, name,
                       m, gm, orbitPeriod, obliquity, siderealPeriod,
                       semiMaj, semiMin,
                       inc, ascendingNode, argOfPeriapsis, meanAnomaly,
                       eqRadius, flattening);
}

pl_system_t*
pl_new_sub_orbit(pl_system_t *parent, sg_scene_t *sc, const char *name,
              double m, double gm,
              double orbitPeriod, double obliquity, double siderealPeriod,
              double semiMaj, double semiMin,
              double inc, double ascendingNode, double argOfPeriapsis,
              double meanAnomaly,
              double eqRadius, double flattening)
{
  assert(parent);
  assert(parent->world);

  pl_system_t * sys = pl_new_orbital_object(parent->world, sc,
                                         name, m, gm, orbitPeriod, obliquity, siderealPeriod,
                                         semiMaj, semiMin,
                                         inc, ascendingNode, argOfPeriapsis, meanAnomaly,
                                         eqRadius, flattening);
  sys->parent = parent;
  obj_array_push(&parent->orbits, sys);
  pl_sys_set_current_pos(sys);

  return sys;
}

void
pl_sys_clear(pl_system_t *sys)
{
  for (size_t i = 0; i < sys->rigidObjs.length ; i ++) {
    pl_object_clear_(sys->rigidObjs.elems[i]);
  }

  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    pl_sys_clear(sys->orbits.elems[i]);
  }
}

void
pl_world_clear(pl_world_t *world)
{
  pl_sys_clear(world->rootSys);
}

float3
pl_compute_gravity(pl_astrobody_t *a, PLobject *b)
{
  float3 dist = lwc_dist(&b->p, &a->obj.p);
  double r12 = vf3_abs_square(dist);
  float3 ndist = vf3_normalise(dist);
  float3 f12 = vf3_s_mul(ndist,
                         //-PL_G * (a->obj.m.m * b->m.m / r12));
                         -a->GM * b->m.m / r12);

  assert(isfinite(dist.x));
  assert(isfinite(dist.y));
  assert(isfinite(dist.z));

  assert(isfinite(ndist.x));
  assert(isfinite(ndist.y));
  assert(isfinite(ndist.z));

  assert(isfinite(r12));

  assert(isfinite(f12.x));
  assert(isfinite(f12.y));
  assert(isfinite(f12.z));

  return f12;
}

void
pl_sys_step(pl_system_t *sys, double dt)
{
  // Add gravitational forces
  for (size_t i = 0; i < sys->rigidObjs.length ; i ++) {
    PLobject *obj = sys->rigidObjs.elems[i];
    PL_CHECK_OBJ(obj);
    float3 f12 = pl_compute_gravity(sys->orbitalBody, obj);
    pl_object_set_gravity3fv(obj, f12);
    pl_object_force3fv(obj, f12);
    PL_CHECK_OBJ(obj);

    if (sys->parent) {
      f12 = pl_compute_gravity(sys->parent->orbitalBody, obj);
      pl_object_set_gravity3fv(obj, f12);
      pl_object_force3fv(obj, f12);
      PL_CHECK_OBJ(obj);
    }

    float3 drag = pl_object_compute_drag(obj);
    pl_object_force3fv(obj, drag);
    PL_CHECK_OBJ(obj);

    pl_object_step(obj, dt);
  }

  pl_sys_update_current_pos(sys, dt);

  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    pl_sys_step(sys->orbits.elems[i], dt);
  }
}

void
pl_sys_init(pl_system_t *sys)
{
  pl_sys_set_current_pos(sys);
  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    pl_sys_init(sys->orbits.elems[i]);
  }
}

void
pl_world_step(pl_world_t *world, double dt)
{
  pl_sys_step(world->rootSys, dt);

  //for (size_t i = 0; i < world->objs.length ; i ++) {
  //  PLobject *obj = world->objs.elems[i];
    //if (obj->drawable) {
    //  sgSetObjectPosLW(obj->drawable, &obj->p);
    //  sgSetObjectQuatv(obj->drawable, obj->q);
    //}
    //}

  for (size_t i = 0; i < world->partSys.length ; ++ i) {
    pl_particles_t *psys = world->partSys.elems[i];
    pl_particles_step(psys, dt);
  }

  pl_collide_step(world->collCtxt);
}

PLobject*
pl_astrobody_get_obj(pl_astrobody_t *abody)
{
  return &abody->obj;
}

// Ugly, but works for now...
float3
pl_compute_current_velocity(pl_astrobody_t *ab)
{
  assert(ab->sys->parent != NULL);
  double t = sim_time_get_jd();

  float3 currentPos = plOrbitPosAtTime(ab->kepler,
                                       ab->sys->parent->orbitalBody->GM,
                                       t*PL_SEC_PER_DAY);
  float3 nextPos = plOrbitPosAtTime(ab->kepler,
                                    ab->sys->parent->orbitalBody->GM,
                                    t*PL_SEC_PER_DAY+1000.0f);

  float3 v = nextPos - currentPos;
  v = vf3_s_div(v, 1000.0f);

  return v;
}
