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
#include "common/lwcoord.h"

#define PL_GRAVITATIONAL_CONST 6.67428e-11

/*
 NOTE: Coordinate system are specified in the normal convention used for mission
       analysis. This means that x is positive towards you, y is positvie to the
       right and z positive going upwards. This is a right handed coordinate
       system. Positive y, in our case points towards the reference point of
       ares on the ecliptic.
 */

struct keplerian_elements {
  double a, b, c, d, e, f;
};

struct state_vectors {
  double vx, vy, vz, rx, ry, rz;
};


void
euler_to_pos(void)
{
//  double x, y, z;
//  x = sqrt(u/p) * ();
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
  OOobjvector *vec = &world->orbits;

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
  float3 newPos = ooGeoEllipseSegPoint(sys->orbitalPath,
                                      (ooTimeGetJD(ooSimTimeState()) /
                                       sys->orbitalPeriod)*
                                      (double)sys->orbitalPath->vec.length);

  if (sys->parent) {
    sys->orbitalBody->p = sys->parent->orbitalBody->p;
    ooLwcTranslate(&sys->orbitalBody->p, newPos);
  } else {
    sys->orbitalBody->p = sys->world->centralBody->p;
    ooLwcTranslate(&sys->orbitalBody->p, newPos);
  }
}

void
plDeleteSys(PLorbsys *sys)
{
  for (size_t i = 0 ; i < sys->orbits.length ; i ++) {
    plDeleteSys(sys->orbits.elems[i]);
  }

  ooGeoEllipseFree(sys->orbitalPath);
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


PLobject__*
plNewObj(PLworld*world, const char *name, double m, OOlwcoord *coord)
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
  //dQuaternion quat = {qw, qx, qy, qz};
  //dBodySetQuaternion(obj->id, quat);
  //dBodySetAngularVel(obj->id, 0.0, 0.0, 0.05);
  obj->drawable = NULL;
  return obj;
}

//void
//plObjSetSceneObj(PLobject__ *obj, OOdrawable *drawable)
//{
// obj->drawable = drawable;
//}

PLobject__*
plNewObjInWorld(PLworld*world, const char *name, double m, OOlwcoord *coord)
{
  PLobject__ *obj = plNewObj(world, name, m, coord);

  ooObjVecPush(&world->objs, obj);
  return obj;
}

PLobject__*
plNewObjInSys(PLorbsys *sys, const char *name, double m, OOlwcoord *coord)
{
  PLobject__ *obj = plNewObj(sys->world, name, m, coord);
  obj->sys = sys;

  ooObjVecPush(&sys->objs, obj);
  return obj;
}


PLworld*
plNewWorld(const char *name, OOscene *sc,
           double m, double radius, double siderealPeriod)
{
  PLworld *world = malloc(sizeof(PLworld));
  ooObjVecInit(&world->orbits);
  ooObjVecInit(&world->objs);
  world->scene = sc;
  world->name = strdup(name);
  world->world = dWorldCreate();

  OOlwcoord p;
  ooLwcSet(&p, 0.0, 0.0, 0.0);
  world->centralBody = plNewObj(world, name, m, &p);

  return world;
}

PLorbsys*
plCreateOrbit(PLworld *world, const char *name,
              double m,
              double orbitPeriod,
              double semiMaj, double semiMin,
              double inc, double ascendingNode, double argOfPeriapsis)
{
  assert(world);

  PLorbsys *sys = malloc(sizeof(PLorbsys));
  ooObjVecInit(&sys->orbits);
  ooObjVecInit(&sys->objs);

  sys->name = strdup(name);
  sys->world = world;
  sys->parent = NULL;

  sys->orbitalPeriod = orbitPeriod;
  sys->orbitalPath = ooGeoEllipseAreaSeg(500, semiMaj, semiMin);

  // TODO: Stack allocation based on untrusted length should not be here
  char orbitName[strlen(name) + strlen(" Orbit") + 1];
  strcpy(orbitName, name); // safe as size is checked in allocation
  strcat(orbitName, " Orbit");

  sys->orbitDrawable = sgNewEllipsis(orbitName,
                                     semiMaj, semiMin,
                                     0.0, 0.0, 1.0, 256);

  // compute orbit quaternion based on inc, asc, and periapsis,
  // TODO: This is not correct at the moment
  quaternion_t qasc = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(ascendingNode));
  float3 vinc = vf3_set(0.0, 1.0, 0.0);
  //matrix_t tmp;
  //q_m_convert(&tmp, qasc);
  //vinc = m_v3_mulf(&tmp, vinc);
  //quaternion_t qinc = q_rotv(vinc, DEG_TO_RAD(inc));
  //quaternion_t q = q_mul(qasc, qinc);
 // q_m_convert(&tmp, q);

 // float3 vperi = vf3_set(1.0, 0.0, 0.0);
  //vperi = m_v3_mulf(&tmp, vperi);
  //quaternion_t qperi = q_rotv(vperi, DEG_TO_RAD(argOfPeriapsis));
  //q = q_mul(q, qperi);
  quaternion_t q = q_rotv(vinc, DEG_TO_RAD(inc));
  //q = q_mul(qasc, q);
  sgSetObjectQuatv(sys->orbitDrawable, q);

  ooSgSceneAddObj(sys->world->scene,
                  sys->orbitDrawable);

  OOlwcoord p;
  ooLwcSet(&p, 0.0, 0.0, 0.0);
  sys->orbitalBody = plNewObj(world, name, m, &p);

  return sys;
}


