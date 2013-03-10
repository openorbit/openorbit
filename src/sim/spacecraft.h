/*
  Copyright 2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SPACECRAFT_H_7SCB1CH8
#define SPACECRAFT_H_7SCB1CH8

#include <openorbit/openorbit.h>
#include <gencds/array.h>

#include <vmath/vmath.h>
#include "physics/physics.h"
#include "simenvironment.h"
#include "rendering/scenegraph.h"
#include "simtypes.h"
#include "common/moduleinit.h"
#include "sim/pubsub.h"
#include "sim/class.h"

/*! Spacecraft system simulation header

 The spacecrafts consist of multiple stages, where several stages may be
 active at the same time (e.g. space shuttle where 2 SRBs are burning at the
 same time).

 Detatching stages is handled by a detach program. This program stores the
 index in the stage array where the detatch procedure starts and also how many
 stages will be detached at the same time (e.g. 2 for the SRBs of a space
 shuttle). The program also keep a pointer to a function called when detatching
 the stage.
*/
typedef struct {
  sim_float_t yaw;
  sim_float_t pitch;
  sim_float_t roll;
  sim_float_t lateral;
  sim_float_t vertical;
  sim_float_t fwd;
  sim_float_t orbital;
  sim_float_t throttle;
} sim_axises_t;


typedef enum sim_stage_state_t {
  SIM_STAGE_IDLE,
  SIM_STAGE_DETATCHED,
  SIM_STAGE_ENABLED
} sim_stage_state_t;

struct sim_stage_t {
  sim_object_t super;
  sim_record_t *rec;
  sim_spacecraft_t *sc;
  float3 pos;
  float expendedMass;
  sim_stage_state_t state;

  PLobject *obj; // Mass and inertia tensor of stage, unit is kg
                 //  obj_array_t actuators;
  obj_array_t engines;
  obj_array_t actuatorGroups;
  obj_array_t payload;

  sg_object_t *sgobj;
};

typedef void (*sim_sc_step_fn_t)(sim_spacecraft_t*, double dt);
typedef void (*sim_sc_action_fn_t)(sim_spacecraft_t*); // Detatch stages

// Wing for lifting design
typedef void (*sim_wing_tstep)(sim_wing_t *, const sim_environment_t *env);
struct sim_wing_t {
  sim_stage_t *stage;
  sim_wing_tstep step;
};

typedef struct sim_simplewing_t sim_simplewing_t;
struct sim_simplewing_t {
  sim_wing_t super;
  double area;
  double liftCoeffs[72]; // one per 5 degree aoa
};


/*
  Some notes that should be elsewhere:
    When detatching a stage, the spaccrafts cog and inertia tensors need
    to be recomputed from the remaining stages.

    Even though cog may vary due to fuel tanks in the wrong place, we do not
    simulate cog movement or mutation of the inertial tensor due to shape
    difference. The inertia tensor of the sc may however be updated due to
    mass difference caused by fuel consumption and so on.


 */

struct sim_spacecraft_t {
  sim_object_t super;

  sim_record_t *rec;

  pl_world_t *world;
  PLobject *obj;

  const char *name;
  sim_axises_t axises;

  obj_array_t stages;
  obj_array_t engines;

  bool detatchPossible;
  bool detatchComplete;
  int detatchSequence;

  sim_sc_step_fn_t prestep;
  sim_sc_step_fn_t poststep;
  sim_sc_action_fn_t detatchStage;

  bool mainEngineOn;
  sim_sc_action_fn_t toggleMainEngine;

  sim_sc_action_fn_t axisUpdate;

  float expendedMass;

  sg_scene_t *scene;
  sg_object_t *sgobj;
};

typedef struct {
  const char *name;
  sim_spacecraft_t *(*alloc)(void);
  void (*init)(sim_spacecraft_t *sc);
  void (*dealloc)(sim_spacecraft_t *sc);
} sim_sc_class_t;


sim_spacecraft_t* sim_new_spacecraft(const char *className, const char *scName);

void sim_spacecraft_init(sim_spacecraft_t *sc, const char *name);
void sim_stage_init(sim_stage_t *stage);
void sim_spacecraft_add_stage(sim_spacecraft_t *sc, sim_stage_t *stage);

void sim_spacecraft_toggle_main_engine(sim_spacecraft_t *sc);

void sim_stage_set_mesh(sim_stage_t *stage, sg_object_t *mesh);

void sim_spaccraft_detatch_stage(sim_spacecraft_t *sc, sim_stage_t *stage);
void sim_spacecraft_step(sim_spacecraft_t *sc, float dt);
void sim_stage_step(sim_stage_t *stage, sim_axises_t *axises, float dt);
void sim_spacecraft_force(sim_spacecraft_t *sc, float rx, float ry, float rz);
void sim_spacecraft_set_scene(sim_spacecraft_t *spacecraft, sg_scene_t *scene);

PLobject* sim_spacecraft_get_pl_obj(sim_spacecraft_t *sc);

/*!
  Set position of spacecraft in global coordinates
 */
void sim_spacecraft_set_pos(sim_spacecraft_t *sc, double x, double y, double z);

/*!
 Set position of spacecraft relative to the named system's center
 \param sc The spacecraft object
 \param sysName Name of system with / to separate levels. Eg /Sol/Earth/Luna
 \param x Position in x dimension with respect to system centre
 \param y Position in y dimension with respect to system centre
 \param z Position in z dimension with respect to system centre
 */
void sim_spacecraft_set_sys_and_pos(sim_spacecraft_t *sc, const char *sysName,
                         double x, double y, double z);

/*!
 Set position of spacecraft relative to the system's central objects surface coordinates
 \param sc The spacecraft object
 \param sysName Name of system with / to separate levels. Eg /Sol/Earth/Luna
 \param longitude Longitude
 \param lattidue Latitude
 \param altitude Altitude ASL
 */
void sim_spacecraft_set_sys_and_coords(sim_spacecraft_t *sc, const char *sysName, double longitude, double latitude, double altitude);


sim_stage_t* sim_new_stage(sim_spacecraft_t *sc, const char *name, const char *mesh);


void sim_stage_set_offset3f(sim_stage_t *stage, float x, float y, float z);
void sim_stage_set_offset3fv(sim_stage_t *stage, float3 p);


void sim_get_axises(sim_axises_t *axises);

float3 sim_stage_get_gravity(sim_stage_t *stage);
//float3 simGetAirspeedVector(sim_stage_t *stage);

float3 sim_spacecraft_get_gravity(sim_spacecraft_t *sc);
float3 sim_spacecraft_get_velocity(sim_spacecraft_t *sc);
float3 sim_spacecraft_get_airspeed(sim_spacecraft_t *sc);
float3 sim_spacecraft_get_force(sim_spacecraft_t *sc);
quaternion_t sim_spacecraft_get_quaternion(sim_spacecraft_t *sc);
const float3x3* sim_spacecraft_get_rotmat(sim_spacecraft_t *sc);
float sim_spacecraft_get_altitude(sim_spacecraft_t *sc);

// Returns the position vector that is relative to the dominating
// gravitational source
float3 sim_spacecraft_get_rel_pos(sim_spacecraft_t *sc);
float3 sim_spacecraft_get_rel_vel(sim_spacecraft_t *sc);
pl_system_t* sim_spacecraft_get_sys(sim_spacecraft_t *sc);

void sim_stage_arm_engines(sim_stage_t *stage);
void sim_stage_disarm_engines(sim_stage_t *stage);
void sim_stage_disable_engines(sim_stage_t *stage);
void sim_stage_lock_engines(sim_stage_t *stage);

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
