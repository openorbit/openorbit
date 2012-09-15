/*
  Copyright 2006,2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "res-manager.h"
#include <openorbit/log.h>
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

void
init_graphics(void)
{
  // Init GL state
  ooSetVideoDefaults();
  sg_load_all_shaders();
  //sgCamInit();
}

void
init_plugins(void)
{
  ooPluginInit();
  ooPluginLoadAll();
  ooPluginPrintAll();
}


void
init_al(void)
{
  ALCdevice *dev = alcOpenDevice(NULL);
  ALenum err = alcGetError(dev);
  if (err != ALC_NO_ERROR) {
    ooLogError("could not get the default al device");
  }

  ALCcontext *ctxt = alcCreateContext(dev, NULL);
  err = alcGetError(dev);
  if (err != ALC_NO_ERROR) {
    ooLogError("could not create al context");
  }

  if (alcMakeContextCurrent(ctxt) == ALC_FALSE) {
    ooLogError("could not make the al context current");
  }
}

int
init_sim(int argc, char*argv[])
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
  init_al();

  // Setup IO-tables, must be done after joystick system has been initialised
  ioInit();

//  if (TTF_Init() == -1) {
//    ooLogFatal("Couldn't initialize SDL_ttf: %s", TTF_GetError());
//  }

  init_graphics();


  //ooIoPrintJoystickNames();

  ooSimInit();

  init_plugins();

  if (!ooScriptingRunFile("script/postinit.py")) {
    ooLogFatal("script/postinit.py missing");
  }

//  ooLogInfo("Shutting down normally...");
  return 0;
}

