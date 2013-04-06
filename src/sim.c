/*
  Copyright 2006,2010,2012,2013 Mattias Holm <lorrden(at)openorbit.org>

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
#include "menu-manager.h"

#include <openorbit/log.h>

sim_state_t gSIM_state = {0.0, NULL, NULL, NULL, NULL};

void sim_setup_menus(sim_state_t *state);


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
  config_get_int_def("openorbit/video/width", &width, 640);
  config_get_int_def("openorbit/video/height", &height, 480);
  config_get_float_def("openorbit/video/gl/fovy", &fovy, 45.0f);
  sg_viewport_t *vp = sg_new_viewport(gSIM_state.win, 0, 0, width, height);
  sg_scene_t *scene = sg_new_scene("main");
  sg_viewport_set_scene(vp, scene);

  sg_background_t *sky = sgCreateBackgroundFromFile("data/stars.csv");
  sg_scene_set_bg(scene, sky);

  sg_camera_t *cam = sg_new_camera(scene);
  (void)cam; // TODO
}

void
sim_init_plugins(void)
{
  plugin_init();
  plugin_load_all();
  plugin_print_all();
}


void
sim_init(void)
{
  pl_init();

  sim_spacecraft_control_init();

  // Set log level, need to do that here
  const char *levStr = NULL;
  config_get_str_def("openorbit/sys/log-level", &levStr, "info");
  log_set_level(log_get_lev_from_str(levStr));

  // Load and run initialisation script
  scripting_init();

  if (!scripting_run_file("script/init.py")) {
    log_fatal("script/init.py missing");
  }

  sim_init_graphics();

  float freq;
  config_get_float_def("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  gSIM_state.stepSize = 1.0 / freq; // Period in s
  
  // Setup IO-tables, must be done after joystick system has been initialised
  io_init();

  gSIM_state.world = sim_load_world(sim_get_scene(), "data/solsystem.hrml");
  pl_time_set(sim_time_get_jd());


  sim_spacecraft_t *sc = sim_new_spacecraft("Mercury", "Mercury I");
  sim_spacecraft_set_sys_and_coords(sc, "Earth",
                      0.0 /*longitude*/,
                      0.0 /*latitude*/,
                      250.0e3 /*altitude*/);
  sim_set_spacecraft(sc);

  sg_camera_t *cam = sg_scene_get_cam(sc->scene);
  sim_stage_t *stage = ARRAY_ELEM(sc->stages, 1);
  sg_camera_track_object(cam, stage->sgobj);
  sg_camera_follow_object(cam, stage->sgobj);
  sg_camera_set_follow_offset(cam, vf3_set(0.0, 0.0, -150.0e9));

  simMfdInitAll(sim_get_main_viewport());

  sim_init_plugins();

  if (!scripting_run_file("script/postinit.py")) {
    log_fatal("script/postinit.py missing");
  }

  sim_setup_menus(&gSIM_state);
}


void
sim_set_orb_sys(pl_system_t *osys)
{
  gSIM_state.orbSys = osys;
}

void
sim_set_orb_world(pl_world_t *world)
{
  gSIM_state.world = world;
}

void
simAxisPush(void)
{
  sim_record_t *io = sim_pubsub_get_record("/io/axis");
  if (io) {
    float yaw_val = io_get_axis(IO_AXIS_RZ);
    sim_value_t *yaw = sim_pubsub_get_value_by_name(io, "yaw");
    sim_pubsub_set_val(yaw, SIM_TYPE_FLOAT, &yaw_val);

    float roll_val = io_get_axis(IO_AXIS_RY);
    sim_value_t *roll = sim_pubsub_get_value_by_name(io, "roll");
    sim_pubsub_set_val(roll, SIM_TYPE_FLOAT, &roll_val);

    float pitch_val = io_get_axis(IO_AXIS_RX);
    sim_value_t *pitch = sim_pubsub_get_value_by_name(io, "pitch");
    sim_pubsub_set_val(pitch, SIM_TYPE_FLOAT, &pitch_val);

    float throttle_val = io_get_slider(IO_SLIDER_THROT_0);
    sim_value_t *throttle = sim_pubsub_get_value_by_name(io, "throttle");
    sim_pubsub_set_val(throttle, SIM_TYPE_FLOAT, &throttle_val);

    log_trace("axises: %f %f %f / %f",
               pitch_val, roll_val, yaw_val, throttle_val);
  }
}

void
sim_step(float dt)
{
  sim_time_tick(dt);
  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);

  simAxisPush();

  pl_world_clear(gSIM_state.world);

  gettimeofday(&end, NULL);

  log_trace("simstep time: %lu us", ((end.tv_sec*1000000 + end.tv_usec) -
                                     (start.tv_sec*1000000 + start.tv_usec)));

  // Step spacecraft systems
  sim_spacecraft_step(gSIM_state.currentSc, dt);
  sim_event_dispatch_pending();

  double jde = sim_time_get_jd();
  pl_world_step(gSIM_state.world, jde, dt);

  log_trace("sim step");

  sg_scene_sync(sim_get_scene());
}

void
sim_set_spacecraft(sim_spacecraft_t *sc)
{
  gSIM_state.currentSc = sc;
  // Update standard pubsub links
  sim_record_t *axis_rec = sim_pubsub_get_record_with_comps("sc", sc->name,
                                                       "axis",
                                                       NULL);

  sim_record_t *io_rec = sim_pubsub_create_record("/io");
  sim_pubsub_link_record(io_rec, "axis", axis_rec);

  // Notify spacecraft that it is current (so it can update custom pubsub links)
}

sim_spacecraft_t*
sim_get_spacecraft(void)
{
  return gSIM_state.currentSc;
}

pl_world_t*
sim_get_world(void)
{
  return gSIM_state.world;
}

void
menu_camera(void *arg)
{
  sg_object_t *obj = arg;

  float radius = sg_object_get_radius(obj);
  log_info("selected camera target: %s radius: %f",
           sg_object_get_name(obj), radius);

  sg_scene_t *sc = sg_object_get_scene(obj);
  sg_camera_t *cam = sg_scene_get_cam(sc);

  sg_camera_track_object(cam, obj);
  sg_camera_follow_object(cam, obj);


  sg_camera_set_follow_offset(cam, vf3_set(0.0, 0.0,
                                           radius * 3.0));

}

void
sim_setup_menus(sim_state_t *state)
{
  menu_t *cam_menu = menu_new(NULL, "Camera", NULL, NULL);

  sg_scene_t *scene = sg_window_get_scene(state->win, 0);
  size_t object_count = sg_scene_get_object_count(scene);
  const sg_object_t **objects = sg_scene_get_objects(scene);
  for (int i = 0 ; i < object_count ; i++) {
    menu_new(cam_menu, sg_object_get_name(objects[i]), menu_camera,
             (void*)objects[i]);
  }
}
