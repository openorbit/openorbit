/*
  Copyright 2006,2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*! \mainpage Open Orbit Source Documentation 
 *
 * \section intro_sec Introduction
 *
 * Open Orbit is an interactive space flight simulator, intended to allow the
 * user experience the wonders of space flight within the solar system. The
 * simulator works with newtonian mechanics. This differs from some other space
 * simulators, that some times use an atmospheric flight model, despite being
 * in vacuum, and microgravity.
 * 
 * \section build_sec Building
 * 
 * The build procedure is documented separatelly. See the file INSTALL.txt for
 * more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <glob.h>

#include "res-manager.h"
#include "log.h"
#include "settings.h"
#include "io-manager.h"
#include "plugin-handler.h"
#include "sim.h"
#include "rendering/render.h"
#include "rendering/camera.h"
#include "rendering/sky.h"
#include "scripting/scripting.h"

#include <ode/ode.h>
#include "SDL.h"
#include "SDL_ttf.h"

//extern settings_t SETTINGS;
/* Simulator SDL events */
#define SIM_STEP_EVENT 0 // Make physics step
#define SIM_DEBUG_EVENT 1
 
// 25Hz
#define SIM_STEP_PERIOD 40

Uint32
sim_step_event(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;


    userevent.type = SDL_USEREVENT;
    userevent.code = SIM_STEP_EVENT;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

static unsigned frames = 0;

Uint32
fps_event(Uint32 interval, void *param)
{
  ooLogInfo("fps = %d", frames);
  frames = 0;
  return interval;
}

static void
main_loop(void)
{
  extern SIMstate gSIM_state;
  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  float period = 1.0 / freq; // Period in s
  Uint32 interv = (Uint32) (period * 1000.0); // SDL wants time in ms
  SDL_Event event;
  const char *evName;
  int done = 0;
  SDL_AddTimer(interv, sim_step_event, NULL);
  SDL_AddTimer(1000, fps_event, NULL);


  while ( !done ) {
  /* Check for events, will do the initial io-decoding */
    while ( SDL_PollEvent (&event) ) {
      switch (event.type) {
      case SDL_ACTIVEEVENT:
        break;
      case SDL_MOUSEMOTION:
        if (event.motion.state) {
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        break;
      case SDL_MOUSEBUTTONUP:
        break;
      case SDL_KEYDOWN:
        evName = ooIoSdlKeyNameLookup(event.key.keysym.sym);
        ooIoDispatchKeyDown(evName, event.key.keysym.mod);
        break;
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_q) done = 1; // for now dont remap q
        else {
          evName = ooIoSdlKeyNameLookup(event.key.keysym.sym);
          ooIoDispatchKeyUp(evName, event.key.keysym.mod);
        }
        break;
      case SDL_JOYAXISMOTION:
      case SDL_JOYBALLMOTION:
        break; // Skip axis motions, we poll these every sim step
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        break;
      case SDL_VIDEORESIZE:
        {
          bool fullscreen;
          ooConfGetBoolDef("openorbit/video/fullscreen", &fullscreen, false);
          ooResizeScreen(event.resize.w, event.resize.h, fullscreen);
        }
        break;
      case SDL_VIDEOEXPOSE:
        break;
      case SDL_USEREVENT:
        switch (event.user.code) {
        case SIM_STEP_EVENT: // this event will make a time step
          ooSimStep(period);
          break;
        case SIM_DEBUG_EVENT: // display console?
          break;
        default:
          break;
        }
        break;
      case SDL_QUIT:
        done = 1;
        break;
      default:
        break;
      }
    }

    // draw as often as possible
    ooSgPaint(gSIM_state.sg);

    SDL_GL_SwapBuffers();
    frames ++;
  }
}


int
main(int argc, char*argv[])
{
  ooLogInit(stderr);

  ooConfInit();
  ooConfLoad(ooResGetConfPath());

  // Set log level, need to do that here
  const char *levStr = NULL;
  ooConfGetStrDef("openorbit/sys/log-level", &levStr, "info");
  ooLogSetLevel(ooLogGetLevFromStr(levStr));

  // Setup IO-tables
  ooIoInitSdlStringMap();

  ooSimInit();

  ooPluginInit();
  ooPluginLoadAll();
  ooPluginPrintAll();

  // Load and run initialisation script
  ooScriptingInit();

  if (!ooScriptingRunFile("script/init.py")) {
    ooLogFatal("script/init.py missing");
  }

  // Initialise ODE, SDL, GL and AL
  dInitODE();

  // Init SDL video subsystem
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK) < 0 ) {
    ooLogFatal("Couldn't initialize SDL: %s", SDL_GetError());
  }

  if (TTF_Init() == -1) {
    ooLogFatal("Couldn't initialize SDL_ttf: %s", TTF_GetError());
  }

  // Init GL state
  ooSetVideoDefaults();
  ooSgCamInit();

  ooTexInit(); // Requires that GL has been initialised

  if (!ooScriptingRunFile("script/postinit.py")) {
    ooLogFatal("script/postinit.py missing");
  }

  atexit(SDL_Quit);

  // Draw, get events...
  main_loop();

  ooLogInfo("Shutting down normally...");
  return 0;
}

