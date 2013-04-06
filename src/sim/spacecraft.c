/*
  Copyright 2009,2010,2011,2013 Mattias Holm <lorrden(at)openorbit.org>

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
#include <openorbit/log.h>
#include "sim.h"
#include "sim/spacecraft.h"
#include "res-manager.h"
#include "parsers/hrml.h"
#include <vmath/vmath.h>
#include <gencds/hashtable.h>
#include <gencds/array.h>
#include "common/moduleinit.h"
#include "sim/actuator.h"
#include "io-manager.h"
#include "sim/pubsub.h"
#include "rendering/object.h"
#include "common/palloc.h"

extern sim_state_t gSIM_state;

typedef struct {
  const char *name;
} InitScArgs;


//static hashtable_t *gSpacecraftClasses;
#if 0
sim_spacecraft_t*
sim_new_spacecraft(const char *class_name, const char *sc_name)
{
  sim_class_t *cls = sim_class_get(class_name);
  sim_spacecraft_t *sc = cls->alloc(cls);

  InitScArgs args = {sc_name};
  cls->init(cls, sc, &args);
  return sc;
}
#endif

#if 0
sim_stage_t*
sim_new_stage(sim_spacecraft_t *sc, const char *name, const char *mesh)
{
  sim_stage_t *stage = smalloc(sizeof(sim_stage_t));

  stage->state = SIM_STAGE_IDLE;
  stage->sc = sc;
  stage->expendedMass = 0.0;
  stage->rec = sim_pubsub_make_record(sc->rec, name);

  obj_array_init(&stage->engines);
  obj_array_init(&stage->actuatorGroups);
  obj_array_init(&stage->payload);

  stage->obj = pl_new_sub_object3f(sc->world, sc->obj, name, 0.0, 0.0, 0.0);

  obj_array_push(&sc->stages, stage);

  // Load stage model
  sg_object_t *model = sg_load_object(mesh, sg_get_shader("spacecraft"));
  stage->sgobj = model;
  sg_scene_t *scene = sc->scene; // TODO: FIX
  sg_scene_add_object(scene, model);

  return stage;
}
#endif

#if 0
void
sim_new_spacecraft_class(const char *name, sim_spacecraft_t *(*alloc)(void),
                         void (*init)(sim_spacecraft_t *sc))
{
  sim_sc_class_t *cls = smalloc(sizeof(sim_sc_class_t));

  cls->name = strdup(name);
  cls->alloc = alloc;
  cls->init = init;
  cls->dealloc = NULL;

  hashtable_insert(gSpacecraftClasses, name, cls);
}
#endif

sim_spacecraft_t*
sim_new_spacecraft(const char *className, const char *scName)
{
  sim_class_t *cls = sim_class_get(className);

  if (!cls) {
    log_error("no such spacecraft class '%s'", className);
    return NULL;
  }

  sim_spacecraft_t *sc = cls->alloc(cls);
  //sim_spacecraft_init(sc, scName);
  InitScArgs args = {scName};
  cls->init(cls, sc, &args);

  pl_object_update_mass(sc->obj);
  // TODO: Update sg properties.
  //       sim_spacecraft_set_scene(sc, sgGetScene(simGetSg(), "main"));

  //  char *axises;
  // asprintf(&axises, "/sc/%s/axis", scName);

  sim_record_t *rec = sim_pubsub_make_record(sc->rec, "axis");

  if (rec) {
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "yaw", &sc->axises.yaw);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "roll", &sc->axises.roll);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "pitch", &sc->axises.pitch);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "lateral", &sc->axises.lateral);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "vertical", &sc->axises.vertical);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "forward", &sc->axises.fwd);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "orbital", &sc->axises.orbital);
    sim_pubsub_publish_val(rec, SIM_TYPE_FLOAT, "throttle", &sc->axises.throttle);
  }

  return sc;
}


void
sim_get_axises(sim_axises_t *axises)
{
  SIM_VAL(axises->pitch) = io_get_axis(IO_AXIS_Y);
  SIM_VAL(axises->roll) = io_get_axis(IO_AXIS_X);
  SIM_VAL(axises->yaw) = io_get_axis(IO_AXIS_RZ);

  SIM_VAL(axises->vertical) = io_get_axis(IO_AXIS_Z);
  SIM_VAL(axises->lateral) = io_get_axis(IO_AXIS_RX);
  SIM_VAL(axises->fwd) = io_get_axis(IO_AXIS_RY);

  SIM_VAL(axises->orbital) = io_get_slider(IO_SLIDER_THROT_0);
}


/*
  Compute the wing lift for a simple wing

  The simple wing relies of having precomputed lift coefficients. We allow one
  per 5 deg angle of attack. We actually intepolate between two angles to
  produce a little bit better results.
 */
