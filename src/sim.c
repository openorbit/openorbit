/*
  Copyright 2006,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "sim.h"
#include "sim/spacecraft-control.h"
#include "sim/world-loader.h"
#include "sim/mfd/mfd.h"
#include "sim/pubsub.h"

#include "res-manager.h"
#include "physics/orbit.h"
#include "rendering/sky.h"
#include "rendering/planet.h"
#include "settings.h"
#include "io-manager.h"


#include <openorbit/log.h>

SIMstate gSIM_state = {0.0, NULL, NULL, NULL, NULL};

void
ooSimInit(void)
{
  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  gSIM_state.stepSize = 1.0 / freq; // Period in s
  //gSIM_state.evQueue = simNewEventQueue();


  SGdrawable *sky = ooSkyNewDrawable("data/stars.csv");
  gSIM_state.sg = sgNewSceneGraph();
  sgSetSky(gSIM_state.sg, sky);

  gSIM_state.world = ooOrbitLoad(gSIM_state.sg, "data/solsystem.hrml");



  sim_spacecraft_t *sc = simNewSpacecraft("Mercury", "Mercury I");
  ooScSetSysAndCoords(sc, "Sol/Earth",
                      0.0 /*longitude*/,
                      0.0 /*latitude*/,
                      250.0e3 /*altitude*/);
  simSetSpacecraft(sc);

  SGscene *scene = sgGetScene(gSIM_state.sg, "main");

  SGcam *cam = sgNewOrbitCam(gSIM_state.sg, scene, ooScGetPLObjForSc(sc),
                             0.0, 0.0, 20.0);
  sgSetCam(gSIM_state.sg, cam);

  simMfdInitAll(gSIM_state.sg);
}


void
ooSimSetSg(SGscenegraph *sg)
{
  gSIM_state.sg = sg;
}


void
ooSimSetOrbSys(PLsystem *osys)
{
  gSIM_state.orbSys = osys;
}

void
ooSimSetOrbWorld(PLworld *world)
{
  gSIM_state.world = world;
}

void
simAxisPush(void)
{
  sim_record_t *io = simPubsubGetRecord("/io/axis");
  if (io) {
    float yaw_val = ioGetAxis(IO_AXIS_RZ);
    sim_value_t *yaw = simGetValueByName(io, "yaw");
    simPubsubSetVal(yaw, SIM_TYPE_FLOAT, &yaw_val);

    float roll_val = ioGetAxis(IO_AXIS_RY);
    sim_value_t *roll = simGetValueByName(io, "roll");
    simPubsubSetVal(roll, SIM_TYPE_FLOAT, &roll_val);

    float pitch_val = ioGetAxis(IO_AXIS_RX);
    sim_value_t *pitch = simGetValueByName(io, "pitch");
    simPubsubSetVal(pitch, SIM_TYPE_FLOAT, &pitch_val);

    float throttle_val = ioGetSlider(IO_SLIDER_THROT_0);
    sim_value_t *throttle = simGetValueByName(io, "throttle");
    simPubsubSetVal(throttle, SIM_TYPE_FLOAT, &throttle_val);

    ooLogTrace("axises: %f %f %f / %f",
               pitch_val, roll_val, yaw_val, throttle_val);
  }
}

void
ooSimStep(float dt)
{
  simTimeTick(dt);
  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);

  simAxisPush();
  sgCamStep(sgGetCam(gSIM_state.sg), dt);

  plWorldClear(gSIM_state.world);

  gettimeofday(&end, NULL);

  ooLogTrace("simstep time: %lu us", ((end.tv_sec*1000000 + end.tv_usec) -
                                     (start.tv_sec*1000000 + start.tv_usec)));

  // Step spacecraft systems
  simScStep(gSIM_state.currentSc, dt);
  simDispatchPendingEvents();

  plWorldStep(gSIM_state.world, dt);
}

void
simSetSpacecraft(sim_spacecraft_t *sc)
{
  gSIM_state.currentSc = sc;
  // Update standard pubsub links
  sim_record_t *axis_rec = simPubsubGetRecordWithComps("sc", sc->name,
                                                       "axis",
                                                       NULL);

  sim_record_t *io_rec = simPubsubCreateRecord("/io");
  simLinkRecord(io_rec, "axis", axis_rec);

  // Notify spacecraft that it is current (so it can update custom pubsub links)
}

sim_spacecraft_t*
simGetSpacecraft(void)
{
  return gSIM_state.currentSc;
}

SGscenegraph*
simGetSg(void)
{
  return gSIM_state.sg;
}

PLworld*
simGetWorld(void)
{
  return gSIM_state.world;
}
