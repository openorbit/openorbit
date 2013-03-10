/*
  Copyright 2008,2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/types.h"
#include "scenegraph.h"
#include "sky.h"
#include "types.h"

#include <openorbit/log.h>
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"
#include "texture.h"
#include <vmath/lwcoord.h>

#include "shader-manager.h"
#include "rendering/camera.h"



// Drawing is done as follows:
//   when drawing is commanded with a camera, we get the cameras scene and
//   paint that, after that, we go to the cam scenes parent and inverse
//   any rotations and transforms and paint that

void
sgAssertNoGLError(void)
{
  assert(glGetError() == GL_NO_ERROR);
}
void
sgClearGLError(void)
{
  glGetError();
}

void
sgCheckGLError(const char *file, int line)
{
  GLenum err = glGetError();
  switch (err) {
  case GL_NO_ERROR:
    // All OK
    break;
  case GL_INVALID_ENUM:
    log_error("GL invalid enum at %s:%d", file, line);
    break;
  case GL_INVALID_VALUE:
    log_error("GL invalid value at %s:%d", file, line);
    break;
  case GL_INVALID_OPERATION:
    log_error("GL invalid operation at %s:%d", file, line);
    break;
      //  case GL_STACK_OVERFLOW:
      //log_error("GL stack overflow at %s:%d", file, line);
      //break;
      //case GL_STACK_UNDERFLOW:
      // log_error("GL stack underflow at %s:%d", file, line);
      //break;
  case GL_OUT_OF_MEMORY:
    log_error("GL out of memory at %s:%d", file, line);
    break;
      //case GL_TABLE_TOO_LARGE:
      //log_error("GL invalid enum at %s:%d", file, line);
      //break;
  default:
    log_error("unknown GL error at %s:%d", file, line);
  }
}
