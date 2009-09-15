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

PLorbsys*
ooOrbitNewRootSys(const char *name, OOscene *scene, double m, double rotPeriod)
{
  PLorbsys *sys = malloc(sizeof(PLorbsys));
  sys->world = dWorldCreate();
  sys->name = strdup(name);
  ooObjVecInit(&sys->sats);
  ooObjVecInit(&sys->objs);

  sys->parent = NULL;

  sys->scale.dist = 1.0f;
  sys->scale.distInv = 1.0f;
  sys->scale.mass = 1.0f;
  sys->scale.massInv = 1.0f;

  sys->phys.k.G = 6.67428e-11;
  sys->phys.param.m = m;
  sys->phys.param.orbitalPeriod = 0.0;
  sys->phys.param.rotationPeriod = rotPeriod;

  sys->scene = scene;
  sys->level = 0;
  sys->orbit = NULL;

  return sys;
}


PLorbsys*
ooOrbitNewSys(const char *name, OOscene *scene,
              double m, double orbitPeriod, double rotPeriod,
              double semiMaj, double semiMin)
{
  PLorbsys *sys = ooOrbitNewRootSys(name, scene, m, rotPeriod);
  sys->orbit = ooGeoEllipseAreaSeg(500, semiMaj, semiMin);

  ooSgSceneAddObj(ooSgSceneGetParent(scene),
                  ooSgNewDrawable(name, sys->orbit,
                  (OOdrawfunc)ooGeoEllipseDraw));

  sys->phys.param.orbitalPeriod = orbitPeriod;
  return sys;
}

PLobject*
ooOrbitNewObj(PLorbsys *sys, const char *name,
              OOdrawable *drawable,
              double m,
              double x, double y, double z,
              double vx, double vy, double vz,
              double qx, double qy, double qz, double qw,
              double rqx, double rqy, double rqz, double rqw)
{
  assert(sys != NULL);
  assert(m >= 0.0);
  PLobject *obj = malloc(sizeof(PLobject));
  obj->name = strdup(name);
  obj->m = m;
  obj->id = dBodyCreate(sys->world);
  dBodySetGravityMode(obj->id, 0); // Ignore standard ode gravity effects

  obj->drawable = drawable;
  dBodySetData(obj->id, obj); // 
  dBodySetMovedCallback(obj->id, ooUpdateObject);

  dQuaternion quat = {qw, qx, qy, qz};
  dBodySetQuaternion(obj->id, quat);
  dBodySetAngularVel(obj->id, 0.0, 0.0, 0.05);

  obj->sys = sys;

  obj->p.offs = vf3_set(x, y, z);
  obj->p.seg = vi3_set(0, 0, 0);
  ooLwcNormalise(&obj->p);

  ooObjVecPush(&sys->objs, obj);

  return obj;
}

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


void
ooOrbitAddChildSys(PLorbsys * restrict parent, PLorbsys * restrict child)
{
  assert(parent != NULL);
  assert(child != NULL);

  ooLogInfo("adding child system %s to %s", child->name, parent->name);
  ooObjVecPush(&parent->sats, child);
  
  child->parent = parent;
  child->level = parent->level + 1;
}

void
ooOrbitSetScale(PLorbsys *sys, double ms, double ds)
{
  assert(sys != NULL);

  sys->scale.mass = ms;
  sys->scale.massInv = 1.0f/ms;
  sys->scale.dist = ds;
  sys->scale.distInv = 1.0f/ds;
}

void
ooOrbitSetScene(PLorbsys *sys, OOscene *scene)
{
  assert(sys != NULL);
  assert(scene != NULL);

  sys->scene = scene;
}


void
ooOrbitClear(PLorbsys *sys)
{
  for (size_t i ; i < sys->objs.length ; i ++) {
    dBodySetForce(((PLobject*)sys->objs.elems[i])->id, 0.0f, 0.0f, 0.0f);
    dBodySetTorque(((PLobject*)sys->objs.elems[i])->id, 0.0f, 0.0f, 0.0f);
  }

  for (size_t i; i < sys->sats.length ; i ++) {
    ooOrbitClear(sys->sats.elems[i]);
  }
}



