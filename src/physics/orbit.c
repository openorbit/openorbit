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

#include "orbit.h"
#include <ode/ode.h>
#include <assert.h>
#include "physics.h"

#include <vmath/vmath.h>
#include "sim.h"
#include "sim/simtime.h"
#include "geo/geo.h"
#include "log.h"
#include "parsers/hrml.h"
#include "res-manager.h"
#include "rendering/scenegraph.h"
#include "rendering/scenegraph-private.h"

#include "common/lwcoord.h"

/// Gravitational constant in m^3/kg/s^2
#define PL_GRAVITATIONAL_CONST 6.67428e-11
#define PL_SEC_PER_DAY (3600.0 * 24.0)
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
plGm(double m0, double m1)
{
  return PL_GRAVITATIONAL_CONST * (m0 + m1);
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
plOrbitalPeriod(double a, double GM)
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
double
plMeanMotion(double u, double a)
{
  return sqrt(u/(a*a*a));
}

/*!
  Computes the estimate of the next eccentric anomaly
 \param E_i Eccentric anomaly of previous step, initialise to n*t.
 \param ecc Eccentricity of orbital ellipse
 \param m Mean anomaly
 */
double
plEccAnomalityStep(double E_i, double ecc, double m)
{
  return E_i - ( (E_i-ecc*sin(E_i)-m) / (1-ecc*cos(E_i)) );
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
double
plEccAnomaly(double ecc, double n, double t)
{
  // 7.37 mm accuracy for an object at the distance of the dwarf-planet Pluto
#define ERR_LIMIT 0.000000000000001
  double meanAnomaly = n * t;

  double E_1 = plEccAnomalityStep(meanAnomaly, ecc, meanAnomaly);
  double E_2 = plEccAnomalityStep(E_1, ecc, meanAnomaly);

  double E_i = E_1;
  double E_i1 = E_2;
  int i = 0;

  while (fabs(E_i1-E_i) > ERR_LIMIT) {
    E_i = E_i1;
    E_i1 = plEccAnomalityStep(E_i, ecc, meanAnomaly);
    i ++;

    if (i > 10) {
      ooLogWarn("ecc anomaly did not converge in %d iters, err = %f", i, fabs(E_i1-E_i));
      break;
    }
  }

  ooLogTrace("ecc anomaly solved in %d iters", i);
  return E_i1;
#undef ERR_LIMIT
}
quaternion_t
plOrbitalQuaternion(PL_keplerian_elements *kepler)
{
  quaternion_t qasc = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(kepler->longAsc));
  quaternion_t qinc = q_rot(0.0, 1.0, 0.0, DEG_TO_RAD(kepler->inc));
  quaternion_t qaps = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(kepler->argPeri));

  quaternion_t q = q_mul(qasc, qinc);
  q = q_mul(q, qaps);
  return q;
}

/*!
 \param a Semi-major axis
 \param b Semi-minor axis
 \param ecc Eccentricity of orbit
 \param GM Gravitational parameter of orbited object GM
 \param t Absolute time.
 */
float3
plOrbitPosAtTime(PL_keplerian_elements *orbit, double GM, double t)
{
  double meanMotion = plMeanMotion(GM, orbit->a);
  double eccAnomaly = plEccAnomaly(orbit->ecc, meanMotion, t);

  /* Compute x, y from anomaly, y is pointing in the direction of the
     periapsis */
  double y = orbit->a * cos(eccAnomaly) - orbit->a * orbit->ecc; // NOTE: on the plane we usually do x = a cos t
  double x = -orbit->b * sin(eccAnomaly); // Since we use y as primary axis, x points downwards

  quaternion_t q = plOrbitalQuaternion(orbit);
  matrix_t m;
  q_m_convert(&m, q);
  float3 v = vf3_set(x, y, 0.0);
  v = m_v3_mulf(&m, v);

  return v;
}

