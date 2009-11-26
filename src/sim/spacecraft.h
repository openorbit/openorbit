/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SPACECRAFT_H_7SCB1CH8
#define SPACECRAFT_H_7SCB1CH8

#include <ode/ode.h>

#include <openorbit/openorbit.h>
#include <gencds/array.h>

#include <vmath/vmath.h>
#include "physics/physics.h"
#include "simenvironment.h"

#include "simtypes.h"

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
  float yaw;
  float pitch;
  float roll;
  float horizontal;
  float vertical;
  float thrust;
} OOaxises;


typedef enum OOstagestate {
  OO_Stage_Idle,
  OO_Stage_Detatched,
  OO_Stage_Enabled
} OOstagestate;

struct OOstage {
  OOstagestate state;
  int detachOrder; // How and when to detach the stage
  PLmass m; // Mass and inertia tensor of stage, unit is kg
  obj_array_t engines; // Main orbital engines
  obj_array_t torquers;
};

typedef void (*OOscstepfunc)(OOspacecraft*, double dt);
typedef void (*OOscdetatchfunc)(OOspacecraft*); // Detatch stages

// Wing for lifting design
typedef struct OOwing OOwing;
typedef void (*OOwingstep)(OOwing *, const OOsimenv *env);
struct OOwing {
  OOstage *stage;
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
    to be recompuded from the remaining stages.

    Even though cog may vary due to fuel tanks in the wrong place, we do not
    simulate cog movement or mutation of the inertial tensor due to shape
    difference. The inertia tensor of the sc may however be updated due to
    mass difference caused by fuel consumption and so on.


 */

typedef void (*OOdetatchfunc)(OOstage*);
typedef struct OOdetatchinstr {
  short numStages;
  short stageIdx;
  OOdetatchfunc detatch;
} OOdetatchinstr;

DECL_ARRAY(OOdetatchinstr,detatchprog)

typedef struct OOdetatchprog {
  int pc;
  detatchprog_array_t instrs;
} OOdetatchprog;

struct OOspacecraft {
  obj_array_t stages;
  int activeStageIdx; // index in stage vector of active stage, this point out where
                      // detachment happens
  PLmass m;  //!< Mass of spacecraft, sum of stage masses

  OOdetatchprog detatchProg;

  OOscstepfunc prestep;
  OOscstepfunc poststep;
  OOscdetatchfunc detatchStage;
};



void ooScDetatchStage(OOspacecraft *sc);
void ooScStep(OOspacecraft *sc);
void ooScStageStep(OOspacecraft *sc, OOstage *stage, OOaxises *axises);
void ooScForce(OOspacecraft *sc, float rx, float ry, float rz);
OOspacecraft* ooScGetCurrent(void);


OOstage* ooScNewStage(void);
void ooScStageAddEngine(OOstage *stage, OOengine *engine);


OOspacecraft* ooScLoad(const char *file);

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
