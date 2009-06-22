/*
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2008 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
*/

#include "orbit.h"
#include <ode/ode.h>
#include <assert.h>


#include <vmath/vmath.h>
#include "sim/simtime.h"
#include "geo/geo.h"
#include "log.h"
#include "parsers/hrml.h"
#include "res-manager.h"
#include "rendering/scenegraph.h"

/*
    NOTE: G is defined as 6.67428 e-11 (m^3)/kg/(s^2), let's call that G_m. In AU,
      this would then be G_au = G_m / (au^3)

      This means that G_au = 1.99316734 e-44 au^3/kg/s^2
      or: G_au = 1.99316734 e-44 au^3/kg/s^2

      1 au = 149 597 870 000 m
*/

OOorbsys*
ooOrbitNewSys(const char *name, OOscene *scene,
              float m, float period,
              float semiMaj, float semiMin)
{
  OOorbsys *sys = malloc(sizeof(OOorbsys));
  sys->world = dWorldCreate();
  sys->name = strdup(name);
  sys->orbit = ooGeoEllipseAreaSeg(300, semiMaj, semiMin);
  ooObjVecInit(&sys->sats);
  ooObjVecInit(&sys->objs);

  sys->parent = NULL;

  sys->scale.dist = 1.0f;
  sys->scale.distInv = 1.0f;
  sys->scale.mass = 1.0f;
  sys->scale.massInv = 1.0f;

  sys->phys.k.G = 6.67428e-11;
  sys->phys.param.m = m;
  sys->phys.param.period = period;

  sys->scene = scene;

  return sys;
}

OOorbobj*
ooOrbitNewObj(OOorbsys *sys, const char *name, float m,
              float x, float y, float z,
              float vx, float vy, float vz,
              float qx, float qy, float qz, float qw,
              float rqx, float rqy, float rqz, float rqw)
{
  assert(sys != NULL);
  assert(m >= 0.0);
  OOorbobj *obj = malloc(sizeof(OOorbobj));
  obj->name = strdup(name);
  obj->id = dBodyCreate(sys->world);
  obj->m = m;

  ooObjVecPush(&sys->objs, obj);

  return obj;
}


void
ooOrbitAddChildSys(OOorbsys * restrict parent, OOorbsys * restrict child)
{
  assert(parent != NULL);
  assert(child != NULL);

  ooLogInfo("adding child system %s to %s", child->name, parent->name);
  ooObjVecPush(&parent->sats, child);
}

void
ooOrbitSetScale(OOorbsys *sys, float ms, float ds)
{
  assert(sys != NULL);

  sys->scale.mass = ms;
  sys->scale.massInv = 1.0f/ms;
  sys->scale.dist = ds;
  sys->scale.distInv = 1.0f/ds;
}

void
ooOrbitSetScene(OOorbsys *sys, OOscene *scene)
{
  assert(sys != NULL);
  assert(scene != NULL);

  sys->scene = scene;
}


void
ooOrbitClear(OOorbsys *sys)
{
  for (size_t i ; i < sys->objs.length ; i ++) {
    dBodySetForce(((OOorbobj*)sys->objs.elems[i])->id, 0.0f, 0.0f, 0.0f);
    dBodySetTorque(((OOorbobj*)sys->objs.elems[i])->id, 0.0f, 0.0f, 0.0f);
  }

  for (size_t i; i < sys->sats.length ; i ++) {
    ooOrbitClear(sys->sats.elems[i]);
  }
}