PL_keplerian_elements*
plNewKeplerElements(double ecc, double a, double inc, double longAsc,
                    double argOfPeriapsis, double meanAnomalyOfEpoch)
{
  PL_keplerian_elements *elems = malloc(sizeof(PL_keplerian_elements));
  elems->ecc = ecc;
  elems->a = a; // Semi-major
  elems->b = ooGeoComputeSemiMinor(a, ecc); // Auxillary semi-minor
  elems->inc = inc;
  elems->longAsc = longAsc;
  elems->argPeri = argOfPeriapsis;
  elems->meanAnomalyOfEpoch = meanAnomalyOfEpoch;
  return elems;
}

PLobject__*
plGetObject(PLworld *world, const char *name)
{
  char str[strlen(name)+1];
  strcpy(str, name); // TODO: We do not trust the user, should probably
                     //       check alloca result
  char *strp = str;
  char *strTok = strsep(&strp, "/");

  // One level name?
  if (strp == NULL) {
    if (!strcmp(strTok, world->centralBody->name)) {
      return world->centralBody;
    }
    return NULL;
  } else if (strcmp(strTok, world->centralBody->name)) {
    return NULL;
  }

  int idx = 0;
  obj_array_t *vec = &world->orbits;

  if (vec->length == 0) {
    return NULL;
  }

  PLorbsys *sys = vec->elems[idx];
  strTok = strsep(&strp, "/");

  while (sys) {
    if (!strcmp(sys->name, strTok)) {
      if (strp == NULL) {
        // At the end of the sys path
        return sys->orbitalBody;
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

float3
plGetPos(const PLobject__ *obj)
{
  return ooLwcGlobal(&obj->p);
}

float3
plGetPosForName(const PLworld *world, const char *name)
{
  PLobject__ *obj = plGetObject((PLworld*)world, name);
  assert(obj);

  return plGetPos(obj);
}

void
plGetPosForName3f(const PLworld *world, const char *name,
                  float *x, float *y, float *z)
{
  assert(world && name && x && y && z);

  float3 p = plGetPosForName(world, name);

  // TODO: Move to clang ext vector support
  *x = vf3_get(p, 0);
  *y = vf3_get(p, 1);
  *z = vf3_get(p, 2);
}

void
plNormaliseObject__(PLobject__ *obj)
{
  // Since we are using non safe casts here, we must ensure this in case someone upgrades
  // to 64 bit positions
  assert(sizeof(obj->p.offs) == sizeof(PLfloat3));

  const dReal *pos = dBodyGetPosition(obj->id);
  ((float*)&obj->p.offs)[0] = pos[0];
  ((float*)&obj->p.offs)[1] = pos[1];
  ((float*)&obj->p.offs)[2] = pos[2];

  ooLwcNormalise(&obj->p);

  dBodySetPosition(obj->id,
                  ((float*)&obj->p.offs)[0],
                  ((float*)&obj->p.offs)[1],
                  ((float*)&obj->p.offs)[2]);
}

void
plClearObject__(PLobject__ *obj)
{
  dBodySetPosition(obj->id,
                  ((float*)&obj->p.offs)[0],
                  ((float*)&obj->p.offs)[1],
                  ((float*)&obj->p.offs)[2]);
  dBodySetForce(obj->id, 0.0, 0.0, 0.0);
  dBodySetTorque (obj->id, 0.0, 0.0, 0.0);
}


void
plUpdateObject(dBodyID body)
{
  PLobject__ *obj = dBodyGetData(body);
  ooLogTrace("updating body %s", obj->name);

  //const dReal *rot = dBodyGetRotation(body);
  const dReal *quat = dBodyGetQuaternion(body);
  const dReal *linVel = dBodyGetLinearVel(body);
  const dReal *angVel = dBodyGetAngularVel(body);

  plNormaliseObject__(obj);

  // ooSgSetObjectPosLW(obj->drawable, &obj->p);
  // ooSgSetObjectQuat(obj->drawable, quat[1], quat[2], quat[3], quat[0]);
  // ooSgSetObjectSpeed(obj->drawable, linVel[0], linVel[1], linVel[2]);
  // ooSgSetObjectAngularSpeed(obj->drawable, angVel[0], angVel[1], angVel[2]);
}

void
plSysSetCurrentPos(PLorbsys *sys)
{
  if (sys->parent) {
    float3 newPos= plOrbitPosAtTime(sys->orbitalBody->kepler,
                                    sys->parent->orbitalBody->GM + sys->orbitalBody->GM,
                                    ooTimeGetJD()*PL_SEC_PER_DAY);
    sys->orbitalBody->p = sys->parent->orbitalBody->p;
    ooLwcTranslate(&sys->orbitalBody->p, newPos);
  } else {
    float3 newPos= plOrbitPosAtTime(sys->orbitalBody->kepler,
                                    sys->world->centralBody->GM + sys->orbitalBody->GM,
                                    ooTimeGetJD()*PL_SEC_PER_DAY);

    double meanMotion = plMeanMotion(sys->world->centralBody->GM + sys->orbitalBody->GM,
                                     sys->orbitalBody->kepler->a);
    double eccAnomaly = plEccAnomaly(sys->orbitalBody->kepler->ecc, meanMotion, ooTimeGetJD()*PL_SEC_PER_DAY);
    double eccDeg = RAD_TO_DEG(eccAnomaly);

    /* Compute x, y from anomaly, y is pointing in the direction of the
     periapsis, x is pointing downwards */
    double y = sys->orbitalBody->kepler->a * cos(eccAnomaly) - sys->orbitalBody->kepler->a * sys->orbitalBody->kepler->ecc; // NOTE: on the plane we usually do x = a cos t
    double x = -sys->orbitalBody->kepler->b * sin(eccAnomaly);


    sys->orbitalBody->p = sys->world->centralBody->p;
    ooLwcTranslate(&sys->orbitalBody->p, newPos);

    //  if (sys->orbitDrawable) {
    //  fprintf(stderr, "el: %f %f\n",
    //         ((SGellipsis*)(sys->orbitDrawable))->verts[0],
    //          ((SGellipsis*)(sys->orbitDrawable))->verts[1]);
    //}
  }
}

void
plDeleteSys(PLorbsys *sys)
{
  for (size_t i = 0 ; i < sys->orbits.length ; i ++) {
    plDeleteSys(sys->orbits.elems[i]);
  }

  //ooGeoEllipseFree(sys->orbitalPath);
  free((char*)sys->name);
  free(sys);
}

void
plDeleteWorld(PLworld *world)
{
  for (size_t i = 0; i < world->orbits.length ; i ++) {
    plDeleteSys(world->orbits.elems[i]);
  }

  free((char*)world->name);
  free(world);
}

void
plSetDrawable(PLobject__ *obj, OOdrawable *drawable)
{
  obj->drawable = drawable;
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
PLobject__*
plNewObj(PLworld*world, const char *name, double m, double gm,
         OOlwcoord * coord, quaternion_t q)
{
  PLobject__ *obj = malloc(sizeof(PLobject__));
  obj->p = *coord;
  obj->name = strdup(name);
  obj->sys = NULL;
  obj->world = world;
  obj->id = dBodyCreate(world->world);
  dBodySetGravityMode(obj->id, 0); // Ignore standard ode gravity effects
  dBodySetData(obj->id, obj);
  dBodySetMovedCallback(obj->id, plUpdateObject);
  dQuaternion quat = {vf4_w(q), vf4_x(q), vf4_y(q), vf4_z(q)};
  // TODO: Ensure quaternion is set for orbit
  //       asc * inc * obl ?

  dBodySetQuaternion(obj->id, quat);
  dBodySetAngularVel(obj->id, 0.0, 0.0, 0.05);
  obj->drawable = NULL;
  obj->m = m;

  if (isnormal(gm)) {
    obj->GM = gm;
  } else {
    obj->GM = m * PL_GRAVITATIONAL_CONST;
  }
  obj->kepler = NULL;
  return obj;
}

//void
//plObjSetSceneObj(PLobject__ *obj, OOdrawable *drawable)
//{
// obj->drawable = drawable;
//}

PLobject__*
plNewObjInWorld(PLworld*world, const char *name, double m, double gm,
                OOlwcoord *coord, quaternion_t q)
{
  PLobject__ *obj = plNewObj(world, name, m, gm, coord, q);

  obj_array_push(&world->objs, obj);
  return obj;
}

PLobject__*
plNewObjInSys(PLorbsys *sys, const char *name, double m, double gm,
              OOlwcoord *coord, quaternion_t q)
{
  PLobject__ *obj = plNewObj(sys->world, name, m, gm, coord, q);
  obj->sys = sys;

  obj_array_push(&sys->objs, obj);
  return obj;
}


PLworld*
plNewWorld(const char *name, OOscene *sc,
           double m, double gm, double radius, double siderealPeriod,
           double obliquity)
{
  PLworld *world = malloc(sizeof(PLworld));
  obj_array_init(&world->orbits);
  obj_array_init(&world->objs);
  world->scene = sc;
  world->name = strdup(name);
  world->world = dWorldCreate();

  OOlwcoord p;
  ooLwcSet(&p, 0.0, 0.0, 0.0);

  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(obliquity));

  world->centralBody = plNewObj(world, name, m, gm, &p, q);

  return world;
}

PLorbsys*
plCreateOrbit(PLworld *world, const char *name,
              double m, double gm,
              double orbitPeriod,
              double obliquity,
              double semiMaj, double semiMin,
              double inc, double ascendingNode, double argOfPeriapsis,
              double meanAnomaly)
{
  assert(world);

  PLorbsys *sys = malloc(sizeof(PLorbsys));
  obj_array_init(&sys->orbits);
  obj_array_init(&sys->objs);

  sys->name = strdup(name);
  sys->world = world;
  sys->parent = NULL;

  sys->orbitalPeriod = orbitPeriod;

  // TODO: Stack allocation based on untrusted length should not be here
  char orbitName[strlen(name) + strlen(" Orbit") + 1];
  strcpy(orbitName, name); // safe as size is checked in allocation
  strcat(orbitName, " Orbit");

  OOlwcoord p;
  ooLwcSet(&p, 0.0, 0.0, 0.0);
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(ascendingNode));
  q = q_mul(q, q_rot(0.0, 1.0, 0.0, DEG_TO_RAD(inc)));
  q = q_mul(q, q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(argOfPeriapsis)));
  q = q_mul(q, q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(obliquity)));

  sys->orbitalBody = plNewObj(world, name, m, gm, &p, q);
  sys->orbitalBody->kepler = plNewKeplerElements(sqrt((semiMaj*semiMaj-semiMin*semiMin)/(semiMaj*semiMaj)),
                                                 semiMaj, inc, ascendingNode,
                                                 argOfPeriapsis, meanAnomaly);

  sys->orbitDrawable = sgNewEllipsis(orbitName, semiMaj, semiMin,
                                     ascendingNode, inc, argOfPeriapsis,
                                     0.0, 0.0, 1.0,
                                     256);
  ooSgSceneAddObj(sys->world->scene,
                  sys->orbitDrawable);

  return sys;
}


PLorbsys*
plNewOrbit(PLworld *world, const char *name,
           double m, double gm,
           double orbitPeriod, double obliquity,
           double semiMaj, double semiMin,
           double inc, double ascendingNode, double argOfPeriapsis,
           double meanAnomaly)
{
  assert(world);
  PLorbsys * sys = plCreateOrbit(world, name, m, gm, orbitPeriod, obliquity,
                                 semiMaj, semiMin,
                                 inc, ascendingNode, argOfPeriapsis,
                                 meanAnomaly);
  obj_array_push(&world->orbits, sys);
  plSysSetCurrentPos(sys);

  return sys;
}
PLorbsys*
plNewSubOrbit(PLorbsys *parent, const char *name,
              double m, double gm,
              double orbitPeriod, double obliquity,
              double semiMaj, double semiMin,
              double inc, double ascendingNode, double argOfPeriapsis,
              double meanAnomaly)
{
  assert(parent);
  assert(parent->world);

  PLorbsys * sys = plCreateOrbit(parent->world,
                                 name, m, gm, orbitPeriod, obliquity,
                                 semiMaj, semiMin,
                                 inc, ascendingNode, argOfPeriapsis, meanAnomaly);
  sys->parent = parent;
  obj_array_push(&parent->orbits, sys);
  plSysSetCurrentPos(sys);

  return sys;
}

void
plSysClear(PLorbsys *sys)
{
  for (size_t i = 0; i < sys->objs.length ; i ++) {
    plClearObject__(sys->objs.elems[i]);
  }

  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    plSysClear(sys->orbits.elems[i]);
  }
}

