/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include "render.h"

#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "SDL.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "settings.h"
#include "camera.h"
#include "sky.h"
#include "texture.h"
#include "planet.h"
#include "log.h"

static SDL_Surface *gScreen = NULL;



void
ooInitSdlScreen(int width, int height, bool fullscreen)
{
  Uint32 flags = 0;

  flags = SDL_OPENGL;
  flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;

  assert(SDL_VideoModeOK(width, height, 32, flags));

  // Create window
  gScreen = SDL_SetVideoMode(width, height, 32, flags);
  if (gScreen == NULL) {
    ooLogError("Couldn't set %dx%d OpenGL video mode: %s\n",
               width, height, SDL_GetError());
    SDL_Quit();
    exit(2);
  }
  ooPrintScreenAttributes();
}

void
ooSetVideoDefaults(void)
{
  int width, height;
  float fovy;
  bool fullscreen;

  ooInitGlAttributes();

  ooConfGetBoolDef("openorbit/video/fullscreen", &fullscreen, false);
  ooConfGetIntDef("openorbit/video/width", &width, 640);
  ooConfGetIntDef("openorbit/video/height", &height, 480);
  ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);
  
  ooInitSdlScreen(width, height, fullscreen);
  ooSetPerspective(fovy, width, height);
}

void
ooResizeScreen(int width, int height, bool fullscreen)
{
  float fovy;
  ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);

  //  ooInitSdlScreen(width, height, fullscreen);
  ooSetPerspective(fovy, width, height);
}


void
ooSetPerspective(float fovy, int width, int height)
{
  SDL_Surface *window = SDL_GetVideoSurface();

  int lowx = 0, lowy = 0;

  if (window->w < width) width = window->w;
  if (window->h < height) height = window->h;

  // lowx = window->w - width;
  // lowy = window->h - height;
  // width += lowx;
  // height += lowy;

  ooLogInfo("resize to %f (%d %d) %d %d", fovy, lowx, lowy, width, height);
  glViewport(lowx, lowy, width, height);

  glMatrixMode(GL_PROJECTION);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glLoadIdentity();
  // Near clipping 1 m away, far clipping 20 au away
  gluPerspective(fovy, (double)width / (double)height,
                 /*near*/1.0, /*far*/149598000000.0*20.0);

  ooLogInfo("\tperspective %f", (double)width / (double)height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

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
ooInitGlAttributes(void)
{
  // Setup attributes we want for the OpenGL context
  int stencilSize, colourSize;
  ooConfGetIntDef("openorbit/video/stencil-bits", &stencilSize, 1);
  ooConfGetIntDef("openorbit/video/colour-bits", &colourSize, 32);

  // Don't set color bit sizes (SDL_GL_RED_SIZE, etc)
  //    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and
  //    5-5-5 RGB for 16-bit screens

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
}

void
ooPrintScreenAttributes(void)
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


