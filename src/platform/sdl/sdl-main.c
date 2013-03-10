/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <SDL/SDL.h>
#include "sim.h"
#include "settings.h"
#include "io-manager.h"
#include "sim/pubsub.h"
#include "rendering/render.h"
#include <openorbit/log.h>
#include "settings.h"
#include "sdl-window.h"
#include "sdl-events.h"
#include "common/moduleinit.h"

/* Simulator SDL events */
#define SIM_STEP_EVENT 0 // Make physics step
#define SIM_DEBUG_EVENT 1
// FIXME: Must be synced with simevent.c
#define SIM_WCT_TIMER 2

// 25Hz
#define SIM_STEP_PERIOD 40

SDL_Window *mainWindow;
SDL_GLContext mainContext;

static void
io_toggle_fs(bool buttonDown, void *data)
{
  sdl_window_toggle_fs(mainWindow);
}

INIT_IO {
  ioRegActionHandler("toggle-fullscreen", io_toggle_fs, NULL);
}

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
  log_info("fps = %d", frames);
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
sdl_main_loop(void)
{
  publish_variables();

  extern sim_state_t gSIM_state;
  config_get_float_def("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  float wc_period = 1.0 / freq; // Period in s
  Uint32 interv = (Uint32) (wc_period * 1000.0); // SDL wants time in ms
  config_get_float_def("openorbit/sim/period", &sim_period, wc_period);

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
        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              ooResizeScreen(0, 0, event.window.data1, event.window.data2, false);
              break;
            default:
              log_trace("unknown window event %d", (int)event.window.event);
          }
          break;
        }
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
          sdl_dispatch_key_down(event.key.keysym.scancode, event.key.keysym.mod);
          break;
        case SDL_KEYUP:
          sdl_dispatch_key_up(event.key.keysym.scancode, event.key.keysym.mod);

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
          config_get_bool_def("openorbit/video/fullscreen", &fullscreen, false);
          ooResizeScreen(0, 0, event.resize.w, event.resize.h, fullscreen);
        }
          break;
        case SDL_VIDEOEXPOSE:
          break;
        case SDL_USEREVENT:
          switch (event.user.code) {
            case SIM_STEP_EVENT: // this event will make a time step
              sim_step(sim_period);
              break;
            case SIM_DEBUG_EVENT: // display console?
              break;
            case SIM_WCT_TIMER: {
              sim_event_handler_fn_t timer_func = event.user.data1;
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
          log_warn("did not handle event number %d in main loop", event.type);
      }
    }

    // draw as often as possible
    sgPaint(gSIM_state.sg);

    SDL_GL_SwapWindow(mainWindow);
    frames ++;
  }
}

void
sdl_atexit(void)
{
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(mainWindow);
  SDL_Quit();
}

int init_sim(int argc, const char *argv[argc]);

/*
 Used to filter away joystick axis events
 */
static int sdl_io_filter(void *data, SDL_Event *ev)
{
  switch (ev->type) {
    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
      return 0;
    default:
      // Accept all other events
      return 1;
  }
}


int
main(int argc, const char *argv[argc])
{
  module_initialize();
  // Init SDL video subsystem
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_JOYSTICK) < 0 ) {
    log_fatal("Couldn't initialize SDL: %s", SDL_GetError());
  }

  assert(SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK);

  SDL_SetEventFilter(sdl_io_filter, NULL);

  atexit(sdl_atexit);

  bool fullscreen;
  int width, height;

  config_get_bool_def("openorbit/video/fullscreen", &fullscreen, false);
  config_get_int_def("openorbit/video/width", &width, 640);
  config_get_int_def("openorbit/video/height", &height, 480);

  mainWindow = sdl_window_init(width, height, fullscreen);
  sdl_init_gl();
  sdl_print_gl_attrs();

  init_sim(argc, argv);

  // Draw, get events...
  sdl_main_loop();
  return 0;
}