void
plWorldClear(PLworld *world)
{
  for (size_t i = 0; i < world->objs.length ; i ++) {
    plClearObject__(world->objs.elems[i]);
  }

  for (size_t i = 0; i < world->orbits.length ; i ++) {
    plSysClear(world->orbits.elems[i]);
  }
}

void
plSysStep(PLorbsys *sys, double dt)
{
  plSysSetCurrentPos(sys);

  // Add gravitational forces
  for (size_t i = 0; i < sys->objs.length ; i ++) {
    PLobject__ *obj = sys->objs.elems[i];
    float3 dist = ooLwcDist(&sys->orbitalBody->p, &obj->p);
    double r12 = vf3_abs_square(dist);
    float3 f12 = vf3_s_mul(vf3_normalise(dist),
                          -PL_GRAVITATIONAL_CONST * sys->orbitalBody->m * obj->m / r12);
    dBodyAddForce(obj->id, vf3_get(f12, 0), vf3_get(f12, 1), vf3_get(f12, 2));
  }

  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    plSysStep(sys->orbits.elems[i], dt);
  }
}

void
plSysUpateSg(PLorbsys *sys)
{
  const dReal *quat = dBodyGetQuaternion(sys->orbitalBody->id);
  ooSgSetObjectQuat(sys->orbitalBody->drawable,
                    quat[1], quat[2], quat[3], quat[0]);
  ooSgSetObjectPosLW(sys->orbitalBody->drawable, &sys->orbitalBody->p);

  // Update orbital path base
  if (sys->parent) {
    ooSgSetObjectPosLW(sys->orbitDrawable, &sys->parent->orbitalBody->p);
  } else {
    ooSgSetObjectPosLW(sys->orbitDrawable, &sys->world->centralBody->p);
  }

  //ooSgSetObjectSpeed(OOdrawable *obj, float dx, float dy, float dz);
  //ooSgSetObjectAngularSpeed(OOdrawable *obj, float drx, float dry, float drz);

  for (size_t i = 0; i < sys->orbits.length ; i ++) {
    plSysUpateSg(sys->orbits.elems[i]);
  }

}