void
ooOrbitStep(PLorbsys *sys, double stepSize)
{
  bool needsCompacting = false;
  // First compute local gravity for each object
  for (size_t i ; i < sys->objs.length ; i ++) {
    // Since objects can migrate to other systems...
    if (sys->objs.elems[i] != NULL) {
      PLobject *obj = sys->objs.elems[i];
      //sys->phys.param.m
      const dReal *objPos_ = dBodyGetPosition(obj->id);
      vector_t objPos = v_set(objPos_[0], objPos_[1], objPos_[2], 0.0f);
      vector_t dist = objPos; // Since system origin is 0.0
      scalar_t r12 = v_abs(dist);
      r12 = r12 * r12;
      vector_t f12 = v_s_mul(v_normalise(v_neg(dist)), //negate, force should point to center object
                            -sys->phys.k.G * sys->phys.param.m * obj->m / r12);
      dBodyAddForce(obj->id, f12.x, f12.y, f12.z);
    } else {
      needsCompacting = true;
    }
  }

  // Do ODE step
  dWorldStep(sys->world, stepSize);

  // Update current position
  if (sys->orbit) {
    sys->phys.param.pos = ooGeoEllipseSegPoint(sys->orbit,
                                               (ooTimeGetJD(ooSimTimeState()) / 
                                                 sys->phys.param.orbitalPeriod)*
                                                 (double)sys->orbit->vec.length);

    ooLogTrace("%f: %s: %f: %vf",
               ooTimeGetJD(ooSimTimeState()),
               sys->name,
               sys->phys.param.orbitalPeriod,
               sys->phys.param.pos);
  } else {
    sys->phys.param.pos = v4f_make(0.0, 0.0, 0.0, 0.0);
  }
  // Recurse and do the same for each subsystem
  for (size_t i = 0; i < sys->sats.length ; i ++) {
    ooOrbitStep(sys->sats.elems[i], stepSize);
  }

  if (sys->scene) {
    vector_t v = {.v = sys->phys.param.pos};
    ooSgSetScenePos(sys->scene, v.x, v.y, v.z);
    //ooSgSetSceneQuat(sys->scene, float x, float y, float z, float w);
    //ooSgSetSceneScale(sys->scene, float scale);
  }

  if (needsCompacting) {
    ooObjVecCompress(&sys->objs);
  }
}

void
ooOrbitSetConstant(PLorbsys *sys, const char *key, double k)
{
    assert(sys != NULL && "sys is null");
    assert(key != NULL && "key is null");

    if (!strcmp(key, "G")) {
        sys->phys.k.G = k;
    }
}

void ooOrbitLoadSatellites(HRMLobject *obj, PLorbsys *sys, OOscene *parentScene);

void
ooOrbitLoadComet(HRMLobject *obj)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  for (HRMLobject *child = obj->children; child != NULL; child = child->next) {

  }
}

PLorbsys*
ooOrbitLoadMoon(HRMLobject *obj, OOscene *parentScene)
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
    } else if (!strcmp(child->name, "sattelites")) {
      sats = child;
    }
  }

  OOscene *sc = ooSgNewScene(parentScene, moonName.u.str/*(planetName)*/);
  // Create scene object for planet


  //ooSgSetObjectAngularSpeed(drawable, )
  // Period will be in years assuming that semiMajor is in au
  double period = 0.1;//comp_orbital_period_for_planet(semiMajor);

  PLorbsys *sys = ooOrbitNewSys(moonName.u.str, sc,
                                mass, period, 1.0,//float period,
                                semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc));

  OOdrawable *drawable = ooSgNewSphere(moonName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(90.0));
  quaternion_t qr = q_rot(0.0/*x*/,1.0/*y*/,0.0/*z*/,DEG_TO_RAD(0.05)); // TODO: real rot

  PLobject *orbObj = ooOrbitNewObj(sys, moonName.u.str, drawable,
                                   mass,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0,
                                   q.x, q.y, q.z, q.w,
                                   qr.x, qr.y, qr.z, qr.w);

  // For now we just set the scale like the parent is in au and this scene in m
  // this should be taken from the hrml file though...
  sys->scale.dist = 1.0;
  sys->scale.distInv = 1.0;
  ooSgSetSceneScale(sc, 1.0);

  return sys;
}