double
ooSimpleWingLift(sim_simplewing_t *wing, const sim_environment_t *env)
{
  double p = env->airDensity;  // air density
  double v = 0.0; // Velocity
  double a = wing->area;  // Wing area

  unsigned idx = (((unsigned)RAD_TO_DEG(env->aoa)) % 360) / 5;

  double cl0 = wing->liftCoeffs[idx]; // lift coefficient
  double cl1 = wing->liftCoeffs[(idx+1)%72]; // lift coefficient

  double cl = cl0 + (cl1 - cl0) * fmod(RAD_TO_DEG(env->aoa), 5.0);
  double L = 0.5 * p * v * v * a * cl;

  return L;
}

void
sim_spacecraft_default_detatch(sim_spacecraft_t *sc)
{

}

void
sim_spacecraft_default_prestep(sim_spacecraft_t *sc, double dt)
{

}

void
sim_spacecraft_default_poststep(sim_spacecraft_t *sc, double dt)
{

}

void
sim_spacecraft_default_engine_toggle(sim_spacecraft_t *sc)
{
}

void
sim_spacecraft_default_axis_update(sim_spacecraft_t *sc)
{
}




void
sim_spacecraft_init(sim_spacecraft_t *sc, const char *name)
{
  sc->name = strdup(name);

  char *sckey;
  asprintf(&sckey, "/sc/%s", name);
  sc->rec = sim_pubsub_create_record(sckey);
  free(sckey);

  pl_world_t *world = sim_get_world();

  obj_array_init(&sc->stages);
  obj_array_init(&sc->engines);

  sc->world = world;
  sc->prestep = sim_spacecraft_default_prestep;
  sc->poststep = sim_spacecraft_default_poststep;
  sc->detatchPossible = true;
  sc->detatchComplete = true;
  sc->detatchStage = sim_spacecraft_default_detatch;
  sc->detatchSequence = 0;
  sc->obj = pl_new_object(world, name);
  sc->scene = NULL;//sgGetScene(sg, "main"); // Just use any of the existing ones
  sc->expendedMass = 0.0;
  sc->mainEngineOn = false;
  sc->toggleMainEngine = sim_spacecraft_default_engine_toggle;
  sc->axisUpdate = sim_spacecraft_default_axis_update;
  pl_mass_set(&sc->obj->m, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

#if 0
// TODO: Pass on pl_system_t instead of pl_world_t to ensure that object has valid
//       systems at all times.
sim_spacecraft_t*
sim_new_spacecraft(pl_world_t *world, sg_scene_t *scene, const char *name)
{
  sim_spacecraft_t *sc = smalloc(sizeof(sim_spacecraft_t));

  sim_spacecraft_init(sc, name);

  return sc;
}
#endif

//void
//dMassSetConeTotal(dMass *m, dReal total_mass,
//                  dReal radius, dReal height)
//{
//  float i11 = 3.0/5.0 * total_mass * height * height +
//    3.0/20.0 * total_mass * radius * radius;
//  float i22 = i11;
//  float i33 = 3.0/10.0 * total_mass * radius * radius;
//  float cogx, cogy, cogz;

//  cogx = 0.0;
//  cogy = 0.25 * height; // 1/4 from base, see wikipedia entry on Cone_(geometry)
//  cogz = 0.0;


//  dMassSetParameters(m, total_mass,
//                     cogx, cogy, cogz, // TODO: fix, COG
//                     i11, i22, i33,
//                    0.0, 0.0, 0.0);
//}


void
sim_spaccraft_detatch_stage(sim_spacecraft_t *sc, sim_stage_t *stage)
{
  stage->state = SIM_STAGE_DETATCHED;
  pl_object_detatch(stage->obj);
}

void
sim_spacecraft_toggle_main_engine(sim_spacecraft_t *sc)
{
  sc->toggleMainEngine(sc);
}

pl_object_t*
sim_spacecraft_get_pl_obj(sim_spacecraft_t *sc)
{
  return (pl_object_t*)sc->obj;
}


void
sim_spacecraft_step(sim_spacecraft_t *sc, float dt)
{
  assert(sc != NULL);


  sc->expendedMass = 0.0;

  sim_axises_t axises;
  sim_get_axises(&axises);

  sc->prestep(sc, dt);
  sc->axisUpdate(sc);
  for (size_t i = 0 ; i < sc->stages.length ; ++ i) {
    sim_stage_t *stage = sc->stages.elems[i];
    sim_stage_step(stage, &axises, dt);
  }

  pl_mass_mod(&sc->obj->m, sc->obj->m.m - sc->expendedMass);

  sc->poststep(sc, dt);
}

void // for scripts and events
sim_spacecraft_force(sim_spacecraft_t *sc, float rx, float ry, float rz)
{
  pl_object_force_relative3f(sc->obj, rx, ry, rz);
}


void
sim_stage_set_mesh(sim_stage_t *stage, sg_object_t *mesh)
{
  assert(stage != NULL);
  assert(mesh != NULL);
  stage->sgobj = mesh;
}

void
sim_stage_sync(sim_stage_t *stage)
{
  //  stage->mesh->p;
  //  stage->mesh->q;
}

void
sim_spacecraft_sync(sim_spacecraft_t *sc)
{
  for (size_t i = 0 ; i < sc->stages.length; ++ i) {
    sim_stage_sync(sc->stages.elems[i]);
  }
}

void
sim_stage_step(sim_stage_t *stage, sim_axises_t *axises, float dt) {
  assert(stage != NULL);
  assert(axises != NULL);

  // Handle for all actuators call actuator handlers
  // FIXME
  //const static char * axisKeys[] = {
  //  "main-throttle",
  //  "vertical-throttle", "horizontal-throttle", "distance-throttle",
  //  "pitch", "roll", "yaw"
  //};
  stage->expendedMass = 0.0f;

  for (int i = 0 ; i < SIM_ACT_GROUP_COUNT ; ++i) {
    sim_actuatorgroup_t *actGroup = (sim_actuatorgroup_t*)stage->actuatorGroups.elems[i];
    //double axisVal = ooIoGetAxis(NULL, axisKeys[i]);
    for (int j = 0 ; j < actGroup->actuators.length ; ++j) {
      //OOactuator *act = actGroup->actuators.elems[j];
      // TODO: Will not really work, we need more parameters if an actuator is
      //       used for multiple functions.
      //act->axisUpdate(act, axisVal);
    }
  }

  ARRAY_FOR_EACH(i, stage->engines) {
    sim_engine_t *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state & SIM_ENGINE_BURNING_BIT) {
      log_info("engine %s burning", eng->name);
      eng->step(eng, dt);
    }
  }

  pl_mass_mod(&stage->obj->m, stage->obj->m.m - stage->expendedMass);
  stage->sc->expendedMass += stage->expendedMass;
}

sim_stage_t*
sim_new_stage(sim_spacecraft_t *sc, const char *name, const char *mesh)
{
  sim_stage_t *stage = smalloc(sizeof(sim_stage_t));
  stage->state = SIM_STAGE_IDLE;
  stage->sc = sc;
  stage->expendedMass = 0.0;
  stage->rec = sim_pubsub_make_record(sc->rec, name);

  obj_array_init(&stage->engines);
  obj_array_init(&stage->actuatorGroups);
  obj_array_init(&stage->payload);
  for (int i = 0 ; i < SIM_ACT_GROUP_COUNT ; ++i) {
    sim_actuatorgroup_t *actGroup = sim_new_actuator_group(sim_get_actuator_group_name(i));
    obj_array_push(&stage->actuatorGroups, actGroup);
  }

  stage->obj = pl_new_sub_object3f(sc->world, sc->obj, name, 0.0, 0.0, 0.0);

  obj_array_push(&sc->stages, stage);

  // Load stage model
  sg_object_t *model = sg_load_object(mesh, sg_get_shader("spacecraft"));

  sim_stage_set_mesh(stage, model);
  sg_scene_t *scene = sc->scene; // TODO: FIX
  sg_scene_add_object(scene, model);
  sg_object_set_rigid_body(model, stage->obj);

  return stage;
}

void
sim_stage_set_offset3f(sim_stage_t *stage, float x, float y, float z)
{
  stage->pos = vf3_set(x, y, z);
  stage->obj->p_offset = vf3_set(x, y, z);
}

void
sim_stage_set_offset3fv(sim_stage_t *stage, float3 p)
{
  stage->pos = p;
  stage->obj->p_offset = p;
}

void
sim_spacecraft_set_scene(sim_spacecraft_t *spacecraft, sg_scene_t *scene)
{
  for (int i = 0 ; i < spacecraft->stages.length ; ++i) {
    sim_stage_t *stage = spacecraft->stages.elems[i];
    (void)stage;
    // TODO: Fixme
    //if (stage->state != SIM_STAGE_DETATCHED) {
    //  sg_scene_add_object(scene, stage->);
    //}
  }
}

#if 0
void
sim_spacecraft_set_system(sim_spacecraft_t *spacecraft, pl_system_t *sys)
{
  pl_system_t *oldSys = spacecraft->obj->sys;

  if (oldSys != NULL) {
    for (int i = 0 ; i < oldSys->rigidObjs.length ; ++i) {
      pl_object_t *oldSysObj = oldSys->rigidObjs.elems[i];
      if (oldSysObj == spacecraft->obj) {
        obj_array_remove(&oldSys->rigidObjs, i);
        break;
      }
    }
  }
  obj_array_push(&sys->rigidObjs, spacecraft->obj);
  spacecraft->obj->sys = sys;
}
#endif

void
sim_spacecraft_set_pos(sim_spacecraft_t *sc, double x, double y, double z)
{
  pl_object_set_pos3d(sc->obj, x, y, z);
}

void
sim_spacecraft_set_sys_and_pos(sim_spacecraft_t *sc, const char *sysName,
                               double x, double y, double z)
{
  //pl_astrobody_t *astrobody = pl_world_get_object(sc->world, sysName);
  pl_celobject_t *body = pl_world_get_celobject(sc->world, sysName);
  if (body != NULL) {
    pl_object_set_pos_celobj_rel(sc->obj, body, vf3_set(x, y, z));
    //sim_spacecraft_set_system(sc, astrobody->sys);
    float3 v = vf3_set(body->cm_orbit->v.x, body->cm_orbit->v.y, body->cm_orbit->v.z);//pl_compute_current_velocity(astrobody);
    pl_object_set_vel3fv(sc->obj, v);
  } else {
    log_warn("astrobody '%s' not found", sysName);
  }
}

void
sim_spacecraft_set_sys_and_coords(sim_spacecraft_t *sc, const char *sysName,
                                  double longitude, double latitude,
                                  double altitude)
{
  // Find planetoid object
  pl_celobject_t *body = pl_world_get_celobject(sc->world, sysName);
  //pl_astrobody_t *astrobody = pl_world_get_object(sc->world, sysName);
  if (body != NULL) {
    // Compute position relative to planet centre, this requires the equatorial
    // radius and the eccentricity of the spheroid, we shoudl also adjust for
    // sideral rotation.
    // TODO: angular eccentricity not provided at present.
    float3 p = geodetic2cart_f(body->cm_orbit->radius, 0.0,
                               latitude, longitude, altitude);
    //pl_object_set_pos_rel3fv(sc->obj, &astrobody->obj, p);
    pl_object_set_pos_celobj_rel(sc->obj, body, p);

    //sim_spacecraft_set_system(sc, astrobody->sys);

    float3 v = vf3_set(body->cm_orbit->v.x, body->cm_orbit->v.y,
                       body->cm_orbit->v.z); //pl_compute_current_velocity(astrobody);

    // Compute standard orbital velocity
    float3 velvec = vf3_normalise(vf3_cross(p, vf3_set(0.0f, 0.0f, 1.0f)));
    velvec = vf3_s_mul(velvec, sqrtf(body->cm_orbit->GM/vf3_abs(p)));
    pl_object_set_vel3fv(sc->obj, vf3_add(v, velvec));
  }
}

void
sim_stage_init(sim_stage_t *stage)
{
}

void
sim_spacecraft_add_stage(sim_spacecraft_t *sc, sim_stage_t *stage)
{
  obj_array_push(&sc->stages, stage);
}

// TODO: This is buggy, gravity is only set on the parent object unless the
//       stage has been detatched.
float3
sim_stage_get_gravity(sim_stage_t *stage)
{
  return stage->obj->g_ack;
}

float3
sim_spacecraft_get_gravity(sim_spacecraft_t *sc)
{
  // TODO: Return the real gravity instead of force ackumulator
  return sc->obj->g_ack;
}

float3
sim_spacecraft_get_velocity(sim_spacecraft_t *sc)
{
  return sc->obj->v;
}

float3
sim_spacecraft_get_airspeed(sim_spacecraft_t *sc)
{
  return pl_compute_airvelocity(sc->obj);
}

float3
sim_spacecraft_get_force(sim_spacecraft_t *sc)
{
  return sc->obj->f_ack;
}

quaternion_t
sim_spacecraft_get_quaternion(sim_spacecraft_t *sc)
{
  return sc->obj->q;
}

const float3x3*
sim_spacecraft_get_rotmat(sim_spacecraft_t *sc)
{
  return (const float3x3*)&sc->obj->R;
}

float
sim_spacecraft_get_altitude(sim_spacecraft_t *sc)
{
  return pl_object_compute_altitude(sc->obj);
}

#if 0
float3
sim_spacecraft_get_rel_pos(sim_spacecraft_t *sc)
{
  return lwc_dist(&sc->obj->p, &sc->obj->sys->orbitalBody->obj.p);
}

float3
sim_spacecraft_get_rel_vel(sim_spacecraft_t *sc)
{
  return sc->obj->v - sc->obj->sys->orbitalBody->obj.v;
}

pl_system_t*
sim_spacecraft_get_sys(sim_spacecraft_t *sc)
{
  return sc->obj->sys;
}
#endif

void
sim_stage_arm_engines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    sim_engine_t *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_DISARMED) {
      sim_engine_arm(eng);
    }
  }
}