void
plWorldStep(PLworld *world, double dt)
{
  dWorldStep(world->world, dt); // TODO: Add callbacks for updating object positions

  for (size_t i = 0; i < world->objs.length ; i ++) {
    PLobject__ *obj = world->objs.elems[i];
    float3 dist = ooLwcDist(&world->centralBody->p, &obj->p);
    double r12 = vf3_abs_square(dist);
    float3 f12 = vf3_s_mul(vf3_normalise(dist),
                          -PL_GRAVITATIONAL_CONST * world->centralBody->m * obj->m / r12);
    dBodyAddForce(obj->id, vf3_get(f12, 0), vf3_get(f12, 1), vf3_get(f12, 2));
  }

  for (size_t i = 0; i < world->orbits.length ; i ++) {
    plSysStep(world->orbits.elems[i], dt);
  }

  // Update SG
  // First move the camera object

  //
  const dReal *quat = dBodyGetQuaternion(world->centralBody->id);
  ooSgSetObjectQuat(world->centralBody->drawable,
                    quat[1], quat[2], quat[3], quat[0]);
  ooSgSetObjectPosLW(world->centralBody->drawable, &world->centralBody->p);
  sgSetLightPosLW(world->centralLightSource, &world->centralBody->p);
  //ooSgSetObjectSpeed(OOdrawable *obj, float dx, float dy, float dz);
  //ooSgSetObjectAngularSpeed(OOdrawable *obj, float drx, float dry, float drz);
  for (size_t i = 0; i < world->orbits.length ; i ++) {
    plSysUpateSg(world->orbits.elems[i]);
  }

}
/*
    NOTE: G is defined as 6.67428 e-11 (m^3)/kg/(s^2), let's call that G_m. In AU,
      this would then be G_au = G_m / (au^3)

      This means that G_au = 1.99316734 e-44 au^3/kg/s^2
      or: G_au = 1.99316734 e-44 au^3/kg/s^2

      1 au = 149 597 870 000 m

*/
// Conforms to dBodySetMovedCallback registered callbacks
void
ooUpdateObject(dBodyID body)
{
  PLobject *obj = dBodyGetData(body);
  ooLogTrace("updating body %s", obj->name);

  //const dReal *rot = dBodyGetRotation(body);
  const dReal *quat = dBodyGetQuaternion(body);
  const dReal *linVel = dBodyGetLinearVel(body);
  const dReal *angVel = dBodyGetAngularVel(body);

  plNormaliseObject(obj);

  ooSgSetObjectPosLW(obj->drawable, &obj->p);
  ooSgSetObjectQuat(obj->drawable, quat[1], quat[2], quat[3], quat[0]);
  ooSgSetObjectSpeed(obj->drawable, linVel[0], linVel[1], linVel[2]);
  ooSgSetObjectAngularSpeed(obj->drawable, angVel[0], angVel[1], angVel[2]);
}

