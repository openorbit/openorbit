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
#include "SDL_opengl.h"

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
static void ooPrintScreenAttributes(void);

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
    fprintf(stderr, "Couldn't set %dx%d OpenGL video mode: %s\n",
            width, height, SDL_GetError());
    SDL_Quit();
    exit(2);
  }
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

  ooInitSdlScreen(width, height, fullscreen);
  ooSetPerspective(fovy, width, height);
}


void
ooSetPerspective(float fovy, int width, int height)
{
  glMatrixMode(GL_PROJECTION);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glLoadIdentity();

  // Near clipping 1 cm away, far clipping 20 au away
  gluPerspective(fovy, (double)width / (double)height,
                /*near*/0.1, /*far*/149598000000.0*20.0);

  glViewport(0, 0, width, height);
}

void
ooInitGlAttributes(void)
{
  // Setup attributes we want for the OpenGL context
  int depthSize, stencilSize, colourSize;
  ooConfGetIntDef("openorbit/video/depth-bits", &depthSize, 16);
  ooConfGetIntDef("openorbit/video/stencil-bits", &stencilSize, 1);
  ooConfGetIntDef("openorbit/video/colour-bits", &colourSize, 32);

  int value;

  // Don't set color bit sizes (SDL_GL_RED_SIZE, etc)
  //    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and
  //    5-5-5 RGB for 16-bit screens

  // Request a 16-bit depth buffer (without this, there is no depth buffer)
  if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthSize)) {
    ooLogFatal("could not set depth to size %d SDL: \"%s\"", depthSize, SDL_GetError());
  }
  
  // Request double-buffered OpenGL
  //     The fact that windows are double-buffered on Mac OS X has no effect
  //     on OpenGL double buffering.
  if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
    ooLogFatal("could not set gl double buffer SDL: \"%s\"", SDL_GetError());
  }
}

static void
ooPrintScreenAttributes(void)
{
  // Print out attributes of the context we created
  int nAttr;
  int i;

  int  attr[] = { SDL_GL_RED_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_GREEN_SIZE,
                  SDL_GL_ALPHA_SIZE, SDL_GL_BUFFER_SIZE, SDL_GL_DEPTH_SIZE };

  char *desc[] = { "Red size: %d bits\n", "Blue size: %d bits\n",
                   "Green size: %d bits\n",
                   "Alpha size: %d bits\n", "Color buffer size: %d bits\n", 
                   "Depth bufer size: %d bits\n" };

  nAttr = sizeof(attr) / sizeof(int);

  for (i = 0; i < nAttr; i++) {
    int value;
    SDL_GL_GetAttribute(attr[i], &value);
    printf(desc[i], value);
  }
}


