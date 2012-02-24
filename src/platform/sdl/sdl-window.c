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

#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "log.h"
#include "rendering/render.h"
#include "settings.h"


// TODO: Ensure that these tables are used for probing the depth and stencil
//       buffer modes. If they are not explicitly set by the user, go with the
//       highest value that works
#define NUM_DEPTH_MODES 17
static int sDepthModes[NUM_DEPTH_MODES] = {
  128, 96, 64, 48, 32, 24, 16, 12, 10, 8, 6, 5, 4, 3, 2, 1, 0
};

#define NUM_STENCIL_MODES 17
static int sStencilModes[NUM_STENCIL_MODES] = {
  128, 96, 64, 48, 32, 24, 16, 12, 10, 8, 6, 5, 4, 3, 2, 1, 0
};


void
sdl_window_resize(SDL_Window *window, int width, int height)
{
  ooResizeScreen(0, 0, width, height, false);
}

void
sdl_window_toggle_fs(SDL_Window *window)
{  
  Uint32 flags = SDL_GetWindowFlags(window);
  if (flags & SDL_WINDOW_FULLSCREEN)
    SDL_SetWindowFullscreen(window, SDL_FALSE);
  else
    SDL_SetWindowFullscreen(window, SDL_TRUE);
  
  SDL_DisplayMode mode;
  SDL_GetWindowDisplayMode(window, &mode);
  ooResizeScreen(0, 0, mode.w, mode.h, false);
}

SDL_Window *
sdl_window_init(int width, int height, bool fullscreen)
{
  SDL_Window *window;
  Uint32 flags = 0;
  
  flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
  flags |= (fullscreen) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
  
  // Create window
  assert(SDL_VideoModeOK(width, height, 32, flags));
  window = SDL_CreateWindow("Open Orbit", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            width, height, flags);

  if (!window) {
    ooLogError("Couldn't set %dx%d OpenGL video mode: %s\n",
               width, height, SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  
  return window;
}

void
sdl_init_gl(void)
{
  // Setup attributes we want for the OpenGL context
  int stencilSize, colourSize;
  ooConfGetIntDef("openorbit/video/stencil-bits", &stencilSize, 1);
  ooConfGetIntDef("openorbit/video/colour-bits", &colourSize, 32);
  
  // Don't set color bit sizes (SDL_GL_RED_SIZE, etc)
  //    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and
  //    5-5-5 RGB for 16-bit screens
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  
  bool depthSetFailed = true;
  for (int i = 0 ; i < NUM_DEPTH_MODES ; ++ i) {
    if (!SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, sDepthModes[i])) {
      // Take largest allowed depth buffer
      depthSetFailed = false;
      break;
    }
  }
  if (depthSetFailed) {
    ooLogFatal("could not set depth size SDL: \"%s\"", SDL_GetError());
  }
  
#if 0
  bool stencilSetFailed = true;
  for (int i = 0 ; i < NUM_STENCIL_MODES ; ++ i) {
    if (!SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, sStencilModes[i])) {
      // Take largest allowed depth buffer
      stencilSetFailed = false;
      break;
    }
  }
  if (stencilSetFailed) {
    ooLogFatal("could not set stencil size SDL: \"%s\"", SDL_GetError());
  }
#endif
  
  // Request double-buffered OpenGL
  //     The fact that windows are double-buffered on Mac OS X has no effect
  //     on OpenGL double buffering.
  if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
    ooLogFatal("could not set gl double buffer SDL: \"%s\"", SDL_GetError());
  }
  
  extern SDL_WindowID mainWindow;
  extern SDL_GLContext mainContext;
  
  mainContext = SDL_GL_CreateContext(mainWindow);
  
  /* This makes our buffer swap syncronized with the monitor's vertical refresh */
  SDL_GL_SetSwapInterval(1);
  
  // Prepare the renderinfo object
  const GLubyte *glvers = glGetString(GL_VERSION);
  const GLubyte *shadervers = glGetString(GL_SHADING_LANGUAGE_VERSION);
  
  assert(sscanf((const char*)glvers, "%u.%u",
                &sgRenderInfo.gl_major_vers,
                &sgRenderInfo.gl_minor_vers) == 2);
  
  assert(sscanf((const char*)shadervers, "%u.%u",
                &sgRenderInfo.glsl_major_vers,
                &sgRenderInfo.glsl_minor_vers) == 2);
  ooLogInfo("gl version = %u.%u", sgRenderInfo.gl_major_vers,
            sgRenderInfo.gl_minor_vers);
  ooLogInfo("glsl version = %u.%u", sgRenderInfo.glsl_major_vers,
            sgRenderInfo.glsl_minor_vers);
}


void
sdl_print_gl_attrs(void)
{
  // Print out attributes of the context we created
  int nAttr;
  int i;
  
  int  attr[] = { SDL_GL_RED_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_GREEN_SIZE,
    SDL_GL_ALPHA_SIZE, SDL_GL_BUFFER_SIZE, SDL_GL_DEPTH_SIZE,
    SDL_GL_STENCIL_SIZE};
  
  char *desc[] = { "Red size: %d bits", "Blue size: %d bits",
    "Green size: %d bits", "Alpha size: %d bits",
    "Color buffer size: %d bits",
    "Depth buffer size: %d bits",
    "Stencil buffer size: %d bits"};
  
  nAttr = sizeof(attr) / sizeof(int);
  
  for (i = 0; i < nAttr; i++) {
    int value;
    SDL_GL_GetAttribute(attr[i], &value);
    ooLogInfo(desc[i], value);
  }
}