#if 0
v4f_t ooOrbitDist(PLobject * restrict a, PLobject * restrict b)
{
  assert(a != NULL);
  assert(b != NULL);

  const dReal *pa = dBodyGetPosition(a->id);
  const dReal *pb = dBodyGetPosition(b->id);

  if (a->sys == b->sys) {
    v4f_t dist = ode2v3(pa) - ode2v3(pb);
    return dist;
  } else {
    assert(0 && "distances between diffrent systems not yet supported");
    return v4f_make(0.0, 0.0, 0.0, 0.0);
  }
}
#endif
void
ooLoadMoon__(PLorbsys *sys, HRMLobject *obj, OOscenegraph *sg)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue moonName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, gm = NAN, axialTilt = 0.0;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong;
  const char *tex = NULL;
  HRMLobject *sats = NULL;

  for (HRMLobject *child = obj->children; child != NULL ; child = child->next) {
    if (!strcmp(child->name, "physical")) {
      for (HRMLobject *phys = child->children; phys != NULL; phys = phys->next) {
        if (!strcmp(phys->name, "mass")) {
          mass = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "radius")) {
          radius = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "sidereal-rotational-period")) {
          siderealPeriod = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "axial-tilt")) {
          axialTilt = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "gm")) {
          gm = hrmlGetReal(phys);
        }
      }
    } else if (!strcmp(child->name, "orbit")) {
      for (HRMLobject *orbit = child->children; orbit != NULL; orbit = orbit->next) {
        if (!strcmp(orbit->name, "semimajor-axis")) {
          semiMajor = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "eccentricity")) {
          ecc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "inclination")) {
          inc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "longitude-ascending-node")) {
          longAscNode = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "longitude-periapsis")) {
          longPerihel = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "mean-longitude")) {
          meanLong = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "reference-date")) {

        } else {
          fprintf(stderr, "load, invalid orbit token: %s\n", orbit->name);
          assert(0);
        }
      }
    } else if (!strcmp(child->name, "atmosphere")) {

    } else if (!strcmp(child->name, "rendering")) {
      for (HRMLobject *rend = child->children; rend != NULL; rend = rend->next) {
        if (!strcmp(rend->name, "model")) {

        } else if (!strcmp(rend->name, "texture")) {
          tex = hrmlGetStr(rend);
        }
      }
    }
  }

  if (isnan(gm)) {
    gm = mass*PL_GRAVITATIONAL_CONST;
  }
  // Period will be in years assuming that semiMajor is in au
  double period = plOrbitalPeriod(semiMajor, sys->orbitalBody->GM * gm) / PL_SEC_PER_DAY;

  //  double period = 0.1;//comp_orbital_period_for_planet(semiMajor);
  OOscene *sc = ooSgGetRoot(sg);
  OOdrawable *drawable = ooSgNewSphere(moonName.u.str, radius, tex);


  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius

  PLorbsys *moonSys = plNewSubOrbit(sys, moonName.u.str, mass, gm,
                                    period, axialTilt,
                                    semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc),
                                    inc, longAscNode, longPerihel, meanLong);

  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);

  plSetDrawable(moonSys->orbitalBody, drawable);
}

