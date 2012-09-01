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
typedef struct OOaxises {
  sim_float_t yaw;
  sim_float_t pitch;
  sim_float_t roll;
  sim_float_t lateral;
  sim_float_t vertical;
  sim_float_t fwd;
  sim_float_t orbital;
  sim_float_t throttle;
} OOaxises;


typedef enum sim_stage_state_t {
  OO_Stage_Idle,
  OO_Stage_Detatched,
  OO_Stage_Enabled
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
};

typedef void (*OOscstepfunc)(sim_spacecraft_t*, double dt);
typedef void (*OOscactionfunc)(sim_spacecraft_t*); // Detatch stages

// Wing for lifting design
typedef void (*OOwingstep)(OOwing *, const OOsimenv *env);
struct OOwing {
  sim_stage_t *stage;
  OOwingstep step;
};

typedef struct OOsimplewing OOsimplewing;
struct OOsimplewing {
  OOwing super;
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

  PLworld *world;
  PLobject *obj;

  const char *name;
  OOaxises axises;

  obj_array_t stages;
  obj_array_t engines;

  bool detatchPossible;
  bool detatchComplete;
  int detatchSequence;

  OOscstepfunc prestep;
  OOscstepfunc poststep;
  OOscactionfunc detatchStage;

  bool mainEngineOn;
  OOscactionfunc toggleMainEngine;

  OOscactionfunc axisUpdate;

  float expendedMass;

  sg_scene_t *scene;
};

typedef struct {
  const char *name;
  sim_spacecraft_t *(*alloc)(void);
  void (*init)(sim_spacecraft_t *sc);
  void (*dealloc)(sim_spacecraft_t *sc);
} SCclass;


sim_spacecraft_t* simNewSpacecraft(const char *className, const char *scName);

void simScInit(sim_spacecraft_t *sc, const char *name);
void simInitStage(sim_stage_t *stage);
void simAddStage(sim_spacecraft_t *sc, sim_stage_t *stage);

sim_spacecraft_t* ooScNew(PLworld *world, sg_scene_t *scene, const char *name);
void simScDetatchStage(sim_spacecraft_t *sc);

void simScToggleMainEngine(sim_spacecraft_t *sc);

void ooScSetStageMesh(sim_stage_t *stage, sg_object_t *mesh);
void simDetatchStage(sim_spacecraft_t *sc, sim_stage_t *stage);
void simScStep(sim_spacecraft_t *sc, float dt);
void ooScStageStep(sim_stage_t *stage, OOaxises *axises, float dt);
void ooScForce(sim_spacecraft_t *sc, float rx, float ry, float rz);
void ooScSetScene(sim_spacecraft_t *spacecraft, sg_scene_t *scene);

PLobject* ooScGetPLObjForSc(sim_spacecraft_t *sc);

/*!
  Set position of spacecraft in global coordinates
 */
void ooScSetPos(sim_spacecraft_t *sc, double x, double y, double z);

/*!
 Set position of spacecraft relative to the named system's center
 \param sc The spacecraft object
 \param sysName Name of system with / to separate levels. Eg /Sol/Earth/Luna
 \param x Position in x dimension with respect to system centre
 \param y Position in y dimension with respect to system centre
 \param z Position in z dimension with respect to system centre
 */
void ooScSetSystemAndPos(sim_spacecraft_t *sc, const char *sysName,
                         double x, double y, double z);

/*!
 Set position of spacecraft relative to the system's central objects surface coordinates
 \param sc The spacecraft object
 \param sysName Name of system with / to separate levels. Eg /Sol/Earth/Luna
 \param longitude Longitude
 \param lattidue Latitude
 \param altitude Altitude ASL
 */
void ooScSetSysAndCoords(sim_spacecraft_t *sc, const char *sysName, double longitude, double latitude, double altitude);


sim_stage_t* simNewStage(sim_spacecraft_t *sc, const char *name, const char *mesh);

sim_spacecraft_t* ooScLoad(PLworld *world, sg_scene_t *scene, const char *fileName);

void scStageSetOffset3f(sim_stage_t *stage, float x, float y, float z);
void scStageSetOffset3fv(sim_stage_t *stage, float3 p);


void ooGetAxises(OOaxises *axises);

float3 simGetStageGravityVector(sim_stage_t *stage);
//float3 simGetAirspeedVector(sim_stage_t *stage);

float3 simGetGravityVector(sim_spacecraft_t *sc);
float3 simGetVelocityVector(sim_spacecraft_t *sc);
float3 simGetAirspeedVector(sim_spacecraft_t *sc);
float3 simGetForceVector(sim_spacecraft_t *sc);
quaternion_t simGetQuaternion(sim_spacecraft_t *sc);
const float3x3* simGetRotMat(sim_spacecraft_t *sc);
float simGetAltitude(sim_spacecraft_t *sc);

// Returns the position vector that is relative to the dominating
// gravitational source
float3 simGetRelPos(sim_spacecraft_t *sc);
float3 simGetRelVel(sim_spacecraft_t *sc);
PLsystem* simGetSys(sim_spacecraft_t *sc);

void simStageArmEngines(sim_stage_t *stage);
void simStageDisarmEngines(sim_stage_t *stage);
void simStageDisableEngines(sim_stage_t *stage);
void simStageLockEngines(sim_stage_t *stage);

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
