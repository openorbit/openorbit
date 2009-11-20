/*
 *  engine.h
 *  orbit
 *
 *  Created by Mattias Holm on 2009-11-16.
 *  Copyright 2009 LIACS. All rights reserved.
 *
 */

#ifndef OO_SIM_ENGINE_H__
#define OO_SIM_ENGINE_H__
#include <vmath/vmath.h>
#include "simtypes.h"


typedef enum OOenginestate {
  OO_Engine_Disabled,
  OO_Engine_Idle,
  OO_Engine_Burning,
  OO_Engine_Fault_Closed,
  OO_Engine_Fault_Open
} OOenginestate;

typedef enum OOtorquerstate {
  OO_Torquer_Disabled,
  OO_Torquer_Idle,
  OO_Torquer_Spinning,
  OO_Torquer_Fault_Stuck,
  OO_Torquer_Fault_Spinning
} OOtorquerstate;

// Note engine structure is for both engines and RCS thrusters

typedef void (*OOenginetoggle)(OOengine *);
typedef void (*OOenginethrottle)(OOengine *, float throttle);
typedef void (*OOenginestep)(OOengine *, float dt);


// Base engine class, do not instanciate directly
struct OOengine {
  OOspacecraft *sc;
  const char *name; //!< Engine name, e.g. "Orbital Maneuvering Engine"
  OOenginestate state;
  float3 p; //!< Local position relative to stage center
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  float3 dir; //!< Unit vector with direction of thruster
  
  OOenginetoggle toggleOn;
  OOenginetoggle toggleOff;
  
  OOenginethrottle adjustThrottle;
  OOenginestep step;
};
// Liquid oxygen engine
struct OOlox_engine {
  OOengine super;
};

// Solid rocket booster engine
struct OOsrb_engine {
  OOengine super;
};

// In atmosphere jet engine
struct OOjet_engine {
  OOengine super;
};

// Maneuvering thruster
struct OOthruster {
  OOengine super;
};

// The torquer structure is for more general torquers (that are not mass expelling
// thrusters). These include magnetotorquers and anything else that include rotating
// bodies.
typedef struct OOtorquer {
  OOspacecraft *sc;
  OOtorquerstate state;
  float torque; //!< Nm
  float3 torqueAxis; //!< Unit vector around which the torque is to be applied
} OOtorquer;

OOengine* ooScNewEngine(OOspacecraft *sc,
                        float f,
                        float x, float y, float z,
                        float dx, float dy, float dz);

OOengine* ooScNewSrb(OOspacecraft *sc,
                     float f,
                     float x, float y, float z,
                     float dx, float dy, float dz);

OOengine* ooScNewLoxEngine(OOspacecraft *sc,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz,
                           float fuelPerNmPerS);

OOengine* ooScNewJetEngine(OOspacecraft *sc,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz);

OOengine* ooScNewThruster(OOspacecraft *sc,
                          float f,
                          float x, float y, float z,
                          float dx, float dy, float dz);

void ooScFireOrbital(OOspacecraft *sc);
void ooScFireVertical(OOspacecraft *sc, float dv);
void ooScFireHorizontal(OOspacecraft *sc, float dh);
void ooScFireForward(OOspacecraft *sc);
void ooScFireReverse(OOspacecraft *sc);

void ooScEngageYaw(OOspacecraft *sc, float dy);
void ooScEngagePitch(OOspacecraft *sc, float dp);
void ooScEngageRoll(OOspacecraft *sc, float dr);

#endif /* ! OO_SIM_ENGINE_H__ */