void
ooLoadPlanet__(PLworld *world, HRMLobject *obj, OOscenegraph *sg)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, axialTilt = 0.0, gm = NAN;
  double semiMajor, ecc, inc = NAN, longAscNode = NAN, longPerihel = NAN, meanLong;
  const char *tex = NULL;
  HRMLobject *sats = NULL;

  for (HRMLobject *child = obj->children; child != NULL ; child = child->next) {
    if (!strcmp(child->name, "physical")) {
      for (HRMLobject *phys = child->children; phys != NULL; phys = phys->next) {
        if (!strcmp(phys->name, "mass")) {
          mass = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "radius")) {
          radius = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "sidereal-rotational-period")) {
          siderealPeriod = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "axial-tilt")) {
          axialTilt = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "gm")) {
          gm = hrmlGetReal(phys);
        }
      }
    } else if (!strcmp(child->name, "orbit")) {
      for (HRMLobject *orbit = child->children; orbit != NULL; orbit = orbit->next) {
        if (!strcmp(orbit->name, "semimajor-axis")) {
          semiMajor = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "eccentricity")) {
          ecc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "inclination")) {
          inc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "longitude-ascending-node")) {
          longAscNode = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "longitude-periapsis")) {
          longPerihel = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "mean-longitude")) {
          meanLong = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "reference-date")) {

        } else {
          fprintf(stderr, "load, invalid orbit token: %s\n", orbit->name);
          assert(0);
        }
      }
    } else if (!strcmp(child->name, "atmosphere")) {

    } else if (!strcmp(child->name, "rendering")) {
      for (HRMLobject *rend = child->children; rend != NULL; rend = rend->next) {
        if (!strcmp(rend->name, "model")) {

        } else if (!strcmp(rend->name, "texture")) {
          tex = hrmlGetStr(rend);
        }
      }
    } else if (!strcmp(child->name, "satellites")) {
      sats = child;
    }
  }

  if (isnan(gm)) {
    gm = mass*PL_GRAVITATIONAL_CONST;
  }
  double period = plOrbitalPeriod(plAuToMetres(semiMajor), world->centralBody->GM+gm) / PL_SEC_PER_DAY;
  OOscene *sc = ooSgGetRoot(sg);
  OOdrawable *drawable = ooSgNewSphere(planetName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLorbsys *sys = plNewOrbit(world, planetName.u.str,
                             mass, gm,
                             comp_orbital_period_for_planet(semiMajor), axialTilt,
                             plAuToMetres(semiMajor),
                             plAuToMetres(ooGeoComputeSemiMinor(semiMajor, ecc)),
                             inc, longAscNode, longPerihel, meanLong);
  plSetDrawable(sys->orbitalBody, drawable);
  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);
  if (sats) {
    for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
      if (!strcmp(sat->name, "moon")) {
        ooLoadMoon__(sys, sat, sg);
      }
    }
  }
}


