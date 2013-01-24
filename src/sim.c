/*
  Copyright 2006,2010,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "rendering/window.h"
#include "rendering/scenegraph.h"
#include "settings.h"
#include "io-manager.h"
#include "scripting/scripting.h"
#include "plugin-handler.h"

#include <openorbit/log.h>

SIMstate gSIM_state = {0.0, NULL, NULL, NULL, NULL};

sg_scene_t*
sim_get_scene(void)
{
  sg_scene_t *sc = sg_window_get_scene(gSIM_state.win, 0);
  return sc;
}

sg_viewport_t*
sim_get_main_viewport(void)
{
  return sg_window_get_viewport(gSIM_state.win, 0);
}

sg_camera_t*
sim_get_current_camera(void)
{
  sg_viewport_t* vp = sim_get_main_viewport();
  return sg_viewport_get_cam(vp);
}

void
sim_init_graphics(void)
{
  sg_load_all_shaders();
  gSIM_state.win = sg_new_window();

  int width, height;
  float fovy;
  ooConfGetIntDef("openorbit/video/width", &width, 640);
  ooConfGetIntDef("openorbit/video/height", &height, 480);
  ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);
  sg_viewport_t *vp = sg_new_viewport(gSIM_state.win, 0, 0, width, height);
  sg_scene_t *scene = sg_new_scene("main");
  sg_viewport_set_scene(vp, scene);

  sg_background_t *sky = sgCreateBackgroundFromFile("data/stars.csv");
  sg_scene_set_bg(scene, sky);

  sg_camera_t *cam = sg_new_camera();
  sg_scene_set_cam(scene, cam);

}

void
sim_init_plugins(void)
{
  ooPluginInit();
  ooPluginLoadAll();
  ooPluginPrintAll();
}


void
sim_init(void)
{
  simScCtrlInit();

  // Set log level, need to do that here
  const char *levStr = NULL;
  ooConfGetStrDef("openorbit/sys/log-level", &levStr, "info");
  ooLogSetLevel(ooLogGetLevFromStr(levStr));

  // Load and run initialisation script
  ooScriptingInit();

  if (!ooScriptingRunFile("script/init.py")) {
    ooLogFatal("script/init.py missing");
  }

  sim_init_graphics();

  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  gSIM_state.stepSize = 1.0 / freq; // Period in s
  
  // Setup IO-tables, must be done after joystick system has been initialised
  ioInit();

  gSIM_state.world = ooOrbitLoad(sim_get_scene(), "data/solsystem.hrml");



  sim_spacecraft_t *sc = simNewSpacecraft("Mercury", "Mercury I");
  ooScSetSysAndCoords(sc, "Sol/Earth",
                      0.0 /*longitude*/,
                      0.0 /*latitude*/,
                      250.0e3 /*altitude*/);
  simSetSpacecraft(sc);
  sg_camera_t *cam = sg_scene_get_cam(sc->scene);
  sim_stage_t *stage = ARRAY_ELEM(sc->stages, 0);
  sg_camera_track_object(cam, stage->sgobj);

  simMfdInitAll(sim_get_main_viewport());

  sim_init_plugins();

  if (!ooScriptingRunFile("script/postinit.py")) {
    ooLogFatal("script/postinit.py missing");
  }

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
  //sgCamStep(sgGetCam(gSIM_state.sg), dt);

  plWorldClear(gSIM_state.world);

  gettimeofday(&end, NULL);

  ooLogTrace("simstep time: %lu us", ((end.tv_sec*1000000 + end.tv_usec) -
                                     (start.tv_sec*1000000 + start.tv_usec)));

  // Step spacecraft systems
  simScStep(gSIM_state.currentSc, dt);
  simDispatchPendingEvents();

  plWorldStep(gSIM_state.world, dt);

  ooLogInfo("sim step");
  sg_scene_update(sim_get_scene());
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

PLworld*
simGetWorld(void)
{
  return gSIM_state.world;
}
