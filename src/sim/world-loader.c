/*
 Copyright 2010,2013 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include "world-loader.h"
#include "scenegraph.h"
#include "physics/physics.h"
#include "physics/reftypes.h"
#include "physics/areodynamics.h"

#include "parsers/hrml.h"
#include "res-manager.h"
#include <openorbit/log.h>

#include "rendering/texture.h"
#include "rendering/types.h"
#include "rendering/material.h"
#include "rendering/shader-manager.h"

#include <jansson.h>

/*
 NOTE: G is defined as 6.67428 e-11 (m^3)/kg/(s^2), let's call that G_m. In AU,
 this would then be G_au = G_m / (au^3)

 This means that G_au = 1.99316734 e-44 au^3/kg/s^2
 or: G_au = 1.99316734 e-44 au^3/kg/s^2

 1 au = 149 597 870 000 m

 */



void
loadMaterial(sg_material_t *mat, HRMLobject *obj)
{
  sg_material_init(mat);

  for (HRMLobject *matEntry = obj->children; matEntry != NULL;
       matEntry = matEntry->next) {
    if (!strcmp(matEntry->name, "emission")) {
      assert(hrmlGetRealArrayLen(matEntry) == 4);
      const double *vec = hrmlGetRealArray(matEntry);
      sg_material_set_emiss4f(mat, vec[0], vec[1], vec[2], vec[3]);
    } else if (!strcmp(matEntry->name, "ambient")) {
      assert(hrmlGetRealArrayLen(matEntry) == 4);
      const double *vec = hrmlGetRealArray(matEntry);
      sg_material_set_amb4f(mat, vec[0], vec[1], vec[2], vec[3]);
    } else if (!strcmp(matEntry->name, "diffuse")) {
      assert(hrmlGetRealArrayLen(matEntry) == 4);
      const double *vec = hrmlGetRealArray(matEntry);
      sg_material_set_diff4f(mat, vec[0], vec[1], vec[2], vec[3]);
    } else if (!strcmp(matEntry->name, "specular")) {
      assert(hrmlGetRealArrayLen(matEntry) == 4);
      const double *vec = hrmlGetRealArray(matEntry);
      sg_material_set_spec4f(mat, vec[0], vec[1], vec[2], vec[3]);
    } else if (!strcmp(matEntry->name, "shininess")) {
      float shininess = hrmlGetReal(matEntry);
      sg_material_set_shininess(mat, shininess);
    }
  }
}

void
ooLoadMoon__(pl_world_t *world, HRMLobject *obj, sg_scene_t *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue moonName = hrmlGetAttrForName(obj, "name");

  double mass, radius, siderealPeriod, gm = NAN, axialTilt = 0.0;
  double semiMajor, ecc, inc, longAscNode, longPerihel, meanLong;

  const char *tex = NULL;
  const char *shader = NULL;
  const char *spec = NULL;
  const char *bump = NULL;
  const char *nightTex = NULL;
  sg_material_t *mat = sg_new_material();
  sg_material_init(mat);

  double flattening = 0.0;
  //HRMLobject *sats = NULL;

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
        } else if (!strcmp(rend->name, "night-texture")) {
          nightTex = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "specular-map")) {
          spec = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "bump-map")) {
          bump = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "shader")) {
          shader = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "material")) {
          loadMaterial(mat, rend);
        }
      }
    }
  }

  if (isnan(gm)) {
    gm = mass*PL_G;
  }
  // Period will be in years assuming that semiMajor is in au
  //double period = pl_orbital_period(semiMajor, sys->orbitalBody->GM * gm) / PL_SEC_PER_DAY;

  //  double period = 0.1;//comp_orbital_period_for_planet(semiMajor);
  sg_object_t *drawable = sg_new_sphere(moonName.u.str, sg_get_shader(shader), radius,
                                        sg_load_texture(tex),
                                        sg_load_texture(nightTex),
                                        sg_load_texture(spec), mat);


  sg_scene_add_object(sc, drawable); // TODO: scale to radius
  pl_celobject_t *celbody = pl_world_get_celobject(world, moonName.u.str);

  sg_object_set_celestial_body(drawable, celbody);


  //pl_system_t *moonSys = pl_new_sub_orbit(sys, moonName.u.str, mass, gm,
  //                                  period, axialTilt, siderealPeriod,
  //                                  semiMajor, ooGeoComputeSemiMinor(semiMajor, ecc),
  //                                  inc, longAscNode, longPerihel, meanLong, radius, flattening);

  //moonSys->orbitalBody->atm = NULL; // Init as vaccuum

  if (shader) {
    sg_object_set_shader_by_name(drawable, shader);
  }

  //sg_object_set_rigid_body(drawable, &moonSys->orbitalBody->obj);

  //plSetDrawable(moonSys->orbitalBody, drawable);
}

