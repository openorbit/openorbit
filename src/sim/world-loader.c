/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include "world-loader.h"
#include "scenegraph.h"
#include "physics/physics.h"
#include "parsers/hrml.h"
#include "res-manager.h"
/*
 NOTE: G is defined as 6.67428 e-11 (m^3)/kg/(s^2), let's call that G_m. In AU,
 this would then be G_au = G_m / (au^3)

 This means that G_au = 1.99316734 e-44 au^3/kg/s^2
 or: G_au = 1.99316734 e-44 au^3/kg/s^2

 1 au = 149 597 870 000 m

 */

void
ooLoadMoon__(PLsystem *sys, HRMLobject *obj, SGscene *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue moonName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, gm = NAN, axialTilt = 0.0;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong;
  const char *tex = NULL;

  double flattening = 0.0;
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
        } else if (!strcmp(phys->name, "flattening")) {
          flattening = hrmlGetReal(phys);
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
    gm = mass*PL_G;
  }
  // Period will be in years assuming that semiMajor is in au
  double period = plOrbitalPeriod(semiMajor, sys->orbitalBody->GM * gm) / PL_SEC_PER_DAY;

  //  double period = 0.1;//comp_orbital_period_for_planet(semiMajor);
  SGdrawable *drawable = sgNewSphere(moonName.u.str, radius, tex);


  sgSceneAddObj(sc, drawable); // TODO: scale to radius

  PLsystem *moonSys = plNewSubOrbit(sys, sys->scene, moonName.u.str, mass, gm,
                                    period, axialTilt, siderealPeriod,
                                    semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc),
                                    inc, longAscNode, longPerihel, meanLong, radius, flattening);

  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);

  plSetDrawable(moonSys->orbitalBody, drawable);
}

void
ooLoadPlanet__(PLworld *world, HRMLobject *obj, SGscene *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, axialTilt = 0.0, gm = NAN;
  double semiMajor, ecc, inc = NAN, longAscNode = NAN, longPerihel = NAN, meanLong;
  const char *tex = NULL;
  HRMLobject *sats = NULL;
  double flattening = 0.0;

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
        } else if (!strcmp(phys->name, "flattening")) {
          flattening = hrmlGetReal(phys);
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
    gm = mass*PL_G;
  }

  // NOTE: At present, all planets must be specified with AUs as parameters
  double period = plOrbitalPeriod(plAuToMetres(semiMajor), world->rootSys->orbitalBody->GM+gm) / PL_SEC_PER_DAY;

  SGdrawable *drawable = sgNewSphere(planetName.u.str, radius, tex);
  sgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLsystem *sys = plNewOrbit(world, sc, planetName.u.str,
                             mass, gm,
                             period, axialTilt, siderealPeriod,
                             plAuToMetres(semiMajor),
                             plAuToMetres(ooGeoComputeSemiMinor(semiMajor, ecc)),
                             inc, longAscNode, longPerihel, meanLong, radius, flattening);
  plSetDrawable(sys->orbitalBody, drawable);
  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);
  if (sats) {
    for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
      if (!strcmp(sat->name, "moon")) {
        ooLoadMoon__(sys, sat, sc);
      }
    }
  }
}


PLworld*
ooLoadStar__(HRMLobject *obj, SGscene *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);
  HRMLvalue starName = hrmlGetAttrForName(obj, "name");
  double mass = 0.0, gm = NAN;
  double radius, siderealPeriod, axialTilt;
  const char *tex = NULL;
  double flattening = 0.0;

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
        } else if (!strcmp(phys->name, "flattening")) {
          flattening = hrmlGetReal(phys);
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
    gm = mass*PL_G;
  }

  sgSetSceneAmb4f(sc, 0.2, 0.2, 0.2, 1.0);
  SGdrawable *drawable = sgNewSphere(starName.u.str, radius, tex);
  SGmaterial *mat = sgSphereGetMaterial((SGsphere*)drawable);
  sgSetMaterialEmiss4f(mat, 1.0, 1.0, 1.0, 0.0);

  SGlight *starLightSource = sgNewPointlight3f(sc, 0.0f, 0.0f, 0.0f);


  sgSceneAddObj(sc, drawable); // TODO: scale to radius
  PLworld *world = plNewWorld(starName.u.str, sc, mass, gm, radius,
                              siderealPeriod, axialTilt, radius, flattening);
  world->rootSys->orbitalBody->lightSource = starLightSource;

  plSetDrawable(world->rootSys->orbitalBody, drawable);
  quaternion_t q = q_rot(1.0, 0.0, 0.0, DEG_TO_RAD(axialTilt));
  sgSetObjectQuatv(drawable, q);

  assert(sats != NULL);
  for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
    if (!strcmp(sat->name, "planet")) {
      ooLoadPlanet__(world, sat, sc);
    } else if (!strcmp(sat->name, "comet")) {
    }
  }

  return world;
}
PLworld*
ooOrbitLoad(SGscenegraph *sg, const char *fileName)
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
  SGscene *sc = sgNewScene(sg, "main");
  for (HRMLobject *node = hrmlGetRoot(solarSys); node != NULL; node = node->next) {
    if (!strcmp(node->name, "openorbit")) {
      for (HRMLobject *star = node->children; star != NULL; star = star->next) {
        if (!strcmp(star->name, "star")) {
          world = ooLoadStar__(star, sc);
        }
      }
    }
  }

  hrmlFreeDocument(solarSys);

  plSysInit(world->rootSys);
  ooLogInfo("loaded solar system");
  return world;
}
