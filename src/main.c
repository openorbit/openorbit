/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006,2008 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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
#include "error.h"
#include "settings.h"
#include "io-manager.h"
#include "plugin-handler.h"
#include "sim.h"
#include "rendering/render.h"
#include "rendering/camera.h"
#include "rendering/sky.h"
#include "scripting/scripting.h"

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
    SDL_Event event;
    const char *evName;
    int done = 0;
    SDL_AddTimer(SIM_STEP_PERIOD, sim_step_event, NULL);
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
                    ooIoDispatchKeyDown(evName, event.key.keysym.mod);
                }
                break;
            case SDL_JOYAXISMOTION:
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_VIDEORESIZE:
            case SDL_VIDEOEXPOSE:
                break;
            case SDL_USEREVENT:
                switch (event.user.code) {
                case SIM_STEP_EVENT: // this event will make a time step
                    ooSimStep(0.05);
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
        //ooSgDraw(gSIM_state.sg, gSIM_state.cam);
        ooSgPaint(gSIM_state.sg);
        
        SDL_GL_SwapBuffers();
        frames ++;
	}
}


int
main(int argc, char*argv[])
{   
    ooConfInit();
    
    ooLogInit(stderr);
    // Setup IO-tables
    ooIoInitSdlStringMap();
    
    //init_cam();

    ooPluginInit();
    ooPluginLoadAll();
    ooPluginPrintAll();
        
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
    
    if (TTF_Init() == -1) {
      ooLogFatal("Couldn't initialize SDL_ttf: %s", TTF_GetError());
    }
    
    // Init GL state
    init_renderer();

    if (!ooScriptingRunFile("script/postinit.py")) {
      ooLogFatal("script/postinit.py missing");
    }
    
    
    atexit(SDL_Quit);
    
        
    // Draw, get events...
    main_loop();
    
    ooLogInfo("Shutting down normally...");
    return 0;
}

