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
#include "parsers/model.h"
#include "render.h"
#include "shader-manager.h"
#include "rendering/camera.h"

sg_camera_t*
sg_get_cam(sg_viewport_t *vp)
{
  assert(vp != NULL);

  return vp->scene->cam;
}

void
sg_init_overlay(sg_overlay_t *overlay, sg_draw_overlay_t drawfunc,
                float x, float y, float w, float h, unsigned rw, unsigned rh)
{
  overlay->enabled = true;
  overlay->draw = drawfunc;
  overlay->x = x;
  overlay->y = y;
  overlay->w = w;
  overlay->h = h;

  glGenFramebuffers(1, &overlay->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, overlay->fbo);
  glGenTextures(1, &overlay->tex);
  glBindTexture(GL_TEXTURE_2D, overlay->tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rw, rh, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  SG_CHECK_ERROR;

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         overlay->tex, 0);
  SG_CHECK_ERROR;

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int
compareDistances(sg_object_t const **o0, sg_object_t const **o1)
{
  bool gt = vf3_gt((*o0)->pos, (*o1)->pos);

  if (gt) return -1;
  else return 1;
}


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



void
sg_scene_set_background(sg_scene_t *sc, sg_background_t *bg)
{
  sc->bg = bg;
}


void
sg_scene_render(sg_scene_t *scene, float dt)
{
  sgAnimateCam(scene->cam, dt);
  sgDrawBackground(scene->bg);
  //sgMoveCam(scene->cam);
  ARRAY_FOR_EACH(i, scene->objects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(scene->objects, i));
    sgAnimateObject(ARRAY_ELEM(scene->objects, i), dt);
    sgDrawObject(ARRAY_ELEM(scene->objects, i));
  }
}

void
sg_window_render(sg_window_t *window, float dt)
{
  ARRAY_FOR_EACH(i, window->viewports) {
    sg_set_viewport(ARRAY_ELEM(window->viewports, i));
    sg_scene_render(((sg_viewport_t*)ARRAY_ELEM(window->viewports, i))->scene, dt);
  }
}



sg_window_t*
sg_new_window(void)
{
  sg_window_t *window = malloc(sizeof(sg_window_t));
  memset(window, 0, sizeof(sg_window_t));
  return window;
}


void
sg_set_viewport(sg_viewport_t *viewport)
{
  glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
}

sg_viewport_t*
sg_new_viewport(sg_window_t *window, unsigned x, unsigned y,
                 unsigned w, unsigned h)
{
  sg_viewport_t *viewport = malloc(sizeof(sg_viewport_t));
  memset(viewport, 0, sizeof(sg_viewport_t));
  return viewport;
}

sg_scene_t*
sg_new_scene(void)
{
  sg_scene_t *scene = malloc(sizeof(sg_scene_t));
  memset(scene, 0, sizeof(sg_scene_t));
  return scene;
}