PLorbsys*
ooOrbitLoadPlanet(HRMLobject *obj, OOscene *parentScene)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

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
    } else if (!strcmp(child->name, "satellites")) {
      sats = child;
    }
  }

  OOscene *sc = ooSgNewScene(parentScene, planetName.u.str/*(planetName)*/);
  // Create scene object for planet


  //ooSgSetObjectAngularSpeed(drawable, )
  // Period will be in years assuming that semiMajor is in au
  double period = comp_orbital_period_for_planet(semiMajor);

  PLorbsys *sys = ooOrbitNewSys(planetName.u.str, sc,
                                mass, period, 1.0,//float period,
                                semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc));

  OOdrawable *drawable = ooSgNewSphere(planetName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(90.0));
  quaternion_t qr = q_rot(0.0/*x*/,1.0/*y*/,0.0/*z*/,DEG_TO_RAD(0.05)); // TODO: real rot

  PLobject *orbObj = ooOrbitNewObj(sys, planetName.u.str, drawable,
                                   mass,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0,
                                   q.x, q.y, q.z, q.w,
                                   qr.x, qr.y, qr.z, qr.w);

  // For now we just set the scale like the parent is in au and this scene in m
  // this should be taken from the hrml file though...
  sys->scale.dist = 149598000000.0;
  sys->scale.distInv = 1.0/149598000000.0;
  ooSgSetSceneScale(sc, 149598000000.0); // AU in m (always relative to parent)

  if (sats) {
    ooOrbitLoadSatellites(sats, sys, sc);
  }

  return sys;
}

void
ooOrbitLoadSatellites(HRMLobject *obj, PLorbsys *sys, OOscene *parentScene)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  for (HRMLobject *child = obj->children; child != NULL; child = child->next) {
    if (!strcmp(child->name, "planet")) {
      PLorbsys *psys = ooOrbitLoadPlanet(child, parentScene);
      ooOrbitAddChildSys(sys, psys);
    } else if (!strcmp(child->name, "moon")) {
      PLorbsys *msys = ooOrbitLoadMoon(child, parentScene);
      ooOrbitAddChildSys(sys, msys);      
    } else if (!strcmp(child->name, "comet")) {

    }
  }
}


PLorbsys*
ooOrbitLoadStar(HRMLobject *obj)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);
  HRMLvalue starName = hrmlGetAttrForName(obj, "name");
  double mass = 0.0;
  double radius, siderealPeriod;
  const char *tex = NULL;
  OOscene *sc = ooSgNewScene(NULL, starName.u.str);

  PLorbsys *sys = ooOrbitNewRootSys(starName.u.str, sc,
                                    mass, 0.0 //float period
                                    );
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


  sys->phys.param.m = mass;
  sys->scale.dist = 1.0;
  sys->scale.distInv = 1.0;

  assert(sats != NULL);
  ooOrbitLoadSatellites(sats, sys, sc);

  OOdrawable *drawable = ooSgNewSphere(starName.u.str, radius, tex);
  ooSgSceneAddObj(sc, drawable); // TODO: scale to radius
  ooSgSetObjectScale(drawable, 1.0/149598000000.0); // Adjust to m in au context
  quaternion_t q = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(90.0));
  quaternion_t qr = q_rot(0.0/*x*/,1.0/*y*/,0.0/*z*/,DEG_TO_RAD(1.0)); // TODO: real rot

  PLobject *orbObj = ooOrbitNewObj(sys, starName.u.str, drawable,
                                   mass,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0,
                                   q.x, q.y, q.z, q.w,
                                   qr.x, qr.y, qr.z, qr.w);

  ooSgSetSceneScale(sc, 1.0); // AU in m (always relative to parent)

  return sys;
}

PLorbsys*
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

  PLorbsys *sys = NULL;
  // Go through the document and handle each entry in the document

  for (HRMLobject *node = hrmlGetRoot(solarSys); node != NULL; node = node->next) {
    if (!strcmp(node->name, "openorbit")) {
      for (HRMLobject *star = node->children; star != NULL; star = star->next) {
        sys = ooOrbitLoadStar(star); //BUG: If more than one star is specified
        ooSgSetRoot(sg, sys->scene);
      }
    }
  }

  hrmlFreeDocument(solarSys);

  ooLogInfo("loaded solar system");
  return sys;
}

PLorbsys*
ooOrbitGetSys(const PLorbsys *root,  const char *name)
{
  char str[strlen(name)+1];
  strcpy(str, name); // TODO: We do not trust the user, should probably
                     // check alloca result

  PLorbsys *sys = (PLorbsys*)root;
  char *strp = str;
  char *strTok = strsep(&strp, "/");
  int idx = 0;
  OOobjvector *vec = NULL;
  while (sys) {
    if (!strcmp(sys->name, strTok)) {
      if (strp == NULL) {
        // At the end of the sys path
        return sys;
      }

      // If this is not the lowest level, go one level down
      strTok = strsep(&strp, "/");

      vec = &sys->sats;
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