void
sim_stage_disarm_engines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    sim_engine_t *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_ARMED) {
      sim_engine_disarm(eng);
    }
  }
}

void
sim_stage_disable_engines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    sim_engine_t *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_BURNING) {
      sim_engine_disable(eng);
    }
  }
}

void
sim_stage_lock_engines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    sim_engine_t *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_BURNING || eng->state == SIM_ARMED) {
      sim_engine_lock(eng);
    }
  }
}



static void
InitSpacecraft(sim_class_t *cls, void *obj, void *arg)
{
  SIM_SUPER_INIT(cls, obj, arg);

  sim_spacecraft_t *sc = obj;
  InitScArgs *args = arg;

  sc->super.name = strdup(args->name);

  pl_world_t *world = sim_get_world();

  obj_array_init(&sc->stages);
  obj_array_init(&sc->engines);

  sc->world = world;
  sc->prestep = sim_spacecraft_default_prestep;
  sc->poststep = sim_spacecraft_default_poststep;
  sc->detatchPossible = true;
  sc->detatchComplete = true;
  sc->detatchStage = sim_spacecraft_default_detatch;
  sc->detatchSequence = 0;
  sc->obj = pl_new_object(world, args->name);
  sc->scene = sim_get_scene(); // Just use any of the existing ones
  sc->expendedMass = 0.0;
  sc->mainEngineOn = false;
  sc->toggleMainEngine = sim_spacecraft_default_engine_toggle;
  sc->axisUpdate = sim_spacecraft_default_axis_update;
  pl_mass_set(&sc->obj->m, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

  // Adding vectors to scenegraph
  char vecname[strlen(args->name) + 5];
  strcpy(vecname, args->name);
  strcat(vecname, ".vec");
  sg_object_t *vectors =
    sg_new_dynamic_vectorset(vecname, sg_get_shader("flat"), sc->obj);
  sg_scene_add_object(sc->scene, vectors);

  //pl_system_add_object(world->rootSys, sc->obj);
}

typedef struct {
  int dummy;
} InitStageArgs;

static void
InitStage(sim_class_t *cls, void *obj, void *arg)
{
  SIM_SUPER_INIT(cls, obj, arg);
  sim_stage_t *stage = obj;
  (void)stage; // TODO
}


MODULE_INIT(spacecraft, "object", NULL)
{
  log_trace("initialising 'spacecraft' module");

  sim_class_t *sc_class = sim_register_class("Object", "Spacecraft",
                                             InitSpacecraft,
                                             sizeof(sim_spacecraft_t));
  sim_class_t *stage_class = sim_register_class("Object", "Stage",
                                                InitStage,
                                                sizeof(sim_stage_t));

  // Fields in the spacecraft class
  sim_class_add_field(sc_class, SIM_TYPE_OBJ_ARR,
                      "stages", offsetof(sim_spacecraft_t, stages));
  sim_class_add_field(sc_class, SIM_TYPE_OBJ_ARR,
                      "engines", offsetof(sim_spacecraft_t, engines));


  // Fields in the stage class
  sim_class_add_field(stage_class, SIM_TYPE_OBJ,
                      "sc", offsetof(sim_stage_t, sc));
  sim_class_add_field(stage_class, SIM_TYPE_FLOAT_VEC3,
                      "pos", offsetof(sim_stage_t, pos));
  sim_class_add_field(stage_class, SIM_TYPE_OBJ_ARR,
                      "engines", offsetof(sim_stage_t, engines));
  sim_class_add_field(stage_class, SIM_TYPE_OBJ_ARR,
                      "payload", offsetof(sim_stage_t, payload));
}