pl_atm_template_t*
load_atm(HRMLobject *obj)
{
  double scale_height = NAN, pressure = NAN;
  double g0 = NAN, M = NAN;
  const double *h_b = NULL, *p_b = NULL, *P_b = NULL, *L_b = NULL, *T_b = NULL;
  size_t h_b_len = 0, p_b_len = 0, P_b_len = 0, L_b_len = 0, T_b_len = 0;
  for ( ; obj != NULL; obj = obj->next) {
    if (!strcmp(obj->name, "surface-pressure")) {
      pressure = hrmlGetReal(obj);
    } else if (!strcmp(obj->name, "scale-height")) {
      scale_height = hrmlGetReal(obj);
    } else if (!strcmp(obj->name, "h_b")) {
      h_b = hrmlGetRealArray(obj);
      h_b_len = hrmlGetRealArrayLen(obj);
    } else if (!strcmp(obj->name, "P_b")) {
      P_b = hrmlGetRealArray(obj);
      P_b_len = hrmlGetRealArrayLen(obj);
    } else if (!strcmp(obj->name, "p_b")) {
      p_b = hrmlGetRealArray(obj);
      p_b_len = hrmlGetRealArrayLen(obj);
    } else if (!strcmp(obj->name, "L_b")) {
      L_b = hrmlGetRealArray(obj);
      L_b_len = hrmlGetRealArrayLen(obj);
    } else if (!strcmp(obj->name, "T_b")) {
      T_b = hrmlGetRealArray(obj);
      T_b_len = hrmlGetRealArrayLen(obj);
    } else if (!strcmp(obj->name, "g0")) {
      g0 = hrmlGetReal(obj);
    } else if (!strcmp(obj->name, "molar-mass")) {
      M = hrmlGetReal(obj);
    }
  }

  if (!(T_b && L_b && p_b && P_b && h_b)) {
    log_error("missing one or more atmospheric parameter array "
               "(T_b, L_b, p_b, P_b or h_b)");
    return NULL;
  }

  if (!(T_b_len == L_b_len) && (L_b_len == p_b_len) && (p_b_len == P_b_len)
      && (P_b_len == h_b_len)) {
    log_error("atmospheric parameter arrays must be of equal length"
               "(%d %d %d %d %d)",
               (int)T_b_len, (int)L_b_len, (int)p_b_len, (int)P_b_len,
               (int)h_b_len);
    return NULL;
  }

  if (!isfinite(g0)) {
    log_error("atmospheric parameter g0 not set / finite");
    return NULL;
  }
  if (!isfinite(M)) {
    log_error("atmospheric parameter molar-mass not set / finite");
    return NULL;
  }

  pl_atm_template_t *atm = pl_new_atmosphere_template(h_b_len, g0, M, p_b, P_b, T_b, h_b,
                                            L_b);
  return atm;
}

void
ooLoadPlanet__(pl_world_t *world, HRMLobject *obj, sg_scene_t *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);

  HRMLvalue planetName = hrmlGetAttrForName(obj, "name");

  pl_atm_template_t *atm = NULL;
  double mass, radius = NAN, siderealPeriod, axialTilt = 0.0, gm = NAN;
  double semiMajor = NAN, ecc, inc = NAN, longAscNode = NAN, longPerihel = NAN, meanLong;
  //double pressure = 0.0, scale_height = 1.0; //TODO
  const char *tex = NULL;
  const char *shader = NULL;
  const char *spec = NULL;
  const char *bump = NULL;
  const char *nightTex = NULL;
  HRMLobject *sats = NULL;
  sg_material_t *mat = sg_new_material();
  sg_material_init(mat);

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
      atm = load_atm(child->children);
    } else if (!strcmp(child->name, "rendering")) {
      for (HRMLobject *rend = child->children; rend != NULL; rend = rend->next) {
        if (!strcmp(rend->name, "model")) {

        } else if (!strcmp(rend->name, "texture")) {
          tex = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "night-texture")) {
          nightTex = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "specular-map")) {
          spec = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "bump-map")) {
          bump = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "shader")) {
          shader = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "material")) {
          loadMaterial(mat, rend);
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
  //double period = pl_orbital_period(pl_au_to_metres(semiMajor), world->rootSys->orbitalBody->GM+gm) / PL_SEC_PER_DAY;
  pl_celobject_t *celbody = pl_world_get_celobject(world, planetName.u.str);
  sg_object_t *drawable = sg_new_sphere(planetName.u.str,
                                        sg_get_shader(shader), radius,
                                        sg_load_texture(tex),
                                        sg_load_texture(nightTex),
                                        sg_load_texture(spec),
                                        mat);

  sg_scene_add_object(sc, drawable); // TODO: scale to radius
  //pl_system_t *sys = pl_new_orbit(world, planetName.u.str,
  //                           mass, gm,
  //                           period, axialTilt, siderealPeriod,
  //                           pl_au_to_metres(semiMajor),
  //                           pl_au_to_metres(ooGeoComputeSemiMinor(semiMajor, ecc)),
  //                           inc, longAscNode, longPerihel, meanLong, radius, flattening);

  char elname[strlen(planetName.u.str)+5];
  strcpy(elname, planetName.u.str);
  strcat(elname, ".orb");
  sg_object_t *ellipse = sg_new_ellipse(elname, sg_get_shader("flat"),
                                        pl_au_to_metres(semiMajor), ecc,
                                        DEG_TO_RAD(inc), DEG_TO_RAD(longAscNode),
                                        DEG_TO_RAD(longPerihel), 500);

  char axname[strlen(planetName.u.str)+6];
  strcpy(axname, planetName.u.str);
  strcat(axname, ".axis");

  sg_object_t *axises = sg_new_axises_with_prime(axname, sg_get_shader("flat"),
                                                 radius*2);
  sg_object_set_celestial_body(axises, celbody);
  sg_scene_add_object(sc, axises);

  pl_celobject_t *solar_celbody = pl_world_get_celobject(world, "sun");

  sg_object_set_celestial_body(ellipse, solar_celbody);
  sg_object_set_celestial_rot_body(ellipse, celbody);
  //  cm_compute_mean_orbital_elements_j2000(cm_orbital_elements_t *elems,
  //                                         cm_body_id_t planet, double T)


  sg_scene_add_object(sc, ellipse);
  //sys->orbitalBody->atm = NULL; // Init as vaccuum
  //if (atm) sys->orbitalBody->atm = pl_new_atmosphere(1000.0, 100000.0, atm);

  sg_object_set_celestial_body(drawable, celbody);
  //sg_object_set_rigid_body(drawable, &sys->orbitalBody->obj);

  if (sats) {
    for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
      if (!strcmp(sat->name, "moon")) {
        ooLoadMoon__(world, sat, sc);
      }
    }
  }
}