PLworld*
ooLoadStar__(HRMLobject *obj, OOscenegraph *sg)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);
  HRMLvalue starName = hrmlGetAttrForName(obj, "name");
  double mass = 0.0, gm = NAN;
  double radius, siderealPeriod, axialTilt;
  const char *tex = NULL;


  HRMLobject *sats = NULL;
  for (HRMLobject *child = obj->children; child != NULL ; child = child->next) {
    if (!strcmp(child->name, "satellites")) {
      sats = child;
    } else if (!strcmp(child->name, "physical")) {
      for (HRMLobject *phys = child->children; phys != NULL; phys = phys->next) {
        if (!strcmp(phys->name, "mass")) {
          mass = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "radius")) {
          radius = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "sidereal-rotational-period")) {
          siderealPeriod = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "axial-tilt")) {
          axialTilt = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "gm")) {
          gm = hrmlGetReal(phys);
        }
      }
    } else if (!strcmp(child->name, "rendering")) {
      for (HRMLobject *rend = child->children; rend != NULL; rend = rend->next) {
        if (!strcmp(rend->name, "model")) {

        } else if (!strcmp(rend->name, "texture")) {
          tex = hrmlGetStr(rend);
        }
      }
    }
  }

  if (isnan(gm)) {
    gm = mass*PL_GRAVITATIONAL_CONST;
  }

  OOscene *sc = ooSgGetRoot(sg);
  sgSetSceneAmb4f(sc, 0.2, 0.2, 0.2, 1.0);
  OOdrawable *drawable = ooSgNewSphere(starName.u.str, radius, tex);
  SGmaterial *mat = sgSphereGetMaterial((OOsphere*)drawable);
  sgSetMaterialEmiss4f(mat, 1.0, 1.0, 1.0, 0.0);

  //char *path = ooResGetPath("spacecrafts/mercury/enterprise.ac");
  //OOdrawable *drawable = sgLoadModel(path);
  //free(path);


  float3 p = vf3_set(0.0, 0.0, 0.0);
  SGlight *starLightSource = sgNewPointlight(sc, p);


  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLworld *world = plNewWorld(starName.u.str, sc, mass, gm, radius,
                              siderealPeriod, axialTilt);
  world->centralLightSource = starLightSource;

  plSetDrawable(world->centralBody, drawable);
  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);

  assert(sats != NULL);
  for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
    if (!strcmp(sat->name, "planet")) {
      ooLoadPlanet__(world, sat, sg);
    } else if (!strcmp(sat->name, "comet")) {
    }
  }

  return world;
}
PLworld*
ooOrbitLoad(OOscenegraph *sg, const char *fileName)
{
  char *file = ooResGetPath(fileName);
  HRMLdocument *solarSys = hrmlParse(file);
  free(file);
  //HRMLschema *schema = hrmlLoadSchema(ooResGetFile("solarsystem.hrmlschema"));
  //hrmlValidate(solarSys, schema);
  if (solarSys == NULL) {
    // Parser is responsible for pestering the users with errors for now.
    return NULL;
  }

  PLworld *world = NULL;
  // Go through the document and handle each entry in the document

  for (HRMLobject *node = hrmlGetRoot(solarSys); node != NULL; node = node->next) {
    if (!strcmp(node->name, "openorbit")) {
      for (HRMLobject *star = node->children; star != NULL; star = star->next) {
        if (!strcmp(star->name, "star")) {
          world = ooLoadStar__(star, sg);
        }
        //sys = ooOrbitLoadStar(star); //BUG: If more than one star is specified
        //ooSgSetRoot(sg, sys->scene);
      }
    }
  }

  hrmlFreeDocument(solarSys);

  ooLogInfo("loaded solar system");
  return world;
}


