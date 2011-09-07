/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "settings.h"
#include "camera.h"
#include "sky.h"
#include "texture.h"
#include "planet.h"
#include "log.h"
#include "common/moduleinit.h"
#include "io-manager.h"

//static SDL_Surface *gScreen = NULL;
SGrenderinfo sgRenderInfo;

void
ooSetVideoDefaults(void)
{
  int width, height;
  float fovy;
  bool fullscreen;

  ooConfGetIntDef("openorbit/video/width", &width, 640);
  ooConfGetIntDef("openorbit/video/height", &height, 480);
  ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);

  sgRenderInfo.w = width;
  sgRenderInfo.h = height;
  sgRenderInfo.aspect = (float)width/(float)height;
  sgRenderInfo.fovy = fovy;

  ooSetPerspective(fovy, width, height);
}

void
ooResizeScreen(int x, int y, int width, int height, bool fullscreen)
{
  float fovy;
  ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);

  sgRenderInfo.w = width;
  sgRenderInfo.h = height;
  sgRenderInfo.aspect = (float)width/(float)height;

  ooLogInfo("resize to %f (%d %d) %d %d", fovy, x, y, width, height);

  glViewport(x, y, width, height);

  ooSetPerspective(fovy, width, height);
}

void
ooSetPerspective(float fovy, int width, int height)
{
  glMatrixMode(GL_PROJECTION);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glLoadIdentity();
  // Near clipping 1 m away, far clipping 20 au away
  gluPerspective(fovy, (double)width / (double)height,
                 /*near*/0.9, /*far*/149598000000.0*20.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


bool
sgGlslVersionIsAtLeast(unsigned major, unsigned minor)
{
  return (major > sgRenderInfo.glsl_major_vers) ||
         ((major == sgRenderInfo.glsl_major_vers) &&
          (minor >= sgRenderInfo.glsl_minor_vers));
}

bool
sgGlslVersionIsAtMost(unsigned major, unsigned minor)
{
  return (major < sgRenderInfo.glsl_major_vers) ||
  ((major == sgRenderInfo.glsl_major_vers) &&
   (minor <= sgRenderInfo.glsl_minor_vers));
}

bool
sgGlslVersionIs(unsigned major, unsigned minor)
{
  return (major == sgRenderInfo.glsl_major_vers) &&
         (minor == sgRenderInfo.glsl_minor_vers);
}


