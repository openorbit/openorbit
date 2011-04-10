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
#include "sim/pubsub.h"
#include "sim/spacecraft-control.h"
#include "rendering/render.h"
#include "rendering/camera.h"
#include "rendering/sky.h"
#include "scripting/scripting.h"
#include "rendering/texture.h"
#include "physics/physics.h"
#include "rendering/shader-manager.h"



#include <SDL/SDL.h>
//#include "SDL.h"
//#include "SDL_ttf.h"

/* Simulator SDL events */
#define SIM_STEP_EVENT 0 // Make physics step
#define SIM_DEBUG_EVENT 1
// FIXME: Must be synced with simevent.c
#define SIM_WCT_TIMER 2

// 25Hz
#define SIM_STEP_PERIOD 40

SDL_WindowID mainWindow;
SDL_GLContext mainContext;


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
static int fps_count = 0;
OOpubsubref fps_count_ref;

Uint32
fps_event(Uint32 interval, void *param)
{
  ooLogInfo("fps = %d", frames);
  fps_count = frames;
  frames = 0;
  simNotifyChange(fps_count_ref); // FIXME: NOT ASYNC SAFE

  return interval;
}

PUBSUB(static float, freq, 0.0f);
PUBSUB(static float, sim_period, 0.0f);

static void
publish_variables(void)
{
  fps_count_ref = simPublishInt("/sim/video/fps", &fps_count);
  freq_ref = simPublishFloat("/sim/freq", &freq);
  sim_period_ref = simPublishFloat("/sim/period", &sim_period);
}

static void
main_loop(void)
{
  publish_variables();

  extern SIMstate gSIM_state;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  float wc_period = 1.0 / freq; // Period in s
  Uint32 interv = (Uint32) (wc_period * 1000.0); // SDL wants time in ms
  ooConfGetFloatDef("openorbit/sim/period", &sim_period, wc_period);

  SDL_Event event;
  const char *evName;
  int done = 0;
  SDL_AddTimer(interv, sim_step_event, NULL);
  SDL_AddTimer(1000, fps_event, NULL);


  while ( !done ) {
  /* Check for events, will do the initial io-decoding */
    while ( SDL_PollEvent(&event) ) {
      switch (event.type) {
      case SDL_ACTIVEEVENT:
        break;
      case SDL_WINDOWEVENT:
        break;
      case SDL_TEXTINPUT:
        break;
      case SDL_TEXTEDITING:
          break;
      case SDL_MOUSEMOTION:
        if (event.motion.state) {
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        break;
      case SDL_MOUSEBUTTONUP:
        break;
      case SDL_MOUSEWHEEL:
          break;

      case SDL_INPUTMOTION:
      case SDL_INPUTBUTTONDOWN:
      case SDL_INPUTBUTTONUP:
      case SDL_INPUTWHEEL:
      case SDL_INPUTPROXIMITYIN:
      case SDL_INPUTPROXIMITYOUT:
        break;

      case SDL_KEYDOWN:
        ioDispatchKeyDown(event.key.keysym.scancode, event.key.keysym.mod);
        break;
      case SDL_KEYUP:
        ioDispatchKeyUp(event.key.keysym.scancode, event.key.keysym.mod);
        break;
      case SDL_JOYHATMOTION:
        break;
      case SDL_JOYBUTTONDOWN:
        ioDispatchButtonDown(event.jbutton.which, event.jbutton.button);
        break;
      case SDL_JOYBUTTONUP:
        ioDispatchButtonUp(event.jbutton.which, event.jbutton.button);
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
          ooSimStep(sim_period);
          break;
        case SIM_DEBUG_EVENT: // display console?
          break;
        case SIM_WCT_TIMER: {
            OOeventhandler timer_func = event.user.data1;
            void *timer_data = event.user.data2;
            timer_func(timer_data);
            break;
          }
        default:
          break;
        }
        break;
      case SDL_FINGERDOWN:
      case SDL_FINGERUP:
      case SDL_FINGERMOTION:
      case SDL_TOUCHBUTTONDOWN:
      case SDL_TOUCHBUTTONUP:
      case SDL_DOLLARGESTURE:
      case SDL_DOLLARRECORD:
      case SDL_MULTIGESTURE:

        break;
      case SDL_QUIT:
        done = 1;
        break;
      default:
        assert(0 && "unhandled event in main event loop");
      }
    }

    // draw as often as possible
    sgPaint(gSIM_state.sg);

    SDL_GL_SwapWindow(mainWindow);
    frames ++;
  }
}

//int
//SDL_main(int argc, char *argv[])
//{
//  assert(0 && "never go here");
//  return 0;
//}

void
sdl_atexit(void)
{
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(mainWindow);
  SDL_Quit();
}

int
main(int argc, char*argv[])
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
  // Initialise SDL, GL and AL

  // Init SDL video subsystem
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK) < 0 ) {
    ooLogFatal("Couldn't initialize SDL: %s", SDL_GetError());
  }

  // Setup IO-tables, must be done after joystick system has been initialised
  ioInit();

//  if (TTF_Init() == -1) {
//    ooLogFatal("Couldn't initialize SDL_ttf: %s", TTF_GetError());
//  }

  // Init GL state
  ooSetVideoDefaults();
  sgLoadAllShaders();
  sgCamInit();

  ooIoPrintJoystickNames();

  ooSimInit();

  ooPluginInit();
  ooPluginLoadAll();
  ooPluginPrintAll();

  if (!ooScriptingRunFile("script/postinit.py")) {
    ooLogFatal("script/postinit.py missing");
  }

  atexit(sdl_atexit);

  // Draw, get events...
  main_loop();

  ooLogInfo("Shutting down normally...");
  return 0;
}