pl_world_t*
ooLoadStar__(HRMLobject *obj, sg_scene_t *sc)
{
  assert(obj);
  assert(obj->val.typ == HRMLNode);
  HRMLvalue starName = hrmlGetAttrForName(obj, "name");
  double mass = 0.0, gm = NAN;
  double radius, siderealPeriod, axialTilt;

  const char *tex = NULL;
  const char *shader = NULL;
  const char *bump = NULL;
  sg_material_t *mat = sg_new_material();
  sg_material_init(mat);

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
        } else if (!strcmp(rend->name, "bump-map")) {
          bump = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "shader")) {
          shader = hrmlGetStr(rend);
        } else if (!strcmp(rend->name, "material")) {
          loadMaterial(mat, rend);
        }
      }
    }
  }

  if (isnan(gm)) {
    gm = mass*PL_G;
  }

  sg_scene_set_amb4f(sc, 0.2, 0.2, 0.2, 1.0);
  sg_material_set_emiss4f(mat, 1.0, 1.0, 1.0, 1.0);
  sg_object_t *drawable = sg_new_sphere(starName.u.str,
                                        sg_get_shader(shader), radius,
                                        sg_load_texture(tex),
                                        NULL,
                                        NULL,
                                        mat);

  sg_scene_add_object(sc, drawable); // TODO: scale to radius
  sg_light_t *starLightSource = sg_new_light3f(sc, 0.0f, 0.0f, 0.0f);
  sg_object_add_light(drawable, starLightSource);

  // Create the new world for physics simulation. The size parameter is used
  // in for example the barnes hut solver as the size of the solar system.
  // 100000 AU is roughly the diameter of the solar system including all comets.
  pl_world_t *world = pl_new_world(100000.0 * PL_M_PER_AU);

  //pl_world_t *world = pl_new_world(starName.u.str, mass, gm, radius,
  //                                 siderealPeriod, axialTilt, radius,
  //                                 flattening, 100000.0 * PL_M_PER_AU);

  //world->rootSys->orbitalBody->atm = NULL; // Init as vaccuum
  pl_celobject_t *celobj = pl_world_get_celobject(world, starName.u.str);
  //sg_object_set_rigid_body(drawable, &world->rootSys->orbitalBody->obj);
  sg_object_set_celestial_body(drawable, celobj);

  if (shader) {
    sg_object_set_shader_by_name(drawable, shader);
  }

  assert(sats != NULL);
  for (HRMLobject *sat = sats->children; sat != NULL; sat = sat->next) {
    if (!strcmp(sat->name, "planet")) {
      ooLoadPlanet__(world, sat, sc);
    } else if (!strcmp(sat->name, "comet")) {
    }
  }

  return world;
}
pl_world_t*
sim_load_world(sg_scene_t *sc, const char *fileName)
{
  char *file = rsrc_get_path(fileName);
  HRMLdocument *solarSys = hrmlParse(file);
  free(file);
  //HRMLschema *schema = hrmlLoadSchema(ooResGetFile("solarsystem.hrmlschema"));
  //hrmlValidate(solarSys, schema);
  if (solarSys == NULL) {
    // Parser is responsible for pestering the users with errors for now.
    return NULL;
  }

  pl_world_t *world = NULL;
  // Go through the document and handle each entry in the document

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

  //pl_sys_init(world->rootSys);
  log_info("loaded solar system");
  return world;
}