PLorbsys*
plNewOrbit(PLworld *world, const char *name,
           double m,
           double orbitPeriod, double semiMaj, double semiMin,
           double inc, double ascendingNode, double argOfPeriapsis)
{
  assert(world);
  PLorbsys * sys = plCreateOrbit(world, name, m, orbitPeriod, semiMaj, semiMin,
                                 inc, ascendingNode, argOfPeriapsis);
  ooObjVecPush(&world->orbits, sys);
  plSysSetCurrentPos(sys);

  return sys;
}
PLorbsys*
plNewSubOrbit(PLorbsys *parent, const char *name,
              double m,
              double orbitPeriod, double semiMaj, double semiMin,
              double inc, double ascendingNode, double argOfPeriapsis)
{
  assert(parent);
  assert(parent->world);

  PLorbsys * sys = plCreateOrbit(parent->world,
                                 name, m, orbitPeriod, semiMaj, semiMin,
                                 inc, ascendingNode, argOfPeriapsis);
  sys->parent = parent;
  ooObjVecPush(&parent->orbits, sys);
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

  double mass, radius, siderealPeriod;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong, rightAsc,
         declin;
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
        } else if (!strcmp(orbit->name, "longitude-perihelion")) {
          longPerihel = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "mean-longitude")) {
          meanLong = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "right-ascension")) {
          rightAsc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "declination")) {
          declin = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "reference-date")) {

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


  // Period will be in years assuming that semiMajor is in au
  double period = 0.1;//comp_orbital_period_for_planet(semiMajor);
  OOscene *sc = ooSgGetRoot(sg);
  OOdrawable *drawable = ooSgNewSphere(moonName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
 
  PLorbsys *moonSys = plNewSubOrbit(sys, moonName.u.str, mass,
                                    period, // orbital period
                                    semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc),
                                    inc, longAscNode, longPerihel);
  
  plSetDrawable(moonSys->orbitalBody, drawable);
}

void
ooLoadPlanet__(PLworld *world, HRMLobject *obj, OOscenegraph *sg)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, axialTilt = 0.0;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong, rightAsc,
         declin;
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
        } else if (!strcmp(orbit->name, "longitude-perihelion")) {
          longPerihel = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "mean-longitude")) {
          meanLong = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "right-ascension")) {
          rightAsc = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "declination")) {
          declin = hrmlGetReal(orbit);
        } else if (!strcmp(orbit->name, "reference-date")) {

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

  OOscene *sc = ooSgGetRoot(sg);
  OOdrawable *drawable = ooSgNewSphere(planetName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLorbsys *sys = plNewOrbit(world, planetName.u.str,
                             mass,
                             comp_orbital_period_for_planet(semiMajor),
                             plAuToMetres(semiMajor),
                             plAuToMetres(ooGeoComputeSemiMinor(semiMajor, ecc)),
                             inc, longAscNode, longPerihel);  
  plSetDrawable(sys->orbitalBody, drawable);
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(axialTilt));
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
  double mass = 0.0;
  double radius, siderealPeriod;
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

  OOscene *sc = ooSgGetRoot(sg);
  OOdrawable *drawable = ooSgNewSphere(starName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLworld *world = plNewWorld(starName.u.str, sc, mass, radius, siderealPeriod);
  plSetDrawable(world->centralBody, drawable);

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


