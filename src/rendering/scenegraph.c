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
#include "scenegraph-private.h"
#include "sky.h"
#include "types.h"

#include <openorbit/log.h>
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"
#include "texture.h"
#include <vmath/lwcoord.h>

#include "render.h"
#include "shader-manager.h"
#include "rendering/camera.h"

struct sg_scenegraph_t {
  obj_array_t windows;
  obj_array_t scenes;
};


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
    ooLogError("GL invalid enum at %s:%d", file, line);
    break;
  case GL_INVALID_VALUE:
    ooLogError("GL invalid value at %s:%d", file, line);
    break;
  case GL_INVALID_OPERATION:
    ooLogError("GL invalid operation at %s:%d", file, line);
    break;
      //  case GL_STACK_OVERFLOW:
      //ooLogError("GL stack overflow at %s:%d", file, line);
      //break;
      //case GL_STACK_UNDERFLOW:
      // ooLogError("GL stack underflow at %s:%d", file, line);
      //break;
  case GL_OUT_OF_MEMORY:
    ooLogError("GL out of memory at %s:%d", file, line);
    break;
      //case GL_TABLE_TOO_LARGE:
      //ooLogError("GL invalid enum at %s:%d", file, line);
      //break;
  default:
    ooLogError("unknown GL error at %s:%d", file, line);
  }
}

sg_scene_t*
sg_scenegraph_get_scene(sg_scenegraph_t *sg, const char *name)
{
  ARRAY_FOR_EACH(i, sg->scenes) {
    sg_scene_t *sc = ARRAY_ELEM(sg->scenes, i);
    if (sg_scene_has_name(sc, name)) {
      return sc;
    }
  }
  return NULL;
}

void
sg_scenegraph_add_scene(sg_scenegraph_t *sg, sg_scene_t *sc)
{
  obj_array_push(&sg->scenes, sc);
}

void
sg_scenegraph_add_window(sg_scenegraph_t *sg, sg_window_t *win)
{
  obj_array_push(&sg->windows, win);
}

sg_scenegraph_t*
sg_new_scenegraph()
{
  sg_scenegraph_t *sg = malloc(sizeof(sg_scenegraph_t));
  memset(sg, 0, sizeof(sg_scenegraph_t));
  obj_array_init(&sg->scenes);
  obj_array_init(&sg->windows);
  return sg;
}