void
ooOrbitStep(OOorbsys *sys, float stepSize)
{
  bool needsCompacting = false;
  // First compute local gravity for each object
  for (size_t i ; i < sys->objs.length ; i ++) {
    // Since objects can migrate to other systems...
    if (sys->objs.elems[i] != NULL) {
      OOorbobj *obj = sys->objs.elems[i];
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
  sys->phys.param.pos = ooGeoEllipseSegPoint(sys->orbit,
                                   (ooTimeGetJD()/sys->phys.param.period)*
                                   (float)sys->orbit->vec.length);

  ooLogTrace("%f: %s: %f: %vf", ooTimeGetJD(), sys->name, sys->phys.param.period, sys->phys.param.pos);

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
ooOrbitSetConstant(OOorbsys *sys, const char *key, float k)
{
    assert(sys != NULL && "sys is null");
    assert(key != NULL && "key is null");

    if (!strcmp(key, "G")) {
        sys->phys.k.G = k;
    }
}


void
ooOrbitLoadComet(HRMLobject *obj)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLobject *child = obj->children;
  while (child) {
    child = child->next;
  }

}

void
ooOrbitLoadMoon(HRMLobject *obj)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLobject *child = obj->children;
  while (child) {
    child = child->next;
  }
}


OOorbsys*
ooOrbitLoadPlanet(HRMLobject *obj, OOscene *parentScene)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

  HRMLobject *child = obj->children;

  double mass, radius;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong, rightAsc,
         declin;

  while (child) {
    if (!strcmp(child->name, "physical")) {
      HRMLobject *phys = child->children;
      while (phys) {
        if (!strcmp(phys->name, "mass")) {
          mass = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "radius")) {
          radius = hrmlGetReal(phys);
        }

        phys = phys->next;
      }

    } else if (!strcmp(child->name, "orbit")) {
      HRMLobject *orbit = child->children;
      while (orbit) {
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
        orbit = orbit->next;
      }
    } else if (!strcmp(child->name, "atmosphere")) {

    } else if (!strcmp(child->name, "rendering")) {
      HRMLobject *rend = child->children;
      while (rend) {
        if (!strcmp(rend->name, "model")) {

        } else if (!strcmp(rend->name, "texture")) {

        }

        rend = rend->next;
      }

    }

    child = child->next;
  }
  
  OOscene *sc = ooSgNewScene(parentScene, ""/*(planetName)*/);
  
  return ooOrbitNewSys(planetName.u.str, sc,
                       mass, 0.0,//float period,
                       semiMajor, ecc*semiMajor);

}

void
ooOrbitLoadSatellites(HRMLobject *obj, OOscene *parentScene)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLobject *child = obj->children;
  while (child) {
    if (!strcmp(child->name, "planet")) {
      ooOrbitLoadPlanet(child, parentScene);
    } else if (!strcmp(child->name, "moon")) {
      ooOrbitLoadMoon(child);
    } else if (!strcmp(child->name, "comet")) {

    }

    child = child->next;
  }

}


OOorbsys*
ooOrbitLoadStar(HRMLobject *obj)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);
  HRMLvalue starName = hrmlGetAttrForName(obj, "name");
  double mass;

  HRMLobject *child = obj->children;

  OOscene *sc = ooSgNewScene(NULL, ""/*(starName)*/);


  while (child) {
    if (!strcmp(child->name, "satellites")) {
      ooOrbitLoadSatellites(child, sc);
    } else if (!strcmp(child->name, "physical")) {
      HRMLobject *phys = child->children;
      while (phys) {
        if (!strcmp(phys->name, "mass")) {
          mass = hrmlGetReal(phys);
        } else if (!strcmp(phys->name, "radius")) {

        }

        phys = phys->next;
      }
    } else if (!strcmp(child->name, "rendering")) {
    }
    
    child = child->next;
  }
  

  return ooOrbitNewSys(starName.u.str, sc,
                       mass, 0.0,//float period,
                       0.0, 0.0);
}

OOorbsys*
ooOrbitLoad(const char *fileName)
{
  FILE *file = ooResGetFile(fileName);
  HRMLdocument *solarSys = hrmlParse(file);
  //HRMLschema *schema = hrmlLoadSchema(ooResGetFile("solarsystem.hrmlschema"));
  //hrmlValidate(solarSys, schema);
  if (solarSys == NULL) {
    // Parser is responsible for pestering the users with errors for now.
    return NULL;
  }

  OOorbsys *sys = NULL;
  // Go through the document and handle each entry in the document
  HRMLobject *node = hrmlGetRoot(solarSys);
  while (node) {
    if (!strcmp(node->name, "openorbit")) {
      HRMLobject *star = node->children;
      while (star) {
        sys = ooOrbitLoadStar(star); //BUG: If more than one star is specified
        star = star->next;
      }
    }
    node = node->next;
  }

  hrmlFreeDocument(solarSys);

  return sys;
}
